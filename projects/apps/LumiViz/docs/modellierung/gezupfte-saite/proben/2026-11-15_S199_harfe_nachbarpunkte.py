"""KQ22 (S199): Wie verschieden ist die Koerperantwort der Harfe an benachbarten Saiten?

Aufruf (vom Projekt-Root):
    python -X utf8 -I projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_harfe_nachbarpunkte.py \
        asset/loop/instrument_bodies/concert_harp.lvinstbody

Liest den Koerper, rechnet |H(f)| je Saite aus den diskreten Moden (wie `antwortSpektrumExakt`:
a (-w^2) / (wn^2 - w^2 + i eta wn w), a = Wert am Stegpunkt mal Abstrahlung am ersten Hoerort) und vergleicht
Nachbarn im Abstand von einer und zwei Saiten in Terzbaendern 100-2500 Hz. Ohne den statistischen Teil.

Die Saiten sind die Stegpunkte der weissen Tasten (Pedal in der Mitte); die uebrigen Tasten liegen seit KQ22 a am
Ort einer Saite und zaehlen hier nicht.

Ergebnis S199 (145 Moden): Abstand 1 Saite je Terzband im Mittel 1,5 dB, 90 % unter 3,3 dB, groesster 10,4 dB;
breitbandig im Mittel 0,4 dB, groesster 1,7 dB. Abstand 2 Saiten: 2,6 / 5,5 / 15,1 dB; breitbandig 0,7 / 2,6 dB.
"""
import json
import sys

import numpy as np

WEISS = {0, 2, 4, 5, 7, 9, 11}

d = json.load(open(sys.argv[1], encoding="utf-8"))
tasten = [p["key"] if isinstance(p, dict) else p for p in d["bridgePoints"]]
saiten = [j for j, t in enumerate(tasten) if t % 12 in WEISS]
print(f"{len(tasten)} Stegpunkte, Tasten {tasten[0]}-{tasten[-1]}, davon {len(saiten)} Saiten; {len(d['modes'])} Moden")

f = np.geomspace(80.0, 2800.0, 4000)
w = 2 * np.pi * f
H = np.zeros((len(saiten), len(f)), dtype=complex)
for m in d["modes"]:
    steg = np.array(m["bridge"])[saiten]
    ab = m["radiation"][0]
    ab = complex(ab[0], ab[1]) if isinstance(ab, list) else ab
    wn = 2 * np.pi * m["f"]
    H += np.outer(steg * ab, (-w * w) / (wn * wn - w * w + 1j * m["eta"] * wn * w))

mitten = [100 * 2 ** (k / 3) for k in range(0, 15)]  # 100 ... 2540 Hz
P = np.zeros((len(saiten), len(mitten)))
for b, fm in enumerate(mitten):
    sel = (f >= fm / 2 ** (1 / 6)) & (f < fm * 2 ** (1 / 6))
    P[:, b] = 10 * np.log10(np.mean(np.abs(H[:, sel]) ** 2, axis=1))
breit = 10 * np.log10(np.mean(np.abs(H) ** 2, axis=1))

for schritt in (1, 2):
    dt = np.abs(P[schritt:] - P[:-schritt])
    db = np.abs(breit[schritt:] - breit[:-schritt])
    wo = np.unravel_index(dt.argmax(), dt.shape)
    print(f"\nAbstand {schritt} Saite(n): je Terzband Mittel {dt.mean():.2f} dB, 90 % {np.percentile(dt, 90):.2f} dB, "
          f"groesster {dt.max():.2f} dB (Saite {wo[0]} vom Bass, Band {mitten[wo[1]]:.0f} Hz); "
          f"breitbandig Mittel {db.mean():.2f} dB, groesster {db.max():.2f} dB")
    n = dt.shape[0]
    for name, a, e in (("Bass", 0, n // 3), ("Mitte", n // 3, 2 * n // 3), ("Diskant", 2 * n // 3, n)):
        print(f"  {name:8s} je Terzband Mittel {dt[a:e].mean():.2f} dB, groesster {dt[a:e].max():.2f} dB; "
              f"breitbandig groesster {db[a:e].max():.2f} dB")
