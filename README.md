# Gambiarra Keypad 🚀

Um Macro Pad multifuncional baseado no **ESP32**, projetado para aumentar a produtividade e controlar mídia. O projeto combina conectividade **Bluetooth Low Energy (BLE)** para atalhos de teclado de baixa latência e um **Servidor Web via Wi-Fi** para controle remoto e exibição de metadados na tela OLED.

Desenvolvido em C++ utilizando a estrutura do **PlatformIO**.

## ✨ Funcionalidades e Modos

O Keypad possui 3 botões físicos e opera em 3 modos distintos, alternados através do botão de MODO:

1. **Modo Produtividade:**
   - **Botão 1:** Alterna workspaces para a Esquerda/Direita (Linux).
   - **Botão 2:** Colar (Ctrl+V).
2. **Modo Mídia:**
   - **Botão 1:** Diminuir Volume / Faixa Anterior.
   - **Botão 2:** Play / Pause.
   - *Integração Wi-Fi:* Exibe na tela OLED a música e o artista atual sincronizados com o PC.
3. **Modo Pomodoro:**
   - **Botão 1:** Inicia / Pausa o timer.
   - **Botão 2:** Reseta o ciclo (Foco 25 min / Pausa 5 min).

Além disso, o ESP32 hospeda uma página Web acessível via `http://gambiarra.local`, permitindo visualizar o status e controlar a mídia pelo navegador de qualquer dispositivo na rede.

---

## 🛠️ Hardware Necessário (Estado Atual)

- 1x Placa de Desenvolvimento **ESP32** (ex: DevKit V1)
- 1x **Display OLED 0.96"** (Controlador SSD1306, interface I2C)
- 3x **Push-buttons** (Chaves tácteis)
- Protoboard e fios jumpers

*Nota: Não é necessário o uso de resistores externos para os botões, pois o código utiliza os resistores internos de pull-up (`INPUT_PULLUP`) do ESP32.*

---

## ⚡ Esquema de Montagem (Wiring)

Conecte os componentes ao ESP32 seguindo o mapa abaixo:

| Componente | Pino do Componente | Pino do ESP32 |
| :--- | :--- | :--- |
| **Botão 1 (Ação A)** | Perna 1 | `GPIO 32` |
| | Perna 2 | `GND` |
| **Botão 2 (Ação B)** | Perna 1 | `GPIO 33` |
| | Perna 2 | `GND` |
| **Botão MODO** | Perna 1 | `GPIO 13` |
| | Perna 2 | `GND` |
| **Display OLED** | `SDA` | `GPIO 21` |
| | `SCL` | `GPIO 22` |
| | `VCC` / `VDD` | `3V3` (ou `VIN/5V` dependendo da tela) |
| | `GND` | `GND` |

---

## 💻 Instalação e Compilação

Este projeto utiliza o **PlatformIO**. Para compilar e enviar o código para a placa:

1. Clone o repositório:
   ```bash
   git clone https://github.com/Ykosfeld/gambiarra-keypad.git
   ```
2. Abra a pasta do projeto no **VS Code** ou **VSCodium** com a extensão PlatformIO instalada.
3. O gerenciador de dependências cuidará de instalar automaticamente as bibliotecas (`ESP32 BLE Keyboard`, `Adafruit SSD1306`, `WiFiManager`, etc.) conforme definido no `platformio.ini`.
4. Conecte o ESP32 via USB e clique no botão **Upload** (➔) na barra inferior.

*O `platformio.ini` já está configurado com `board_build.partitions = huge_app.csv` para garantir o espaço de memória necessário para o uso simultâneo de BLE e Wi-Fi.*

---

## 🐧 Integração de Mídia (Linux)

Para que o ESP32 exiba o nome da música e artista que estão tocando no seu computador, é necessário rodar o script Python de ponte (`leitor_midia.py`), que lê os metadados via **MPRIS** (o mesmo padrão usado pela aba de reprodução de mídia do KDE Plasma) e envia para `http://gambiarra.local/update`.

### Dependências

```bash
sudo dnf install playerctl   # Fedora
# ou sudo apt install playerctl (Debian/Ubuntu)
pip install requests
```

### Rodando o script

```bash
python3 leitor_midia.py --url http://gambiarra.local
```

Use `--debug` para ver o log detalhado de cada faixa detectada, e `--interval` para ajustar o intervalo de checagem (padrão: 2s).

### ⚠️ Streaming em navegador (YouTube Music, etc.)

Players "nativos" como Spotify Desktop ou VLC expõem artista e música corretamente via MPRIS sem configuração extra. **Streaming pelo navegador é diferente**: o MPRIS nativo do Chrome/Chromium/Brave para abas costuma repassar só o **título da aba**, não os metadados reais da página — então em vez de `Artista: Muse` / `Música: Madness`, o `playerctl` recebe algo como `Música: Madness | YouTube Music` e artista vazio.

O `leitor_midia.py` já tem um fallback que limpa esse sufixo (`| YouTube Music`, `- YouTube Music`, etc.), mas sem artista de verdade. Para ter artista e música separados corretamente, instale o **bridge do [mprisence](https://github.com/lazykern/mprisence)**, que expõe um player MPRIS dedicado (`mprisence_web...`) com os metadados reais da Media Session da página. O script já prioriza esse player automaticamente quando ele existe.

**Instalação (Fedora):**

```bash
# 1. Toolchain de build (Rust + C/C++ + OpenSSL)
sudo dnf install cargo openssl-devel pkgconf-pkg-config gcc gcc-c++ cmake make

# 2. Instalar o binário
cargo install mprisence

# 3. Garantir que ~/.cargo/bin está no PATH
echo 'export PATH="$HOME/.cargo/bin:$PATH"' >> ~/.bashrc   # ou ~/.zshrc, conforme seu shell
source ~/.bashrc

# 4. Instalar a extensão do navegador pela loja correspondente
#    Firefox: https://addons.mozilla.org/en-US/firefox/addon/mprisence-bridge/
#    Chrome/Chromium/Brave/Edge/Vivaldi: https://chromewebstore.google.com/detail/pnkkjbdopihogobhhjbgapbpfccinjjo

# 5. Registrar o native host e verificar
mprisence web install
mprisence web doctor
```

Recarregue a aba do site de streaming (ex: `music.youtube.com`) depois de instalar a extensão, e confirme com:

```bash
playerctl -l | grep mprisence_web
```

**Brave (e possivelmente outros navegadores baseados em Chromium fora Chrome/Chromium puro):** o `mprisence web install` só escreve o manifest do native host nas pastas de Firefox, Chromium e Google Chrome. Brave usa uma pasta própria, então é preciso copiar manualmente:

```bash
mkdir -p ~/.config/BraveSoftware/Brave-Browser/NativeMessagingHosts
cp ~/.config/chromium/NativeMessagingHosts/mprisence.web.bridge.json \
   ~/.config/BraveSoftware/Brave-Browser/NativeMessagingHosts/
```

Depois reinicie o navegador por completo (feche todas as janelas, ou `brave://restart`).

*Nota:* não é necessário deixar o `mprisence` rodando em segundo plano nem configurar Discord Rich Presence — o `web install` só registra o native host, que o navegador invoca sob demanda. Se aparecerem players MPRIS duplicados (ex: o do próprio navegador e o do `plasma-browser-integration` do KDE), não tem problema: o script já prioriza `mprisence_web` e ignora os outros automaticamente.

### Configurando o Serviço (Systemd)

Para rodar o `leitor_midia.py` em segundo plano automaticamente:

1. Copie `leitor_midia.py` para um local fixo, ex: `~/gambiarra-keypad/leitor_midia.py`.
2. Copie o arquivo `gambiarra.service` (incluído no repositório) para `~/.config/systemd/user/`, ajustando o `ExecStart` se necessário.
3. Ative e inicie o serviço:

```bash
systemctl --user daemon-reload
systemctl --user enable --now gambiarra.service
```

Para ver os logs em tempo real: `journalctl --user -u gambiarra.service -f`

---

## 🚀 Próximos Passos (Roadmap)
O projeto está em constante evolução. Aqui estão as melhorias planejadas para o hardware e software:

[ ] Feedback Tátil (Motor de Vibração): Adicionar um pequeno motor de vibração (via transistor NPN) no GPIO 14 para alertar fisicamente quando o ciclo do Pomodoro terminar, sem precisar olhar para a tela.

[ ] Upgrade de Display: Substituir a tela OLED de 0.96" por uma versão maior de 1.3" (SH1106) ou experimentar uma tela IPS Colorida para desenhar ícones mais ricos e capas de álbuns no modo mídia.

[ ] Gabinete Customizado: Modelar e imprimir em 3D um case definitivo para aposentar a protoboard e dar cara de produto final ao Macro Pad.