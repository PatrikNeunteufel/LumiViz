"""S207 - Kopie des Harfen-Bauplans mit 16 Hoerorten auf der vorderen Halbkugel (Radius 2,06 m wie die TU-Mikrofone,
um die Mitte des Bodens x = 0, y = 0,655, z = 0; Fibonacci-Gitter, Hoehe ueber der Platte mindestens 10 Grad) und dem
bisherigen Ort `front` als erstem. Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <zielordner>
"""
import json
import math
import pathlib
import sys

plan = json.loads(pathlib.Path("asset/loop/instrument_bodies/concert_harp.lvinstbodyplan").read_text(encoding="utf-8"))
orte = [plan["listeningPoints"][0]]
N, R = 16, 2.06
z0 = math.sin(math.radians(10))
for i in range(N):
    z = z0 + (1 - z0) * (i + 0.5) / N
    a = i * math.pi * (3 - math.sqrt(5))
    r = math.sqrt(1 - z * z)
    orte.append({"name": f"k{i + 1:02d}", "x": round(R * r * math.cos(a), 4), "y": round(0.655 + R * r * math.sin(a), 4),
                 "z": round(R * z, 4), "source": "S207 probe: front hemisphere, 2.06 m"})
plan["listeningPoints"] = orte
ziel = pathlib.Path(sys.argv[1])
ziel.mkdir(parents=True, exist_ok=True)
(ziel / "concert_harp.lvinstbodyplan").write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8")
print(f"{len(orte)} Hoerorte -> {ziel}")
