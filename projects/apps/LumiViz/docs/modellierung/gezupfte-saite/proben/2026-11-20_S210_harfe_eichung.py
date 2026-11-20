"""S210 - Harfe: Eichung des Holzes am freien Boden ohne Leisten mit der Dicke des Kandidaten (KQ47 a).

Der Boden allein (131 cm, 41 -> 10 cm, Faser quer), Rand frei, ohne Leisten und ohne Luft, gegen Bell 1988 Tab. 3.1
((0,n), n = 6 ... 12, Mittel dreier Boeden) und Tab. 3.2 ((2,0), (2,2), (2,3), (2,4)). Paragraph 7.1 des Harfenberichts
eichte mit 10 -> 3 mm; hier mit der Dicke aus den Schaltern (Vorgabe 12 -> 2 mm, dazu 1 mm Furnier).

Aufruf vom Projekt-Root:
    python -X utf8 -B <diese datei> lauf  [Schalter]      eine Rechnung
    python -X utf8 -B <diese datei> fit   [Schalter]      E quer, E laengs, G an die elf Moden anpassen
    python -X utf8 -B <diese datei> netz  [Schalter]      dieselbe Rechnung am Netz fuer 520 / 1500 / 5000 Hz
Schalter: --bass <mm> (12) --diskant <mm> (2) --furnier <mm> (1) --dichte <kg/m3> (aus der Masse, s. u.)
          --masse <kg> (1.375 = Mittel aus 1,513 / 1,333 / 1,278) --ex <GPa> (7.3) --ey <GPa> (0.855) --g <GPa> (0.45)
          --nu <-> (0.3) --mesh-to <Hz> (520)
Ohne --dichte folgt die Dichte aus --masse und dem Volumen des Bodens.
Die Basis ist der Bauplan im Ordner dieser Probe (`2026-11-20_S210_harfe_eichung_basis.lvinstbodyplan`, Kopie aus dem
Repo nach KQ46 a) - nicht der Bauplan im Repo.
"""
import copy
import importlib.util
import json
import os
import pathlib
import subprocess
import sys
import tempfile

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
AUS = pathlib.Path(os.environ.get("LUMIVIZ_HARFE_EICHUNG", pathlib.Path(tempfile.gettempdir()) / "lumiviz_harfe_eichung"))
TOOL = pathlib.Path.cwd() / "out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe"
PLAN = HIER / "2026-11-20_S210_harfe_eichung_basis.lvinstbodyplan"

_spec = importlib.util.spec_from_file_location("harfe_moden", HIER / "2026-11-13_S197_harfe_moden.py")
moden = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(moden)

LAENGE, B0, B1 = 1.31, 0.41, 0.10
ZIELE = moden.TAB_FREI


def schalter(args, name, vorgabe, typ=float):
    return typ(args[args.index(name) + 1]) if name in args else vorgabe


def volumen(bass, diskant, furnier):
    """Volumen des Bodens: Breite und Dicke laufen linear ueber die Laenge."""
    y = np.linspace(0.0, LAENGE, 20001)
    b = B0 + (B1 - B0) * y / LAENGE
    h = bass + (diskant - bass) * y / LAENGE + furnier
    return float(np.trapezoid(b * h, y))


def werte(args):
    w = {"bass": schalter(args, "--bass", 12.0) / 1000, "diskant": schalter(args, "--diskant", 2.0) / 1000,
         "furnier": schalter(args, "--furnier", 1.0) / 1000, "ex": schalter(args, "--ex", 7.3) * 1e9,
         "ey": schalter(args, "--ey", 0.855) * 1e9, "g": schalter(args, "--g", 0.45) * 1e9,
         "nu": schalter(args, "--nu", 0.3), "netz": schalter(args, "--mesh-to", 520.0)}
    v = volumen(w["bass"], w["diskant"], w["furnier"])
    w["dichte"] = schalter(args, "--dichte", schalter(args, "--masse", 1.375) / v)
    w["masse"] = w["dichte"] * v
    return w


def bauplan(w):
    p = copy.deepcopy(json.loads(PLAN.read_text(encoding="utf-8")))
    g = p["plate"]["thicknessGrid"]
    vals = []
    for j in range(g["ny"]):
        y = min(g["origin"][1] + j * g["step"], LAENGE)
        vals.extend([round(w["bass"] + (w["diskant"] - w["bass"]) * y / LAENGE + w["furnier"], 6)] * g["nx"])
    g["values"] = vals
    for m in p["materials"]:
        if m["name"] == p["plate"]["material"]:
            m.update({"density": w["dichte"], "ex": w["ex"], "ey": w["ey"], "gxy": w["g"], "nuxy": w["nu"]})
    p["ribs"], p["bridges"], p["pointSprings"] = [], [], []
    p["barOverlap"] = "first"
    p["cavity"]["volume"] = 0.0
    p["cavity"]["openings"] = []
    return p


def rechne(w, name="lauf"):
    AUS.mkdir(parents=True, exist_ok=True)
    pd, bd, fd = AUS / f"{name}.lvinstbodyplan", AUS / f"{name}.lvinstbody", AUS / f"{name}.formen.txt"
    pd.write_text(json.dumps(bauplan(w), indent=1) + "\n", encoding="utf-8", newline="\n")
    for d in (bd, fd):
        if d.exists():
            d.unlink()
    r = subprocess.run([str(TOOL), str(pd), "--free-rim", "--from", "20", "--to", "520", "--mesh-to", str(w["netz"]),
                        "--statistics-to", "0", "--out", str(bd), "--shapes", str(fd)],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    if r.returncode != 0 or not fd.exists():
        raise RuntimeError(f"{name}: {r.stdout[-800:]} {r.stderr[-800:]}")
    knoten, hz, formen = moden.formen(str(fd))
    gefunden = {}
    for k in range(len(hz)):
        gefunden.setdefault(moden.indizes(knoten, formen[k]), []).append(float(hz[k]))
    return gefunden


def abweichungen(gefunden):
    """(0,n) nach der Zaehlung der Knotenlinien. Die Familie mit zwei Knotenlinien laengs der Reihe nach: Bells vier
    Werte (2,0), (2,2), (2,3), (2,4) gegen die vier tiefsten gerechneten (0 / 1 / 2 / 3 Knotenlinien quer) - wie
    Par. 7.1 (S197). Annahme: Bell nennt (2,1) "nicht gefunden"; nach seiner Zaehlung laegen (2,2)-(2,4) 14-16 % zu
    hoch, waehrend (2,0) stimmt, und die Abstaende (gemessen 68 / 49 / 46 Hz, gerechnet 67 / 54 / 51) passen nur der
    Reihe nach. Die Moden sitzen am Bassende; die Zahl der Vorzeichenwechsel laengs ist dort unsicher."""
    aus = {}
    # (0,n): ab der als (0,6) gezaehlten Mode der Reihe nach - wo eine (0,n) sich mit der Bewegung des Bassendes
    # mischt (12 -> 2 mm: (0,12) bei 329 Hz neben (2,0) bei 341 Hz), zaehlt das Skript die Wechsel laengs falsch
    null = sorted(f for mn, fs in gefunden.items() if mn[0] == 0 for f in fs)
    if (0, 6) in gefunden:
        ab6 = [f for f in null if f >= min(gefunden[(0, 6)])]
        for n, f in zip(range(6, 13), ab6):
            aus[(0, n)] = (f, 100 * (f / ZIELE[(0, n)] - 1))
    zwei = sorted(f for mn, fs in gefunden.items() if mn[0] == 2 for f in fs)
    for (mn, soll), f in zip(((k, v) for k, v in ZIELE.items() if k[0] == 2), zwei):
        aus[mn] = (f, 100 * (f / soll - 1))
    return aus


def zeige(w, gefunden, kopf=""):
    ab = abweichungen(gefunden)
    print(f"== {kopf} Dicke {w['bass'] * 1000:.1f} -> {w['diskant'] * 1000:.1f} mm + {w['furnier'] * 1000:.1f}, "
          f"Dichte {w['dichte']:.1f} kg/m3 (Masse {w['masse']:.3f} kg), E quer {w['ex'] / 1e9:.3f}, laengs {w['ey'] / 1e9:.4f}, "
          f"G {w['g'] / 1e9:.4f} GPa, nu {w['nu']:.2f}, Netz {w['netz']:.0f} Hz")
    for fam in (0, 2):
        z = [mn for mn in ZIELE if mn[0] == fam]
        print(f"   ({fam},n) " + "  ".join(f"n={mn[1]}: {ab[mn][0]:.0f} ({ab[mn][1]:+.1f} %)" if mn in ab else f"n={mn[1]}: -" for mn in z))
    v = [ab[mn][1] for mn in ab]
    print(f"   gefunden {len(ab)} / {len(ZIELE)}, groesste {max(abs(x) for x in v):.1f} %, Wurzel aus dem Mittel der Quadrate {np.sqrt(np.mean(np.square(v))):.2f} %")
    andere = {mn: fs for mn, fs in gefunden.items() if mn[0] != 0 or mn not in ZIELE}
    print("   weitere (Zaehlung des Skripts): " + "  ".join(f"{mn}: " + "/".join(f"{f:.0f}" for f in fs) for mn, fs in sorted(andere.items())))
    return ab


def fit(w):
    from scipy.optimize import least_squares

    zaehler = [0]

    def rest(x):
        v = dict(w)
        v["ex"], v["ey"], v["g"] = (float(np.exp(t)) for t in x)
        ab = abweichungen(rechne(v, "fit"))
        zaehler[0] += 1
        r = [ab[mn][1] if mn in ab else 30.0 for mn in ZIELE]
        print(f"   {zaehler[0]:3d}  E quer {v['ex'] / 1e9:.3f}  laengs {v['ey'] / 1e9:.4f}  G {v['g'] / 1e9:.4f}  -> "
              + " ".join(f"{t:+.1f}" for t in r), flush=True)
        return np.array(r)

    x0 = np.log([w["ex"], w["ey"], w["g"]])
    e = least_squares(rest, x0, diff_step=0.03, xtol=1e-4, ftol=1e-4, max_nfev=40)
    v = dict(w)
    v["ex"], v["ey"], v["g"] = (float(np.exp(t)) for t in e.x)
    return v


if __name__ == "__main__":
    was, args = sys.argv[1], sys.argv[2:]
    w = werte(args)
    if was == "lauf":
        zeige(w, rechne(w))
    elif was == "fit":
        zeige(w, rechne(w), "Start:")
        v = fit(w)
        zeige(v, rechne(v), "Ergebnis:")
    elif was == "netz":
        for hz in (520.0, 1500.0, 5000.0):
            v = dict(w)
            v["netz"] = hz
            zeige(v, rechne(v, f"netz_{hz:.0f}"))
