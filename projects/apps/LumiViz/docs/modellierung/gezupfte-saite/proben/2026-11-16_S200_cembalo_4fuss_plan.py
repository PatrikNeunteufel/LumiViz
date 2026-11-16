"""S200, 34k-6 — Probe: der Bauplan des Cembalos mit Stegpunkten auf BEIDEN Stegen (8' und 4').

Aufruf (vom Projekt-Root): python -I -B <dieses Skript> <zielordner>

Schreibt `<zielordner>/harpsichord_4fuss.lvinstbodyplan`: der Bauplan aus `make_cembalo.py`, unveraendert bis auf die
Stegpunkte — zu den 61 Punkten auf dem 8'-Steg (Tasten 29-89) kommen 61 auf dem 4'-Steg, gefuehrt als Tasten 129-189
(Taste + 100; das Format traegt einen Punkt je Taste, die Probe braucht beide Stege in EINEM Koerper, damit der
Pruefstand die Register ueber den Boden koppeln kann). Quer liegt Taste k auf dem 4'-Steg wie auf dem 8'-Steg
(gleichmaessig zwischen dem ersten und letzten Stift des 4'-Stegs); die neun Tasten des Taskin ausserhalb des Dulcken am
Punkt der naechsten Randtaste (KQ16 a). Kein Koerper fuer das Programm.

Danach: InstrumentBodyTool <zielordner>/harpsichord_4fuss.lvinstbodyplan --to 5000
"""
import importlib.util
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[7]
spec = importlib.util.spec_from_file_location("make_cembalo", ROOT / "asset/loop/instrument_bodies/make_cembalo.py")
mc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mc)

if __name__ == "__main__":
    ziel = pathlib.Path(sys.argv[1])
    geo = json.load(open(ROOT / mc.GEOMETRIE, encoding="utf-8-sig"))
    dicke = json.load(open(ROOT / mc.DICKE, encoding="utf-8-sig"))
    plan = mc.baue(geo, dicke, mc.TIEFE, mc.ROSE_OFFEN)
    vier = mc.tasten_am_steg(geo["steg4"], geo["steg4"], mc.TASTEN)
    je_taste = {mc.ERSTE_TASTE + k: p for k, p in enumerate(vier)}
    neu = []
    for t in range(mc.TASKIN_VON, mc.TASKIN_BIS + 1):
        p = je_taste[min(max(t, mc.ERSTE_TASTE), mc.ERSTE_TASTE + mc.TASTEN - 1)]
        neu.append({"key": t + 100, "x": round(p[0], 4), "y": round(p[1], 4),
                    "source": "probe S200: the key 100 below, on the line of the 4' bridge"})
    plan["bridgePoints"] += neu
    plan["title"] += " - probe with points on both bridges"
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "harpsichord_4fuss.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    acht = {p["key"]: p for p in plan["bridgePoints"][:61]}
    print(datei, len(plan["bridgePoints"]), "Stegpunkte")
    for t in (29, 36, 48, 60, 72, 84, 89):
        a, b = acht[t], neu[t - 29]
        print(f"Taste {t}: 8' ({a['x']:.3f}, {a['y']:.3f})  4' ({b['x']:.3f}, {b['y']:.3f})  Abstand laengs {a['x'] - b['x']:+.3f} m")
