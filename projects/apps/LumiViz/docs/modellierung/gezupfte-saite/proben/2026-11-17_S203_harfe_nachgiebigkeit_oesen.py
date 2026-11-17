"""S203: Harfe — fehlt der Koerperdatei die Nachgiebigkeit der hohen Moden? Je Saite: Beitrag jeder Oktave der Moden zur
statischen Nachgiebigkeit phi^2/w_n^2, und |Y| bei der Saitenfrequenz aus allen Moden gegen den Federteil der Moden ueber
2 f (i w Summe phi^2 / w_n^2). Aufruf: python -X utf8 -B harfe_rest.py"""
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
print(f"{len(f)} Moden, {f.min():.1f} … {f.max():.0f} Hz")
BELL = {33: 80.3, 26: 86.1, 21: 94.8, 17: 91.2, 13: 99.2, 10: 99.2}
grenzen = [0, 250, 500, 1000, 2000, 4000, 8000, 16000, 40000]
for nr, hz, *_ in ha.NYLON_DARM + ha.METALL:
    if nr == "00" or int(nr) not in BELL:
        continue
    taste = int(round(69 + 12 * np.log2(hz / 440.0))) + 1
    p = kp["tasten"].index(taste)
    phi2 = steg[:, p] ** 2
    beitrag = phi2 / wn2
    teile = [beitrag[(f >= a) & (f < b)].sum() for a, b in zip(grenzen[:-1], grenzen[1:])]
    ges = beitrag.sum()
    w = 2 * np.pi * hz
    Y = 1j * w * np.sum(phi2 / (wn2 * (1 + 1j * eta) - w * w))
    hoch = f > 2 * hz
    Yfeder = w * beitrag[hoch].sum()
    db = lambda y: 20 * np.log10(abs(y)) + 116
    print(f"Saite {nr:>2} {hz:5.0f} Hz | Anteile je Band an phi²/w² (%): "
          + " ".join(f"{100 * t / ges:5.1f}" for t in teile)
          + f" | Y {db(Y):5.1f} dB, Federteil > 2f {db(Yfeder):5.1f} dB, Bell {BELL[int(nr)]:5.1f}")
print("Baender:", " | ".join(f"{a}-{b}" for a, b in zip(grenzen[:-1], grenzen[1:])), "Hz")
# Dichte und mittleres phi^2 je Oktave an Saite 13 (Ausblick auf den Rest ueber dem letzten Mode)
p = kp["tasten"].index(int(round(69 + 12 * np.log2(740 / 440.0))) + 1)
for a, b in zip(grenzen[1:-1], grenzen[2:]):
    m = (f >= a) & (f < b)
    if m.any():
        print(f"  Saite 13, {a}-{b} Hz: {m.sum():4d} Moden, mittleres phi² {np.mean(steg[m, p] ** 2):.3e}")
