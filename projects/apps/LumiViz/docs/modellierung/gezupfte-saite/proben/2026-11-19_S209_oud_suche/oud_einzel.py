"""S209 - einzelne Fassungen mit Steg im Einzelnen; dazu die Kurve am Antwortpunkt fuer drei Anregungen."""
import numpy as np

import oud_kombi as k
import oud_suche as s

for name, kw in (
        ("steg_130x20x12_650_d1.6", dict(dicke=0.0016, steg=(0.13, 0.020, 0.012, 650.0, 10e9))),
        ("steg_130x25x8_650_d1.5", dict(dicke=0.0015, steg=(0.13, 0.025, 0.008, 650.0, 10e9))),
        ("steg_130x25x12_650_d1.6", dict(dicke=0.0016, steg=(0.13, 0.025, 0.012, 650.0, 10e9))),
        ("steg_130x25x12_650_d1.6_ab3", dict(dicke=0.0016, abstand=0.003, steg=(0.13, 0.025, 0.012, 650.0, 10e9))),
        ("steg_130x20x10_650_d1.5_gestuetzt", dict(dicke=0.0015, halt="elastic", steg=(0.13, 0.020, 0.010, 650.0, 10e9))),
):
    e = s.rechne(k.fassung(**kw), name)
    # Kurve am Antwortpunkt fuer Anregung nur auf der Mittellinie (x = 0) und am Steg
    knoten, dreiecke, hz, w = s.formen(s.HIER / f"{name}_formen.txt")
    import json
    body = json.loads((s.HIER / f"{name}.lvinstbody").read_text(encoding="utf-8"))
    eta = np.array([m["eta"] for m in body["modes"]])
    nae = lambda x, y: int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))
    ap = nae(0.075, 0.135)
    f = np.arange(60.0, 400.01, 0.25)
    kern = -((2 * np.pi * f[:, None]) ** 2) / ((2 * np.pi * hz[None, :]) ** 2 * (1 + 1j * eta[None, :]) - (2 * np.pi * f[:, None]) ** 2)
    for was, orte in (("Anregung am Steg (Mitte)", [nae(0.0, 0.09)]), ("Anregung Mittellinie y 6-41 cm", [nae(0.0, y / 100) for y in range(6, 24, 5)] + [nae(0.0, 0.41)]),
                      ("Anregung am Steg, Antwort am Steg", None)):
        if orte is None:
            a = np.abs(kern @ (w[:, nae(0.0, 0.09)] ** 2))
        else:
            a = np.sqrt(np.mean([np.abs(kern @ (w[:, ap] * w[:, o])) ** 2 for o in orte], axis=0))
        sp = s.spitzen(f, a)
        print(f"   {was}: " + " ".join(f"{p[0]:.0f}:{p[1] / max(q[1] for q in sp):.2f}" for p in sp))
print("gemessen (Abb. 6): 101:0.18 139:0.51 203:1.00 280:0.43 362:0.37")
