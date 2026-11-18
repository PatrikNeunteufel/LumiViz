"""Probe-Bauplan der Harfe, genau wie „Leisten uebereinander addieren sich“: die Deckleiste (Steg, geht im Kern vor)
bekommt in ihrer Breite die Hoehe Deckleiste + Zugleiste; die Zugleiste bleibt als Rippe und wirkt in den Streifen,
die sie breiter ist. Aufruf: python variante_b.py <ziel.lvinstbodyplan> [q=<Guete der Luft>] [kante=<N/m2>|fest]"""
import json, sys
import numpy as np

Q = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbodyplan"
p = json.load(open(Q, encoding="utf-8"))
opts = dict(a.split("=") for a in sys.argv[2:])
deck = p["bridges"][0]
zug = next(r for r in p["ribs"] if r["name"] == "straining bar")
y0, y1 = deck["line"][0][1], deck["line"][1][1]
yz = [q[1] for q in zug["line"]]
ys = sorted(set([y0, y1] + [y for y in yz if y0 < y < y1]))
dh = lambda y: np.interp(y, [y0, y1], deck["height"])
dw = lambda y: np.interp(y, [y0, y1], deck["width"])
zh = lambda y: np.interp(y, yz, zug["height"]) if yz[0] <= y <= yz[-1] else 0.0
hoehe = [round(float(dh(y) + zh(y)), 5) for y in ys]
breite = [round(float(dw(y)), 5) for y in ys]
deck["line"] = [[0.0, round(y, 4)] for y in ys]
deck["height"], deck["width"] = hoehe, breite
if "q" in opts:
    p["cavity"]["q"] = float(opts["q"])
if "kante" in opts:
    if opts["kante"] == "fest":
        p["rim"]["sections"] = []
    else:
        p["rim"]["sections"][0]["translationalStiffness"] = float(opts["kante"])
json.dump(p, open(sys.argv[1], "w", encoding="utf-8"), indent=1)
print("Deckleiste + Zugleiste (y, Hoehe, Breite):", list(zip(ys, hoehe, breite)))
