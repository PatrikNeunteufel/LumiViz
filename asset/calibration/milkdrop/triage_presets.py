#!/usr/bin/env python3
"""Triage-Batchlauf ueber eine MilkDrop-Preset-Sammlung (Session 63).

Faehrt MilkdropStandalone --auto je Preset als EIGENEN Prozess (ein Absturz
oder Haenger kostet nur dieses Preset), sammelt Konsolen-Statistik + Screenshot
und schreibt je Preset eine JSONL-Zeile. Laeufe sind resuembar: bereits
vermessene Presets werden uebersprungen.

Zweitpass fuer schwarz endende Presets: Kurzlauf mit --frames 30 — war da
anfangs etwas zu sehen, ist die Klasse VERBLASST statt SCHWARZ.

Aufruf (Repo-Root):
  python asset/calibration/milkdrop/triage_presets.py [--presets DIR] [--out DIR]
         [--frames N] [--size WxH] [--limit N]
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
DEFAULT_EXE = (REPO / "out/build/windows-ninja-release-clang/exec/MilkdropStandalone"
                    / "bin/Release/MilkdropStandalone.exe")
DEFAULT_PRESETS = REPO / "asset/Milkdrop3/presets"
DEFAULT_OUT = REPO / "out/milkdrop_triage"

RX_STATS = re.compile(
    r"mean RGB=\(([\d.]+), ([\d.]+), ([\d.]+)\), Luma min=([\d.]+) max=([\d.]+)")
RX_RESULT = re.compile(
    r"Ergebnis .*: custom=(\S+?)(?: \(nicht erwartet\))?, GL-Fehler=(.*), schwarz=(\S+)")
RX_CUSTOM_SRC = re.compile(r"warpCustomSrc=(\d+) Zeichen, compCustomSrc=(\d+) Zeichen")


def run_one(exe: Path, preset: Path, out_dir: Path, frames: int, size: str,
            timeout_s: int) -> dict:
    """Ein Preset vermessen; liefert die JSONL-Zeile als dict."""
    row: dict = {"preset": preset.name, "frames": frames}
    t0 = time.monotonic()
    try:
        proc = subprocess.run(
            [str(exe), str(preset), "--auto", "--frames", str(frames),
             "--out", str(out_dir), "--size", size],
            capture_output=True, text=True, errors="replace", timeout=timeout_s)
        row["exit"] = proc.returncode
        out = proc.stdout + proc.stderr
    except subprocess.TimeoutExpired as exc:
        row["exit"] = "TIMEOUT"
        out = ((exc.stdout or b"").decode(errors="replace") if isinstance(exc.stdout, bytes)
               else (exc.stdout or ""))
    row["dauer_s"] = round(time.monotonic() - t0, 1)

    if m := RX_STATS.search(out):
        row["meanRGB"] = [float(m.group(1)), float(m.group(2)), float(m.group(3))]
        row["lumaMin"] = float(m.group(4))
        row["lumaMax"] = float(m.group(5))
    if m := RX_RESULT.search(out):
        row["custom"] = m.group(1)
        row["glFehler"] = m.group(2).strip()
        row["schwarz"] = m.group(3)
    if m := RX_CUSTOM_SRC.search(out):
        row["warpSrcLen"] = int(m.group(1))
        row["compSrcLen"] = int(m.group(2))
    row["importZeilen"] = [ln.strip() for ln in out.splitlines()
                           if ln.startswith("[Import]")]
    if "LADEN FEHLGESCHLAGEN" in out:
        row["ladefehler"] = True
    return row


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    ap.add_argument("--presets", type=Path, default=DEFAULT_PRESETS)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--frames", type=int, default=240)
    ap.add_argument("--size", default="640x480")
    ap.add_argument("--timeout", type=int, default=120, help="Sekunden je Preset")
    ap.add_argument("--limit", type=int, default=0, help="nur die ersten N (Testlauf)")
    ap.add_argument("--nur-schwarz-zweitpass", action="store_true",
                    help="nur den 30-Frame-Zweitpass fuer schwarz endende Presets fahren")
    args = ap.parse_args()

    shots = args.out / "shots"
    shots.mkdir(parents=True, exist_ok=True)
    results_path = args.out / "results.jsonl"

    done: dict[str, dict] = {}
    if results_path.exists():
        for line in results_path.read_text(encoding="utf-8").splitlines():
            if line.strip():
                row = json.loads(line)
                done[row["preset"]] = row

    presets = sorted(args.presets.glob("*.milk"), key=lambda p: p.name.lower())
    if args.limit:
        presets = presets[: args.limit]

    if not args.nur_schwarz_zweitpass:
        todo = [p for p in presets if p.name not in done]
        print(f"[Triage] {len(presets)} Presets, {len(done)} fertig, {len(todo)} offen",
              flush=True)
        with results_path.open("a", encoding="utf-8") as sink:
            for i, preset in enumerate(todo, 1):
                row = run_one(args.exe, preset, shots, args.frames, args.size,
                              args.timeout)
                sink.write(json.dumps(row, ensure_ascii=False) + "\n")
                sink.flush()
                done[row["preset"]] = row
                print(f"[{i}/{len(todo)}] {preset.name}: exit={row['exit']} "
                      f"lumaMax={row.get('lumaMax', '?')} "
                      f"gl={row.get('glFehler', '?')[:40]}", flush=True)

    # Zweitpass: schwarz am Ende — war nach 30 Frames etwas zu sehen?
    early_path = args.out / "results_early.jsonl"
    early_done: set[str] = set()
    if early_path.exists():
        early_done = {json.loads(l)["preset"]
                      for l in early_path.read_text(encoding="utf-8").splitlines()
                      if l.strip()}
    schwarz = [p for p in presets
               if done.get(p.name, {}).get("lumaMax", 1.0) < 0.02
               and p.name not in early_done]
    if schwarz:
        early_shots = args.out / "shots_early"
        early_shots.mkdir(parents=True, exist_ok=True)
        print(f"[Triage] Zweitpass (30 Frames) fuer {len(schwarz)} schwarze Presets",
              flush=True)
        with early_path.open("a", encoding="utf-8") as sink:
            for i, preset in enumerate(schwarz, 1):
                row = run_one(args.exe, preset, early_shots, 30, args.size,
                              args.timeout)
                sink.write(json.dumps(row, ensure_ascii=False) + "\n")
                sink.flush()
                print(f"[früh {i}/{len(schwarz)}] {preset.name}: "
                      f"lumaMax={row.get('lumaMax', '?')}", flush=True)

    print("[Triage] fertig.", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
