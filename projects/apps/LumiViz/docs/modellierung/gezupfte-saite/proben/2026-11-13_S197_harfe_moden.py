"""Ordnet die gerechneten Formen des Harfenbodens (aus `InstrumentBodyTool --shapes`) den Familien (m,n) zu:
m = Knotenlinien laengs (Vorzeichenwechsel quer), n = Knotenlinien quer (Vorzeichenwechsel laengs) — die Zaehlung
von Bell 1988 (Waller-Indizes). Vergleich mit Tab. 3.1 und 3.2 (freier Boden ohne Leisten, Mittel dreier Boeden).

    python harfe_moden.py <formen.txt> [--tabelle frei]
"""
import sys

import numpy as np

LAENGE, B0, B1 = 1.31, 0.41, 0.10
TAB_FREI = {(0, 6): 75, (0, 7): 105, (0, 8): 139, (0, 9): 174, (0, 10): 216, (0, 11): 258, (0, 12): 313,
            (2, 0): 277, (2, 2): 345, (2, 3): 394, (2, 4): 440}


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


def wechsel(werte, schwelle):
    """Vorzeichenwechsel einer Folge; Werte unter der Schwelle zaehlen nicht."""
    v = [x for x in werte if abs(x) > schwelle]
    return sum(1 for a, b in zip(v, v[1:]) if a * b < 0)


def indizes(knoten, w):
    gross = np.abs(w).max()

    def naechster(x, y):
        return int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))

    # quer: an den Stellen laengs, an denen die Form gross ist; die haeufigste Zahl der Wechsel
    zahlen = []
    for y in np.linspace(0.03, LAENGE - 0.03, 60):
        b = B0 + (B1 - B0) * y / LAENGE
        linie = np.array([w[naechster(x, y)] for x in np.linspace(-b / 2 + 0.002, b / 2 - 0.002, 21)])
        a = np.abs(linie).max()
        if a < 0.3 * gross:
            continue
        zahlen.append(wechsel(linie, 0.2 * a))
    m = int(np.bincount(zahlen).argmax()) if zahlen else -1
    # laengs: auf der Linie mit der groesseren Auslenkung (Mitte fuer gerade m, Rand fuer ungerade)
    beste, n = 0.0, 0
    for anteil in (0.0, 0.45):
        linie = np.array([w[naechster(anteil * (B0 + (B1 - B0) * y / LAENGE), y)] for y in np.linspace(0.005, LAENGE - 0.005, 200)])
        a = np.abs(linie).max()
        if a > beste:
            beste, n = a, wechsel(linie, 0.1 * a)
    return m, n


if __name__ == "__main__":
    knoten, hz, w = formen(sys.argv[1])
    gefunden = {}
    for k in range(len(hz)):
        mn = indizes(knoten, w[k])
        gefunden.setdefault(mn, []).append(hz[k])
    print("gerechnet (m,n): Hz")
    for mn in sorted(gefunden):
        print(f"   {mn}: " + " / ".join(f"{f:.0f}" for f in gefunden[mn]))
    if "--tabelle" in sys.argv:
        print("gegen Bell Tab. 3.1 / 3.2:")
        ab = []
        for mn, soll in TAB_FREI.items():
            if mn in gefunden:
                f = min(gefunden[mn], key=lambda v: abs(v - soll))
                ab.append((mn, soll, f, 100 * (f / soll - 1)))
                print(f"   {mn}: gemessen {soll}, gerechnet {f:.0f} ({100 * (f / soll - 1):+.0f} %)")
            else:
                print(f"   {mn}: gemessen {soll}, gerechnet -")
        null = [a[3] for a in ab if a[0][0] == 0]
        zwei = [a[3] for a in ab if a[0][0] == 2]
        if null:
            print(f"   Mittel (0,n): {np.mean(null):+.1f} %   Mittel (2,n): {np.mean(zwei) if zwei else float('nan'):+.1f} %")
