"""S209 - Stoerungsrechnung: Masse am Rand der Loecher (Rosetten, Abdallah 2026 Abb. 4 / 5 a zeigen sie in allen drei
Loechern) - um wie viel senkt sie jede Mode? df/f = -0,5 m <phi^2 am Lochrand>. Formen Masse 1 normiert."""
import sys

import numpy as np

import oud_suche as s

name = sys.argv[1]
knoten, dreiecke, hz, w = s.formen(s.HIER / f"{name}_formen.txt")


def rand(x, y, r):
    d = np.hypot(knoten[:, 0] - x, knoten[:, 1] - y)
    return np.where(np.abs(d - r) < 0.004)[0]


gross = rand(0.0, 0.30, 0.053)
klein = np.concatenate([rand(-0.103, 0.204, 0.020), rand(0.103, 0.204, 0.020)])
print(name, f"- Knoten am Rand: grosses Loch {len(gross)}, kleine Loecher {len(klein)}")
for j in range(len(hz)):
    if hz[j] > 420:
        break
    g, kl = float(np.mean(w[j, gross] ** 2)), float(np.mean(w[j, klein] ** 2))
    print(f"  {hz[j]:6.1f} Hz: je 1 g am grossen Loch {-50 * 0.001 * g:+.2f} %, je 1 g an den kleinen (zusammen) {-50 * 0.001 * kl:+.2f} %"
          f"   -> 7 g + 2 g: {-50 * (0.007 * g + 0.002 * kl):+.1f} %")
