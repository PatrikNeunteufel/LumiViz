"""Lücke 2: zwei Polarisationen einer Saite am 2x2-Steg — Störungsrechnung gegen exakte Wurzel.

Saite 0 <= x <= L, Steg bei x = 0 (Admittanzmatrix Y, 2x2, Achsen z senkrecht, y parallel zur Decke), Sattel x = L
starr. w(x) = a sin(k (L - x)), Kraft der Saite auf den Steg F = T w'(0), Stegschnelle i w w(0) = Y F
  => [tan(kL) I - i Z0 Y(w)] a = 0,  Z0 = sqrt(T m).                                        (hergeleitet)
Schwache Kopplung: kL = n pi + i Z0 lambda,  lambda Eigenwert von Y(w_n)
  => w_+- = w_n (1 + i Z0 lambda_+- / (n pi)),  eta_b,+- = 2 Z0 Re(lambda_+-) / (n pi)  (1-D: Woodhouse 2021 Gl. 1)
     Frequenzaufspaltung  df = f_n Z0 (Im(lambda_-) - Im(lambda_+)) / (n pi).
Körper: g34 (classical_guitar.lvinstbody, nur gelesen), Moden mit Richtung tan(theta_n) — ANNAHME wie im Prüfstand
(richtung_tan, Samen 7). Saiten: Woodhouse u. a. 2012 Tab. I wie im Prüfstand (GITARRE).
"""
import json
import math
import pathlib

import numpy as np

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\classical_guitar.lvinstbody")
k = json.loads(P.read_text(encoding="utf-8"))
fn = np.array([m["f"] for m in k["modes"]])
eta = np.array([m["eta"] for m in k["modes"]])
phi = np.array([m["bridge"] for m in k["modes"]])
wn = 2 * np.pi * fn

M64 = (1 << 64) - 1


class Zufall:
    def __init__(self, s):
        self.z = s

    def gleich(self):
        self.z = (self.z + 0x9E3779B97F4A7C15) & M64
        z = self.z
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & M64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & M64
        return (((z ^ (z >> 31)) >> 11) + 0.5) / 9007199254740992.0


def richtung_tan(f, samen=7):
    z = Zufall(samen)
    b = np.clip(0.03 * (f / 200.0) ** 1.27, 0.03, 0.4)
    return b * np.array([1.0 if z.gleich() < 0.5 else -1.0 for _ in f])


TAN = richtung_tan(fn)


def Ymat(w, s, tan=TAN):
    """2x2 Admittanz am Stegpunkt der Saite s (Index 0 = Saite 6), w komplex erlaubt."""
    c = 1j * w / (wn ** 2 - w ** 2 + 1j * eta * wn * w) * phi[:, s] ** 2
    return np.array([[np.sum(c), np.sum(c * tan)], [np.sum(c * tan), np.sum(c * tan * tan)]])


GITARRE = [(71.6, 6.24e-3, 1.2), (73.9, 3.61e-3, 0.9), (71.2, 1.95e-3, 1.2), (58.3, 0.90e-3, 1.7),
           (53.4, 0.52e-3, 1.2), (70.3, 0.38e-3, 1.5)]       # T, m, eta_A (Woodhouse 2012 Tab. I, Satz a)
NAMEN = ["6 E2", "5 A2", "4 D3", "3 G3", "2 B3", "1 E4"]
L = 0.650


def exakt(s, n, w0, tan=TAN):
    T, m, _ = GITARRE[s]
    Z0, cs = math.sqrt(T * m), math.sqrt(T / m)

    def det(w):
        t = np.tan(w / cs * L)
        A = t * np.eye(2) - 1j * Z0 * Ymat(w, s, tan)
        return A[0, 0] * A[1, 1] - A[0, 1] * A[1, 0]
    w = w0
    for _ in range(60):
        h = 1e-6 * abs(w)
        d = (det(w + h) - det(w - h)) / (2 * h)
        w_neu = w - det(w) / d
        if abs(w_neu - w) < 1e-10 * abs(w):
            return w_neu
        w = w_neu
    return w


print("Saite n | f_n Hz | |Yzz| dB | |Yzy/Yzz| dB | |Yyy/Yzz| dB | eta_b+ eta_b- (Stoerung) | exakt | eta_air | T60+ T60- s | df Hz")
for s in range(6):
    T, m, etaA = GITARRE[s]
    Z0, cs = math.sqrt(T * m), math.sqrt(T / m)
    for n in (1, 2):
        w_n = n * math.pi * cs / L
        Y = Ymat(w_n, s)
        lam, vec = np.linalg.eig(Y)
        o = np.argsort(-lam.real)
        lam = lam[o]
        wpm = w_n * (1 + 1j * Z0 * lam / (n * math.pi))
        eb = 2 * Z0 * lam.real / (n * math.pi)
        ex = [exakt(s, n, w0) for w0 in wpm]
        eb_ex = [2 * w.imag / w.real for w in ex]
        e_air = etaA / w_n            # Valette-Term eta_A / w (Woodhouse 2012 Gl. 4, ohne eta_F und Biegung)
        t60 = [13.8 / ((e + e_air) * w_n) for e in eb]
        df = (wpm[0].real - wpm[1].real) / (2 * math.pi)
        print(f"{NAMEN[s]} {n} | {w_n/2/math.pi:6.1f} | {20*math.log10(abs(Y[0,0])):6.1f} | {20*math.log10(abs(Y[0,1]/Y[0,0])):6.1f} |"
              f" {20*math.log10(abs(Y[1,1]/Y[0,0])):6.1f} | {eb[0]:.2e} {eb[1]:.2e} | {eb_ex[0]:.2e} {eb_ex[1]:.2e} |"
              f" {e_air:.2e} | {t60[0]:5.2f} {t60[1]:5.2f} | {df:+.3f}")

print()
print("Kontrolle 1-D (nur Yzz) gegen Woodhouse 2021 Gl. (1) eta_b = 2 Re(Y) / (pi n Y0), Saite 6, n = 1:")
T, m, _ = GITARRE[0]
w1 = math.pi * math.sqrt(T / m) / L
Y = Ymat(w1, 0)[0, 0]
print(f"  Re(Yzz) = {Y.real:.3e} m/s/N, Y0 = {1/math.sqrt(T*m):.3f} -> eta_b = {2*Y.real*math.sqrt(T*m)/math.pi:.3e}")
