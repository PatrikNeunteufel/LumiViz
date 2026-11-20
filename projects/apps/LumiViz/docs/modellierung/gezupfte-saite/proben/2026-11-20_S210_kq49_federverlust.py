"""S210 - Vorlage KQ49: der Verlust der Punktfedern. Kandidat der Harfe (d12-2 mu2.0 bass fest), Saiten c = 0,2.
a: Federn ohne Verlust (wie harfe_saiten.besaitet, S209).
b: die Feder zaehlt nicht - der Verlustfaktor der Mode folgt wie ohne Federn dem Gesetz des Bauplans an der neuen
   Frequenz, gemischt mit dem Anteil der Luft: eta = (1 - luft) * alpha(f) / (2 pi f) + luft / Q.
Aufruf vom Projekt-Root, LUMIVIZ_HARFE_SUCHE = .claude/handover/s209_harfe_suche/harfe"""
import json
import math
import pathlib
import shutil
import subprocess
import sys

PROBEN = pathlib.Path("projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben")
sys.path.insert(0, str(PROBEN / "2026-11-19_S209_harfe_suche"))
import harfe_saiten as sa  # noqa: E402
import harfe_suche as s  # noqa: E402

name, c = "d12-2 mu2.0 bass fest", 0.2
z = s.AUS / "laut" / name.replace(" ", "_").replace(".", "")
k = json.loads((z / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
plan = json.loads((z / "concert_harp.lvinstbodyplan").read_text(encoding="utf-8"))
d, q = plan["damping"], plan["cavity"]["q"]
a = sa.besaitet(k, c)
b = json.loads(json.dumps(a))
for m in b["modes"]:
    f = m["f"]
    m["eta"] = (1 - m["airShare"]) * (d["a"] * f * f + d["b"] * f + d["c"]) / (2 * math.pi * f) + m["airShare"] / q

print("Mode    f ohne   eta ohne |  f mit   eta a    eta b    b/a   dB an der Spitze (a gegen b)")
for i in range(8):
    o, ma, mb = k["modes"][i], a["modes"][i], b["modes"][i]
    print(f"{i + 1:3d}  {o['f']:8.1f}  {o['eta']:.4f} | {ma['f']:7.1f}  {ma['eta']:.4f}  {mb['eta']:.4f}  {mb['eta'] / ma['eta']:.2f}  "
          f"{20 * math.log10(mb['eta'] / ma['eta']):+.1f}", flush=True)
verh = [mb["eta"] / ma["eta"] for ma, mb in zip(a["modes"], b["modes"])]
print(f"alle {len(verh)} Moden: eta b / eta a von {min(verh):.3f} bis {max(verh):.3f}", flush=True)


def miss(koerper, ordner, titel):
    ordner.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(z / "concert_harp.lvinstbodyplan", ordner / "concert_harp.lvinstbodyplan")
    (ordner / "concert_harp.lvinstbody").write_text(json.dumps(koerper), encoding="utf-8", newline="\n")
    tsv = ordner / "kq36.tsv"
    subprocess.run([sys.executable, "-X", "utf8", "-B", str(PROBEN / "2026-11-19_S207_harfe_viele_orte.py"), str(ordner), str(tsv)],
                   check=True, capture_output=True)
    r = subprocess.run([sys.executable, "-X", "utf8", "-B", str(PROBEN / "2026-11-19_S207_tu_auswerten.py"), str(tsv)],
                       capture_output=True, text=True, encoding="utf-8")
    zeilen = [l.split("\t") for l in tsv.read_text(encoding="utf-8").splitlines() if l.strip()]
    print(f"== {titel}: " + r.stdout.splitlines()[1].strip(), flush=True)
    print("   " + " ".join(f"{int(float(l[0]))}:{float(l[1]):.1f}" for l in zeilen), flush=True)


basis = pathlib.Path(sys.argv[1])
miss(a, basis / "a", "a Federn ohne Verlust")
miss(b, basis / "b", "b Verlust nach dem Gesetz")
