"""Liest aus Abdallah 2026 Abb. 3 (rechte Tafel, graue Decke) Umriss, Balkenlagen und Loecher in cm.
Eingabe: der Ausschnitt oud_kontrolle.png (rechte Haelfte der Seite). Ausgabe: JSON auf stdout.
Aufruf: oud_umriss.py <ausschnitt.png>"""
import json
import sys
from PIL import Image

bild = Image.open(sys.argv[1]).convert("RGB")
w, h = bild.size
px = bild.load()
X0, X1 = int(w * 0.29), int(w * 0.77)   # zwischen Massketten links und Bildrahmen rechts
Y0, Y1 = int(h * 0.125), int(h * 0.695)  # ueber dem Balkenprofil


def grau(p):
    r, g, b = p
    return abs(r - g) < 14 and abs(g - b) < 14 and 110 < r < 190


zeilen = {}
for y in range(Y0, Y1):
    xs = [x for x in range(X0, X1) if grau(px[x, y])]
    if len(xs) > 30:
        zeilen[y] = (min(xs), max(xs))
ys = sorted(zeilen)
oben, unten = ys[0], ys[-1]
skala = 50.0 / (unten - oben + 1)
breiteste = max(ys, key=lambda y: zeilen[y][1] - zeilen[y][0])
mitte = (zeilen[breiteste][0] + zeilen[breiteste][1]) / 2


def cm_x(x):
    return (x - mitte) * skala


def cm_y(y):
    return (unten - y) * skala


aus = {"skala_cm_je_px": skala, "groesste_breite_cm": (zeilen[breiteste][1] - zeilen[breiteste][0]) * skala,
       "hoehe_groesste_breite_cm": cm_y(breiteste), "umriss": [], "balken": [], "loecher": []}
for k in range(0, 101):
    y = min(max(round(unten - k * 0.5 / skala), oben), unten)
    l, r = zeilen[y]
    aus["umriss"].append([k * 0.5, round(cm_x(l), 2), round(cm_x(r), 2)])

# Balken: farbige Laeufe; Zeilen zu Gruppen
gruppen = []
for y in ys:
    l, r = zeilen[y]
    farbig = [x for x in range(l, r + 1)
              if (max(px[x, y]) - min(px[x, y])) > 22 and min(px[x, y]) < 225]
    if len(farbig) > 40:
        if gruppen and y - gruppen[-1]["y1"] <= 2:
            g = gruppen[-1]
            g["y1"] = y
            g["xs"] += farbig
        else:
            gruppen.append({"y0": y, "y1": y, "xs": list(farbig)})
for g in gruppen:
    xs = sorted(g["xs"])
    # Luecke in der Mitte (Balken 4 und 5 liegen links und rechts des Lochs)
    teile, start = [], xs[0]
    for a, b in zip(xs, xs[1:]):
        if b - a > 60:
            teile.append((start, a))
            start = b
    teile.append((start, xs[-1]))
    for a, b in teile:
        if (b - a) * skala > 3:
            aus["balken"].append({"hoehe_cm": round(cm_y((g["y0"] + g["y1"]) / 2), 2), "von_cm": round(cm_x(a), 2),
                                  "bis_cm": round(cm_x(b), 2), "laenge_cm": round((b - a) * skala, 2)})

# Loecher: weisse Flaechen innen, je Zeile Laeufe; zu Kreisen zusammengefasst ueber die Ausdehnung
laeufe = []
for y in ys:
    l, r = zeilen[y]
    start = None
    for x in range(l, r + 1):
        weiss = min(px[x, y]) > 238
        if weiss and start is None:
            start = x
        if (not weiss or x == r) and start is not None:
            if x - start > 25:
                laeufe.append((y, start, x))
            start = None
kreise = []
for y, a, b in laeufe:
    m = (a + b) / 2
    for k in kreise:
        if abs(k["m"] - m) < 40 and y - k["y1"] <= 3:
            k["y1"] = y
            k["a"] = min(k["a"], a)
            k["b"] = max(k["b"], b)
            k["m"] = (k["a"] + k["b"]) / 2
            break
    else:
        kreise.append({"y0": y, "y1": y, "a": a, "b": b, "m": m})
for k in kreise:
    aus["loecher"].append({"mitte_x_cm": round(cm_x((k["a"] + k["b"]) / 2), 2),
                           "mitte_hoehe_cm": round(cm_y((k["y0"] + k["y1"]) / 2), 2),
                           "durchmesser_quer_cm": round((k["b"] - k["a"]) * skala, 2),
                           "durchmesser_laengs_cm": round((k["y1"] - k["y0"]) * skala, 2)})
print(json.dumps(aus, indent=1))
