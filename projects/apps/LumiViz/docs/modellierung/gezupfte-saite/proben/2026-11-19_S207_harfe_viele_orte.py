"""S207 - Harfe am Pruefstand mit einem Koerper, der viele Hoerorte traegt (Kopie des Bauplans mit 16 Orten auf der
vorderen Halbkugel in 2,06 m, dazu `front`): je Taste das Momentary-Maximum je Hoerort, als tsv wie die TU-Tabelle
(MIDI, Leistungsmittel aller Orte ohne `front`, je Ort) - auszuwerten mit `tu_auswerten.py`.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <ordner mit concert_harp.lvinstbody> <ausgabe.tsv> [von=38] [bis=104]
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
ps.KOERPER = pathlib.Path(sys.argv[1])
opts = dict(a.split("=") for a in sys.argv[3:] if "=" in a)

kp = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
print("Hoerorte:", kp["orte"], flush=True)
with open(sys.argv[2], "w", encoding="utf-8") as aus:
    for taste in range(int(opts.get("von", 38)), int(opts.get("bis", 104)) + 1):
        sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(taste)])
        x = ps.zeit(ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
        je = [ps.momentary_max(x[:, o:o + 1]) for o in range(x.shape[1])]
        alle = ps.momentary_max(x[:, 1:]) - 10 * np.log10(x.shape[1] - 1)      # Leistungsmittel statt -summe
        aus.write("\t".join([str(taste), f"{alle:.3f}"] + [f"{v:.3f}" for v in je]) + "\n")
        aus.flush()
        print(f"{taste:3d}  front {je[0]:6.1f}   Mittel der 16 {alle:6.1f}   je Ort {min(je[1:]):6.1f} ... {max(je[1:]):6.1f}", flush=True)
