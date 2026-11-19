"""S207 - Harfe: haengt die Lautheit der Tasten, deren Grundton auf einer Koerpermode liegt (58: 233 Hz, 64: 329 Hz, 80: 841 Hz),
an der Guete der Mode? Lautheit am Hoerort fuer einige Tasten mit eta mal 1, 2, 4 (alle Moden) und mit der Mode um
+-3 % verschoben.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>
"""
import importlib.util
import pathlib

import numpy as np

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

kp0 = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp0)
w = ps.frequenzen()
TASTEN = (58, 60, 62, 64, 67, 78, 80)


def lauf(kp):
    aus = []
    for taste in TASTEN:
        sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(taste)])
        x = ps.zeit(ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
        aus.append((ps.momentary_max(x), ps.abfall(x)[0]))
    return aus


print("Tasten: " + "  ".join(f"{t:>12d}" for t in TASTEN) + "   (Lautheit LUFS / Abfall frueh dB/s)")
for name, faktor, schub in (("wie Datei", 1.0, 1.0), ("eta x 2", 2.0, 1.0), ("eta x 4", 4.0, 1.0), ("Moden +3 %", 1.0, 1.03), ("Moden -3 %", 1.0, 0.97)):
    kp = dict(kp0)
    kp["eta"] = kp0["eta"] * faktor
    kp["f"] = kp0["f"] * schub
    r = lauf(kp)
    print(f"{name:11s}: " + "  ".join(f"{a:6.1f}/{b:5.1f}" for a, b in r), flush=True)
