"""S208 - |p/F| an der Oese jeder Saite bei der Frequenz der leeren Saite, Pruefkoerper mit 16 Hoerorten (Leistungsmittel)
und Ort `front`, gegen Bell 1988 Abb. 7.23 (besaitete Harfe, Schalldruck an den Saitenorten und -frequenzen; PDF 309,
in S208 am Seitenbild von Auge gelesen, +-1 dB; Bezug nicht genannt - verglichen wird gegen Saite 26).
Dazu Le Carrou, Gautier & Foltete 2007 Abb. 7 (Camac, |p/F| an Punkt C in 2,35 m, lineare Achse, am Bild gelesen).

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner mit concert_harp.lvinstbody>
Der Ordner wie in `2026-11-19_S208_harfe_mitschwingen_16.py` (Pruefkoerper mit 16 Hoerorten).
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

BELL_723 = {33: -26, 32: -24, 31: -31, 30: -32, 29: -34, 28: -28.5, 27: -10, 26: -8, 25: -16, 24: -12, 23: -5.5, 22: -8,
            21: -5, 20: -10.5, 19: -12, 18: -17, 17: -15.5, 16: -18, 15: -24, 14: -13, 13: -20, 12: -17, 11: -10.5, 10: -17.5}
LECARROU = [(80, 1.5), (100, 0.4), (110, 0.3), (120, 1.0), (125, 0.3), (130, 1.5), (140, 4.0), (150, 13.0), (157, 4.3),
            (165, 13.0), (172, 15.0), (180, 14.0), (186, 15.0), (195, 20.7), (198, 19.5), (204, 27.0), (215, 12.0), (225, 6.0),
            (240, 4.0), (250, 3.7), (257, 3.8), (280, 2.3), (290, 2.0)]

kp = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp)
alle = ps.HARFE_MONO + ps.HARFE_METALL


def pf(punkt, hz):
    w = np.array([2 * np.pi * hz + 0j])
    _, H = ps.koerper_matrix(kp, w, tan, [punkt])
    h = np.abs(H[0, 0, :]) ** 2
    return 10 * np.log10(np.mean(h[1:])), 10 * np.log10(h[0])


print("Saite  Hz   | Bell 7.23 gegen Saite 26 | Modell 16 Orte gegen Saite 26 | front gegen Saite 26 | Modell - Bell")
ref = None
zeilen = []
for n in sorted(BELL_723, reverse=True):
    hz = alle[n + 1][0]
    t = int(round(69 + 12 * np.log2(hz / 440.0)))
    zeilen.append((n, hz, BELL_723[n], *pf(kp["tasten"].index(t + 1), hz)))
r = next(z for z in zeilen if z[0] == 26)
for n, hz, b, m16, mf in zeilen:
    print(f"{n:4d} {hz:6.0f} | {b - r[2]:+6.1f} | {m16 - r[3]:+6.1f} | {mf - r[4]:+6.1f} | {m16 - r[3] - (b - r[2]):+6.1f}")
d = np.array([z[3] - r[3] - (z[2] - r[2]) for z in zeilen])
print(f"Modell - Bell: Median {np.median(d):+.1f} dB, Streuung {np.std(d):.1f} dB; Saiten 33-28 {np.mean(d[:6]):+.1f}, 27-10 {np.mean(d[6:]):+.1f}")

print("\nLe Carrou 2007 Abb. 7 gegen 204 Hz | Modell (16 Orte, Oese der Saite 21 = Mitte des Bodens) gegen 204 Hz")
punkt = kp["tasten"].index(65)
m0 = pf(punkt, 204)[0]
for hz, v in LECARROU:
    print(f"{hz:4d} Hz  {20 * np.log10(v / 27.0):+6.1f}  {pf(punkt, hz)[0] - m0:+6.1f}")
