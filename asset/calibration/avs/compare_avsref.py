# -*- coding: utf-8 -*-
"""Diff-Harness: AvsRef (originaler vis_avs-Kern) vs. AvsStandalone (LumiViz).

Rendert jedes Kalibrier-.avs mit BEIDEN Renderern (gleiche Frames, gleiche
Groesse, byte-identisches synthetisches Audio) und vergleicht die Endbilder:
mean-RGB-/Luma-Differenzen plus Seite-an-Seite-Montage (ref | lumiviz | diff)
je Preset. Ergebnis: Konsolen-Tabelle + report.md im Ausgabeverzeichnis.

Aufruf:
  python compare_avsref.py                     # alle s*-Kalibrier-Presets
  python compare_avsref.py --tests             # zusaetzlich ../tests/*.avs (P-Presets)
  python compare_avsref.py pfad/zu/preset.avs  # einzelne Presets
  python compare_avsref.py --frames 120 --size 320x240 --out DIR

Hinweise:
- AvsStandalone braucht ein SICHTBARES Fenster (Merkregel S44) — Fenster
  blitzen kurz auf. QT_ENABLE_HIGHDPI_SCALING=0 haelt logisch==physisch.
- Beide Seiten erkennen Beats selbst (Original-Detektor vs. bpm.cpp-Port) —
  bei Beat-Drift koennen Einzelframes abweichen; Urteil primaer ueber die
  Montagen und die mean/max-Metriken.
- Presets mit rand() sind zwischen Laeufen nicht bit-stabil.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).parent
LUMI_EXE = (ROOT / "../../../out/build/windows-ninja-release-clang/exec/"
            "AvsStandalone/bin/Release/AvsStandalone.exe").resolve()
REF_EXE = (ROOT / "../../../tools/AvsRef/build/Release/AvsRef.exe").resolve()

STATS_RE = re.compile(
    r"mean RGB=\(([\d.]+), ([\d.]+), ([\d.]+)\), Luma min=([\d.]+) max=([\d.]+)")


def run_ref(avs: Path, frames: int, size: str, out: Path) -> Path:
    """AvsRef rendern; liefert den BMP-Pfad des letzten Frames."""
    proc = subprocess.run(
        [str(REF_EXE), str(avs), "--frames", str(frames), "--size", size,
         "--out", str(out)],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=300)
    if proc.returncode != 0:
        raise RuntimeError(f"AvsRef rc={proc.returncode}\n{proc.stdout}{proc.stderr}")
    base = avs.name.replace(".", "_")
    bmp = out / f"{base}_f{frames:04d}_ref.bmp"
    if not bmp.exists():
        raise RuntimeError(f"AvsRef-BMP fehlt: {bmp}\n{proc.stdout}")
    return bmp


def run_lumi(avs: Path, frames: int, size: str, out: Path) -> Path:
    """AvsStandalone --auto rendern; liefert den PNG-Pfad des Screenshots."""
    import os
    env = dict(os.environ)
    env["QT_ENABLE_HIGHDPI_SCALING"] = "0"  # logische == physische Pixel
    proc = subprocess.run(
        [str(LUMI_EXE), str(avs), "--auto", "--frames", str(frames),
         "--size", size, "--out", str(out)],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=300, env=env)
    if proc.returncode != 0:
        raise RuntimeError(f"AvsStandalone rc={proc.returncode}\n"
                           f"{proc.stdout}{proc.stderr}")
    base = avs.name.replace(".", "_")
    png = out / f"{base}_auto.png"
    if not png.exists():
        raise RuntimeError(f"AvsStandalone-PNG fehlt: {png}\n{proc.stdout}")
    return png


def load_rgb(path: Path) -> np.ndarray:
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) / 255.0


def luma(img: np.ndarray) -> np.ndarray:
    return 0.2126 * img[..., 0] + 0.7152 * img[..., 1] + 0.0722 * img[..., 2]


def compare(ref: np.ndarray, lumi: np.ndarray) -> dict:
    d = {}
    d["mean_ref"] = ref.mean(axis=(0, 1))
    d["mean_lumi"] = lumi.mean(axis=(0, 1))
    d["d_mean"] = float(np.abs(d["mean_ref"] - d["mean_lumi"]).max())
    d["d_maxluma"] = float(abs(luma(ref).max() - luma(lumi).max()))
    d["mae"] = float(np.abs(ref - lumi).mean())
    return d


def montage(ref: np.ndarray, lumi: np.ndarray, out_png: Path) -> None:
    """ref | lumiviz | absdiff (Diff 4x verstaerkt) nebeneinander."""
    diff = np.clip(np.abs(ref - lumi) * 4.0, 0.0, 1.0)
    gap = np.ones((ref.shape[0], 4, 3)) * 0.5
    row = np.concatenate([ref, gap, lumi, gap, diff], axis=1)
    Image.fromarray((row * 255).astype(np.uint8)).save(out_png)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("presets", nargs="*", type=Path)
    ap.add_argument("--tests", action="store_true",
                    help="auch ../tests/*.avs (P-Presets) vergleichen")
    ap.add_argument("--frames", type=int, default=120)
    ap.add_argument("--size", default="320x240")
    ap.add_argument("--out", type=Path,
                    default=ROOT / "../../../out/avsref_compare")
    args = ap.parse_args()

    for exe, name in ((REF_EXE, "AvsRef (tools/AvsRef, -A Win32 bauen)"),
                      (LUMI_EXE, "AvsStandalone (build-ninja-release-clang)")):
        if not exe.exists():
            print(f"FEHLER: {name} fehlt: {exe}")
            return 2

    presets = list(args.presets)
    if not presets:
        presets = sorted(ROOT.glob("s*/**/*.avs"))
        if args.tests:
            presets += sorted((ROOT / "../tests").glob("*.avs"))
    if not presets:
        print("FEHLER: keine Presets gefunden")
        return 2

    out: Path = args.out.resolve()
    (out / "ref").mkdir(parents=True, exist_ok=True)
    (out / "lumi").mkdir(exist_ok=True)
    (out / "montage").mkdir(exist_ok=True)

    rows = []
    failures = 0
    for avs in presets:
        rel = avs.name
        try:
            ref_img = load_rgb(run_ref(avs, args.frames, args.size, out / "ref"))
            lumi_img = load_rgb(run_lumi(avs, args.frames, args.size, out / "lumi"))
            if ref_img.shape != lumi_img.shape:
                raise RuntimeError(
                    f"Groessen ungleich: ref{ref_img.shape} vs lumi{lumi_img.shape}"
                    " — DPI-Skalierung? (QT_ENABLE_HIGHDPI_SCALING)")
            d = compare(ref_img, lumi_img)
            mont = out / "montage" / f"{avs.stem}.png"
            montage(ref_img, lumi_img, mont)
            # Schwellen: bewusst grob (GL- vs. Software-Rasterizer rastern
            # Linien verschieden) — Feinurteil ueber die Montage
            urteil = "OK" if (d["d_mean"] <= 0.02 and d["d_maxluma"] <= 0.10
                              and d["mae"] <= 0.03) else "PRUEFEN"
            rows.append((rel, d["d_mean"], d["d_maxluma"], d["mae"], urteil))
            print(f"  {urteil:7s} {rel}  dMean={d['d_mean']:.3f} "
                  f"dMaxLuma={d['d_maxluma']:.3f} MAE={d['mae']:.3f}")
        except Exception as e:  # noqa: BLE001 — je Preset weitermachen
            failures += 1
            rows.append((rel, None, None, None, "FEHLER"))
            print(f"  FEHLER  {rel}: {e}")

    report = out / "report.md"
    with report.open("w", encoding="utf-8") as f:
        f.write(f"# AvsRef-Vergleich — {args.frames} Frames, {args.size}\n\n")
        f.write("| Preset | dMean | dMaxLuma | MAE | Urteil |\n|---|---|---|---|---|\n")
        for rel, dm, dl, mae, urteil in rows:
            if dm is None:
                f.write(f"| {rel} | – | – | – | {urteil} |\n")
            else:
                f.write(f"| {rel} | {dm:.3f} | {dl:.3f} | {mae:.3f} | {urteil} |\n")
        f.write("\nMontagen (ref | lumiviz | 4x-Diff): `montage/`\n")
    print(f"\nReport: {report}")
    pruefen = sum(1 for r in rows if r[4] != "OK")
    print(f"{len(rows) - pruefen}/{len(rows)} OK, {pruefen} zu pruefen"
          f" (davon {failures} Fehler)")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
