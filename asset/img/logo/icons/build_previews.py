"""Vorschaubilder der Datei-Icons bauen (Session 100).

Aufruf (normales Python mit Pillow):

    python asset/img/logo/icons/build_previews.py

Legt zu jeder `.ico` in diesem Ordner ein PNG mit 64 px Kantenlaenge nach
`projects/apps/LumiViz/docs/bilder/handbuch/icons/<name>.png`. Dort liegen sie
EINMAL: das Benutzerhandbuch (§ Dateitypen und ihre Icons) und die README in
diesem Ordner zeigen dieselben Dateien. Markdown-Ansichten stellen `.ico`
nicht verlaesslich dar — darum PNG.

Nach jedem neuen oder geaenderten Icon noch einmal laufen lassen.
"""
import pathlib
import sys
import warnings

from PIL import Image

ORDNER = pathlib.Path(__file__).parent
ZIEL = ORDNER.parents[3] / "projects" / "apps" / "LumiViz" / "docs" / "bilder" / "handbuch" / "icons"
KANTE = 64


def vorschau(ico: pathlib.Path) -> Image.Image:
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")  # die Handarbeit-Icons melden eine abweichende Groesse
        bild = Image.open(ico)
        bild.size = max(bild.info["sizes"])  # die groesste enthaltene Stufe
        bild.load()
    bild = bild.convert("RGBA")
    f = KANTE / max(bild.size)
    klein = bild.resize((max(1, round(bild.width * f)), max(1, round(bild.height * f))), Image.LANCZOS)
    out = Image.new("RGBA", (KANTE, KANTE), (0, 0, 0, 0))
    out.alpha_composite(klein, ((KANTE - klein.width) // 2, (KANTE - klein.height) // 2))
    return out


def main() -> int:
    ZIEL.mkdir(parents=True, exist_ok=True)
    for ico in sorted(ORDNER.glob("*.ico")):
        vorschau(ico).save(ZIEL / (ico.stem + ".png"))
        print(f"{ico.stem}.png")
    return 0


if __name__ == "__main__":
    sys.exit(main())
