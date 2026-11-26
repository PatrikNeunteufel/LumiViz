"""S223: der Bezug 34k-7d-1 noch einmal, aber fuer Koerper ohne Richtung am Steg (Bauplan) mit Winkel 0 statt der
Annahme nach Scherrer (`richtung_tan`). Schreibt die Bezugsdatei in die angegebene Ausgabe (nicht ins Repo)."""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
P = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"
spec = importlib.util.spec_from_file_location("bezug", P / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
bz.ps.richtung_von = lambda kp: kp["tan"] if kp.get("tan") is not None else np.zeros(len(kp["f"]))
sys.argv = [sys.argv[0], sys.argv[1]]
bz.main()
