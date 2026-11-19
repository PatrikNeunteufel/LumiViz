"""S207 - prueft, dass ein Pruefkoerper (mehr Hoerorte) dieselben Moden traegt wie der mitgelieferte: Zahl, Frequenz,
Verlustfaktor, Formwerte, statistischer Teil ohne `radiation`, Abstrahlgewicht des ersten Hoerorts.
Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <name> <ordner des pruefkoerpers>"""
import json
import pathlib
import sys

import numpy as np

a = json.loads(pathlib.Path(f"asset/loop/instrument_bodies/{sys.argv[1]}.lvinstbody").read_text(encoding="utf-8"))
b = json.loads((pathlib.Path(sys.argv[2]) / f"{sys.argv[1]}.lvinstbody").read_text(encoding="utf-8"))
print(f"{sys.argv[1]}: Moden {len(a['modes'])} / {len(b['modes'])}")
if len(a["modes"]) == len(b["modes"]):
    for feld in ("f", "eta"):
        x, y = np.array([m[feld] for m in a["modes"]]), np.array([m[feld] for m in b["modes"]])
        print(f"  {feld}: groesste relative Abweichung {np.max(np.abs(x / y - 1)):.2e}")
    x, y = np.array([m["bridge"] for m in a["modes"]]), np.array([m["bridge"] for m in b["modes"]])
    print(f"  bridge: groesste Abweichung {np.max(np.abs(x - y)):.2e} (Betrag bis {np.max(np.abs(x)):.2f}); Vorzeichen je Mode gleich: "
          f"{bool(np.all(np.sign(x[:, 0]) == np.sign(y[:, 0])))}")
    x = np.array([complex(*m["radiation"][0]) for m in a["modes"]])
    y = np.array([complex(*m["radiation"][0]) for m in b["modes"]])
    print(f"  radiation, erster Hoerort: groesste Abweichung {np.max(np.abs(x - y)):.2e} (Betrag bis {np.max(np.abs(x)):.3f})")
sa, sb = dict(a["statistical"]), dict(b["statistical"])
ra, rb = sa.pop("radiation"), sb.pop("radiation")
print(f"  statistisch ohne radiation gleich: {sa == sb}; radiation erster Ort {ra[0]} / {rb[0]}")
if sa != sb:
    for k in sa:
        if sa[k] != sb.get(k):
            print(f"    {k}: {sa[k]} / {sb.get(k)}")
