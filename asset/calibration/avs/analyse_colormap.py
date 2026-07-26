"""Color-Map-Kennlinie aus den Sonden-Bildern rekonstruieren (S49).

Der Color-Map-Effekt ist eine APE ohne Quelltext. Statt zu raten wird gemessen:
`make_colormap_probes.py` rendert dieselbe Farbwolke einmal ohne und einmal mit
Color Map. Aus den Pixelpaaren (Eingangsfarbe, Ausgangsfarbe) faellt die
komplette Abbildung heraus. Wichtig: die Kennlinie wird JE RENDERER abgeleitet —
so stoert es nicht, wenn die Farbwolke selbst minimal anders gezeichnet wird.

  python analyse_colormap.py            # rendert beide Seiten und vergleicht
"""
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).parent
PROBES = ROOT / "colormap_probe"
OUT = (ROOT / "../../../out/colormap_probe").resolve()
LUMI_EXE = (ROOT / "../../../out/build/windows-ninja-release-clang/exec/"
            "AvsStandalone/bin/Release/AvsStandalone.exe").resolve()
REF_EXE = (ROOT / "../../../tools/AvsRef/build/Release/AvsRef.exe").resolve()
APE_DIR = (ROOT / "../../../../VisualsPresets/avs").resolve()
SIZE = "256x256"
FRAMES = 2

KEYS = {
    "R": lambda r, g, b: r,
    "G": lambda r, g, b: g,
    "B": lambda r, g, b: b,
    "(R+G+B)/2": lambda r, g, b: np.minimum((r + g + b) // 2, 255),
    "MAX": lambda r, g, b: np.maximum(np.maximum(r, g), b),
    "(R+G+B)/3": lambda r, g, b: (r + g + b) // 3,
}


def render_ref(name: str) -> np.ndarray:
    out = OUT / "ref"
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(REF_EXE), str(PROBES / f"{name}.avs"), "--frames", str(FRAMES),
                    "--size", SIZE, "--out", str(out), "--ape-dir", str(APE_DIR)],
                   capture_output=True, text=True, timeout=300, check=True)
    return np.asarray(Image.open(out / f"{name}_avs_f{FRAMES:04d}_ref.bmp")
                      .convert("RGB")).astype(int)


def render_lumi(name: str) -> np.ndarray:
    out = OUT / "lumi"
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(LUMI_EXE), str(PROBES / f"{name}.avs"), "--auto",
                    "--frames", str(FRAMES), "--size", SIZE, "--out", str(out)],
                   capture_output=True, text=True, timeout=300, check=True)
    png = next(out.glob(f"{name}_avs*.png"))
    return np.asarray(Image.open(png).convert("RGB")).astype(int)


def lut_from_pair(src: np.ndarray, dst: np.ndarray, index: np.ndarray):
    """Fuer jeden Indexwert 0..255 die beobachtete Ausgangsfarbe (oder None)."""
    table = [None] * 256
    idx = index.reshape(-1)
    out = dst.reshape(-1, 3)
    for v in range(256):
        hit = out[idx == v]
        if len(hit) == 0:
            continue
        # Mehrdeutig waere ein Fehler in der Annahme — hier faellt er auf
        uniq = np.unique(hit, axis=0)
        table[v] = tuple(uniq[0]) if len(uniq) == 1 else ("uneindeutig", len(uniq))
    return table


def identify_key(src: np.ndarray, dst: np.ndarray, lut) -> str:
    """Welche Key-Funktion erklaert dst aus src ueber die bekannte Kennlinie?"""
    r, g, b = src[..., 0], src[..., 1], src[..., 2]
    best, bestScore = "?", -1.0
    for name, fn in KEYS.items():
        idx = np.clip(fn(r, g, b), 0, 255)
        pred = np.zeros_like(dst)
        ok = np.ones(idx.shape, bool)
        for v in range(256):
            e = lut[v]
            m = idx == v
            if not m.any():
                continue
            if e is None or isinstance(e[0], str):
                ok &= ~m
                continue
            pred[m] = e
        score = float((np.all(pred == dst, axis=-1) & ok).sum()) / max(ok.sum(), 1)
        if score > bestScore:
            best, bestScore = name, score
    return f"{best} ({bestScore * 100:.1f} % erklaert)"


def main() -> None:
    if not (LUMI_EXE.exists() and REF_EXE.exists()):
        print("FEHLER: AvsStandalone/AvsRef fehlen")
        return 2
    print(f"Sonden: {PROBES}\n")
    for side, render in (("AvsRef", render_ref), ("LumiViz", render_lumi)):
        src = render("00_rampe")
        # Kennlinie aus key0 (Index = R, deckt 0..255 ab)
        dst0 = render("01_key0_ident")
        lut = lut_from_pair(src, dst0, src[..., 0])
        known = [(v, lut[v]) for v in range(256) if lut[v] is not None]
        print(f"--- {side} ---")
        print(f"  Kennlinie schwarz->weiss, {len(known)} Stuetzwerte gemessen")
        for v in (0, 1, 32, 64, 128, 200, 254, 255):
            print(f"    idx {v:3d} -> {lut[v]}")
        for key in range(6):
            dst = render(f"01_key{key}_ident")
            print(f"  key={key}: {identify_key(src, dst, lut)}")
        # Stuetzstellen-Kennlinie (Verlauf endet bei 140)
        dst = render("02_stops_bis140")
        lut2 = lut_from_pair(src, dst, src[..., 0])
        print("  Verlauf 0/65/92/140 (key 0 = R) an ausgewaehlten Indizes:")
        for v in (0, 1, 32, 64, 65, 66, 78, 91, 92, 93, 116, 139, 140, 141, 200, 255):
            print(f"    idx {v:3d} -> {lut2[v]}")
        print()
    return 0


if __name__ == "__main__":
    sys.exit(main())
