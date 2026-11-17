"""Prueft die Rahmenerkennung von d1_elie_abb317.py: Spalten/Zeilen mit vielen dunklen Pixeln und die senkrechten
Gitterlinien (gepunktet, grau/schwarz) im Diagramm von Elie 2012 Abb. 3.17."""
import pathlib
import importlib.util

spec = importlib.util.spec_from_file_location("d1", pathlib.Path(__file__).parent / "d1_elie_abb317.py")
# nur die Ladefunktion wiederverwenden, ohne die Auswertung zu starten: Datei selbst lesen
P = pathlib.Path(__file__).parent / "seiten" / "Elie2012_These_S124_300dpi-124.ppm"
b = P.read_bytes()
kopf = b.split(b"\n", 3)
w, h = map(int, kopf[1].split())
px = kopf[3]


def rgb(x, y):
    i = 3 * (y * w + x)
    return px[i], px[i + 1], px[i + 2]


def dunkel(c):
    return c[0] < 140 and c[1] < 140 and c[2] < 140


Y0, Y1 = int(0.10 * h), int(0.42 * h)
zaehl = []
for x in range(int(0.15 * w), int(0.85 * w)):
    n = sum(dunkel(rgb(x, y)) for y in range(Y0, Y1))
    zaehl.append((n, x))
print("Spalten mit > 150 dunklen Pixeln:", [(x, n) for n, x in sorted(zaehl, key=lambda t: t[1]) if n > 150])
zz = []
for y in range(Y0, Y1):
    n = sum(dunkel(rgb(x, y)) for x in range(int(0.15 * w), int(0.85 * w)))
    zz.append((n, y))
print("Zeilen mit > 300 dunklen Pixeln:", [(y, n) for n, y in sorted(zz, key=lambda t: t[1]) if n > 300])
