"""S209 - wie oud_feinsuche4, aber der Hohlraum bleibt bei 20,61 l (Schalentiefe aus Abb. 3, Querschnitt Annahme wie
KQ10 a) und die Masse der grossen Rosette ist frei (2-12 g; kleine je 1 g fest). Frei: Deckendicke, Steghoehe,
Stegbreite, Muendung, Rosette gross."""
import numpy as np
from scipy.optimize import minimize

import oud_feinsuche4 as f4
import oud_kombi as k
import oud_suche as s

VOLUMEN = 0.02061
START = np.array([2.0, 7.5, 18.0, 1.5, 7.0])
GRENZEN = [(1.4, 2.4), (4.0, 16.0), (10.0, 32.0), (1.2, 2.2), (2.0, 12.0)]
n = 0
bestes = (1e9, None)


def plan(x):
    return k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, x[2] / 1000, x[1] / 1000, 650.0, 10e9), volumen=VOLUMEN, muendung=x[3])


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= v <= b) for v, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), "fein5", to=1000, netz=2500, zeigen=False)
    if e is None:
        return 1e6
    f4.RG = x[4] / 1000
    lag = f4.lagen(e)
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, np.array(x, dtype=float))
        print(f"{n:4d} Kosten {c:7.2f}  " + " ".join(f"{v:.3f}" for v in x) + "  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


r = minimize(kosten, START, method="Nelder-Mead", options={"maxfev": 280, "xatol": 0.005, "fatol": 0.05,
                                                           "initial_simplex": START + np.vstack([np.zeros(5), np.diag([0.12, 2.0, 4.0, 0.15, 2.0])])})
x = bestes[1]
f4.RG = x[4] / 1000
print(f"Ende nach {n} Auswertungen: Decke {x[0]:.3f} mm, Steg 17 cm x {x[2]:.2f} x {x[1]:.2f} mm ({0.17 * x[2] * x[1] * 0.65:.1f} g), Hohlraum 20,61 l, Muendung {x[3]:.3f}, Rosette gross {x[4]:.2f} g")
e = s.rechne(plan(x), "fein5_bestes", to=1000, netz=2500)
print("   Lagen (Rosetten, Aufnehmer) gegen 139 / 203 / 280 / 362 / 385: " + " / ".join(f"{v:+.1f} %" for v in f4.lagen(e)))
for was, f in (("Bauplan (Lochraender ohne Masse)", e["alle_hz"]), ("Instrument (mit Rosetten)", f4.mit_massen(e, True, False)), ("wie gemessen (Rosetten, Aufnehmer)", f4.mit_massen(e))):
    print(f"   {was}: " + " / ".join(f"{v:.1f}" for v in f[:6]))
