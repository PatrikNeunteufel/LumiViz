"""S222, 34k-7d-1 — warum der Kandidat S200 bei kurzen Saiten schneller abklingt als der Bezug: die Abklingrate je Teilton am starren Steg aus dem Umlauf: -20 lg |E(w_n)| * f_n (dB/s), E = exp(-2 i k L) auf der
reellen Achse; dazu was das FIR des Kandidaten bei w_n traegt (Betrag seiner Antwort)."""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
spec = importlib.util.spec_from_file_location("bezug", ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps, kd = bz.ps, bz.kd
for name in sys.argv[1:]:
    fall = next(f for f in bz.faelle() if f[0] == name)
    saiten, laengen, *_ = bz.eingaben(fall[1], fall[2], fall[3])
    sa, L = saiten[0], laengen[0]
    h, mitte = kd.umlauf_fir(sa, L)
    f0 = 440.0 * 2 ** ((fall[2] - 69) / 12)
    print(f"{name}: L {L:.4f} m, FIR {len(h)} Werte, Spitze bei {mitte}")
    for n in range(1, 5):
        w = np.array([2 * np.pi * n * f0 + 0j])
        k, _ = ps.saite_wellen(sa, w)
        E = np.exp(-2j * k * L)[0]
        H = np.sum(h * np.exp(-1j * w.real[0] / ps.FS * np.arange(len(h))))
        print(f"  n {n}: |E| {abs(E):.6f} -> {-20 * np.log10(abs(E)) * n * f0:7.2f} dB/s je Umlauf-Takt n f0;  |FIR| {abs(H):.6f} -> "
              f"{-20 * np.log10(abs(H)) * n * f0:7.2f} dB/s")
