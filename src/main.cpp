#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BleKeyboard.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

// --- DEFINIÇÕES DE PINOS ---
#define BTN_1 32
#define BTN_2 33
#define BTN_3 13
#define PINO_VIBRACAO 14

// --- ENCODER ROTATIVO KY-040 ---
#define ENCODER_CLK 27
#define ENCODER_DT 26
#define ENCODER_SW 25

// --- CONFIGURAÇÃO DO DISPLAY ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- BLUETOOTH & WEB SERVER ---
BleKeyboard bleKeyboard("Gambiarra Keypad", "Yuri", 100);
WebServer server(80);
bool wifiConfigAtivo = false;
unsigned long momentoSalvo = 0;
bool exibirSalvo = false;

// --- ESTRUTURA DE DADOS (MODOS) ---
struct Action {
  String type; // "media", "macro", "none"
  String val;  // "PREV", "PLAY_PAUSE", "NEXT"
  String key;  // "DOWN", "UP", "RIGHT", "a", "c", "v"
  std::vector<String> mods; // "GUI", "SHIFT", "CTRL", "ALT"
};

struct Mode {
  String name;
  Action b1;
  Action b2;
  Action b3;
};

std::vector<Mode> modes;
int modoAtual = 0;

int getTotalModos() {
  return modes.size() + 1; // +1 pro modo Configuração
}

bool encoderVolumeMode = false;

// --- VARIÁVEIS DE DEBOUNCE E ENCODER ---
unsigned long ultimoClique = 0;
int atrasoDebounce = 250;

volatile bool precisaAtualizarTela = true;
volatile int8_t encoderDir = 0;

const int8_t tabelaEncoder[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};
volatile uint8_t estadoEncoder = 0;
volatile int8_t acumuladorEncoder = 0;

void IRAM_ATTR isrEncoder() {
  uint8_t atual = (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);
  estadoEncoder = ((estadoEncoder << 2) | atual) & 0x0F;

  int8_t movimento = tabelaEncoder[estadoEncoder];
  if (movimento == 0) return;

  acumuladorEncoder += movimento;

  if (acumuladorEncoder >= 4) {
    encoderDir = 1;
    acumuladorEncoder = 0;
  } else if (acumuladorEncoder <= -4) {
    encoderDir = -1;
    acumuladorEncoder = 0;
  }
}

// --- VARIÁVEIS DE VIBRAÇÃO ---
bool vibrando = false;
unsigned long marcoTempoVibracao = 0;
int duracaoAtualVibracao = 0;
int pulsosRestantes = 0;
int duracaoPulsoPadrao = 0;
int pausaPulsoPadrao = 0;
bool emPausaEntrePulsos = false;

void iniciarVibracao(int duracaoMs) {
  pulsosRestantes = 0;
  digitalWrite(PINO_VIBRACAO, HIGH);
  vibrando = true; 
  emPausaEntrePulsos = false;
  marcoTempoVibracao = millis();
  duracaoAtualVibracao = duracaoMs;
}

void iniciarPadraoVibracao(int numPulsos, int duracaoPulsoMs, int pausaMs) {
  duracaoPulsoPadrao = duracaoPulsoMs;
  pausaPulsoPadrao = pausaMs;
  pulsosRestantes = numPulsos - 1;
  digitalWrite(PINO_VIBRACAO, HIGH);
  vibrando = true;
  emPausaEntrePulsos = false;
  marcoTempoVibracao = millis();
  duracaoAtualVibracao = duracaoPulsoMs;
}

void checarVibracao() {
  if (!vibrando) return;
  unsigned long decorrido = millis() - marcoTempoVibracao;

  if (decorrido < (unsigned long)duracaoAtualVibracao) return;

  if (!emPausaEntrePulsos) {
    digitalWrite(PINO_VIBRACAO, LOW);
    if (pulsosRestantes > 0) {
      emPausaEntrePulsos = true;
      marcoTempoVibracao = millis();
      duracaoAtualVibracao = pausaPulsoPadrao;
    } else {
      vibrando = false;
    }
  } else {
    pulsosRestantes--;
    digitalWrite(PINO_VIBRACAO, HIGH);
    emPausaEntrePulsos = false;
    marcoTempoVibracao = millis();
    duracaoAtualVibracao = duracaoPulsoPadrao;
  }
}

void drawMediaIcon(int x, int y, String val) {
  if (val == "PREV") {
    display.fillTriangle(x+8, y, x+8, y+12, x+2, y+6, SSD1306_WHITE);
    display.fillTriangle(x+14, y, x+14, y+12, x+8, y+6, SSD1306_WHITE);
    display.drawFastVLine(x, y, 12, SSD1306_WHITE);
  } else if (val == "NEXT") {
    display.fillTriangle(x+2, y, x+2, y+12, x+8, y+6, SSD1306_WHITE);
    display.fillTriangle(x+8, y, x+8, y+12, x+14, y+6, SSD1306_WHITE);
    display.drawFastVLine(x+16, y, 12, SSD1306_WHITE);
  } else if (val == "PLAY_PAUSE") {
    display.fillTriangle(x, y, x, y+12, x+8, y+6, SSD1306_WHITE);
    display.fillRect(x+10, y, 2, 12, SSD1306_WHITE);
    display.fillRect(x+14, y, 2, 12, SSD1306_WHITE);
  }
}

void drawButtonLabel(int position, Action act) {
  int x = 4 + (position * 42); 
  int y = 30; 
  
  if (act.type == "media") {
    drawMediaIcon(x + 10, y, act.val);
  } else if (act.type == "macro") {
    display.setTextSize(1);
    display.setCursor(x, y + 2);
    String label = "";
    if (act.mods.size() > 0) label += "*"; 
    label += act.key;
    if (label.length() > 6) label = label.substring(0, 6);
    display.print(label);
  } else {
    display.drawFastHLine(x+10, y+6, 8, SSD1306_WHITE);
  }
}

// --- TELA ---
void atualizarTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (encoderVolumeMode) {
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(" VOLUME ");
    // Draw speaker icon
    int x = 50; int y = 35;
    display.fillRect(x, y+4, 6, 8, SSD1306_WHITE);
    display.fillTriangle(x+6, y+4, x+14, y, x+14, y+16, SSD1306_WHITE);
    display.fillTriangle(x+6, y+12, x+14, y+16, x+14, y, SSD1306_WHITE);
    display.drawCircle(x+18, y+8, 6, SSD1306_WHITE);
    display.drawCircle(x+18, y+8, 10, SSD1306_WHITE);
  } else if (modoAtual == modes.size()) {
    display.println("--- CONFIGURACAO ---");
    display.setCursor(0, 16);
    if (wifiConfigAtivo) {
      if (exibirSalvo) {
        display.setTextSize(2);
        display.setCursor(0, 24);
        display.println("  SALVO!  ");
        display.setTextSize(1);
      } else {
        display.println("WIFI:Gambiarra-Config");
        display.println("IP: 192.168.4.1");
        display.println("   [BLOQUEADO]   ");
        display.println("B1 p/ Sair");
      }
    } else {
      display.println("Modo Web Inativo");
      display.setCursor(0, 40);
      display.println("B1 p/ Ativar");
    }

  } else {
    display.println("--- " + modes[modoAtual].name + " ---");
    drawButtonLabel(0, modes[modoAtual].b1);
    drawButtonLabel(1, modes[modoAtual].b2);
    drawButtonLabel(2, modes[modoAtual].b3);
  }

  display.setCursor(0, 56);
  if (bleKeyboard.isConnected()) {
    display.print("BLE: ON");
  } else {
    display.print("BLE: OFF");
  }

  display.display();
}

// --- CONFIG JSON ---
void setupDefaultModes() {
  modes.clear();

  Mode m1;
  m1.name = "Controle de Midia";
  m1.b1.type = "media"; m1.b1.val = "PREV";
  m1.b2.type = "media"; m1.b2.val = "PLAY_PAUSE";
  m1.b3.type = "media"; m1.b3.val = "NEXT";
  modes.push_back(m1);

  Mode m2;
  m2.name = "Discord";
  m2.b1.type = "macro"; m2.b1.mods.push_back("GUI"); m2.b1.key = "DOWN";
  m2.b2.type = "macro"; m2.b2.mods.push_back("GUI"); m2.b2.key = "UP";
  m2.b3.type = "none";
  modes.push_back(m2);

  Mode m3;
  m3.name = "Estudo";
  m3.b1.type = "macro"; m3.b1.mods.push_back("SHIFT"); m3.b1.mods.push_back("GUI"); m3.b1.key = "RIGHT";
  m3.b2.type = "none";
  m3.b3.type = "none";
  modes.push_back(m3);
}

void parseAction(JsonObject obj, Action& act) {
  if (obj.isNull()) {
    act.type = "none";
    return;
  }
  act.type = obj["type"] | "none";
  act.val = obj["val"] | "";
  act.key = obj["key"] | "";
  act.mods.clear();
  JsonArray modsArr = obj["mods"];
  for (String m : modsArr) {
    act.mods.push_back(m);
  }
}

void saveConfig() {
  File file = LittleFS.open("/config.json", "w");
  if (!file) return;

  JsonDocument doc;
  JsonArray arr = doc["modes"].to<JsonArray>();
  for (Mode& m : modes) {
    JsonObject obj = arr.add<JsonObject>();
    obj["name"] = m.name;
    
    JsonObject b1 = obj["b1"].to<JsonObject>();
    b1["type"] = m.b1.type; b1["val"] = m.b1.val; b1["key"] = m.b1.key;
    JsonArray mods1 = b1["mods"].to<JsonArray>();
    for (String mod : m.b1.mods) mods1.add(mod);

    JsonObject b2 = obj["b2"].to<JsonObject>();
    b2["type"] = m.b2.type; b2["val"] = m.b2.val; b2["key"] = m.b2.key;
    JsonArray mods2 = b2["mods"].to<JsonArray>();
    for (String mod : m.b2.mods) mods2.add(mod);

    JsonObject b3 = obj["b3"].to<JsonObject>();
    b3["type"] = m.b3.type; b3["val"] = m.b3.val; b3["key"] = m.b3.key;
    JsonArray mods3 = b3["mods"].to<JsonArray>();
    for (String mod : m.b3.mods) mods3.add(mod);
  }
  serializeJson(doc, file);
  file.close();
}

void loadConfig() {
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    setupDefaultModes();
    saveConfig();
    return;
  }
  
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    setupDefaultModes();
    saveConfig();
    return;
  }

  modes.clear();
  JsonArray arr = doc["modes"];
  for (JsonObject m : arr) {
    Mode mode;
    mode.name = m["name"] | "Sem Nome";
    parseAction(m["b1"], mode.b1);
    parseAction(m["b2"], mode.b2);
    parseAction(m["b3"], mode.b3);
    modes.push_back(mode);
  }
  file.close();

  if (modes.size() == 0) {
    setupDefaultModes();
    saveConfig();
  }
}

// --- WEB SERVER ---

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Gambiarra Keypad Config</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #e0e0e0; margin: 0; padding: 20px; }
    h1 { color: #bb86fc; text-align: center; }
    textarea { width: 100%; height: 300px; background: #1e1e1e; color: #e0e0e0; border: 1px solid #333; padding: 10px; font-family: monospace; }
    button { background: #bb86fc; color: #121212; border: none; padding: 10px 20px; margin-top: 10px; cursor: pointer; font-weight: bold; }
    button:active { background: #9955e8; }
  </style>
</head>
<body>
  <h1>Configuracao</h1>
  <p>Edite o JSON abaixo com cuidado. Modificacoes tem efeito imediato apos salvar.</p>
  <textarea id="json-input"></textarea>
  <br>
  <button onclick="salvar()">Salvar e Aplicar</button>
  <div id="status"></div>

  <script>
    fetch('/api/config').then(r => r.text()).then(txt => {
      document.getElementById('json-input').value = txt;
    });

    function salvar() {
      const val = document.getElementById('json-input').value;
      try { JSON.parse(val); } catch(e) { alert("JSON invalido!"); return; }
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: val
      }).then(r => r.text()).then(txt => {
        document.getElementById('status').innerText = "Salvo com sucesso!";
        setTimeout(() => document.getElementById('status').innerText = "", 3000);
      });
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", htmlPage);
}

void handleGetConfig() {
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    server.send(500, "text/plain", "Erro ao abrir config");
    return;
  }
  server.streamFile(file, "application/json");
  file.close();
}

void handlePostConfig() {
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Faltando body");
    return;
  }
  String body = server.arg("plain");
  
  File file = LittleFS.open("/config.json", "w");
  file.print(body);
  file.close();

  loadConfig();
  exibirSalvo = true;
  momentoSalvo = millis();
  precisaAtualizarTela = true;
  
  server.send(200, "text/plain", "OK");
}

void startConfigServer() {
  WiFi.softAP("Gambiarra-Config");
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.begin();
  wifiConfigAtivo = true;
}

void stopConfigServer() {
  server.stop();
  WiFi.softAPdisconnect(true);
  wifiConfigAtivo = false;
}

void changeMode(int newMode) {
  modoAtual = newMode;
  
  if (modoAtual != modes.size() && wifiConfigAtivo) {
    stopConfigServer();
  }
  precisaAtualizarTela = true;
}

// --- EXECUÇÃO DE AÇÕES ---

uint8_t getModifier(String mod) {
  if (mod == "CTRL") return KEY_LEFT_CTRL;
  if (mod == "SHIFT") return KEY_LEFT_SHIFT;
  if (mod == "ALT") return KEY_LEFT_ALT;
  if (mod == "GUI" || mod == "META") return KEY_LEFT_GUI;
  return 0;
}

uint8_t getKey(String key) {
  if (key == "DOWN") return KEY_DOWN_ARROW;
  if (key == "UP") return KEY_UP_ARROW;
  if (key == "LEFT") return KEY_LEFT_ARROW;
  if (key == "RIGHT") return KEY_RIGHT_ARROW;
  if (key.length() == 1) {
    char c = key.charAt(0);
    if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    return c;
  }
  return 0;
}

const uint8_t* getMediaKey(String val) {
  if (val == "PREV") return KEY_MEDIA_PREVIOUS_TRACK;
  if (val == "NEXT") return KEY_MEDIA_NEXT_TRACK;
  if (val == "PLAY_PAUSE") return KEY_MEDIA_PLAY_PAUSE;
  if (val == "VOL_UP") return KEY_MEDIA_VOLUME_UP;
  if (val == "VOL_DOWN") return KEY_MEDIA_VOLUME_DOWN;
  if (val == "MUTE") return KEY_MEDIA_MUTE;
  return nullptr; // fallback
}

void executeAction(Action act) {
  if (!bleKeyboard.isConnected()) {
    iniciarPadraoVibracao(2, 60, 80);
    return;
  }

  if (act.type == "media") {
    const uint8_t* mk = getMediaKey(act.val);
    if (mk) bleKeyboard.write(mk);
    iniciarVibracao(50);
  } else if (act.type == "macro") {
    for (String mod : act.mods) {
      uint8_t m = getModifier(mod);
      if (m) bleKeyboard.press(m);
    }
    uint8_t k = getKey(act.key);
    if (k) bleKeyboard.press(k);
    
    delay(50);
    bleKeyboard.releaseAll();
    iniciarVibracao(50);
  } else {
    // none ou invalido, sem feedback ou um pequeno beep
  }
}

// --- SETUP E LOOP ---

void setup() {
  Serial.begin(115200);
  
  if (!LittleFS.begin(true)) {
    Serial.println("Falha ao montar LittleFS");
  }
  loadConfig();

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(PINO_VIBRACAO, OUTPUT);
  digitalWrite(PINO_VIBRACAO, LOW);

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), isrEncoder, CHANGE);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha SSD1306");
  }

  bleKeyboard.begin();
  atualizarTela();
}

void loop() {
  if (wifiConfigAtivo) {
    server.handleClient();
  }

  checarVibracao();
  unsigned long tempoAtual = millis();

  if (exibirSalvo && (tempoAtual - momentoSalvo > 2000)) {
    exibirSalvo = false;
    precisaAtualizarTela = true;
  }

  if (encoderDir != 0) {
    if (wifiConfigAtivo) {
      // Bloqueia o encoder no modo configuracao ativo
      encoderDir = 0; 
    } else if (encoderVolumeMode) {
      if (bleKeyboard.isConnected()) {
        if (encoderDir > 0) bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        else bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      }
      encoderDir = 0;
      precisaAtualizarTela = true;
    } else {
      int newMode = modoAtual + encoderDir;
      if (newMode < 0) newMode = getTotalModos() - 1;
      if (newMode >= getTotalModos()) newMode = 0;
      changeMode(newMode);
      iniciarVibracao(80);
      encoderDir = 0;
      precisaAtualizarTela = true;
    }
  }

  if (precisaAtualizarTela) {
    precisaAtualizarTela = false;
    atualizarTela();
  }
  
  if (tempoAtual - ultimoClique > atrasoDebounce) {
    static bool estAntB1 = HIGH;
    static bool estAntB2 = HIGH;
    static bool estAntB3 = HIGH;
    static bool estAntSw = HIGH;

    bool estAtualB1 = digitalRead(BTN_1);
    bool estAtualB2 = digitalRead(BTN_2);
    bool estAtualB3 = digitalRead(BTN_3);
    bool estAtualSw = digitalRead(ENCODER_SW);

    // Encoder SW Toggle Volume Mode
    if (estAntSw == HIGH && estAtualSw == LOW) {
      encoderVolumeMode = !encoderVolumeMode;
      precisaAtualizarTela = true;
      iniciarVibracao(50);
      ultimoClique = tempoAtual;
    }

    // Botões Principais (Só funcionam se não estiver no modo config)
    if (modoAtual < modes.size()) {
      if (estAntB1 == HIGH && estAtualB1 == LOW) {
        executeAction(modes[modoAtual].b1);
        ultimoClique = tempoAtual;
      }
      else if (estAntB2 == HIGH && estAtualB2 == LOW) {
        executeAction(modes[modoAtual].b2);
        ultimoClique = tempoAtual;
      }
      else if (estAntB3 == HIGH && estAtualB3 == LOW) {
        executeAction(modes[modoAtual].b3);
        ultimoClique = tempoAtual;
      }
    } else {
      // Modo Configuracao Selecionado
      if (estAntB1 == HIGH && estAtualB1 == LOW) {
        if (wifiConfigAtivo) {
          stopConfigServer();
        } else {
          startConfigServer();
        }
        iniciarVibracao(50);
        precisaAtualizarTela = true;
        ultimoClique = tempoAtual;
      }
    }

    estAntB1 = estAtualB1;
    estAntB2 = estAtualB2;
    estAntB3 = estAtualB3;
    estAntSw = estAtualSw;
  }
}
