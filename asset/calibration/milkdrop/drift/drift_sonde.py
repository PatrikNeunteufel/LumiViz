# -*- coding: utf-8 -*-
"""Drift-Sonde (S80): steht ein einmaliger Shape-Blitz ohne jede Preset-
Bewegung still? MilkdropRef (Original-Kern) gegen MilkdropStandalone.

`DriftSonde.milk`: kein Shader (PSVERSION 0), decay=1, zoom=1, warp=0, keine
Wave; eine weisse Shape leuchtet nur in den Frames 1-3 (Zaehler ueber q1).
Danach darf sich im Bild NICHTS mehr tun — die Referenz steht pixelgenau.

Befund S80 (vor dem Fix): wir wanderten exakt 0,5 px je Frame nach oben links
und verloren bis Frame 120 29 % Energie — der D3D9-Halbtexel-Versatz im
Warp-Mesh, unter GL ein echter Versatz (Offene_Punkte §3).

Aufruf:  python drift_sonde.py [marken]        # Vorgabe 3,10,30,60,120
Ausgabe: out/milkref_drift/ (Montagen je Marke) + Tabelle auf stdout.
Urteil:  Schwerpunkt-Wanderung <= 1 px und Energieverlust <= 5 % bei der
         letzten Marke = STEHT, sonst DRIFTET.
"""
import sys
from pathlib import Path

import numpy as np

HIER = Path(__file__).parent
sys.path.insert(0, str(HIER.parent))
import compare_milkref as cm  # noqa: E402

MILK = HIER / "DriftSonde.milk"
SIZE = "640x480"
OUT = cm.REPO / "out/milkref_drift"


def mass(path: Path) -> dict:
    img = cm.load_rgb(path)
    lum = 0.2126 * img[..., 0] + 0.7152 * img[..., 1] + 0.0722 * img[..., 2]
    s = float(lum.sum())
    ys, xs = np.mgrid[0:lum.shape[0], 0:lum.shape[1]]
    cx = float((lum * xs).sum() / s) if s > 0 else float("nan")
    cy = float((lum * ys).sum() / s) if s > 0 else float("nan")
    return {"sum": s, "cx": cx, "cy": cy, "max": float(lum.max())}


def main() -> int:
    marken = [int(a) for a in (sys.argv[1] if len(sys.argv) > 1
                               else "3,10,30,60,120").split(",")]
    refroot = OUT / "refroot"
    erste = {}
    letzte = {}
    for f in marken:
        ref_bmp, stumm, _ = cm.run_ref(MILK, f, SIZE, OUT / f"ref_f{f}", refroot,
                                       "klassisch", True, 180)
        lumi_png = cm.run_lumi(MILK, f, SIZE, OUT / f"lumi_f{f}", "klassisch", True, 180)
        seiten = {"REF ": mass(ref_bmp), "LUMI": mass(lumi_png)}
        cm.montage(cm.load_rgb(ref_bmp), cm.load_rgb(lumi_png),
                   OUT / f"montage_f{f:03d}.png")
        for name, m in seiten.items():
            erste.setdefault(name, m)
            letzte[name] = m
            print(f"f{f:4d} {name} sum={m['sum']:8.1f} max={m['max']:.3f} "
                  f"c=({m['cx']:7.2f},{m['cy']:7.2f})"
                  f"{'  REF-STUMM' if stumm and name == 'REF ' else ''}", flush=True)
    for name in erste:
        a, b = erste[name], letzte[name]
        weg = float(np.hypot(b["cx"] - a["cx"], b["cy"] - a["cy"]))
        verlust = 1.0 - b["sum"] / a["sum"] if a["sum"] > 0 else float("nan")
        urteil = "STEHT" if weg <= 1.0 and verlust <= 0.05 else "DRIFTET"
        print(f"{name}: Wanderung {weg:.2f} px, Energieverlust {verlust * 100:.1f} % "
              f"-> {urteil}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
