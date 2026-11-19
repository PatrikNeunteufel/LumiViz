"""S209 - Kombinationen: Balkenenden (Abstand zum Rand), Halt am Rand, Deckendicke, Steg."""
import copy
import sys

import numpy as np

import oud_suche as s

B = s.basis()


def halbe_breite(plan, y):
    u = plan["plate"]["outline"]
    xs = []
    for a, b in zip(u, u[1:] + u[:1]):
        if (a[1] - y) * (b[1] - y) <= 0 and a[1] != b[1]:
            t = (y - a[1]) / (b[1] - a[1])
            xs.append(a[0] + t * (b[0] - a[0]))
    return max(xs)


def fassung(dicke=0.0015, abstand=None, halt="clamped", feder=0.0, steg=None, volumen=None, muendung=None, balken=None):
    """abstand: None = wie gezeichnet, sonst Abstand der Balkenenden zum Rand in m (alle durchgehenden Balken)."""
    p = copy.deepcopy(B)
    p["plate"]["thickness"] = dicke
    p["rim"]["support"] = halt
    p["rim"]["rotationalStiffness"] = feder
    if abstand is not None:
        for r in p["ribs"]:
            y = r["line"][0][1]
            hb = halbe_breite(p, y) - abstand
            if r["name"] == "brace 5":
                r["line"][0][0] = -hb
            elif r["name"] == "brace 4":
                r["line"][2][0] = hb
            else:
                r["line"][0][0], r["line"][2][0] = -hb, hb
            r["line"][1][0] = 0.5 * (r["line"][0][0] + r["line"][2][0])
    if balken:
        for r in p["ribs"]:
            if r["name"] in balken:
                r["height"] = [h * balken[r["name"]] for h in r["height"]]
    if steg:
        laenge, breite, hoehe, dichte, e = steg
        p["materials"].append({"name": "bridgewood", "kind": "orthotropic", "density": dichte, "ex": e, "ey": e, "gxy": e / 2.6,
                               "gxz": e / 2.6, "gyz": e / 2.6, "nuxy": 0.3, "lossFactor": 0.0, "source": "probe"})
        p["bridges"] = [{"name": "bridge", "material": "bridgewood", "line": [[-laenge / 2, 0.09], [0.0, 0.09], [laenge / 2, 0.09]],
                         "height": [hoehe] * 3, "width": [breite] * 3}]
    if volumen:
        p["cavity"]["volume"] = volumen
    if muendung:
        p["cavity"]["endCorrection"] = muendung
    return p


if __name__ == "__main__":
    g = sys.argv[1]
    if g == "rand":
        for ab in (0.002,):
            for halt, feder in (("clamped", 0.0), ("elastic", 0.0), ("elastic", 100.0), ("elastic", 1000.0)):
                for d in (0.0015, 0.0020):
                    s.rechne(fassung(dicke=d, abstand=ab, halt=halt, feder=feder), f"ab{ab * 1000:.0f}_{halt}{feder:.0f}_d{d * 1000:.2f}")
    if g == "abstand":
        for ab in (0.003, 0.004, 0.005, 0.006, 0.008):
            for d in (0.0015, 0.0018):
                s.rechne(fassung(dicke=d, abstand=ab), f"ab{ab * 1000:.0f}_d{d * 1000:.2f}")
    if g == "steg":
        for st in ((0.17, 0.015, 0.010, 650.0, 11e9), (0.17, 0.015, 0.010, 650.0, 1e9), (0.17, 0.020, 0.010, 900.0, 11e9)):
            s.rechne(fassung(steg=st), f"steg_{st[1] * 1000:.0f}x{st[2] * 1000:.0f}_{st[3]:.0f}_{st[4] / 1e9:.0f}GPa")
