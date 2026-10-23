"""Fasst bend_slide_roh.tsv zusammen: Einzelaufnahmen und Lick-Ereignisse getrennt.

    python -I fasse_bend_slide.py <bend_slide_roh.tsv>
"""
import ast
import re
import sys
from collections import defaultdict

import numpy as np

zeilen = []
for z in open(sys.argv[1], encoding="utf-8"):
    name, j, art, r = z.rstrip("\n").split("\t", 3)
    r = re.sub(r"np\.float64\(([^)]*)\)", r"\1", r).replace("nan", "None")
    zeilen.append((name, int(j), art, ast.literal_eval(r)))


def med(werte):
    w = [v for v in werte if v is not None]
    if not w:
        return "-"
    return f"{np.median(w):.0f} ({np.min(w):.0f}..{np.max(w):.0f}, n={len(w)})"


def med2(werte):
    w = [v for v in werte if v is not None]
    if not w:
        return "-"
    return f"{np.median(w):.2f} ({np.min(w):.2f}..{np.max(w):.2f}, n={len(w)})"


print("## Bend, Einzelaufnahmen (G-Saite Bund 5)")
for name, j, art, r in zeilen:
    if art == "bending":
        print(f"{name} #{j}: Hub {r.get('hub_cent')} c, Halt {r.get('halt_cent')} c, Anstieg {r.get('anstieg_ms')} ms, "
              f"Form {r.get('form_25_50_75')}, Mehrverlust {r.get('mehrverlust_db')} dB, Halte-Rate {r.get('rate_halt_dbs')} dB/s")

print("\n## Bend in Licks")
lb = [r for _, _, a, r in zeilen if a == "lick_bend" and r.get("anstieg_ms") is not None]
klassen = defaultdict(list)
for r in lb:
    st = round(r["hub_cent"] / 100)
    klassen[st].append(r)
for st in sorted(klassen):
    rs = klassen[st]
    print(f"Hub ~{st} HT: n={len(rs)}, Hub {med([r['hub_cent'] for r in rs])} c, Anstieg 10-90 {med([r['anstieg_ms'] for r in rs])} ms, "
          f"t10 nach Onset {med([r['t10_ms'] for r in rs])} ms, 50%-Lage {med2([r['form_25_50_75'][1] for r in rs])}, "
          f"Ueberschwinger {med([r.get('ueberschwinger_cent') for r in rs])} c, Ereignisdauer {med([r['dauer_ms'] for r in rs])} ms")
alle_form = np.array([r["form_25_50_75"] for r in lb if None not in r["form_25_50_75"]])
if len(alle_form):
    print("Form gesamt (Median 25/50/75 %-Lage im 10-90-Anstieg):", np.round(np.median(alle_form, axis=0), 2),
          "linear waere [0.19 0.5 0.81]")
kein = [r for _, _, a, r in zeilen if a == "lick_bend" and r.get("kein_bend")]
print("BE-Ereignisse ohne messbaren Hub (<30 c):", len(kein))

print("\n## Slide, Einzelaufnahmen")
pts = []
for name, j, art, r in zeilen:
    if art == "slide" and r.get("gleit_ms") is not None:
        pts.append((abs(r["weg_cent"]) / 100, r["gleit_ms"], r.get("mehrverlust_db"), r.get("stufig_anteil"), name))
        print(f"{name} #{j}: Weg {r['weg_cent']} c, Gleiten {r['gleit_ms']} ms, {r['cent_pro_s']} c/s, stufig {r.get('stufig_anteil')}, "
              f"R2 cent/Hz {r.get('r2_cent')}/{r.get('r2_hz')}, Mehrverlust {r.get('mehrverlust_db')} dB")
p = np.array([(a, b) for a, b, *_ in pts if a >= 1.5])
k = np.polyfit(p[:, 0], p[:, 1], 1)
print(f"Gerade Gleitdauer = {k[1]:.1f} ms + {k[0]:.1f} ms je Halbton (Slides ab 2 HT, n={len(p)})")
print("Mehrverlust je Gitarre:", {g: med([m for a, b, m, s, n in pts if n.startswith(g)]) for g in ("AR", "FS", "LP")})
print("Stufigkeit (Anteil +-20 c um Halbton; gleichverteilt 0,40):", med2([s for a, b, m, s, n in pts]))

print("\n## Slide in Licks")
ls = [r for _, _, a, r in zeilen if a == "lick_slide" and r.get("gleit_ms") is not None]
for r in ls:
    pass
pl = np.array([(abs(r["weg_cent"]) / 100, r["gleit_ms"]) for r in ls])
if len(pl):
    print(f"n={len(pl)}, Weg {med([abs(r['weg_cent']) for r in ls])} c, Gleiten {med([r['gleit_ms'] for r in ls])} ms, "
          f"c/s {med([r['cent_pro_s'] for r in ls])}, stufig {med2([r.get('stufig_anteil') for r in ls])}, "
          f"Pegel nach-vor {med2([r.get('pegel_diff_db') for r in ls])} dB")
    for st in (1, 2, 3, 4, 5):
        sel = [r for r in ls if round(abs(r['weg_cent']) / 100) == st]
        if sel:
            print(f"  ~{st} HT: n={len(sel)}, Gleiten {med([r['gleit_ms'] for r in sel])} ms, Anregung {sorted(set(r['exc'] for r in sel))}")
