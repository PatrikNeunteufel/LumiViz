"""S204 - UL3 a, die Ausnahmen: kurze Umlaeufe mit Teiltoenen ueber 10 kHz (Harfe 96-104, Cembalo 83-89, Gitarre Saite 1
in hohen Lagen). Dort blieb die Anpassung aus `…_S204_dispersion_biquads.py` bei 5-28 Cent.

Aufruf: python -X utf8 -B <dieses Skript>      -> Konsole (in eine .txt umleiten)

Zwei Aenderungen gegen die erste Fassung, einzeln geschaltet:
  T  das Thiran-Glied wird MIT angepasst (sein Verzug Delta ist Unbekannte, der ganzzahlige Verzug N fest), statt es
     als reinen Verzug zu behandeln und seine Phase in drei Runden nachzufuehren
  S  viele Startwerte (Zufall mit festem Samen) statt eines einzigen; das beste Ergebnis zaehlt
Je Taste und Zahl M: groesster Fehler der zaehlenden Teiltoene in Cent (genau ausgewertet), Abklingen in der Schwelle.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np
import scipy.optimize as opt

_spec = importlib.util.spec_from_file_location("bq", pathlib.Path(__file__).with_name("2026-11-17_S204_dispersion_biquads.py"))
bq = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bq)
uu, ue, weg2 = bq.uu, bq.ue, bq.weg2
FS, F, W = uu.FS, uu.F, uu.W
bq.RMAX = 0.98
STARTS = 40


def th_phase(Dl, wn):
    return ue.phase_von(ue.antwort_ba(*ue.thiran(Dl, 1), np.concatenate([[1e-6], wn])))[1:]


def passe_mit_thiran(rest_n, wn, n, M, N, rng, starts):
    kf = 1200 / np.log(2) / (2 * np.pi * n)
    kf = kf * np.where(n == 1, 10.0, 1.0)                     # der Grundton zaehlt zehnfach

    def aus(p):
        return bq.RMAX / (1 + np.exp(-p[:M])), np.pi / (1 + np.exp(-p[M:2 * M])), 0.5 + 1 / (1 + np.exp(-p[2 * M]))

    def res(p):
        r, th, Dl = aus(np.clip(p, -40, 40))
        return kf * (N * wn + th_phase(Dl, wn) + bq.bq_phase(r, th, wn) - rest_n)
    best = None
    for s in range(starts):
        p0 = np.concatenate([rng.normal(-0.5, 1.2, M), rng.normal(-1.0, 1.5, M), [rng.normal(0, 1)]])
        q = opt.least_squares(res, p0, method="trf", max_nfev=300)
        if best is None or q.cost < best.cost:
            best = q
    return aus(np.clip(best.x, -40, 40))


def taste(inst, name, sa, L):
    Gt, ph = ue.ziel(sa, L)
    fn, sg = ue.teiltoene(Gt, ph)
    m = fn < 0.45 * FS
    fk, sk = fn[m], sg[m]
    hoer = sk <= ue.SIG_REL
    for Nv in (8, 4, 2):
        if 2 * Nv + 1 <= 2 * len(fk):
            b, a, _ = uu.verlust(Gt, fn, sg, Nv, "R")
            break
    hl = ue.antwort_ba(b, a)
    rest = ph - ue.phase_von(hl)
    fz = fk[hoer]
    n = np.arange(1, len(fz) + 1)
    wn = 2 * np.pi * fz / FS
    rest_n = np.interp(fz, F, rest)
    D1 = rest_n[0] / wn[0]
    print(f"\n### {inst} {name}: f1 {fn[0]:.1f} Hz, Umlauf {D1:.1f} Takte, zaehlend {len(fz)} bis {fz[-1]:.0f} Hz, Verlust Ordnung {Nv}", flush=True)
    rng = np.random.default_rng(204)
    for M in (1, 2, 3, 4):
        zeile = f"  M {M}:"
        for N_ab in (2 * M + 1, 2 * M + 2, 2 * M + 3):         # ganzzahliger Verzug = Umlauf - N_ab (Platz fuer Kette und Thiran)
            N = int(np.floor(D1)) - N_ab
            if N < 1:
                continue
            r, th, Dl = passe_mit_thiran(rest_n, wn, n, M, N, rng, STARTS)
            v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
            if v is None:
                continue
            fe, se, c, p = v
            h = se <= ue.SIG_REL
            pin = int(np.sum((p[h] >= uu.P_LO) & (p[h] <= uu.P_HI)))
            zeile += f" | N {N}: {uu.mx(c[h]):6.2f} Cent (1-12 {uu.mx(c[:12]):5.2f}), Abkl. {pin}/{int(h.sum())}, r max {r.max():.3f}"
        print(zeile, flush=True)


ZIEL = {("Harfe", "T96"), ("Harfe", "T100"), ("Harfe", "T101"), ("Harfe", "T102"), ("Harfe", "T103"), ("Harfe", "T104"),
        ("Cembalo", "T83"), ("Cembalo", "T85"), ("Cembalo", "T88"), ("Cembalo", "T89"), ("Gitarre", "S1 B12"), ("Gitarre", "S1 B17")}
if __name__ == "__main__":
    for inst, name, sa, L in weg2.tasten_umfang():
        if (inst, name) in ZIEL:
            taste(inst, name, sa, L)
