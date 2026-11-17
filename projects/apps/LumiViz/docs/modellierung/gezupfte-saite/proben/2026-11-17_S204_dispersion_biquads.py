"""S204 - Dispersion des Umlaufs als Kette von Biquad-Allpaessen, je Taste vorab angepasst.

Aufruf: python -X utf8 -B <dieses Skript> [stich | Instrument ...]     -> Konsole (in eine .txt umleiten)

Start: Platzierung nach Abel & Smith an unserer Zielphase (`as_biquads` in `…_S204_umlauf_umfang.py`); hat die Platzierung
weniger als M Biquads, kommen Biquads mit reellem Doppelpol dazu (das sind je zwei gleiche Allpaesse 1. Ordnung).
Danach kleinste Quadrate (scipy `least_squares`) ueber Radius und Winkel jedes Biquads und den Verzug:
Fehler je zaehlendem Teilton in Cent, linearisiert als  -(1200/ln 2) (phi_m(f_n) - 2 pi n) / (2 pi n).
Ausgewertet wird danach genau (Thiran 1 am Grundton nachgestimmt, Teilton = Phase 2 pi n).
Je Taste die Reihe M = Start, dann verdoppelt bis hoechstens MMAX oder bis alle zaehlenden Teiltoene in ZIEL Cent liegen.
Zaehlend: sigma <= 100 1/s (wie Bericht 11). Verlust: Weg R (Gewicht des relativen Fehlers), hoechste Ordnung aus 8/4/2.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
import time
sys.dont_write_bytecode = True
import numpy as np
import scipy.optimize as opt

_spec = importlib.util.spec_from_file_location("uu", pathlib.Path(__file__).with_name("2026-11-17_S204_umlauf_umfang.py"))
uu = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(uu)
ue, weg2 = uu.ue, uu.weg2
FS, F, W = uu.FS, uu.F, uu.W
ZIEL = (10.0, 3.0)
MMAX = 160
RMAX = 0.9995


def bq_phase(r, th, w):
    """Phasenverzug (positiv) einer Kette von Biquad-Allpaessen mit Polen r e^{+-i th}; w Spalte."""
    w = w[:, None]
    s = 2 * w + 2 * np.arctan2(r * np.sin(w - th), 1 - r * np.cos(w - th)) + 2 * np.arctan2(r * np.sin(w + th), 1 - r * np.cos(w + th))
    return s.sum(axis=1)


def _g_r(u, r):
    return np.sin(u) / (1 - 2 * r * np.cos(u) + r * r)


def _g_u(u, r):
    return (r * np.cos(u) - r * r) / (1 - 2 * r * np.cos(u) + r * r)


def passe_an(rest_n, wn, n, r0, th0):
    """rest_n: Restphase (nach Verlust) an den zaehlenden Teiltoenen; -> r, th, Verzug D. Jacobi-Matrix geschlossen."""
    M = len(r0)
    kf = 1200 / np.log(2) / (2 * np.pi * n)
    wc = wn[:, None]

    def aus(p):
        pc = np.clip(p[:2 * M], -40, 40)
        return RMAX / (1 + np.exp(-pc[:M])), np.pi / (1 + np.exp(-pc[M:])), p[2 * M]

    def res(p):
        r, th, D = aus(p)
        return kf * (D * wn + bq_phase(r, th, wn) - rest_n)

    def jac(p):
        r, th, D = aus(p)
        dr = 2 * (_g_r(wc - th, r) + _g_r(wc + th, r)) * (r * (1 - r / RMAX))
        dt = 2 * (_g_u(wc + th, r) - _g_u(wc - th, r)) * (th * (1 - th / np.pi))
        return kf[:, None] * np.hstack([dr, dt, wc])
    D0 = (rest_n[0] - bq_phase(r0, th0, wn[:1])[0]) / wn[0]
    p0 = np.concatenate([np.log(r0 / (RMAX - r0)), np.log(th0 / (np.pi - th0)), [D0]])
    q = opt.least_squares(res, p0, jac=jac, method="trf", max_nfev=400, x_scale="jac")
    return aus(q.x)


def start(rest, f_d, M):
    al, cc = uu.as_biquads(rest, f_d)
    r, th = list(-al), list(np.arccos(np.clip(cc, -1, 1)))
    k = 0
    while len(r) < M:                       # reelle Doppelpole dazu, leicht gespreizt
        r.append(0.45)
        th.append(0.02 + 0.03 * k)
        k += 1
    return np.clip(np.array(r[:M]), 0.02, RMAX - 1e-3), np.clip(np.array(th[:M]), 1e-3, np.pi - 1e-3)


def werte_aus(Gt, ph, fn, hl, rest, r, th):
    al, cc = -r, np.cos(th)
    hd = ue.biquads(al, cc)
    rest2 = rest - ue.phase_von(hd)
    f1, w1 = fn[0], 2 * np.pi * fn[0] / FS
    Dtot = np.interp(f1, F, rest2) / w1
    if Dtot < 2.0:
        return None
    Nint = int(np.floor(Dtot - 0.5))
    soll = np.interp(f1, F, rest2) - w1 * Nint
    Dl = Dtot - Nint
    pf = lambda D: ue.phase_von(ue.antwort_ba(*ue.thiran(D, 1), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    hf = ue.antwort_ba(*ue.thiran(Dl, 1))
    werte_aus.thiran = Dl
    return uu.je_teilton(Gt, ph, np.abs(hl * hd * hf), ue.phase_von(hl * hd * hf) + W * Nint)


def thiran_abweichung(Dl, wn):
    """Phase des Thiran-Glieds 1. Ordnung minus reiner Verzug Dl (an den Teiltoenen)."""
    return ue.phase_von(ue.antwort_ba(*ue.thiran(Dl, 1), np.concatenate([[1e-6], wn])))[1:] - Dl * wn


def taste(inst, name, sa, L):
    Gt, ph = ue.ziel(sa, L)
    fn, sg = ue.teiltoene(Gt, ph)
    m = fn < 0.45 * FS
    fk, sk = fn[m], sg[m]
    hoer = sk <= ue.SIG_REL
    b = a = None
    for N in (8, 4, 2):
        if 2 * N + 1 <= 2 * len(fk):
            b, a, _ = uu.verlust(Gt, fn, sg, N, "R")
            break
    if b is None:
        return None
    hl = ue.antwort_ba(b, a)
    rest = ph - ue.phase_von(hl)
    fz = fk[hoer]
    n = np.arange(1, len(fz) + 1)
    wn = 2 * np.pi * fz / FS
    rest_n = np.interp(fz, F, rest)
    f_d = min(fz[-1] * 1.02, 19500.0)
    al_as = uu.as_biquads(rest, f_d)[0]
    nap = len(al_as)
    # Grenze des Polradius: halber Abstand der engsten Abel-Smith-Platzierung zum Einheitskreis, mindestens 0,98.
    # Ein Pol naeher am Kreis traegt eine schmale Spitze der Gruppenlaufzeit und verfaelscht dort das Abklingen.
    global RMAX
    RMAX = min(0.9995, max(0.98, 1 - 0.5 * (1 + al_as.min()))) if nap else 0.98
    Dmax = fn[0] and FS / fn[0]
    print(f"\n### {inst} {name}: f1 {fn[0]:.2f} Hz, Umlauf {Dmax:.1f} Takte, zaehlend {len(fz)} bis {fz[-1]:.0f} Hz, Abel-Smith {nap} Biquads", flush=True)
    M = max(nap, 1)
    erg = {}
    while True:
        t0 = time.perf_counter()
        r0, th0 = start(rest, f_d, M)
        v0 = werte_aus(Gt, ph, fn, hl, rest, r0, th0)
        r, th, D = passe_an(rest_n, wn, n, r0, th0)
        v = werte_aus(Gt, ph, fn, hl, rest, r, th)
        for _ in range(3):                  # die Phase des Thiran-Glieds (kein reiner Verzug) mit anpassen
            if v is None:
                break
            r, th, D = passe_an(rest_n - thiran_abweichung(werte_aus.thiran, wn), wn, n, r, th)
            v = werte_aus(Gt, ph, fn, hl, rest, r, th)
        dt = time.perf_counter() - t0
        if v is None:
            print(f"  M {M:3d}: passt nicht in den Umlauf")
            break
        fe, se, c, p = v
        h = se <= ue.SIG_REL
        c0 = uu.mx(v0[2][v0[1] <= ue.SIG_REL]) if v0 is not None else float("nan")
        pin = int(np.sum((p[h] >= uu.P_LO) & (p[h] <= uu.P_HI)))
        erg[M] = uu.mx(c[h])
        print(f"  M {M:3d} ({2 * M:3d} Mult.) | Start {c0:8.2f} Cent | angepasst: 1-12 {uu.mx(c[:12]):6.2f}, zaehlend {uu.mx(c[h]):7.2f}, alle {uu.mx(c):8.2f} Cent | "
              f"groesster Radius {r.max():.4f} | Abklingen zaehlend {uu.mx(p[h]):6.2f} %, in der Schwelle {pin}/{int(h.sum())} | {dt:5.1f} s", flush=True)
        if erg[M] <= min(ZIEL) or M >= MMAX:
            break
        M = min(int(np.ceil(1.5 * M)), MMAX) if M >= 4 else M + 1
    return inst, nap, erg, pin == int(h.sum()) if erg else False


def zusammenfassung(liste):
    print("\n\n===== Zusammenfassung (zaehlende Teiltoene, groesster Fehler je Taste) =====")
    print("Instrument | Tasten | Abel-Smith-Zahl Median / max | kleinstes M fuer 10 Cent: Tasten, Median / max | fuer 3 Cent: Tasten, Median / max | "
          "bestes Ergebnis Cent Median / max | Abklingen in der Schwelle")
    for inst in dict.fromkeys(x[0] for x in liste):
        z = [x for x in liste if x[0] == inst]
        nap = np.array([x[1] for x in z])
        zeile = f"{inst:8s} | {len(z):3d} | {np.median(nap):4.0f} / {nap.max():3d}"
        for ziel in ZIEL:
            ms = [min([M for M, c in x[2].items() if c <= ziel], default=None) for x in z]
            ms = np.array([m for m in ms if m is not None])
            zeile += f" | {len(ms):3d}, {np.median(ms) if len(ms) else float('nan'):4.0f} / {ms.max() if len(ms) else 0:3d}"
        best = np.array([min(x[2].values()) if x[2] else np.nan for x in z])
        zeile += f" | {np.nanmedian(best):5.2f} / {np.nanmax(best):6.2f} | {sum(bool(x[3]) for x in z):3d}"
        print(zeile)


STICH = {("Gitarre", "S6 B0"), ("Gitarre", "S3 B2"), ("Gitarre", "S1 B12"), ("Harfe", "T27"), ("Harfe", "T46"), ("Harfe", "T60"),
         ("Harfe", "T96"), ("Harfe", "T103"), ("Cembalo", "T29"), ("Cembalo", "T36"), ("Cembalo", "T60"), ("Cembalo", "T84"),
         ("Laute", "C1.1 B0"), ("Laute", "C7.1 B5"), ("Oud", "C1.1 B0"), ("Oud", "C5.1 B5")}

if __name__ == "__main__":
    arg = sys.argv[1:] or ["stich"]
    if "jeder" in arg:                      # jeder Teilton bis 20 kHz zaehlt (statt sigma <= 100 1/s)
        arg.remove("jeder")
        ue.SIG_REL = 1e12
        print("Zaehlend: jeder Teilton bis 20 kHz")
    liste = []
    for inst, name, sa, L in weg2.tasten_umfang():
        if arg == ["stich"]:
            if (inst, name) not in STICH:
                continue
        elif arg != ["alle"] and inst not in arg:
            continue
        e = taste(inst, name, sa, L)
        if e is not None:
            liste.append(e)
    zusammenfassung(liste)
