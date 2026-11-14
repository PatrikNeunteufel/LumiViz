"""Probe: Rand gestuetzt (harte Wegfeder), die Diskantkante (y = L) mit eigener Wegfeder kwe (0 = frei).
Messorte nach Bell Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m. Hoehe mit Q 40. Aufruf: diskantkante.py <proben> <basis_frei.txt> kwe ..."""
import importlib.util
import os
import sys

import numpy as np


def lade(ordner, datei):
    spec = importlib.util.spec_from_file_location("m", os.path.join(ordner, datei))
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


rf = lade(sys.argv[1], "2026-11-13_S198_harfe_rand_feder.py")
L = 1.31
knoten, hz, phi = rf.hm.formen(sys.argv[2])
dreiecke = rf.dreiecke_aus(sys.argv[2])
masse = rf.massen(knoten, dreiecke)
st = rf.starr(knoten, masse, phi)
hz = np.concatenate([np.zeros(3), hz])
phi = np.vstack([st, phi])
m = phi.shape[0]
seiten, ende = np.zeros((m, m)), np.zeros((m, m))
for (a, b), t in rf.randkanten(dreiecke):
    pa, pb = knoten[a], knoten[b]
    lang = np.linalg.norm(pb - pa)
    wa, wb = phi[:, a], phi[:, b]
    teil = lang / 6.0 * (2 * np.outer(wa, wa) + np.outer(wa, wb) + np.outer(wb, wa) + 2 * np.outer(wb, wb))
    if min(pa[1], pb[1]) > L - 1e-6:
        ende += teil
    else:
        seiten += teil
orte = np.arange(1, 39)
idx = [int(np.argmin(knoten[:, 0] ** 2 + (knoten[:, 1] - (0.035 * (40 - o) - 0.057)) ** 2)) for o in orte]
for kwe in [float(v) for v in sys.argv[3:]]:
    wert, vek = np.linalg.eigh(np.diag((2 * np.pi * hz) ** 2) + 1e10 * seiten + kwe * ende)
    f = np.sqrt(np.maximum(wert, 0.0)) / (2 * np.pi)
    print(f"Diskantkante kw = {kwe:.1e} N/m2")
    for n in range(len(f)):
        if f[n] < 150 or f[n] > 1000:
            continue
        w = vek[:, n] @ phi
        p = np.abs(w[idx])
        mm, nn = rf.indizes(knoten, w)
        if mm != 0 or p.max() < 0.3 * np.abs(w).max():
            continue
        j = int(np.argmax(p))
        db = lambda v: 20 * np.log10(v ** 2 * 40 / (2 * np.pi * f[n]) + 1e-30) + 116
        print(f"   {f[n]:6.0f} Hz  Bauch Ort {orte[j]:2d}  Hoehe {db(p[j]):6.1f} dB (Q 40); an Ort 2 / 5 / 10: {db(p[1]):.0f} / {db(p[4]):.0f} / {db(p[9]):.0f}")
