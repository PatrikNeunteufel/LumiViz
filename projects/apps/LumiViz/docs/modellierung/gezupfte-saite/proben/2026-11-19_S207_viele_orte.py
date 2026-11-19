"""S207, KQ36 a - Laute, Oud oder Cembalo am Pruefstand mit einem Pruefkoerper, der 16 Hoerorte in 2,06 m traegt
(`..._viele_orte_plan.py`, dann InstrumentBodyTool): je Taste das Momentary-Maximum je Hoerort, als tsv wie die
TU-Tabelle (MIDI, Leistungsmittel der 16, der bisherige Hoerort, je Ort) - auszuwerten mit `..._tu_auswerten.py`
nach `cut -f1,2,4-`. Saiten, Zupf und Tasten wie `chor_instrument` / `tasten_instrument` des Pruefstands.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> laute|oud|cembalo <ordner mit dem koerper> <ausgabe.tsv>
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
name = sys.argv[1]
ps.KOERPER = pathlib.Path(sys.argv[2])

kp = ps.lade_koerper({"laute": "lute", "oud": "oud", "cembalo": "harpsichord"}[name])
tan = ps.richtung_von(kp)
w = ps.frequenzen()
print(f"{name}: {len(kp['f'])} Moden, Hoerorte {kp['orte']}", flush=True)


def toene():
    if name == "cembalo":
        for taste in range(kp["tasten"][0], kp["tasten"][-1] + 1):
            sa, laenge, xp, d, winkel, breite, tempo = ps.cembalo_taste(taste)
            Y, H = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(taste)])
            yield taste, ps.ton_mit([sa], [laenge], 0, xp, d, winkel, Y, H, w, breite, tempo)[0]
        return
    ch, mensur, cent = ps.choere(name)
    kurven = {}
    for taste in range(ch[0][0], ch[-1][0] + 13):
        c = max(i for i, (t, _) in enumerate(ch) if t <= taste)
        saiten = ch[c][1]
        if c not in kurven:
            kurven = {c: ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(ch[c][0])] * len(saiten))}
        Y, H = kurven[c]
        laenge = mensur * 2 ** (-(taste - ch[c][0]) / 12)
        gleich = len(saiten) == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else laenge for i in range(len(saiten))]
        yield taste, ps.ton_mit(saiten, laengen, list(range(len(saiten))), min(ps.ZUPFORT, 0.5 * laenge), 0.002, 30.0, Y, H, w)[0]


with open(sys.argv[3], "w", encoding="utf-8") as aus:
    for taste, spektrum in toene():
        x = ps.zeit(spektrum)
        je = [ps.momentary_max(x[:, o:o + 1]) for o in range(x.shape[1])]
        alle = ps.momentary_max(x[:, 1:]) - 10 * np.log10(x.shape[1] - 1)
        aus.write("\t".join([str(taste), f"{alle:.3f}"] + [f"{v:.3f}" for v in je]) + "\n")
        aus.flush()
        print(f"{taste:3d}  bisheriger Ort {je[0]:6.1f}   Mittel der 16 {alle:6.1f}   je Ort {min(je[1:]):6.1f} ... {max(je[1:]):6.1f}", flush=True)
