"""S203 (22)(e): Form der Lautheit je Taste — Modell (Pruefstand nach KQ28 a, Hoerorte 10 cm ueber der Decke) gegen die
TU-Gitarre (ff, reflexionsarm, 1,05 m, Leistung ueber 32 Mikrofone); je Seite das Mittel abgezogen."""
import csv
import pathlib
import re
import sys

import numpy as np

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben")
modell = {}
for z in (P / "2026-11-16_S201_pruefstand_gitarre_je_taste_kq28.txt").read_text(encoding="utf-8").splitlines():
    m = re.match(r"Taste\s+(\d+).*?(-?\d+\.\d+) LUFS", z)
    if m:
        modell.setdefault(int(m.group(1)), float(m.group(2)))
tu = {int(r["midi"]): float(r["L_summe"]) for r in csv.DictReader(open(P / "2026-11-17_S203_kq33_tu_gitarre.tsv", encoding="utf-8"),
                                                                    delimiter="\t") if "dynamic = ff" in r["beschreibung"]}
tasten = sorted(set(modell) & set(tu))
a = np.array([modell[t] for t in tasten]); b = np.array([tu[t] for t in tasten])
a -= a.mean(); b -= b.mean()
print(f"{len(tasten)} gemeinsame Tasten {tasten[0]}-{tasten[-1]}; Korrelation {np.corrcoef(a, b)[0, 1]:.2f}; "
      f"Abstand Modell - TU: Effektivwert {np.sqrt(np.mean((a - b) ** 2)):.1f} LU")
for lo, hi, name in ((40, 47, "E2-B2 (um A0 98 Hz)"), (48, 55, "C3-G3"), (56, 59, "G#3-B3 (um T1 200-225 Hz)"),
                     (60, 71, "C4-B4"), (72, 83, "C5-B5")):
    s = [i for i, t in enumerate(tasten) if lo <= t <= hi]
    print(f"  {name:28s}: Modell {a[s].mean():+5.1f}, TU {b[s].mean():+5.1f}, Modell - TU {a[s].mean() - b[s].mean():+5.1f} LU")
