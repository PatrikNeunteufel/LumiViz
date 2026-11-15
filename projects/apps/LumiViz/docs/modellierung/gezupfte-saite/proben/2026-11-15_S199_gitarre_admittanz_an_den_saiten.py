"""S199, 34k-6 — die Punktadmittanz des Gitarrenkoerpers an den sechs Saiten je Terzband, gegen Scherrer 2013 Abb. A.11.

Aufruf: python -B <dieses Skript> [<qualified_selected_impulses.mat>]

Mit der Messdatei: dazu der Pegel der gemessenen Uebertragung (Schlagort -> Aufnehmer, Mittel der sechs Paare) als
Mobilitaet je Terzband, und der Koerper nach `teile_neu` (Pruefstand): Formwert und Abstrahlgewicht je Terzband ab
2 kHz so geteilt, dass die Punktadmittanz im Mittel der Saiten 3 dB ueber der gemessenen Uebertragung liegt.

Scherrer 2013 (Diss. McGill, Anhang A, Abb. A.11, am Bild gelesen, +-3 dB): eine andere klassische Gitarre, Yzz
(senkrecht zur Decke) am Ort der Saiten I und IV: Spitzen -15 dB bei rund 200 und 250 Hz, -20 ... -22 dB bei 520 Hz,
ueber 2 kHz ein Band um -45 ... -50 dB re 1 m/s/N bis 10 kHz. Yyy (parallel): unter 500 Hz -45 ... -60 dB ohne die
tiefen Spitzen, ueber 1 kHz -50 ... -55 dB; Yzy unter 500 Hz -55 ... -70 dB (Spitze -27 dB bei 250 Hz an Saite I),
darueber -50 ... -60 dB.
"""
import importlib.util
import pathlib

import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.with_name("2026-11-15_S199_pruefstand_zupf.py"))
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

if __name__ == "__main__":
    kp = ps.lade_koerper("classical_guitar")
    hz = np.arange(60.0, 20000.0, 1.0)
    mitten = [100 * 2 ** (k / 3) for k in range(0, 23)]
    print("Terzband Hz  " + " ".join(f"{m:6.0f}" for m in mitten))
    for s in range(6):
        Y, _ = ps.koerper_kurven(kp, s, 2 * np.pi * hz + 0j)
        db = [10 * np.log10(np.mean(np.abs(Y[(hz >= m / 2 ** (1 / 6)) & (hz < m * 2 ** (1 / 6))]) ** 2)) for m in mitten]
        print(f"Saite {6 - s}      " + " ".join(f"{v:6.1f}" for v in db))
    if len(sys.argv) > 1:
        import scipy.io as sio
        spec2 = importlib.util.spec_from_file_location("mg", HIER.parents[7] / "asset/loop/instrument_bodies/make_gitarre.py")
        mg = importlib.util.module_from_spec(spec2)
        spec2.loader.exec_module(mg)
        d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"][mg.GITARRE - 1]
        fh, H = mg.frfs({ort: d[o * mg.FS:(o + 1) * mg.FS, :] for o, ort in enumerate(mg.ORTE)})
        mob = np.mean([np.abs(H[p][1:] / (2 * np.pi * fh[1:])) ** 2 for p in mg.PAARE], axis=0)
        mess = [10 * np.log10(np.mean(mob[(fh[1:] >= m / 2 ** (1 / 6)) & (fh[1:] < m * 2 ** (1 / 6))])) for m in mitten]
        print("gemessen     " + " ".join(f"{v:6.1f}" for v in mess) + "   (Uebertragung, ab 12,7 kHz Rauschen)")
        ps.teile_neu(kp)
        for s in range(6):
            Y, _ = ps.koerper_kurven(kp, s, 2 * np.pi * hz + 0j)
            db = [10 * np.log10(np.mean(np.abs(Y[(hz >= m / 2 ** (1 / 6)) & (hz < m * 2 ** (1 / 6))]) ** 2)) for m in mitten]
            print(f"neu Saite {6 - s}  " + " ".join(f"{v:6.1f}" for v in db))
