"""Stegadmittanz der Laute: gerechnet (Formen aus `InstrumentBodyTool --shapes`) gegen Woodhouse 2021 Abb. 1.

    python laute_admittanz.py <kurve.txt> <bild.png|-> <x_m> <y_m> <name>=<formen.txt> [...] [--eta <wert>]

Punktadmittanz senkrecht zur Decke am Punkt (x, y) — bei Woodhouse „on the bridge saddle between the notches for the
first and second strings or courses“:
    Y(f) = i ω Σ_n φ_n(p)² / (ω_n² (1 + i η) − ω²),   φ massennormiert (Masse 1, wie das Werkzeug sie schreibt)
Verglichen werden die Lagen der Spitzen und ihre Hoehe in dB re 1 m/s/N.
"""
import sys

import numpy as np
from scipy.signal import find_peaks


SAITEN = 0.0  # N s/m; 2.47 = Summe der Wellenwiderstaende der 15 Saiten nach Woodhouse & Lynch-Aird 2019 Tab. II


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


def admittanz(knoten, hz, w, f, x, y, eta):
    k = int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))
    om = 2.0 * np.pi * f[:, None]
    omn = 2.0 * np.pi * hz[None, :]
    yy = (1j * om / (omn ** 2 * (1.0 + 1j * eta) - om ** 2)) @ (w[:, k] ** 2)
    yy = yy / (1.0 + SAITEN * yy)  # gedaempfte Saiten als Widerstand am Steg (Summe der Wellenwiderstaende)
    return 20.0 * np.log10(np.abs(yy)), w[:, k]


def spitzen(f, d, prominenz=3.0):
    i, _ = find_peaks(d, prominence=prominenz)
    return [(float(f[k]), float(d[k])) for k in i]


def main():
    args = sys.argv[1:]
    eta = 0.03
    if "--eta" in args:
        i = args.index("--eta")
        eta = float(args[i + 1])
        args = args[:i] + args[i + 2:]
    global SAITEN
    if "--saiten" in args:
        i = args.index("--saiten")
        SAITEN = float(args[i + 1])
        args = args[:i] + args[i + 2:]
    kurve, bild, x, y, faelle =args[0], args[1], float(args[2]), float(args[3]), args[4:]
    mess = np.loadtxt(kurve)
    mf, md = mess[:, 0], mess[:, 2]
    gem = [p for p in spitzen(mf, md) if 100.0 < p[0] < 600.0 and p[1] > -36.0]
    print("gemessen:", "  ".join(f"{p[0]:.0f} Hz {p[1]:.0f} dB" for p in gem))
    f = np.arange(70.0, 1000.0, 0.25)
    ergebnisse = []
    for fall in faelle:
        name, datei = fall.split("=", 1)
        knoten, hz, w = formen(datei)
        d, wert = admittanz(knoten, hz, w, f, x, y, eta)
        ger = [p for p in spitzen(f, d) if p[0] < 600.0]
        zeile = []
        frei = list(ger)
        for p in gem:
            if not frei:
                break
            q = min(frei, key=lambda g: abs(np.log(g[0] / p[0])))
            zeile.append(f"{p[0]:.0f}->{q[0]:.0f} ({100 * (q[0] / p[0] - 1):+.0f} %, {q[1] - p[1]:+.0f} dB)")
        print(f"{name}: Moden < 600: " + " ".join(f"{h:.0f}" for h in hz if h < 600))
        print("   Spitzen: " + "  ".join(f"{p[0]:.0f}/{p[1]:.0f}" for p in ger))
        print("   " + ", ".join(zeile))
        ergebnisse.append((name, d))
    if bild != "-":
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(11, 5.5))
        ax.semilogx(mf, md, "b-", lw=1.6, label="gemessen (Woodhouse 2021 Abb. 1, gelesen)")
        for name, d in ergebnisse:
            ax.semilogx(f, d, lw=0.9, label=name)
        ax.set_xlim(70, 1000)
        ax.set_ylim(-70, 0)
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=7)
        ax.set_xlabel("Hz")
        ax.set_ylabel("dB re 1 m/s/N")
        fig.savefig(bild, dpi=110)
        print(bild)


if __name__ == "__main__":
    main()
