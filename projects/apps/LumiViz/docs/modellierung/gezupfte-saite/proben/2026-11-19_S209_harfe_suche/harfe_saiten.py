"""S209 - Harfe: die Saiten als Federn an den Oesen, im Modenraum auf einen gerechneten Koerper gesetzt
(K = diag(w^2) + sum k_s phi phi^T; die Formen sind massennormiert samt Luft).
Feder je Saite: k = c * E A / L (Laengssteife der Saite; c = Quadrat des Richtungsanteils senkrecht zum Boden mal
Nachgiebigkeit von Hals und Rahmen). E: Nylon 4 GPa, Darm 5,5 GPa, Stahlkern 200 GPa (Annahmen); Durchmesser und
Laengen Bell 1988 Tab. 2.1 / 2.2 (am Bild gelesen, S198).
Aufruf: python -X utf8 -B harfe_saiten.py <koerper aus harfe/> [c ...]"""
import json
import math
import sys

import numpy as np
from scipy.linalg import eigh

import harfe_suche as s

MONO = [(2960, 0.52, 0.071), (2637, 0.52, 0.082), (2489, 0.56, 0.088), (2218, 0.56, 0.095), (1976, 0.61, 0.105),
        (1864, 0.62, 0.113), (1661, 0.65, 0.123), (1480, 0.69, 0.132), (1318, 0.70, 0.143), (1244, 0.69, 0.154),
        (1109, 0.70, 0.164), (988, 0.73, 0.177), (932, 0.76, 0.192), (830, 0.80, 0.208), (740, 0.85, 0.225),
        (659, 0.90, 0.242), (622, 0.95, 0.265), (554, 1.00, 0.285), (494, 1.06, 0.327), (466, 1.12, 0.354),
        (415, 1.18, 0.385), (370, 1.24, 0.421), (330, 1.30, 0.455), (311, 1.36, 0.498), (277, 1.44, 0.542),
        (247, 1.52, 0.591), (233, 1.60, 0.644), (207, 1.68, 0.699), (185, 1.76, 0.753), (165, 1.84, 0.813),
        (155, 1.92, 0.880), (139, 2.00, 0.945), (123, 2.10, 0.999), (116, 2.20, 1.053), (104, 2.35, 1.103)]
METALL = [(92, 0.80, 1.45, 1.150), (82, 0.80, 1.55, 1.180), (77, 0.80, 1.65, 1.222), (69, 0.85, 1.75, 1.257),
          (61, 0.90, 1.92, 1.299), (58, 0.95, 2.05, 1.330), (52, 1.00, 2.15, 1.359), (46, 1.10, 2.30, 1.397),
          (41, 1.10, 2.45, 1.421), (39, 1.25, 2.60, 1.445), (35, 1.30, 2.75, 1.475)]
E_NYLON, E_DARM, E_STAHL = 4.0e9, 5.5e9, 200.0e9


def federn():
    """[(Taste der Saite mit Pedal in der Mitte, E A / L in N/m)] fuer die 46 Saiten."""
    aus = []
    for i, (hz, d, L) in enumerate(MONO):
        e = E_NYLON if i < 10 else E_DARM
        aus.append((int(round(69 + 12 * math.log2(hz / 440.0))) + 1, e * math.pi * (d / 2000) ** 2 / L))
    for hz, kern, _, L in METALL:
        aus.append((int(round(69 + 12 * math.log2(hz / 440.0))) + 1, E_STAHL * math.pi * (kern / 2000) ** 2 / L))
    return aus


def besaitet(k, c):
    """Koerper (dict wie die Datei) mit den Saiten als Federn; gibt einen Koerper derselben Form zurueck."""
    m = k["modes"]
    f = np.array([x["f"] for x in m])
    eta = np.array([x["eta"] for x in m])
    luft = np.array([x["airShare"] for x in m])
    steg = np.array([x["bridge"] for x in m])
    G = np.array([[complex(*g) for g in x["radiation"]] for x in m])
    w2 = (2 * np.pi * f) ** 2
    K = np.diag(w2)
    for taste, ks in federn():
        v = steg[:, k["bridgePoints"].index(taste)]
        K = K + c * ks * np.outer(v, v)
    lam, q = eigh(K)
    f2 = np.sqrt(lam) / (2 * np.pi)
    eta2 = (q ** 2 * (eta * w2)[:, None]).sum(axis=0) / lam
    luft2 = (q ** 2 * luft[:, None]).sum(axis=0)
    steg2, G2 = q.T @ steg, q.T @ G
    neu = dict(k)
    neu["modes"] = [{"f": float(f2[i]), "eta": float(eta2[i]), "airShare": float(luft2[i]), "bridge": [float(v) for v in steg2[i]],
                     "radiation": [[float(g.real), float(g.imag)] for g in G2[i]]} for i in range(len(f2))]
    return neu


if __name__ == "__main__":
    name = sys.argv[1]
    k = json.loads((s.AUS / f"{name}.lvinstbody").read_text(encoding="utf-8"))
    plan = json.loads((s.AUS / f"{name}.lvinstbodyplan").read_text(encoding="utf-8"))
    fe = federn()
    print(f"Summe E A / L: Nylon {sum(v for _, v in fe[:10]):.3g}, Darm {sum(v for _, v in fe[10:35]):.3g}, Metall {sum(v for _, v in fe[35:]):.3g} N/m")
    print("Bell, Kasten G in der Harfe ohne Saiten -> besaitet: S(1,1)- 170 -> 187, S(1,1)+ 216 -> 207, S(1,2) 249 -> 257, "
          "S(1,3) 316 -> 334, S(1,4) 432 -> 430, S(1,5) 545 -> 555, S(1,6) 709 -> 697")
    for c in [float(v) for v in sys.argv[2:]] or [0.0, 0.1, 0.2, 0.33]:
        s.messe(besaitet(k, c), plan, f"{name}, Saiten c = {c}")
