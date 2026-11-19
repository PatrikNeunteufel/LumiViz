"""Ablesung der Abstrahlkurven aus Değirmenli 2023, İdil 104, Abb. 3 (S. 531), am Ausschnitt
seiten/Degirmenli_2023_S531_Abb3_130dpi-07.png (pdftoppm -r 130, 580 x 400 px).

Kalibrierung (kurve_kalib.py): Gitterlinien x = 103/162/229/277/344/429/485/544 px bei 70/100/150/200/300/500/700/1000 Hz
-> 382 px je Dekade (Streuung 381-384), 50 Hz bei x = 47. y-Gitter 46/84/.../308 px bei 180/160/.../40 dB -> 1,875 px/dB.
Kurven unten (y > 180 px, unter rund 107 dB): rot, blau, grün; Legende (x > 385, y > 275) ausgeblendet.
Ausgabe je Frequenz: Wert an der Frequenz (Mittel der Spalte, linear zwischen Nachbarspalten) und Höchstwert in +-3 %.
"""
import sys, math
from PIL import Image

PFAD = sys.argv[1]
im = Image.open(PFAD).convert("RGB")
w, h = im.size
px = im.load()
X50, PXDEK = 47.2, 382.0
Y180, PXDB = 46.0, 1.875

def x_von_f(f):
    return X50 + PXDEK * math.log10(f / 50.0)

def db_von_y(y):
    return 180.0 - (y - Y180) / PXDB

KLASSEN = {
    "rot": lambda r, g, b: r > 190 and g < 90 and b < 90,
    "blau": lambda r, g, b: b > 170 and r < 90 and g < 130,
    "gruen": lambda r, g, b: g > 110 and r < 90 and b < 90,
}

def spalte(x, klasse):
    ys = []
    for y in range(181, 350):
        if x > 385 and y > 275:
            continue
        r, g, b = px[x, y]
        if KLASSEN[klasse](r, g, b):
            ys.append(y)
    return ys

def wert(f, klasse):
    x = x_von_f(f)
    xs = [int(math.floor(x)), int(math.floor(x)) + 1]
    werte = []
    for xi in xs:
        ys = spalte(xi, klasse)
        if ys:
            # Linie kann steil sein: oberster und unterster Treffer -> Spanne
            werte.append((db_von_y(max(ys)), db_von_y(min(ys))))
    return werte

def spitze(f, klasse, rel=0.03):
    x0, x1 = int(x_von_f(f * (1 - rel))), int(x_von_f(f * (1 + rel))) + 1
    best = None
    for xi in range(x0, x1 + 1):
        ys = spalte(xi, klasse)
        if ys:
            v = db_von_y(min(ys))
            if best is None or v > best[0]:
                best = (v, 50 * 10 ** ((xi - X50) / PXDEK))
    return best

def verlauf(klasse, luecke=6):
    """Mitte der Treffer je Spalte; Lücken bis `luecke` px (verdeckt durch andere Farben) linear überbrückt."""
    roh = {}
    for x in range(int(X50), int(X50 + PXDEK * math.log10(20)) + 1):
        ys = spalte(x, klasse)
        if ys:
            roh[x] = db_von_y(sorted(ys)[len(ys) // 2])
    xs = sorted(roh)
    voll = dict(roh)
    for a, b in zip(xs, xs[1:]):
        if 1 < b - a <= luecke:
            for x in range(a + 1, b):
                voll[x] = roh[a] + (roh[b] - roh[a]) * (x - a) / (b - a)
    return voll

if "--verlauf" in sys.argv:
    V = {kl: verlauf(kl) for kl in KLASSEN}
    print("Frequenz | " + " | ".join(KLASSEN) + "   (dB wie Achse; '--' = nicht ablesbar)")
    for name, f0 in [("C2", 65.41), ("F2", 87.31), ("A2", 110.0), ("D3", 146.83), ("G3", 196.0), ("C4", 261.63)]:
        for k in (1, 2, 3, 4):
            f = f0 * k
            x = round(x_von_f(f))
            werte = []
            for kl in KLASSEN:
                v = V[kl].get(x)
                werte.append(f"{v:5.1f}" if v is not None else "  -- ")
            print(f"{name} T{k} {f:6.1f} Hz | " + " | ".join(werte))
    print()
    f = 50.0
    while f <= 1000:
        x = round(x_von_f(f))
        werte = []
        for kl in KLASSEN:
            v = V[kl].get(x)
            werte.append(f"{v:5.1f}" if v is not None else "  -- ")
        print(f"{f:6.1f} Hz | " + " | ".join(werte))
        f *= 2 ** (1 / 6)
    sys.exit(0)

ZIELE = {
    "C2": 65.41, "F2": 87.31, "A2": 110.0, "D3": 146.83, "G3": 196.0, "C4": 261.63,
}
print("Frequenzen der Teiltöne 1-4 der leeren Chöre, Werte in dB wie auf der Achse von Abb. 3")
for name, f0 in ZIELE.items():
    for k in (1, 2, 3, 4):
        f = f0 * k
        if f > 1000:
            continue
        zeile = [f"{name} T{k} {f:7.1f} Hz"]
        for kl in KLASSEN:
            w_ = wert(f, kl)
            s = spitze(f, kl)
            if w_:
                lo = min(a for a, b in w_)
                hi = max(b for a, b in w_)
                zeile.append(f"{kl}: {lo:5.1f}-{hi:5.1f} (max+-3% {s[0]:5.1f} bei {s[1]:5.1f})")
            else:
                zeile.append(f"{kl}: --")
        print(" | ".join(zeile))
print()
print("Verlauf je 1/12 Oktave (Höchstwert der Spalte), 50-400 Hz")
f = 50.0
while f <= 400:
    zeile = [f"{f:6.1f}"]
    for kl in KLASSEN:
        w_ = wert(f, kl)
        zeile.append(f"{kl} {max(b for a, b in w_):5.1f}" if w_ else f"{kl}   -- ")
    print("  ".join(zeile))
    f *= 2 ** (1 / 12)
