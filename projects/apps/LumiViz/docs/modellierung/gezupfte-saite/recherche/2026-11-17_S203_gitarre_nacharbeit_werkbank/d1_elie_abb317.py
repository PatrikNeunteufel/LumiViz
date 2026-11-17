"""Digitalisiert Elie 2012 (These), Abb. 3.17, S. 124: Yzz (rot), Yyz (blau, gestrichelt), Yyy (gruen), 0-4 kHz,
-60..10 dB. Eingabe: Seitenbild 300 dpi als PPM (pdftoppm ohne -png). Achsen aus dem Rahmen des Diagramms.
Ausgabe: je Band Median (dB) und Energiemittel (dB) je Kurve, dazu Differenzen gegen Yzz.
Grenzen: Linienbreite ~2 px (= 0,1 dB / 3 Hz); gestrichelte Kurve hat Luecken; senkrechte Flanken zaehlen mit."""
import math
import pathlib
import statistics

P = pathlib.Path(__file__).parent / "seiten" / "Elie2012_These_S124_300dpi-124.ppm"


def lade(p):
    b = p.read_bytes()
    teile, pos = [], 0
    while len(teile) < 4:
        while b[pos:pos + 1].isspace():
            pos += 1
        if b[pos:pos + 1] == b"#":
            pos = b.index(b"\n", pos) + 1
            continue
        anf = pos
        while not b[pos:pos + 1].isspace():
            pos += 1
        teile.append(b[anf:pos])
    pos += 1
    w, h = int(teile[1]), int(teile[2])
    return w, h, b[pos:]


w, h, px = lade(P)


def rgb(x, y):
    i = 3 * (y * w + x)
    return px[i], px[i + 1], px[i + 2]


def dunkel(c):
    return c[0] < 60 and c[1] < 60 and c[2] < 60


# Rahmen: im oberen Seitendrittel bis -hälfte die Spalte/Zeile mit den meisten schwarzen Pixeln
Y0, Y1 = int(0.08 * h), int(0.45 * h)
spalten = [(sum(dunkel(rgb(x, y)) for y in range(Y0, Y1, 2)), x) for x in range(int(0.1 * w), int(0.9 * w))]
spalten.sort(reverse=True)
xs = sorted(x for _, x in spalten[:12])
links, rechts = xs[0], xs[-1]
zeilen = [(sum(dunkel(rgb(x, y)) for x in range(links, rechts, 2)), y) for y in range(Y0, Y1)]
zeilen.sort(reverse=True)
ys = sorted(y for _, y in zeilen[:12])
oben, unten = ys[0], ys[-1]
# Die automatische Erkennung greift die Achsenbeschriftung; d1b_rahmen.py zeigt den Rahmen bei x = 586 und 1960
# (Gitterlinien alle 171,75 px = 500 Hz: 758, 930, ..., 1960) und y = 388 (10 dB) und 1376 (-60 dB). Fest gesetzt:
links, rechts, oben, unten = 586, 1960, 388, 1376
print(f"Bild {w}x{h}; Rahmen x {links}..{rechts}, y {oben}..{unten}")


def f_von(x):
    return (x - links) / (rechts - links) * 4000.0


def db_von(y):
    return 10.0 - (y - oben) / (unten - oben) * 70.0


def farbe(c):
    r, g, b = c
    if r > 170 and g < 90 and b < 90:
        return "Yzz"
    if b > 170 and r < 90 and g < 120:
        return "Yyz"
    if g > 90 and r < 70 and b < 70:
        return "Yyy"
    return None


# Legende (oben rechts) ausnehmen: f > 3200 Hz und dB > -12
kurven = {"Yzz": {}, "Yyz": {}, "Yyy": {}}
for x in range(links + 3, rechts - 2):
    f = f_von(x)
    for y in range(oben + 3, unten - 2):
        k = farbe(rgb(x, y))
        if k is None:
            continue
        d = db_von(y)
        if f > 3200 and d > -12:
            continue
        kurven[k].setdefault(x, []).append(d)

# je Spalte Median der Pixel -> eine Kurve
kurve = {k: {f_von(x): statistics.median(v) for x, v in s.items()} for k, s in kurven.items()}
BAENDER = [(50, 150), (150, 300), (300, 500), (500, 1000), (1000, 1500), (1500, 2000), (2000, 2500), (2500, 3000),
           (3000, 3500), (3500, 4000), (1000, 4000), (2200, 4000)]
print("Band Hz      | Kurve | Punkte | Median dB | Energiemittel dB | Median - Yzz | Energie - Yzz")
for a, e in BAENDER:
    ref = None
    for k in ("Yzz", "Yyz", "Yyy"):
        werte = [d for f, d in kurve[k].items() if a <= f < e]
        if not werte:
            print(f"{a:5d}-{e:<5d} | {k} | 0")
            continue
        med = statistics.median(werte)
        en = 10 * math.log10(sum(10 ** (d / 10) for d in werte) / len(werte))
        if k == "Yzz":
            ref = (med, en)
            print(f"{a:5d}-{e:<5d} | {k} | {len(werte):5d} | {med:7.1f} | {en:7.1f} |")
        else:
            print(f"{a:5d}-{e:<5d} | {k} | {len(werte):5d} | {med:7.1f} | {en:7.1f} | {med - ref[0]:6.1f} | {en - ref[1]:6.1f}")
# Spitzen unter 500 Hz
for k in ("Yzz", "Yyz", "Yyy"):
    pk = max(((d, f) for f, d in kurve[k].items() if 150 <= f < 400), default=None)
    print(f"Spitze 150-400 Hz {k}: {pk[0]:.1f} dB bei {pk[1]:.0f} Hz")
