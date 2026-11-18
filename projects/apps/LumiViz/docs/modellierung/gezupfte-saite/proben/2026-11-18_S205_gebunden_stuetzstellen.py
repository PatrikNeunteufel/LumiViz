"""S205 - UL3 a: gebundene Anpassung der Biquad-Kette je Saite, mit Stuetzstellen zwischen den Buenden und dem
gebrochenen Verzug IN der Anpassung (Fortsetzung von `2026-11-17_S204_verlauf_buende.py`, Weg "gebunden").

Aufruf: python -X utf8 -B <dieses Skript> <Instrument> [glied=T1,T3,L5] [grad=2,3] [saiten=1,2]
        Instrument: Gitarre | Laute | Oud        -> Konsole (in eine .txt umleiten)

Was gegen S204 anders ist:
  Stuetzstellen  angepasst wird an den Buenden UND auf halbem Weg (2 nb + 1 Stellen); geprueft wird zusaetzlich an
                 sechs Stellen je Bund dazwischen (Achtel), die die Anpassung nicht kennt. Die Polynom-Beiwerte ab
                 Grad 1 sind gedaempft (reg=, Vorgabe 0,05): ohne das kippte an der Laute ein ueberzaehliges Biquad
                 zwischen zwei Stuetzstellen um (logit in der Saettigung, 10 Cent). Der Verlustfilter abseits der
                 Buende: b und a linear zwischen den Nachbarbuenden (S204: bleibt in der Schwelle, |H| <= 1).
  Verzug         das Glied fuer den gebrochenen Verzug steht mit seiner genauen Phase in der Anpassung; sein Verzug
                 ist je Stuetzstelle eine Unbekannte, der ganzzahlige Verzug folgt der Regel der Laufzeit
                 (Bruchteil im Bereich des Glieds) und wird zwischen den Runden nachgefuehrt. Der Grundton zaehlt
                 zehnfach; ausgewertet wird genau (Grundton am Glied nachgestimmt).
  Glied          T<N> Thiran-Allpass der Ordnung N, Verzug in [N - 0,5 ; N + 0,5)
                 L<N> Lagrange-Interpolator der Ordnung N (FIR), Verzug in [(N - 1)/2 ; (N + 1)/2) - sein Betrag unter 1
                      geht in das Abklingen ein
Die Kette: logit(Radius / RMAX) und logit(Winkel / pi) jedes Biquads als Polynom (Legendre, Grad P) in x = Bund / nb.
Zaehlende Teiltoene nach UL1 a, Schwellen nach UL2 a (3 Cent; Abklingen -30,1 / +26,2 %).
Buende: Gitarre 0-19; Laute und Oud 0-12 (Annahme - die Oud hat keine Buende, gemeint ist die Lage laengs der Saite).
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
import time
sys.dont_write_bytecode = True
import numpy as np
import scipy.optimize as opt

_spec = importlib.util.spec_from_file_location("bq", pathlib.Path(__file__).with_name("2026-11-17_S204_dispersion_biquads.py"))
bq = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bq)
uu, ue = bq.uu, bq.ue
ps = ue.ps
FS, F, W = uu.FS, uu.F, uu.W
bq.RMAX = 0.99
KF = 1200 / np.log(2)
REG = 0.05                             # Gewicht der Daempfung der Polynom-Beiwerte ab Grad 1 (in Cent je Einheit)


# ------------------------------------------------------------------ das Glied fuer den gebrochenen Verzug
def glied(g, D):
    return ue.thiran(D, g[1]) if g[0] == "T" else ue.lagrange(D, g[1])


def unten(g):
    return g[1] - 0.5 if g[0] == "T" else (g[1] - 1) / 2


def glied_phase(g, D, wn):
    return ue.phase_von(ue.antwort_ba(*glied(g, D), np.concatenate([[1e-6], wn])))[1:]


# ------------------------------------------------------------------ Stellen laengs der Saite
def stelle(sa, mens, u, nb, bund):
    """u: Lage in Buenden (auch gebrochen). bund: Liste der Buende (fuer den Verlustfilter) oder None fuer einen Bund."""
    Gt, ph = ue.ziel(sa, mens * 2 ** (-u / 12))
    fn, sg = ue.teiltoene(Gt, ph)
    m = fn < 0.45 * FS
    fz = fn[m][sg[m] <= ue.SIG_REL]
    if bund is None:
        b = a = None
        for N in (8, 4, 2):
            if 2 * N + 1 <= 2 * int(m.sum()):
                b, a, _ = uu.verlust(Gt, fn, sg, N, "R")
                break
    else:
        k = min(int(np.floor(u)), nb - 1)
        t = u - k
        b = (1 - t) * bund[k]["b"] + t * bund[k + 1]["b"]
        a = (1 - t) * bund[k]["a"] + t * bund[k + 1]["a"]
    hl = ue.antwort_ba(b, a)
    rest = ph - ue.phase_von(hl)
    return dict(u=u, x=u / nb, Gt=Gt, ph=ph, fn=fn, fz=fz, b=b, a=a, hl=hl, rest=rest, wn=2 * np.pi * fz / FS,
                n=np.arange(1, len(fz) + 1), rest_n=np.interp(fz, F, rest))


def werte(s, r, th, g):
    """genaue Auswertung an einer Stelle: Grundton am Glied nachgestimmt -> (Cent groesster, in der Schwelle, zaehlend, Abklingen %)"""
    hd = ue.biquads(-r, np.cos(th))
    rest2 = s["rest"] - ue.phase_von(hd)
    f1 = s["fn"][0]
    w1 = 2 * np.pi * f1 / FS
    tot = np.interp(f1, F, rest2)
    lo = unten(g)
    if tot / w1 < lo + 1:
        return None
    Nint = int(np.floor(tot / w1 - lo))
    soll = tot - w1 * Nint
    Dl = tot / w1 - Nint
    pf = lambda D: ue.phase_von(ue.antwort_ba(*glied(g, D), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    hf = ue.antwort_ba(*glied(g, Dl))
    v = uu.je_teilton(s["Gt"], s["ph"], np.abs(s["hl"] * hd * hf), ue.phase_von(s["hl"] * hd * hf) + W * Nint)
    if v is None:
        return None
    fe, se, c, p = v
    h = se <= ue.SIG_REL
    return uu.mx(c[h]), int(np.sum((p[h] >= uu.P_LO) & (p[h] <= uu.P_HI))), int(h.sum()), uu.mx(p[h])


# ------------------------------------------------------------------ gebundene Anpassung
def gebunden(st, M, P, r0, th0, g):
    """st: Stuetzstellen. -> Funktion x -> (r, th), Zahl der Runden"""
    K = len(st)
    V = np.polynomial.legendre.legvander(2 * np.array([s["x"] for s in st]) - 1, P)      # K x (P+1)
    nres = [len(s["wn"]) for s in st]
    off = np.concatenate([[0], np.cumsum(nres)])
    nk = M * (P + 1)
    npar = 2 * nk + K
    idx = np.array([i for i in range(2 * nk) if i % (P + 1)])      # Beiwerte ab Grad 1: gedaempft (REG)
    lo = unten(g)
    kf = [KF / (2 * np.pi * s["n"]) * np.where(s["n"] == 1, 10.0, 1.0) for s in st]
    Nint = np.zeros(K)
    rein = [True]                      # erste Stufe: reiner Verzug je Stelle, frei (die Kette findet ihren Verzug)

    def aus(p, v):
        A = p[:nk].reshape(M, P + 1)
        B = p[nk:2 * nk].reshape(M, P + 1)
        return bq.RMAX / (1 + np.exp(-np.clip(A @ v, -40, 40))), np.pi / (1 + np.exp(-np.clip(B @ v, -40, 40)))

    def res(p):
        out = np.empty(off[-1] + len(idx))
        out[off[-1]:] = REG * p[idx]
        for k, s in enumerate(st):
            r, th = aus(p, V[k])
            D = p[2 * nk + k]
            vz = D * s["wn"] if rein[0] else Nint[k] * s["wn"] + glied_phase(g, D, s["wn"])
            out[off[k]:off[k + 1]] = kf[k] * (vz + bq.bq_phase(r, th, s["wn"]) - s["rest_n"])
        return out

    def jac(p):
        J = np.zeros((off[-1] + len(idx), npar))
        J[off[-1] + np.arange(len(idx)), idx] = REG
        for k, s in enumerate(st):
            r, th = aus(p, V[k])
            wc = s["wn"][:, None]
            kc = kf[k][:, None]
            dr = kc * 2 * (bq._g_r(wc - th, r) + bq._g_r(wc + th, r)) * (r * (1 - r / bq.RMAX))
            dt = kc * 2 * (bq._g_u(wc + th, r) - bq._g_u(wc - th, r)) * (th * (1 - th / np.pi))
            sl = slice(off[k], off[k + 1])
            J[sl, :nk] = (dr[:, :, None] * V[k][None, None, :]).reshape(len(wc), -1)
            J[sl, nk:2 * nk] = (dt[:, :, None] * V[k][None, None, :]).reshape(len(wc), -1)
            D = p[2 * nk + k]
            if rein[0]:
                J[sl, 2 * nk + k] = kf[k] * s["wn"]
            else:
                J[sl, 2 * nk + k] = kf[k] * (glied_phase(g, D + 1e-5, s["wn"]) - glied_phase(g, D - 1e-5, s["wn"])) / 2e-5
        return J

    p = np.zeros(npar)
    p[:nk].reshape(M, P + 1)[:, 0] = np.log(r0 / (bq.RMAX - r0))
    p[nk:2 * nk].reshape(M, P + 1)[:, 0] = np.log(th0 / (np.pi - th0))
    for k, s in enumerate(st):
        p[2 * nk + k] = (s["rest_n"][0] - bq.bq_phase(r0, th0, s["wn"][:1])[0]) / s["wn"][0]
    p = opt.least_squares(res, p, jac=jac, method="trf", max_nfev=600, x_scale="jac").x
    rein[0] = False                    # zweite Stufe: das Glied mit seiner genauen Phase
    Nint = np.floor(p[2 * nk:] - lo)
    p[2 * nk:] -= Nint
    lb =np.full(npar, -np.inf)
    ub = np.full(npar, np.inf)
    lb[2 * nk:], ub[2 * nk:] = lo - 0.25, lo + 1.25
    runden = 0
    for runden in range(1, 13):
        p[2 * nk:] = np.clip(p[2 * nk:], lo - 0.24, lo + 1.24)
        p = opt.least_squares(res, p, jac=jac, bounds=(lb, ub), method="trf", max_nfev=400, x_scale="jac").x
        Dtot = Nint + p[2 * nk:]
        neu = np.floor(Dtot - lo)
        fertig = np.array_equal(neu, Nint)
        Nint = neu
        p[2 * nk:] = Dtot - Nint
        if fertig:
            break
    return (lambda x: aus(p, np.polynomial.legendre.legvander(np.array([2 * x - 1]), P)[0])), runden


# ------------------------------------------------------------------ Saiten je Instrument
def saiten(inst):
    if inst == "Gitarre":
        return [(f"Saite {6 - s}", ps.GITARRE[s], ps.MENSUR, 19) for s in range(6)]
    ch, mens, _ = ps.choere(inst.lower())
    out = []
    for c, (_, sn) in enumerate(ch):
        for j, sa in enumerate(sn):
            if j and abs(sa["hz"] - sn[0]["hz"]) < 1e-9 and sa.get("d") == sn[0].get("d"):
                continue
            out.append((f"Chor {c + 1}.{j + 1}", sa, mens, 12))
    return out


def zeile(st, f, g):
    cm, pm, aus_ = [], [], 0
    for s in st:
        v = werte(s, *f(s["x"]), g)
        if v is None:
            return None
        cm.append(v[0])
        pm.append(v[3])
        aus_ += v[1] != v[2]
    i = int(np.argmax(cm))
    return np.median(cm), max(cm), st[i]["u"], max(pm), aus_, len(st)


if __name__ == "__main__":
    inst = sys.argv[1]
    opts = dict(a.split("=") for a in sys.argv[2:])
    GL = [(x[0], int(x[1:])) for x in opts.get("glied", "T1,T3,L5").split(",")]
    GRAD = [int(x) for x in opts.get("grad", "2,3").split(",")]
    NUR = opts.get("saiten")
    REG = float(opts.get("reg", "0.05"))
    print(f"{inst}: Stuetzstellen an Buenden und auf halbem Weg, geprueft auch an sechs Stellen je Bund dazwischen; RMAX {bq.RMAX}, REG {REG}")
    for nr, (name, sa, mens, nb) in enumerate(saiten(inst)):
        if NUR and str(nr + 1) not in NUR.split(","):
            continue
        bund = [stelle(sa, mens, float(k), nb, None) for k in range(nb + 1)]
        halb = [stelle(sa, mens, k + 0.5, nb, bund) for k in range(nb)]
        viertel = [stelle(sa, mens, k + q, nb, bund) for k in range(nb) for q in (0.125, 0.25, 0.375, 0.625, 0.75, 0.875)]
        st = sorted(bund + halb, key=lambda s: s["u"])
        nap = [len(uu.as_biquads(t["rest"], min(t["fz"][-1] * 1.02, 19500.0))[0]) for t in bund]
        M = max(8, max(nap) + 2)
        print(f"\n### {inst} {name}: f1 {bund[0]['fn'][0]:.1f}-{bund[-1]['fn'][0]:.1f} Hz, zaehlend bis {bund[0]['fz'][-1]:.0f} Hz "
              f"({len(bund[0]['fz'])}-{len(bund[-1]['fz'])} Teiltoene), Abel-Smith {min(nap)}-{max(nap)}, M = {M}", flush=True)
        r0, th0 = bq.start(bund[0]["rest"], min(bund[0]["fz"][-1] * 1.02, 19500.0), M)
        for g in GL:
            for P in GRAD:
                t0 = time.perf_counter()
                f, runden = gebunden(st, M, P, r0, th0, g)
                e = [zeile(x, f, g) for x in (bund, halb, viertel)]
                kopf = f"  {g[0]}{g[1]} Grad {P} ({2 * M * (P + 1):3d} Zahlen, {runden} Runden, {time.perf_counter() - t0:4.0f} s):"
                if any(x is None for x in e):
                    print(kopf + " nicht auswertbar", flush=True)
                    continue
                txt = " | ".join(f"{was} Median {x[0]:5.2f}, groesster {x[1]:6.2f} (Lage {x[2]:5.2f})"
                                 for was, x in zip(("Buende", "halb", "dazwischen"), e))
                print(f"{kopf} {txt} | Abklingen groesster Fehler {max(x[3] for x in e):5.2f} %, ausserhalb "
                      f"{sum(x[4] for x in e)}/{sum(x[5] for x in e)}", flush=True)
