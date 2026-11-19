"""S209 - Gegenprobe zur Aufnehmermasse: Stoerungsrechnung aus den Formen (Masse 1 normiert).
df/f = -0,5 m phi(AP)^2; wirksame Masse der Mode am Antwortpunkt = 1 / phi(AP)^2. Dazu Z2 fuer drei Anregungen."""
import json
import sys

import numpy as np

import oud_suche as s

name = sys.argv[1]
knoten, dreiecke, hz, w = s.formen(s.HIER / f"{name}_formen.txt")
body = json.loads((s.HIER / f"{name}.lvinstbody").read_text(encoding="utf-8"))
eta = np.array([m["eta"] for m in body["modes"]])
nae = lambda x, y: int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))
ap, steg = nae(0.075, 0.135), nae(0.0, 0.09)
print(name)
for j in range(len(hz)):
    if hz[j] > 420:
        break
    print(f"  {hz[j]:6.1f} Hz: wirksame Masse am Antwortpunkt {1000 / w[j, ap] ** 2:8.1f} g, am Steg {1000 / max(w[j, steg] ** 2, 1e-12):8.1f} g; "
          f"5,8 g am Antwortpunkt: {-50 * 0.0058 * w[j, ap] ** 2:+.1f} %")
f = np.arange(60.0, 400.01, 0.25)
kern = -((2 * np.pi * f[:, None]) ** 2) / ((2 * np.pi * hz[None, :]) ** 2 * (1 + 1j * eta[None, :]) - (2 * np.pi * f[:, None]) ** 2)
raster = []
for xr in range(-15, 16, 5):
    for yr in range(0, 41, 5):
        x, y = xr / 100.0, 0.46 - yr / 100.0
        kn = nae(x, y)
        if np.hypot(knoten[kn, 0] - x, knoten[kn, 1] - y) < 0.012:
            raster.append((xr, kn))
for was, orte in (("Raster, alle Punkte", [kn for _, kn in raster]), ("Raster, nur Mittellinie x = 0", [kn for xr, kn in raster if xr == 0]),
                  ("Raster, nur die Seite des Aufnehmers x > 0", [kn for xr, kn in raster if xr > 0]), ("am Steg (Mitte)", [steg])):
    a = np.sqrt(np.mean([np.abs(kern @ (w[:, ap] * w[:, o])) ** 2 for o in orte], axis=0))
    sp = s.spitzen(f, a)
    um = [p for p in sp if 185 <= p[0] <= 225]
    an = [p for p in sp if not (185 <= p[0] <= 225)]
    z2 = 20 * np.log10(max(p[1] for p in um) / max(p[1] for p in an)) if um and an else float("nan")
    print(f"  Anregung {was} ({len(orte)}): Z2 {z2:+.1f} dB; " + " ".join(f"{p[0]:.0f}:{p[1] / max(q[1] for q in sp):.2f}" for p in sp))
print("  gemessen (Abb. 6): 101:0.18 139:0.51 203:1.00 280:0.43 362:0.37")
