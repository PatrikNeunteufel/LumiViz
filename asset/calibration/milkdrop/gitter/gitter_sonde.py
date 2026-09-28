# -*- coding: utf-8 -*-
"""Gitter-Sonde (S80): in welcher Reihenfolge rechnen die Gitterpunkte ihren
per_pixel-Code — im Original und bei uns?

Anlass: Presets, die JE GITTERPUNKT wuerfeln (`Rock The House`:
`zoom=1+0.1*sin(…)*(rand(3)-1)`), passen mit dem Original-rand() schlechter,
obwohl der Generator nachweislich exakt ist (`test_NseelZufall`). Verdacht:
die Punkte ziehen in anderer Reihenfolge, jeder bekommt die Zahl eines anderen.

Aufbau je Probe (`Gitter_K<k>.milk`, erzeugt von diesem Skript):
- eine grosse Shape malt in den Frames 1-3 einen radialen Verlauf, danach
  steht das Bild (decay=1, zoom=1, warp=0, kein Shader);
- `per_pixel` zaehlt die Punkte mit (`idx` 0..824 je Frame bei 32x24);
- NUR Punkt `idx == K` bekommt einen Schub `dx` — dort verzerrt sich das Bild.

Gemessen wird der Schwerpunkt der Differenz zur Kontrolle (ohne Schub) — die
Lage von Punkt K auf dem Schirm. Beide Seiten: Referenz mit `--mesh 32x24`.

Aufruf:  python gitter_sonde.py            -> Tabelle Referenz | LumiViz je K
"""
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np

HIER = Path(__file__).parent
sys.path.insert(0, str(HIER.parent))
import compare_milkref as cm  # noqa: E402

OUT = cm.REPO / "out/milkref_gitter"
SIZE = "320x240"
FRAMES = int(__import__("os").environ.get("GITTER_FRAMES", "30"))
KS = [-1, 0, 1, 34, 35, 67, 100, 250, 412, 413, 445, 600, 790, 824]  # -1 = Kontrolle
SPALTEN = 33  # 32x24-Mesh = 33x25 Punkte

VORLAGE = """MILKDROP_PRESET_VERSION=201
PSVERSION=0
PSVERSION_WARP=0
PSVERSION_COMP=0
[preset00]
fRating=3.000
fGammaAdj=1.000
fDecay=1.000
fVideoEchoZoom=1.000
fVideoEchoAlpha=0.000
nVideoEchoOrientation=0
nWaveMode=0
bAdditiveWaves=0
bWaveDots=0
bWaveThick=0
bModWaveAlphaByVolume=0
bMaximizeWaveColor=0
bTexWrap=0
bDarkenCenter=0
bRedBlueStereo=0
bBrighten=0
bDarken=0
bSolarize=0
bInvert=0
fWaveAlpha=0.000
fWaveScale=1.000
fWaveSmoothing=0.000
fWaveParam=0.000
fModWaveAlphaStart=0.750
fModWaveAlphaEnd=0.950
fWarpAnimSpeed=1.000
fWarpScale=1.000
fZoomExponent=1.00000
fShader=0.000
zoom=1.00000
rot=0.00000
cx=0.500
cy=0.500
dx=0.00000
dy=0.00000
warp=0.00000
sx=1.00000
sy=1.00000
wave_r=1.000
wave_g=1.000
wave_b=1.000
wave_x=0.500
wave_y=0.500
ob_size=0.010
ob_r=0.000
ob_g=0.000
ob_b=0.000
ob_a=0.000
ib_size=0.010
ib_r=0.000
ib_g=0.000
ib_b=0.000
ib_a=0.000
nMotionVectorsX=12.000
nMotionVectorsY=9.000
mv_dx=0.000
mv_dy=0.000
mv_l=0.900
mv_r=1.000
mv_g=1.000
mv_b=1.000
mv_a=0.000
shapecode_0_enabled=1
shapecode_0_sides=40
shapecode_0_additive=0
shapecode_0_thickOutline=0
shapecode_0_textured=0
shapecode_0_num_inst=1
shapecode_0_x=0.500
shapecode_0_y=0.500
shapecode_0_rad=3.000
shapecode_0_ang=0.000
shapecode_0_tex_ang=0.000
shapecode_0_tex_zoom=1.000
shapecode_0_r=0.000
shapecode_0_g=0.000
shapecode_0_b=0.000
shapecode_0_a=1.000
shapecode_0_r2=1.000
shapecode_0_g2=1.000
shapecode_0_b2=1.000
shapecode_0_a2=1.000
shapecode_0_border_r=1.000
shapecode_0_border_g=1.000
shapecode_0_border_b=1.000
shapecode_0_border_a=0.000
shape_0_per_frame1=a=below(q1,3.5);
shape_0_per_frame2=a2=a;
per_frame_1=cnt=cnt+1;
per_frame_2=q1=cnt;
per_pixel_1=idx=k; k=k+1; k=k*below(k,{punkte});
per_pixel_2=dx=dx+0.02*equal(idx,{K})*above(q1,3.5);
"""


def erzeuge(k: int) -> Path:
    ziel = OUT / "presets" / f"Gitter_K{k if k >= 0 else 'X'}.milk"
    ziel.parent.mkdir(parents=True, exist_ok=True)
    ziel.write_text(VORLAGE.format(K=k, punkte=SPALTEN * 25), encoding="latin-1")
    return ziel


def ref(milk: Path) -> np.ndarray:
    root = cm.refwurzel_bauen(OUT / "refroot")
    kopie = root / milk.name
    shutil.copy2(milk, kopie)
    out = OUT / "ref"
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(cm.REF_EXE), str(kopie), "--frames", str(FRAMES), "--size", SIZE,
                    "--out", str(out), "--zufall-lumiviz", "--mesh", "32x24"],
                   capture_output=True)
    return cm.load_rgb(out / f"{milk.stem}_ref.bmp")


def lumi(milk: Path) -> np.ndarray:
    return cm.load_rgb(cm.run_lumi(milk, FRAMES, SIZE, OUT / "lumi", "klassisch", True, 300))


def lage(bild: np.ndarray, kontrolle: np.ndarray) -> str:
    d = np.abs(bild - kontrolle).sum(axis=2)
    s = d.sum()
    if s < 1e-6:
        return "   (keine Wirkung)   "
    ys, xs = np.mgrid[0:d.shape[0], 0:d.shape[1]]
    return f"x={float((d * xs).sum() / s):6.1f} y={float((d * ys).sum() / s):6.1f}"


def wirkung(bild: np.ndarray, kontrolle: np.ndarray) -> np.ndarray:
    """Was der Schub am Bild aendert (Luma, mit Vorzeichen)."""
    lum = lambda x: 0.2126 * x[..., 0] + 0.7152 * x[..., 1] + 0.0722 * x[..., 2]  # noqa: E731
    return lum(bild) - lum(kontrolle)


def gleichheit(a: np.ndarray, b: np.ndarray) -> float:
    """Korrelation zweier Wirkungsbilder: 1 = wirkt identisch, 0 = unabhaengig.
    Robust gegen den kleinen Unterschied der Kontrollbilder (Shape 1 px
    versetzt, Offene_Punkte §3) — verglichen wird die Aenderung, nicht das Bild."""
    a = a.ravel() - a.mean()
    b = b.ravel() - b.mean()
    n = float(np.sqrt((a * a).sum() * (b * b).sum()))
    return float((a * b).sum() / n) if n > 0 else float("nan")


def zufallsprobe() -> None:
    """Teil 2: Zufall JE GITTERPUNKT. Jeder Punkt schiebt um 0,004*(rand(3)-1).
    Stimmen Generator, Strom und Reihenfolge, wirkt das Muster wie im Original
    — mit dem bisherigen rand() (ganze Zahlen) nicht."""
    import os
    text = VORLAGE.format(K=-1, punkte=SPALTEN * 25).replace(
        "per_pixel_2=dx=dx+0.02*equal(idx,-1)*above(q1,3.5);",
        "per_pixel_2=dx=dx+0.004*(rand(3)-1)*above(q1,3.5);\n"
        "per_pixel_3=dy=dy+0.004*(rand(3)-1)*above(q1,3.5);")
    milk = OUT / "presets" / "Gitter_Zufall.milk"
    milk.write_text(text, encoding="latin-1")
    kontrolle = erzeuge(-1)
    wr = wirkung(ref(milk), ref(kontrolle))
    lk = lumi(kontrolle)
    os.environ.pop("LUMIVIZ_MILKDROP_EEL_RAND", None)
    wb = wirkung(lumi(milk), lk)
    os.environ["LUMIVIZ_MILKDROP_EEL_RAND"] = "original"
    wo = wirkung(lumi(milk), lk)
    os.environ.pop("LUMIVIZ_MILKDROP_EEL_RAND", None)
    print(f"Zufall je Gitterpunkt ({FRAMES} Frames) — Gleichheit der Wirkung mit dem "
          f"Original: LumiViz bisher {gleichheit(wr, wb):+.3f} · Original-rand "
          f"{gleichheit(wr, wo):+.3f}")


def musterprobe() -> None:
    """Teil 3: dasselbe Schubmuster OHNE Zufall ((idx%3)-1) — prueft, ob der
    Versatz je Gitterpunkt auf beiden Seiten gleich wirkt."""
    text = VORLAGE.format(K=-1, punkte=SPALTEN * 25).replace(
        "per_pixel_2=dx=dx+0.02*equal(idx,-1)*above(q1,3.5);",
        "per_pixel_2=dx=dx+0.004*((idx%3)-1)*above(q1,3.5);\n"
        "per_pixel_3=dy=dy+0.004*((idx%5)-2)*above(q1,3.5);")
    milk = OUT / "presets" / "Gitter_Muster.milk"
    milk.write_text(text, encoding="latin-1")
    kontrolle = erzeuge(-1)
    wr = wirkung(ref(milk), ref(kontrolle))
    wl = wirkung(lumi(milk), lumi(kontrolle))
    print(f"Muster je Gitterpunkt, ohne Zufall ({FRAMES} Frames) — Gleichheit der "
          f"Wirkung mit dem Original: {gleichheit(wr, wl):+.3f}")


def main() -> int:
    if "zufall" in sys.argv[1:]:
        zufallsprobe()
        return 0
    if "muster" in sys.argv[1:]:
        musterprobe()
        return 0
    presets = {k: erzeuge(k) for k in KS}
    r0, l0 = ref(presets[-1]), lumi(presets[-1])
    print(f"Kontrolle: Referenz-Luma {r0.mean():.3f}, LumiViz-Luma {l0.mean():.3f}")
    print(f"{'K':>5} {'erwartet (Zeile, Spalte)':>26} | {'Referenz':^22} | {'LumiViz':^22}")
    for k in KS[1:]:
        print(f"{k:5d} {'(' + str(k // SPALTEN) + ', ' + str(k % SPALTEN) + ')':>26} | "
              f"{lage(ref(presets[k]), r0):22s} | {lage(lumi(presets[k]), l0):22s}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
