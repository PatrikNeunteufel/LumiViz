"""Liest die blaue Kurve (Laute) aus Woodhouse 2021, Abb. 1 (Stegadmittanz, dB re 1 m/s/N).

Aufruf: python laute_abb1_kurve.py <abb1.jpg> <ausgabe.txt> [<bild.png>]
Achsen: x logarithmisch, y linear 0 ... -75 dB. Der Rahmen wird aus den schwarzen Linien bestimmt,
die x-Skala aus den Teilstrichen 100 und 10000 Hz (rechter Rahmen).
"""
import sys

import numpy as np
from PIL import Image
from scipy.signal import find_peaks


def rahmen(grau):
    dunkel = grau < 90
    spalten = dunkel.sum(axis=0)
    zeilen = dunkel.sum(axis=1)
    xs = np.where(spalten > 0.8 * spalten.max())[0]
    ys = np.where(zeilen > 0.8 * zeilen.max())[0]
    return xs.min(), xs.max(), ys.min(), ys.max()


def teilstriche_unten(grau, x0, x1, y1):
    """Grosse Teilstriche an der unteren Achse: dunkle Spalten knapp ueber dem Rahmen."""
    band = grau[y1 - 9:y1 - 3, x0 + 3:x1 - 3] < 110
    treffer = np.where(band.sum(axis=0) >= 5)[0] + x0 + 3
    gruppen = []
    for x in treffer:
        if gruppen and x - gruppen[-1][-1] <= 2:
            gruppen[-1].append(x)
        else:
            gruppen.append([x])
    return [float(np.mean(g)) for g in gruppen]


def main():
    bild = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(int)
    grau = bild.sum(axis=2) // 3
    x0, x1, y0, y1 = rahmen(grau)
    striche = teilstriche_unten(grau, x0, x1, y1)
    print("Rahmen x", x0, x1, "y", y0, y1)
    print("Teilstriche unten (px):", [round(s, 1) for s in striche])

    r, g, b = bild[..., 0], bild[..., 1], bild[..., 2]
    blau = (b > 150) & (r < 90) & (g < 90)

    def hz(x, x100, x10000):
        return 100.0 * 10 ** (2.0 * (x - x100) / (x10000 - x100))

    def db(y):
        # Rahmen oben = 0 dB, unten = -75 dB
        return -75.0 * (y - y0) / (y1 - y0)

    return bild, blau, (x0, x1, y0, y1), striche, hz, db


if __name__ == "__main__":
    bild, blau, (x0, x1, y0, y1), striche, hz, db = main()
    # Die Zuordnung der Teilstriche wird von Hand geprueft (Ausgabe oben); x100 = erster grosser
    # Teilstrich mit Beschriftung 100, x10000 = rechter Rahmen.
    x100 = float(sys.argv[4]) if len(sys.argv) > 4 else None
    if x100 is None:
        print("x100 als viertes Argument angeben (aus den Teilstrichen)")
        sys.exit(0)
    x10000 = float(x1)
    fs, ds = [], []
    for x in range(x0 + 2, x1 - 1):
        ys = np.where(blau[y0 + 2:y1 - 1, x])[0]
        if ys.size == 0:
            continue
        ys = ys + y0 + 2
        # Kurvenmitte: bei steilen Flanken deckt die Linie viele Zeilen, Mitte nehmen
        fs.append(hz(x, x100, x10000))
        ds.append(db((ys.min() + ys.max()) / 2.0))
        # Spitze: oberster Punkt
    fs, ds = np.array(fs), np.array(ds)
    # oberste blaue Pixel fuer die Spitzensuche
    fo, do = [], []
    for x in range(x0 + 2, x1 - 1):
        ys = np.where(blau[y0 + 2:y1 - 1, x])[0]
        if ys.size:
            fo.append(hz(x, x100, x10000))
            do.append(db(ys.min() + y0 + 2))
    fo, do = np.array(fo), np.array(do)
    with open(sys.argv[2], "w", encoding="utf-8", newline="\n") as aus:
        aus.write("# Woodhouse 2021 (JASA 150), Abb. 1, blaue Kurve: Laute, Stegadmittanz senkrecht zur Decke\n")
        aus.write("# aus dem Bild gelesen (952 x 743 px); Spalten: Hz, dB re 1 m/s/N (Linienmitte), dB (Oberkante)\n")
        aus.write("# Luecken: dort verdecken andere Kurven die blaue\n")
        for f, d, o in zip(fs, ds, do):
            aus.write(f"{f:.1f}\t{d:.1f}\t{o:.1f}\n")
    print("Punkte:", len(fs), "von", fs.min(), "bis", fs.max(), "Hz")
    spitzen, eig = find_peaks(do, prominence=3.0)
    print("Spitzen (Prominenz >= 3 dB) bis 1 kHz:")
    for i, p in zip(spitzen, eig["prominences"]):
        if fo[i] <= 1000:
            print(f"  {fo[i]:7.1f} Hz  {do[i]:6.1f} dB  Prominenz {p:4.1f} dB")
    taeler, eig = find_peaks(-ds, prominence=5.0)
    print("Taeler (Prominenz >= 5 dB) bis 1 kHz:")
    for i in taeler:
        if fs[i] <= 1000:
            print(f"  {fs[i]:7.1f} Hz  {ds[i]:6.1f} dB")
    if len(sys.argv) > 3 and sys.argv[3] != "-":
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(11, 5))
        ax.semilogx(fo, do, "b-", lw=0.8)
        for i in spitzen:
            if fo[i] <= 1000:
                ax.annotate(f"{fo[i]:.0f}", (fo[i], do[i]), fontsize=7, ha="center", va="bottom")
        ax.set_xlim(70, 1000)
        ax.set_ylim(-70, -5)
        ax.grid(True, which="both", alpha=0.3)
        ax.set_xlabel("Hz")
        ax.set_ylabel("dB re 1 m/s/N")
        ax.set_title("Laute, Stegadmittanz - aus Woodhouse 2021 Abb. 1 gelesen")
        fig.savefig(sys.argv[3], dpi=110)
