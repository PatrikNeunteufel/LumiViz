"""S204 - UL3 a: der entworfene Umlauf-Filter im Pruefstand MIT Koerper (Lautheit, Pegel, Teiltoene am Ton).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> gitarre | cembalo | harfe

Rahmen und Messung aus `2026-11-16_S200_umlauf_filter.py` (dort: exp(-2 i k L) im Pruefstand durch die Antwort der Filter
ersetzt, die Quelle des Zupfs bleibt genau; dieselben Tasten). Verglichen werden gegen den genauen Umlauf:
  pol1   Pol 1. Ordnung, keine Steifheit (der Kern heute)
  UL3    Entwurf nach UL3 a: Verlust nach Bank & Valimaki ohne Glaettung / HF-Vorgabe, Gewicht des relativen Fehlers,
         |H| <= 1, hoechste Ordnung aus 8/4/2; Biquad-Allpaesse an den zaehlenden Teiltoenen angepasst (kleinste Zahl
         fuer 3 Cent; bei kurzem Umlauf mit Thiran-Glied, ganzzahligem Verzug und vielen Startwerten gemeinsam);
         Thiran 1. Ordnung am Grundton nachgestimmt
Je Taste: Lautheit LU | Pegel 1-2 s dB | Teiltoene 1-12 groesster Betrag Cent (Grundton) | Abfall frueh->spaet dB.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np

HIER = pathlib.Path(__file__).resolve()


def lade(datei, name):
    spec = importlib.util.spec_from_file_location(name, HIER.with_name(datei))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


s200 = lade("2026-11-16_S200_umlauf_filter.py", "s200")
ku = lade("2026-11-17_S204_kurzer_umlauf.py", "ku")
bq, uu, ue = ku.bq, ku.uu, ku.ue
FS, F, W = uu.FS, uu.F, uu.W
_baue_alt = s200.baue


def entwirf(sa, L):
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
    f_d = min(fz[-1] * 1.02, 19500.0)
    al_as = uu.as_biquads(rest, f_d)[0]
    bq.RMAX = min(0.9995, max(0.98, 1 - 0.5 * (1 + al_as.min()))) if len(al_as) else 0.98
    best = None
    M = max(len(al_as), 1)
    while True:
        r, th = bq.start(rest, f_d, M)
        r, th, _ = bq.passe_an(rest_n, wn, n, r, th)
        v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
        for _ in range(3):
            if v is None:
                break
            r, th, _ = bq.passe_an(rest_n - bq.thiran_abweichung(bq.werte_aus.thiran, wn), wn, n, r, th)
            v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
        if v is None:
            break
        c = uu.mx(v[2][v[1] <= ue.SIG_REL])
        if best is None or c < best[0]:
            best = (c, r, th)
        if c <= 3.0 or M >= bq.MMAX:
            break
        M = min(int(np.ceil(1.5 * M)), bq.MMAX) if M >= 4 else M + 1
    D1 = rest_n[0] / wn[0]
    if (best is None or best[0] > 3.0) and D1 < 90:            # kurzer Umlauf: gemeinsam mit Thiran, N und Startwerten
        rng = np.random.default_rng(204)
        for M in (3, 4):
            for N_ab in (2 * M + 1, 2 * M + 2, 2 * M + 3):
                N = int(np.floor(D1)) - N_ab
                if N < 1:
                    continue
                r, th, _ = ku.passe_mit_thiran(rest_n, wn, n, M, N, rng, 20)
                v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
                if v is not None:
                    c = uu.mx(v[2][v[1] <= ue.SIG_REL])
                    if best is None or c < best[0]:
                        best = (c, r, th)
    c, r, th = best
    hd = ue.biquads(-r, np.cos(th))
    rest2 = rest - ue.phase_von(hd)
    f1, w1 = fn[0], 2 * np.pi * fn[0] / FS
    Dtot = np.interp(f1, F, rest2) / w1
    Nint = int(np.floor(Dtot - 0.5))
    soll = np.interp(f1, F, rest2) - w1 * Nint
    Dl = Dtot - Nint
    pf = lambda D: ue.phase_von(ue.antwort_ba(*ue.thiran(D, 1), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    return dict(b=b, a=a, r=r, th=th, N=Nint, th1=ue.thiran(Dl, 1), cent=c, ordnung=Nv)


def baue(fassung, saite, L, f0):
    if fassung != "UL3":
        return _baue_alt(fassung, saite, L, f0)
    e = entwirf(saite, L)
    baue.info = f"[Ordnung {e['ordnung']}, {len(e['r'])} Biquads, starr {e['cent']:.1f} Cent]"

    def E(w):
        zi = np.exp(-1j * w / FS)
        h = np.polyval(e["b"][::-1], zi) / np.polyval(e["a"][::-1], zi)
        for r, t in zip(e["r"], e["th"]):
            al, c = -r, np.cos(t)
            h = h * (al * al + 2 * al * c * zi + zi * zi) / (1 + 2 * al * c * zi + al * al * zi * zi)
        bt, at = e["th1"]
        return h * np.polyval(bt[::-1], zi) / np.polyval(at[::-1], zi) * zi ** e["N"]
    return E


s200.baue = baue
s200.FASSUNGEN = ("pol1", "UL3")

if __name__ == "__main__":
    import json
    ps = s200.ps
    was = sys.argv[1] if len(sys.argv) > 1 else ""
    if was == "gitarre":
        kp = ps.lade_koerper("classical_guitar")
        datei = len(json.loads((ps.KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))["modes"])
        faelle = []
        for s, bund in ((0, 0), (0, 7), (1, 3), (2, 5), (3, 2), (4, 0), (4, 8), (5, 0), (5, 5), (5, 12)):
            sa = ps.GITARRE[s]
            L = ps.MENSUR * 2 ** (-bund / 12)
            faelle.append((f"Saite {6 - s} Bund {bund}", sa, L, min(ps.ZUPFORT, 0.5 * L), 0.002, 30.0, 0.0075,
                           kp["tasten"].index(sa["taste"]), ps.hz_von(sa["taste"] + bund)))
        s200.laufe("Gitarre", faelle, kp, datei)
    elif was in ("cembalo", "harfe"):
        name, fn_, tasten = (("harpsichord", ps.cembalo_taste, (29, 36, 43, 50, 57, 64, 71, 78, 84, 89)) if was == "cembalo"
                             else ("concert_harp", ps.harfe_taste, (26, 31, 36, 43, 48, 55, 60, 67, 72, 79, 84, 91, 96, 103)))
        kp = ps.lade_koerper(name)
        datei = len(json.loads((ps.KOERPER / f"{name}.lvinstbody").read_text(encoding="utf-8"))["modes"])
        faelle = []
        for taste in tasten:
            x = fn_(taste)
            if x:
                sa, L, xp, d, winkel, breite, _ = x
                faelle.append((f"Taste {taste}", sa, L, xp, d, winkel, breite, kp["tasten"].index(taste), ps.hz_von(taste)))
        s200.laufe(was.capitalize(), faelle, kp, datei)
    else:
        sys.exit(__doc__)
