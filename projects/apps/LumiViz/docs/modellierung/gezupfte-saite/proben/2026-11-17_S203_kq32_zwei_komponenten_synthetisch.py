"""S203 KQ32: zwei Komponenten mit Boden an die Schroeder-Kurve — Verfahren an exakten Huellkurven pruefen
(ohne Spektrum), Vergleich mit den C++-Faellen. Aufruf: python -B zweikomp.py"""
import numpy as np
from scipy.optimize import least_squares


def huelle(t60f, t60s, abstand_db, dauer, schritt=0.01, boden=1e-12, schwebung_hz=None):
    t = np.arange(0, dauer, schritt)
    s1, s2 = -60 / t60f, -60 / t60s
    a2 = 10 ** (abstand_db / 10)
    e = 10 ** (s1 * t / 10) + a2 * 10 ** (s2 * t / 10)
    if schwebung_hz:  # Mischterm zweier Moden im selben Band
        e = e + 2 * np.sqrt(10 ** (s1 * t / 10) * a2 * 10 ** (s2 * t / 10)) * np.cos(2 * np.pi * schwebung_hz * t)
    return t, np.maximum(e, 0) + boden


def anpassen(t, e, m=None, boden=True):
    m = m or len(t)
    E = np.cumsum(e[::-1])[::-1]
    Ed = 10 * np.log10(E[:m])

    def modell(p):
        a1, s1, a2, s2, lb = p
        x = np.exp(a1) * 10 ** (s1 * t / 10) + np.exp(a2) * 10 ** (s2 * t / 10) + (np.exp(lb) if boden else 0.0)
        S = np.cumsum(x[::-1])[::-1]
        return 10 * np.log10(np.maximum(S[:m], 1e-300)) - Ed

    p0 = [0.0, -30.0, np.log(0.01), -4.0, np.log(1e-9)]
    r = least_squares(modell, p0, method="lm", x_scale=[1, 10, 1, 1, 1])
    a1, s1, a2, s2, lb = r.x
    return -60 / s1, -60 / s2


for name, args in [("flach 40/4, -20 dB", (1.5, 15, -20, 8)), ("Harfe 150/20, -20 dB", (0.4, 3, -20, 4)),
                   ("tief 3 s / 12 s, -30 dB", (3, 12, -30, 8))]:
    t, e = huelle(*args)
    print(name, "ohne Schwebung:", ["%.3f" % v for v in anpassen(t, e)])
    t, e = huelle(*args, schwebung_hz=10.0)
    print(name, "Schwebung 10 Hz:", ["%.3f" % v for v in anpassen(t, e)])
