# -*- coding: utf-8 -*-
"""Spektrum-Wave-Sonde (S80): liefern Custom-Waves mit bSpectrum=1 dieselben
value1/value2 wie das Original?

Anlass: `martin - stratospheric turbulences` baut seine Teilchen-Positionen aus
einer UNSICHTBAREN Spektrum-Wave (per_point: gmegabuf(t1) = … value1+value2).
Stimmen die Werte nicht, landen alle Leuchtkugeln woanders.

Die Sonde zeichnet eine Spektrum-Wave SICHTBAR als Kurve: x = sample,
y = 0.1 + k·(value1+value2). Gemessen wird je Bildspalte die Hoehe der Kurve
(Schwerpunkt der hellen Pixel) — ein Profil, das man direkt vergleichen kann.
Zweite Probe mit bSpectrum=0 (Wellenform) als Gegenprobe.

Aufruf:  python spektrum_wave.py [--frames N]
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HIER = Path(__file__).parent
sys.path.insert(0, str(HIER))
import compare_milkref as cm  # noqa: E402

OUT = cm.REPO / "out/milkref_spektrum"

KOPF = """MILKDROP_PRESET_VERSION=201
PSVERSION=0
PSVERSION_WARP=0
PSVERSION_COMP=0
[preset00]
fDecay=0.000
fGammaAdj=1.000
fVideoEchoAlpha=0.000
nWaveMode=0
fWaveAlpha=0.000
bModWaveAlphaByVolume=0
zoom=1.00000
rot=0.00000
warp=0.00000
ob_a=0.000
ib_a=0.000
mv_a=0.000
wavecode_0_enabled=1
wavecode_0_samples={samples}
wavecode_0_sep=0
wavecode_0_bSpectrum={spektrum}
wavecode_0_bUseDots=1
wavecode_0_bDrawThick=1
wavecode_0_bAdditive=0
wavecode_0_scaling={scaling}
wavecode_0_smoothing={smoothing}
wavecode_0_r=1.000
wavecode_0_g=1.000
wavecode_0_b=1.000
wavecode_0_a=1.000
wave_0_per_point1=x = sample;
wave_0_per_point2=y = 0.1 + {k}*(value1+value2);
"""


def preset(name: str, **p) -> Path:
    ziel = OUT / "presets" / f"{name}.milk"
    ziel.parent.mkdir(parents=True, exist_ok=True)
    ziel.write_text(KOPF.format(**p), encoding="latin-1")
    return ziel


def ref(milk: Path, frames: int, size: str) -> np.ndarray:
    root = cm.refwurzel_bauen(OUT / "refroot")
    shutil.copy2(milk, root / milk.name)
    out = OUT / "ref"
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(cm.REF_EXE), str(root / milk.name), "--frames", str(frames), "--size",
                    size, "--out", str(out), "--zufall-lumiviz", "--mesh", "32x24"],
                   capture_output=True)
    return cm.load_rgb(out / f"{milk.stem}_ref.bmp")


def lumi(milk: Path, frames: int, size: str) -> np.ndarray:
    return cm.load_rgb(cm.run_lumi(milk, frames, size, OUT / "lumi", "klassisch", False, 300))


def profil(img: np.ndarray) -> np.ndarray:
    """Hoehe der Kurve je Bildspalte, 0 = unten, 1 = oben (NaN = nichts)."""
    lum = img.mean(axis=2)
    h = lum.shape[0]
    ys = np.arange(h)[:, None]
    gewicht = (lum > 0.5).astype(float)
    s = gewicht.sum(axis=0)
    with np.errstate(invalid="ignore", divide="ignore"):
        y = (gewicht * ys).sum(axis=0) / s
    return 1.0 - y / (h - 1)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--size", default="640x480")
    a = ap.parse_args()
    proben = [
        ("spektrum", dict(samples=128, spektrum=1, scaling="3.54480", smoothing="0.13000", k=0.4)),
        ("wellenform", dict(samples=128, spektrum=0, scaling="1.00000", smoothing="0.00000", k=0.2)),
    ]
    for name, p in proben:
        milk = preset(f"Spektrum_{name}", **p)
        r, l = ref(milk, a.frames, a.size), lumi(milk, a.frames, a.size)
        pr, pl = profil(r), profil(l)
        gueltig = ~np.isnan(pr) & ~np.isnan(pl)
        print(f"{name:10s}: Spalten mit Kurve Original {int((~np.isnan(pr)).sum())}, "
              f"LumiViz {int((~np.isnan(pl)).sum())}; mittlere Hoehe Original "
              f"{np.nanmean(pr):.3f}, LumiViz {np.nanmean(pl):.3f}; "
              f"Hoehen-Abweichung {np.abs(pr[gueltig] - pl[gueltig]).mean() if gueltig.any() else float('nan'):.3f}",
              flush=True)
        bogen = np.concatenate([r, np.ones((r.shape[0], 4, 3)) * 0.5, l], axis=1)
        Image.fromarray((bogen * 255).astype(np.uint8)).save(OUT / f"{name}.png")
        # Profil in Achteln fuer den Ueberblick
        teile = np.array_split(np.arange(len(pr)), 8)
        print("   Achtel Original: " + " ".join(f"{np.nanmean(pr[t]):.2f}" for t in teile))
        print("   Achtel LumiViz : " + " ".join(f"{np.nanmean(pl[t]):.2f}" for t in teile))
    return 0


if __name__ == "__main__":
    sys.exit(main())
