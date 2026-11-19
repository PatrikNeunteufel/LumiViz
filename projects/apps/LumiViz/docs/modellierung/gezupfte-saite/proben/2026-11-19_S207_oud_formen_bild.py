"""S207 - Formen der gekoppelten Moden der Oud unter 420 Hz (`InstrumentBodyTool --shapes`) als Bild, mit Steg
(y = 9 cm), Antwortpunkt 3 der Quelle (x = 7,5, y = 13,5 cm), Balken und Loechern; dazu je Mode die Auslenkung am
Steg (Mitte, Chor 1) und am Antwortpunkt, der groesste Betrag und sein Ort, und das verdraengte Volumen
(Integral w dA) gegen Integral |w| dA.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <formen.txt> <bild.png> [<bauplan>]
"""
import json
import pathlib
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.tri as mtri
import numpy as np

z = open(sys.argv[1], encoding="utf-8").read().split("\n")
n = int(z[1].split()[1])
knoten = np.array([[float(v) for v in s.split()] for s in z[2:2 + n]])
t = int(z[2 + n].split()[1])
dreiecke = np.array([[int(v) for v in s.split()] for s in z[3 + n:3 + n + t]])
m = int(z[3 + n + t].split()[1])
hz = np.array([float(z[4 + n + t + 2 * k].split()[1]) for k in range(m)])
w = np.array([[float(v) for v in z[5 + n + t + 2 * k].split()] for k in range(m)])
plan = json.loads(pathlib.Path(sys.argv[3] if len(sys.argv) > 3 else "asset/loop/instrument_bodies/oud.lvinstbodyplan").read_text(encoding="utf-8"))

p = knoten[dreiecke]
flaeche = 0.5 * np.abs((p[:, 1, 0] - p[:, 0, 0]) * (p[:, 2, 1] - p[:, 0, 1]) - (p[:, 2, 0] - p[:, 0, 0]) * (p[:, 1, 1] - p[:, 0, 1]))


def naechster(x, y):
    return int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))


steg, chor1, antwort = naechster(0.0, 0.09), naechster(-0.0275, 0.09), naechster(0.075, 0.135)
print(f"{n} Knoten, {t} Dreiecke, Flaeche {flaeche.sum():.4f} m2")
print("Mode Hz | w am Steg Mitte / Chor 1 / Antwortpunkt (gegen den groessten Betrag) | Ort des groessten Betrags x, y cm | Volumen / Betrag")
zeigen = [k for k in range(m) if hz[k] < 420]
fig, achsen = plt.subplots(2, (len(zeigen) + 1) // 2, figsize=(3.0 * ((len(zeigen) + 1) // 2), 8.4))
tri = mtri.Triangulation(knoten[:, 0], knoten[:, 1], dreiecke)
for a, k in zip(achsen.ravel(), zeigen):
    v = w[k] / np.abs(w[k]).max()
    i = int(np.argmax(np.abs(w[k])))
    mittel = v[dreiecke].mean(axis=1)
    print(f"{hz[k]:7.1f} | {v[steg]:+.2f} / {v[chor1]:+.2f} / {v[antwort]:+.2f} | {100 * knoten[i, 0]:+5.1f} {100 * knoten[i, 1]:5.1f} | "
          f"{np.sum(mittel * flaeche) / np.sum(np.abs(mittel) * flaeche):+.2f}")
    a.tricontourf(tri, v, levels=np.linspace(-1, 1, 21), cmap="RdBu_r")
    for r in plan["ribs"]:
        pfad = np.array(r.get("path", r.get("line", [])))
        if pfad.size:
            a.plot(pfad[:, 0], pfad[:, 1], color="k", linewidth=0.8)
    a.plot([-0.0275, 0.0275], [0.09, 0.09], color="lime", linewidth=3)
    a.plot([0.075], [0.135], "o", color="yellow", markeredgecolor="k", markersize=5)
    a.set_title(f"{hz[k]:.1f} Hz", fontsize=9)
    a.set_aspect("equal")
    a.axis("off")
for a in achsen.ravel()[len(zeigen):]:
    a.axis("off")
fig.suptitle("Oud, gekoppelte Moden (rot / blau = Auslenkung gegeneinander); gruen der Steg, gelb der Antwortpunkt 3 der Quelle", fontsize=9)
fig.tight_layout()
fig.savefig(sys.argv[2], dpi=110)
print(sys.argv[2])
print("Rippen-Schluessel:", list(plan["ribs"][0].keys()))
