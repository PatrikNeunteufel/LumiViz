"""S201, 34k-6 Punkt (1) — Harfe: was macht die Guete der Moden S(1,1)- / S(1,1)+ an den Tasten 50-62?

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>

Die Koerperdatei traegt die Moden 191,8 Hz und 208,4 Hz mit Guete 27. Bell 1988 misst am Kasten G in der Harfe ohne
Saiten S(1,1)- mit Q 11 (170 Hz, Abb. 7.8) und S(1,1)+ mit Q 16 (216 Hz, Abb. 7.9); am losen Kasten F 18 und 14
(Tab. 6.1). Der Pruefstand (eine Saite je Taste, ohne Mitschwingen, wie `harfe --je-taste`) mit der Datei und mit den
gemessenen Gueten an diesen zwei Moden: Lautheit (Momentary-Maximum) und frueher Abfall je Taste.

Schreibt nichts.
"""
import importlib.util
import pathlib

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("pruefstand", HIER / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

TASTEN = list(range(48, 65))


def reihe(kp):
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    aus = []
    for taste in TASTEN:
        sa, laenge, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(taste)])
        x = ps.zeit(ps.ton_mit([sa], [laenge], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
        aus.append((ps.momentary_max(x), ps.abfall(x)[0]))
    return np.array(aus)


kp = ps.lade_koerper("concert_harp")
i1, i2 = int(np.argmin(np.abs(kp["f"] - 191.8))), int(np.argmin(np.abs(kp["f"] - 208.4)))
print(f"Moden {kp['f'][i1]:.1f} Hz (Q {1 / kp['eta'][i1]:.0f}) und {kp['f'][i2]:.1f} Hz (Q {1 / kp['eta'][i2]:.0f})")
datei = reihe(kp)
kp["eta"] = kp["eta"].copy()
kp["eta"][i1], kp["eta"][i2] = 1 / 11.0, 1 / 16.0
bell = reihe(kp)
print("Taste | Datei: LUFS, Abfall frueh dB/s | Q 11 / 16: LUFS, Abfall frueh dB/s | Unterschied LU")
for t, a, b in zip(TASTEN, datei, bell):
    print(f"{t:4d}  | {a[0]:6.1f}  {a[1]:6.1f} | {b[0]:6.1f}  {b[1]:6.1f} | {b[0] - a[0]:+5.1f}")
for name, r in (("Datei    ", datei), ("Q 11 / 16", bell)):
    l = r[:, 0]
    mitte = np.max(l[7:10])                                       # Tasten 55-57
    rand = np.median(np.concatenate([l[2:6], l[11:15]]))          # Tasten 50-53 und 59-62
    paare = max(abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, min(i + 5, len(l))))
    print(f"{name}: Tasten 55-57 (lauteste) gegen den Median der Tasten 50-53 und 59-62 {mitte - rand:+.1f} LU; groesster Sprung ueber Paare bis 4 Halbtoene {paare:.1f} LU; "
          f"groesster frueher Abfall {r[:, 1].max():.1f} dB/s")
