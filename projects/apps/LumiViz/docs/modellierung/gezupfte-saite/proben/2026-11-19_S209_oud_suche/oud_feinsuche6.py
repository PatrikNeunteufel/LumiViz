"""S209 - Feinsuche Familie R mit den Rosetten als weiche Leisten im Bauplan (KQ44 a; Ring 5 mm breit, Mitte 3 mm
ausserhalb des Lochrands, 10 mm hoch, E 10 MPa, Dichte aus der Masse) und dem Aufnehmer (5,8 g) als Massenaenderung
im Modenraum (KQ43 a). Netz fuer 2500 Hz, Moden bis 1000 Hz. Hohlraum 20,61 l.
Verglichen: die ersten fuenf Moden wie gemessen gegen 139 / 203 / 280 / 362 / 385 Hz.
Frei: Deckendicke mm, Steghoehe mm, Stegbreite mm, Muendung, Rosette gross g (kleine je 1 g)."""
import math
import sys

import numpy as np
from scipy.linalg import eigh
from scipy.optimize import minimize

import oud_kombi as k
import oud_suche as s

ZIELE = np.array([139.0, 203.0, 280.0, 362.0, 385.0])
START = np.array([2.06, 6.4, 20.0, 1.52, 10.0])
GRENZEN = [(1.4, 2.4), (4.0, 16.0), (10.0, 32.0), (1.2, 2.2), (2.0, 14.0)]
VOLUMEN = 0.02061
n = 0
bestes = (1e9, None)


def rosetten(p, gross_g, klein_g=1.0):
    for name, x, y, r, m in (("rosette", 0.0, 0.30, 0.053, gross_g / 1000), ("rosette left", -0.103, 0.204, 0.020, klein_g / 1000),
                             ("rosette right", 0.103, 0.204, 0.020, klein_g / 1000)):
        rr, breite, hoehe, ecken = r + 0.003, 0.005, 0.010, 48
        p["materials"].append({"name": name, "kind": "orthotropic", "density": m / (2 * math.pi * rr * breite * hoehe), "ex": 1e7, "ey": 1e7,
                               "gxy": 4e6, "gxz": 4e6, "gyz": 4e6, "nuxy": 0.3, "lossFactor": 0.0, "source": "probe"})
        linie = [[round(x + rr * math.cos(2 * math.pi * i / ecken), 6), round(y + rr * math.sin(2 * math.pi * i / ecken), 6)] for i in range(ecken + 1)]
        p["ribs"].append({"name": name, "material": name, "line": linie, "height": [hoehe] * (ecken + 1), "width": [breite] * (ecken + 1)})
    return p


def plan(x):
    return rosetten(k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, x[2] / 1000, x[1] / 1000, 650.0, 10e9), volumen=VOLUMEN, muendung=x[3]), x[4])


def wie_gemessen(e):
    w, hz = e["_w"], e["alle_hz"]
    v = w[:, e["_antwort"]]
    return np.sqrt(eigh(np.diag((2 * np.pi * hz) ** 2), np.eye(len(hz)) + 0.0058 * np.outer(v, v), eigvals_only=True)) / (2 * np.pi)


def lagen(e):
    return [100 * (a / z - 1) for a, z in zip(wie_gemessen(e)[:5], ZIELE)]


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= v <= b) for v, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), "fein6", to=1000, netz=2500, zeigen=False)
    if e is None:
        return 1e6
    lag = lagen(e)
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, np.array(x, dtype=float))
        print(f"{n:4d} Kosten {c:7.2f}  " + " ".join(f"{v:.3f}" for v in x) + "  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


if __name__ == "__main__":
    minimize(kosten, START, method="Nelder-Mead", options={"maxfev": 260, "xatol": 0.005, "fatol": 0.05,
                                                           "initial_simplex": START + np.vstack([np.zeros(5), np.diag([0.1, 1.5, 3.0, 0.12, 2.0])])})
    x = bestes[1]
    print(f"Ende nach {n} Auswertungen: Decke {x[0]:.3f} mm, Steg 17 cm x {x[2]:.2f} x {x[1]:.2f} mm ({0.17 * x[2] * x[1] * 0.65:.1f} g), Muendung {x[3]:.3f}, Rosette gross {x[4]:.2f} g")
    e = s.rechne(plan(x), "fein6_bestes", to=1000, netz=2500)
    print("   Lagen wie gemessen gegen 139 / 203 / 280 / 362 / 385: " + " / ".join(f"{v:+.1f} %" for v in lagen(e)))
    print("   Instrument: " + " / ".join(f"{v:.1f}" for v in e["alle_hz"][:6]))
    print("   wie gemessen (mit Aufnehmer): " + " / ".join(f"{v:.1f}" for v in wie_gemessen(e)[:6]))
