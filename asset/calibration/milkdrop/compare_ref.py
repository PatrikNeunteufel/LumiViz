#!/usr/bin/env python3
"""Ground-Truth-Vergleich: MilkdropRef (Original-Kern) vs. LumiViz-Triage (S63).

Fuer jedes Problem-Preset aus klassen.json (SCHWARZ/MONOCHROM/SCHWACH):
LumiViz-Screenshot (shots/<stem>_auto.png) gegen Referenz
(ref_shots/<stem>_ref.bmp) stellen, Metriken beider Seiten messen und urteilen:

  PRESET-IST-SO   Referenz zeigt dasselbe Bildverhalten -> kein Port-Bug
  PORT-BUG        Referenz ist lebendig, LumiViz nicht -> Kalibrier-Befund
  PRUEFEN         uneindeutig -> Montage ansehen

Ausgabe: VERGLEICH.md + montage_vergleich_*.png (Paare: links LumiViz,
rechts Referenz).
"""

from __future__ import annotations

import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

REPO = Path(__file__).resolve().parents[3]
DIR = REPO / "out/milkdrop_triage"
THUMB = (256, 192)
LABEL_H = 14
PAIRS_PER_ROW = 3


def measure(path: Path) -> dict | None:
    if not path.exists():
        return None
    img = np.asarray(Image.open(path).convert("RGB"), dtype=np.float32) / 255.0
    r, g, b = img[..., 0], img[..., 1], img[..., 2]
    luma = 0.2126 * r + 0.7152 * g + 0.0722 * b
    quant = (img * 15).astype(np.uint8).reshape(-1, 3)
    _, counts = np.unique(quant, axis=0, return_counts=True)
    return {
        "lumaMean": float(luma.mean()),
        "lumaStd": float(luma.std()),
        "lumaMax": float(luma.max()),
        "modalFrac": float(counts.max() / quant.shape[0]),
    }


def is_black(m: dict) -> bool:
    return m["lumaMax"] < 0.05


def is_flat(m: dict) -> bool:
    return m["modalFrac"] > 0.90 and m["lumaStd"] < 0.03


def is_weak(m: dict) -> bool:
    return m["lumaMean"] < 0.01 or m["lumaStd"] < 0.015


def verdict(ours: dict, ref: dict) -> str:
    """Grobe Automatik — die Montage bleibt das letzte Wort (§9)."""
    ours_dead = is_black(ours) or is_flat(ours) or is_weak(ours)
    ref_dead = is_black(ref) or is_flat(ref) or is_weak(ref)
    if ours_dead and ref_dead:
        return "PRESET-IST-SO"
    if ours_dead and not ref_dead:
        return "PORT-BUG"
    return "PRUEFEN"


def main() -> int:
    klassen = json.loads((DIR / "klassen.json").read_text(encoding="utf-8"))
    problem: list[tuple[str, str]] = []
    for cls in ("SCHWARZ", "MONOCHROM", "SCHWACH"):
        problem += [(cls, name) for name in klassen.get(cls, [])]

    rows = []
    pairs: dict[str, list[tuple[str, Path, Path]]] = {"PORT-BUG": [], "PRESET-IST-SO": [],
                                                      "PRUEFEN": [], "REF-FEHLT": []}
    for cls, name in problem:
        stem = Path(name).stem
        ours_png = DIR / "shots" / f"{stem}_auto.png"
        ref_bmp = DIR / "ref_shots" / f"{stem}_ref.bmp"
        ours = measure(ours_png)
        ref = measure(ref_bmp)
        if ours is None or ref is None:
            rows.append((cls, name, ours, ref, "REF-FEHLT"))
            pairs["REF-FEHLT"].append((stem, ours_png, ref_bmp))
            continue
        v = verdict(ours, ref)
        rows.append((cls, name, ours, ref, v))
        pairs[v].append((stem, ours_png, ref_bmp))

    # --- Montagen: Paare links LumiViz / rechts Referenz -----------------------------
    for v, items in pairs.items():
        if not items or v == "REF-FEHLT":
            continue
        cell_w = THUMB[0] * 2 + 4
        cell_h = THUMB[1] + LABEL_H
        rows_n = (len(items) + PAIRS_PER_ROW - 1) // PAIRS_PER_ROW
        canvas = Image.new("RGB", (PAIRS_PER_ROW * (cell_w + 8), rows_n * cell_h),
                           (24, 24, 24))
        draw = ImageDraw.Draw(canvas)
        for i, (stem, ours_png, ref_bmp) in enumerate(items):
            x = (i % PAIRS_PER_ROW) * (cell_w + 8)
            y = (i // PAIRS_PER_ROW) * cell_h
            if ours_png.exists():
                canvas.paste(Image.open(ours_png).convert("RGB").resize(THUMB), (x, y))
            if ref_bmp.exists():
                canvas.paste(Image.open(ref_bmp).convert("RGB").resize(THUMB),
                             (x + THUMB[0] + 4, y))
            draw.text((x + 2, y + THUMB[1]), f"{stem[:40]}  (L=LumiViz R=Ref)",
                      fill=(220, 220, 220))
        out = DIR / f"montage_vergleich_{v.lower().replace('-', '_')}.png"
        canvas.save(out)

    # --- Report -----------------------------------------------------------------------
    lines = ["# MilkdropRef-Ground-Truth-Vergleich (32 Problem-Presets)", ""]
    counts = {v: sum(1 for r in rows if r[4] == v)
              for v in ("PORT-BUG", "PRESET-IST-SO", "PRUEFEN", "REF-FEHLT")}
    lines.append("| Urteil | Anzahl |")
    lines.append("|---|---|")
    for v, n in counts.items():
        lines.append(f"| {v} | {n} |")
    lines.append("")
    for v in ("PORT-BUG", "PRUEFEN", "PRESET-IST-SO", "REF-FEHLT"):
        sel = [r for r in rows if r[4] == v]
        if not sel:
            continue
        lines.append(f"## {v} ({len(sel)})")
        lines.append("")
        lines.append("| Klasse | Preset | LumiViz mean/std/max | Ref mean/std/max |")
        lines.append("|---|---|---|---|")
        for cls, name, ours, ref, _ in sel:
            fo = (f"{ours['lumaMean']:.3f}/{ours['lumaStd']:.3f}/{ours['lumaMax']:.3f}"
                  if ours else "—")
            fr = (f"{ref['lumaMean']:.3f}/{ref['lumaStd']:.3f}/{ref['lumaMax']:.3f}"
                  if ref else "—")
            lines.append(f"| {cls} | `{name}` | {fo} | {fr} |")
        lines.append("")

    (DIR / "VERGLEICH.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"[Vergleich] {DIR / 'VERGLEICH.md'}")
    for v, n in counts.items():
        print(f"  {v}: {n}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
