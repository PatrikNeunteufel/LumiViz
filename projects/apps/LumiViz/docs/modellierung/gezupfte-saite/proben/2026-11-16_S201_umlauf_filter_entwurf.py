"""S201, 34k-6 Punkt (3) — Entwurf des Umlauf-Filters je Taste: was Ordnung und Anpassband bringen, ueber den Umfang.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> gitarre | harfe | cembalo [--fein]

Die Probe S200 (`2026-11-16_S200_umlauf_filter.py`) rechnete vier feste Fassungen an 10-14 Tasten durch den ganzen
Pruefstand. Offen blieb: 13-33 Cent an steifen Saiten (G-Saite der Gitarre, Darmsaiten der Harfe) trotz Allpaessen,
Ausreisser an den Tasten 96 / 103 der Harfe mit fir9+ap, bis 1,7 LU am Cembalo. Hier NUR der Umlauf (starrer Steg),
dafuer jede Taste und ein Raster der Entwurfsgroessen - das kostet Millisekunden je Fall:

  Teilton n des Modells: Frequenz, bei der die Phase des Umlaufs 2 pi n ist  ->  Fehler in Cent gegen den genauen Umlauf
  Abklingen des Teiltons: sigma_n = -f_n ln|E(f_n)|                         ->  Fehler in % gegen den genauen Umlauf

Entwurf (je Taste, ohne Optimierer ueber viele Groessen):
  Steifheit  K gleiche Allpaesse 1. Ordnung (a + z^-1)/(1 + a z^-1), ein Wert a, angepasst an die Phase der Teiltoene
             bis `band` Hz (hoechstens 40), jeder Teilton gleich gewichtet in Cent; der ganze Verzug so, dass der
             Grundton stimmt. K = 0, 2, 4, 8, 16, 32 - begrenzt dadurch, dass die Filter zusammen in einen Umlauf passen
             muessen (Rest >= 2 Takte).
  Verlust    pol1 (wie der Kern heute: Betrag bei f0 und 8 f0), FIR symmetrisch mit 2 M + 1 Werten (M = 2, 4),
             nichtnegative Beiwerte, angepasst am relativen Verlust vom Grundton bis `band_verlust`.

Ausgabe: je Entwurf ueber alle Tasten des Instruments der groesste und der mittlere Fehler (Teiltoene 1-12 unter
10 kHz; dazu alle Teiltoene unter 5 kHz) und die Taste des groessten Fehlers. Mit --fein je Taste eine Zeile fuer den
besten Entwurf. Schreibt nichts.
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.optimize as opt

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.parent / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
FS = ps.FS
RASTER = np.arange(1.0, 20000.0, 0.5)


def genau(saite, L, f):
    k, _ = ps.saite_wellen(saite, 2 * np.pi * np.asarray(f, dtype=float) + 0j)
    return np.exp(2 * k.imag * L), 2 * k.real * L


def teiltoene_aus_phase(phase, bis=12000.0):
    n = np.arange(1, int(phase[RASTER <= bis][-1] / (2 * np.pi)) + 1)
    return np.interp(2 * np.pi * n, phase, RASTER)


def ap_phase(a, x, K):
    return -K * np.angle((a + np.exp(-1j * x)) / (1 + a * np.exp(-1j * x)))


def entwurf_steifheit(saite, L, K, band, verzug_filter):
    """-> (a, K_wirklich, Phase des Umlaufs auf RASTER). K wird halbiert, bis alles in einen Umlauf passt."""
    g, phase = genau(saite, L, RASTER)
    fn = teiltoene_aus_phase(phase)
    x = 2 * np.pi * fn / FS
    ph = np.interp(fn, RASTER, phase)
    nfit = max(2, min(40, int(np.sum(fn <= band))))
    xr = 2 * np.pi * RASTER / FS
    while K > 0:
        def fehler(a, K=K):
            d = (ph[0] - ap_phase(a, x[0], K)) / x[0]
            dphi = (d * x[:nfit] + ap_phase(a, x[:nfit], K)) - ph[:nfit]
            return float(np.sum((1200 / np.log(2) * dphi / (2 * np.pi * np.arange(1, nfit + 1))) ** 2))
        raster = np.linspace(-0.97, 0.0, 195)
        i = int(np.argmin([fehler(r) for r in raster]))
        a = float(opt.minimize_scalar(fehler, bounds=(raster[max(i - 1, 0)], raster[min(i + 1, 194)]), method="bounded").x)
        d = (ph[0] - ap_phase(a, x[0], K)) / x[0] - verzug_filter
        if d >= 2.0:
            return a, K, (d + verzug_filter) * xr + ap_phase(a, xr, K)
        K //= 2
    return 0.0, 0, (ph[0] / x[0]) * xr


def entwurf_verlust(saite, L, art, band_verlust=20000.0):
    """-> (Betrag des Verlustfilters auf RASTER, Verzug in Takten)."""
    g, phase = genau(saite, L, RASTER)
    fn = teiltoene_aus_phase(phase)
    xr = 2 * np.pi * RASTER / FS
    if art == "pol1":
        f1, f8 = fn[0], min(8 * fn[0], 0.45 * FS)
        g1, g8 = genau(saite, L, [f1, f8])[0]
        w1, w8 = 2 * np.pi * f1 / FS, 2 * np.pi * f8 / FS
        betrag = lambda a, y: 1 / np.sqrt(1 + a * a - 2 * a * np.cos(y))
        lo, hi = 0.0, 0.95
        for _ in range(60):
            mi = 0.5 * (lo + hi)
            lo, hi = (mi, hi) if betrag(mi, w8) / betrag(mi, w1) > g8 / g1 else (lo, mi)
        a = 0.5 * (lo + hi)
        return g1 / betrag(a, w1) * betrag(a, xr), 0.0     # der kleine Phasengang des Pols ist hier vernachlaessigt
    M = int(art[3:]) // 2                                   # "fir5" -> 2, "fir9" -> 4
    fr = np.geomspace(fn[0], band_verlust, 160)
    x = 2 * np.pi * fr / FS
    ziel = 1 - genau(saite, L, fr)[0]
    gew = np.sqrt(fn[0] / fr)
    A = np.stack([np.ones_like(x)] + [1 - np.cos(m * x) for m in range(1, M + 1)], axis=1)
    l, _ = opt.nnls(A / ziel[:, None] * gew[:, None], gew)
    return 1 - l[0] - sum(l[m] * (1 - np.cos(m * xr)) for m in range(1, M + 1)), float(M)


def messe(saite, L, phase_modell, betrag_modell):
    g, phase = genau(saite, L, RASTER)
    f_genau = teiltoene_aus_phase(phase)
    f_mod = teiltoene_aus_phase(phase_modell)
    n = min(len(f_genau), len(f_mod))
    cent = 1200 * np.log2(f_mod[:n] / f_genau[:n])
    sig_g = -f_genau[:n] * np.log(np.interp(f_genau[:n], RASTER, g))
    sig_m = -f_mod[:n] * np.log(np.clip(np.interp(f_mod[:n], RASTER, betrag_modell), 1e-12, 1 - 1e-12))
    return f_genau[:n], cent, 100 * (sig_m / sig_g - 1)


def faelle(was):
    if was == "gitarre":
        for s, sa in enumerate(ps.GITARRE):
            for bund in range(0, 20):
                yield f"Saite {6 - s} Bund {bund:2d}", sa, ps.MENSUR * 2 ** (-bund / 12)
    elif was == "harfe":
        for taste in range(25, 105):
            a = ps.harfe_taste(taste)
            if a:
                yield f"Taste {taste:3d}", a[0], a[1]
    elif was == "cembalo":
        for taste in range(29, 90):
            a = ps.cembalo_taste(taste)
            yield f"Taste {taste:3d}", a[0], a[1]


if __name__ == "__main__":
    was = sys.argv[1] if len(sys.argv) > 1 else ""
    if was not in ("gitarre", "harfe", "cembalo"):
        sys.exit(__doc__)
    alle = list(faelle(was))
    print(f"{was}: {len(alle)} Tasten. Steifheit - Fehler der Teiltoene in Cent (1-12 unter 10 kHz | alle unter 5 kHz): groesster, "
          "Mittel der groessten je Taste, Taste des groessten; K wirklich (kleinster / groesster)")
    beste = None
    for band in (5000.0, 10000.0):
        for K in (0, 2, 4, 8, 16, 32):
            erg = []
            for titel, sa, L in alle:
                a, kw, phase = entwurf_steifheit(sa, L, K, band, 2.0)          # 2 Takte fuer ein FIR mit 5 Werten
                f, cent, _ = messe(sa, L, phase, np.ones_like(RASTER) * 0.999)
                m12 = (np.arange(len(f)) < 12) & (f < 10000)
                erg.append((np.abs(cent[m12]).max(), np.abs(cent[f < 5000]).max() if (f < 5000).any() else 0.0, kw, titel, a))
            e = np.array([(x[0], x[1], x[2]) for x in erg])
            i = int(np.argmax(e[:, 0]))
            print(f"  Band {band / 1000:4.0f} kHz, K {K:2d}: {e[:, 0].max():5.1f} / {e[:, 0].mean():4.1f} ({erg[i][3]}) | "
                  f"{e[:, 1].max():5.1f} / {e[:, 1].mean():4.1f} | K {int(e[:, 2].min())} ... {int(e[:, 2].max())}")
            if K == 16 and band == 5000.0:
                beste = erg
    print("Verlust - Fehler des Abklingens je Teilton in % (Teiltoene 1-12 unter 10 kHz): groesster Betrag, Mittel der "
          "groessten je Taste, Taste; dazu der Grundton allein (groesster Betrag)")
    for art in ("pol1", "fir5", "fir9"):
        for bv in ((20000.0,) if art == "pol1" else (20000.0, 10000.0, 6000.0)):
            erg = []
            for titel, sa, L in alle:
                betrag, _ = entwurf_verlust(sa, L, art, bv)
                _, phase = genau(sa, L, RASTER)
                f, _, proz = messe(sa, L, phase, betrag)
                m12 = (np.arange(len(f)) < 12) & (f < 10000)
                erg.append((np.abs(proz[m12]).max(), abs(proz[0]), titel, float(betrag.max())))
            e = np.array([(x[0], x[1], x[3]) for x in erg])
            i = int(np.argmax(e[:, 0]))
            print(f"  {art:5s} Band {bv / 1000:4.0f} kHz: {e[:, 0].max():7.1f} / {e[:, 0].mean():6.1f} ({erg[i][2]}) | Grundton {e[:, 1].max():6.1f} "
                  f"| groesster Betrag des Filters {e[:, 2].max():.6f}")
    if "--fein" in sys.argv and beste:
        print("je Taste, Steifheit mit K 16, Band 5 kHz: Cent 1-12 | Cent unter 5 kHz | K wirklich | a")
        for x in beste:
            print(f"  {x[3]}: {x[0]:5.1f} | {x[1]:5.1f} | {x[2]:2d} | {x[4]:+.3f}")
