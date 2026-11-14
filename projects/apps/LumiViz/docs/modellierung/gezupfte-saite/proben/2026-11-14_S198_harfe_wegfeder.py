"""Probe: Wegfeder am Rand (Basis Rand frei) - Lage, Ort des staerksten Bauchs und Hoehe (Q fest 40) je Mode der Mitte.
Aufruf: wegfeder.py <proben> <basis_frei.txt> kw ..."""
import importlib.util
import os
import sys

import numpy as np


def lade(ordner, datei):
    spec = importlib.util.spec_from_file_location("m", os.path.join(ordner, datei))
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


proben = sys.argv[1]
rf = lade(proben, "2026-11-13_S198_harfe_rand_feder.py")
hm = rf.hm
L = 1.31
knoten, hz, phi = hm.formen(sys.argv[2])
dreiecke = rf.dreiecke_aus(sys.argv[2])
masse = rf.massen(knoten, dreiecke)
st = rf.starr(knoten, masse, phi)
hz = np.concatenate([np.zeros(3), hz])
phi = np.vstack([st, phi])
weg, dreh = rf.rand_matrizen(knoten, dreiecke, phi, "gleich")
orte = np.arange(1, 41)
idx = [int(np.argmin(knoten[:, 0] ** 2 + (knoten[:, 1] - L * (40 - o + 0.5) / 40) ** 2)) for o in orte]
print(f"Basis: {len(hz)} Formen bis {hz[-1]:.0f} Hz")
for kw in [float(v) for v in sys.argv[3:]]:
    a = np.diag((2 * np.pi * hz) ** 2) + kw * weg
    wert, vek = np.linalg.eigh(a)
    f = np.sqrt(np.maximum(wert, 0.0)) / (2 * np.pi)
    print(f"kw = {kw:.1e} N/m2")
    for n in range(len(f)):
        if f[n] < 150 or f[n] > 1000:
            continue
        w = vek[:, n] @ phi
        p = np.abs(w[idx])
        if p.max() < 0.3 * np.abs(w).max():
            continue
        m, nn = rf.indizes(knoten, w)
        if m != 0:
            continue
        j = int(np.argmax(p))
        rand = np.abs(w[[k for (ab, t) in rf.randkanten(dreiecke) for k in ab]]).max()
        db = 20 * np.log10(p[j] ** 2 * 40 / (2 * np.pi * f[n])) + 116
        print(f"   {f[n]:6.0f} Hz  Bauch Ort {orte[j]:2d}  Hoehe {db:6.1f} dB (Q 40)  Wechsel laengs {nn}  Rand/Bauch {rand / p[j]:.2f}")
