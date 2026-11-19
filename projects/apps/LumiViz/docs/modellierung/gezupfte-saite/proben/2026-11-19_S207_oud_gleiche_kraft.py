"""S207 - Oud: Kennzahlen nach KQ34 a (Mittel der 16 Orte, KQ36 a), wenn jeder Chor mit gleicher Kraft statt mit
gleicher Auslenkung (2 mm je Saite) gezupft wird. Der Pruefstand ist in der Auslenkung linear, die Kraft beim
Loslassen je Saite ist T d L / (x (L - x)): gleiche Kraft je Chor = Pegel je Chor um -20 lg(Summe T / Bezug).
Dazu: gleiche Kraft je Saite (-20 lg(T / Bezug)).

Aufruf: python -X utf8 -B <dieses Skript> <tsv: MIDI, Mittel der 16, ...>
"""
import math
import sys

import numpy as np

CHOERE = [(36, [35.9]), (41, [28.0, 28.0]), (45, [31.8, 31.8]), (50, [39.6, 39.6]), (55, [28.8, 28.8]), (60, [36.1, 36.1])]
z = np.array([[float(v) for v in s.split("\t")[:2]] for s in open(sys.argv[1], encoding="utf-8") if s.strip()])
t, l = z[:, 0].astype(int), z[:, 1]
chor = np.array([max(i for i, (a, _) in enumerate(CHOERE) if a <= x) for x in t])
i, j = np.array([(a, b) for a in range(len(t)) for b in range(a + 1, len(t)) if 0 < t[b] - t[a] <= 4]).T


def zeile(name, werte):
    d = np.abs(werte[i] - werte[j])
    k = int(np.argmax(d))
    print(f"{name:34s}: Spanne {werte.max() - werte.min():5.1f}  groesster {d.max():5.1f} ({t[i[k]]}/{t[j[k]]})  mittlerer {np.median(d):4.1f}   "
          f"Chor 1 gegen den Median der uebrigen {np.median(werte[chor == 0]) - np.median(werte[chor > 0]):+.1f} LU")


zeile("gleiche Auslenkung (Pruefstand)", l)
zeile("gleiche Kraft je Chor", l - np.array([20 * math.log10(sum(CHOERE[c][1]) / 60.0) for c in chor]))
zeile("gleiche Kraft je Saite", l - np.array([20 * math.log10(CHOERE[c][1][0] / 30.0) for c in chor]))
