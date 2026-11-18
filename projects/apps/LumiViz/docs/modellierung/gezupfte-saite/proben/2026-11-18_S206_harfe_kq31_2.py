"""S206 - KQ31-2 a: die Harfe ohne Saiten an den Moden S(1,2) ... S(1,6) gemessen.

Je Mode: die Frequenz der hoechsten Spitze der Punktadmittanz ueber die Oesen (Fenster +-12 % um Bells Frequenz), dort
das Profil laengs der Deckleiste an den 46 Oesen (Ort nach Bell Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m), darin die
Baeuche (Maxima, die ihre Umgebung um mindestens 3 dB ueberragen). Verglichen mit Bell 1988 Abb. 7.10-7.14, Feld 2
(Kasten G in der Harfe ohne Saiten), am Seitenbild von Auge gelesen in S206 (+-1 Ort, +-2 dB; PDF 296-301):

    Lage der Frequenz <= 7 % | Zahl der Baeuche gleich | Lage jedes Bauchs <= 2 Messorte |
    Hoehe der uebrigen Baeuche gegen den staerksten <= 6 dB | Hoehe des staerksten Bauchs: nur berichtet

Dazu die Gueten der zwei Moden mit Luft (Bell Abb. 7.8, 7.9: Q 11 und 16) und S(1,7), S(1,8) als Bericht.
116 dB = 1 s/kg.

Mit `feder=<N/m>` liegt in Reihe zur Admittanz des Modells eine Feder (Y + i w / k) - Bells Messung traegt eine von
2,6e5 N/m (Harfenbericht Par. 15.2); sie hebt den Boden der Profile (bei 545 Hz auf 78 dB) und damit die schwachen
Baeuche. Die Frequenz der Spitze wird ohne Feder gesucht.

Aufruf (vom Projekt-Root): python -X utf8 -I -B <dieses Skript> [<koerper.lvinstbody>] [feder=<N/m>]
(ohne Datei: der mitgelieferte Koerper)
Schreibt nichts.
"""
import json
import math
import pathlib
import sys

import numpy as np

# Mode: Hz, Guete, Baeuche (Ort, dB) vom Diskant zum Bass; der erste ist der staerkste
BELL = {
    "S(1,2)": (249.0, 39, [(13.0, 96.0), (30.0, 78.0)]),
    "S(1,3)": (316.0, 45, [(7.5, 104.0), (18.5, 89.0), (31.5, 84.0)]),
    "S(1,4)": (432.0, 43, [(6.0, 105.5), (14.5, 91.0), (22.5, 85.5), (32.0, 84.0)]),
    "S(1,5)": (545.0, 31, [(4.0, 101.5), (10.5, 93.0), (16.5, 82.0), (25.0, 86.0), (33.5, 86.5)]),
    "S(1,6)": (709.0, 10, [(3.5, 101.0), (8.5, 95.5), (14.0, 90.0), (20.0, 86.5), (27.5, 89.5), (34.0, 88.5)]),
}
BERICHT = {"S(1,7)": (757.0, 3.0, 99.5), "S(1,8)": (905.0, 4.0, 102.0)}

args = [a for a in sys.argv[1:] if not a.startswith("feder=")]
FEDER = next((float(a[6:]) for a in sys.argv[1:] if a.startswith("feder=")), 0.0)
datei = pathlib.Path(args[0] if args else "asset/loop/instrument_bodies/concert_harp.lvinstbody")
k = json.loads(datei.read_text(encoding="utf-8"))
plan = json.loads(datei.with_suffix(".lvinstbodyplan").read_text(encoding="utf-8"))
f = np.array([m["f"] for m in k["modes"]])
eta = np.array([m["eta"] for m in k["modes"]])
steg = np.array([m["bridge"] for m in k["modes"]])
# je Oese (eigener Ort) ein Stegpunkt
ys = {}
for i, p in enumerate(plan["bridgePoints"]):
    ys.setdefault(p["y"], i)
y = np.array(sorted(ys))
pk = np.array([ys[v] for v in y])
ort = 40.0 - (y + 0.057) / 0.035
wn = 2 * np.pi * f


def profil(hz, feder=0.0):
    w = 2 * np.pi * hz
    n = 1.0 / (wn ** 2 * (1 + 1j * eta) - w * w)
    zu = 1j * w / feder if feder > 0 else 0.0
    return np.array([20 * np.log10(abs(1j * w * np.sum(steg[:, p] ** 2 * n) + zu)) + 116 for p in pk])


def spitze(mitte, breite=0.12):
    best = (0.0, -1e9)
    for hz in np.arange(mitte * (1 - breite), mitte * (1 + breite), 0.25):
        v = profil(hz).max()
        if v > best[1]:
            best = (hz, v)
    return best


def baeuche(v, schwelle=3.0):
    """Maxima des Profils, die beidseits um `schwelle` dB abfallen, bevor es hoeher geht (Prominenz)."""
    aus = []
    for i in range(len(v)):
        if any(v[j] > v[i] for j in (i - 1, i + 1) if 0 <= j < len(v)):
            continue
        # je Seite der tiefste Wert, bevor es hoeher geht; am Ende der Oesen zaehlt nur die innere Seite
        tief = []
        for schritt in (-1, 1):
            t, j = v[i], i + schritt
            if not 0 <= j < len(v):
                continue
            while 0 <= j < len(v) and v[j] <= v[i]:
                t = min(t, v[j])
                j += schritt
            if 0 <= j < len(v):  # laeuft die Seite bis ans Ende der Oesen, begrenzt sie den Bauch nicht
                tief.append(t)
        if not tief or v[i] - max(tief) >= schwelle:
            o, d = ort[i], v[i]
            if 0 < i < len(v) - 1:  # Scheitel der Parabel durch die drei Oesen (sie liegen ungleich weit)
                c = np.polyfit(ort[i - 1:i + 2], v[i - 1:i + 2], 2)
                if c[0] < 0:
                    o = -c[1] / (2 * c[0])
                    d = np.polyval(c, o)
            aus.append((o, d))
    return sorted(aus, key=lambda b: b[0])


print(f"{datei.name}: {len(f)} Moden, {len(pk)} Oesen (Ort {ort.min():.1f} ... {ort.max():.1f})"
      + (f"; Feder {FEDER:g} N/m in Reihe" if FEDER > 0 else "; ohne Feder"))
print("Moden mit Luft (Bell Abb. 7.8, 7.9: 170 Hz Q 11, 216 Hz Q 16):",
      "; ".join(f"{m['f']:.1f} Hz Q {1 / m['eta']:.1f} Luft {m['airShare']:.2f}" for m in k["modes"] if m["airShare"] > 0.1))
alles = True
for name, (hz, q, soll) in BELL.items():
    fs, _ = spitze(hz)
    v = profil(fs, FEDER)
    ist = baeuche(v)
    lage = fs / hz - 1
    stark = max(ist, key=lambda b: b[1])
    gut = abs(lage) <= 0.07 and len(ist) == len(soll)
    zeilen = []
    if len(ist) == len(soll):
        for (o1, d1), (o0, d0) in zip(ist, sorted(soll)):
            rel1, rel0 = d1 - stark[1], d0 - soll[0][1]
            ok = abs(o1 - o0) <= 2.0 and abs(rel1 - rel0) <= 6.0
            gut = gut and ok
            zeilen.append(f"      Ort {o1:5.1f} (Bell {o0:4.1f}, {o1 - o0:+.1f})  rel {rel1:6.1f} dB (Bell {rel0:6.1f}, {rel1 - rel0:+.1f})"
                          f"{'' if ok else '   <-- ausserhalb'}")
    else:
        zeilen.append("      Baeuche Modell: " + ", ".join(f"{o:.1f} ({d:.1f} dB)" for o, d in ist))
        zeilen.append("      Baeuche Bell:   " + ", ".join(f"{o:.1f} ({d:.1f} dB)" for o, d in sorted(soll)))
    alles = alles and gut
    print(f"{name}: {fs:6.1f} Hz (Bell {hz:.0f}, {100 * lage:+.1f} %), {len(ist)} Baeuche (Bell {len(soll)}), "
          f"staerkster {stark[1]:.1f} dB (Bell {soll[0][1]:.1f}, {stark[1] - soll[0][1]:+.1f} — bekannte Abweichung)  "
          f"{'in der Schwelle' if gut else 'AUSSERHALB'}")
    print("\n".join(zeilen))
for name, (hz, o0, d0) in BERICHT.items():
    fs, d = spitze(hz, 0.08)
    v = profil(fs)
    print(f"{name} (Bericht): {fs:6.1f} Hz (Bell {hz:.0f}, {100 * (fs / hz - 1):+.1f} %), staerkster {d:.1f} dB an Ort {ort[v.argmax()]:.1f} "
          f"(Bell {d0:.1f} an Ort {o0:.0f}, {d - d0:+.1f})")
print("KQ31-2 a:", "alle fuenf Moden in der Schwelle" if alles else "nicht alle Moden in der Schwelle")
