#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BleKeyboard.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

// teste da config de wallet vscodium

// --- DEFINIÇÕES DE PINOS ---
#define BTN_1 32
#define BTN_2 33
#define BTN_MODO 13

// --- CONFIGURAÇÃO DO DISPLAY ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- BLUETOOTH & WEB SERVER ---
BleKeyboard bleKeyboard("Gambiarra Keypad", "Yuri", 100);
WebServer server(80);

// --- ESTADOS DOS MODOS ---
enum Modos { PRODUTIVIDADE, MIDIA, POMODORO };
int modoAtual = PRODUTIVIDADE;
const int TOTAL_MODOS = 3;

// --- VARIÁVEIS POMODORO ---
unsigned long tempoRestante = 25 * 60;
unsigned long ultimoMillis = 0;
bool pomodoroAtivo = false;
bool faseFoco = true;

// --- VARIÁVEIS DE DEBOUNCE ---
unsigned long ultimoClique = 0;
int atrasoDebounce = 250;

// --- DADOS DE MÍDIA (Recebidos via Web) ---
String musicaAtual = "Nenhuma musica";
String artistaAtual = "Aguardando script...";

// --- HTML DA PÁGINA WEB ---
const char paginaHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Gambiarra Keypad</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #e0e0e0; text-align: center; margin: 0; padding: 20px; }
    h1 { color: #bb86fc; font-size: 1.5rem; }
    .card { background: #1e1e1e; padding: 20px; border-radius: 12px; margin: 15px auto; max-width: 400px; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
    .btn { background: #bb86fc; color: #121212; border: none; padding: 12px 20px; margin: 8px; font-size: 1rem; font-weight: bold; border-radius: 8px; cursor: pointer; width: 80%; }
    .btn:active { background: #9955e8; }
    #info { font-size: 1.1rem; color: #03dac6; margin: 10px 0; }
  </style>
</head>
<body>
  <h1>Gambiarra Keypad Web</h1>
  <div class="card">
    <h3>Midia Atual</h3>
    <p id="artista">Artista: Carregando...</p>
    <p id="musica">Musica: Carregando...</p>
  </div>
  <div class="card">
    <h3>Controles</h3>
    <button class="btn" onclick="enviarComando('prev')">|&lt; Anterior</button>
    <button class="btn" onclick="enviarComando('playpause')">Play / Pause</button>
    <button class="btn" onclick="enviarComando('next')">Proxima &gt;|</button>
  </div>
<script>
  function enviarComando(cmd) {
    fetch('/cmd?acao=' + cmd);
  }
  setInterval(() => {
    fetch('/status')
      .then(res => res.json())
      .then(data => {
        document.getElementById('artista').innerText = "Artista: " + data.artista;
        document.getElementById('musica').innerText = "Musica: " + data.musica;
      });
  }, 2000);
</script>
</body>
</html>
)rawliteral";

void atualizarTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (modoAtual == PRODUTIVIDADE) {
    display.println("--- PRODUTIVIDADE ---");
    display.setCursor(0, 20);
    display.println("B1: Copiar (Ctrl+C)");
    display.println("B2: Colar (Ctrl+V)");
  } 
  else if (modoAtual == MIDIA) {
    display.println("--- CONTROLE MIDIA ---");
    display.setCursor(0, 16);
    display.println(artistaAtual);
    display.setCursor(0, 32);
    display.println(musicaAtual);
  } 
  else if (modoAtual == POMODORO) {
    display.println("--- MODO POMODORO ---");
    display.setTextSize(2);
    display.setCursor(20, 20);
    int minutos = tempoRestante / 60;
    int segundos = tempoRestante % 60;
    if(minutos < 10) display.print("0");
    display.print(minutos);
    display.print(":");
    if(segundos < 10) display.print("0");
    display.println(segundos);
    
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Fase: ");
    display.print(faseFoco ? "FOCO" : "PAUSA");
  }

  display.setCursor(0, 56);
  display.print("IP/Web: Ativo");
  display.display();
}

void handleRoot() {
  server.send_P(200, "text/html", paginaHTML);
}

void handleStatus() {
  String json = "{\"artista\":\"" + artistaAtual + "\",\"musica\":\"" + musicaAtual + "\"}";
  server.send(200, "application/json", json);
}

void handleCmd() {
  if (server.hasArg("acao")) {
    String acao = server.arg("acao");
    if (acao == "playpause" && bleKeyboard.isConnected()) {
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    } else if (acao == "next" && bleKeyboard.isConnected()) {
      bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
    } else if (acao == "prev" && bleKeyboard.isConnected()) {
      bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleUpdateMedia() {
  if (server.hasArg("artista") && server.hasArg("musica")) {
    artistaAtual = server.arg("artista");
    musicaAtual = server.arg("musica");
    if (modoAtual == MIDIA) atualizarTela();
  }
  server.send(200, "text/plain", "Atualizado");
}

void executarAcaoBotao1() {
  if (modoAtual == PRODUTIVIDADE && bleKeyboard.isConnected()) {
    // Variável estática para alternar o estado (lembra o valor entre um clique e outro)
    static bool alternadorWorkspace = false;

    // Pressiona as teclas Modificadoras comuns: Super/Meta (Windows Key) + Left Ctrl
    bleKeyboard.press(KEY_LEFT_GUI); // Meta / Windows
    bleKeyboard.press(KEY_LEFT_CTRL);

    if (alternadorWorkspace) {
      // Comando A: Seta Direita (Próximo Workspace)
      bleKeyboard.press(KEY_RIGHT_ARROW);
      delay(50);
      bleKeyboard.releaseAll();
      Serial.println("Ação Proj: Workspace Direita");
    } else {
      // Comando B: Seta Esquerda (Workspace Anterior)
      bleKeyboard.press(KEY_LEFT_ARROW);
      delay(50);
      bleKeyboard.releaseAll();
      Serial.println("Ação Proj: Workspace Esquerda");
    }

    // Inverte o estado para o próximo clique
    alternadorWorkspace = !alternadorWorkspace;
  } 
  else if (modoAtual == MIDIA && bleKeyboard.isConnected()) {
    // Continua sendo Diminuir Volume
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
  }
  else if (modoAtual == POMODORO) {
    // Inicia ou pausa o Pomodoro
    pomodoroAtivo = !pomodoroAtivo;
    atualizarTela();
  }
}

void executarAcaoBotao2() {
  if (modoAtual == PRODUTIVIDADE && bleKeyboard.isConnected()) {
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press('v');
    delay(50);
    bleKeyboard.releaseAll();
  } 
  else if (modoAtual == MIDIA && bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  }
  else if (modoAtual == POMODORO) {
    pomodoroAtivo = false;
    faseFoco = true;
    tempoRestante = 25 * 60;
    atualizarTela();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_MODO, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha ao iniciar SSD1306"));
  }

  bleKeyboard.begin();

  // WiFiManager para conexao sem fio
  WiFiManager wm;
  // wm.resetSettings(); // Descomente caso precise resetar o wifi salvo
  
  if (!wm.autoConnect("Gambiarra-Setup")) {
    Serial.println("Falha na conexao Wi-Fi. Reiniciando...");
    ESP.restart();
  }

  Serial.println("Wi-Fi Conectado!");

  // Configura mDNS para acessar via http://gambiarra.local
  if (MDNS.begin("gambiarra")) {
    Serial.println("mDNS iniciado: http://gambiarra.local");
  }

  // Rotas do Servidor Web
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/cmd", handleCmd);
  server.on("/update", handleUpdateMedia);
  server.begin();

  atualizarTela();
}

void loop() {
  server.handleClient();
  unsigned long tempoAtual = millis();
  
  // LÓGICA DO POMODORO
  if (pomodoroAtivo && (tempoAtual - ultimoMillis >= 1000)) {
    ultimoMillis = tempoAtual;
    if (tempoRestante > 0) {
      tempoRestante--;
    } else {
      pomodoroAtivo = false;
      faseFoco = !faseFoco;
      tempoRestante = faseFoco ? (25 * 60) : (5 * 60);
    }
    if (modoAtual == POMODORO) atualizarTela();
  }

  // LEITURA DOS BOTÕES
  // LEITURA DOS BOTÕES (Com Detecção de Borda e Debounce)
  if (tempoAtual - ultimoClique > atrasoDebounce) {
    
    // Variáveis estáticas para lembrar o estado anterior de cada botão
    static bool estadoAnteriorModo = HIGH;
    static bool estadoAnteriorB1 = HIGH;
    static bool estadoAnteriorB2 = HIGH;

    bool estadoAtualModo = digitalRead(BTN_MODO);
    bool estadoAtualB1 = digitalRead(BTN_1);
    bool estadoAtualB2 = digitalRead(BTN_2);

    // --- BOTÃO DE MODO (Detecta quando é pressionado: HIGH para LOW) ---
    if (estadoAnteriorModo == HIGH && estadoAtualModo == LOW) {
      modoAtual = (modoAtual + 1) % TOTAL_MODOS;
      atualizarTela();
      ultimoClique = tempoAtual;
    }
    
    // --- BOTÃO 1 ---
    else if (estadoAnteriorB1 == HIGH && estadoAtualB1 == LOW) {
      executarAcaoBotao1();
      ultimoClique = tempoAtual;
    }
    
    // --- BOTÃO 2 ---
    else if (estadoAnteriorB2 == HIGH && estadoAtualB2 == LOW) {
      executarAcaoBotao2();
      ultimoClique = tempoAtual;
    }

    // Salva o estado atual para a próxima volta do loop
    estadoAnteriorModo = estadoAtualModo;
    estadoAnteriorB1 = estadoAtualB1;
    estadoAnteriorB2 = estadoAtualB2;
  }
}