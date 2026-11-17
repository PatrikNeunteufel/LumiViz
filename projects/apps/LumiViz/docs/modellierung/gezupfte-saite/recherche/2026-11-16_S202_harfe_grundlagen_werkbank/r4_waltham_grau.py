"""R4: Waltham et al. 2008 Abb. 4 (Salvi Aurora, Punkt-Admittanz auf der Saitenleiste): Grauwert an den Saitenpunkten
-> Stufe der Legende -> s/kg, unter zwei Lesarten der Legende (10^-1 ... 10^-5 oder 10^-1 ... 10^-3)."""
import math
from PIL import Image
d = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\seiten"
im = Image.open(d + r"\waltham300-2.png").convert("L")
X0, Y0 = 1338, 33
# Stufen der Legende (Grauwert, von dunkel=hoch nach hell=tief), aus r3 gemessen
STUFEN = [1, 27, 56, 84, 114, 141, 171, 199, 226]
def stufe(g): return min(range(len(STUFEN)), key=lambda i: abs(STUFEN[i] - g))
# weisse Kreise im Bildfeld suchen
pts = []
for y in range(262, 590):
    for x in range(150, 826):
        if im.getpixel((X0 + x, Y0 + y)) > 236:
            pts.append((x, y))
cl = []
for x, y in pts:
    for c in cl:
        if abs(c[0] / c[2] - x) < 12 and abs(c[1] / c[2] - y) < 12:
            c[0] += x; c[1] += y; c[2] += 1; break
    else:
        cl.append([x, y, 1])
kreise = sorted([(c[0] / c[2], c[1] / c[2]) for c in cl if c[2] > 5])
f_von_x = lambda x: (x - 123.0) / 0.71        # 0 Hz bei x 123, 1000 Hz bei x 833 (Achsenteilung)
h_von_y = lambda y: (609.0 - y) / 0.2825      # 0 mm bei y 609, 1200 mm bei y 270
print("f[Hz]  h[mm]  Grau  Stufe(0=dunkel)  dB_Bell(10^-1..10^-5)  dB_Bell(10^-1..10^-3)")
erg = []
for x, y in kreise:
    ring = []
    for a in range(0, 360, 15):
        for r in (13, 16):
            px = int(x + r * math.cos(math.radians(a))); py = int(y + r * math.sin(math.radians(a)))
            ring.append(im.getpixel((X0 + px, Y0 + py)))
    ring.sort(); g = ring[len(ring) // 2]
    s = stufe(g)
    # Stufe s (0..8) ueberdeckt in log10 Y den Bereich -1 - (s+1)*D ... -1 - s*D, Mitte -1 - (s+0.5)*D
    y5 = -1 - (s + 0.5) * 4 / 9; y3 = -1 - (s + 0.5) * 2 / 9
    erg.append((f_von_x(x), s, 20 * y5 + 116, 20 * y3 + 116))
    print(f"{f_von_x(x):6.0f} {h_von_y(y):6.0f} {g:5d} {s:4d}  {20*y5+116:8.1f}  {20*y3+116:8.1f}")
print(len(kreise), "Kreise")
