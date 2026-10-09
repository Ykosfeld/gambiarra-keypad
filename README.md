# Gambiarra Keypad 2.0 🚀

Um Macro Pad multifuncional, dinâmico e configurável baseado no **ESP32**, projetado para aumentar a produtividade e controlar mídia. 
Esta versão 2.0 abandonou as configurações "chumbadas" no código: agora você pode editar, adicionar e remover modos diretamente pelo navegador, sem precisar recompilar nada!

Desenvolvido em C++ utilizando a estrutura do **PlatformIO**.

## ✨ Funcionalidades Principais

- **Configuração Dinâmica (JSON + LittleFS):** Os modos e botões são configurados via uma interface Web embarcada e salvos na memória do ESP32.
- **Conectividade On-Demand:** O Wi-Fi só é ligado quando você entra no modo de configuração, economizando energia e processamento. O keypad cria sua própria rede (`Gambiarra-Config`).
- **Controle BLE:** Envia teclas e atalhos via Bluetooth Low Energy de baixíssima latência (compatível com Windows, Mac, Linux, Android e iOS).
- **Interface OLED Inteligente:** A tela desenha ícones dinamicamente (Símbolos de Mídia, Nomes de Teclas de Macro) dependendo de como o botão foi configurado.
- **Trava de Volume Global:** Apertar o botão do Encoder Rotativo alterna para o "Modo Volume", onde o giro do encoder altera o volume do sistema operacional e exibe um ícone de alto-falante na tela.
- **Feedback Tátil:** Motor de vibração embutido para confirmar troca de modos, erros de pareamento e salvamento de configurações.

---

## 🛠️ Modos Padrão (Factory Reset)
Se o aparelho for ligado pela primeira vez, ele criará automaticamente 3 modos base:
1. **Controle de Mídia:** B1 (Faixa Anterior), B2 (Play/Pause), B3 (Próxima Faixa).
2. **Discord:** B1 (Muta Mic - `GUI+DOWN`), B2 (Muta Fone - `GUI+UP`).
3. **Estudo:** B1 (Macro Customizada - `SHIFT+GUI+RIGHT`).

Gire o encoder até o final para acessar o **Modo CONFIGURACAO**, aperte B1 para ativar o Wi-Fi, conecte-se à rede `Gambiarra-Config` e acesse `http://192.168.4.1` para editar esses modos!

---

## 🛠️ Hardware Necessário

- 1x Placa de Desenvolvimento **ESP32** (ex: DevKit V1)
- 1x **Display OLED 0.96"** (Controlador SSD1306, interface I2C)
- 1x **Encoder Rotativo (KY-040)**
- 3x **Push-buttons** (Chaves tácteis)
- 1x **Motor de Vibração** (Coin motor 3V, ligado com pequeno transistor NPN se necessário)
- Protoboard e fios jumpers

*Nota: O código utiliza os resistores internos de pull-up (`INPUT_PULLUP`) do ESP32 para os botões e encoder.*

---

## ⚡ Esquema de Montagem (Wiring)

| Componente | Pino do Componente | Pino do ESP32 |
| :--- | :--- | :--- |
| **Botão 1 (Esquerda)** | Perna 1 | `GPIO 32` |
| **Botão 2 (Centro)** | Perna 1 | `GPIO 33` |
| **Botão 3 (Direita)** | Perna 1 | `GPIO 13` |
| **Encoder Rotativo** | `CLK` | `GPIO 27` |
| | `DT` | `GPIO 26` |
| | `SW` (Clique) | `GPIO 25` |
| **Motor de Vibração**| Positivo (via circuito) | `GPIO 14` |
| **Display OLED** | `SDA` | `GPIO 21` |
| | `SCL` | `GPIO 22` |

---

## 💻 Instalação e Compilação

Este projeto utiliza o **PlatformIO**. 

1. Clone o repositório.
2. Abra a pasta do projeto no **VS Code** ou **VSCodium** com a extensão PlatformIO instalada.
3. O gerenciador de dependências cuidará de instalar as bibliotecas (`ESP32 BLE Keyboard`, `Adafruit SSD1306`, `ArduinoJson`) conforme definido no `platformio.ini`.
4. Conecte o ESP32 via USB e clique no botão **Upload e Upload File System Image** na barra inferior (Para subir o código e inicializar o LittleFS).

*O `platformio.ini` já está configurado com `board_build.partitions = huge_app.csv` e `board_build.filesystem = littlefs`.*

---

## 🚀 Próximos Passos e Possíveis Upgrades
O projeto está em constante evolução. Aqui estão algumas ideias para o futuro:

- [ ] **Gabinete Customizado:** Modelar e imprimir em 3D um case definitivo para aposentar a protoboard e dar cara de produto final ao Macro Pad.
- [ ] **Construtor Visual na Web:** Melhorar a interface do servidor Web (que hoje usa edição JSON direta) para uma interface rica com botões "drag and drop" e design responsivo.
- [ ] **Ações de Hold (Clique Longo):** Expandir a configuração para suportar diferentes ações caso o usuário segure o botão ao invés de apenas clicar.
