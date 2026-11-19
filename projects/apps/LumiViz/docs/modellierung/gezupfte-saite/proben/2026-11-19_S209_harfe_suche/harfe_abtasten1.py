"""S209 - Harfe: einzelne Stellgroessen abtasten (KQ40 a: Luftraum, Halt am Rand).
Aufruf: python -X utf8 -B harfe_abtasten1.py bass|seiten|luft|leisten"""
import copy
import sys

import harfe_suche as s

B = s.basis()
g = sys.argv[1]
if g == "bass":
    # Bassende: gehalten wie heute, dazu Drehfeder (N); zuletzt eingespannt
    for dreh in (1e2, 1e3, 1e4, 1e5, 1e7):
        s.rechne(s.bassende(B, 1e12, dreh), f"bass_dreh{dreh:g}")
if g == "seiten":
    for dreh in (300.0, 1000.0, 3000.0):
        p = copy.deepcopy(B)
        p["rim"]["rotationalStiffness"] = dreh
        s.rechne(p, f"seiten_dreh{dreh:g}")
if g == "luft":
    for vol, mu in ((0.027, 2.2), (0.027, 3.0), (0.023, 2.6), (0.031, 2.6)):
        p = copy.deepcopy(B)
        p["cavity"]["volume"], p["cavity"]["endCorrection"] = vol, mu
        s.rechne(p, f"luft_v{vol * 1000:.0f}_m{mu}")
if g == "leisten":
    # Deck- und Zugleiste bis an die Bassenkante (heute ab y = 5 mm), mit eingespanntem Bassende
    p = copy.deepcopy(B)
    for r in p["ribs"] + p["bridges"]:
        if r["name"] in ("straining bar", "cover bar"):
            r["line"][0][1] = 0.0
    s.rechne(p, "leisten_bis_kante")
    s.rechne(s.bassende(p, 1e12, 1e7), "leisten_bis_kante_bass_eingespannt")
