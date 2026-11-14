"""Frequenzgang der Punktadmittanz an Messorten (Bell-Pegel, 116 dB = 1 s/kg): Spitzen und Taeler. gang.py <koerper> <plan> ort ..."""
import json
import sys

import numpy as np

L = 1.31
k = json.load(open(sys.argv[1], encoding="utf-8"))
plan = json.load(open(sys.argv[2], encoding="utf-8"))
ys = np.array([p["y"] for p in plan["bridgePoints"]])
hz = np.array([m["f"] for m in k["modes"]])
eta = np.array([m["eta"] for m in k["modes"]])
phi = np.array([m["bridge"] for m in k["modes"]])
if phi.ndim == 3:
    phi = phi[:, :, 0]
fs = np.arange(150.0, 1000.0, 1.0)
for ort in [int(v) for v in sys.argv[3:]]:
    y = L * (40 - ort + 0.5) / 40
    j = int(np.argmin(np.abs(ys - y)))
    om = 2 * np.pi * fs[:, None]
    omn = 2 * np.pi * hz[None, :]
    p = 20 * np.log10(np.abs((1j * om / (omn ** 2 * (1 + 1j * eta[None, :]) - om ** 2) * phi[None, :, j] ** 2).sum(axis=1))) + 116
    teile = []
    for i in range(1, len(fs) - 1):
        if p[i] > p[i - 1] and p[i] >= p[i + 1]:
            teile.append(f"^{fs[i]:.0f}:{p[i]:.0f}")
        if p[i] < p[i - 1] and p[i] <= p[i + 1]:
            teile.append(f"v{fs[i]:.0f}:{p[i]:.0f}")
    print(f"Ort {ort} (Oese bei y {ys[j]:.3f}, Soll {y:.3f}): " + "  ".join(teile))
    print("   Raster: " + "  ".join(f"{f:.0f}:{p[int(f - 150)]:.0f}" for f in range(200, 1000, 50)))
