"""Die gemessene Uebertragungskurve der Oud (Abdallah u. a. 2026, Abb. 6) gegen die gerechnete (34k-5, S196).

Zwei Aufrufe:

  --lesen <abb6.png> <kurve.txt>
      liest die blaue Kurve („Blue Tip“) aus dem Ausschnitt der Abbildung (Seite 8 bei 300 dpi, Ausschnitt
      0,33–0,93 der Breite und 0,585–0,82 der Hoehe) und schreibt Frequenz und Amplitude je Bildspalte.
      Die Achsen werden am Rahmen der Abbildung gefunden (0–400 Hz, 0–8 m/s²/N).

  --vergleich <kurve.txt> <bild.png> <name>=<formen.txt> [<name>=<formen.txt> …] [--eta <wert>]
      rechnet aus den Formen (`InstrumentBodyTool --shapes`, mit Luftraum die gekoppelten Moden, Masse 1)
      die Beschleunigung je Kraft am Messpunkt und legt sie neben die Messung:
        A(f) = Σ_n −ω² φ_n(r) φ_n(e) / (ω_n² (1 + i η) − ω²)
      r = Antwortpunkt 3 aus Abb. 5 b (roter Punkt, x = 7,5, y = 32,5 im Raster der Abbildung),
      e = jeder Punkt des Anregungsrasters (5 cm) auf der Decke; gezeigt wird der quadratische Mittelwert
      ueber das Raster und die Kurve fuer die Anregung am Antwortpunkt selbst.
      Lage des Rasters im Bauplan: y_Bauplan = 46 cm − y_Abb5b (Steg 37 → 9 cm, grosses Loch 15 → 31 cm,
      kleine Loecher 25 → 21 cm; im Bauplan 9 / 30 / 20,4 cm).
      Welche Anregung Abb. 6 zeigt, sagt die Quelle nicht — verglichen werden deshalb die Lagen der Spitzen,
      nicht ihre Hoehen. η einheitlich (Vorgabe 0,02): die Daempfung der Oud ist nicht belegt.
"""
import sys

import numpy as np


def lesen(bild, ziel):
    from PIL import Image
    im = np.asarray(Image.open(bild).convert("RGB")).astype(int)
    r, g, b = im[:, :, 0], im[:, :, 1], im[:, :, 2]
    dunkel = (r < 60) & (g < 60) & (b < 60)
    # Rahmen: die laengste dunkle Zeile unten und oben, die laengste dunkle Spalte links und rechts
    zeilen = dunkel.sum(axis=1)
    spalten = dunkel.sum(axis=0)
    voll_z = np.where(zeilen > 0.75 * zeilen.max())[0]
    voll_s = np.where(spalten > 0.75 * spalten.max())[0]
    oben, unten = voll_z.min(), voll_z.max()
    links, rechts = voll_s.min(), voll_s.max()
    blau = (b > 200) & (r < 90) & (g > 130)
    punkte = []
    for x in range(links + 3, rechts - 2):
        ys = np.where(blau[oben + 3:unten - 2, x])[0]
        if len(ys) == 0:
            continue
        y = oben + 3 + (ys.min() + ys.max()) / 2.0 if ys.max() - ys.min() < 14 else oben + 3 + ys.min() + 3.0
        f = 400.0 * (x - links) / (rechts - links)
        a = 8.0 * (unten - y) / (unten - oben)
        punkte.append((f, a))
    with open(ziel, "w", encoding="utf-8") as aus:
        aus.write("# Abdallah et al. 2026 (Meccanica 61:89, CC BY 4.0), Fig. 6, blue tip; read from the figure\n")
        aus.write(f"# frame px: left {links} right {rechts} top {oben} bottom {unten}\n# Hz  m/s2/N\n")
        for f, a in punkte:
            aus.write(f"{f:.2f} {a:.3f}\n")
    print(f"Rahmen {links}..{rechts} x {oben}..{unten}, {len(punkte)} Punkte -> {ziel}")


def spitzen(f, a, mindest, abstand_hz=4.0):
    """Lokale Maxima, die um mindestens `mindest` (Anteil) ueber beide Nachbartaeler ragen."""
    from scipy.signal import find_peaks
    a = np.asarray(a)
    schritt = float(np.median(np.diff(f)))
    orte, _ = find_peaks(a, prominence=mindest * a.max(), distance=max(1, int(abstand_hz / schritt)))
    return [(float(f[i]), float(a[i])) for i in orte]


def formen(datei):
    z = open(datei, encoding="utf-8").read().split("\n")
    n = int(z[1].split()[1])
    knoten = np.array([[float(v) for v in s.split()] for s in z[2:2 + n]])
    t = int(z[2 + n].split()[1])
    m = int(z[3 + n + t].split()[1])
    hz, w = [], []
    for k in range(m):
        hz.append(float(z[4 + n + t + 2 * k].split()[1]))
        w.append(np.array([float(v) for v in z[5 + n + t + 2 * k].split()]))
    return knoten, np.array(hz), np.array(w)


def frf(knoten, hz, w, f, eta):
    def naechster(x, y):
        return int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))

    antwort = naechster(0.075, 0.46 - 0.325)
    raster = []
    for xr in range(-15, 16, 5):
        for yr in range(0, 41, 5):
            x, y = xr / 100.0, 0.46 - yr / 100.0
            k = naechster(x, y)
            if np.hypot(knoten[k, 0] - x, knoten[k, 1] - y) < 0.012:  # auf der Decke, nicht im Loch
                raster.append(k)
    om = 2.0 * np.pi * f[:, None]
    omn = 2.0 * np.pi * hz[None, :]
    nenner = omn ** 2 * (1.0 + 1j * eta) - om ** 2
    kern = -(om ** 2) / nenner  # f x n
    alle = np.array([np.abs(kern @ (w[:, antwort] * w[:, e])) for e in raster])
    return np.sqrt((alle ** 2).mean(axis=0)), np.abs(kern @ (w[:, antwort] ** 2)), len(raster)


def vergleich(args):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    eta = 0.02
    if "--eta" in args:
        i = args.index("--eta")
        eta = float(args[i + 1])
        args = args[:i] + args[i + 2:]
    kurve, bild, faelle = args[0], args[1], args[2:]
    mess = np.loadtxt(kurve)
    mf, ma = mess[:, 0], mess[:, 1]
    gemessen = [p for p in spitzen(mf, ma, 0.04) if p[0] > 60.0]
    print("gemessen (Abb. 6, blau):", " / ".join(f"{p[0]:.0f}" for p in gemessen), "Hz")
    f = np.arange(60.0, 400.01, 0.25)
    fig, achsen = plt.subplots(len(faelle) + 1, 1, figsize=(9, 2.3 * (len(faelle) + 1)), sharex=True)
    achsen[0].plot(mf, ma, color="tab:blue")
    achsen[0].set_title("gemessen: Abdallah u. a. 2026 Abb. 6 (Blue Tip), aus dem Bild gelesen", fontsize=9)
    for p in gemessen:
        achsen[0].axvline(p[0], color="0.8", linewidth=0.6)
    for a, fall in zip(achsen[1:], faelle):
        name, datei = fall.split("=", 1)
        knoten, hz, w = formen(datei)
        mittel, punkt, n = frf(knoten, hz, w, f, eta)
        a.plot(f, mittel / mittel.max(), color="k", label=f"Mittel über {n} Rasterpunkte")
        a.plot(f, punkt / punkt.max(), color="tab:red", linewidth=0.8, label="Anregung am Antwortpunkt")
        for p in gemessen:
            a.axvline(p[0], color="0.8", linewidth=0.6)
        a.set_title(f"{name}: Moden " + " / ".join(f"{h:.0f}" for h in hz if h < 400.0) + " Hz", fontsize=9)
        a.legend(fontsize=7, loc="upper left")
        gerechnet = spitzen(f, mittel, 0.04)
        print(f"{name}: Moden unter 400 Hz", " / ".join(f"{h:.1f}" for h in hz if h < 400.0))
        print("   Spitzen der gerechneten Kurve (Mittel):", " / ".join(f"{p[0]:.0f}" for p in gerechnet), "Hz")
        # je gemessene Spitze die naechste gerechnete Spitze
        zeile = []
        for p in gemessen:
            if not gerechnet:
                continue
            q = min(gerechnet, key=lambda g: abs(g[0] - p[0]))
            zeile.append(f"{p[0]:.0f}->{q[0]:.0f} ({100.0 * (q[0] / p[0] - 1.0):+.0f} %)")
        print("   naechste gerechnete Spitze je gemessene:", ", ".join(zeile))
    achsen[-1].set_xlabel("Hz")
    achsen[-1].set_xlim(60, 400)
    fig.tight_layout()
    fig.savefig(bild, dpi=130)
    print(bild)


if __name__ == "__main__":
    if sys.argv[1] == "--lesen":
        lesen(sys.argv[2], sys.argv[3])
    elif sys.argv[1] == "--vergleich":
        vergleich(sys.argv[2:])
    else:
        raise SystemExit(__doc__)
