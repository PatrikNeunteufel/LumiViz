# Abb. 7.6 (Bell 1988, PDF 293, Messort 17): die drei Admittanzkurven spaltenweise abgreifen.
# Geometrie aus d1_abb76_geometrie.py (120 dpi):
#   x: Beschriftung 100 Hz Mitte 178,5 px, 800 Hz Mitte 798 px (linear, 0,885 px/Hz)
#   Panel 1 (loser Kasten):          90 dB Zeile 202,5 / 80 dB 291,5 / 70 dB 380,5
#   Panel 2 (Harfe ohne Saiten):     90 dB 558,5 / 80 dB 647,5 / 70 dB 735,5
#   Panel 3 (Harfe mit Saiten):      90 dB 857   / 80 dB 947,5
# Aufruf: python d2_abb76_kurven.py
import math, pathlib

HIER = pathlib.Path(__file__).parent
PGM = HIER / "seiten" / "bell293_120-293.pgm"


def lade_pgm(pfad):
    daten = pfad.read_bytes()
    teile, i = [], 0
    while len(teile) < 4:
        while daten[i:i + 1].isspace():
            i += 1
        j = i
        while not daten[j:j + 1].isspace():
            j += 1
        teile.append(daten[i:j])
        i = j
    i += 1
    w, h = int(teile[1]), int(teile[2])
    return w, h, daten[i:i + w * h]


W, H, PIX = lade_pgm(PGM)
dunkel = lambda x, y: PIX[y * W + x] < 140

X100, X800 = 178.5, 798.0
f_von_x = lambda x: 100 + (x - X100) * 700 / (X800 - X100)
x_von_f = lambda f: X100 + (f - 100) * (X800 - X100) / 700

PANELS = {
    # name: (y_oben, y_unten, (y_a, dB_a), (y_b, dB_b), gitterzeilen, titelzeile_bis)
    "lose": (140, 442,  # unter 442 stehen die Zahlen der Frequenzachse
(202.5, 90), (380.5, 70), (202, 203, 291, 292, 380, 381), 140),
    "ohne": (505, 822, (558.5, 90), (735.5, 70), (558, 559, 647, 648, 735, 736), 505),
    "mit": (850, 1033, (857.0, 90), (947.5, 80), (857, 947, 948), 850),
}


def db_von_y(y, a, b):
    (ya, da), (yb, db) = a, b
    return da + (y - ya) * (db - da) / (yb - ya)


def spur(name):
    y0, y1, a, b, gitter, _ = PANELS[name]
    werte = {}
    for x in range(int(X100), int(X800) + 1):
        ys = [y for y in range(y0, y1) if dunkel(x, y) and all(abs(y - g) > 1 for g in gitter)]
        if not ys:
            continue
        ys.sort()
        werte[x] = (db_von_y(ys[len(ys) // 2], a, b), db_von_y(ys[0], a, b), db_von_y(ys[-1], a, b))
    return werte


def wert_bei(sp, f, breite=2):
    x = round(x_von_f(f))
    nah = [sp[k][0] for k in range(x - breite, x + breite + 1) if k in sp]
    return sum(nah) / len(nah) if nah else float("nan")


def regression(sp, fmin, fmax):
    pts = [(math.log2(f_von_x(x)), v[0]) for x, v in sp.items() if fmin <= f_von_x(x) <= fmax]
    n = len(pts)
    mx = sum(p[0] for p in pts) / n
    my = sum(p[1] for p in pts) / n
    sxx = sum((p[0] - mx) ** 2 for p in pts)
    sxy = sum((p[0] - mx) * (p[1] - my) for p in pts)
    return sxy / sxx, n


def band_mittel(sp, fa, fb, art):
    v = [w[0] for x, w in sp.items() if fa <= f_von_x(x) < fb]
    if art == "dB":
        return sum(v) / len(v)
    # energetisch (Mittel von |Y|^2)
    return 10 * math.log10(sum(10 ** (d / 10) for d in v) / len(v))


def main():
    sp = {n: spur(n) for n in PANELS}
    for n in sp:
        print(f"{n}: {len(sp[n])} Spalten mit Kurve von {int(X800 - X100) + 1}")
    print()
    print("f [Hz] | lose | ohne Saiten | mit Saiten | mit - ohne")
    for f in list(range(110, 800, 30)) + [790]:
        l, o, m = (wert_bei(sp[n], f) for n in ("lose", "ohne", "mit"))
        print(f"{f:4d} | {l:5.1f} | {o:5.1f} | {m:5.1f} | {m - o:+5.1f}")
    print()
    print("Regression Pegel ueber log2 f (dB/Oktave):")
    for n in sp:
        for fa, fb in ((110, 790), (200, 790)):
            s, cnt = regression(sp[n], fa, fb)
            print(f"  {n:5s} {fa}-{fb} Hz: {s:+5.2f} dB/Oktave (n={cnt})")
    print()
    print("Bandmittel (arithm. dB / energetisch):")
    for fa, fb in ((110, 200), (200, 400), (400, 790)):
        z = []
        for n in ("lose", "ohne", "mit"):
            z.append(f"{n} {band_mittel(sp[n], fa, fb, 'dB'):5.1f}/{band_mittel(sp[n], fa, fb, 'E'):5.1f}")
        print(f"  {fa}-{fb} Hz: " + " | ".join(z))
    print()
    print("Minima und Maxima je Band (Median-Spur):")
    for fa, fb in ((110, 200), (200, 400), (400, 790)):
        z = []
        for n in ("lose", "ohne", "mit"):
            v = [w[0] for x, w in sp[n].items() if fa <= f_von_x(x) < fb]
            z.append(f"{n} {min(v):5.1f}..{max(v):5.1f}")
        print(f"  {fa}-{fb} Hz: " + " | ".join(z))


main()
