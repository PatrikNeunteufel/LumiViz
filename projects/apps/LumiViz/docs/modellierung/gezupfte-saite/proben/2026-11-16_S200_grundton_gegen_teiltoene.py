"""S200, 34k-6 — Verstimmt der Koerper den Grundton? Iowa-Gitarre gegen den Pruefstand, dieselbe Messung.

Aufruf (vom Projekt-Root): python -B <dieses Skript> <Ordner Iowa_MIS_Guitar_mono2496> [<ausgabe.txt>]

Anlass (Probe Bend, `2026-11-16_S200_zeitbereich_bend.py`): im Modell klingt G2 auf Saite 6 gehalten 10 Cent zu tief,
der Grundton allein 21 Cent — er liegt auf der Luftmode A0 des Koerpers g34 (98,1 Hz). Ist das an einer echten Gitarre
auch so gross?

Messgroesse, unabhaengig von der Stimmung des Instruments: der Grundton gegen die Teiltoene 2-4,
    d = 1200 lg2( f1 / Median(f_n / n, n = 2..4) )  in Cent
(Frequenzen je Teilton wie in `2026-11-15_S199_ausklang_je_teilton.py`: groesste Projektion 0,05-0,45 s nach dem
Einsatz). Eine steife Saite gibt d leicht negativ (unter 1 Cent bei diesen Saiten); der Koerper schiebt den Grundton
nahe einer starken Mode. Iowa MIS: Raimundo 118, jede Saite chromatisch, pp / mf / ff — eine andere Gitarre als g34, die
Moden liegen dort anders; verglichen wird die Groesse der Ausschlaege im Bereich 80-260 Hz, nicht die Taste.
"""
import importlib.util
import pathlib
import re
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve()


def lade(name):
    spec = importlib.util.spec_from_file_location(name, HIER.with_name(name + ".py"))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


at = lade("2026-11-15_S199_ausklang_je_teilton")
ps = at.ps


def mass(teile):
    f = {n: hz for n, hz, *_ in teile}
    if 1 not in f or sum(n in f for n in (2, 3, 4)) < 2:
        return float("nan")
    return 1200 * np.log2(f[1] / np.median([f[n] / n for n in (2, 3, 4) if n in f]))


if __name__ == "__main__":
    ordner = pathlib.Path(sys.argv[1])
    aus = open(sys.argv[2], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 2 else None

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    iowa = {}
    for pfad in sorted(ordner.glob("Guitar.*.aif")):
        m = re.fullmatch(r"Guitar\.(pp|mf|ff)\.(sul_?[A-G])\.([A-G]b?\d)([A-G]b?\d)?\.mono\.aif", pfad.name)
        s = at.SAITEN[m.group(2)]
        von, bis = at.midi(m.group(3)), at.midi(m.group(4) or m.group(3))
        x, fs = at.lies_aiff(pfad)
        st = at.stuecke(x, fs)
        if len(st) != bis - von + 1:
            schreibe(f"# {pfad.name}: {len(st)} Stuecke, erwartet {bis - von + 1} - ausgelassen")
            continue
        for (a, e), taste in zip(st, range(von, bis + 1)):
            gem = at.teiltoene(x[a:e], fs, ps.hz_von(taste))
            if gem:
                iowa.setdefault((s, taste), {})[m.group(1)] = mass(gem)
    kp = ps.lade_koerper("classical_guitar")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    modell = {}
    for s in range(6):
        kurven = ps.koerper_kurven2(kp, s, w, tan)
        for bund in range(0, 13):
            L = ps.MENSUR * 2 ** (-bund / 12)
            y = ps.zeit(ps.ton2(ps.GITARRE[s], L, min(ps.ZUPFORT, 0.5 * L), 0.002, 30.0, *kurven, w))
            taste = ps.GITARRE[s]["taste"] + bund
            modell[(s, taste)] = mass(at.teiltoene(y[:, 0] + y[:, 1], ps.FS, ps.hz_von(taste)))
    schreibe("Saite Taste Hz | Grundton gegen die Teiltoene 2-4 in Cent: Iowa pp / mf / ff | Modell (g34)")
    for (s, taste) in sorted(set(iowa) | set(modell)):
        i = iowa.get((s, taste), {})
        schreibe(f"{6 - s}  {taste:3d}  {ps.hz_von(taste):6.1f} | " + " / ".join(f"{i.get(k, float('nan')):+6.1f}" for k in ("pp", "mf", "ff")) +
                 f" | {modell.get((s, taste), float('nan')):+6.1f}")
    schreibe()
    for name, lo, hi in (("80-130 Hz (um A0)", 80, 130), ("130-260 Hz (um T1)", 130, 260), ("260-700 Hz", 260, 700)):
        iw = np.array([v for (s, t), d in iowa.items() if lo <= ps.hz_von(t) < hi for v in d.values() if np.isfinite(v)])
        mw = np.array([v for (s, t), v in modell.items() if lo <= ps.hz_von(t) < hi and np.isfinite(v)])
        schreibe(f"{name}: Iowa {len(iw)} Werte, Median {np.median(iw):+.1f}, 10.-90. Perzentil {np.percentile(iw, 10):+.1f} ... {np.percentile(iw, 90):+.1f}, "
                 f"groesster Betrag {np.abs(iw).max():.1f} Cent | Modell {len(mw)} Werte, Median {np.median(mw):+.1f}, "
                 f"{np.percentile(mw, 10):+.1f} ... {np.percentile(mw, 90):+.1f}, groesster Betrag {np.abs(mw).max():.1f} Cent")
