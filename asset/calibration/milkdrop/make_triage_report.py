#!/usr/bin/env python3
"""Fehlerklassen-Report aus dem Triage-Batchlauf (Session 63).

Liest out/milkdrop_triage/results.jsonl (+ results_early.jsonl vom
30-Frame-Zweitpass), vermisst die Screenshots nach (raeumliche Varianz,
Buntheit, Modalfarben-Anteil) und klassifiziert jedes Preset:

  LADEFEHLER  Import scheiterte oder Prozess crashte/hing
  GL-FEHLER   Renderpfad meldet einen GL-Fehler
  SCHWARZ     Endbild schwarz, Fruehbild (Frame 30) auch schwarz
  VERBLASST   Endbild schwarz, Fruehbild zeigte etwas (Energie-Verlust)
  MONOCHROM   hell, aber praktisch einfarbig (kaum raeumliche Struktur)
  SCHWACH     Struktur vorhanden, aber sehr dunkel/kontrastarm
  OK          lebendiges Bild

Ausgabe: REPORT.md + Montage-PNGs je Klasse in out/milkdrop_triage/.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

REPO = Path(__file__).resolve().parents[3]
DEFAULT_DIR = REPO / "out/milkdrop_triage"

THUMB = (192, 144)
COLS = 8


def load_jsonl(path: Path) -> dict[str, dict]:
    rows: dict[str, dict] = {}
    if path.exists():
        for line in path.read_text(encoding="utf-8").splitlines():
            if line.strip():
                row = json.loads(line)
                rows[row["preset"]] = row
    return rows


def measure_png(path: Path) -> dict:
    """Bildmetriken: Luma-Statistik, Buntheit, Modalfarben-Anteil."""
    img = np.asarray(Image.open(path).convert("RGB"), dtype=np.float32) / 255.0
    r, g, b = img[..., 0], img[..., 1], img[..., 2]
    luma = 0.2126 * r + 0.7152 * g + 0.0722 * b
    # Hasler-Suesstrunk-Buntheit
    rg = r - g
    yb = 0.5 * (r + g) - b
    colorfulness = float(np.sqrt(rg.std() ** 2 + yb.std() ** 2)
                         + 0.3 * np.sqrt(rg.mean() ** 2 + yb.mean() ** 2))
    # Anteil der Pixel nahe der haeufigsten (quantisierten) Farbe
    quant = (img * 15).astype(np.uint8)
    flat = quant.reshape(-1, 3)
    _, counts = np.unique(flat, axis=0, return_counts=True)
    modal_frac = float(counts.max() / flat.shape[0])
    return {
        "lumaMean": float(luma.mean()),
        "lumaStd": float(luma.std()),
        "lumaMax": float(luma.max()),
        "colorfulness": colorfulness,
        "modalFrac": modal_frac,
    }


def classify(row: dict, m: dict | None, early: dict | None) -> str:
    if row.get("ladefehler") or row.get("exit") not in (0, 1):
        return "LADEFEHLER"
    if row.get("glFehler") and row["glFehler"] != "keiner":
        return "GL-FEHLER"
    if m is None:
        return "LADEFEHLER"
    if m["lumaMax"] < 0.02:
        if early is not None and early.get("lumaMax", 0.0) >= 0.02:
            return "VERBLASST"
        return "SCHWARZ"
    if m["modalFrac"] > 0.90 and m["lumaStd"] < 0.03:
        return "MONOCHROM"
    if m["lumaMean"] < 0.01 or m["lumaStd"] < 0.015:
        return "SCHWACH"
    return "OK"


def montage(names: list[str], shots: Path, out_png: Path) -> None:
    if not names:
        return
    rows = (len(names) + COLS - 1) // COLS
    label_h = 14
    cell_w, cell_h = THUMB[0], THUMB[1] + label_h
    canvas = Image.new("RGB", (COLS * cell_w, rows * cell_h), (24, 24, 24))
    draw = ImageDraw.Draw(canvas)
    for i, name in enumerate(names):
        x, y = (i % COLS) * cell_w, (i // COLS) * cell_h
        png = shots / f"{Path(name).stem}_auto.png"
        if png.exists():
            canvas.paste(Image.open(png).convert("RGB").resize(THUMB), (x, y))
        draw.text((x + 2, y + THUMB[1]), Path(name).stem[:30], fill=(220, 220, 220))
    canvas.save(out_png)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", type=Path, default=DEFAULT_DIR)
    args = ap.parse_args()

    results = load_jsonl(args.dir / "results.jsonl")
    early = load_jsonl(args.dir / "results_early.jsonl")
    shots = args.dir / "shots"

    classes: dict[str, list[str]] = defaultdict(list)
    metrics: dict[str, dict] = {}
    for name, row in sorted(results.items(), key=lambda kv: kv[0].lower()):
        png = shots / f"{Path(name).stem}_auto.png"
        m = measure_png(png) if png.exists() else None
        if m is not None:
            metrics[name] = m
        classes[classify(row, m, early.get(name))].append(name)

    # Import-Warnungs-Cluster (normalisiert: Zahlen/Namen raus)
    warn_counter: Counter[str] = Counter()
    warn_beispiel: dict[str, str] = {}
    for name, row in results.items():
        for line in row.get("importZeilen", []):
            key = line.replace("[Import]", "").strip()
            for tok in ("'", '"'):
                parts = key.split(tok)
                if len(parts) >= 3:
                    key = parts[0] + "<X>" + parts[-1]
            key = " ".join("<N>" if any(c.isdigit() for c in w) else w
                           for w in key.split())
            warn_counter[key] += 1
            warn_beispiel.setdefault(key, f"{name}: {line}")

    order = ["LADEFEHLER", "GL-FEHLER", "SCHWARZ", "VERBLASST", "MONOCHROM",
             "SCHWACH", "OK"]
    lines = ["# MilkDrop-Preset-Triage — Fehlerklassen-Report", ""]
    lines.append(f"Vermessen: **{len(results)}** Presets · Quelle "
                 f"`asset/Milkdrop3/presets` · 240 Frames je Preset, 640×480, "
                 f"synthetisches Audio (120-BPM-Puls)")
    lines.append("")
    lines.append("| Klasse | Anzahl | Anteil |")
    lines.append("|---|---|---|")
    for cls in order:
        n = len(classes.get(cls, []))
        lines.append(f"| {cls} | {n} | {100.0 * n / max(1, len(results)):.0f}% |")
    lines.append("")

    for cls in order:
        names = classes.get(cls, [])
        if not names or cls == "OK":
            continue
        lines.append(f"## {cls} ({len(names)})")
        lines.append("")
        for name in names:
            row = results[name]
            m = metrics.get(name)
            extra = []
            if m:
                extra.append(f"lumaMean={m['lumaMean']:.3f} std={m['lumaStd']:.3f} "
                             f"modal={m['modalFrac']:.2f}")
            if row.get("glFehler") and row["glFehler"] != "keiner":
                extra.append(f"GL: {row['glFehler'][:80]}")
            if row.get("custom") == "NEIN(!)":
                extra.append("CUSTOM-SHADER NICHT AKTIV")
            if name in early:
                extra.append(f"früh(30f) lumaMax={early[name].get('lumaMax', '?')}")
            lines.append(f"- `{name}` — {' · '.join(extra)}")
        lines.append("")
        montage(names, shots, args.dir / f"montage_{cls.lower().replace('-', '')}.png")

    ok_names = classes.get("OK", [])
    montage(ok_names[:64], shots, args.dir / "montage_ok_auswahl.png")
    lines.append(f"## OK ({len(ok_names)}) — Montage-Auswahl siehe "
                 f"`montage_ok_auswahl.png`")
    lines.append("")

    lines.append("## Import-Warnungs-Cluster (normalisiert)")
    lines.append("")
    lines.append("| Anzahl | Muster | Beispiel |")
    lines.append("|---|---|---|")
    for key, n in warn_counter.most_common(30):
        beispiel = warn_beispiel[key][:120].replace("|", "\\|")
        lines.append(f"| {n} | {key[:100].replace('|', '&#124;')} | {beispiel} |")
    lines.append("")

    # Klassenliste als JSON fuer Folgewerkzeuge
    (args.dir / "klassen.json").write_text(
        json.dumps({k: v for k, v in classes.items()}, ensure_ascii=False, indent=1),
        encoding="utf-8")
    (args.dir / "REPORT.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"[Report] {args.dir / 'REPORT.md'}")
    for cls in order:
        print(f"  {cls}: {len(classes.get(cls, []))}")
    return 0


if __name__ == "__main__":
    sys_exit = main()
    raise SystemExit(sys_exit)
