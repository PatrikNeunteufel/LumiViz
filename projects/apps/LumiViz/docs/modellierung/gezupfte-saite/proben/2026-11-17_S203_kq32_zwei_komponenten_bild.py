"""S203 KQ32: C++-Huellkurve (Messfall LUMIVIZ_KQ32_AUS) im Messbereich — Geraden und zwei Komponenten."""
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import least_squares

datei, von, bis, aus = sys.argv[1], float(sys.argv[2]), float(sys.argv[3]), sys.argv[4]
d = np.loadtxt(datei)
t, p = d[:, 0], d[:, 1]
m = (t >= von - 1e-9) & (t <= bis + 1e-9)
t, p = t[m] - t[m][0], p[m]
e = 10 ** (p / 10)
E = np.cumsum(e[::-1])[::-1]
L = 10 * np.log10(E / E[0])
n = len(t)


def gerade(a, b):
    k, c = np.polyfit(t[a:b], L[a:b], 1)
    return k, c, np.sum((L[a:b] - (k * t[a:b] + c)) ** 2)


best = (np.inf,)
for j in range(max(10, n // 20), n - max(10, n // 5)):
    g1, g2 = gerade(0, j), gerade(j, n)
    if g1[2] + g2[2] < best[0]:
        best = (g1[2] + g2[2], j, g1, g2)
_, kn, g1, g2 = best
Ed = 10 * np.log10(E)


def rest(q, ab=0):
    a1, s1, a2, s2, lb = q
    x = np.exp(a1) * 10 ** (s1 * t / 10) + np.exp(a2) * 10 ** (s2 * t / 10) + np.exp(lb)
    return (10 * np.log10(np.cumsum(x[::-1])[::-1]) - Ed)[ab:]


q0 = [np.log(e[0]), g1[0], np.log(e[0]) - 2.3, g2[0], np.log(e[-1] * 0.1)]
r = least_squares(rest, q0, method="lm")
ab = max(10, n // 20)
r2 = least_squares(lambda q: rest(q, ab), q0, method="lm")
for name, q in (("ab 0", r.x), (f"ab Rahmen {ab}", r2.x)):
    a1, s1, a2, s2, lb = q
    print(f"Komponenten {name}: frueh {-60 / s1:.2f} s, spaet {-60 / s2:.2f} s, Knick {10 * (a1 - a2) / np.log(10) / (s2 - s1):.2f} s")
print(f"Geraden: frueh {-60 / g1[0]:.2f} s, spaet {-60 / g2[0]:.2f} s, Knick {t[kn]:.2f} s")
fig, ax = plt.subplots(2, 1, figsize=(11, 8))
ax[0].plot(t, p - p[0], color="0.5", lw=0.8, label="C++-Pegel Teilton 1")
for q, f, name in ((r.x, "b", "ab 0"), (r2.x, "g", f"ab Rahmen {ab}")):
    a1, s1, a2, s2, lb = q
    ax[0].plot(t, 10 * np.log10(np.exp(a1) * 10 ** (s1 * t / 10) / e[0]), f + "--", label=f"{name}: Komp. 1 T60 {-60 / s1:.2f} s")
    ax[0].plot(t, 10 * np.log10(np.exp(a2) * 10 ** (s2 * t / 10) / e[0]), f + ":", label=f"{name}: Komp. 2 T60 {-60 / s2:.2f} s")
ax[0].set_ylim(-60, 5); ax[0].legend(fontsize=8)
ax[1].plot(t, L, "k", label="Schroeder")
ax[1].plot(t[:kn], g1[0] * t[:kn] + g1[1], "r", label=f"Geraden {-60 / g1[0]:.2f} / {-60 / g2[0]:.2f} s, Knick {t[kn]:.2f}")
ax[1].plot(t[kn:], g2[0] * t[kn:] + g2[1], "r")
ax[1].legend(fontsize=8); ax[1].set_xlabel("s ab Anfang des Bereichs")
fig.tight_layout(); fig.savefig(aus, dpi=85)
