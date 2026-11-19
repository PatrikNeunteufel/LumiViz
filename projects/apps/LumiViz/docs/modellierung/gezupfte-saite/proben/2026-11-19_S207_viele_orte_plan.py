"""S207, KQ36 a - Pruefkoerper: Kopie eines Bauplans mit 16 Hoerorten auf der vorderen Halbkugel (Radius 2,06 m wie
die TU-Mikrofone, um die Mitte des Umrisses der Platte, Fibonacci-Gitter, mindestens 10 Grad ueber der Platte); der
bisherige erste Hoerort bleibt als erster. Die Harfe wurde mit `..._harfe_viele_orte_plan.py` geschrieben (Mitte
x = 0, y = 0,655 - dasselbe auf 1 mm).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <name des koerpers> <zielordner>
"""
import json
import math
import pathlib
import sys

name = sys.argv[1]
plan = json.loads(pathlib.Path(f"asset/loop/instrument_bodies/{name}.lvinstbodyplan").read_text(encoding="utf-8"))
umriss = plan["plate"]["outline"]
mx = 0.5 * (min(p[0] for p in umriss) + max(p[0] for p in umriss))
my = 0.5 * (min(p[1] for p in umriss) + max(p[1] for p in umriss))
orte = [plan["listeningPoints"][0]]
N, R = 16, 2.06
z0 = math.sin(math.radians(10))
for i in range(N):
    z = z0 + (1 - z0) * (i + 0.5) / N
    a = i * math.pi * (3 - math.sqrt(5))
    r = math.sqrt(1 - z * z)
    orte.append({"name": f"k{i + 1:02d}", "x": round(mx + R * r * math.cos(a), 4), "y": round(my + R * r * math.sin(a), 4),
                 "z": round(R * z, 4), "source": "S207 probe (KQ36 a): front hemisphere, 2.06 m"})
plan["listeningPoints"] = orte
ziel = pathlib.Path(sys.argv[2])
ziel.mkdir(parents=True, exist_ok=True)
(ziel / f"{name}.lvinstbodyplan").write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8")
print(f"{name}: Mitte {mx:.3f} / {my:.3f}, {len(orte)} Hoerorte -> {ziel}")
