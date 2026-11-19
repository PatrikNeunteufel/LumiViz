"""S209 - Feinsuche Familie R (KQ42 a) mit Rosetten (Masse am Lochrand, Stoerungsrechnung) und mit oder ohne
Aufnehmer; frei: Deckendicke, Steghoehe, Stegbreite, Hohlraum, Muendung. Balken nach Tab. 2, Enden 2 mm vor dem Rand.
Lagen: je Ziel (139 / 203 / 280 / 362 Hz) die naechste am Antwortpunkt sichtbare Mode.
Aufruf: python -X utf8 -B oud_feinsuche3.py mit|ohne <Rosette gross g> <Rosetten klein zusammen g>
"""
import sys

import numpy as np
from scipy.optimize import minimize

import oud_kombi as k
import oud_suche as s

MIT = sys.argv[1] == "mit"
RG, RK = float(sys.argv[2]) / 1000, float(sys.argv[3]) / 1000
ZIELE = (139.0, 203.0, 280.0, 362.0)
START = np.array([1.95, 9.0, 20.0, 21.0, 1.45])
GRENZEN = [(1.4, 2.4), (5.0, 16.0), (10.0, 32.0), (16.0, 26.0), (1.2, 2.2)]
KENN = f"fein3_{sys.argv[1]}_{sys.argv[2]}_{sys.argv[3]}"
n = 0
bestes = (1e9, None)


def plan(x):
    return k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, x[2] / 1000, x[1] / 1000, 650.0, 10e9), volumen=x[3] / 1000, muendung=x[4])


def frequenzen(e, aufnehmer):
    """Moden des Instruments mit Rosetten (frei) und, wenn verlangt, mit dem Aufnehmer der Messung."""
    f = e["alle_hz"] * (1.0 - 0.5 * (RG * e["phi_gross2"] + RK * e["phi_klein2"]))
    return f * (1.0 - 0.5 * 0.0058 * e["phi_ap2"]) if aufnehmer else f


def lagen(e):
    f = frequenzen(e, MIT)
    sicht = f[e["phi_ap2"] > 1.0 / 3.0]
    return [100 * (sicht[np.argmin(np.abs(sicht - z))] / z - 1) for z in ZIELE]


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= w <= b) for w, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), KENN, zeigen=False)
    if e is None:
        return 1e6
    lag = lagen(e)
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, np.array(x, dtype=float))
        print(f"{n:4d} Kosten {c:7.2f}  " + " ".join(f"{v:.3f}" for v in x) + "  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


r = minimize(kosten, START, method="Nelder-Mead", options={"maxfev": 300, "xatol": 0.005, "fatol": 0.05,
                                                           "initial_simplex": START + np.vstack([np.zeros(5), np.diag([0.15, 2.0, 4.0, 2.0, 0.2])])})
x = bestes[1]
print(f"Ende nach {n} Auswertungen: Decke {x[0]:.2f} mm, Steg 17 cm x {x[2]:.1f} x {x[1]:.1f} mm ({0.17 * x[2] * x[1] * 0.65:.0f} g), Hohlraum {x[3]:.1f} l, Muendung {x[4]:.2f}")
e = s.rechne(plan(x), KENN + "_bestes")
print("   Lagen nach Moden (Rosetten" + (", Aufnehmer" if MIT else "") + "): " + " / ".join(f"{v:+.1f} %" for v in lagen(e)))
f = e["alle_hz"]
print("   Moden Bauplan -> mit Rosetten -> mit Rosetten und Aufnehmer: " + "  ".join(
    f"{a:.0f} -> {b:.0f} -> {c:.0f}" for a, b, c in zip(f[f < 420], frequenzen(e, False), frequenzen(e, True))))
