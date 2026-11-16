"""Zwei Varianten gegen das Raster: (1) Allpaesse in zwei Gruppen mit je eigenem Wert; (2) Verlust-FIR ohne den Zwang
nichtnegativer Beiwerte, und Verlust als Pol 1. Ordnung plus symmetrisches FIR."""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.optimize as opt

P = pathlib.Path(sys.argv[1])
spec = importlib.util.spec_from_file_location("ue", P / "2026-11-16_S201_umlauf_filter_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
ps, FS, RASTER = ue.ps, ue.FS, ue.RASTER
xr = 2 * np.pi * RASTER / FS


def zwei_gruppen(sa, L, K, band=5000.0):
    g, phase = ue.genau(sa, L, RASTER)
    fn = ue.teiltoene_aus_phase(phase)
    x = 2 * np.pi * fn / FS
    ph = np.interp(fn, RASTER, phase)
    nfit = max(2, min(40, int(np.sum(fn <= band))))
    k1 = K // 2

    def app(p, y):
        return ue.ap_phase(p[0], y, k1) + ue.ap_phase(p[1], y, K - k1)

    def fehler(p):
        if not (-0.98 < p[0] < 0.2 and -0.98 < p[1] < 0.2):
            return 1e12
        d = (ph[0] - app(p, x[0])) / x[0]
        dphi = (d * x[:nfit] + app(p, x[:nfit])) - ph[:nfit]
        return float(np.sum((1200 / np.log(2) * dphi / (2 * np.pi * np.arange(1, nfit + 1))) ** 2))

    a0, _, _ = ue.entwurf_steifheit(sa, L, K, band, 2.0)
    best = min((opt.minimize(fehler, s, method="Nelder-Mead", options={"xatol": 1e-5, "fatol": 1e-6, "maxiter": 600})
                for s in ([a0, a0], [a0 * 0.6, min(a0 * 1.3, -0.01) if a0 < 0 else -0.3], [-0.2, -0.8])), key=lambda r: r.fun)
    p = best.x
    d = (ph[0] - app(p, x[0])) / x[0]
    return d * xr + app(p, xr), d


def verlust_frei(sa, L, M, pol):
    """Kleinste Quadrate des relativen Verlusts ohne Vorzeichenzwang; mit `pol` dazu ein Pol b/(1 - a z^-1) (a gerastert)."""
    g, phase = ue.genau(sa, L, RASTER)
    fn = ue.teiltoene_aus_phase(phase)
    fr = np.geomspace(fn[0], 20000.0, 200)
    x = 2 * np.pi * fr / FS
    gz = ue.genau(sa, L, fr)[0]
    gew = np.sqrt(fn[0] / fr)
    best = None
    for a in (np.linspace(0.0, 0.9, 19) if pol else [0.0]):
        polb = (1 - a) / np.sqrt(1 + a * a - 2 * a * np.cos(x))
        polr = (1 - a) / np.sqrt(1 + a * a - 2 * a * np.cos(xr))
        # Betrag = polb * (c0 + sum c_m cos(m x)); Ziel in ln: klein -> linear in 1 - Betrag
        A = np.stack([polb * np.cos(m * x) for m in range(0, M + 1)], axis=1)
        c = np.linalg.lstsq(A / (1 - gz)[:, None] * gew[:, None], gz / (1 - gz) * gew, rcond=None)[0]
        betrag = polr * sum(c[m] * np.cos(m * xr) for m in range(M + 1))
        fehl = float(np.sum((((A @ c) - gz) / (1 - gz) * gew) ** 2))
        if betrag.max() < 1.0 and betrag.min() > 0 and (best is None or fehl < best[0]):
            best = (fehl, betrag, a)
    return best


was = sys.argv[2]
alle = list(ue.faelle(was))
print(was, len(alle), "Tasten")
for K in (8, 16):
    e1, e2 = [], []
    for titel, sa, L in alle:
        a, kw, ph1 = ue.entwurf_steifheit(sa, L, K, 5000.0, 2.0)
        ph2, d = zwei_gruppen(sa, L, K)
        for ph, ziel in ((ph1, e1), (ph2 if d >= 4.0 else ph1, e2)):
            f, cent, _ = ue.messe(sa, L, ph, np.ones_like(RASTER) * 0.999)
            m12 = (np.arange(len(f)) < 12) & (f < 10000)
            ziel.append((np.abs(cent[m12]).max(), np.abs(cent[f < 5000]).max()))
    e1, e2 = np.array(e1), np.array(e2)
    print(f"  K {K:2d}: ein Wert  {e1[:, 0].max():5.1f} / {e1[:, 0].mean():4.1f} Cent (1-12) | {e1[:, 1].max():6.1f} / {e1[:, 1].mean():5.1f} (unter 5 kHz)   "
          f"zwei Werte {e2[:, 0].max():5.1f} / {e2[:, 0].mean():4.1f} | {e2[:, 1].max():6.1f} / {e2[:, 1].mean():5.1f}")
for name, M, pol in (("fir5 frei", 2, False), ("fir9 frei", 4, False), ("pol + fir3", 1, True), ("pol + fir5", 2, True)):
    erg, ohne = [], 0
    for titel, sa, L in alle:
        r = verlust_frei(sa, L, M, pol)
        if r is None:
            ohne += 1
            continue
        _, phase = ue.genau(sa, L, RASTER)
        f, _, proz = ue.messe(sa, L, phase, r[1])
        m12 = (np.arange(len(f)) < 12) & (f < 10000)
        erg.append((np.abs(proz[m12]).max(), abs(proz[0])))
    e = np.array(erg)
    print(f"  Verlust {name:10s}: Abklingen 1-12 groesster {e[:, 0].max():6.1f} %, Mittel {e[:, 0].mean():5.1f} % | Grundton groesster {e[:, 1].max():5.1f} % "
          f"| ohne zulaessige Loesung (Betrag >= 1 oder < 0): {ohne} von {len(alle)}")
