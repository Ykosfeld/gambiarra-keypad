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

Para que o ESP32 exiba o nome da música e artista que estão tocando no seu computador, é necessário rodar o script Python de ponte.

### Dependências
No seu terminal Linux, instale o controlador de mídia e a biblioteca HTTP:
```bash
sudo dnf install playerctl  # No Fedora/RedHat
# ou sudo apt install playerctl (Debian/Ubuntu)
pip install requests
```

### Configurando o Serviço (Systemd)
O script leitor_midia.py escuta as mudanças de faixa e envia requisições HTTP para `http://gambiarra.local/update`. Para rodá-lo em segundo plano automaticamente:

Edite o script informando o IP ou domínio mDNS correto do seu ESP32.

Crie um serviço de usuário do systemd em `~/.config/systemd/user/gambiarra.service.`

Ative e inicie o serviço:

```bash
systemctl --user daemon-reload
systemctl --user enable gambiarra.service
systemctl --user start gambiarra.service
```

---

## 🚀 Próximos Passos (Roadmap)
O projeto está em constante evolução. Aqui estão as melhorias planejadas para o hardware e software:

[ ] Feedback Tátil (Motor de Vibração): Adicionar um pequeno motor de vibração (via transistor NPN) no GPIO 14 para alertar fisicamente quando o ciclo do Pomodoro terminar, sem precisar olhar para a tela.

[ ] Upgrade de Display: Substituir a tela OLED de 0.96" por uma versão maior de 1.3" (SH1106) ou experimentar uma tela IPS Colorida para desenhar ícones mais ricos e capas de álbuns no modo mídia.

[ ] Gabinete Customizado: Modelar e imprimir em 3D um case definitivo para aposentar a protoboard e dar cara de produto final ao Macro Pad.