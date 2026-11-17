"""Oud: Basschor C2 einzeln gegen doppelt, und Varianten der umsponnenen Saite, im Pruefstand S199 (nur gelesen,
importiert ohne Bytecode). Rechnet nur die Tasten 36-40 (Chor 1) neu; jede Taste ist im Pruefstand unabhaengig, die
uebrigen Tasten bleiben wie in `2026-11-15_S199_pruefstand_oud_je_taste.txt`. Schreibt nichts.
Aufruf: python -B -I <dieses Skript>
"""
import importlib.util
import sys

import numpy as np

sys.dont_write_bytecode = True
PFAD = (r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung"
        r"\gezupfte-saite\proben\2026-11-15_S199_pruefstand_zupf.py")
spec = importlib.util.spec_from_file_location("pruefstand", PFAD)
P = importlib.util.module_from_spec(spec)
spec.loader.exec_module(P)

# Lautheit (LUFS) aus 2026-11-15_S199_pruefstand_oud_je_taste.txt, Tasten 36-72
HEUTE = [-18.4, -16.2, -17.9, -17.4, -17.9, -14.7, -14.1, -11.5, -6.7, -10.9, -12.7, -10.1, -7.8, -7.2, -4.2, -5.7, -8.0,
         -11.1, -11.0, -10.3, -3.1, -10.0, -11.2, -7.1, -4.5, -2.9, -2.6, -2.6, -3.9, -5.8, -7.6, -8.8, -8.0, -10.0, -8.5,
         -8.8, -12.1]


def masse(l):
    l = np.asarray(l)
    d3 = np.abs(l[3:] - l[:-3])
    paare = [abs(l[i + k] - l[i]) for k in range(1, 5) for i in range(len(l) - k)]
    return f"Spanne {l.max() - l.min():.1f} LU, Terzabstand: groesster {d3.max():.1f} / mittlerer (Median) {np.median(d3):.1f} LU; " \
           f"Paare bis 4 Halbtoene: groesster {max(paare):.1f} / Mittel {np.mean(paare):.1f} LU"


kp = P.lade_koerper("oud")
w = P.frequenzen()
tan = P.richtung_tan(kp["f"])
ch, mensur, cent = P.choere("oud")
punkt = kp["tasten"].index(36)
einzel = ch[0][1][0]
core = dict(einzel, EI=23.1e-6)            # EI des Kerns als massives Nylon (rechnung_saiten.py), statt 50e-6 uebertragen
varianten = {
    "einzeln (heute)": [einzel],
    "doppelt, +-1,5 cent": [einzel, einzel],
    "einzeln, EI Kern 23e-6": [core],
}
v, e = P.VORLAUF, P.VORLAUF + 0.4
ergebnis = {}
for name, saiten in varianten.items():
    Y, H = P.koerper_matrix(kp, w, tan, [punkt] * len(saiten))
    werte = []
    for taste in range(36, 41):
        bund = taste - 36
        laenge = mensur * 2 ** (-bund / 12)
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if len(saiten) == 2 else laenge for i in range(len(saiten))]
        xp = min(P.ZUPFORT, 0.5 * laenge)
        x = P.zeit(P.ton_mit(saiten, laengen, list(range(len(saiten))), xp, 0.002, 30.0, Y, H, w)[0])
        werte.append((taste, P.momentary_max(x), P.rms_db(x, 1, 2) - P.rms_db(x, v, e)))
        print(f"{name:24s} Taste {taste}: {werte[-1][1]:6.1f} LUFS, Pegel 1-2 s gegen Anfang {werte[-1][2]:6.1f} dB", flush=True)
    ergebnis[name] = werte
print()
for name, werte in ergebnis.items():
    l = [z[1] for z in werte] + HEUTE[5:]
    print(f"{name:24s}: {masse(l)}")
