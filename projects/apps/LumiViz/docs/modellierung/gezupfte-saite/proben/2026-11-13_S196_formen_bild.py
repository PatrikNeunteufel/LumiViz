"""Zeichnet die Modenformen aus `InstrumentBodyTool --shapes` (34k-5, KQ7 a: Formen zuordnen).

Aufruf: <formen.txt> <bild.png> [<wieviele>] [<titel>]

Je Mode ein Feld: w an den Knoten, rot/blau = entgegengesetzte Richtung, auf den groessten Betrag
normiert; dazu je Mode zwei Kennzahlen:
  sym  = Anteil der Form, der links/rechts gleich ist (1 = symmetrisch, 0 = antisymmetrisch),
         aus der Spiegelung an x = 0 (naechster Knoten),
  netto = |Summe w| / Summe |w| (1 = ein Bauch, 0 = Baeuche heben sich auf)."""
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.tri as mtri
import numpy as np

zeilen = open(sys.argv[1], encoding="utf-8").read().split("\n")
assert zeilen[0].startswith("lumiviz-body-shapes")
n = int(zeilen[1].split()[1])
knoten = np.array([[float(v) for v in z.split()] for z in zeilen[2:2 + n]])
t = int(zeilen[2 + n].split()[1])
dreiecke = np.array([[int(v) for v in z.split()] for z in zeilen[3 + n:3 + n + t]])
m = int(zeilen[3 + n + t].split()[1])
moden = []
for k in range(m):
    hz = float(zeilen[4 + n + t + 2 * k].split()[1])
    w = np.array([float(v) for v in zeilen[5 + n + t + 2 * k].split()])
    moden.append((hz, w))

wieviele = int(sys.argv[3]) if len(sys.argv) > 3 else 6
titel = sys.argv[4] if len(sys.argv) > 4 else ""
moden = moden[:wieviele]

# Spiegelpartner je Knoten (an x = 0)
gespiegelt = knoten * np.array([-1.0, 1.0])
partner = np.array([np.argmin(((knoten - p) ** 2).sum(axis=1)) for p in gespiegelt])

netz = mtri.Triangulation(knoten[:, 0], knoten[:, 1], dreiecke)
spalten = min(len(moden), 6)
reihen = (len(moden) + spalten - 1) // spalten
fig, achsen = plt.subplots(reihen, spalten, figsize=(2.6 * spalten, 3.6 * reihen), squeeze=False)
for a in achsen.ravel():
    a.axis("off")
for k, (hz, w) in enumerate(moden):
    a = achsen.ravel()[k]
    w = w / np.abs(w).max()
    if w[np.argmax(np.abs(w))] < 0:
        w = -w
    gleich = (w + w[partner]) / 2.0
    sym = (gleich ** 2).sum() / (w ** 2).sum()
    netto = abs(w.sum()) / np.abs(w).sum()
    a.tricontourf(netz, w, levels=np.linspace(-1, 1, 21), cmap="RdBu_r")
    a.triplot(netz, color="k", linewidth=0.05)
    a.set_aspect("equal")
    a.set_title(f"{k + 1}: {hz:.0f} Hz\nsym {sym:.2f}, netto {netto:.2f}", fontsize=9)
    print(f"Mode {k + 1}: {hz:.1f} Hz, sym {sym:.2f}, netto {netto:.2f}")
if titel:
    fig.suptitle(titel, fontsize=10)
fig.tight_layout()
fig.savefig(sys.argv[2], dpi=130)
print(sys.argv[2])
