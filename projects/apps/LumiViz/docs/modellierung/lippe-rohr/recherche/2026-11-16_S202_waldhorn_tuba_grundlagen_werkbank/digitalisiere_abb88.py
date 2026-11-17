"""Digitalisiert Wachter 2008, Abb. 88 (Mensurverlauf F-Horn mit Mundstueck, Ventilstellung 0) aus dem 600-dpi-Bild.

Kalibrierung ueber die Gitterlinien (grau, gestrichelt): senkrecht alle 0,1 m von -0,2 bis 3,9; waagrecht alle
0,01 m von -0,17 bis 0,17. Die rote Kurve: obere und untere Kontur; Radius = halber Abstand (hebt einen Versatz der
Nulllinie auf). Ausgabe: x [m], r [mm] je 5 mm und die Gitterkalibrierung.
"""
import sys
import numpy as np
from PIL import Image

pfad = sys.argv[1]
img = np.asarray(Image.open(pfad).convert("RGB")).astype(int)
R, G, B = img[..., 0], img[..., 1], img[..., 2]
rot = (R > 180) & (G < 110) & (B < 110)
grau = (abs(R - G) < 12) & (abs(G - B) < 12) & (R > 100) & (R < 215)
h, w = rot.shape

def linien(maske, achse, mindest):
    s = maske.sum(axis=achse)
    kand = np.where(s > mindest)[0]
    gruppen = []
    for k in kand:
        if gruppen and k - gruppen[-1][-1] <= 2:
            gruppen[-1].append(k)
        else:
            gruppen.append([k])
    return [float(np.mean(g)) for g in gruppen], s

vx, _ = linien(grau, 0, 0.25 * h)
vx = [v for v in vx if v > 100]  # links davon liegen die Beschriftungen
hy, _ = linien(grau, 1, 0.25 * w)
print("senkrechte Gitterlinien:", len(vx), "waagrechte:", len(hy))
print("vx:", [round(v) for v in vx])
print("hy:", [round(v) for v in hy])

def ordne(pos):
    pos = np.array(pos)
    d = np.median(np.diff(pos))
    idx = np.round((pos - pos[0]) / d)
    # je Index nur eine Linie (die naechste am Raster)
    fit = np.polyfit(idx, pos, 1)
    for _ in range(3):
        rest = abs(np.polyval(fit, idx) - pos)
        ok = rest < 0.2 * d
        fit = np.polyfit(idx[ok], pos[ok], 1)
    return idx[ok], pos[ok], fit

xs, vxo, px_x = ordne(vx)
ys, hyo, px_y = ordne(hy)
vx, hy = vxo, hyo
print("Index x:", xs.min(), xs.max(), " Index y:", ys.min(), ys.max())
print("x: Pixel/0,1 m = %.3f, Rest max %.2f px" % (px_x[0], np.max(abs(np.polyval(px_x, xs) - vx))))
print("y: Pixel/0,01 m = %.3f, Rest max %.2f px" % (px_y[0], np.max(abs(np.polyval(px_y, ys) - hy))))
# erste senkrechte Linie = -0,2 m (falls 42 Linien); erste waagrechte = +0,17 m (oben)
x0_wert = -0.2 if len(vx) >= 41 else None
def x_von_px(p):
    return -0.2 + (p - px_x[1]) / px_x[0] * 0.1
def y_von_px(p):
    return 0.17 - (p - px_y[1]) / px_y[0] * 0.01
# Nulllinie
null_px = px_y[1] + 17 * px_y[0]
print("Nulllinie bei px %.1f" % null_px)

oben = np.full(w, np.nan)
unten = np.full(w, np.nan)
# waagrechte Stetigkeit: ein Pixel zaehlt zur Kurve, wenn in den Spalten c-3 und c+3 in +-2 Zeilen auch Rot liegt
# (die Strichmarken an den Abschnittsgrenzen sind senkrecht und fallen heraus)
from scipy.ndimage import maximum_filter
rot_dil = maximum_filter(rot.astype(np.uint8), size=(5, 1)).astype(bool)
links = np.zeros_like(rot)
rechts = np.zeros_like(rot)
links[:, 3:] = rot_dil[:, :-3]
rechts[:, :-3] = rot_dil[:, 3:]
kurve = rot & links & rechts
for c in range(w):
    zeilen = np.where(kurve[:, c])[0]
    o = zeilen[zeilen < null_px - 2]
    u = zeilen[zeilen > null_px + 2]
    if len(o):
        oben[c] = o.max()  # innerste Kurvenzeile oberhalb der Null
    if len(u):
        unten[c] = u.min()
# Becher (steil): je Zeile die Spalte der roten Pixel rechts von x = 3,45 m
becher = []
c_ab = int(px_x[1] + (3.45 + 0.2) / 0.1 * px_x[0])
for z in range(0, int(null_px) - 2):
    cs = np.where(rot[z, c_ab:])[0]
    zs = np.where(rot[int(2 * null_px) - z, c_ab:])[0] if int(2 * null_px) - z < h else []
    if len(cs) and len(zs):
        becher.append((x_von_px(c_ab + cs.min()), x_von_px(c_ab + zs.min()), 0.17 - (z - px_y[1]) / px_y[0] * 0.01))
np.savetxt(sys.argv[2].replace(".txt", "_becher.txt"), np.array(becher), fmt="%.5f",
           header="x_oben_m x_unten_m r_m (Becher je Bildzeile)")
print("Becher je Zeile (x_oben, x_unten, r) — Auswahl:")
for b in becher[:: max(1, len(becher) // 40)]:
    print("  x_o=%.4f x_u=%.4f r=%.1f mm" % (b[0], b[1], b[2] * 1000))
# Median ueber ein Fenster von 9 Spalten gegen die Strichmarken
def med(a, k=9):
    out = np.full_like(a, np.nan)
    for i in range(len(a)):
        f = a[max(0, i - k // 2): i + k // 2 + 1]
        f = f[~np.isnan(f)]
        if len(f):
            out[i] = np.median(f)
    return out
oben_m, unten_m = med(oben), med(unten)
spalten = np.arange(w)
xm = x_von_px(spalten)
r_mm = (y_von_px(oben_m) - y_von_px(unten_m)) / 2 * 1000
mitte_mm = (y_von_px(oben_m) + y_von_px(unten_m)) / 2 * 1000
gueltig = ~np.isnan(r_mm)
print("x-Bereich der Kurve: %.4f ... %.4f m" % (xm[gueltig].min(), xm[gueltig].max()))
print("Pixel: 1 px = %.2f mm in x, %.3f mm in r" % (0.1 / px_x[0] * 1000, 0.01 / px_y[0] * 1000 / 2 * 2))
np.savetxt(sys.argv[2], np.column_stack([xm[gueltig], r_mm[gueltig], mitte_mm[gueltig]]), fmt="%.5f",
           header="x_m r_mm mitte_mm (Wachter 2008 Abb. 88, digitalisiert)")
# Tabelle je 50 mm (und dichter im Becher)
punkte = list(np.arange(0.0, 3.0, 0.1)) + list(np.arange(3.0, 3.7, 0.05)) + list(np.arange(3.7, 3.87, 0.01))
xv, rv = xm[gueltig], r_mm[gueltig]
for p in punkte:
    i = np.argmin(abs(xv - p))
    if abs(xv[i] - p) < 0.003:
        print("x=%.3f r=%.2f" % (xv[i], rv[i]))
print("letzter Punkt: x=%.4f r=%.2f" % (xv[-1], rv[-1]))
