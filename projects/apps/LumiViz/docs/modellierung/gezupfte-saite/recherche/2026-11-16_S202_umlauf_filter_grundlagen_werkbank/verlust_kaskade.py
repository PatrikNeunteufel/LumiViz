"""LVR Auftrag 11, Punkt 1/2: Verlustfilter, passiv durch den Bau (Gl. V7):
    H(z) = g * prod_{m=1..M} (1 + a_m) / (1 + a_m z^-1),   0 < g <= 1,  -1 < a_m <= 0   =>  |H| <= g <= 1 fuer alle w.
-ln|H(w)| = -ln g + sum_m 1/2 ln[(1 + 2 a_m cos w + a_m^2) / (1 + a_m)^2]  (jeder Summand >= 0, steigt mit w).
Angepasst offline je Taste (scipy least_squares, Parameter g = exp(-u^2), a_m = -sin^2(v_m)) am relativen Fehler von
-ln|H| an den hoerbaren Teiltoenen (sigma <= 100 1/s) = relativer Fehler des Abklingens (sigma = -ln|H| / tau_g).
Verglichen mit iir4 (Bank & Valimaki) und pol1p im selben Umlauf (K16 5k, Thiran 1). Aufruf: python -B verlust_kaskade.py
"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np
import scipy.optimize as opt

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ue", HIER / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
FS = ue.FS


def kaskade(Gt, fn, se, M):
    """M = (Zahl der Pol-Glieder, Zahl der Nullstellen-Glieder). Pol-Glied (1 + a)/(1 + a z^-1), -1 < a <= 0;
    Nullstellen-Glied (1 + c z^-1)/(1 + c), 0 <= c < 1: beide |.| <= 1, Betrag 1 bei w = 0 (Gl. V7)."""
    Mp, Mz = M
    sel = se <= 100.0
    x = 2 * np.pi * fn[sel] / FS
    ziel = -np.log(np.interp(fn[sel], ue.F, Gt))

    def mlog(p, xx):
        a, c = p[1:1 + Mp], p[1 + Mp:]
        return (p[0] + sum(0.5 * np.log((1 + 2 * am * np.cos(xx) + am * am) / (1 + am) ** 2) for am in a)
                - sum(0.5 * np.log((1 + 2 * cm * np.cos(xx) + cm * cm) / (1 + cm) ** 2) for cm in c))

    lo = np.r_[0.0, [-0.9999] * Mp, [0.0] * Mz]
    hi = np.r_[1.0, [0.0] * Mp, [0.9999] * Mz]
    best = None
    rng = np.random.default_rng(1)
    for _ in range(60):
        p0 = np.r_[ziel[0] * rng.uniform(0.2, 1.0), rng.uniform(-0.99, 0.0, Mp), rng.uniform(0.0, 0.99, Mz)]
        r = opt.least_squares(lambda p: mlog(p, x) / ziel - 1, p0, bounds=(lo, hi))
        if best is None or r.cost < best.cost:
            best = r
    p = best.x
    g = np.exp(-p[0])
    a, c = p[1:1 + Mp], p[1 + Mp:]
    num = np.array([g * np.prod(1 + a) / np.prod(1 + c)])
    for cm in c:
        num = np.convolve(num, [1.0, cm])
    den = np.array([1.0])
    for am in a:
        den = np.convolve(den, [1.0, am])
    return num, den


def bewerte_mit(sa, L, b, den):
    o = ue.entwurf(sa, L, "pol1p", ("K", 16, 5000.0), ("thiran", 1))   # Dispersion/Stimmung wie Tabelle B
    Gt, ph = o[0], o[1]
    hl_alt = ue.antwort_pol1(*o[4]["koef"])
    hl = ue.antwort_ba(b, den)
    Gm = o[2] / np.abs(hl_alt) * np.abs(hl)
    phm = o[3] - ue.phase_von(hl_alt) + ue.phase_von(hl)
    # Stimmung nachziehen: Phase am Grundton exakt (Phasenverzug des neuen Filters gegen den alten)
    fn, _ = ue.teiltoene(Gt, ph)
    w1 = 2 * np.pi * fn[0] / FS
    d = (np.interp(fn[0], ue.F, phm) - np.interp(fn[0], ue.F, ph)) / w1
    phm = phm - ue.W * d
    return ue.bewerte(Gt, ph, Gm, phm)


print("Taste | Abklingen %: 1-12 / <=5k / hoerbar (max|H|)  fuer  pol1p | iir4 | kas P1N1 | kas P1N2 | kas P2N2")
for titel, sa, L in ue.tasten():
    Gt, ph = ue.ziel(sa, L)
    fn, se = ue.teiltoene(Gt, ph)
    zeile = f"{titel[:26]:26s}"
    for v in ("pol1p", "iir4"):
        o = ue.entwurf(sa, L, v, ("K", 16, 5000.0), ("thiran", 1))
        r = ue.bewerte(*o[:4])
        zeile += f" | {r['p12']:5.1f}/{r['p5k']:5.1f}/{r['pr']:6.1f}"
    for M in ((1, 1), (1, 2), (2, 2)):
        b, den = kaskade(Gt, fn, se, M)
        r = bewerte_mit(sa, L, b, den)
        mx = np.abs(ue.antwort_ba(b, den, np.linspace(0, np.pi, 4097))).max()
        zeile += f" | {r['p12']:5.1f}/{r['p5k']:5.1f}/{r['pr']:6.1f} ({mx:.5f})"
    print(zeile, flush=True)
