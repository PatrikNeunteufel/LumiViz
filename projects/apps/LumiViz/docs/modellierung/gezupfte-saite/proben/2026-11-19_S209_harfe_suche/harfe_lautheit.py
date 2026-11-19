"""S209 - Harfe: eine Fassung (Bauplan aus harfe/<name>.lvinstbodyplan, 17 Hoerorte) bis 5 kHz rechnen und je Taste
38-104 die Lautheit nach KQ36 a messen (16 Hoerorte, ohne Mitschwingen); mit c > 0 dazu die besaitete Fassung
(Saiten als Federn im Modenraum, harfe_saiten.besaitet).
Aufruf vom Projekt-Root: python -X utf8 -B harfe_lautheit.py <name> [c ...]"""
import json
import pathlib
import shutil
import subprocess
import sys

import harfe_saiten as sa
import harfe_suche as s

P = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/"
name = sys.argv[1]
z = s.AUS / "laut" / name.replace(" ", "_").replace(".", "")
z.mkdir(parents=True, exist_ok=True)
plan = z / "concert_harp.lvinstbodyplan"
shutil.copyfile(s.AUS / f"{name}.lvinstbodyplan", plan)
if not (z / "concert_harp.lvinstbody").exists():
    r = subprocess.run([str(s.TOOL), str(plan), "--to", "5000"], capture_output=True, text=True, encoding="utf-8", errors="replace")
    assert r.returncode == 0, r.stdout[-500:]


def miss(ordner, titel):
    tsv = ordner / "kq36.tsv"
    subprocess.run([sys.executable, "-X", "utf8", "-B", P + "2026-11-19_S207_harfe_viele_orte.py", str(ordner), str(tsv)], check=True, capture_output=True)
    r = subprocess.run([sys.executable, "-X", "utf8", "-B", P + "2026-11-19_S207_tu_auswerten.py", str(tsv)], capture_output=True, text=True, encoding="utf-8")
    zeilen = [l.split("\t") for l in tsv.read_text(encoding="utf-8").splitlines() if l.strip()]
    print(f"== {titel}: " + r.stdout.splitlines()[1].strip(), flush=True)
    print("   " + " ".join(f"{int(float(l[0]))}:{float(l[1]):.1f}" for l in zeilen), flush=True)


miss(z, name)
k = json.loads((z / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
for c in [float(v) for v in sys.argv[2:]]:
    zs = pathlib.Path(str(z) + f"_saiten{c}")
    zs.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(plan, zs / "concert_harp.lvinstbodyplan")
    (zs / "concert_harp.lvinstbody").write_text(json.dumps(sa.besaitet(k, c)), encoding="utf-8", newline="\n")
    miss(zs, f"{name} + Saiten {c}")
