"""S209 - Suchgeruest fuer den Bauplan der Harfe (KQ40 a). Rechnet eine Fassung mit InstrumentBodyTool (16 Hoerorte in
2,06 m wie KQ36 a, dazu `front`) und misst:
  L1  Lage der zwei Moden mit Luft gegen Bell in der Harfe ohne Saiten 170 / 216 Hz (7 %)
  L2  Lage S(1,2) ... S(1,6) gegen 249 / 316 / 432 / 545 / 709 Hz (7 %, KQ31-2 a; hoechste Spitze der Punktadmittanz
      ueber die Oesen im Fenster +-12 %)
  A   |p/F| an der Oese der Saiten 33 ... 22 bei ihrer Frequenz gegen Bell 1988 Abb. 7.23, Bezug Saite 26 (6 dB);
      Leistungsmittel der 16 Hoerorte, Kraft senkrecht zum Boden
Aufruf vom Projekt-Root; Basis ist der Bauplan vor S209 (`harfe_ausgang.lvinstbodyplan`, Kopie aus dem Repo).
"""
import copy
import os
import tempfile
import json
import math
import pathlib
import subprocess

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
AUS = pathlib.Path(os.environ.get("LUMIVIZ_HARFE_SUCHE", pathlib.Path(tempfile.gettempdir()) / "lumiviz_harfe_suche"))  # Ausgaben des Werkzeugs
TOOL = pathlib.Path.cwd() / "out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe"
PLAN = HIER / "harfe_ausgang.lvinstbodyplan"

# Saite: (Hz leer, Bell Abb. 7.23 in dB, am Seitenbild gelesen S208)
BELL_723 = {33: (104, -26), 32: (116, -24), 31: (123, -31), 30: (139, -32), 29: (155, -34), 28: (165, -28.5), 27: (185, -10),
            26: (207, -8), 25: (233, -16), 24: (247, -12), 23: (277, -5.5), 22: (311, -8)}
S1N = {"S(1,2)": 249.0, "S(1,3)": 316.0, "S(1,4)": 432.0, "S(1,5)": 545.0, "S(1,6)": 709.0}


def basis():
    plan = json.loads(PLAN.read_text(encoding="utf-8"))
    orte = [plan["listeningPoints"][0]]
    N, R, mx, my = 16, 2.06, 0.0, 0.655
    z0 = math.sin(math.radians(10))
    for i in range(N):
        z = z0 + (1 - z0) * (i + 0.5) / N
        a = i * math.pi * (3 - math.sqrt(5))
        r = math.sqrt(1 - z * z)
        orte.append({"name": f"k{i + 1:02d}", "x": round(mx + R * r * math.cos(a), 4), "y": round(my + R * r * math.sin(a), 4),
                     "z": round(R * z, 4), "source": "probe (KQ36 a)"})
    plan["listeningPoints"] = orte
    return plan


def rechne(plan, name, to=1200, zeigen=True):
    AUS.mkdir(parents=True, exist_ok=True)
    pd, bd = AUS / f"{name}.lvinstbodyplan", AUS / f"{name}.lvinstbody"
    pd.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    if bd.exists():
        bd.unlink()
    r = subprocess.run([str(TOOL), str(pd), "--to", str(to), "--statistics-to", "0", "--out", str(bd)],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    if r.returncode != 0 or not bd.exists():
        print(name, "FEHLER", r.stdout[-500:], r.stderr[-500:])
        return None
    return messe(json.loads(bd.read_text(encoding="utf-8")), plan, name, zeigen)


def messe(k, plan, name="", zeigen=True):
    f = np.array([m["f"] for m in k["modes"]])
    eta = np.array([m["eta"] for m in k["modes"]])
    luft = np.array([m["airShare"] for m in k["modes"]])
    steg = np.array([m["bridge"] for m in k["modes"]])
    G = np.array([[complex(*g) for g in m["radiation"]] for m in k["modes"]])
    tasten = k["bridgePoints"]
    wn = 2 * np.pi * f

    def pf(taste, hz):
        w = 2 * np.pi * hz
        n = -(w * w) / (wn ** 2 * (1 + 1j * eta) - w * w)
        p = (steg[:, tasten.index(taste)] * n) @ G
        return 10 * np.log10(np.mean(np.abs(p[1:]) ** 2))

    a = {}
    for s, (hz, bell) in BELL_723.items():
        t = int(round(69 + 12 * np.log2(hz / 440.0))) + 1  # die Saite mit dem Pedal in der Mitte: ein Halbton hoeher
        a[s] = pf(t, hz)
    abw = {s: (a[s] - a[26]) - (BELL_723[s][1] - BELL_723[26][1]) for s in a}

    ys = {}
    for i, p in enumerate(plan["bridgePoints"]):
        ys.setdefault(p["y"], i)
    pk = np.array([ys[v] for v in sorted(ys)])

    def spitze(mitte, breite=0.12):
        best = (0.0, -1e9)
        for hz in np.arange(mitte * (1 - breite), mitte * (1 + breite), 0.25):
            w = 2 * np.pi * hz
            n = 1.0 / (wn ** 2 * (1 + 1j * eta) - w * w)
            v = max(abs(w * np.sum(steg[:, p] ** 2 * n)) for p in pk)
            if v > best[1]:
                best = (hz, v)
        return best[0]

    lagen = {n: spitze(hz) for n, hz in S1N.items()}
    mitluft = [(float(f[i]), float(luft[i]), float(1 / eta[i])) for i in range(len(f)) if luft[i] > 0.08]
    e = {"name": name, "f": f, "luft": luft, "mitluft": mitluft, "lagen": lagen, "abw": abw, "pf": a, "k": k}
    if zeigen:
        zeige(e)
    return e


def zeige(e):
    print(f"== {e['name']}")
    print("   Moden bis 350 Hz (Luft): " + "  ".join(f"{v:.1f} ({l:.2f})" for v, l in zip(e["f"], e["luft"]) if v < 350))
    ml = e["mitluft"]
    if len(ml) >= 2:
        print(f"   L1 mit Luft: {ml[0][0]:.1f} Hz ({100 * (ml[0][0] / 170 - 1):+.0f} % gegen 170, Q {ml[0][2]:.0f})  {ml[1][0]:.1f} Hz ({100 * (ml[1][0] / 216 - 1):+.0f} % gegen 216, Q {ml[1][2]:.0f})"
              + "".join(f"  [{v:.1f} Luft {l:.2f}]" for v, l, _ in ml[2:]))
    print("   L2 " + "  ".join(f"{n} {v:.0f} ({100 * (v / S1N[n] - 1):+.1f} %)" for n, v in e["lagen"].items()))
    ab = e["abw"]
    print("   A  Saite 33..22 Modell - Bell (dB, Bezug 26): " + " ".join(f"{ab[s]:+.1f}" for s in sorted(ab, reverse=True))
          + f"   halten {sum(abs(v) <= 6 for v in ab.values())} / 12, groesste {max(abs(v) for v in ab.values()):.1f}")


def bassende(plan, weg=1e12, dreh=0.0):
    """Das Bassende (Kante 0 des Umrisses) als Abschnitt mit Weg- und Drehfeder."""
    p = copy.deepcopy(plan)
    p["rim"]["sections"] = [s for s in p["rim"]["sections"] if s["name"] != "bass edge"]
    p["rim"]["sections"].append({"name": "bass edge", "edges": [0, 0], "translationalStiffness": weg, "rotationalStiffness": dreh, "source": "probe"})
    return p


if __name__ == "__main__":
    import time
    t = time.time()
    rechne(basis(), "ausgang")
    print(f"{time.time() - t:.1f} s")
