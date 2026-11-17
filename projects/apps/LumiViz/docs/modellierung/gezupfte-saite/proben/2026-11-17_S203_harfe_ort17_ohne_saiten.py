"""S203: Harfe ohne Saiten an Bells Ort 17 (Mittelleiste 80 cm, zwischen den Oesen der Saiten 20 und 21): Koerperdatei
gegen Abb. 7.6 „ohne Saiten“ (digitalisiert in Bericht 22, +-2 dB, 116 dB = 1 s/kg). Aufruf: python -X utf8 -B harfe_ort17.py"""
import importlib.util
import pathlib

import numpy as np

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben")


def modul(datei, name):
    spec = importlib.util.spec_from_file_location(name, P / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


ps = modul("2026-11-15_S199_pruefstand_zupf.py", "pruefstand")
ha = modul("2026-11-14_S198_harfe_admittanz.py", "harfe_admittanz")
kp = ps.lade_koerper("concert_harp")
f, eta, steg = kp["f"], kp["eta"], kp["steg"]
wn2 = (2 * np.pi * f) ** 2
# Bericht 22 Tab. 1.2: f, lose, ohne Saiten, mit Saiten
BELL = [(140, 75.2, 78.6, 76.3), (170, 69.2, 69.8, 79.1), (200, 80.6, 78.6, 81.6), (230, 79.6, 84.7, 84.9), (260, 94.0, 88.5, 86.1),
        (290, 69.7, 67.6, 77.6), (320, 74.2, 83.5, 86.9), (350, 76.6, 78.3, 78.0), (380, 67.7, 64.9, 81.2), (410, 74.5, 69.2, 83.3),
        (440, 82.9, 80.2, 81.0), (470, 64.7, 71.4, 84.0), (500, 74.6, 73.6, 85.2), (530, 75.8, 78.3, 88.4), (560, 82.4, 85.1, 82.8),
        (590, 68.6, 77.7, 84.5), (620, 69.2, 68.9, 86.2), (650, 73.9, 73.3, 87.0), (680, 75.7, 74.6, 89.1), (710, 78.8, 77.6, 88.1),
        (740, 76.6, 77.4, 89.1), (770, 79.5, 76.9, 89.7), (790, 75.6, 77.4, 90.3)]
punkte = {}
for nr, hz, *_ in ha.NYLON_DARM:
    if nr in ("20", "21"):
        punkte[nr] = kp["tasten"].index(int(round(69 + 12 * np.log2(hz / 440.0))) + 1)


def Y(p, hz):
    w = 2 * np.pi * hz
    return 1j * w * np.sum(steg[:, p] ** 2 / (wn2 * (1 + 1j * eta) - w * w))


db = lambda y: 20 * np.log10(abs(y)) + 116
print("f Hz | Bell ohne Saiten | Modell Oese 21 / 20 (dB re Bell)")
mod, bell = [], []
for hz, _, ohne, _ in BELL:
    a, b = db(Y(punkte["21"], hz)), db(Y(punkte["20"], hz))
    mod.append(10 * np.log10((10 ** (a / 10) + 10 ** (b / 10)) / 2)); bell.append(ohne)
    print(f"{hz:4d} | {ohne:5.1f} | {a:5.1f} {b:5.1f}")
mod, bell = np.array(mod), np.array(bell)
fr = np.array([z[0] for z in BELL])
e = lambda v: 10 * np.log10(np.mean(10 ** (v / 10)))
for lo, hi in ((140, 200), (200, 400), (400, 790)):
    m = (fr >= lo) & (fr <= hi)
    print(f"Band {lo}-{hi} Hz energetisch: Bell {e(bell[m]):5.1f}, Modell {e(mod[m]):5.1f}, Abstand {e(mod[m]) - e(bell[m]):+5.1f} dB")
print(f"Steigung 200-790 Hz (dB/Okt.): Bell {np.polyfit(np.log2(fr[fr >= 200]), bell[fr >= 200], 1)[0]:+.1f}, "
      f"Modell {np.polyfit(np.log2(fr[fr >= 200]), mod[fr >= 200], 1)[0]:+.1f}")
fein = np.linspace(140, 790, 2000)
yf = np.array([db(Y(punkte["21"], h)) for h in fein])
print(f"Modell Oese 21 fein 140-790 Hz: Median {np.median(yf):.1f}, 10./90. Perzentil {np.percentile(yf, 10):.1f} / {np.percentile(yf, 90):.1f} dB")
