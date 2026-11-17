# Bell 1988, Abb. 7.21 (PDF 308, gedreht): Steigung der besaiteten Admittanz von 50 Hz bis zur Hochflaeche.
# Schritt 1: Ticks der Frequenzachse (senkrecht, rechts) und dB-Linien (senkrecht) finden.
# Schritt 2: Kurven Messort 4 und 7 zeilenweise abgreifen und Pegel ueber log2 f regressieren.
# Aufruf: python d4_abb721_steigung.py
import math, pathlib, subprocess

PDF = r"C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\gezupfte-saite\Bell_1988_AcousticalInvestigationConcertHarp_PhD.pdf"
HIER = pathlib.Path(__file__).parent
ZIEL = HIER / "seiten" / "bell308_110"


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


subprocess.run(["pdftoppm", "-r", "110", "-f", "308", "-l", "308", "-gray", PDF, str(ZIEL)], check=True)
W, H, PIX = lade_pgm(sorted(ZIEL.parent.glob("bell308_110*.pgm"))[0])
d = lambda x, y: PIX[y * W + x] < 140
print("Groesse", W, H)

# dB-Linien: Spalten mit vielen dunklen Pixeln zwischen y 280 und 1050
linien = [x for x in range(100, 760) if sum(d(x, y) for y in range(280, 1050)) > 500]
print("senkrechte Linien (x):", linien)

# Frequenzachse: die laengste senkrechte Linie rechts von x 650; Ticks = kurze waagrechte Striche rechts davon
achse = 680  # die -40-dB-Linie traegt die Ticks der Frequenzachse (725 ist der Rahmen)
print("Achse x =", achse)
ticks = []
if achse:
    for y in range(250, 1100):
        if all(d(x, y) for x in range(achse + 2, achse + 8)):
            ticks.append(y)
print("Tick-Zeilen:", ticks)

# Schritt 2. Achsen aus Schritt 1: Ticks 50 Hz y 1022, 100 Hz 925,5, 200 Hz 828, 500 Hz 699, 1 kHz 602, 2 kHz 504,
# 5 kHz 374,5, 10 kHz 278 (97,3 px je Oktave); dB-Linien 0 dB x 161, -10 x 291, -20 x 420, -30 x 549,5, -40 x 679,5.
Y50, PX_OKT = 1022.0, (1022.0 - 278.0) / math.log2(10000 / 50)
f_von_y = lambda y: 50 * 2 ** ((Y50 - y) / PX_OKT)
db_von_x = lambda x: -(x - 161.0) * 40 / (679.5 - 161.0)
GITTER = (161, 291, 420, 549, 550, 679, 680)

ort4, ort7 = {}, {}
for y in range(279, 1022):
    # ab x 677 liegen Achse und Ticks
    xs = [x for x in range(105, 677) if d(x, y) and all(abs(x - g) > 2 for g in GITTER)]
    if not xs:
        continue
    gruppen, g = [], [xs[0]]
    for x in xs[1:]:
        if x - g[-1] > 40:
            gruppen.append(g)
            g = []
        g.append(x)
    gruppen.append(g)
    if len(gruppen) >= 2 and gruppen[-1][0] - gruppen[0][-1] > 100:
        ort4[y] = db_von_x(sum(gruppen[0]) / len(gruppen[0]))
        ort7[y] = db_von_x(sum(gruppen[-1]) / len(gruppen[-1])) + 20.0  # Bild: Ort 7 um 20 dB nach unten verschoben


def regr(sp, fa, fb):
    pts = [(math.log2(f_von_y(y)), v) for y, v in sp.items() if fa <= f_von_y(y) <= fb]
    n = len(pts)
    mx, my = sum(p[0] for p in pts) / n, sum(p[1] for p in pts) / n
    return sum((p[0] - mx) * (p[1] - my) for p in pts) / sum((p[0] - mx) ** 2 for p in pts), n


def bei(sp, f):
    y = round(Y50 - math.log2(f / 50) * PX_OKT)
    v = [sp[k] for k in range(y - 3, y + 4) if k in sp]
    return sum(v) / len(v) if v else float("nan")


print()
print(f"Zeilen mit beiden Kurven: {len(ort4)}")
print("f [Hz] | Ort 4 [dB rel.] | Ort 7 [dB rel., Verschiebung zurueckgerechnet]")
for f in (50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 3150, 5000, 8000, 10000):
    print(f"{f:6d} | {bei(ort4, f):6.1f} | {bei(ort7, f):6.1f}")
for name, sp in (("Ort 4", ort4), ("Ort 7", ort7)):
    for fa, fb in ((50, 200), (50, 300), (50, 1000), (2000, 10000)):
        s, n = regr(sp, fa, fb)
        print(f"{name}: Steigung {fa}-{fb} Hz {s:+5.2f} dB/Oktave (n={n})")
    # Hochflaeche: Maximum des gleitenden Mittels ueber 1/3 Oktave
    glatt = {}
    for y in sp:
        fen = [sp[k] for k in range(y - 16, y + 17) if k in sp]
        glatt[y] = sum(fen) / len(fen)
    ymax = max(glatt, key=glatt.get)
    print(f"{name}: hoechstes Terzmittel {glatt[ymax]:+5.1f} dB bei {f_von_y(ymax):5.0f} Hz; "
          f"56 Hz bis dorthin {(glatt[ymax] - bei(sp, 56)) / math.log2(f_von_y(ymax) / 56):+5.2f} dB/Oktave")
