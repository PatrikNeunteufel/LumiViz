# -*- coding: utf-8 -*-
"""Laeuft die Modul-Sonden (make_module_probes.py) gegen AvsRef.

Urteil bewusst NICHT ueber dMean: duenne Vordergruende bewegen den Mittelwert
nicht — in Session 50 meldete die Bisektionsleiter viermal "OK", waehrend wir
sichtbar nichts zeichneten. Verglichen wird deshalb primaer, WAS gezeichnet
wurde: Zahl der Pixel, die sich vom Hintergrund abheben, und deren
Schwerpunkt. Beides ist flaechenunabhaengig und faellt sofort auf, wenn ein
Modul gar nichts oder an der falschen Stelle zeichnet.

Aufruf:
  python run_module_probes.py [--frames N] [--size WxH] [unterordner ...]
"""
import argparse
import sys
from pathlib import Path

import numpy as np
from PIL import Image

import compare_avsref as ca

ROOT = Path(__file__).parent
PROBES = ROOT / "probes"


def drawn(img: np.ndarray) -> tuple[int, float, float]:
    """(Pixelzahl, Schwerpunkt-Zeile, Schwerpunkt-Spalte) alles Nicht-Hintergrund.

    Hintergrund = der haeufigste Farbwert des Bildes; alles, was um mehr als
    8 Stufen abweicht, gilt als gezeichnet.
    """
    grey = img.mean(axis=2) * 255.0
    vals, counts = np.unique(np.round(grey).astype(int), return_counts=True)
    bg = float(vals[int(np.argmax(counts))])
    lit = np.abs(grey - bg) > 8.0
    if not lit.any():
        return 0, -1.0, -1.0
    ys, xs = np.nonzero(lit)
    return int(lit.sum()), float(ys.mean()), float(xs.mean())


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ordner", nargs="*", help="Unterordner von probes/ (Default: alle)")
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--size", default="320x240")
    ap.add_argument("--beat-period", type=int, default=30)
    ap.add_argument("--out", type=Path, default=ROOT / "../../../out/module_probes")
    args = ap.parse_args()

    presets = []
    for d in (args.ordner or [""]):
        presets += sorted((PROBES / d).rglob("*.avs"))
    if not presets:
        print("FEHLER: keine Sonden gefunden — erst make_module_probes.py laufen lassen")
        return 2

    out: Path = args.out.resolve()
    (out / "ref").mkdir(parents=True, exist_ok=True)
    (out / "lumi").mkdir(exist_ok=True)
    (out / "montage").mkdir(exist_ok=True)

    rows = []
    for avs in presets:
        rel = f"{avs.parent.name}/{avs.name}"
        try:
            src = ca.ascii_safe(avs, out / "_ascii")
            ref = ca.load_rgb(ca.run_ref(src, args.frames, args.size, out / "ref",
                                         args.beat_period, ca.APE_DIR))
            lumi = ca.load_rgb(ca.run_lumi(src, args.frames, args.size, out / "lumi",
                                           args.beat_period))
            ca.montage(ref, lumi, out / "montage" / f"{avs.stem}.png")
            d = ca.compare(ref, lumi)
            nr, yr, xr = drawn(ref)
            nl, yl, xl = drawn(lumi)
            # Urteil: relative Abweichung der gezeichneten Menge (mit Sockel
            # gegen Rauschen bei winzigen Figuren) UND Lage des Schwerpunkts.
            menge = abs(nr - nl) / max(nr, nl, 50)
            lage = 0.0 if nr == 0 or nl == 0 else max(abs(yr - yl), abs(xr - xl))
            leer = (nr == 0) != (nl == 0)
            urteil = ("LEER" if leer else
                      "OK" if (menge <= 0.10 and lage <= 2.0 and d["mae"] <= 0.03)
                      else "PRUEFEN")
            rows.append((rel, nr, nl, menge, lage, d["mae"], urteil))
            print(f"  {urteil:7s} {rel:34s} px {nr:6d}/{nl:6d}  "
                  f"Menge {menge:5.2f}  Lage {lage:5.1f}  MAE {d['mae']:.3f}")
        except Exception as e:  # noqa: BLE001 — je Sonde weitermachen
            rows.append((rel, -1, -1, 0.0, 0.0, 0.0, "FEHLER"))
            print(f"  FEHLER  {rel}: {e}")

    with (out / "report.md").open("w", encoding="utf-8") as f:
        f.write(f"# Modul-Sonden — {args.frames} Frames, {args.size}\n\n")
        f.write("| Sonde | px ref | px lumi | Menge | Lage | MAE | Urteil |\n")
        f.write("|---|---|---|---|---|---|---|\n")
        for rel, nr, nl, menge, lage, mae, urteil in rows:
            f.write(f"| {rel} | {nr} | {nl} | {menge:.2f} | {lage:.1f} | "
                    f"{mae:.3f} | {urteil} |\n")

    ok = sum(1 for r in rows if r[6] == "OK")
    print(f"\nReport: {out / 'report.md'}")
    print(f"{ok}/{len(rows)} OK")
    return 0 if ok == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main())
