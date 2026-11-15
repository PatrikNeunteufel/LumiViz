"""S199, 34k-5 — Gegenprobe: der Modalfit der Gitarre (make_gitarre.py) bis 10 kHz statt 5 kHz.

Aufruf: python -B 2026-11-15_S199_mores_fit_bis_10khz.py <qualified_selected_impulses.mat> <ergebnis.npz>

Derselbe Weg wie make_gitarre.py mit BIS = 10000: Pole, Paare, freier Fit, Normalmoden mit Biegung. Meldet die Zahl der
Moden je Bereich, den Fehler ueber die sechs Stegkurven je Bereich (frei und gebunden), die Modenueberlappung
(Halbwertsbreite gegen Abstand) und legt f, s, a, b, c in die npz-Datei. Schreibt nichts ins Repo.
"""
import importlib.util
import pathlib
import sys
import time

import numpy as np
import scipy.io as sio

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("make_gitarre", HIER.parents[7] / "asset/loop/instrument_bodies/make_gitarre.py")
mg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mg)

if __name__ == "__main__":
    mg.BIS = 10000.0
    start = time.time()
    FS = mg.FS
    d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"][mg.GITARRE - 1]
    hz, H = mg.frfs({ort: d[o * FS:(o + 1) * FS, :] for o, ort in enumerate(mg.ORTE)})
    sel = (hz >= mg.VON) & (hz <= mg.BIS)
    w = 2 * np.pi * np.maximum(hz, 1e-9)
    acc = [H[p] for p in mg.PAARE]
    mob = [y / (1j * w) for y in acc]
    f, s = mg.pole(hz, acc, mob, sel)
    f, s = mg.lege_paare_zusammen(hz, mob, f, s, sel, [])
    print(f"Pole: {len(f)} nach {time.time() - start:.0f} s", flush=True)
    R, rest, fits_frei = mg.frei(hz, mob, f, s, sel)
    bereiche = ((60, 400), (400, 1000), (1000, 2500), (2500, 5000), (5000, 7000), (7000, 10000))
    print("Bereich Hz      Moden  Abstand Hz  Breite Hz  Ueberlappung  Fehler frei dB")
    for lo, hi in bereiche:
        m = (f >= lo) & (f < hi)
        abstand = (hi - lo) / max(1, m.sum())
        breite = float(np.mean(s[m]) / np.pi) if m.any() else 0.0
        print(f"{lo:5d}-{hi:5d}   {int(m.sum()):4d}   {abstand:8.1f}   {breite:8.1f}   {breite / abstand:8.2f}      "
              f"{mg.fehler(hz, sel, mob, fits_frei, lo, hi):.2f}", flush=True)
    a, b, c, fits_steg = mg.stegfit(hz, mob, f, s, sel, R, rest)
    print("Fehler mit Normalmoden dB: " + " / ".join(f"{mg.fehler(hz, sel, mob, fits_steg, lo, hi):.2f}" for lo, hi in bereiche))
    np.savez(sys.argv[2], f=f, s=s, a=a, b=b, c=c)
    print(f"{time.time() - start:.0f} s")
