"""Lücke 1 — zwei Gegenrechnungen.

(A) Zwei-Oszillator-Modell Christensen & Vistisen nach Wright 1996 Gl. 4.9-4.17 (am Seitenbild geprüft) mit den
    Werten von Wright: Decke BR2 Tab. 5.4 (T(1,1) 165 Hz, 0,062 kg, Q 36, A 0,059 m^2), Helmholtz S. 138 (133 Hz,
    Q 30, A_h 5,7e-3 m^2, V 14e-3 m^3).  Druck je Kraft in 1 m vorn (Monopol, Gl. 4.17).
    Konvention: x nach außen positiv, Delta P = -mu (A_t x_t + A_h x_h), mu = rho c^2 / V.
(B) Kolben im Schallwand, Rayleigh-Integral, gleichförmige Schnelle: Druck an den Mores-Orten (10 cm über der Decke,
    auf der Achse und 10 cm seitlich) gegen den Druck in 1 m vorn (Vollraum-Monopol rho*i*w*U/(4 pi r) und Halbraum),
    je Terzband 80 Hz - 5 kHz.  Prüft Hess 2013 Gl. (2) p_F = a/(4r) p_N (Grenzfall Nahabstand -> 0).
"""
import math

import numpy as np

rho, c = 1.2, 343.0

# ---------------------------------------------------------------- (A)
mt, ft, Qt, At = 0.062, 165.0, 36.0, 0.059
fh, Qh, Ah, V = 133.0, 30.0, 5.7e-3, 14e-3
mu = rho * c * c / V
mh = mu * Ah * Ah / (2 * math.pi * fh) ** 2
print(f"(A) mu = {mu:.4g} Pa/m^3, m_h = {mh*1e3:.3f} g, l_eff = {mh/(rho*Ah)*100:.1f} cm")
f = np.linspace(60, 400, 34001)
w = 2 * np.pi * f
Dt = mt * ((2 * np.pi * ft) ** 2 - w ** 2 + 1j * w * (2 * np.pi * ft / Qt))
Dh = mh * ((2 * np.pi * fh) ** 2 - w ** 2 + 1j * w * (2 * np.pi * fh / Qh))
al = mu * Ah * At
N = Dh * Dt - al ** 2
xt, xh = Dh / N, -al / N                     # je 1 N
Y = 1j * w * xt
U = 1j * w * (At * xt + Ah * xh)
p1 = 1j * w * rho * U / (4 * np.pi * 1.0)
Ut = 1j * w * At * xt
p1_ohne_loch = 1j * w * rho * Ut / (4 * np.pi * 1.0)
from scipy.signal import find_peaks
i, _ = find_peaks(np.abs(Y))
for k in i:
    print(f"    Spitze der Admittanz {f[k]:6.1f} Hz: |Y| = {20*np.log10(abs(Y[k])):6.1f} dB re 1 m/s/N, "
          f"|p/F| 1 m = {20*np.log10(abs(p1[k])):5.1f} dB re 1 Pa/N (nur Decke {20*np.log10(abs(p1_ohne_loch[k])):5.1f}), "
          f"Volumenanteil Loch/Decke = {abs(Ah*xh[k])/abs(At*xt[k]):.2f}, Phase {math.degrees(np.angle(Ah*xh[k]/(At*xt[k]))):+.0f} Grad")
for fb in (80, 100, 125, 160, 200, 250, 315):
    b = (f >= fb / 2 ** (1 / 6)) & (f < fb * 2 ** (1 / 6))
    print(f"    Terz {fb:4d} Hz: p/F in 1 m (Leistungsmittel) = {10*np.log10(np.mean(np.abs(p1[b])**2)):6.1f} dB re 1 Pa/N")

# ---------------------------------------------------------------- (B)
def rayleigh(a, x0, z0, k, n=500):
    """p / (i w rho u) an (x0, 0, z0) über einer Kreisscheibe Radius a im Schallwand: (1/2pi) Int e^{-ikR}/R dS."""
    r = (np.arange(n) + 0.5) * a / n
    ph = (np.arange(2 * n) + 0.5) * np.pi / n
    R, P = np.meshgrid(r, ph, indexing="ij")
    X, Yy = R * np.cos(P), R * np.sin(P)
    D = np.sqrt((X - x0) ** 2 + Yy ** 2 + z0 ** 2)
    dS = R * (a / n) * (np.pi / n)
    return np.sum(np.exp(-1j * k * D) / D * dS) / (2 * np.pi)


def fern(a, k, r=1.0, halbraum=False, theta=0.0):
    """p / (i w rho u) in r: Vollraum U/(4 pi r) bzw. Halbraum mit Kolbenrichtwirkung (Theta gegen die Achse)."""
    U = np.pi * a * a
    if not halbraum:
        return U / (4 * np.pi * r)
    x = k * a * math.sin(theta)
    d = 1.0 if x < 1e-9 else 2 * __import__("scipy.special", fromlist=["j1"]).j1(x) / x
    return U / (2 * np.pi * r) * d


print()
print("(B) Kolben: Nahabstand -> 0 (Hess) gegen 10 cm; Verhältnis Fern/Nah in dB")
for a in (0.109, 0.137):
    k = 2 * np.pi * 50 / c
    near0 = rayleigh(a, 0.0, 1e-4, k)
    print(f"  a = {a} m, 50 Hz: Nah auf Achse z->0: {20*np.log10(abs(fern(a,k)/near0)):+.1f} dB "
          f"(Hess a/(4r) = {20*np.log10(a/4):+.1f} dB)")
    print("   Terz Hz |  ka  | Achse 10 cm: Voll  Halb | seitl. 10 cm: Voll  Halb")
    for fb in (80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000):
        werte = []
        for x0 in (0.0, 0.10):
            pv, ph_ = [], []
            for ff in fb * 2 ** (np.linspace(-1, 1, 7) / 6):
                kk = 2 * np.pi * ff / c
                nah = abs(rayleigh(a, x0, 0.10, kk, n=160 if ff > 1500 else 100))
                pv.append((fern(a, kk) / nah) ** 2)
                ph_.append((fern(a, kk, halbraum=True) / nah) ** 2)
            werte += [10 * np.log10(np.mean(pv)), 10 * np.log10(np.mean(ph_))]
        print(f"   {fb:6d} | {2*np.pi*fb/c*a:4.2f} |  {werte[0]:+6.1f} {werte[1]:+6.1f}    |  {werte[2]:+6.1f} {werte[3]:+6.1f}")
