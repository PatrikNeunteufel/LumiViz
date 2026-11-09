"""Umzug U6 (S182, Aufloesung_IC_Plan.md, AU3, AU3-1 a): Hoerproben nach `…\\cmake\\samples\\Hoerproben\\`.

    python tools/hoerproben_umzug_s182.py kopiere     # kopieren, je Datei SHA-256 pruefen
    python tools/hoerproben_umzug_s182.py loesche     # Quellen in LumiViz loeschen (nur, wenn das Ziel gleich ist)

a) IC `hoertest/listen/<klasse>/` ganz -> `Hoerproben\\IC\\<klasse>\\`; `liste.json`, `klaenge.sha256` zusaetzlich nach
   LumiViz `hoertest/listen/ic/<klasse>/` (versioniert, AU3-1 a). In IC wird nichts geloescht.
b) LumiViz `hoertest/listen/<liste>/` mit `klaenge/` -> `Hoerproben\\LumiViz\\<liste>\\` (Liste, Abdruecke, Klaenge); im
   Repo bleiben `liste.json`, `klaenge.sha256`; `loesche` entfernt dort nur `klaenge/` (nicht versioniert).
c) LumiViz `.claude/handover/hoerproben_*` und `salamander_31d3`, `salamander_31d6` unveraendert ->
   `Hoerproben\\LumiViz\\<ordner>\\`; `loesche` entfernt die Quelle.
d) HoerTest_Helper `out/build/windows-ninja-release-clang/exec/Hoertest/bin/Release/` -> `Hoerproben\\_werkzeug\\Hoertest\\`
   mit `FASSUNG.txt` (Commit von HoerTest_Helper).
Schreibt `Hoerproben\\UMZUG_S182.tsv` (sha256, Ziel, Quelle).
"""

import hashlib
import shutil
import subprocess
import sys
from pathlib import Path

C = Path(__file__).resolve().parents[2]
R, IC, HT = C / "LumiViz", C / "InstrumentsCalculation", C / "HörTest_Helper"
ZIEL = C / "samples" / "Hoerproben"
LISTE = ZIEL / "UMZUG_S182.tsv"
HANDOVER = R / ".claude" / "handover"
HT_EXE = HT / "out/build/windows-ninja-release-clang/exec/Hoertest/bin/Release"


def sha(p):
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for b in iter(lambda: f.read(1 << 20), b""):
            h.update(b)
    return h.hexdigest()


def plan():
    """(Quelle, Ziel, loeschbar) je Datei."""
    aus = []
    for k in sorted((IC / "hoertest/listen").iterdir()):
        for p in sorted(k.rglob("*")):
            if p.is_file():
                aus.append((p, ZIEL / "IC" / k.name / p.relative_to(k), False))
                if p.parent == k and p.name in ("liste.json", "klaenge.sha256"):
                    aus.append((p, R / "hoertest/listen/ic" / k.name / p.name, False))
    for l in sorted((R / "hoertest/listen").iterdir()):
        if not (l / "klaenge").is_dir():
            continue
        for p in sorted(l.rglob("*")):
            if p.is_file():
                im_repo = p.parent == l
                aus.append((p, ZIEL / "LumiViz" / l.name / p.relative_to(l), not im_repo))
    for o in sorted(list(HANDOVER.glob("hoerproben_*")) + [HANDOVER / "salamander_31d3", HANDOVER / "salamander_31d6"]):
        for p in sorted(o.rglob("*")):
            if p.is_file():
                aus.append((p, ZIEL / "LumiViz" / o.name / p.relative_to(o), True))
    for p in sorted(HT_EXE.rglob("*")):
        if p.is_file():
            aus.append((p, ZIEL / "_werkzeug" / "Hoertest" / p.relative_to(HT_EXE), False))
    return aus


def kopiere():
    zeilen = ["sha256\tziel\tquelle"]
    n = b = 0
    for q, z, _ in plan():
        h = sha(q)
        if z.exists() and sha(z) != h:
            raise SystemExit(f"Ziel mit anderem Inhalt: {z}")
        z.parent.mkdir(parents=True, exist_ok=True)
        if not z.exists():
            shutil.copy2(q, z)
        if sha(z) != h:
            raise SystemExit(f"Kopie weicht ab: {z}")
        zeilen.append(f"{h}\t{z}\t{q}")
        n += 1
        b += q.stat().st_size
    commit = subprocess.run(["git", "-C", str(HT), "log", "-1", "--format=%H %ci %s"], capture_output=True, text=True,
                            encoding="utf-8").stdout.strip()
    (ZIEL / "_werkzeug" / "Hoertest" / "FASSUNG.txt").write_text(
        f"Hoertest.exe aus HoerTest_Helper, Bau windows-ninja-release-clang (Release)\nCommit: {commit}\n"
        f"Kopiert: S182 (LumiViz), tools/hoerproben_umzug_s182.py\n", encoding="utf-8", newline="\n")
    LISTE.write_text("\n".join(zeilen) + "\n", encoding="utf-8", newline="\n")
    print(f"{n} Dateien kopiert ({b / 1e6:.0f} MB), je SHA-256 gleich; HoerTest_Helper {commit[:12]}")


def loesche():
    weg = 0
    ordner = set()
    for q, z, loeschbar in plan():
        if not loeschbar or not q.exists():
            continue
        if sha(q) != sha(z):
            raise SystemExit(f"nicht gleich, nichts geloescht ab: {q}")
        q.unlink()
        weg += 1
        ordner.add(q.parent)
    for o in sorted(ordner, key=lambda p: len(p.parts), reverse=True):
        while o != R and o.exists() and not any(o.iterdir()) and o != HANDOVER and o != R / "hoertest/listen":
            o.rmdir()
            o = o.parent
    print(f"{weg} Dateien in LumiViz geloescht")


if __name__ == "__main__":
    {"kopiere": kopiere, "loesche": loesche}[sys.argv[1]]()
