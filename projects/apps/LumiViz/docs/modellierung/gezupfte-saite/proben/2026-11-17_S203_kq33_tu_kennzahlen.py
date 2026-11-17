"""S203 KQ33 a: Kennzahlen wie GZ14 a (Spanne, 10.-90. Perzentil, groesster / mittlerer Sprung ueber Paare bis 4 Halbtoene)
an der TU-Berlin-Datenbank, reflexionsarm. Eingabe: die TSV aus lautheit.m. Aufruf: python -I -B kennzahlen.py <ordner>"""
import csv
import pathlib
import sys

import numpy as np

ordner = pathlib.Path(sys.argv[1])


def kennzahlen(tasten, l):
    l = np.asarray(l)
    d = np.array([abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= 4])
    return l.max() - l.min(), np.percentile(l, 90) - np.percentile(l, 10), d.max(), np.median(d), len(l)


for name in ("Acoustic_guitar_modern", "Double_action_harp_modern"):
    zeilen = list(csv.DictReader(open(ordner / f"{name}.tsv", encoding="utf-8"), delimiter="\t"))
    for staerke in ("pp", "ff"):
        z = sorted((int(r["midi"]), float(r["L_summe"]), float(r["L_mik_std"])) for r in zeilen if f"dynamic = {staerke}" in r["beschreibung"])
        doppelt = len(z) - len({t for t, *_ in z})
        tasten = [t for t, *_ in z]
        for titel, ab in (("voll", 0), ("ab D2", 38)):
            if name.startswith("Acoustic") and ab:
                continue
            sel = [(t, l) for t, l, _ in z if t >= ab]
            sp, p, gr, mi, n = kennzahlen([t for t, _ in sel], [l for _, l in sel])
            print(f"{name:26s} {staerke} {titel:6s} | {n:2d} Toene {min(tasten)}-{max(tasten)} (doppelt {doppelt}) | Spanne {sp:5.1f} LU, "
                  f"10.-90. {p:5.1f} LU, groesster Sprung {gr:4.1f} LU, mittlerer {mi:3.1f} LU | Streuung ueber Mikrofone im Median "
                  f"{np.median([s for *_, s in z]):.1f} dB")
