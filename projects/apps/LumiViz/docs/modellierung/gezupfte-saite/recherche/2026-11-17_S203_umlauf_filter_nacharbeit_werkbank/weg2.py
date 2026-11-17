"""LVR Auftrag 21, Punkt 2: Verlustfilter mit Nebenbedingung |H(e^{iw})| <= 1, gleiche Vorgabe und gleiches Gewicht wie
Punkt 1 (Bank & Valimaki vollstaendig, `bank_voll.py`).

Weg 2a (konvex): Nenner A aus dem Bank-Entwurf fest; Zaehler ueber R(w) = |B(e^{iw})|^2 = r0 + 2 sum r_n cos(n w)
  (linear in r). Ziel: sum_k w_k/(4 g_k^2) (R_k/|A_k|^2 - g_k^2)^2  (erste Ordnung von w_k (|H_k| - g_k)^2),
  Nebenbedingungen auf dichtem Raster: delta |A_j|^2 <= R_j <= (1 - eps) |A_j|^2.  Quadratisches Problem mit
  linearen Ungleichungen -> LSI -> LDP -> NNLS (Lawson & Hanson 1974, Kap. 23); danach Spektralfaktorisierung
  (Wurzeln von z^N R(z) im Einheitskreis) -> B mit Mindestphase.
Weg 2b (durch den Bau): H = g (A1(z) + A2(z)) / 2, A1, A2 stabile Allpaesse (Ordnung n1 + n2 = N), 0 < g <= 1
  -> |H| <= g <= 1 fuer alle w. Angepasst an sum_k w_k (|H_k| - g_k)^2 (Gl. 1 der Quelle, nicht linearisiert),
  scipy least_squares, 24 Starts je Aufteilung (n1, n2) mit festem Startwert des Zufallsgenerators.
Aufruf: python -B weg2.py tasten    -> weg2_tasten.txt   (18 Tasten, voller Umlauf wie Bericht 11 Tab. 2c)
        python -B weg2.py umfang    -> weg2_umfang.txt   (ganzer Umfang, Mass nur des Verlustfilters)
"""
import sys
sys.dont_write_bytecode = True
import pathlib
import time

import numpy as np
import scipy.optimize as opt

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import bank_voll as bv  # noqa: E402

ue, ps = bv.ue, bv.ue.ps
FS, F = bv.FS, bv.F
WG = np.linspace(0, np.pi, 4097)                   # Raster der Nebenbedingungen


# ------------------------------------------------------------------ Weg 2a
def lsi(E, f, G, h):
    """min ||E x - f|| mit G x >= h (Lawson & Hanson, LSI -> LDP -> NNLS)."""
    Q, R = np.linalg.qr(E)
    f1 = Q.T @ f
    Ri = np.linalg.inv(R)
    Gt = G @ Ri
    ht = h - Gt @ f1
    n = E.shape[1]
    Em = np.vstack([Gt.T, ht[None, :]])
    fv = np.zeros(n + 1)
    fv[-1] = 1.0
    u, _ = opt.nnls(Em, fv, maxiter=50 * Em.shape[1])
    rv = Em @ u - fv
    if np.linalg.norm(rv) < 1e-12:
        raise RuntimeError("Nebenbedingungen unvereinbar")
    z = -rv[:n] / rv[n]
    return Ri @ (z + f1)


def spektralfaktor(r):
    N = len(r) - 1
    c = np.concatenate([r[::-1], r[1:]])
    w = np.roots(c)
    w = w[np.argsort(np.abs(w))][:N]
    b = np.real(np.poly(w))
    j = np.argmax(np.abs(r[0] + 2 * np.cos(np.outer(WG, np.arange(1, N + 1))) @ r[1:]))
    Rj = r[0] + 2 * np.cos(WG[j] * np.arange(1, N + 1)) @ r[1:]
    Bj = np.abs(np.polyval(b[::-1], np.exp(-1j * WG[j])))
    return b * np.sqrt(Rj) / Bj


def weg2a(fk, g, Hd, N, eps=1e-7, delta=1e-9):
    _, a, _ = bv.wls(fk, g, Hd, N, "eq")
    x = 2 * np.pi * fk / FS
    n = np.arange(N + 1)
    C = np.where(n == 0, 1.0, 2.0) * np.cos(np.outer(x, n))
    Ak2 = np.abs(np.exp(-1j * np.outer(x, np.arange(N + 1))) @ a) ** 2
    w = 1 / (g ** 2 * np.log(g) ** 4)
    sw = np.sqrt(w / w.max()) / (2 * g)
    E = C / Ak2[:, None] * sw[:, None]
    f = g ** 2 * sw
    Cg = np.where(n == 0, 1.0, 2.0) * np.cos(np.outer(WG, n))
    Ag2 = np.abs(np.exp(-1j * np.outer(WG, np.arange(N + 1))) @ a) ** 2
    G = np.vstack([Cg, -Cg])
    h = np.concatenate([delta * Ag2, -(1 - eps) * Ag2])
    r = lsi(E, f, G, h)
    return spektralfaktor(r), a


# ------------------------------------------------------------------ Weg 2b
def sig(p):
    return 1 / (1 + np.exp(-p))


def allpass_d(p, n):
    """Nenner D(z) eines stabilen Allpasses der Ordnung n aus freien Parametern (2 je Biquad, 1 fuer den Rest)."""
    d = np.array([1.0])
    i = 0
    for _ in range(n // 2):
        r, th = 0.9995 * sig(p[i]), np.pi * sig(p[i + 1])
        d = np.convolve(d, [1.0, -2 * r * np.cos(th), r * r])
        i += 2
    if n % 2:
        d = np.convolve(d, [1.0, 0.9995 * np.tanh(p[i])])
        i += 1
    return d, i


def ap_sum_ba(p, n1, n2):
    d1, i = allpass_d(p, n1)
    d2, j = allpass_d(p[i:], n2)
    g = sig(p[i + j])
    b = 0.5 * g * (np.convolve(d1[::-1], d2) + np.convolve(d2[::-1], d1))
    return b, np.convolve(d1, d2)


def weg2b(fk, g, N, starts=24, samen=21):
    x = 2 * np.pi * fk / FS
    w = 1 / (g ** 2 * np.log(g) ** 4)
    sw = np.sqrt(w / w.max())
    Ez = np.exp(-1j * np.outer(x, np.arange(N + 1)))
    rng = np.random.default_rng(samen)
    best = None
    for n1 in sorted({max(N // 2 - 1, 0), N // 2}):
        n2 = N - n1
        npar = n1 // 2 * 2 + n1 % 2 + n2 // 2 * 2 + n2 % 2 + 1

        def res(p):
            b, a = ap_sum_ba(p, n1, n2)
            return sw * (np.abs((Ez[:, :len(b)] @ b) / (Ez[:, :len(a)] @ a)) - g)
        for s in range(starts):
            p0 = rng.normal(0, 2, npar)
            p0[-1] = np.log(g[0] / (1 - g[0] + 1e-9)) + 1
            r = opt.least_squares(res, p0, method="trf", max_nfev=400 * npar)
            if best is None or r.cost < best[0]:
                best = (r.cost, r.x, n1, n2)
    return ap_sum_ba(best[1], best[2], best[3]), (best[2], best[3])


# ------------------------------------------------------------------ gemeinsamer Rahmen
MODUS = {"weg": "1", "zeit": 0.0, "max": 0.0, "extra": None}


def entwerfe(Gl, fn, N, sig_=None):
    t0 = time.perf_counter()
    fk, g, Hd, gu, info = bv.vorgabe(Gl, fn, sig_, MODUS.get("o", bv.VOLL))
    if MODUS["weg"] == "1":
        b, a, _ = bv.wls(fk, g, Hd, N, "eq")
    elif MODUS["weg"] == "1sm":
        b, a, _ = bv.wls(fk, g, Hd, N, "sm")
    elif MODUS["weg"] == "2a":
        b, a = weg2a(fk, g, Hd, N)
    else:
        (b, a), MODUS["extra"] = weg2b(fk, g, N)
    MODUS["zeit"] = time.perf_counter() - t0
    MODUS["max"] = bv.maxbetrag(b, a)[0]
    return b, a


def iir_patch(Gl, fn, N, iters=None):
    b, a = entwerfe(Gl, fn, N, bv.ZUSTAND["sig"])
    iir_patch.vorher = MODUS["max"]
    if MODUS["max"] > 1 and MODUS["weg"].startswith("2"):
        b = b / MODUS["max"] * (1 - 1e-12)          # nur Rundung (2a: Raster der Nebenbedingung); wird gemeldet
    return b, a


iir_patch.vorher = 0.0


def verlust_mass(sa, L, N):
    """Fehler des Abklingens nur durch das Verlustfilter: p_n = 100 (ln|H(f_n)| / ln|H_ziel(f_n)| - 1)
    (Annahme: Dispersion und Verzug treffen die Phase des Ziels; dann ist tau_g gleich und sigma ~ -ln|H|)."""
    Gt, ph = ue.ziel(sa, L)
    fn, sg = ue.teiltoene(Gt, ph)
    b, a = entwerfe(Gt, fn, N, sg)
    m = fn < 0.45 * FS
    fn, sg = fn[m], sg[m]
    h = np.abs(ue.antwort_ba(b, a, 2 * np.pi * fn / FS))
    p = 100 * (np.log(h) / np.log(np.interp(fn, F, Gt)) - 1)
    hr = sg <= ue.SIG_REL
    return {"max": MODUS["max"], "zeit": MODUS["zeit"], "p1": p[0], "p12": np.abs(p[:12]).max(),
            "p5k": np.abs(p[fn <= 5000]).max(), "pr": np.abs(p[hr]).max() if hr.any() else np.nan,
            "in5r": int(np.sum(np.abs(p[hr]) <= 5)), "nr": int(hr.sum())}


def tasten_umfang():
    out = []
    for s in range(6):
        for bund in range(20):
            out.append(("Gitarre", f"S{6 - s} B{bund}", ps.GITARRE[s], ps.MENSUR * 2 ** (-bund / 12)))
    for t in range(25, 105):
        x = ps.harfe_taste(t)
        if x is not None:
            out.append(("Harfe", f"T{t}", x[0], x[1]))
    for t in range(29, 90):
        x = ps.cembalo_taste(t)
        out.append(("Cembalo", f"T{t}", x[0], x[1]))
    for name, kurz in (("laute", "Laute"), ("oud", "Oud")):
        ch, mens, _ = ps.choere(name)
        for c, (_, saiten) in enumerate(ch):
            for j, sa in enumerate(saiten):
                if j and abs(sa["hz"] - saiten[0]["hz"]) < 1e-9 and sa.get("d") == saiten[0].get("d"):
                    continue                     # gleiche Saite im Einklang: einmal
                for bund in (0, 5):
                    out.append((kurz, f"C{c + 1}.{j + 1} B{bund}", sa, mens * 2 ** (-bund / 12)))
    return out


def main_tasten():
    sys.stdout = bv.Tee("weg2_tasten.txt")
    ue.iir_bank = iir_patch
    print("Je Zeile: Weg, Ordnung | max|H| (Weg 2: vor der Rundungs-Skalierung) | Abklingen % Grundton, 1-12, <=5k, hoerbar "
          "(voller Umlauf, K16 5k, Thiran 1) | dieselben nur Verlustfilter | Entwurf ms")
    for titel, sa, L in ue.tasten():
        Gt, ph = ue.ziel(sa, L)
        fn, sg = ue.teiltoene(Gt, ph)
        K = int(np.sum(fn < 0.45 * FS))
        print(f"\n### {titel}: f1 {fn[0]:.2f} Hz, Sollpunkte {K}", flush=True)
        for weg in ("1", "2a", "2b"):
            for N in (2, 4, 8):
                if 2 * N + 1 > 2 * K:
                    print(f"  Weg {weg:3s} iir{N}: -- zu wenig Sollpunkte")
                    continue
                MODUS["weg"] = weg
                bv.ZUSTAND.update(sig=sg)
                Gt_, ph_, Gm, phm, info = ue.entwurf(sa, L, f"iir{N}", ("K", 16, 5000.0), ("thiran", 1))
                r = ue.bewerte(Gt_, ph_, Gm, phm)
                mx, zt, ex = MODUS["max"], MODUS["zeit"], MODUS["extra"]
                v = verlust_mass(sa, L, N)
                ap = f" (n1,n2)={ex}" if weg == "2b" else ""
                print(f"  Weg {weg:3s} iir{N} | {mx:.6f} | {r['p1']:+7.2f} {r['p12']:7.2f} {r['p5k']:8.2f} {r['pr']:8.2f} | "
                      f"{v['p1']:+7.2f} {v['p12']:7.2f} {v['p5k']:8.2f} {v['pr']:8.2f} | Cent 1-12 {r['c12']:6.2f} <=5k {r['c5k']:6.2f} "
                      f"hoerbar {r['cr']:6.2f} | {1000 * zt:7.1f}{ap}", flush=True)


def main_umfang():
    sys.stdout = bv.Tee("weg2_umfang.txt")
    print("Ganzer Umfang, Mass nur des Verlustfilters (verlust_mass). Je Taste und Ordnung: Weg 1 max|H|, Fehler; "
          "wenn Weg 1 > 1: Weg 2a und 2b.")
    print("Spalten: Instrument Taste | N | W1 max|H| | W1 % Grundton 1-12 <=5k hoerbar | W2a max|H| % ... | W2b % ... | ms 2a/2b")
    for inst, name, sa, L in tasten_umfang():
        Gt, ph = ue.ziel(sa, L)
        fn, sg = ue.teiltoene(Gt, ph)
        K = int(np.sum(fn < 0.45 * FS))
        for N in (2, 4, 8):
            if 2 * N + 1 > 2 * K:
                print(f"{inst:8s} {name:9s} | {N} | zu wenig Sollpunkte ({K})", flush=True)
                continue
            MODUS["weg"] = "1"
            MODUS["o"] = dict(bv.VOLL, glatt=False)
            v0 = verlust_mass(sa, L, N)
            MODUS["o"] = bv.VOLL
            v1 = verlust_mass(sa, L, N)
            z = (f"{inst:8s} {name:9s} | {N} | {v1['max']:.6f} | {v1['p1']:+7.2f} {v1['p12']:7.2f} {v1['p5k']:8.2f} {v1['pr']:8.2f}"
                 f" | oG {v0['max']:.6f} {v0['p1']:+7.2f} {v0['p12']:7.2f} {v0['p5k']:8.2f} {v0['pr']:8.2f}")
            if v1["max"] > 1 or v0["max"] > 1:
                MODUS["weg"] = "2a"
                try:
                    va = verlust_mass(sa, L, N)
                    z += f" | {va['max']:.6f} {va['p1']:+7.2f} {va['p12']:7.2f} {va['p5k']:8.2f} {va['pr']:8.2f}"
                    ta = va["zeit"]
                except Exception as e:  # noqa: BLE001 - Fehler wird als Ergebnis gemeldet
                    z += f" | 2a gescheitert: {e}"
                    ta = np.nan
                MODUS["weg"] = "2b"
                vb = verlust_mass(sa, L, N)
                z += f" | {vb['p1']:+7.2f} {vb['p12']:7.2f} {vb['p5k']:8.2f} {vb['pr']:8.2f} | {1000 * ta:.0f}/{1000 * vb['zeit']:.0f}"
            print(z, flush=True)


if __name__ == "__main__":
    {"tasten": main_tasten, "umfang": main_umfang}[sys.argv[1]]()
