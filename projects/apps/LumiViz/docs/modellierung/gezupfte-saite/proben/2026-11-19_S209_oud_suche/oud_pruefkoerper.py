"""S209 - Pruefkoerper nach KQ36 a (16 Hoerorte in 2,06 m) aus der besten Fassung der Familie R (fein5):
Decke 2,061 mm, Steg 17 cm x 20,04 x 6,44 mm (650 kg/m3), Balkenenden 2 mm vor dem Rand, Hohlraum 20,61 l,
Muendung 1,522. OHNE die Masse der Rosetten (der Bauplan kann sie noch nicht tragen) - eine Vorschau.
Aufruf: python -X utf8 -B oud_pruefkoerper.py <zielordner>"""
import json
import math
import pathlib
import sys

import oud_kombi as k

plan = k.fassung(dicke=0.002061, abstand=0.002, steg=(0.17, 0.02004, 0.00644, 650.0, 10e9), volumen=0.02061, muendung=1.522)
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
