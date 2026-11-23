"""S214 (KQ58 a) - Probe: wie stark daempfen bandgedaempfte Saiten die Moden des Harfenbodens?

Bell 1988 (S. 147) und Le Carrou 2007 (S. 560) messen die besaitete Harfe mit stark gedaempften Saiten (Kunststoffband,
Papier). Eine Saite, deren Welle nicht zurueckkommt, wirkt an der Oese wie ein Daempfer mit dem Wellenwiderstand
Z0 = sqrt(T m') quer zur Saite; laengs bleibt sie die Feder aus KQ52 a. Der Boden bewegt sich senkrecht zur Decke, die
Saite steht rund 35 Grad zur Decke (Bell S. 157: 30 Grad): quer zur Saite wirkt der Anteil cos^2(35 Grad) = 0,67.

  Zuwachs des Verlustfaktors je Mode  d_eta = sum_i 0,67 Z0_i phi_m(y_i)^2 / w_m   (Stoerung erster Ordnung)

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner mit concert_harp.lvinstbody und .lvinstbodyplan>
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

ANTEIL = np.cos(np.radians(35.0)) ** 2
# Bell 1988, Kasten G: Guete in der Harfe ohne / mit Saiten (Bildunterschriften Abb. 7.8-7.16; Harfenbericht Par. 8)
BELL_Q = {"S(1,2)": (249, 39, 257, 32), "S(1,3)": (316, 45, 334, 27), "S(1,4)": (432, 43, 430, 28), "S(1,5)": (545, 31, 555, 19),
          "S(1,7)": (757, 33, 753, 17)}


def wellenwiderstaende():
    z = []
    for i, (hz, d, L) in enumerate(ps.HARFE_MONO):
        sa = ps.monofil(d, hz, L, "N" if i <= 9 else "D")
        z.append((hz, float(np.sqrt(sa["T"] * sa["m"])), float(sa["T"])))
    for hz, kern, gesamt, L in ps.HARFE_METALL:
        m = 7850 * np.pi * (kern / 1000) ** 2 / 4 + 8900 * np.pi * ((gesamt / 1000) ** 2 - (kern / 1000) ** 2) / 4
        T = 4 * L * L * hz * hz * m
        z.append((hz, float(np.sqrt(T * m)), float(T)))
    return sorted(z)                                   # tiefste Saite zuerst


def main():
    o = pathlib.Path(sys.argv[1])
    k = json.loads((o / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
    p = json.loads((o / "concert_harp.lvinstbodyplan").read_text(encoding="utf-8"))
    z = wellenwiderstaende()
    federn = sorted(p["pointSprings"], key=lambda f: f["y"])          # Bassende zuerst
    assert len(z) == len(federn), f"{len(z)} Saiten, {len(federn)} Federn"
    ys = {}
    for i, b in enumerate(p["bridgePoints"]):
        ys.setdefault(round(b["y"], 4), i)
    yy = np.array(sorted(ys))
    idx = [ys[y] for y in yy]
    Z0 = np.array([q[1] for q in z])
    print(f"{len(z)} Saiten: Summe der Zuege {sum(q[2] for q in z):.0f} N (Bell: 9350 N), Summe Z0 {Z0.sum():.1f} kg/s, "
          f"groesste {Z0.max():.2f} (tiefste Saite), Median {np.median(Z0):.2f} kg/s")
    print("Mode     Hz   eta Modell   d_eta Saiten   Q vorher -> nachher")
    for m in k["modes"][:14]:
        if m["f"] > 800:
            break
        phi = np.interp([f["y"] for f in federn], yy, np.array(m["bridge"])[idx])
        d = float(np.sum(ANTEIL * Z0 * phi ** 2) / (2 * np.pi * m["f"]))
        print(f"      {m['f']:6.1f}   {m['eta']:.4f}      {d:.4f}        {1 / m['eta']:5.1f} -> {1 / (m['eta'] + d):5.1f}")
    print("Bell, Kasten G in der Harfe ohne -> mit Saiten: " + "  ".join(
        f"{n} Q {a[1]} -> {a[3]} (d_eta {1 / a[3] - 1 / a[1]:.4f})" for n, a in BELL_Q.items()))


if __name__ == "__main__":
    main()
