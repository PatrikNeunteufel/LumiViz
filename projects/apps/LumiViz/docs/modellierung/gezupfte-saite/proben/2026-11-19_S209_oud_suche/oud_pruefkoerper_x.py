"""S209 - Pruefkoerper nach KQ36 a (16 Hoerorte in 2,06 m) fuer eine beliebige Fassung der Familie R mit Rosetten als
weichen Leisten (oud_feinsuche6.plan).
Aufruf: python -X utf8 -B oud_pruefkoerper_x.py <zielordner> <decke mm> <steghoehe mm> <stegbreite mm> <muendung> <rosette gross g> [ohne]
"ohne" laesst die Rosetten weg."""
import json
import math
import pathlib
import sys

import oud_feinsuche6 as f6
import oud_kombi as k

x = [float(v) for v in sys.argv[2:7]]
if len(sys.argv) > 7 and sys.argv[7] == "ohne":
    plan = k.fassung(dicke=x[0] / 1000, abstand=0.002, steg=(0.17, x[2] / 1000, x[1] / 1000, 650.0, 10e9), volumen=f6.VOLUMEN, muendung=x[3])
else:
    plan = f6.plan(x)
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
                 "z": round(R * z, 4), "source": "probe (KQ36 a): front hemisphere, 2.06 m"})
plan["listeningPoints"] = orte
ziel = pathlib.Path(sys.argv[1])
ziel.mkdir(parents=True, exist_ok=True)
(ziel / "oud.lvinstbodyplan").write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
print(f"{len(orte)} Hoerorte -> {ziel}")
