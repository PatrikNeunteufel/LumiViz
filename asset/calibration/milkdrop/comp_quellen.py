# -*- coding: utf-8 -*-
"""Comp-Quellen-Sonde (S80): welche Bildquelle des Comp-Shaders weicht ab?

Anlass: bei `martin - stratospheric turbulences` leuchten im Original gelbe
Kugeln, bei uns gruene/keine — der Comp-Shader ist eingegrenzt (ohne ihn stimmt
die Szene auf 0,004-0,012). Er holt Licht aus drei Quellen: GetPixel (das
Feedback-Bild), GetBlur1 und GetBlur2 (die Blur-Pyramide).

Die Sonde ersetzt den Comp-Shader eines Presets durch Minimal-Shader, die je
genau EINE Quelle zeigen, und vergleicht mit dem Original (MilkdropRef
--zufall-lumiviz: gleicher Zufall auf beiden Seiten; Skript-Zufall bei uns
Original = Standard seit S80). Alles andere am Preset bleibt.

Aufruf:  python comp_quellen.py [preset.milk] [--frames N]
"""
import argparse
import re
import shutil
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HIER = Path(__file__).parent
sys.path.insert(0, str(HIER))
import compare_milkref as cm  # noqa: E402

OUT = cm.REPO / "out/milkref_compquellen"
VORGABE = cm.REPO / "asset/Milkdrop3/presets/martin - stratospheric turbulences.milk"

QUELLEN = {
    "pixel": "ret = GetPixel(uv);",
    "blur1": "ret = GetBlur1(uv);",
    "blur2": "ret = GetBlur2(uv);",
    "blur3": "ret = GetBlur3(uv);",
}


def variante(milk: Path, name: str, body: str | None) -> Path:
    zeilen = milk.read_text(encoding="latin-1").splitlines()
    out = [z for z in zeilen if not z.startswith("comp_")]
    if body is not None:
        out += ["comp_1=`shader_body", "comp_2=`{", f"comp_3=`    {body}", "comp_4=`}"]
    ziel = OUT / "presets" / f"{milk.stem}__{name}.milk"
    ziel.parent.mkdir(parents=True, exist_ok=True)
    ziel.write_text("\n".join(out) + "\n", encoding="latin-1")
    return ziel


def ref(milk: Path, frames: int, size: str) -> np.ndarray:
    root = cm.refwurzel_bauen(OUT / "refroot")
    kopie = root / milk.name
    shutil.copy2(milk, kopie)
    out = OUT / f"ref_f{frames}"
    out.mkdir(parents=True, exist_ok=True)
    cm.messlauf.run([str(cm.REF_EXE), str(kopie), "--frames", str(frames), "--size", size,
                    "--out", str(out), "--zufall-lumiviz"], capture_output=True)
    return cm.load_rgb(out / f"{milk.stem}_ref.bmp")


def lumi(milk: Path, frames: int, size: str) -> np.ndarray:
    return cm.load_rgb(cm.run_lumi(milk, frames, size, OUT / f"lumi_f{frames}", "klassisch",
                                   False, 300))


def kanal(img: np.ndarray) -> str:
    m = img.reshape(-1, 3).mean(axis=0)
    return f"R{m[0]:.3f} G{m[1]:.3f} B{m[2]:.3f}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("preset", nargs="?", type=Path, default=VORGABE)
    ap.add_argument("--frames", type=int, default=30)
    ap.add_argument("--size", default="640x480")
    a = ap.parse_args()
    reihen = []
    for name, body in [("voll", "__VOLL__")] + list(QUELLEN.items()):
        milk = a.preset if body == "__VOLL__" else variante(a.preset, name, body)
        r, l = ref(milk, a.frames, a.size), lumi(milk, a.frames, a.size)
        mae = float(np.abs(r - l).mean())
        print(f"{name:6s} MAE {mae:.3f}   Original {kanal(r)}   LumiViz {kanal(l)}", flush=True)
        reihen.append((name, r, l))
    h, w = reihen[0][1].shape[:2]
    bogen = np.ones((h * len(reihen) + 4 * len(reihen), w * 2 + 4, 3)) * 0.5
    for i, (_, r, l) in enumerate(reihen):
        y = i * (h + 4)
        bogen[y:y + h, :w] = r
        bogen[y:y + h, w + 4:] = l
    Image.fromarray((bogen * 255).astype(np.uint8)).save(OUT / f"{a.preset.stem}_quellen.png")
    print(f"Bogen: {OUT / (a.preset.stem + '_quellen.png')} (links Original, rechts LumiViz)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
