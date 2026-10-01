"""Datei-Icons bauen: `lyrics_lrc.svg` erzeugen und alle SVG-Icons als .ico ablegen.

Aufruf (normales Python mit PySide6 und Pillow):

    python asset/img/logo/icons/build_icons.py

- schreibt `lyrics_lrc.svg` (Songtext-Datei, im Stil der `audio_*.svg`);
- legt zu jeder `*.svg` in diesem Ordner eine gleichnamige `.ico` mit den
  Groessen 16–256 px ab — das Format, das Windows fuer Datei-Icons braucht.

Die Zuordnung Endung -> Icon und der Hinweis fuer einen spaeteren Installer
stehen in `README.md` daneben. `avs.ico`, `milkdrop.ico` und `lumiviz.ico`
sind Handarbeit und werden hier nicht angefasst.
"""
import io
import pathlib
import sys

from PIL import Image
from PySide6.QtCore import QByteArray, QBuffer, QIODevice, Qt
from PySide6.QtGui import QGuiApplication, QImage, QPainter
from PySide6.QtSvg import QSvgRenderer

ORDNER = pathlib.Path(__file__).parent
GROESSEN = (16, 24, 32, 48, 64, 128, 256)
HAND = {"avs", "milkdrop", "lumiviz"}  # diese .ico gibt es schon, nicht ueberschreiben


def lrc_svg() -> str:
    """Rahmen und Grund wie die Audio-Icons; statt der Welle vier Textzeilen,
    die zweite als laufende Zeile hervorgehoben (zur Haelfte „gesungen")."""
    farbe, hell = "#ff4fa3", "#ffc2df"
    zeilen = [(30, 62, 0.35), (44, 76, 1.0), (58, 54, 0.35), (72, 68, 0.35)]
    teile = []
    for y, breite, deck in zeilen:
        x = 64 - breite / 2
        if deck >= 1.0:
            teile.append(f'  <rect x="{x - 4:.1f}" y="{y - 5}" width="{breite + 8}" height="12" '
                         f'rx="6" fill="{farbe}" fill-opacity="0.22"/>')
            teile.append(f'  <rect x="{x:.1f}" y="{y - 2}" width="{breite}" height="6" rx="3" '
                         f'fill="{hell}" fill-opacity="0.45"/>')
            teile.append(f'  <rect x="{x:.1f}" y="{y - 2}" width="{breite * 0.55:.1f}" height="6" '
                         f'rx="3" fill="{hell}"/>')
        else:
            teile.append(f'  <rect x="{x:.1f}" y="{y - 2}" width="{breite}" height="5" rx="2.5" '
                         f'fill="{farbe}" fill-opacity="{deck}"/>')
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.2" baseProfile="tiny" width="128" height="128" viewBox="0 0 128 128">
  <defs>
    <linearGradient id="rand" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#f2f2f2"/>
      <stop offset="0.5" stop-color="#7a7a7a"/>
      <stop offset="1" stop-color="#d9d9d9"/>
    </linearGradient>
    <linearGradient id="grund" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#2a2d33"/>
      <stop offset="1" stop-color="#050608"/>
    </linearGradient>
  </defs>
  <rect x="4" y="4" width="120" height="120" rx="22" ry="22" fill="url(#rand)"/>
  <rect x="10" y="10" width="108" height="108" rx="17" ry="17" fill="url(#grund)"/>
{chr(10).join(teile)}
  <rect x="24" y="92" width="80" height="1.5" fill="{farbe}" fill-opacity="0.6"/>
  <text x="64" y="112" text-anchor="middle" font-family="Segoe UI, Arial, sans-serif" font-weight="bold" font-size="20" fill="{hell}">LRC</text>
</svg>
'''


def als_png(renderer: QSvgRenderer, kante: int) -> Image.Image:
    img = QImage(kante, kante, QImage.Format_ARGB32)
    img.fill(Qt.transparent)
    p = QPainter(img)
    p.setRenderHint(QPainter.Antialiasing, True)
    p.setRenderHint(QPainter.SmoothPixmapTransform, True)
    renderer.render(p)
    p.end()
    daten = QByteArray()
    puffer = QBuffer(daten)
    puffer.open(QIODevice.WriteOnly)
    img.save(puffer, "PNG")
    return Image.open(io.BytesIO(bytes(daten))).convert("RGBA")


def main() -> int:
    app = QGuiApplication(sys.argv)  # Schriften fuer den SVG-Text
    (ORDNER / "lyrics_lrc.svg").write_text(lrc_svg(), encoding="utf-8", newline="\n")
    for svg in sorted(ORDNER.glob("*.svg")):
        if svg.stem in HAND:
            continue
        renderer = QSvgRenderer(str(svg))
        if not renderer.isValid():
            print(f"FEHLER: {svg.name} laesst sich nicht lesen")
            return 1
        bilder = [als_png(renderer, k) for k in GROESSEN]
        ziel = svg.with_suffix(".ico")
        bilder[-1].save(ziel, format="ICO", sizes=[(k, k) for k in GROESSEN],
                        append_images=bilder[:-1])
        print(f"{ziel.name}: {len(GROESSEN)} Groessen")
    del app
    return 0


if __name__ == "__main__":
    sys.exit(main())
