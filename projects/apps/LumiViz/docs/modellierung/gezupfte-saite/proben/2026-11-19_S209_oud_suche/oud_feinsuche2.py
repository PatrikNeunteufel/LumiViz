"""S209 - Feinsuche Familie R (KQ42 a) nach Moden statt nach Spitzen, mit oder ohne Aufnehmer der Messung.
Lagen: je Ziel (139 / 203 / 280 / 362 Hz) die naechste am Antwortpunkt sichtbare Mode (wirksame Masse < 3 kg);
mit Aufnehmer: f' = f (1 - 0,5 * 5,8 g * phi(AP)^2) (Stoerungsrechnung erster Ordnung).
Aufruf: python -X utf8 -B oud_feinsuche2.py mit|ohne [frei3]
  frei3: auch die Hoehe von Balken 3 und der Balken 4 / 5 ist frei (Faktor auf Tab. 2) - nur als Probe.
"""
import sys

import numpy as np
from scipy.optimize import minimize

import oud_kombi as k
import oud_suche as s

MIT = sys.argv[1] == "mit"
FREI3 = "frei3" in sys.argv
ZIELE = (139.0, 203.0, 280.0, 362.0)
START = [1.95, 9.2, 22.0, 1.5] + ([1.0, 1.0] if FREI3 else [])
GRENZEN = [(1.4, 2.4), (5.0, 16.0), (16.0, 26.0), (1.2, 2.2)] + ([(0.6, 1.2), (0.6, 1.2)] if FREI3 else [])
n = 0
bestes = (1e9, None)


def plan(x):
    balken = {"brace 3": x[4], "brace 4": x[5], "brace 5": x[5]} if FREI3 else None
    return k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, 0.020, x[1] / 1000, 650.0, 10e9), volumen=x[2] / 1000, muendung=x[3], balken=balken)


def lagen(e):
    f = e["alle_hz"] * (1.0 - 0.5 * 0.0058 * e["phi_ap2"]) if MIT else e["alle_hz"]
    sicht = f[e["phi_ap2"] > 1.0 / 3.0]
    return [100 * (sicht[np.argmin(np.abs(sicht - z))] / z - 1) for z in ZIELE]


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= w <= b) for w, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), f"fein2_{sys.argv[1]}{'_frei3' if FREI3 else ''}", zeigen=False)
    if e is None:
        return 1e6
    lag = lagen(e)
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, np.array(x, dtype=float))
        print(f"{n:4d} Kosten {c:7.2f}  " + " ".join(f"{v:.3f}" for v in x) + "  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


x0 = np.array(START)
schritt = np.diag([0.15, 2.0, 2.0, 0.2] + ([0.1, 0.1] if FREI3 else []))
r = minimize(kosten, x0, method="Nelder-Mead", options={"maxfev": 320, "xatol": 0.005, "fatol": 0.05, "initial_simplex": x0 + np.vstack([np.zeros(len(x0)), schritt])})
x = bestes[1]
print(f"Ende nach {n} Auswertungen: Decke {x[0]:.2f} mm, Steg 17 cm x 20 x {x[1]:.1f} mm ({0.17 * 0.02 * x[1] * 650:.0f} g), Hohlraum {x[2]:.1f} l, Muendung {x[3]:.2f}"
      + (f", Balken 3 x{x[4]:.2f}, Balken 4 / 5 x{x[5]:.2f}" if FREI3 else ""))
e = s.rechne(plan(x), f"fein2_{sys.argv[1]}{'_frei3' if FREI3 else ''}_bestes")
print("   Lagen nach Moden" + (" mit Aufnehmer" if MIT else "") + ": " + " / ".join(f"{v:+.1f} %" for v in lagen(e)))
f = e["alle_hz"]
print("   Moden ohne -> mit Aufnehmer: " + "  ".join(f"{a:.0f} -> {a * (1 - 0.5 * 0.0058 * p):.0f}" for a, p in zip(f[f < 420], e["phi_ap2"])))
