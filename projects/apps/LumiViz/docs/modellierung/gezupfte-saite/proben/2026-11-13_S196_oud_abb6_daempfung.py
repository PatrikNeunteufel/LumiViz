"""Verlustfaktor der Oud aus der gemessenen Kurve (Abdallah u. a. 2026, Abb. 6): Halbwertsbreite je Spitze
(34k-5, S196). Aufruf: <kurve.txt>

Je Spitze: Sockel = der kleinere der beiden Nachbartaeler; Breite dort, wo die Kurve auf
Sockel + (Spitze − Sockel)/√2 gefallen ist (linear zwischen den Stuetzstellen); η = Δf / f.
Der Sockel traegt die Nachbarmoden — ohne ihn (Breite bei Spitze/√2) faellt η groesser aus; beide Werte
stehen in der Ausgabe. Die Kurve ist aus einem Bild gelesen (0,31 Hz je Bildspalte)."""
import sys

import numpy as np
from scipy.signal import find_peaks

d = np.loadtxt(sys.argv[1])
f, a = d[:, 0], d[:, 1]
orte, _ = find_peaks(a, prominence=0.04 * a.max(), distance=12)


def breite(i, schwelle):
    li = i
    while li > 0 and a[li] > schwelle:
        li -= 1
    re = i
    while re < len(a) - 1 and a[re] > schwelle:
        re += 1
    fl = f[li] + (schwelle - a[li]) * (f[li + 1] - f[li]) / (a[li + 1] - a[li]) if a[li + 1] != a[li] else f[li]
    fr = f[re - 1] + (schwelle - a[re - 1]) * (f[re] - f[re - 1]) / (a[re] - a[re - 1]) if a[re] != a[re - 1] else f[re]
    return fr - fl


for n, i in enumerate(orte):
    if f[i] < 60.0:
        continue
    links = orte[n - 1] if n > 0 else 0
    rechts = orte[n + 1] if n + 1 < len(orte) else len(a) - 1
    sockel = min(a[links:i + 1].min(), a[i:rechts + 1].min())
    mit = breite(i, sockel + (a[i] - sockel) / np.sqrt(2.0))
    ohne = breite(i, a[i] / np.sqrt(2.0))
    print(f"{f[i]:6.1f} Hz  Spitze {a[i]:.2f}  Sockel {sockel:.2f}  eta mit Sockel {mit / f[i]:.4f}  ohne {ohne / f[i]:.4f}")
