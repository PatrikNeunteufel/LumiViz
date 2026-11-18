"""S205 - KQ31 a: wo die bewegte Masse einer Mode sitzt. Liest eine Formdatei (`InstrumentBodyTool --shapes`) und den
Bauplan dazu; nimmt die hoechste Mode der Datei. Gibt die Masse (gesamt, auf Leisten), die groesste Auslenkung auf der
Mittellinie mit 1/phi² (die wirksame Masse dort), den Anteil an m w² je Abschnitt laengs und die Form quer aus.
Die Dicke der Platte (10 -> 3 mm + 1 mm) ist fest eingetragen - nur fuer die Harfe.

Aufruf: python -X utf8 -B <dieses Skript> <formen.txt> <bauplan.lvinstbodyplan>
Schreibt nichts.
"""
import json, sys
import numpy as np

datei, plan = sys.argv[1], json.load(open(sys.argv[2], encoding="utf-8"))
z = open(datei, encoding="utf-8").read().split("\n")
n = int(z[1].split()[1]); xy = np.array([[float(a) for a in l.split()] for l in z[2:2 + n]])
nt = int(z[2 + n].split()[1]); tri = np.array([[int(a) for a in l.split()] for l in z[3 + n:3 + n + nt]])
i = 3 + n + nt + 1
moden = {}
while i < len(z) and z[i].startswith("mode"):
    moden[float(z[i].split()[1])] = np.array([float(a) for a in z[i + 1].split()]); i += 2
f = max(moden); w = moden[f]
mat = {m["name"]: m for m in plan["materials"]}
L = 1.31
def leiste(l, x, y):
    ys = [p[1] for p in l["line"]]; xs = [p[0] for p in l["line"]]
    if y < ys[0] or y > ys[-1]: return 0.0
    if abs(x - np.interp(y, ys, xs)) > np.interp(y, ys, l["width"]) / 2: return 0.0
    return float(np.interp(y, ys, l["height"]))
def rho_h(x, y):
    hp = 0.010 + (0.003 - 0.010) * y / L + 0.001
    for l in plan["bridges"] + plan["ribs"]:
        h = leiste(l, x, y)
        if h > 0: return mat[l["material"]]["density"] * (hp + h), True
    return mat[plan["plate"]["material"]]["density"] * hp, False
c = xy[tri].mean(axis=1)
e1, e2 = xy[tri[:, 1]] - xy[tri[:, 0]], xy[tri[:, 2]] - xy[tri[:, 0]]
a = 0.5 * np.abs(e1[:, 0] * e2[:, 1] - e1[:, 1] * e2[:, 0])
wm2 = (w[tri] ** 2).mean(axis=1)
rh = np.array([rho_h(*p) for p in c])
m = rh[:, 0] * a; auf = rh[:, 1].astype(bool)
ke = m * wm2
print(f"Mode {f:.1f} Hz: Masse gesamt {m.sum():.3f} kg, davon Mittelleiste+Leisten {m[auf].sum():.3f} kg; Summe m w² = {ke.sum():.3f} (massennormiert = 1)")
mitte = np.abs(xy[:, 0]) < 1e-9
ym, wmi = xy[mitte, 1], w[mitte]; o = np.argsort(ym); ym, wmi = ym[o], wmi[o]
k = np.argmax(np.abs(wmi)); print(f"groesste Auslenkung auf der Mittellinie bei y = {ym[k]:.3f} m (Ort {40 - (ym[k] + 0.057) / 0.035:.1f}): phi = {wmi[k]:.3f}, 1/phi² = {1e3 / wmi[k] ** 2:.0f} g")
print("Anteil an m w² je Abschnitt laengs (y) | davon auf Leisten:")
for y0, y1 in ((0, 0.4), (0.4, 0.8), (0.8, 1.0), (1.0, 1.15), (1.15, 1.31)):
    s = (c[:, 1] >= y0) & (c[:, 1] < y1)
    print(f"  y {y0:.2f}-{y1:.2f} m: {100 * ke[s].sum() / ke.sum():5.1f} % | {100 * ke[s & auf].sum() / ke.sum():5.1f} %")
for yq in (ym[k], 0.7, 0.3):
    s = np.abs(xy[:, 1] - yq) < 0.012
    xs, ws = xy[s, 0], w[s]; o = np.argsort(xs)
    b = 0.41 + (0.10 - 0.41) * yq / L
    print(f"quer bei y = {yq:.2f} m (Breite {b * 100:.0f} cm): " + " ".join(f"{x / (b / 2):+.2f}:{v / wmi[np.argmin(np.abs(ym - yq))]:.2f}" for x, v in zip(xs[o][::max(1, len(o) // 12)], ws[o][::max(1, len(o) // 12)])))
