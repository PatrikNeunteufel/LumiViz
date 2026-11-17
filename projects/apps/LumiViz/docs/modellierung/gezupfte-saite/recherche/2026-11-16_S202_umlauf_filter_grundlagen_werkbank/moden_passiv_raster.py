"""LVR Auftrag 11, Punkt 4: Passivitaet je Mode ueber ein Raster (f_n, Q): min Re Y_d(e^{iwT}) auf 0..fs/2, relativ zur Spitze
von Re Y; dazu der Durchgriff. Negativ = nicht passiv. Aufruf: python -B moden_passiv_raster.py"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np

spec = importlib.util.spec_from_file_location("md", pathlib.Path(__file__).resolve().parent / "moden_diskret.py")
md = importlib.util.module_from_spec(spec)
import io, contextlib
with contextlib.redirect_stdout(io.StringIO()):
    spec.loader.exec_module(md)

print("Q    | f_n kHz:  " + "  ".join(f"{x:>8.0f}" for x in (100, 1000, 3000, 6000, 10000, 14000, 18000, 22000)))
for art in ("ii", "trapez", "foh", "bilin"):
    print(art)
    for Q in (0.6, 1.0, 1.5, 3.0, 10.0, 100.0):
        werte = []
        for fn in (100, 1000, 3000, 6000, 10000, 14000, 18000, 22000):
            if Q < 0.5:
                continue
            Y, d0 = md.diskret(float(fn), Q, art)
            Yc = md.kont(float(fn), Q)
            werte.append(Y.real.min() / Yc.real.max())
        print(f"  {Q:5.1f} | " + "  ".join(f"{v:+.1e}" for v in werte))
# Hoehe der Spitze foh gegen sinc^2(f/fs)
for fn in (5000, 8000, 12000, 16000):
    print(f"foh: sinc^2(f_n/fs) bei {fn} Hz = {20 * np.log10(np.sinc(fn / md.FS) ** 2):+.2f} dB")
