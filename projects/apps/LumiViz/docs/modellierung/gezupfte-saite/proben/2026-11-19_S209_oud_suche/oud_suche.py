"""S209 - Suchgeruest fuer den Bauplan der Oud (KQ38 a, KQ41 a). Rechnet eine Fassung des Bauplans mit
InstrumentBodyTool bis 450 Hz und misst die drei Ziele:
  Z1  Lagen: Spitzen der Kurve am Antwortpunkt 3 gegen 101 / 203 / 280 / 362 Hz (KQ10 a, 3 %)
  Z2  am Antwortpunkt 3 die Spitze um 203 Hz die staerkste unter 400 Hz (dB gegen die staerkste andere)
  Z3  Hauptspitze der Abstrahlung in 1 m / Luftmode im Band 1,5-2,0
Bericht: Moden mit Luftanteil, Formwert am Steg, Basschor T2 / T3 (131 / 196 Hz) gegen das Maximum unter 300 Hz.
"""
import copy
import os
import tempfile
import json
import pathlib
import subprocess

import numpy as np
from scipy.signal import find_peaks

ROOT = pathlib.Path.cwd()  # vom Projekt-Root starten
TOOL = ROOT / "out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe"
PLAN = pathlib.Path(__file__).resolve().parent / "oud_ausgang_s196.lvinstbodyplan"  # der Bauplan vor S209 (Commit 9a9cc4d6)
HIER = pathlib.Path(os.environ.get("LUMIVIZ_OUD_SUCHE", pathlib.Path(tempfile.gettempdir()) / "lumiviz_oud_suche"))  # Ausgaben des Werkzeugs
ZIELE = (101.0, 203.0, 280.0, 362.0)


def basis():
    plan = json.loads(PLAN.read_text(encoding="utf-8"))
    u = plan["plate"]["outline"]
    mx = 0.5 * (min(p[0] for p in u) + max(p[0] for p in u))
    my = 0.5 * (min(p[1] for p in u) + max(p[1] for p in u))
    plan["listeningPoints"].append({"name": "m1", "x": round(mx, 4), "y": round(my, 4), "z": 1.0, "source": "probe"})
    return plan


def formen(datei):
    z = open(datei, encoding="utf-8").read().split("\n")
    n = int(z[1].split()[1])
    knoten = np.array([[float(v) for v in s.split()] for s in z[2:2 + n]])
    t = int(z[2 + n].split()[1])
    dreiecke = np.array([[int(v) for v in s.split()] for s in z[3 + n:3 + n + t]])
    m = int(z[3 + n + t].split()[1])
    hz = np.array([float(z[4 + n + t + 2 * k].split()[1]) for k in range(m)])
    w = np.array([[float(v) for v in z[5 + n + t + 2 * k].split()] for k in range(m)])
    return knoten, dreiecke, hz, w


def spitzen(f, a, mindest=0.04, abstand_hz=4.0):
    schritt = float(np.median(np.diff(f)))
    orte, _ = find_peaks(a, prominence=mindest * a.max(), distance=max(1, int(abstand_hz / schritt)))
    return [(float(f[i]), float(a[i])) for i in orte]


def rechne(plan, name, to=450, zeigen=True, netz=None):
    HIER.mkdir(parents=True, exist_ok=True)
    pd = HIER / f"{name}.lvinstbodyplan"
    bd = HIER / f"{name}.lvinstbody"
    fd = HIER / f"{name}_formen.txt"
    pd.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    r = subprocess.run([str(TOOL), str(pd), "--to", str(to), "--statistics-to", "0", "--out", str(bd), "--shapes", str(fd)] + (["--mesh-to", str(netz)] if netz else []),
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    if r.returncode != 0 or not bd.exists():
        print(name, "FEHLER", r.stdout[-600:], r.stderr[-600:])
        return None
    k = json.loads(bd.read_text(encoding="utf-8"))
    knoten, dreiecke, hz, w = formen(fd)
    mf = np.array([m["f"] for m in k["modes"]])
    eta = np.array([m["eta"] for m in k["modes"]])
    luft = np.array([m["airShare"] for m in k["modes"]])
    steg = np.array([m["bridge"] for m in k["modes"]])
    G = np.array([[complex(*g) for g in m["radiation"]] for m in k["modes"]])[:, -1]
    assert len(mf) == len(hz) and np.allclose(mf, hz, rtol=1e-4), "Moden der Formen passen nicht zur Koerperdatei"

    def naechster(x, y):
        return int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))

    # Antwortpunkt 3 (Abb. 5 b): x = 7,5, y = 46 - 32,5 = 13,5 cm; Anregung auf dem 5-cm-Raster
    antwort = naechster(0.075, 0.135)
    raster = []
    for xr in range(-15, 16, 5):
        for yr in range(0, 41, 5):
            x, y = xr / 100.0, 0.46 - yr / 100.0
            kn = naechster(x, y)
            if np.hypot(knoten[kn, 0] - x, knoten[kn, 1] - y) < 0.012:
                raster.append(kn)
    f = np.arange(60.0, 400.01, 0.25)
    om = 2 * np.pi * f[:, None]
    omn = 2 * np.pi * mf[None, :]
    kern = -(om ** 2) / (omn ** 2 * (1 + 1j * eta[None, :]) - om ** 2)
    alle = np.array([np.abs(kern @ (w[:, antwort] * w[:, e])) for e in raster])
    mittel = np.sqrt((alle ** 2).mean(axis=0))
    punkt = np.abs(kern @ (w[:, antwort] ** 2))
    sp = spitzen(f, mittel)
    lagen = []
    for z in ZIELE:
        q = min(sp, key=lambda g: abs(g[0] - z))
        lagen.append((q[0], 100 * (q[0] / z - 1)))
    um203 = [p for p in sp if 185 <= p[0] <= 225]
    andere = [p for p in sp if not (185 <= p[0] <= 225)]
    z2 = (20 * np.log10(max(p[1] for p in um203) / max(p[1] for p in andere))) if um203 and andere else float("nan")
    # dasselbe mit Anregung am Antwortpunkt
    spp = spitzen(f, punkt)
    um = [p for p in spp if 185 <= p[0] <= 225]
    an = [p for p in spp if not (185 <= p[0] <= 225)]
    z2p = (20 * np.log10(max(p[1] for p in um) / max(p[1] for p in an))) if um and an else float("nan")

    # Abstrahlung in 1 m: Mittel ueber die Stegpunkte der Choere 6 (36), 3 (50), 1 (60), Terzglaettung
    fh = np.arange(20.0, 440.0, 0.5)
    ws = 2 * np.pi * fh[:, None]
    wn = 2 * np.pi * mf[None, :]
    nenner = 1.0 / (wn ** 2 - ws ** 2 + 1j * eta[None, :] * wn * ws)
    tasten = k["bridgePoints"]
    p2 = np.mean([np.abs((-ws ** 2 * nenner) @ (steg[:, tasten.index(t)] * G)) ** 2 for t in (36, 50, 60)], axis=0)
    c = np.concatenate([[0.0], np.cumsum(p2)])
    lo = np.searchsorted(fh, fh / 2 ** (1 / 6))
    hi = np.maximum(np.searchsorted(fh, fh * 2 ** (1 / 6)), lo + 1)
    db = 10 * np.log10((c[hi] - c[lo]) / (hi - lo)) - 20 * np.log10(20e-6)
    spa = [p for p in spitzen(fh, 10 ** (db / 20), mindest=0.02, abstand_hz=8.0) if 60 < p[0] < 390]
    f_luft = spa[0][0]
    rest = [p for p in spa if p[0] > 1.2 * f_luft]
    f_haupt = max(rest, key=lambda p: p[1])[0]

    def bei(x):
        return float(db[int(np.argmin(np.abs(fh - x)))])

    sel = (fh > f_luft) & (fh < f_haupt)
    tal = float(db[sel].min()) if sel.any() else float("nan")
    max300 = float(db[(fh > 60) & (fh < 300)].max())
    pp = knoten[dreiecke]
    fl = 0.5 * np.abs((pp[:, 1, 0] - pp[:, 0, 0]) * (pp[:, 2, 1] - pp[:, 0, 1]) - (pp[:, 2, 0] - pp[:, 0, 0]) * (pp[:, 1, 1] - pp[:, 0, 1]))
    sk = naechster(0.0, plan["bridgePoints"][0]["y"])
    form = []
    for j in range(len(mf)):
        if mf[j] >= 420:
            break
        vv = w[j] / np.abs(w[j]).max()
        mi = vv[dreiecke].mean(axis=1)
        vol = float(np.sum(mi * fl) / np.sum(np.abs(mi) * fl))
        vz = 1.0 if vol >= 0 else -1.0
        form.append((vz * float(vv[sk]), vz * float(vv[antwort]), 100 * float(knoten[int(np.argmax(np.abs(vv))), 1]), abs(vol), float(abs(G[j]))))
    def lochrand(x, y, r):
        dd = np.hypot(knoten[:, 0] - x, knoten[:, 1] - y)
        return np.where(np.abs(dd - r) < 0.004)[0]

    gross = lochrand(0.0, 0.30, 0.053)
    klein = np.concatenate([lochrand(-0.103, 0.204, 0.020), lochrand(0.103, 0.204, 0.020)])
    erg = {"name": name, "form": form, "alle_hz": mf, "phi_gross2": (w[:, gross] ** 2).mean(axis=1), "phi_klein2": (w[:, klein] ** 2).mean(axis=1), "phi_ap2": w[:, antwort] ** 2, "phi_steg2": w[:, sk] ** 2, "moden": mf[mf < 420], "luft": luft[mf < 420], "steg": np.abs(steg[mf < 420]).mean(axis=1),
           "lagen": lagen, "z2": z2, "z2p": z2p, "f_luft": f_luft, "f_haupt": f_haupt, "z3": f_haupt / f_luft,
           "luft_db": bei(f_luft) - bei(f_haupt), "tal_db": tal - bei(f_haupt), "t2": bei(131) - max300, "t3": bei(196) - max300,
           "spitzen_ap": sp, "spitzen_abstr": spa, "_w": w, "_knoten": knoten, "_antwort": antwort, "_gross": gross, "_klein": klein}
    if zeigen:
        zeige(erg)
    return erg


def zeige(e):
    z1 = max(abs(l[1]) for l in e["lagen"])
    ok = lambda b: "ja  " if b else "NEIN"
    print(f"== {e['name']}")
    print("   Moden Hz (Luft | Steg rel. / AP3 rel. | Bauch y cm | Volumen | |G| 1 m): " + "  ".join(
        f"{f:.0f} ({a:.2f}|{o[0]:+.2f}/{o[1]:+.2f}|{o[2]:.0f}|{o[3]:.2f}|{1000 * o[4]:.0f})" for f, a, o in zip(e["moden"], e["luft"], e["form"])))
    print(f"   Z1 {ok(z1 <= 3.0)} Lagen " + " / ".join(f"{l[0]:.0f} ({l[1]:+.1f} %)" for l in e["lagen"]))
    print(f"   Z2 {ok(e['z2'] > 0)} Spitze um 203 Hz gegen die staerkste andere: {e['z2']:+.1f} dB (Raster), {e['z2p']:+.1f} dB (am Punkt)"
          f"   Spitzen: " + " ".join(f"{p[0]:.0f}:{p[1]:.2f}" for p in e["spitzen_ap"]))
    print(f"   Z3 {ok(1.5 <= e['z3'] <= 2.0)} Abstrahlung 1 m: Luftmode {e['f_luft']:.0f} Hz, Hauptspitze {e['f_haupt']:.0f} Hz, Verhaeltnis {e['z3']:.2f}"
          f"   Luftmode {e['luft_db']:+.1f} dB, Tal {e['tal_db']:+.1f} dB; Basschor T2 {e['t2']:+.1f} / T3 {e['t3']:+.1f} dB unter Max < 300 Hz"
          f"   Spitzen: " + " ".join(f"{p[0]:.0f}" for p in e["spitzen_abstr"]))


if __name__ == "__main__":
    rechne(basis(), "basis")
