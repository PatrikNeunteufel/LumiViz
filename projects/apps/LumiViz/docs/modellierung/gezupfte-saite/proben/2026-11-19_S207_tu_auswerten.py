"""S207 - Kennzahlen nach KQ34 a an den TU-Aufnahmen (ff) je Zahl der Mikrofone: Leistungssumme ueber k der 32
Mikrofone, fuer k = 1 alle 32, fuer k = 2 alle 496 Paare, fuer k = 4, 8, 16 je 500 gezogene Auswahlen, k = 32.

Aufruf: python -X utf8 -B <dieses Skript> <tsv aus tu_je_mikrofon.m> [ab=<MIDI>]
Zeile der tsv: MIDI, L der Leistungssumme aller 32, L je Mikrofon (32 Werte).
"""
import itertools
import sys

import numpy as np

opts = dict(a.split("=") for a in sys.argv[2:] if "=" in a)
ab = int(opts.get("ab", 0))
z = np.array([[float(v) for v in zeile.split("\t")] for zeile in open(sys.argv[1], encoding="utf-8") if zeile.strip()])
z = z[z[:, 0] >= ab]
z = z[np.argsort(z[:, 0])]
t, summe, mik = z[:, 0].astype(int), z[:, 1], z[:, 2:]
rng = np.random.default_rng(207)
i, j = np.array([(a, b) for a in range(len(t)) for b in range(a + 1, len(t)) if 0 < t[b] - t[a] <= 4]).T


def kenn(l):
    d = np.abs(l[i] - l[j])
    return l.max() - l.min(), d.max(), np.median(d)


def lautheit(auswahl):
    """Naeherung: Leistungsmittel der Momentary-Maxima der gewaehlten Mikrofone (die Maxima liegen nicht immer im selben Fenster)."""
    return 10 * np.log10(np.mean(10 ** (mik[:, auswahl] / 10), axis=1))


print(f"{len(t)} Toene {t.min()}-{t.max()}, {mik.shape[1]} Mikrofone; Streuung ueber die Mikrofone je Ton: Median {np.median(mik.std(axis=1)):.1f} dB")
s, g, m = kenn(summe)
print(f"Leistungssumme aller 32 (wie KQ34 a)       : Spanne {s:5.1f}  groesster {g:5.1f}  mittlerer {m:4.1f}")
s, g, m = kenn(lautheit(list(range(mik.shape[1]))))
print(f"dasselbe aus den Maxima je Mikrofon         : Spanne {s:5.1f}  groesster {g:5.1f}  mittlerer {m:4.1f}   (prueft die Naeherung)")
for k in (1, 2, 4, 8, 16):
    if k <= 2:
        wahl = [list(c) for c in itertools.combinations(range(mik.shape[1]), k)]
    else:
        wahl = [list(rng.choice(mik.shape[1], k, replace=False)) for _ in range(500)]
    w = np.array([kenn(lautheit(c)) for c in wahl])
    print(f"{k:2d} Mikrofon(e), {len(wahl):3d} Auswahlen: Spanne Median {np.median(w[:, 0]):5.1f} | groesster Sprung Median {np.median(w[:, 1]):5.1f} "
          f"(5.-95. {np.percentile(w[:, 1], 5):.1f}-{np.percentile(w[:, 1], 95):.1f}, kleinster {w[:, 1].min():.1f}) | "
          f"mittlerer Median {np.median(w[:, 2]):4.1f} (5.-95. {np.percentile(w[:, 2], 5):.1f}-{np.percentile(w[:, 2], 95):.1f}) | "
          f"Anteil mit groesstem Sprung <= 12 LU: {100 * np.mean(w[:, 1] <= 12):.0f} %, mit mittlerem <= 3 LU: {100 * np.mean(w[:, 2] <= 3):.0f} %")
