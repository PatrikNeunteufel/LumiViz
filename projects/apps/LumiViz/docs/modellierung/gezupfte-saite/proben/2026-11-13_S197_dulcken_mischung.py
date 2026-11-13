"""Probe: wie deckend liegt die Dickenkarte in Abb. 1 ueber dem Foto? Farben an bekannten Stellen und die
Haeufung der Farben der Karte (k-means), gegen die Farbskala."""
import sys

import numpy as np
from PIL import Image
from scipy.cluster.vq import kmeans2

Image.MAX_IMAGE_PIXELS = None
a1 = np.asarray(Image.open(sys.argv[1]).convert("RGB").resize((1960, 1018), Image.LANCZOS)).astype(float)
stufen, farben = [], []
for mm in np.arange(2.25, 7.3, 0.5):
    x = 200.0 + (mm - 2.5) * (1760.0 - 200.0) / 4.5
    stufen.append(mm)
    farben.append(a1[960:966, int(x) - 6:int(x) + 7].reshape(-1, 3).mean(axis=0))
farben = np.array(farben)
for name, x, y in (("5,72 Bass (neben der Schrift)", 215, 640), ("5,56 Mitte", 800, 640), ("4,29", 1450, 440), ("blau oben", 1560, 110),
                   ("gelber Fleck", 690, 690), ("gelber Fleck 2", 1010, 560), ("braun bei der Rose", 1300, 650), ("Foto rechts (ohne Karte)", 1750, 500)):
    f = a1[y - 5:y + 6, x - 5:x + 6].reshape(-1, 3)
    print(f"{name:32s} {np.median(f, axis=0).round(0)}")
bild = a1[40:800, 100:1640].reshape(-1, 3)
mx, mn = bild.max(axis=1), bild.min(axis=1)
px = bild[((mx - mn) > 14) & (mx > 70)]
np.random.seed(1)
zentren, zu = kmeans2(px[::20], 9, minit="++", seed=3)
anz = np.bincount(zu, minlength=9)
print("Haeufungen der Kartenfarben (Farbe, Anteil):")
for k in np.argsort(-anz):
    print("  ", zentren[k].round(0), f"{100 * anz[k] / anz.sum():.1f} %")
print("Farbskala:")
for s, f in zip(stufen, farben):
    print(f"   {s:.2f} mm {f.round(0)}")
