"""S213 (KQ56 a) - Probe: wie stark koppelt der Boden der Harfe an die Luft?

Der gebaute Koerper (F4, S211) hat mit Saiten 173 / 229 Hz (Verhaeltnis 1,32); gemessen sind an zwei besaiteten
Harfen 187 / 207 Hz (Bell 1988, Kasten G; 1,11) und 153,5 / 172 Hz (Le Carrou 2007, Camac; 1,12). Die Probe rechnet
die Kopplung von `Luftraum.hpp` aus den Moden der Platte im Vakuum nach (Diagonale plus Rang 1) und schwaecht das
Volumen je Mode mit einem Faktor beta: P = -alpha (beta sum a_k q_k + u).

  vak <gebaut> <ziel>                 Bauplan ohne Hohlraum nach <ziel> (dann InstrumentBodyTool <ziel>/concert_harp.lvinstbodyplan
                                      --to 5000 --shapes <ziel>/formen.txt)
  pruefe <vak> <gebaut>               beta = 1 gegen den gebauten Koerper
  raster <vak> <gebaut>               beta x Muendung: Paar, Lagen, Abstrahlung an den Saiten 33-22
  schreibe <vak> <gebaut> <ziel> <beta> <muendung> [<guete>]   Koerper nach <ziel>/concert_harp.lvinstbody (fuer die Lautheit)

Naeherungen gegen das Werkzeug: Abstrahlung der Platte je Mode bei ihrer Frequenz im Vakuum, die Laufzeit zur neuen
Frequenz nachgefuehrt; Verlust der Platte aus dem Gesetz bei der neuen Frequenz, Anteil der Federn nach Verformungsenergie.
Aufruf vom Projekt-Root mit Python 3.12 (numpy).
"""
import copy
import json
import math
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HIER / "2026-11-19_S209_harfe_suche"))

# Bell 1988, Kasten G in der Harfe, besaitet (Bildunterschriften Abb. 7.8-7.16; Harfenbericht Par. 8)
BELL_BESAITET = {"S(1,1)-": 187.0, "S(1,1)+": 207.0, "S(1,2)": 257.0, "S(1,3)": 334.0, "S(1,4)": 430.0, "S(1,5)": 555.0,
                 "S(1,6)": 697.0}


def lade(pfad):
    return json.loads(pathlib.Path(pfad).read_text(encoding="utf-8"))


def formen(pfad):
    """Volumen je Mode a_k = Integral der Form ueber die Platte (linear ueber die Ecken der Dreiecke)."""
    z = pathlib.Path(pfad).read_text(encoding="utf-8").split("\n")
    assert z[0].startswith("lumiviz-body-shapes")
    n = int(z[1].split()[1])
    xy = np.array([[float(v) for v in z[2 + i].split()] for i in range(n)])
    t = int(z[2 + n].split()[1])
    tri = np.array([[int(v) for v in z[3 + n + i].split()] for i in range(t)])
    m = int(z[3 + n + t].split()[1])
    a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
    fl = 0.5 * np.abs((b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1]))
    hz, vol = [], []
    for i in range(m):
        hz.append(float(z[4 + n + t + 2 * i].split()[1]))
        w = np.array(z[5 + n + t + 2 * i].split(), dtype=float)
        vol.append(float(np.sum(fl * w[tri].mean(axis=1))))
    return np.array(hz), np.array(vol), float(fl.sum())


class Vakuum:
    def __init__(self, ordner):
        o = pathlib.Path(ordner)
        self.k = lade(o / "concert_harp.lvinstbody")
        self.plan = lade(o / "concert_harp.lvinstbodyplan")
        md = self.k["modes"]
        self.f = np.array([m["f"] for m in md])
        self.eta = np.array([m["eta"] for m in md])
        self.steg = np.array([m["bridge"] for m in md])
        self.G = np.array([[complex(*g) for g in m["radiation"]] for m in md])
        self.feder = np.array([m["springShare"] for m in md])
        self.stoff = np.array([m["materialShares"] for m in md])
        hz, vol, flaeche = formen(o / "formen.txt")
        assert len(hz) == len(self.f) and np.allclose(hz, self.f, rtol=1e-6), "Formen und Koerper passen nicht"
        self.a = vol
        self.flaeche = flaeche
        self.rho = self.k["air"]["density"]
        self.c = self.k["air"]["speedOfSound"]
        self.orte = np.array([[p["x"], p["y"], p["z"]] for p in self.plan["listeningPoints"]])
        self.mitte = np.array([0.0, 0.655, 0.0])


def kopple(v, cav, beta, muendung, guete):
    """Wie `koppleLuftraum` / `mitLuftraum`; cav = der Hohlraum des gebauten Bauplans."""
    rho, c = v.rho, v.c
    alpha = rho * c * c / cav["volume"]
    lo = []
    for o in cav["openings"]:
        L = o["lipLength"] + muendung * math.sqrt(o["area"] / math.pi)
        lo.append((o["x"], o["y"], o["z"], o["area"], L))
    leit = sum(A / (rho * L) for *_, A, L in lo)
    Ma = 1.0 / leit
    anteil = np.array([A / L for *_, A, L in lo])
    anteil /= anteil.sum()
    n = len(v.f)
    d = np.append((2 * np.pi * v.f) ** 2, 0.0)
    z = np.append(beta * v.a, 1.0 / math.sqrt(Ma))
    lam, vec = np.linalg.eigh(np.diag(d) + alpha * np.outer(z, z))
    fneu = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    C = vec[:n, :].T                       # je gekoppelter Mode die Anteile der Plattenmoden
    y = vec[n, :]
    luft = y * y
    u = y / math.sqrt(Ma)
    steg = C @ v.steg
    # Abstrahlung der Platte, Laufzeit auf die neue Frequenz nachgefuehrt
    R = np.linalg.norm(v.orte - v.mitte, axis=1)
    G = np.zeros((n + 1, len(R)), dtype=complex)
    dp = np.zeros((n + 1, len(R)), dtype=complex)
    for m in range(n + 1):
        dk = 2 * np.pi * (fneu[m] - v.f) / c                      # je Plattenmode
        G[m] = (C[m][:, None] * v.G * np.exp(-1j * np.outer(dk, R))).sum(axis=0)
        k = 2 * np.pi * fneu[m] / c
        for (x, yy, zz, A, L), an in zip(lo, anteil):
            r = np.linalg.norm(v.orte - np.array([x, yy, zz]), axis=1)
            dp[m] += rho / (4 * np.pi) * u[m] * an / r * np.exp(-1j * k * r)
    # Verlust: Gesetz bei der neuen Frequenz, Federn nach Verformungsenergie
    dm = v.k["damping"]
    e = C ** 2 * (2 * np.pi * v.f) ** 2
    e = e / np.maximum(e.sum(axis=1, keepdims=True), 1e-300)
    feder = e @ v.feder
    stoff = e @ v.stoff
    gesetz = (dm["a"] * fneu ** 2 + dm["b"] * fneu + dm["c"]) / (2 * np.pi * np.maximum(fneu, 1e-9))
    eta = (1 - luft) * (1 - feder) * gesetz + luft / guete
    return {"f": fneu, "eta": eta, "luft": luft, "steg": steg, "G": G + dp, "Gplatte": G, "Gloch": dp, "feder": feder,
            "stoff": stoff, "helmholtz": math.sqrt(alpha / Ma) / (2 * np.pi), "u": u, "C": C,
            "fc": math.sqrt(alpha) * abs(beta * v.a[0]) / (2 * np.pi)}


def koerper(v, gebaut, e):
    k = copy.deepcopy(gebaut)
    k["modes"] = [{"airShare": float(e["luft"][m]), "bridge": [float(x) for x in e["steg"][m]], "bridgeAngle": 0,
                   "eta": float(e["eta"][m]), "f": float(e["f"][m]), "materialShares": [float(x) for x in e["stoff"][m]],
                   "radiation": [[float(g.real), float(g.imag)] for g in e["G"][m]], "springShare": float(e["feder"][m])}
                  for m in range(len(e["f"]))]
    return k


def lagen(k, plan):
    import harfe_suche as s
    r = s.messe(k, plan, zeigen=False)
    ml = r["mitluft"]
    f = np.array([m["f"] for m in k["modes"]])
    eta = np.array([m["eta"] for m in k["modes"]])
    steg = np.array([m["bridge"] for m in k["modes"]])
    wn = 2 * np.pi * f
    ys = {}
    for i, p in enumerate(plan["bridgePoints"]):
        ys.setdefault(p["y"], i)
    pk = np.array([ys[q] for q in sorted(ys)])

    def spitze(mitte, breite=0.10):
        best = (0.0, -1e9)
        for hz in np.arange(mitte * (1 - breite), mitte * (1 + breite), 0.25):
            w = 2 * np.pi * hz
            nn = 1.0 / (wn ** 2 * (1 + 1j * eta) - w * w)
            val = max(abs(w * np.sum(steg[:, p] ** 2 * nn)) for p in pk)
            if val > best[1]:
                best = (hz, val)
        return best[0]

    lg = {n: spitze(hz) for n, hz in BELL_BESAITET.items() if not n.startswith("S(1,1)")}
    return r, ml, lg


def zeile(name, e, k, plan):
    r, ml, lg = lagen(k, plan)
    ab = r["abw"]
    paar = "  ".join(f"{f:6.1f} (Luft {l:.2f}, Q {q:4.1f})" for f, l, q in ml[:3])
    txt = (f"{name:22s} Helmholtz {e['helmholtz']:5.1f}  f_c {e['fc']:4.1f}  | {paar}"
           f"\n{'':22s} gegen Bell besaitet: " + (f"{100 * (ml[0][0] / 187 - 1):+.1f} / {100 * (ml[1][0] / 207 - 1):+.1f} %" if len(ml) >= 2 else "-")
           + "  " + "  ".join(f"{n} {100 * (x / BELL_BESAITET[n] - 1):+.1f}" for n, x in lg.items())
           + f"\n{'':22s} A 33..22: " + " ".join(f"{ab[s]:+.0f}" for s in sorted(ab, reverse=True))
           + f"  halten {sum(abs(x) <= 6 for x in ab.values())} / 12, groesste {max(abs(x) for x in ab.values()):.1f} dB")
    print(txt, flush=True)


def main():
    was = sys.argv[1]
    if was == "vak":
        q, zi = pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])
        p = lade(q / "concert_harp.lvinstbodyplan")
        p["cavity"]["volume"] = 0.0
        zi.mkdir(parents=True, exist_ok=True)
        (zi / "concert_harp.lvinstbodyplan").write_text(json.dumps(p, indent=1) + "\n", encoding="utf-8", newline="\n")
        print("geschrieben", zi / "concert_harp.lvinstbodyplan")
        return
    v = Vakuum(sys.argv[2])
    gp = lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
    gk = lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbody")
    cav = gp["cavity"]
    if was == "pruefe":
        e = kopple(v, cav, 1.0, cav["endCorrection"], cav["q"])
        print(f"Platte: Flaeche {v.flaeche:.4f} m2; Volumen der ersten Moden a_k {v.a[:5]}")
        print(f"erste Mode im Vakuum {v.f[0]:.2f} Hz: a = {v.a[0]:.4f} m3/sqrt(kg), a^2 = {v.a[0] ** 2:.5f}; "
              f"Punktmasse an der Oese Taste 45: {1 / v.steg[0][[p['key'] for p in v.plan['bridgePoints']].index(45)] ** 2:.2f} kg")
        print("Mode: f Probe / Werkzeug | Luft | eta | Steg[40] | |G| front, k01")
        for m in range(8):
            w = gk["modes"][m]
            gw = [abs(complex(*g)) for g in w["radiation"][:2]]
            print(f"  {e['f'][m]:8.2f} / {w['f']:8.2f} | {e['luft'][m]:.3f} / {w['airShare']:.3f} | {e['eta'][m]:.4f} / {w['eta']:.4f} | "
                  f"{abs(e['steg'][m][40]):.4f} / {abs(w['bridge'][40]):.4f} | {abs(e['G'][m][0]):.4g} / {gw[0]:.4g}, {abs(e['G'][m][1]):.4g} / {gw[1]:.4g}")
        fw = np.array([m["f"] for m in gk["modes"]])
        print(f"groesste Abweichung der Frequenz ueber alle {len(fw)} Moden: {np.max(np.abs(e['f'] / fw - 1)) * 100:.3f} %")
        zeile("Probe beta 1", e, koerper(v, gk, e), gp)
        zeile("Werkzeug (gebaut)", e, gk, gp)
        return
    if was == "raster":
        for beta in (1.0, 0.7, 0.5, 0.4, 0.35, 0.3, 0.25, 0.2):
            for mu in (2.0, 2.3, 2.6, 2.9):
                e = kopple(v, cav, beta, mu, cav["q"])
                zeile(f"beta {beta:.2f} mu {mu:.1f}", e, koerper(v, gk, e), gp)
        return
    if was == "schreibe":
        zi = pathlib.Path(sys.argv[4])
        beta, mu = float(sys.argv[5]), float(sys.argv[6])
        q = float(sys.argv[7]) if len(sys.argv) > 7 else cav["q"]
        e = kopple(v, cav, beta, mu, q)
        k = koerper(v, gk, e)
        zi.mkdir(parents=True, exist_ok=True)
        (zi / "concert_harp.lvinstbody").write_text(json.dumps(k) + "\n", encoding="utf-8", newline="\n")
        (zi / "concert_harp.lvinstbodyplan").write_text(json.dumps(gp, indent=1) + "\n", encoding="utf-8", newline="\n")
        zeile(f"beta {beta:.2f} mu {mu:.1f} Q {q:g}", e, k, gp)
        return


if __name__ == "__main__":
    main()
