"""S213 (KQ56 a) - Probe: der Boden der besaiteten Harfe als flache Schale (Bauch unter dem Saitenzug).

Unter 9350 N Saitenzug woelbt sich der Boden quer zur Mittelleiste. Ein gewoelbter Streifen, dessen Raender in der
Ebene gehalten sind, wehrt sich gegen jede Auslenkung, die seine Laenge aendert: die Kraft in der Ebene ist je Streifen
N = (E h / b) * Integral(kappa0 w dx), kappa0 = 8 w0 / b^2 (Parabel, Stich w0, Breite b; Marguerre, Kraft in der
Ebene je Streifen gleichmaessig). Die Verformungsenergie dazu:

    U = 1/2 * Integral dy (E h / b) kappa0^2 (Integral w dx)^2

- sie bestraft je Querschnitt das Volumen der Form, also genau das, womit der Boden an die Luft koppelt.
Gerechnet im Modenraum der Platte im Vakuum (alle Moden bis 5 kHz), danach die Luft wie `…_harfe_kopplung.py`.
Stich w0(y) = w0max * (b / bmax)^n; Boden nach Bell Abb. 2.4: 131 cm, 41 -> 10 cm breit, 10 -> 3 mm dick, E quer 7,3 GPa.

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <vak> <gebaut> [schreibe <ziel> <w0mm> <n> <muendung>]
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("kop", HIER / "2026-11-22_S213_harfe_kopplung.py")
kop = importlib.util.module_from_spec(spec)
spec.loader.exec_module(kop)

LAENGE, B0, B1, H0, H1, EQUER = 1.31, 0.41, 0.10, 0.010, 0.003, 7.3e9
DY = 0.02


def streifen(pfad):
    """Je Streifen laengs y und je Mode das Volumen V_s = Integral der Form ueber den Streifen."""
    z = pathlib.Path(pfad).read_text(encoding="utf-8").split("\n")
    n = int(z[1].split()[1])
    xy = np.array([[float(v) for v in z[2 + i].split()] for i in range(n)])
    t = int(z[2 + n].split()[1])
    tri = np.array([[int(v) for v in z[3 + n + i].split()] for i in range(t)])
    m = int(z[3 + n + t].split()[1])
    a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
    fl = 0.5 * np.abs((b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1]))
    ym = (a[:, 1] + b[:, 1] + c[:, 1]) / 3
    ns = int(np.ceil(LAENGE / DY))
    wo = np.clip((ym / DY).astype(int), 0, ns - 1)
    V = np.zeros((ns, m))
    for i in range(m):
        w = np.array(z[5 + n + t + 2 * i].split(), dtype=float)
        np.add.at(V[:, i], wo, fl * w[tri].mean(axis=1))
    return V


def mit_bauch(v, V, w0max, n):
    """Die Moden der Platte im Vakuum mit der Schalensteife; gibt ein Vakuum-Objekt mit den neuen Moden."""
    ys = (np.arange(V.shape[0]) + 0.5) * DY
    b = B0 + (B1 - B0) * ys / LAENGE
    h = H0 + (H1 - H0) * ys / LAENGE
    w0 = w0max * (b / B0) ** n
    kappa = 8 * w0 / b ** 2
    steife = EQUER * h * kappa ** 2 / (b * DY)                # je Streifen, auf V_s^2
    K = np.diag((2 * np.pi * v.f) ** 2) + (V.T * steife) @ V
    lam, U = np.linalg.eigh(K)
    neu = object.__new__(kop.Vakuum)
    neu.__dict__.update(v.__dict__)
    neu.f = np.sqrt(lam) / (2 * np.pi)
    C = U.T
    neu.a = C @ v.a
    neu.steg = C @ v.steg
    neu.G = C @ v.G
    e = C ** 2 * (2 * np.pi * v.f) ** 2 / lam[:, None]        # Anteil der Biegeenergie je alter Mode
    neu.feder = e @ v.feder
    neu.stoff = (e @ v.stoff)
    neu.stoff = neu.stoff / np.maximum(neu.stoff.sum(axis=1, keepdims=True), 1e-300)
    neu.schale = 1 - e.sum(axis=1)                            # Anteil der Energie in der Ebene
    return neu


def main():
    v = kop.Vakuum(sys.argv[1])
    g = pathlib.Path(sys.argv[2])
    gp, gk = kop.lade(g / "concert_harp.lvinstbodyplan"), kop.lade(g / "concert_harp.lvinstbody")
    cav = gp["cavity"]
    V = streifen(pathlib.Path(sys.argv[1]) / "formen.txt")
    assert np.allclose(V.sum(axis=0), v.a, rtol=1e-9, atol=1e-12)
    if len(sys.argv) > 3 and sys.argv[3] == "schreibe":
        ziel, w0, n, mu = pathlib.Path(sys.argv[4]), float(sys.argv[5]) * 1e-3, float(sys.argv[6]), float(sys.argv[7])
        nv = mit_bauch(v, V, w0, n)
        e = kop.kopple(nv, cav, 1.0, mu, cav["q"])
        k = kop.koerper(nv, gk, e)
        ziel.mkdir(parents=True, exist_ok=True)
        (ziel / "concert_harp.lvinstbody").write_text(kop.json.dumps(k) + "\n", encoding="utf-8", newline="\n")
        (ziel / "concert_harp.lvinstbodyplan").write_text(kop.json.dumps(gp, indent=1) + "\n", encoding="utf-8", newline="\n")
        kop.zeile(f"Bauch {w0 * 1e3:.1f} mm n {n:g} mu {mu:.1f}", e, k, gp)
        return
    for n in (2.0, 3.0):
        for w0 in (0.0, 1.0, 2.0, 3.0, 4.0, 6.0):
            nv = mit_bauch(v, V, w0 * 1e-3, n)
            print(f"-- n {n:g}, Stich {w0:.0f} mm: Platte im Vakuum {nv.f[:6].round(1)} Hz, a_1..3 {nv.a[:3].round(4)}, "
                  f"Schalenanteil {nv.schale[:3].round(2)}")
            for mu in (2.0, 2.3, 2.6):
                e = kop.kopple(nv, cav, 1.0, mu, cav["q"])
                kop.zeile(f"Bauch {w0:.0f} mm n {n:g} mu {mu:.1f}", e, kop.koerper(nv, gk, e), gp)


if __name__ == "__main__":
    main()
