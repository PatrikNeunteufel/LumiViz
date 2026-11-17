"""LVR Auftrag 11, Punkt 1 und 2: Entwurf des Umlaufs je Taste und Fehler gegen H_ziel = exp(-2 i k L) des Pruefstands.

Aufruf: python -B umlauf_entwurf.py [kurz]
Liest nur (Pruefstand und Gegenprobe aus dem LumiViz-Repo werden importiert, ohne Bytecode zu schreiben).

Umlauf des Modells: H_m(z) = z^-N * H_fd(z) * A_disp(z) * H_loss(z)
Teiltoene: Frequenz, bei der die Phase des Umlaufs 2 pi n ist (starrer Steg).
Abklingen: sigma_n = -ln|H(f_n)| / tau_g(f_n), tau_g = d phi / d omega (Gruppenlaufzeit des Umlaufs).
"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np
import scipy.optimize as opt

PROBEN = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben")


def lade(datei, name):
    spec = importlib.util.spec_from_file_location(name, PROBEN / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


ps = lade("2026-11-15_S199_pruefstand_zupf.py", "ps")
gp = lade("2026-11-16_S201_dispersion_formeln_gegenprobe.py", "gp")
FS = 48000.0
F = np.arange(1.0, 20000.5, 0.5)
W = 2 * np.pi * F / FS                    # omega T


# ------------------------------------------------------------------ Ziel
def ziel(sa, L, f=F):
    k, _ = ps.saite_wellen(sa, 2 * np.pi * np.asarray(f, float) + 0j)
    return np.exp(2 * k.imag * L), 2 * k.real * L          # |E|, Phase (positiv, waechst)


def teiltoene(betrag, phase):
    n = np.arange(1, int(phase[-1] / (2 * np.pi)) + 1)
    fn = np.interp(2 * np.pi * n, phase, F)
    tau = np.gradient(phase, 2 * np.pi * F)              # s
    sig = -np.log(np.interp(fn, F, betrag)) / np.interp(fn, F, tau)
    return fn, sig


# ------------------------------------------------------------------ Bausteine (Antwort auf W, ohne ganzzahligen Verzug)
def pol1_2p(Gt, f1, f2):
    """Gl. (V2): H = b / (1 + a z^-1); |H| bei f1 und f2 getroffen."""
    g1, g2 = np.interp([f1, f2], F, Gt)
    c1, c2 = np.cos(2 * np.pi * f1 / FS), np.cos(2 * np.pi * f2 / FS)
    r = g1 ** 2 / g2 ** 2
    if abs(1 - r) < 1e-15:
        a = 0.0
    else:
        p = (c2 - r * c1) / (1 - r)
        a = -p + np.sign(p) * np.sqrt(max(p * p - 1, 0.0))     # Produkt der Wurzeln = 1: die innere
        if abs(a) >= 1:
            a = 1 / a
    b = g1 * np.sqrt(1 + 2 * a * c1 + a * a)
    return b, a


def pol1_passiv(Gt, f1, f2):
    """Gl. (V3): wie (V2); liegt der groesste Betrag (bei 0 bzw. fs/2) ueber 1, Gleichstrom-Betrag = 1 und f1 getroffen."""
    b, a = pol1_2p(Gt, f1, f2)
    if b / (1 - abs(a)) <= 1.0:
        return b, a, False
    g1 = np.interp(f1, F, Gt)
    c1 = np.cos(2 * np.pi * f1 / FS)
    # (1 + a)^2 = g1^2 (1 + 2 a c1 + a^2)
    A2, A1 = 1 - g1 * g1, 2 * (1 - g1 * g1 * c1)
    wur = np.roots([A2, A1, A2])
    a = float(np.real(wur[np.argmin(np.abs(wur))]))
    return 1 + a, a, True


def antwort_pol1(b, a, w=W):
    return b / (1 + a * np.exp(-1j * w))


def iir_bank(Gt, fn, N, iters=6):
    """Bank & Valimaki 2003: Betrag an den Teiltoenen, Mindestphase, Gewicht w_k = 1/(1-g_k)^4 (Gl. 2), Gleichungsfehler
    + Steiglitz-McBride-Iteration (lineare Loesungen, offline). -> (b, a)"""
    # Mindestphase aus dem Betrag (Cepstrum) auf einem gleichmaessigen Raster 0..fs/2
    nfft = 1 << 16
    fu = np.linspace(0, FS / 2, nfft // 2 + 1)
    gu = np.interp(np.maximum(fu, F[0]), F, Gt, right=Gt[-1])
    c = np.fft.irfft(np.log(gu), nfft)
    c[1:nfft // 2] *= 2
    c[nfft // 2 + 1:] = 0
    Hmin = np.exp(np.fft.rfft(c, nfft))
    fk = fn[fn < 0.45 * FS]
    Hd = np.interp(fk, fu, Hmin.real) + 1j * np.interp(fk, fu, Hmin.imag)
    gk = np.abs(Hd)
    wk = 1.0 / np.maximum(1 - gk, 1e-7) ** 2                # Wurzel aus w_k
    wk /= wk.max()
    x = 2 * np.pi * fk / FS
    E = np.exp(-1j * np.outer(x, np.arange(N + 1)))
    A_prev = np.ones(len(fk), complex)
    for _ in range(iters):
        M = np.hstack([E, -Hd[:, None] * E[:, 1:]])
        rhs = Hd
        s = wk / np.abs(A_prev)
        Mr = np.vstack([(M * s[:, None]).real, (M * s[:, None]).imag])
        rr = np.concatenate([(rhs * s).real, (rhs * s).imag])
        p = np.linalg.lstsq(Mr, rr, rcond=None)[0]
        b, a = p[:N + 1], np.concatenate([[1.0], p[N + 1:]])
        A_prev = E @ a
    # instabile Pole spiegeln (Betrag bleibt bis auf einen Faktor)
    r = np.roots(a)
    if np.any(np.abs(r) >= 1):
        r = np.where(np.abs(r) >= 1, 1 / np.conj(r), r)
        a = np.real(np.poly(r))
    # Passivitaet: groesster Betrag auf 0..fs/2 (dichtes Raster); ueber 1 -> b skaliert (Gl. V6)
    mx = float(np.abs(antwort_ba(b, a, np.linspace(0, np.pi, 16385))).max())
    iir_bank.vorher = mx
    if mx > 1.0:
        b = b / mx * (1 - 1e-9)
    return b, a


def antwort_ba(b, a, w=W):
    z = np.exp(-1j * np.outer(w, np.arange(max(len(b), len(a)))))
    return (z[:, :len(b)] @ b) / (z[:, :len(a)] @ a)


def ap1(a, K, w=W):
    e = np.exp(-1j * w)
    return ((a + e) / (1 + a * e)) ** K


def biquads(alpha, cc, w=W):
    e = np.exp(-1j * w)
    h = np.ones_like(e)
    for a_, c_ in zip(alpha, cc):
        h = h * (a_ * a_ + 2 * a_ * c_ * e + e * e) / (1 + 2 * a_ * c_ * e + a_ * a_ * e * e)
    return h


def thiran(D, N):
    """Thiran-Allpass N-ter Ordnung, Verzug D (JOS PASP): a_k = (-1)^k C(N,k) prod_n (D-N+n)/(D-N+k+n)."""
    from math import comb
    a = np.array([(-1) ** k * comb(N, k) * np.prod([(D - N + n) / (D - N + k + n) for n in range(N + 1)]) for k in range(N + 1)])
    return a[::-1].copy(), a


def lagrange(D, N):
    h = np.ones(N + 1)
    for k in range(N + 1):
        for m in range(N + 1):
            if m != k:
                h[k] *= (D - m) / (k - m)
    return h, np.array([1.0])


def phase_von(h):
    return -np.unwrap(np.angle(h))


# ------------------------------------------------------------------ Entwurf
def fit_ap(phase_rest, fn, K, band):
    """K gleiche Allpaesse, ein Wert a, an die Phase der Teiltoene bis `band` (hoechstens 40) angepasst (wie S201)."""
    x = 2 * np.pi * fn / FS
    ph = np.interp(fn, F, phase_rest)
    nfit = max(2, min(40, int(np.sum(fn <= band))))
    n = np.arange(1, nfit + 1)

    def fehler(a):
        pa = -K * np.angle((a + np.exp(-1j * x)) / (1 + a * np.exp(-1j * x)))
        d = (ph[0] - pa[0]) / x[0]
        return float(np.sum((1200 / np.log(2) * ((d * x + pa)[:nfit] - ph[:nfit]) / (2 * np.pi * n)) ** 2))
    r = np.linspace(-0.97, 0.0, 195)
    i = int(np.argmin([fehler(v) for v in r]))
    return float(opt.minimize_scalar(fehler, bounds=(r[max(i - 1, 0)], r[min(i + 1, 194)]), method="bounded").x)


def entwurf(sa, L, verlust="pol1p", disp=("K", 16, 5000.0), fd=("thiran", 1), nachstimmen=True):
    Gt, ph = ziel(sa, L)
    fn, _ = teiltoene(Gt, ph)
    f1 = fn[0]
    w1 = 2 * np.pi * f1 / FS
    info = {}
    hfd_betrag = np.ones_like(W)
    for durchgang in range(2):                    # 2. Durchgang: Betrag des Lagrange-Filters im Verlust ausgleichen
        Gl = np.minimum(Gt / hfd_betrag, 1.0)
        info["unmoeglich"] = int(np.sum(Gt[F < 20000] / hfd_betrag[F < 20000] > 1.0))
        if verlust == "keiner":
            hl = np.ones_like(W, complex)
        elif verlust in ("pol1", "pol1p"):
            f2 = min(8 * f1, 0.45 * FS)
            if verlust == "pol1":
                b, a = pol1_2p(Gl, f1, f2); info["geklemmt"] = False
            else:
                b, a, info["geklemmt"] = pol1_passiv(Gl, f1, f2)
            hl = antwort_pol1(b, a)
            info["koef"] = (b, a)
        else:
            N = int(verlust[3:])
            b, a = iir_bank(Gl, fn, N)
            hl = antwort_ba(b, a)
            info["koef"] = (b, a)
            info["geklemmt"] = iir_bank.vorher > 1.0
            info["vorher"] = iir_bank.vorher
        info["max_loss"] = float(np.abs(antwort_ba(*info["koef"], np.linspace(0, np.pi, 8193))).max()) if "koef" in info and verlust.startswith("iir") \
            else (float(np.abs(hl).max()) if verlust != "keiner" else 1.0)
        rest = ph - phase_von(hl)
        # Dispersion
        if disp is None:
            hd = np.ones_like(W, complex); info["K"] = 0
        elif disp[0] == "K":
            K = disp[1]
            while K > 0:
                a_ = fit_ap(rest, fn, K, disp[2])
                hd = ap1(a_, K)
                if (np.interp(f1, F, rest - phase_von(hd))) / w1 >= fd[1] + 2:
                    break
                K //= 2
            if K == 0:
                hd = np.ones_like(W, complex)
            info["K"], info["a_ap"] = K, (a_ if K else 0.0)
        elif disp[0] == "rauhala":
            B = np.pi ** 2 * sa["EI"] / (sa["T"] * L * L)
            a_ = gp.rauhala(f1, B, disp[1], FS)
            hd = ap1(a_, disp[1]); info["K"], info["a_ap"] = disp[1], a_
        elif disp[0] == "adf":
            B = np.pi ** 2 * sa["EI"] / (sa["T"] * L * L)
            al, cc = gp.adf(f1, B, disp[1], 0.85, FS)
            hd = biquads(al, cc); info["K"] = f"{len(al)}bq"
        rest2 = rest - phase_von(hd)
        # Stimmung: Gesamtverzug D_tot bei f1
        Dtot = np.interp(f1, F, rest2) / w1
        art, Nf = fd
        Nint = int(np.floor(Dtot - (Nf - 0.5))) if art == "thiran" else int(np.floor(Dtot - (Nf - 1) / 2))
        Delta = Dtot - Nint
        mk = thiran if art == "thiran" else lagrange
        if nachstimmen:                           # Newton: Phasenverzug des Bruchteil-Filters bei f1 = Delta
            soll = np.interp(f1, F, rest2) - w1 * Nint
            Dl = Delta
            for _ in range(8):
                pf = lambda D: phase_von(antwort_ba(*mk(D, Nf), np.array([1e-6, w1])))[1]
                g0 = pf(Dl) - soll
                Dl -= g0 / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
            Delta = Dl
        hf = antwort_ba(*mk(Delta, Nf))
        info["Delta"], info["Nint"] = Delta, Nint
        if art == "lagrange" and durchgang == 0:
            hfd_betrag = np.abs(hf)
            continue
        break
    Hm = hl * hd * hf * np.exp(-1j * W * Nint)
    return Gt, ph, np.abs(Hm), phase_von(hl * hd * hf) + W * Nint, info


def bewerte(Gt, ph, Gm, phm, fmax=20000.0):
    fe, se = teiltoene(Gt, ph)
    fm, sm = teiltoene(Gm, phm)
    n = min(len(fe), len(fm))
    fe, se, fm, sm = fe[:n], se[:n], fm[:n], sm[:n]
    c = 1200 * np.log2(fm / fe)
    p = 100 * (sm / se - 1)
    m = fe <= fmax
    b = F <= fmax
    return {"n": int(m.sum()), "c1": c[0], "p1": p[0],
            "c12": np.abs(c[:12][m[:12]]).max(), "p12": np.abs(p[:12][m[:12]]).max(),
            "c5k": np.abs(c[fe <= 5000]).max(), "p5k": np.abs(p[fe <= 5000]).max(),
            "c20": np.abs(c[m]).max(), "p20": np.abs(p[m]).max(),
            "in10": int(np.sum(np.abs(c[m]) <= 10)), "in5": int(np.sum(np.abs(p[m]) <= 5)),
            "dphi5": np.abs(phm - ph)[F <= 5000].max(), "dphi20": np.abs(phm - ph)[b].max(),
            "dln20": np.abs(np.log(Gm) - np.log(Gt))[b].max(),
            # hoerbar lang klingende Teiltoene: sigma <= 100 1/s (T60 >= 69 ms)
            "nr": int(np.sum(se <= SIG_REL)), "cr": np.abs(c[se <= SIG_REL]).max(), "pr": np.abs(p[se <= SIG_REL]).max(),
            "in10r": int(np.sum(np.abs(c[se <= SIG_REL]) <= 10)), "in5r": int(np.sum(np.abs(p[se <= SIG_REL]) <= 5))}


SIG_REL = 100.0


# ------------------------------------------------------------------ Tasten
def tasten():
    out = []
    for s, bund in ((0, 0), (3, 0), (3, 2), (3, 12), (5, 12)):
        sa = ps.GITARRE[s]
        out.append((f"Gitarre Saite {6 - s} Bund {bund}", sa, ps.MENSUR * 2 ** (-bund / 12)))
    for t in (27, 46, 60, 96, 103):
        sa, L, *_ = ps.harfe_taste(t)
        art = "Metall" if "etaF" in sa else ("Nylon" if sa["rho"] < 1100 else "Darm")
        out.append((f"Harfe Taste {t} ({art})", sa, L))
    for t in (29, 36, 60, 84):
        sa, L, *_ = ps.cembalo_taste(t)
        out.append((f"Cembalo Taste {t}", sa, L))
    ch, mens, _ = ps.choere("laute")
    out.append(("Laute Chor 1 (F) leer", ch[0][1][0], mens))
    out.append(("Laute Chor 7 (N) Bund 5", ch[6][1][0], mens * 2 ** (-5 / 12)))
    ch, mens, _ = ps.choere("oud")
    out.append(("Oud Chor 1 (umsp.) leer", ch[0][1][0], mens))
    out.append(("Oud Chor 5 (N) Bund 5", ch[4][1][0], mens * 2 ** (-5 / 12)))
    return out


DISP = [("ohne", None), ("K8 5k", ("K", 8, 5000.0)), ("K16 5k", ("K", 16, 5000.0)), ("K16 10k", ("K", 16, 10000.0)),
        ("K32 5k", ("K", 32, 5000.0)), ("Rauhala M8", ("rauhala", 8)), ("adf 5k", ("adf", 5000.0))]
VERL = ["pol1", "pol1p", "iir2", "iir4", "iir8"]
FD = [("thiran", 1), ("thiran", 3), ("lagrange", 1), ("lagrange", 3), ("lagrange", 5)]

if __name__ == "__main__":
    kurz = "kurz" in sys.argv
    for titel, sa, L in tasten():
        B = np.pi ** 2 * sa["EI"] / (sa["T"] * L * L)
        Gt, ph = ziel(sa, L)
        fn, se = teiltoene(Gt, ph)
        print(f"\n### {titel}: f1 {fn[0]:.2f} Hz, L {L:.3f} m, T {sa['T']:.1f} N, EI {sa['EI']:.3g} N m2, B {B:.3g}, "
              f"Umlauf {ph[np.searchsorted(F, fn[0])] / (2 * np.pi * fn[0] / FS):.1f} Takte, Teiltoene bis 20 kHz {np.sum(fn <= 20000)}, "
              f"sigma_1 {se[0]:.3f} 1/s, sigma(5 kHz) {np.interp(5000, fn, se):.2f} 1/s")
        print("  A Dispersion (Verlust pol1p, Thiran 1, nachgestimmt) | K | Cent: Grundton, 1-12, <=5k, <=20k | in +-10 Cent | dphi rad <=5k / <=20k")
        for name, d in DISP:
            Gt_, ph_, Gm, phm, info = entwurf(sa, L, "pol1p", d, ("thiran", 1))
            r = bewerte(Gt_, ph_, Gm, phm)
            print(f"    {name:11s} | {str(info['K']):>4s} | {r['c1']:+6.2f} {r['c12']:7.1f} {r['c5k']:7.1f} {r['c20']:8.1f} | {r['in10']:3d}/{r['n']:3d} | {r['dphi5']:6.2f} / {r['dphi20']:6.2f}")
        print("  B Verlust (Dispersion K16 5k, Thiran 1) | max|H_loss| | Abklingen %: Grundton, 1-12, <=5k, <=20k | in 5 % | max |d ln|H|| <=20k")
        for v in VERL:
            Gt_, ph_, Gm, phm, info = entwurf(sa, L, v, ("K", 16, 5000.0), ("thiran", 1))
            r = bewerte(Gt_, ph_, Gm, phm)
            kl = " geklemmt" if info.get("geklemmt") else ""
            print(f"    {v:6s} | {info['max_loss']:.6f}{kl:9s} | {r['p1']:+7.1f} {r['p12']:7.1f} {r['p5k']:7.1f} {r['p20']:8.1f} | {r['in5']:3d}/{r['n']:3d} | {r['dln20']:.2e}")
        if kurz:
            continue
        print("  C Bruchteil-Verzug (Verlust iir4 mit Ausgleich des Betrags, K16 5k) | Grundton Cent ohne / mit Nachstimmen | Cent <=20k | Abklingen % <=5k / <=20k | Frequenzen, an denen der Verlust > 1 sein muesste")
        for fd in FD:
            o = entwurf(sa, L, "iir4", ("K", 16, 5000.0), fd, nachstimmen=False)
            r0 = bewerte(*o[:4])
            Gt_, ph_, Gm, phm, info = entwurf(sa, L, "iir4", ("K", 16, 5000.0), fd)
            r = bewerte(Gt_, ph_, Gm, phm)
            print(f"    {fd[0]:8s} {fd[1]} | {r0['c1']:+7.3f} / {r['c1']:+7.3f} | {r['c20']:8.1f} | {r['p5k']:7.1f} / {r['p20']:8.1f} | {info['unmoeglich']}")
