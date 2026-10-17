"""Y-Sonden auswerten — am Bild, nicht an der MAE (S130).

Die Sonden zeichnen wenig (ein Verlauf, ein kleines Quadrat): ein gespiegeltes
Quadrat ergibt eine winzige MAE, das Vergleichsskript meldet dann „OK“. Dieses
Skript liest die Montagen von `compare_milkref.py` (Referenz | LumiViz | Diff)
und urteilt je Sonde:

- Verlauf (`Y_comp`, `Y_warp*` ausser `_samp`): Rot im oberen und unteren
  Zehntel, Referenz gegen LumiViz.
- Quadrat (`Y_dy`, `Y_warp_samp`, `Y_comp_samp`, `Y_shape_*`): Schwerpunkt des
  Rots (x, y; 0 = links/oben), Referenz gegen LumiViz.

Aufruf (nach dem Lauf, den die README nennt):

    python asset/calibration/milkdrop/ysonde/ysonde_auswerten.py out/ysonde --size 640x480

Rueckgabe 0, wenn alle Sonden stimmen, sonst 1.
"""
import argparse
import sys
from pathlib import Path

import numpy as np
from PIL import Image

GRENZE = 0.03  # erlaubte Abweichung (Anteil der Bildhoehe/-breite bzw. Rotwert)


def felder(montage: Path, breite: int):
    """Die drei Felder der Montage: Referenz, LumiViz (Diff wird nicht gebraucht)."""
    a = np.asarray(Image.open(montage).convert("RGB"), dtype=np.float64) / 255.0
    luecke = (a.shape[1] - 3 * breite) // 2
    return a[:, :breite], a[:, breite + luecke:2 * breite + luecke]


def oben_unten(bild):
    z = max(1, bild.shape[0] // 10)
    return bild[:z, :, 0].mean(), bild[-z:, :, 0].mean()


def schwerpunkt(bild):
    rot = np.clip(bild[:, :, 0] - np.maximum(bild[:, :, 1], bild[:, :, 2]), 0.0, None)
    summe = rot.sum()
    if summe <= 0.0:
        return None
    ys, xs = np.mgrid[0:bild.shape[0], 0:bild.shape[1]]
    return (xs * rot).sum() / summe / bild.shape[1], (ys * rot).sum() / summe / bild.shape[0]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("lauf", type=Path, help="Ausgabeordner von compare_milkref.py (--out)")
    ap.add_argument("--size", default="640x480")
    ap.add_argument("--frame", type=int, default=30)
    ap.add_argument("--saat", default="0")
    args = ap.parse_args()
    breite = int(args.size.split("x")[0])
    montagen = sorted((args.lauf / "montage").glob(f"Y_*_s{args.saat}_f{args.frame}.png"))
    if not montagen:
        print(f"keine Montagen in {args.lauf / 'montage'} (Frame {args.frame}, Saat {args.saat})")
        return 1
    fehler = 0
    for m in montagen:
        name = m.name.split(f"_s{args.saat}_")[0]
        ref, lumi = felder(m, breite)
        if name in ("Y_dy", "Y_warp_samp", "Y_comp_samp") or name.startswith("Y_shape"):
            r, l = schwerpunkt(ref), schwerpunkt(lumi)
            if r is None or l is None:
                print(f"  ?? {name}: kein Rot (Referenz {r}, LumiViz {l})")
                fehler += 1
                continue
            ok = abs(r[0] - l[0]) <= GRENZE and abs(r[1] - l[1]) <= GRENZE
            print(f"  {'OK' if ok else 'BEFUND'} {name}: Schwerpunkt Referenz ({r[0]:.3f}, {r[1]:.3f}), "
                  f"LumiViz ({l[0]:.3f}, {l[1]:.3f})")
        else:
            r, l = oben_unten(ref), oben_unten(lumi)
            ok = abs(r[0] - l[0]) <= GRENZE and abs(r[1] - l[1]) <= GRENZE
            print(f"  {'OK' if ok else 'BEFUND'} {name}: Rot oben/unten Referenz {r[0]:.3f}/{r[1]:.3f}, "
                  f"LumiViz {l[0]:.3f}/{l[1]:.3f}")
        fehler += 0 if ok else 1
    print(f"{len(montagen) - fehler}/{len(montagen)} stimmen")
    return 0 if fehler == 0 else 1


sys.exit(main())
