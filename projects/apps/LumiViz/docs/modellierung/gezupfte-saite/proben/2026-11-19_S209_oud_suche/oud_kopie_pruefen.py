"""S209 - prueft den Bauplan aus make_oud.py (Kopie): Lagen des Instruments und wie gemessen (Aufnehmer 5,8 g im
Modenraum), Netz fuer 2500 Hz, Moden bis 1000 Hz. Aufruf: python -X utf8 -B oud_kopie_pruefen.py <oud.lvinstbodyplan>"""
import json
import sys

import numpy as np

import oud_feinsuche6 as f6
import oud_suche as s

plan = json.loads(open(sys.argv[1], encoding="utf-8").read())
u = plan["plate"]["outline"]
plan["listeningPoints"].append({"name": "m1", "x": 0.0, "y": round(0.5 * (min(p[1] for p in u) + max(p[1] for p in u)), 4), "z": 1.0, "source": "probe"})
e = s.rechne(plan, "kopie", to=1000, netz=int(sys.argv[2]) if len(sys.argv) > 2 else 2500)
print("Instrument: " + " / ".join(f"{v:.1f}" for v in e["alle_hz"][:6]))
print("wie gemessen: " + " / ".join(f"{v:.1f}" for v in f6.wie_gemessen(e)[:6]))
print("Lagen wie gemessen gegen 139 / 203 / 280 / 362 / 385: " + " / ".join(f"{v:+.1f} %" for v in f6.lagen(e)))
