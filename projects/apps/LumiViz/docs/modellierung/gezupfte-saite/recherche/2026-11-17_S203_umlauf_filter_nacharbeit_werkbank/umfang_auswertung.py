"""LVR Auftrag 21, Punkt 2: Auswertung von weg2_umfang.txt (ganzer Umfang). Zaehlt alles aus der Datei, nichts von Hand.
Aufruf: python -B umfang_auswertung.py -> umfang_auswertung.txt
"""
import sys
sys.dont_write_bytecode = True
import pathlib
from collections import defaultdict

import numpy as np

HIER = pathlib.Path(__file__).parent
zeilen = (HIER / "weg2_umfang.txt").read_text(encoding="utf-8").splitlines()[2:]
aus = open(HIER / "umfang_auswertung.txt", "w", encoding="utf-8")


def p(*a):
    s = " ".join(str(x) for x in a)
    print(s)
    aus.write(s + "\n")


D = defaultdict(dict)          # (inst, taste) -> N -> dict
ohne = defaultdict(int)
for z in zeilen:
    t = [s.strip() for s in z.split("|")]
    kopf = t[0].split()
    inst, taste = kopf[0], " ".join(kopf[1:])
    N = int(t[1])
    if "zu wenig" in t[2]:
        D[(inst, taste)][N] = None
        ohne[(inst, N)] += 1
        continue
    w1 = [float(v) for v in t[2:4][0].split()] + [float(v) for v in t[3].split()]
    og = [float(v) for v in t[4].split()[1:]]
    e = {"w1": w1, "og": og}
    if len(t) > 5:
        if "gescheitert" in t[5]:
            e["w2a"] = None
        else:
            e["w2a"] = [float(v) for v in t[5].split()]
        e["w2b"] = [float(v) for v in t[6].split()]
        ms = t[7].split("/")
        e["ms"] = [float(ms[0]) if ms[0] != "nan" else np.nan, float(ms[1])]
    D[(inst, taste)][N] = e

p("Ganzer Umfang - Verlustfilter allein (verlust_mass); W1 = Bank & Valimaki vollstaendig, oG = dasselbe ohne Glaettung.")
p("Fehler = groesster |Fehler des Abklingens| in % an den hoerbaren Teiltoenen (sigma <= 100 1/s); Median und Groesstwert ueber die Tasten.\n")
inst_liste = ["Gitarre", "Harfe", "Cembalo", "Laute", "Oud"]
for N in (2, 4, 8):
    p(f"=== Ordnung {N}")
    p("Instrument | Tasten | gerechnet | W1 passiv | W1 nicht passiv (Tasten) | oG passiv | W1 hoerbar % Median / max (nur passive) | "
      "oG hoerbar % Median / max (nur passive) | W1 1-12 % Median / max | oG 1-12 % Median / max | W2a hoerbar % an den nicht passiven | W2b dto. | ms 2a / 2b")
    for inst in inst_liste:
        keys = [k for k in D if k[0] == inst]
        e = [(k, D[k].get(N)) for k in keys]
        ger = [(k, x) for k, x in e if x is not None]
        pas = [(k, x) for k, x in ger if x["w1"][0] <= 1]
        nicht = [(k, x) for k, x in ger if x["w1"][0] > 1]
        pas0 = [(k, x) for k, x in ger if x["og"][0] <= 1]
        w1r = np.array([x["w1"][4] for _, x in pas])
        ogr = np.array([x["og"][4] for _, x in pas0])
        w112 = np.array([x["w1"][2] for _, x in pas])
        og12 = np.array([x["og"][2] for _, x in pas0])
        f = lambda a: f"{np.nanmedian(a):6.1f} / {np.nanmax(a):7.1f}" if len(a) else "   -  /    -   "
        w2 = [x for _, x in ger if "w2b" in x]
        w2a = np.array([x["w2a"][4] if x["w2a"] else np.nan for x in w2])
        w2b = np.array([x["w2b"][3] for x in w2])
        ms = np.array([x["ms"] for x in w2]) if w2 else np.zeros((0, 2))
        p(f"{inst:8s} | {len(keys):3d} | {len(ger):3d} | {len(pas):3d} | {len(nicht):2d} {', '.join(k[1] for k, _ in nicht)} | {len(pas0):3d} | "
          f"{f(w1r)} | {f(ogr)} | {f(w112)} | {f(og12)} | {', '.join(f'{v:.1f}' for v in w2a)} | {', '.join(f'{v:.1f}' for v in w2b)} | "
          f"{', '.join(f'{a:.0f}/{b:.0f}' for a, b in ms)}")
    p("")

p("=== Regel 'kleinster Fehler (hoerbar) unter den passiven Ordnungen 2/4/8' - W1 und oG")
for var in ("w1", "og"):
    for inst in inst_liste:
        keys = [k for k in D if k[0] == inst]
        best, keine, ordn = [], [], defaultdict(int)
        for k in keys:
            c = [(x[var][4], N) for N, x in D[k].items() if x is not None and x[var][0] <= 1 and not np.isnan(x[var][4])]
            if c:
                v, n = min(c)
                best.append(v)
                ordn[n] += 1
            else:
                keine.append(k[1])
        b = np.array(best)
        p(f"{var} {inst:8s}: Tasten {len(keys)}, ohne passive Ordnung {len(keine)} {keine}; Fehler hoerbar Median {np.median(b):.1f} %, "
          f"max {b.max():.1f} %, Tasten mit <= 5 %: {int(np.sum(b <= 5))}; gewaehlte Ordnung {dict(sorted(ordn.items()))}")
aus.close()
