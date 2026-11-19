"""S209 - Harfe: Platte ohne Luft, Dicke des Bodens (Bell & Firth: 12 -> 2 mm), Drehfeder nur am Bassteil der Seiten.
Aufruf: python -X utf8 -B harfe_abtasten2.py vakuum|dicke|bassseiten"""
import copy
import sys

import harfe_suche as s

B = s.basis()
LAENGE = 1.31


def dicke(plan, bass, diskant, furnier=0.001):
    p = copy.deepcopy(plan)
    g = p["plate"]["thicknessGrid"]
    werte = []
    for j in range(g["ny"]):
        y = min(j * g["step"], LAENGE)
        werte.extend([round(bass + (diskant - bass) * y / LAENGE + furnier, 6)] * g["nx"])
    g["values"] = werte
    return p


def bassseiten(plan, bis, dreh):
    """Die Seiten bis y = `bis` als eigene Abschnitte mit Drehfeder `dreh` (N), Auslenkung gehalten."""
    p = copy.deepcopy(plan)
    u = p["plate"]["outline"]  # [links unten, rechts unten, rechts oben, links oben]
    x = lambda y: 0.205 + (0.05 - 0.205) * y / LAENGE
    p["plate"]["outline"] = [u[0], u[1], [round(x(bis), 5), bis], u[2], u[3], [round(-x(bis), 5), bis]]
    for a in p["rim"]["sections"]:
        if a["name"] == "treble edge":
            a["edges"] = [3, 3]
    for name, kante in (("right bass side", 1), ("left bass side", 5)):
        p["rim"]["sections"].append({"name": name, "edges": [kante, kante], "translationalStiffness": 1e12, "rotationalStiffness": dreh, "source": "probe"})
    return p


g = sys.argv[1] if __name__ == "__main__" else ""
if g == "vakuum":
    p = copy.deepcopy(B)
    p["cavity"]["volume"] = 0.0
    s.rechne(p, "vakuum")
if g == "dicke":
    for b, d in ((0.012, 0.002), (0.012, 0.003), (0.011, 0.003), (0.013, 0.003)):
        s.rechne(dicke(B, b, d), f"dicke_{b * 1000:.0f}_{d * 1000:.0f}")
if g == "bassseiten":
    for bis in (0.35, 0.6):
        for dreh in (1000.0, 3000.0, 1e4, 1e7):
            s.rechne(bassseiten(B, bis, dreh), f"bassseiten_{bis}_{dreh:g}")
