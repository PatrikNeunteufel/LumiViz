"""Lücke 1 (C): Zwei-Oszillator-Modell (Wright 1996 Gl. 4.9-4.17, Werte BR2 Tab. 5.4 + S. 138) mit zwei Kolben —
Decke (Kreis, Fläche A_t, Mitte am Steg) und Schallloch (Kreis, Fläche A_h, Mitte 0,17 m vom Steg Richtung Hals,
ANNAHME). Nahfeld an den Mores-Orten (10 cm über der Decke; bass/treble 10 cm seitlich vom Stegmittelpunkt) mit dem
Rayleigh-Integral (Schallwand, nahe der Decke vertretbar), Fernfeld 1 m vorn als Vollraum-Monopol (Wright Gl. 4.17).
Vergleich mit der gemessenen Spitze an g34 (Körperdatei, r1_koerper.py): T1 200 Hz |p/F| 18,5 / 19,9 dB re 1 Pa/N.
"""
import math

import numpy as np

rho, c = 1.2, 343.0
mt, ft, Qt, At = 0.062, 165.0, 36.0, 0.059
fh, Qh, Ah, V = 133.0, 30.0, 5.7e-3, 14e-3
mu = rho * c * c / V
mh = mu * Ah * Ah / (2 * math.pi * fh) ** 2
at, ah = math.sqrt(At / math.pi), math.sqrt(Ah / math.pi)
LOCH = 0.17


def kolben(a, mitte, punkt, k, n=120):
    r = (np.arange(n) + 0.5) * a / n
    ph = (np.arange(2 * n) + 0.5) * np.pi / n
    R, P = np.meshgrid(r, ph, indexing="ij")
    X, Yy = mitte[0] + R * np.cos(P), mitte[1] + R * np.sin(P)
    D = np.sqrt((X - punkt[0]) ** 2 + (Yy - punkt[1]) ** 2 + punkt[2] ** 2)
    return np.sum(np.exp(-1j * k * D) / D * R * (a / n) * (np.pi / n)) / (2 * np.pi)   # p/(i w rho u), Schallwand


ORTE = {"bass": (0.0, -0.10, 0.10), "treble": (0.0, 0.10, 0.10), "upper east (Annahme)": (0.20, 0.10, 0.10)}
print(f"a_t = {at:.3f} m, a_h = {ah:.3f} m, Loch {LOCH} m vom Steg (Annahme)")
print(" f/Hz  | |p/F| 1 m | " + " | ".join(f"{o} nah  fern-nah" for o in ORTE))
for f in (76.3, 100.0, 125.0, 160.0, 197.7, 250.0, 315.0):
    w = 2 * math.pi * f
    k = w / c
    Dt = mt * ((2 * math.pi * ft) ** 2 - w * w + 1j * w * 2 * math.pi * ft / Qt)
    Dh = mh * ((2 * math.pi * fh) ** 2 - w * w + 1j * w * 2 * math.pi * fh / Qh)
    al = mu * Ah * At
    N = Dh * Dt - al * al
    ut, uh = 1j * w * Dh / N, 1j * w * (-al) / N            # Schnellen je 1 N
    fern = 1j * w * rho * (At * ut + Ah * uh) / (4 * math.pi)
    zeile = f"{f:6.1f} | {20*math.log10(abs(fern)):6.1f} dB | "
    for o, pkt in ORTE.items():
        nah = 1j * w * rho * (ut * kolben(at, (0, 0), pkt, k) + uh * kolben(ah, (LOCH, 0), pkt, k))
        zeile += f"{20*math.log10(abs(nah)):6.1f} {20*math.log10(abs(fern/nah)):+6.1f} | "
    print(zeile)
