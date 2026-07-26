# -*- coding: utf-8 -*-
"""Erzeugt bzw. prueft die .lvfx-Zwillinge der Kalibrier-.avs.

Jedes .avs bekommt ein .lvfx mit identischem Basisnamen: die von
AvsStandalone --dump ausgegebene uebersetzte Chain (chainToJson pur — genau
das Format, das loadChainFile liest). Die eingefrorenen Zwillinge sind der
Parser-/Translator-Pruefstand: aendert sich die Uebersetzung eines .avs,
schlaegt --verify an (gewollt bei Fixes -> Zwilling nach Review neu einfrieren,
ungewollt = Regression).

Aufruf:
  python freeze_lvfx_twins.py            # fehlende Zwillinge erzeugen
  python freeze_lvfx_twins.py --refreeze # alle Zwillinge neu einfrieren
  python freeze_lvfx_twins.py --verify   # Dump gegen Zwillinge diffen (CI-Modus)

Hinweis: AvsStandalone braucht ein SICHTBARES Fenster (Merkregel Session 44) —
beim Lauf blitzen kurz Fenster auf.
"""
import json
import subprocess
import sys
from pathlib import Path

# Konsole auf UTF-8: Preset-Namen enthalten Nicht-ASCII, cp1252 wuerfe beim
# Ausgeben eine Ausnahme und risse den Lauf mit (Befund S50).
for _stream in (sys.stdout, sys.stderr):
    if hasattr(_stream, "reconfigure"):
        _stream.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).parent
EXE = (ROOT / "../../../out/build/windows-ninja-release-clang/exec/AvsStandalone/"
       "bin/Release/AvsStandalone.exe").resolve()


def dump_chain(avs: Path) -> dict:
    """Ein Preset laden, [Chain]-JSON aus stdout extrahieren."""
    proc = subprocess.run(
        [str(EXE), str(avs), "--dump", "--auto", "--frames", "2",
         "--size", "320x240"],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=120)
    out = proc.stdout
    start = out.find("[Chain]")
    if start < 0:
        raise RuntimeError(f"{avs.name}: kein [Chain]-Block im Dump\n{out}")
    start = out.index("\n", start) + 1
    end = out.find("[Standalone]", start)
    text = out[start:end if end > 0 else len(out)].strip()
    warnings = [ln for ln in out.splitlines()
                if ln.startswith("[Import]") and "ℹ" not in ln]
    for w in warnings:
        print(f"    WARNUNG {avs.name}: {w}")
    return json.loads(text)


def canonical(obj: dict) -> str:
    return json.dumps(obj, indent=2, sort_keys=True, ensure_ascii=False)


def main() -> int:
    refreeze = "--refreeze" in sys.argv
    verify = "--verify" in sys.argv
    if not EXE.exists():
        print(f"FEHLER: AvsStandalone.exe fehlt: {EXE}")
        return 2

    presets = sorted(ROOT.glob("s*/**/*.avs")) + sorted(ROOT.glob("matrix/**/*.avs"))
    if not presets:
        print("FEHLER: keine .avs gefunden — erst make_calibration_presets.py")
        return 2

    failures = 0
    for avs in presets:
        twin = avs.with_suffix(".lvfx")
        rel = avs.relative_to(ROOT)
        if verify:
            if not twin.exists():
                print(f"  FEHLT {rel.with_suffix('.lvfx')}")
                failures += 1
                continue
            current = canonical(dump_chain(avs))
            frozen = canonical(json.loads(twin.read_text(encoding="utf-8")))
            if current != frozen:
                print(f"  DIFF {rel} — Uebersetzung weicht vom Zwilling ab")
                failures += 1
            else:
                print(f"  ok   {rel}")
        else:
            if twin.exists() and not refreeze:
                print(f"  uebersprungen (existiert): {rel.with_suffix('.lvfx')}")
                continue
            chain = dump_chain(avs)
            twin.write_text(canonical(chain) + "\n", encoding="utf-8")
            print(f"  eingefroren: {rel.with_suffix('.lvfx')}")

    if verify:
        print(f"\n{'GRUEN' if failures == 0 else 'ROT'}: "
              f"{len(presets) - failures}/{len(presets)} Zwillinge stimmen")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
