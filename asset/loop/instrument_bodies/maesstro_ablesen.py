"""Liest die Kurven des Pruefsteins „Reference piano" (RP) aus MAESSTRO ab (S150, 31c-6 Teil 5;
Entscheide Patrik PS1 a, PF2 a) und schreibt sie als Bezug fuer die Tests.

    python -I asset/loop/instrument_bodies/maesstro_ablesen.py <MAESSTRO.pdf> [--bild <ordner>]

Quelle: B. Elie, B. Cotté, X. Boutillon, „Physically-based sound synthesis software for
Computer-Aided-Design of piano soundboards", Acta Acustica 6 (2022) 30 — die PDF liegt nur lokal
unter `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`. Gebraucht werden
`pdftoppm` (Poppler), numpy, Pillow.

Schreibt nach `projects/apps/LumiViz/tests/unit/UnitTests/werte/`:

- `maesstro_abb6_rp.txt` — Abb. 6 unten, die blaue Kurve: je „Note index" 1–84 die mittlere
  Mobilitaet nach Gl. 12 in dB re 1 s/kg. Index 1 = C1 (MIDI 24): die 84 Noten C1–B7 aus RT-0425
  Anhang A, der Text nennt „notes from C1 to G#7" (PF2 a). Abgelesen an den Ecken der Linie:
  flach die Mitte, an einer Spitze die obere Kante plus die halbe Linienbreite, in einem Tal die
  untere Kante minus die halbe Linienbreite.
- `maesstro_abb5_rp.txt` — Abb. 5, die blaue Kurve: die Modendichte (Gl. 6, gleitendes Mittel ueber
  sechs Moden) je Frequenz, alle 50 Hz von 100 bis 4950 Hz.

Beide Seiten werden mit 600 dpi gerendert (1 dB in Abb. 6 sind 34 px, 0,01 /Hz in Abb. 5 sind 16 px);
der Rahmen jedes Diagramms wird gesucht. `--bild` legt je Diagramm ein Kontrollbild mit den
abgelesenen Punkten ab — zuerst ansehen, dann den Bezug glauben.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

Image.MAX_IMAGE_PIXELS = None
WERTE = Path(__file__).resolve().parents[3] / "projects/apps/LumiViz/tests/unit/UnitTests/werte"


def seite(pdf, nummer, ordner):
    subprocess.run(["pdftoppm", "-r", "600", "-f", str(nummer), "-l", str(nummer), "-png", pdf,
                    str(Path(ordner) / "s")], check=True)
    datei = next(Path(ordner).glob(f"s-*{nummer}.png"))
    return np.asarray(Image.open(datei).convert("RGB")).astype(int)


def rahmen_x(a, y_oben, y_unten, von, bis):
    """Die senkrechten Linien des Rahmens zwischen den Spalten `von` und `bis`."""
    dunkel = a.sum(axis=2) < 450
    spalten = dunkel[int(y_oben):int(y_unten)].sum(axis=0)
    return [x for x in range(von, min(bis, a.shape[1])) if spalten[x] > 0.8 * (y_unten - y_oben)]


def kontrolle(a, x0, y0, x1, y1, punkte, pfad):
    bild = Image.fromarray(a[int(y0) - 40:int(y1) + 40, int(x0) - 40:int(x1) + 40].astype(np.uint8))
    z = ImageDraw.Draw(bild)
    for x, y in punkte:
        x, y = x - (int(x0) - 40), y - (int(y0) - 40)
        z.ellipse([x - 9, y - 9, x + 9, y + 9], outline=(255, 0, 255), width=3)
    bild.resize((bild.width // 2, bild.height // 2)).save(pfad)


def abb6(a, bild):
    """Abb. 6 unten: y = 2274,5 (-55 dB) bis 3131 (-80 dB), Index 0 am linken Rahmen, 90 am rechten."""
    y55, y80 = 2274.5, 3131.0
    x0 = float(np.mean(rahmen_x(a, y55 + 300, y80 - 20, 500, 900)))
    x90 = float(np.mean(rahmen_x(a, y55 + 300, y80 - 20, 4200, 4500)))  # unter der Legende
    r, g, b = a[:, :, 0], a[:, :, 1], a[:, :, 2]
    blau = (r < 60) & (np.abs(g - 114) < 45) & (np.abs(b - 189) < 50)  # MATLAB (0, 114, 189)
    blau[: int(y55) - 5] = False
    blau[int(y80) + 5:] = False

    def x_px(i):
        return x0 + i * (x90 - x0) / 90.0

    blau[: int(y55 + 9 / 25 * (y80 - y55)), int(x_px(80.5)):] = False  # die Legende rechts oben

    def lauf(x):
        ys = np.nonzero(blau[:, x])[0]
        if len(ys) == 0:
            return None
        stuecke = np.split(ys, np.nonzero(np.diff(ys) > 3)[0] + 1)
        return max(stuecke, key=len)

    laengen = [len(l) for l in (lauf(x) for x in range(int(x_px(45)), int(x_px(50)))) if l is not None]
    breite = float(np.percentile(laengen, 10))
    kanten = {}
    for i in range(1, 85):
        for dx in (0, -2, -4):  # die Linie endet am letzten Index
            l = lauf(int(round(x_px(i))) + dx)
            if l is not None:
                kanten[i] = (float(l[0]), float(l[-1]))
                break
    werte, punkte = [], []
    for i in range(1, 85):
        oben, unten = kanten[i]
        mitte = (oben + unten) / 2
        nach = [sum(kanten[k]) / 2 for k in (i - 1, i + 1) if k in kanten]
        y = mitte
        if unten - oben > 1.5 * breite and len(nach) == 2:
            if all(n > mitte for n in nach):
                y = oben + breite / 2
            elif all(n < mitte for n in nach):
                y = unten - breite / 2
        werte.append((i, -55.0 - (y - y55) / (y80 - y55) * 25.0))
        punkte.append((x_px(i), y))
    print(f"Abb. 6: Rahmen x {x0:.1f} … {x90:.1f}, Linienbreite {breite:.0f} px")
    if bild:
        kontrolle(a, x0, y55, x90, y80, punkte, Path(bild) / "maesstro_abb6_kontrolle.png")
    return werte


def abb5(a, bild):
    """Abb. 5: y = 652,5 (0,6 /Hz) bis 1641,5 (0 /Hz), 0 Hz am linken Rahmen, 5000 Hz am rechten."""
    y06, y0 = 652.5, 1641.5
    rahmen = rahmen_x(a, y06 + 20, y0 - 20, 300, a.shape[1])
    links = float(np.mean([x for x in rahmen if x < 2000]))
    rechts = float(np.mean([x for x in rahmen if x > 2000]))
    r, g, b = a[:, :, 0], a[:, :, 1], a[:, :, 2]
    blau = (r < 80) & (b > 150) & (b - g > 50)
    blau[: int(y06) + 5] = False
    blau[int(y0) - 2:] = False
    blau[: int(y0 - 0.38 / 0.6 * (y0 - y06)), : int(links + 0.2 * (rechts - links))] = False  # Legende

    def x_px(f):
        return links + f / 5000.0 * (rechts - links)

    werte, punkte = [], []
    for f in range(100, 5000, 50):
        ys = np.nonzero(blau[:, int(round(x_px(f))) - 1:int(round(x_px(f))) + 2].any(axis=1))[0]
        if not len(ys):
            raise SystemExit(f"Abb. 5: bei {f} Hz keine Kurve")
        y = float(np.median(ys))
        werte.append((f, (y0 - y) / (y0 - y06) * 0.6))
        punkte.append((x_px(f), y))
    print(f"Abb. 5: Rahmen x {links:.1f} … {rechts:.1f}")
    if bild:
        kontrolle(a, links, y06, rechts, y0, punkte, Path(bild) / "maesstro_abb5_kontrolle.png")
    return werte


def main(argv):
    if not argv or argv[0].startswith("--"):
        raise SystemExit(__doc__)
    pdf, bild = argv[0], None
    if len(argv) >= 3 and argv[1] == "--bild":
        bild = argv[2]
        Path(bild).mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        w6 = abb6(seite(pdf, 9, tmp), bild)
    with tempfile.TemporaryDirectory() as tmp:
        w5 = abb5(seite(pdf, 8, tmp), bild)
    kopf = "# MAESSTRO (Elie et al., Acta Acustica 6 (2022) 30), Reference piano - abgelesen von maesstro_ablesen.py\n"
    (WERTE / "maesstro_abb6_rp.txt").write_text(
        kopf + "# Abb. 6 unten: Note index (1 = C1 = MIDI 24), mittlere Mobilitaet Gl. 12 in dB re 1 s/kg\n"
        + "".join(f"{i} {db:.2f}\n" for i, db in w6), encoding="utf-8", newline="\n")
    (WERTE / "maesstro_abb5_rp.txt").write_text(
        kopf + "# Abb. 5: Frequenz in Hz, Modendichte in 1/Hz (Gl. 6, gleitend ueber sechs Moden)\n"
        + "".join(f"{f} {d:.4f}\n" for f, d in w5), encoding="utf-8", newline="\n")
    print(f"geschrieben: {WERTE / 'maesstro_abb6_rp.txt'}, {WERTE / 'maesstro_abb5_rp.txt'}")


if __name__ == "__main__":
    main(sys.argv[1:])
