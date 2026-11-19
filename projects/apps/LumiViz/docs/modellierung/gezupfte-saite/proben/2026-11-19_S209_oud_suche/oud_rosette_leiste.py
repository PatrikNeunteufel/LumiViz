"""S209 - Gegenprobe fuer die Vorlage: Rosettenmasse als weiche Leiste rund um die Loecher (heute im Bauplan moeglich)
gegen die genaue Massenaenderung im Modenraum. Fassung fein5; Ring 5 mm breit, Mitte 3 mm ausserhalb des Lochrands,
Hoehe 10 mm, Dichte so, dass die Masse 10,23 g (gross) und je 1 g (klein) ist; E 10 MPa."""
import copy
import math

import numpy as np

import oud_feinsuche4 as f4
import oud_kombi as k
import oud_suche as s

p0 = k.fassung(dicke=0.002061, abstand=0.002, steg=(0.17, 0.02004, 0.00644, 650.0, 10e9), volumen=0.02061, muendung=1.522)
f4.RG = 0.01023
for netz in (450, 2500, 5000):
    e = s.rechne(copy.deepcopy(p0), "ring_ohne", to=1000, netz=netz, zeigen=False)
    genau = f4.mit_massen(e, True, False)
    p = copy.deepcopy(p0)
    for name, x, y, r, m in (("rosette", 0.0, 0.30, 0.053, 0.01023), ("rosette l", -0.103, 0.204, 0.020, 0.001), ("rosette r", 0.103, 0.204, 0.020, 0.001)):
        rr, breite, hoehe = r + 0.003, 0.005, 0.010
        dichte = m / (2 * math.pi * rr * breite * hoehe)
        p["materials"].append({"name": name, "kind": "orthotropic", "density": dichte, "ex": 1e7, "ey": 1e7, "gxy": 4e6, "gxz": 4e6, "gyz": 4e6,
                               "nuxy": 0.3, "lossFactor": 0.0, "source": "probe"})
        n = 48
        linie = [[x + rr * math.cos(2 * math.pi * i / n), y + rr * math.sin(2 * math.pi * i / n)] for i in range(n + 1)]
        p["ribs"].append({"name": name, "material": name, "line": linie, "height": [hoehe] * (n + 1), "width": [breite] * (n + 1)})
    e2 = s.rechne(p, "ring_mit", to=1000, netz=netz, zeigen=False)
    print(f"Netz fuer {netz} Hz:")
    print("   ohne Rosetten:                 " + " / ".join(f"{v:6.1f}" for v in e["alle_hz"][:6]))
    print("   Massen im Modenraum (genau):   " + " / ".join(f"{v:6.1f}" for v in genau[:6]))
    if e2:
        print("   weiche Leisten um die Loecher: " + " / ".join(f"{v:6.1f}" for v in e2["alle_hz"][:6]))
