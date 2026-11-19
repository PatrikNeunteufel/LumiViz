"""S209 - Pruefkoerper nach KQ36 a (16 Hoerorte in 2,06 m) aus der besten Fassung von oud_feinsuche6 (Rosetten als
weiche Leisten im Bauplan): Decke 2,233 mm, Steg 17 cm x 25,84 x 8,61 mm (650 kg/m3), Balkenenden 2 mm vor dem Rand,
Hohlraum 20,61 l, Muendung 1,522, Rosette gross 11,45 g, klein je 1 g.
Aufruf: python -X utf8 -B oud_pruefkoerper6.py <zielordner>"""
import json
import math
import pathlib
import sys

import oud_feinsuche6 as f6

plan = f6.plan([2.233, 8.615, 25.838, 1.522, 11.448])
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
print(f"Mitte {mx:.3f} / {my:.3f}, {len(orte)} Hoerorte -> {ziel}")
