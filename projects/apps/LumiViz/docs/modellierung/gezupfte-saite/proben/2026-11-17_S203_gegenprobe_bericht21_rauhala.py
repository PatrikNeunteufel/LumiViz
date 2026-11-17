"""Gegenprobe S203 zu Bericht 21, Abschnitt 3: Rauhala & Valimaki DAFx-06 Gl. (3)-(8), Abb. 2-5, adf-Teiltongesetz.
Eigene Umsetzung, ohne Code des Laufs."""
import numpy as np
from scipy.optimize import brentq

k1, k2, k3 = -0.00179, -0.0233, -2.93
m1, m2, m3, m4 = 0.0126, 0.0606, -0.00825, 1.97


def ikey(f0):
    return 12 * np.log2(f0 * 2 ** (1 / 12) / 27.5)


def kd(B):
    return np.exp(k1 * np.log(B) ** 2 + k2 * np.log(B) + k3)


def cd(B, M, gedruckt):
    lM, lB = np.log(M), np.log(B)
    x = lM if gedruckt else lB
    return np.exp((m1 * lM + m2) * x + m3 * lM + m4)


def a1(f0, B, M, gedruckt=False):
    D = np.exp(cd(B, M, gedruckt) - ikey(f0) * kd(B))
    return (1 - D) / (D + 1)


print("Abb. 2: C1(M) = m1 ln M + m2, C2(M) = m3 ln M + m4")
for M in (1, 8):
    print(f"  M={M}: C1={m1*np.log(M)+m2:.4f}  C2={m3*np.log(M)+m4:.4f}")

print("\nAbb. 3 (f0 65.406, Taste 16) und Abb. 4 (M 8): a1 gedruckt / mit ln B")
for f0, M in [(65.406, 1), (65.406, 5), (65.406, 10), (65.406, 20), (65.406, 40)]:
    z = [f"{a1(f0,B,M,True):+.3f}/{a1(f0,B,M):+.3f}" for B in (1e-6, 1e-5, 1e-4, 1e-3)]
    print(f"  Abb3 M={M:2d} B=1e-6..1e-3: " + "  ".join(z))
for key in (4, 16, 28, 40):
    f0 = 27.5 * 2 ** ((key - 1) / 12)
    z = [f"{a1(f0,B,8,True):+.3f}/{a1(f0,B,8):+.3f}" for B in (1e-6, 1e-5, 1e-4, 1e-3)]
    print(f"  Abb4 key={key:2d} (Ikey {ikey(f0):.2f}): " + "  ".join(z))


def lag_ap(w, a):
    # Phasenverzug (rad) des Allpasses (a + z^-1)/(1 + a z^-1)
    return w + 2 * np.arctan2(a * np.sin(w), 1 + a * np.cos(w)) * 1.0 - 2 * np.arctan2(a * np.sin(w), 1 + a * np.cos(w)) * 0 \
        if False else -np.angle((a + np.exp(-1j * w)) / (1 + a * np.exp(-1j * w)))


def abb5(f0, B, M, fs, nmax):
    a = a1(f0, B, M)
    w0 = 2 * np.pi * f0 / fs
    phi_ap0 = M * np.unwrap([lag_ap(x, a) for x in np.linspace(1e-9, w0, 200)])[-1]
    L = (2 * np.pi - phi_ap0) / w0  # reiner Verzug in Takten, am Grundton gestimmt
    ws = np.linspace(1e-9, np.pi * 0.999, 200001)
    lag = L * ws + M * np.unwrap(lag_ap(ws, a))
    out = []
    for n in range(1, nmax + 1):
        if lag[-1] < 2 * np.pi * n:
            break
        fn = np.interp(2 * np.pi * n, lag, ws) * fs / (2 * np.pi)
        ziel = n * f0 * np.sqrt(1 + B * n * n)
        out.append((ziel, 1200 * np.log2(fn / ziel)))
    return out


print("\nAbb. 5: groesste positive Abweichung (Cent) und ihre Frequenz, je fs")
for name, f0, B in [("a", 32.703, 0.0002), ("b", 65.406, 0.0001), ("c", 130.82, 0.00015)]:
    for M in (1, 10, 40):
        z = []
        for fs in (44100, 48000):
            r = abb5(f0, B, M, fs, 60)
            c = np.array([x[1] for x in r]); f = np.array([x[0] for x in r])
            i = int(np.argmax(c))
            z.append(f"fs {fs}: max {c[i]:+5.1f} c bei {f[i]:6.0f} Hz")
        print(f"  ({name}) M={M:2d}: " + " | ".join(z))

print("\nadf.m Z. 38: pd(f) = tau0/sqrt(1+B (f/f0)^2), f pd(f)/fs = n  ->  f_n = n f0 / sqrt(1 - B n^2)?")
f0, B = 32.702, 2e-4
tau0 = 44100 / f0
for n in (12, 47):
    fn = brentq(lambda f: f * (tau0 / np.sqrt(1 + B * (f / f0) ** 2)) / 44100 - n, 1, 20000)
    geschl = n * f0 / np.sqrt(1 - B * n * n)
    r1 = n * f0 * np.sqrt(1 + B * n * n)
    print(f"  n={n}: numerisch {fn:.3f} Hz, geschlossen {geschl:.3f} Hz, R1 {r1:.3f} Hz, Abstand {1200*np.log2(fn/r1):.2f} c")
