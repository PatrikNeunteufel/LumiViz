"""S208 - Harfe je Taste MIT Mitschwingen nach GZ8 a (Oktaven, Quinten, Quarten der gespielten Saite, 20 Saiten) am
Pruefkoerper mit 16 Hoerorten (KQ36 a). Rechenweg aus `2026-11-16_S200_harfe_mitschwingen.py` (`rechne`, Block fuer
Block); die gespielte Saite mit ihrem Pedal wie `harfe_taste` im Pruefstand, die uebrigen mit dem Pedal in der Mitte.
Je Taste zwei Auswahlen: "keine" (nur die gespielte, wie der Pruefstand bisher) und "O+Q" (GZ8 a).

Ausgabe tsv: Taste, Saite (Taste der leeren Saite), Versatz, dann je Auswahl Leistungsmittel der 16 Orte und je Ort.
Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner mit concert_harp.lvinstbody> <ausgabe.tsv> [von=38] [bis=104]
Der Ordner: `2026-11-19_S207_harfe_viele_orte_plan.py <ordner>`, dann `InstrumentBodyTool <ordner>/concert_harp.lvinstbodyplan
--to 5000` (aus einer Kopie, rund 160 s). Rund 50 s je Taste. Auswerten: `cut -f1,4,6-21` (nur die gespielte Saite) bzw.
`cut -f1,22,24-39` (mit Mitschwingen), dann `2026-11-19_S207_tu_auswerten.py`.
"""
import importlib.util
import pathlib
import sys
import time

import numpy as np

ROOT = pathlib.Path.cwd()
S200 = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-16_S200_harfe_mitschwingen.py"
spec = importlib.util.spec_from_file_location("s200", S200)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
ps = m.ps
ps.KOERPER = pathlib.Path(sys.argv[1])
opts = dict(a.split("=") for a in sys.argv[3:] if "=" in a)

alt_auswahlen = m.auswahlen
m.auswahlen = lambda s, n: {k: v for k, v in alt_auswahlen(s, n).items() if k in ("keine", "O+Q")}

kp = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
grund = m.saiten()                      # (Taste der leeren Saite, Saite, Laenge mit Pedal Mitte), aufsteigend
offen = [t for t, _, _ in grund]
print(f"{len(kp['f'])} Moden, {kp['G'].shape[1]} Hoerorte, {len(grund)} Saiten", flush=True)


def mittel(x):
    return ps.momentary_max(x[:, 1:]) - 10 * np.log10(x.shape[1] - 1)


with open(sys.argv[2], "w", encoding="utf-8") as aus:
    for taste in range(int(opts.get("von", 38)), int(opts.get("bis", 104)) + 1):
        t0 = time.time()
        sa, L, *_ = ps.harfe_taste(taste)
        versatz = next(v for v in (1, 2, 0) if taste - v in offen and (v or taste == min(offen)))
        s = offen.index(taste - versatz)
        alle = list(grund)
        alle[s] = (alle[s][0], alle[s][1], L)          # die gespielte Saite mit ihrem Pedal
        erg = m.rechne(kp, tan, alle, s, w)
        zeile = [str(taste), str(offen[s]), str(versatz)]
        werte = {}
        for name in ("keine", "O+Q"):
            x, anz = erg[name]
            werte[name] = (mittel(x), [ps.momentary_max(x[:, o:o + 1]) for o in range(x.shape[1])], anz)
            zeile += [f"{werte[name][0]:.3f}"] + [f"{v:.3f}" for v in werte[name][1]]
        aus.write("\t".join(zeile) + "\n")
        aus.flush()
        print(f"{taste:3d} Saite {offen[s]:3d}+{versatz}  keine {werte['keine'][0]:6.1f}  O+Q ({werte['O+Q'][2]:2d}) {werte['O+Q'][0]:6.1f}"
              f"  Unterschied {werte['O+Q'][0] - werte['keine'][0]:+5.2f} LU   {time.time() - t0:.0f} s", flush=True)
