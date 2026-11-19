"""S209 - Feinsuche je Familie (Nelder-Mead ueber Deckendicke, Steghoehe, Volumen, Muendung).
  R: Balkenenden 2 mm vor dem Rand, Steg 17 cm x 20 mm, Lesart B (139 / 203 / 280 / 362 Hz)
  F: Balken wie gezeichnet, Steg 13 cm x 20 mm, Lesart A mit 139 (101 / 139 / 203 / 280 / 362 Hz)
Kosten: Summe der Quadrate der Lagefehler in Prozent (Spitzen der Kurve am Antwortpunkt 3, Raster).
Aufruf: python -X utf8 -B oud_feinsuche.py R|F
"""
import sys

import numpy as np
from scipy.optimize import minimize

import oud_kombi as k
import oud_suche as s

FAM = sys.argv[1]
ZIELE = (139.0, 203.0, 280.0, 362.0) if FAM == "R" else (101.0, 139.0, 203.0, 280.0, 362.0)
START = np.array([1.9, 10.0, 20.6, 1.7]) if FAM == "R" else np.array([1.55, 10.0, 20.6, 1.7])
GRENZEN = ((1.3, 2.4), (5.0, 16.0), (16.0, 24.0), (1.3, 2.2))
n = 0
bestes = (1e9, None)


def plan(x):
    d, h, v, m = x
    if FAM == "R":
        return k.fassung(dicke=d / 1000, abstand=0.002, steg=(0.17, 0.020, h / 1000, 650.0, 10e9), volumen=v / 1000, muendung=m)
    return k.fassung(dicke=d / 1000, steg=(0.13, 0.020, h / 1000, 650.0, 10e9), volumen=v / 1000, muendung=m)


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= w <= b) for w, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), f"fein_{FAM}", zeigen=False)
    if e is None:
        return 1e6
    sp = e["spitzen_ap"]
    lag = [100 * (min(sp, key=lambda g: abs(g[0] - z))[0] / z - 1) for z in ZIELE]
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, x.copy())
        print(f"{n:4d} Kosten {c:7.1f}  Decke {x[0]:.2f} mm, Steghoehe {x[1]:.1f} mm, Volumen {x[2]:.1f} l, Muendung {x[3]:.2f}  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


r = minimize(kosten, START, method="Nelder-Mead", options={"maxfev": 220, "xatol": 0.01, "fatol": 0.2,
                                                           "initial_simplex": START + np.vstack([np.zeros(4), np.diag([0.15, 2.0, 2.0, 0.2])])})
print("Ende:", r.x, r.fun, n, "Auswertungen")
s.rechne(plan(bestes[1]), f"fein_{FAM}_bestes")
