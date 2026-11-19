"""S207 - Kopie des Oud-Bauplans mit einem Hoerort 1 m vor der Mitte der Decke (wie das Mikrofon in Degirmenli 2017
Abb. 4). Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <zielordner>"""
import json
import pathlib
import sys

plan = json.loads(pathlib.Path("asset/loop/instrument_bodies/oud.lvinstbodyplan").read_text(encoding="utf-8"))
u = plan["plate"]["outline"]
mx = 0.5 * (min(p[0] for p in u) + max(p[0] for p in u))
my = 0.5 * (min(p[1] for p in u) + max(p[1] for p in u))
plan["listeningPoints"] = [{"name": "m1", "x": round(mx, 4), "y": round(my, 4), "z": 1.0,
                            "source": "S207 probe: 1 m in front of the centre of the soundboard"}]
ziel = pathlib.Path(sys.argv[1])
ziel.mkdir(parents=True, exist_ok=True)
(ziel / "oud.lvinstbodyplan").write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8")
print(f"Hoerort {mx:.3f} / {my:.3f} / 1.0 -> {ziel}")
