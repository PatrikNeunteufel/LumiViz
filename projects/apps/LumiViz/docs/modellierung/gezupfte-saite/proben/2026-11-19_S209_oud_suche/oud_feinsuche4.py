"""S209 - Feinsuche Familie R nach KQ42 a, KQ43 a, KQ44 a: Netz fuer 2500 Hz, Moden bis 1000 Hz als Basis,
Rosetten (7 g am grossen Loch, je 1 g an den kleinen) und Aufnehmer (5,8 g am Antwortpunkt 3) als Massenaenderung
im Modenraum (genau, nicht erste Ordnung). Verglichen: die ersten fuenf Moden des Instruments mit Rosetten und
Aufnehmer, der Reihe nach, gegen die fuenf Spitzen der Messung 139 / 203 / 280 / 362 / 385 Hz.
Frei: Deckendicke, Steghoehe, Stegbreite, Hohlraum, Muendung. Balken nach Tab. 2, Enden 2 mm vor dem Rand.
Aufruf: python -X utf8 -B oud_feinsuche4.py [<start: fuenf Zahlen>]
"""
import sys

import numpy as np
from scipy.linalg import eigh
from scipy.optimize import minimize

import oud_kombi as k
import oud_suche as s

ZIELE = np.array([139.0, 203.0, 280.0, 362.0, 385.0])
START = np.array([float(v) for v in sys.argv[1:6]]) if len(sys.argv) >= 6 else np.array([1.85, 8.0, 20.0, 21.5, 1.4])
GRENZEN = [(1.4, 2.4), (4.0, 16.0), (10.0, 32.0), (16.0, 26.0), (1.2, 2.2)]
RG, RK, AUFNEHMER = 0.007, 0.002, 0.0058
n = 0
bestes = (1e9, None)


def plan(x):
    return k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, x[2] / 1000, x[1] / 1000, 650.0, 10e9), volumen=x[3] / 1000, muendung=x[4])


def mit_massen(e, rosetten=True, aufnehmer=True):
    w, hz = e["_w"], e["alle_hz"]
    M = np.eye(len(hz))
    paare = []
    if rosetten:
        paare += [(i, RG / len(e["_gross"])) for i in e["_gross"]] + [(i, RK / len(e["_klein"])) for i in e["_klein"]]
    if aufnehmer:
        paare.append((e["_antwort"], AUFNEHMER))
    for i, m in paare:
        M += m * np.outer(w[:, i], w[:, i])
    return np.sqrt(eigh(np.diag((2 * np.pi * hz) ** 2), M, eigvals_only=True)) / (2 * np.pi)


def lagen(e):
    f = mit_massen(e)[:5]
    return [100 * (a / z - 1) for a, z in zip(f, ZIELE)]


def kosten(x):
    global n, bestes
    n += 1
    if any(not (a <= v <= b) for v, (a, b) in zip(x, GRENZEN)):
        return 1e6
    e = s.rechne(plan(x), "fein4", to=1000, netz=2500, zeigen=False)
    if e is None:
        return 1e6
    lag = lagen(e)
    c = float(sum(v * v for v in lag))
    if c < bestes[0]:
        bestes = (c, np.array(x, dtype=float))
        print(f"{n:4d} Kosten {c:7.2f}  " + " ".join(f"{v:.3f}" for v in x) + "  Lagen " + " ".join(f"{v:+.1f}" for v in lag), flush=True)
    return c


if __name__ == "__main__":
    r = minimize(kosten, START, method="Nelder-Mead", options={"maxfev": 260, "xatol": 0.005, "fatol": 0.05,
                                                               "initial_simplex": START + np.vstack([np.zeros(5), np.diag([0.12, 2.0, 4.0, 1.5, 0.15])])})
    x = bestes[1]
    print(f"Ende nach {n} Auswertungen: Decke {x[0]:.3f} mm, Steg 17 cm x {x[2]:.2f} x {x[1]:.2f} mm ({0.17 * x[2] * x[1] * 0.65:.1f} g), Hohlraum {x[3]:.2f} l, Muendung {x[4]:.3f}")
    e = s.rechne(plan(x), "fein4_bestes", to=1000, netz=2500)
    print("   Lagen (Rosetten, Aufnehmer) gegen 139 / 203 / 280 / 362 / 385: " + " / ".join(f"{v:+.1f} %" for v in lagen(e)))
    for was, f in (("Bauplan (Lochraender ohne Masse)", e["alle_hz"]), ("Instrument (mit Rosetten)", mit_massen(e, True, False)), ("wie gemessen (Rosetten, Aufnehmer)", mit_massen(e))):
        print(f"   {was}: " + " / ".join(f"{v:.1f}" for v in f[:6]))
