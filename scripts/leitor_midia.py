#!/usr/bin/env python3
"""
leitor_midia.py — Ponte entre o MPRIS (playerctl) e o Gambiarra Keypad.

Lê periodicamente qual player está tocando no computador (Spotify, Firefox,
Chrome, VLC, etc. — qualquer coisa que apareça no painel de mídia do KDE já
funciona aqui, porque é a mesma fonte: MPRIS via D-Bus) e envia
artista/música para o ESP32 via HTTP GET em /update.

Requisitos:
    sudo dnf install playerctl        # Fedora
    pip install requests

Uso:
    python3 leitor_midia.py
    python3 leitor_midia.py --url http://gambiarra.local --interval 2 --debug
"""

import argparse
import logging
import subprocess
import sys
import time
from dataclasses import dataclass
from typing import Optional

import requests

# --- CONFIGURAÇÃO PADRÃO ---
URL_PADRAO = "http://gambiarra.local"
INTERVALO_PADRAO = 2.0          # segundos entre checagens (mesmo ritmo do polling da página web)
TIMEOUT_HTTP = 2.0              # segundos — não pode travar o script se o ESP32 estiver fora do ar
MAX_TENTATIVAS_HTTP = 3
TAM_MAX_CAMPO = 40              # corta strings gigantes antes de mandar (a tela é pequena)

# Ordem de prioridade quando há mais de um player tocando ao mesmo tempo.
# "mprisence_web" vem primeiro: é o player registrado pelo bridge do mprisence
# (extensão + native host), que expõe artista/música REAIS da Media Session da
# página — diferente do MPRIS nativo do Chromium, que só espelha o título da
# aba (por isso "Madness | YouTube Music" aparecia sem artista). Sem o bridge
# instalado esse player simplesmente não aparece e o script cai pros outros.
# Ajuste conforme seu uso — nomes batem com o prefixo retornado por `playerctl -l`.
PRIORIDADE_PLAYERS = ["mprisence_web", "spotify", "firefox", "chromium", "chrome", "vlc"]

# Sufixos que navegadores costumam colar no título da aba (usados só como
# fallback de limpeza quando o player NÃO é o mprisence_web, ou seja, quando
# só temos o MPRIS "cru" do navegador e o artista provavelmente virá vazio).
SUFIXOS_TITULO_ABA = [
    " | YouTube Music",
    " - YouTube Music",
    " | YouTube",
    " - YouTube",
]

logging.basicConfig(
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
    level=logging.INFO,
)
log = logging.getLogger("gambiarra")


@dataclass
class Faixa:
    artista: str
    musica: str
    tocando: bool

    def __eq__(self, other):
        if not isinstance(other, Faixa):
            return False
        return (self.artista, self.musica, self.tocando) == (
            other.artista, other.musica, other.tocando
        )


def _playerctl(args: list[str]) -> Optional[str]:
    """Roda um comando playerctl e retorna stdout limpo, ou None se falhar/vazio."""
    try:
        resultado = subprocess.run(
            ["playerctl", *args],
            capture_output=True,
            text=True,
            timeout=3,
        )
    except FileNotFoundError:
        log.error("playerctl não encontrado. Instale com: sudo dnf install playerctl")
        sys.exit(1)
    except subprocess.TimeoutExpired:
        return None

    saida = resultado.stdout.strip()
    return saida if saida else None


def listar_players() -> list[str]:
    saida = _playerctl(["-l"])
    if not saida:
        return []
    return [linha.strip() for linha in saida.splitlines() if linha.strip()]


def escolher_player(players: list[str]) -> Optional[str]:
    """Entre os players ativos, prioriza quem está tocando (Playing) e depois
    a ordem definida em PRIORIDADE_PLAYERS."""
    if not players:
        return None

    tocando = []
    for p in players:
        status = _playerctl(["-p", p, "status"])
        if status == "Playing":
            tocando.append(p)

    candidatos = tocando if tocando else players

    def chave_prioridade(nome_player: str) -> int:
        base = nome_player.split(".")[0].lower()
        for i, prefixo in enumerate(PRIORIDADE_PLAYERS):
            if base.startswith(prefixo):
                return i
        return len(PRIORIDADE_PLAYERS)

    candidatos.sort(key=chave_prioridade)
    return candidatos[0]


def limpar_titulo_aba(titulo: str) -> str:
    """Remove sufixos de site (ex: '- YouTube Music') que sobram quando o
    MPRIS só tem o título da aba do navegador, sem metadados reais."""
    for sufixo in SUFIXOS_TITULO_ABA:
        if titulo.endswith(sufixo):
            return titulo[: -len(sufixo)].strip()
    return titulo


def ler_faixa_atual() -> Optional[Faixa]:
    players = listar_players()
    player = escolher_player(players)
    if not player:
        return None

    eh_bridge = player.split(".")[0].lower().startswith("mprisence_web")

    artista = _playerctl(["-p", player, "metadata", "artist"])
    musica = _playerctl(["-p", player, "metadata", "title"]) or "Sem titulo"
    status = _playerctl(["-p", player, "status"]) or "Stopped"

    musica = musica.strip()

    if artista:
        artista = artista.strip()
    elif not eh_bridge:
        # Sem o bridge, o "title" costuma ser o título cru da aba
        # (ex: "Madness | YouTube Music"). Limpamos o sufixo do site, mas
        # não inventamos um artista que não temos.
        musica = limpar_titulo_aba(musica)
        artista = ""
    else:
        artista = ""

    return Faixa(
        artista=artista[:TAM_MAX_CAMPO],
        musica=musica[:TAM_MAX_CAMPO],
        tocando=(status == "Playing"),
    )


def enviar_para_esp32(url_base: str, faixa: Faixa) -> bool:
    url = f"{url_base}/update"
    params = {"artista": faixa.artista, "musica": faixa.musica}

    for tentativa in range(1, MAX_TENTATIVAS_HTTP + 1):
        try:
            resp = requests.get(url, params=params, timeout=TIMEOUT_HTTP)
            if resp.status_code == 200:
                log.info("Enviado: %s — %s", faixa.artista, faixa.musica)
                return True
            log.warning("ESP32 respondeu %s na tentativa %d", resp.status_code, tentativa)
        except requests.exceptions.RequestException as e:
            log.warning("Falha ao conectar (tentativa %d/%d): %s", tentativa, MAX_TENTATIVAS_HTTP, e)
        time.sleep(0.5 * tentativa)  # backoff simples

    return False


def loop_principal(url_base: str, intervalo: float):
    ultima_faixa: Optional[Faixa] = None
    log.info("Monitorando mídia via playerctl. Alvo: %s", url_base)

    while True:
        try:
            faixa = ler_faixa_atual()

            if faixa is None:
                # Nenhum player ativo — só reseta uma vez, não fica martelando o ESP32
                if ultima_faixa is not None:
                    faixa_vazia = Faixa("", "Nenhuma musica", False)
                    if enviar_para_esp32(url_base, faixa_vazia):
                        ultima_faixa = None
            elif faixa != ultima_faixa:
                if enviar_para_esp32(url_base, faixa):
                    ultima_faixa = faixa

        except Exception as e:
            log.exception("Erro inesperado no loop: %s", e)

        time.sleep(intervalo)


def main():
    parser = argparse.ArgumentParser(description="Ponte de mídia PC -> Gambiarra Keypad")
    parser.add_argument("--url", default=URL_PADRAO, help=f"URL base do ESP32 (padrão: {URL_PADRAO})")
    parser.add_argument("--interval", type=float, default=INTERVALO_PADRAO, help="Intervalo entre checagens, em segundos")
    parser.add_argument("--debug", action="store_true", help="Log detalhado")
    args = parser.parse_args()

    if args.debug:
        log.setLevel(logging.DEBUG)

    try:
        loop_principal(args.url.rstrip("/"), args.interval)
    except KeyboardInterrupt:
        log.info("Encerrado pelo usuário.")


if __name__ == "__main__":
    main()