"""Digitalisiert eine Impedanzkurve aus einem Seitenbild (Wachter 2008 Abb. 58 bzw. 89).

Aufruf: python digitalisiere_kurve.py <png> <farbe rot|blau|gruen> <x0_px> <x0_wert> <x1_px> <x1_wert>
        <y0_px> <y0_wert> <y1_px> <y1_wert> [<fmin> <fmax>]
Die Kalibrierpunkte (Pixel der Achsenteilung) werden vorher am Bild bestimmt (Gitterlinien, siehe Ausgabe
`--gitter`). Ausgabe: je Spalte der hoechste Pixel der Farbe; daraus die oertlichen Maxima (Frequenz, Hoehe).
"""
import sys
import numpy as np
from PIL import Image

img = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(int)
R, G, B = img[..., 0], img[..., 1], img[..., 2]
if sys.argv[2] == "--gitter":
    # dunkle bzw. graue, nicht farbige Pixel je Spalte/Zeile zaehlen
    grau = (abs(R - G) < 25) & (abs(G - B) < 25) & (R < 200)
    s0 = grau.sum(axis=0)
    s1 = grau.sum(axis=1)
    h, w = grau.shape
    print("Spalten mit vielen grauen Pixeln:", [i for i in range(w) if s0[i] > 0.15 * h])
    print("Zeilen mit vielen grauen Pixeln:", [i for i in range(h) if s1[i] > 0.15 * w])
    sys.exit()
farbe = sys.argv[2]
if farbe == "rot":
    m = (R > 170) & (G < 110) & (B < 110)
elif farbe == "blau":
    m = (B > 140) & (R < 90) & (G < 90)
else:
    m = (G > 150) & (R < 190) & (B < 90) & (G - R > 30)
x0p, x0v, x1p, x1v, y0p, y0v, y1p, y1v = map(float, sys.argv[3:11])
fmin, fmax = (float(sys.argv[11]), float(sys.argv[12])) if len(sys.argv) > 12 else (20.0, 900.0)
fx = lambda p: x0v + (p - x0p) * (x1v - x0v) / (x1p - x0p)
fy = lambda p: y0v + (p - y0p) * (y1v - y0v) / (y1p - y0p)
h, w = m.shape
oben = np.full(w, np.nan)
for c in range(w):
    z = np.where(m[:, c])[0]
    if len(z):
        oben[c] = z.min()
f = fx(np.arange(w))
y = fy(oben)
ok = ~np.isnan(y) & (f >= fmin) & (f <= fmax)
f, y = f[ok], y[ok]
# oertliche Maxima: hoeher als alles in +-(Fenster) Hz, Fenster 30 % des Abstands bei 40 Hz
spitzen = []
for i in range(len(f)):
    fen = (abs(f - f[i]) < 12.0)
    if y[i] >= y[fen].max() and y[i] > 3.0:
        if not spitzen or f[i] - spitzen[-1][0] > 15.0:
            # Mitte eines flachen Gipfels
            gipfel = f[fen & (y >= y[i] - 1e-9)]
            spitzen.append((float(gipfel.mean()), float(y[i])))
for fs, ys in spitzen:
    print("%.1f Hz  %.1f" % (fs, ys))
