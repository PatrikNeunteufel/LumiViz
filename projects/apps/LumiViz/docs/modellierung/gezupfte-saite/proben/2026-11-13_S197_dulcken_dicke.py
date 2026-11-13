"""Liest die Dickenkarte des Dulcken-Bodens aus Abb. 1 (Bader, Plath & Kontopidis 2025): je Bildpunkt die
naechste Farbe der Farbskala (Stufen von 0,5 mm), dann auf ein Raster von 2 cm im Mass von Abb. 2.

    python dulcken_dicke.py <abb1.png> <ziel.json> [kontrolle.png]

Massstab und Lage (aus Abb. 2 und dem Vergleich der Umrisse): Abb. 2 hat 14 px je cm, der Boden liegt dort in
x 92..2421, y 76..1191; Abb. 1 (auf 1960 px Breite verkleinert) zeigt denselben Boden in x 111..1631, unten y 790.
Koordinaten des Rasters: X laengs vom Schwanz (links) in m, Y quer von der geraden Wand (unten) in m.
"""
import json
import sys

import numpy as np
from PIL import Image
from scipy import ndimage

Image.MAX_IMAGE_PIXELS = None
PX2 = 14.0                    # px je cm in Abb. 2
K = 1520.0 / 2329.0           # Abb. 2 -> Abb. 1 (verkleinert)
SCHRITT = 0.02                # m


def main():
    a1 = np.asarray(Image.open(sys.argv[1]).convert("RGB").resize((1960, 1018), Image.LANCZOS)).astype(float)
    # Die Karte liegt halb deckend ueber einem Foto: ihre Farben treffen die Farbskala nicht genau. Zuordnung
    # der vorkommenden Kartenfarben (Haeufungen, `…_dulcken_mischung.py`) zu den Stufen von Hand, geprueft an den
    # im Text genannten Werten (Bass um 5,7, hinter der Querleiste 4,3, Diskant um 2,4, groesste um 7,5 mm):
    tafel = [((60, 100, 160), 2.75), ((127, 139, 178), 3.25), ((134, 154, 200), 2.75), ((137, 128, 133), 3.75),
             ((154, 132, 114), 4.25), ((196, 154, 125), 4.25), ((217, 159, 104), 4.75), ((238, 174, 105), 5.25),
             ((223, 185, 130), 5.75), ((242, 188, 127), 5.75), ((234, 201, 132), 6.25), ((237, 220, 134), 6.75),
             ((236, 230, 130), 7.25)]
    farben = np.array([f for f, _ in tafel], dtype=float)
    stufen = np.array([s for _, s in tafel])
    bild = a1[:830]
    abstand = np.linalg.norm(bild[:, :, None, :] - farben[None, None, :, :], axis=3)
    naechste = abstand.argmin(axis=2)
    guete = abstand.min(axis=2)
    dicke = stufen[naechste]
    mx, mn = bild.max(axis=2), bild.min(axis=2)
    foto = (mx - mn < 8.0) & (mx > 175.0)                       # das graue Foto ohne Karte
    gueltig = (guete < 38.0) & ~foto & (mx > 80.0)              # nicht Schrift und Linien
    dicke[~gueltig] = np.nan
    print("gueltige Bildpunkte:", int(gueltig.sum()))

    nx = int(round(2329.0 / PX2 / 100.0 / SCHRITT)) + 1
    ny = int(round(1115.0 / PX2 / 100.0 / SCHRITT)) + 1
    werte = np.full((ny, nx), np.nan)
    for j in range(ny):
        for i in range(nx):
            X, Y = i * SCHRITT, j * SCHRITT                    # m
            x2, y2 = 92.0 + X * 100.0 * PX2, 1191.0 - Y * 100.0 * PX2
            x1, y1 = 111.0 + (x2 - 92.0) * K, 790.0 - (1191.0 - y2) * K
            h = 6                                              # Fenster 13 px = 1,4 cm
            if not (h <= x1 < 1960 - h and h <= y1 < 830 - h):
                continue
            aus = dicke[int(y1) - h:int(y1) + h + 1, int(x1) - h:int(x1) + h + 1]
            if np.isfinite(aus).sum() >= 0.35 * aus.size:
                werte[j, i] = np.nanmedian(aus)
    print("Raster", nx, "x", ny, "Zellen mit Wert:", int(np.isfinite(werte).sum()))
    for name, x1, y1, soll in (("Bass", 250, 675, 5.72), ("Mitte", 830, 612, 5.56), ("hinter der Querleiste", 1327, 468, 4.29)):
        x2, y2 = 92.0 + (x1 - 111.0) / K, 1191.0 - (790.0 - y1) / K
        i, j = int(round((x2 - 92.0) / PX2 / 100.0 / SCHRITT)), int(round((1191.0 - y2) / PX2 / 100.0 / SCHRITT))
        nah = werte[max(j - 3, 0):j + 4, max(i - 3, 0):i + 4]
        print(f"  {name}: Abb. 1 nennt {soll} mm, gelesen im Umkreis von 6 cm Median {np.nanmedian(nah):.2f} mm (Stufen {np.nanmin(nah):.2f}..{np.nanmax(nah):.2f})")
    print("  Verteilung der Stufen (mm: Zellen):", {float(s): int((werte == s).sum()) for s in sorted(set(stufen.tolist())) if (werte == s).sum()})
    json.dump({"quelle": "Bader, Plath & Kontopidis 2025 (arXiv 2511.09037), Fig. 1: thickness map, read from the figure (bands of 0.5 mm)",
               "einheit": "mm; Raster in m: X laengs vom Schwanz, Y quer von der geraden Wand",
               "schritt": SCHRITT, "nx": nx, "ny": ny,
               "werte": [[None if not np.isfinite(v) else float(v) for v in zeile] for zeile in werte]},
              open(sys.argv[2], "w", encoding="utf-8"))
    if len(sys.argv) > 3:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(14, 7))
        bildchen = ax.imshow(werte, origin="lower", extent=(0, nx * SCHRITT, 0, ny * SCHRITT), cmap="viridis", vmin=2, vmax=7.5)
        fig.colorbar(bildchen, ax=ax, label="mm")
        ax.set_title("Dulcken 1755: Dicke des Bodens, aus Abb. 1 gelesen (Raster 2 cm)")
        fig.savefig(sys.argv[3], dpi=90)


if __name__ == "__main__":
    main()
