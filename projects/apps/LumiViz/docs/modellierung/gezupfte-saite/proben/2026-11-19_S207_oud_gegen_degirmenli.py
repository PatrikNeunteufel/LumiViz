"""S207 - Abstrahlung der Oud: Modell gegen Degirmenli 2017 (GU J Sci Part C 5(2), Abb. 7: sieben tuerkische Ouds,
Schalldruck je Kraft in 1 m vor der Decke, Hammer am Steg an Bass, Mitte und Diskant, gemittelt, Terzglaettung,
dB re 20 uPa/N). Modell: Pruefkoerper mit einem Hoerort 1 m vor der Mitte der Decke, |p/F|^2 gemittelt ueber die
Stegpunkte der Choere 1, 3/4 und 6, Terzglaettung (Leistung).

Die Werte der Messung sind in S207 am Seitenbild (300 dpi, S. 29) abgelesen: Achse 0,895 px/Hz und 15 px/dB im
dreifach vergroesserten Ausschnitt, also auf rund +-10 Hz und +-1,5 dB; die Spanne ist die der sieben Kurven.
Das Modell ist eine arabische Oud (Luftmode 100 Hz), die Messung tuerkische (erste Spitze 123 Hz): verglichen wird
deshalb auch gegen die Lage der eigenen Luftmode.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <ordner mit oud.lvinstbody, erster Hoerort 1 m>
"""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
ps.KOERPER = pathlib.Path(sys.argv[1])

# abgelesen: Stelle, Hz, dB von - bis ueber die sieben Ouds
MESSUNG = [("unter der Luftmode, 0,65 f_Luft", 80, 53, 59), ("Luftmode (erste Spitze)", 123, 81, 87), ("Tal dahinter", 145, 70, 72),
           ("Hauptspitze", 205, 93, 101), ("Plateau 300-450 Hz", 375, 84, 92)]

kp = ps.lade_koerper("oud")
print("Hoerort:", kp["orte"][0])
hz = np.arange(20.0, 1200.0, 0.5)
punkte = [kp["tasten"].index(t) for t in (36, 50, 60)]
p2 = np.mean([np.abs(ps.koerper_kurven(kp, p, 2 * np.pi * hz + 0j)[1][:, 0]) ** 2 for p in punkte], axis=0)
c = np.concatenate([[0.0], np.cumsum(p2)])
lo = np.searchsorted(hz, hz / 2 ** (1 / 6))
hi = np.maximum(np.searchsorted(hz, hz * 2 ** (1 / 6)), lo + 1)
db = 10 * np.log10((c[hi] - c[lo]) / (hi - lo)) - 20 * np.log10(20e-6)
roh = 10 * np.log10(p2) - 20 * np.log10(20e-6)


def bei(f):
    return float(db[int(np.argmin(np.abs(hz - f)))])


sel = (hz > 60) & (hz < 160)
f_luft = float(hz[sel][np.argmax(db[sel])])
sel = (hz > 160) & (hz < 330)
f_haupt = float(hz[sel][np.argmax(db[sel])])
sel = (hz > f_luft) & (hz < f_haupt)
f_tal = float(hz[sel][np.argmin(db[sel])])
plateau = float(10 * np.log10(np.mean(10 ** (db[(hz > 1.45 * f_haupt) & (hz < 2.2 * f_haupt)] / 10))))
modell = [(0.65 * f_luft, bei(0.65 * f_luft)), (f_luft, bei(f_luft)), (f_tal, bei(f_tal)), (f_haupt, bei(f_haupt)), (1.8 * f_haupt, plateau)]
haupt_m = 0.5 * (MESSUNG[3][2] + MESSUNG[3][3])
print("Stelle                           | Messung Hz, dB (sieben Ouds), gegen die Hauptspitze | Modell Hz, dB, gegen die Hauptspitze")
for (name, f, a, b), (fm, wert) in zip(MESSUNG, modell):
    print(f"{name:32s} | {f:4d}  {a}-{b}  {0.5 * (a + b) - haupt_m:+6.1f} | {fm:6.1f}  {wert:5.1f}  {wert - modell[3][1]:+6.1f}")
print("\nModell, Terzglaettung, dB re 20 uPa/N in 1 m (in Klammern ungeglaettet):")
for f in (50, 66, 82, 100, 115, 131, 147, 165, 185, 208, 233, 262, 294, 330, 370, 415, 466, 523, 700, 1000):
    print(f"  {f:5d} Hz  {bei(f):5.1f}  ({float(roh[int(np.argmin(np.abs(hz - f)))]):5.1f})")
