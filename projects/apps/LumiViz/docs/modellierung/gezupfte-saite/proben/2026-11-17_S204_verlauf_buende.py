"""S204 - UL3 a: Verlauf der Beiwerte des Umlauf-Filters laengs der Buende (fuer Bend und Slide).

Aufruf: python -X utf8 -B <dieses Skript>      -> Konsole (in eine .txt umleiten)

Je Gitarrensaite die Buende 0-19 mit fester Zahl M von Biquads (Abel-Smith-Zahl + 2, mindestens 8). Drei Wege:
  unabhaengig  jeder Bund fuer sich (Start nach Abel & Smith), Biquads nach Winkel sortiert
  fortgesetzt  Bund 0 wie oben, jeder weitere Bund startet von der Loesung des Bunds davor
  gebunden     logit(Radius) und logit(Winkel) jedes Biquads als Polynom (Grad 2, 3) in x = Bund / 19, alle Buende
               gemeinsam angepasst; ausgewertet an den Buenden und auf halbem Weg (dort nicht angepasst)
Gemessen: (1) Fehler an den Buenden selbst; (2) groesster Sprung von Winkel und Radius eines Biquads zwischen
Nachbarbuenden; (3) die Probe, auf die es ankommt: auf halbem Weg zwischen zwei Buenden (Viertelton) die Beiwerte linear
aus den Nachbarn gemittelt (Radius, Winkel; Verlust: b und a direkt), der Umlauf damit gegen das Ziel dieser Laenge -
Cent und Abklingen der zaehlenden Teiltoene (UL1 a, UL2 a), groesster Betrag des gemittelten Verlustfilters.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np

_spec = importlib.util.spec_from_file_location("bq", pathlib.Path(__file__).with_name("2026-11-17_S204_dispersion_biquads.py"))
bq = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bq)
uu, ue = bq.uu, bq.ue
ps = ue.ps
FS, F, W = uu.FS, uu.F, uu.W
bq.RMAX = 0.99


def lage(sa, L):
    Gt, ph = ue.ziel(sa, L)
    fn, sg = ue.teiltoene(Gt, ph)
    m = fn < 0.45 * FS
    hoer = sg[m] <= ue.SIG_REL
    return Gt, ph, fn, sg, fn[m][hoer]


def passe(Gt, ph, fn, fz, b, a, r0, th0):
    hl = ue.antwort_ba(b, a)
    rest = ph - ue.phase_von(hl)
    n = np.arange(1, len(fz) + 1)
    wn = 2 * np.pi * fz / FS
    rest_n = np.interp(fz, F, rest)
    r, th, _ = bq.passe_an(rest_n, wn, n, r0, th0)
    v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
    for _ in range(3):
        if v is None:
            break
        r, th, _ = bq.passe_an(rest_n - bq.thiran_abweichung(bq.werte_aus.thiran, wn), wn, n, r, th)
        v = bq.werte_aus(Gt, ph, fn, hl, rest, r, th)
    return r, th, v


def masse(v):
    fe, se, c, p = v
    h = se <= ue.SIG_REL
    return uu.mx(c[h]), int(np.sum((p[h] >= uu.P_LO) & (p[h] <= uu.P_HI))), int(h.sum()), uu.mx(p[h])


def gebunden(tab, M, P, r0, th0):
    """Alle Buende gemeinsam: logit(Radius) und logit(Winkel) jedes Biquads sind Polynome vom Grad P in x = Bund / 19;
    der Verzug je Bund bleibt frei (er wird ohnehin am Grundton gestimmt). -> Funktion x -> (r, th)"""
    import scipy.optimize as opt
    K = len(tab)
    xs = np.arange(K) / (K - 1)
    V = np.vander(xs, P + 1, increasing=True)                 # K x (P+1)
    dat = []
    for t in tab:
        fz = t["fz"]
        dat.append(dict(wn=2 * np.pi * fz / FS, n=np.arange(1, len(fz) + 1), rest_n=np.interp(fz, F, t["rest"]), kor=0.0))
    nres = [len(d["wn"]) for d in dat]
    off = np.concatenate([[0], np.cumsum(nres)])
    npar = 2 * M * (P + 1) + K

    def aus(p, v):
        A = p[:M * (P + 1)].reshape(M, P + 1)
        B = p[M * (P + 1):2 * M * (P + 1)].reshape(M, P + 1)
        return bq.RMAX / (1 + np.exp(-np.clip(A @ v, -40, 40))), np.pi / (1 + np.exp(-np.clip(B @ v, -40, 40)))

    def res(p):
        out = np.empty(off[-1])
        for k, d in enumerate(dat):
            r, th = aus(p, V[k])
            kf = 1200 / np.log(2) / (2 * np.pi * d["n"])
            out[off[k]:off[k + 1]] = kf * (p[2 * M * (P + 1) + k] * d["wn"] + bq.bq_phase(r, th, d["wn"]) - (d["rest_n"] - d["kor"]))
        return out

    def jac(p):
        J = np.zeros((off[-1], npar))
        for k, d in enumerate(dat):
            r, th = aus(p, V[k])
            wc = d["wn"][:, None]
            kf = (1200 / np.log(2) / (2 * np.pi * d["n"]))[:, None]
            dr = kf * 2 * (bq._g_r(wc - th, r) + bq._g_r(wc + th, r)) * (r * (1 - r / bq.RMAX))
            dt = kf * 2 * (bq._g_u(wc + th, r) - bq._g_u(wc - th, r)) * (th * (1 - th / np.pi))
            sl = slice(off[k], off[k + 1])
            J[sl, :M * (P + 1)] = (dr[:, :, None] * V[k][None, None, :]).reshape(len(wc), -1)
            J[sl, M * (P + 1):2 * M * (P + 1)] = (dt[:, :, None] * V[k][None, None, :]).reshape(len(wc), -1)
            J[sl, 2 * M * (P + 1) + k] = kf[:, 0] * d["wn"]
        return J
    p = np.zeros(npar)
    p[:M * (P + 1)].reshape(M, P + 1)[:, 0] = np.log(r0 / (bq.RMAX - r0))
    p[M * (P + 1):2 * M * (P + 1)].reshape(M, P + 1)[:, 0] = np.log(th0 / (np.pi - th0))
    for k, d in enumerate(dat):
        p[2 * M * (P + 1) + k] = (d["rest_n"][0] - bq.bq_phase(r0, th0, d["wn"][:1])[0]) / d["wn"][0]
    for runde in range(3):                                     # Phase des Thiran-Glieds je Bund nachfuehren
        p = opt.least_squares(res, p, jac=jac, method="trf", max_nfev=600, x_scale="jac").x
        for k, (t, d) in enumerate(zip(tab, dat)):
            r, th = aus(p, V[k])
            if bq.werte_aus(t["Gt"], t["ph"], t["fn"], ue.antwort_ba(t["b"], t["a"]), t["rest"], r, th) is not None:
                d["kor"] = bq.thiran_abweichung(bq.werte_aus.thiran, d["wn"])
    return lambda x: aus(p, x ** np.arange(P + 1))


for s in range(6):
    sa = ps.GITARRE[s]
    Ls = [ps.MENSUR * 2 ** (-k / 12) for k in range(20)]
    tab = []
    for L in Ls:
        Gt, ph, fn, sg, fz = lage(sa, L)
        b, a, _ = uu.verlust(Gt, fn, sg, 8, "R")
        rest = ph - ue.phase_von(ue.antwort_ba(b, a))
        nap = len(uu.as_biquads(rest, min(fz[-1] * 1.02, 19500.0))[0])
        tab.append(dict(L=L, Gt=Gt, ph=ph, fn=fn, sg=sg, fz=fz, b=b, a=a, rest=rest, nap=nap))
    M = max(8, max(t["nap"] for t in tab) + 2)
    print(f"\n### Gitarre Saite {6 - s}: Abel-Smith-Zahl ueber die Buende {min(t['nap'] for t in tab)}-{max(t['nap'] for t in tab)}, feste Zahl M = {M}")
    for weg in ("unabhaengig", "fortgesetzt"):
        R, T = [], []
        schlecht = 0.0
        for k, t in enumerate(tab):
            if weg == "unabhaengig" or k == 0:
                r0, th0 = bq.start(t["rest"], min(t["fz"][-1] * 1.02, 19500.0), M)
            else:
                r0, th0 = R[-1], T[-1]
            r, th, v = passe(t["Gt"], t["ph"], t["fn"], t["fz"], t["b"], t["a"], r0, th0)
            if weg == "unabhaengig":
                i = np.argsort(th)
                r, th = r[i], th[i]
            R.append(r)
            T.append(th)
            schlecht = max(schlecht, masse(v)[0])
        R, T = np.array(R), np.array(T)
        dth = np.abs(np.diff(T, axis=0)).max() * FS / (2 * np.pi)
        dr = np.abs(np.diff(R, axis=0)).max()
        print(f"  {weg}: an den Buenden groesster Fehler {schlecht:5.2f} Cent | groesster Sprung zwischen Nachbarbuenden: Winkel {dth:7.0f} Hz, Radius {dr:.3f}")
        cm, pm, hm, aus = [], [], [], 0
        for k in range(19):
            L = ps.MENSUR * 2 ** (-(k + 0.5) / 12)
            Gt, ph, fn, sg, fz = lage(sa, L)
            b = 0.5 * (tab[k]["b"] + tab[k + 1]["b"])
            a = 0.5 * (tab[k]["a"] + tab[k + 1]["a"])
            hm.append(uu.bv.maxbetrag(b, a)[0])
            hl = ue.antwort_ba(b, a)
            v = bq.werte_aus(Gt, ph, fn, hl, ph - ue.phase_von(hl), 0.5 * (R[k] + R[k + 1]), 0.5 * (T[k] + T[k + 1]))
            c, drin, nh, p = masse(v)
            cm.append(c)
            pm.append(p)
            aus += drin != nh
        print(f"     auf halbem Weg (gemittelte Beiwerte): Cent Median {np.median(cm):6.2f}, groesster {max(cm):7.2f} (zwischen Bund {int(np.argmax(cm))} und {int(np.argmax(cm)) + 1}) | "
              f"Abklingen groesster Fehler {max(pm):6.2f} %, Stellen ausserhalb der Schwelle {aus}/19 | groesster Betrag des Verlusts {max(hm):.6f}", flush=True)
    # gebunden: Radius und Winkel jedes Biquads als Polynom laengs der Saite, alle Buende gemeinsam angepasst
    r0, th0 = bq.start(tab[0]["rest"], min(tab[0]["fz"][-1] * 1.02, 19500.0), M)
    for P in (2, 3):
        f = gebunden(tab, M, P, r0, th0)
        erg = {}
        for halb in (0, 1):
            cm, pm, aus = [], [], 0
            for k in range(20 - halb):
                x = (k + 0.5 * halb) / 19
                if halb:
                    Gt, ph, fn, sg, fz = lage(sa, ps.MENSUR * 2 ** (-(k + 0.5) / 12))
                    b, a = 0.5 * (tab[k]["b"] + tab[k + 1]["b"]), 0.5 * (tab[k]["a"] + tab[k + 1]["a"])
                else:
                    t = tab[k]
                    Gt, ph, fn, b, a = t["Gt"], t["ph"], t["fn"], t["b"], t["a"]
                hl = ue.antwort_ba(b, a)
                v = bq.werte_aus(Gt, ph, fn, hl, ph - ue.phase_von(hl), *f(x))
                c, drin, nh, p_ = masse(v)
                cm.append(c)
                pm.append(p_)
                aus += drin != nh
            erg[halb] = (np.median(cm), max(cm), int(np.argmax(cm)), max(pm), aus)
        print(f"  gebunden Grad {P} ({2 * M * (P + 1)} Zahlen je Saite): an den Buenden Cent Median {erg[0][0]:5.2f}, groesster {erg[0][1]:6.2f} (Bund {erg[0][2]}) | "
              f"auf halbem Weg Median {erg[1][0]:5.2f}, groesster {erg[1][1]:6.2f} (nach Bund {erg[1][2]}) | Abklingen groesster Fehler {max(erg[0][3], erg[1][3]):5.2f} %, "
              f"ausserhalb der Schwelle {erg[0][4] + erg[1][4]}/39", flush=True)
