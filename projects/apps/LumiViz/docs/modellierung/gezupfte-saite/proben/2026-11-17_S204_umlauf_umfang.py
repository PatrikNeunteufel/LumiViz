"""S204 - Entwurf des Umlauf-Filters: Kandidaten ueber den ganzen Umfang (295 Tasten), je Teilton gemessen.

Aufruf: python -X utf8 -B <dieses Skript> [Instrument ...]      -> Ausgabe auf die Konsole (in eine .txt umleiten)

Baut auf den Rechnungen der Research-Werkbank auf (Berichte 11 und 21; `umlauf_entwurf.py`, `bank_voll.py`, `weg2.py`
im Archiv der Werkbank - dieselben Dateien liegen neben den Berichten unter `recherche/`).

Umlauf des Modells: H_m = z^-N * Thiran1 * A_disp * H_loss, am starren Steg, gegen H_ziel = exp(-2 i k L) des Pruefstands.

Verlust (Vorgabe nach Bank & Valimaki 2003 OHNE Glaettung und OHNE Hochfrequenz-Vorgabe - die Ueberblendung der letzten
fuenf Sollpunkte in einen Ein-Pol aendert dort das Ziel selbst -, unter f1 der Wert bei f1 gehalten, Sollpunkte bis
21,6 kHz):
  W1   Gleichungsfehler wie invfreqz, Gewicht der Quelle 1/(g^2 ln^4 g)             (Bericht 21, "oG")
  A    Weg 2a (Nenner aus W1, Zaehler mit |H| <= 1), Gewicht der Quelle             (Bericht 21)
  R    wie A, aber Gewicht 1/(g ln g)^2 in Nenner und Zaehler
       Herleitung: Abklingzeit tau = -1/(f0 ln g)  ->  d ln tau = -dg / (g ln g). Die Quelle minimiert den Fehler von
       tau in Sekunden (Gewicht ~ 1/ln^4 g, lange Teiltoene zaehlen fast allein); die Hoerschwelle (Tolonen &
       Jarvelainen 2000) ist ein FAKTOR der Abklingzeit, also zaehlt der relative Fehler -> Gewicht 1/(g ln g)^2.
Mass des Verlusts allein: p_n = 100 (ln|H(f_n)| / ln|H_ziel(f_n)| - 1)  = relativer Fehler der Abklingrate.
Schwelle Abklingen (HW1 a): Faktor der Abklingzeit 0,7923 ... 1,4309  ->  p in [-30,1 ; +26,2] %.

Dispersion (an der Restphase nach dem Verlust):
  K16, K32   K gleiche Allpaesse 1. Ordnung, ein Wert je Taste, Band 5 kHz                    (S201, Bericht 11 D4)
  AS         Biquad-Allpaesse nach dem Verfahren von Abel & Smith / adf.m, aber an UNSERER Zielphase:
             psi(w) = Restphase - mu w,  mu so, dass psi(w_d) = 2 pi nap;  Polwinkel exakt bei psi = (2k-1) pi;
             Radius aus der Bandbreite wie adf.m (beta 0,85).  Band w_d = oberster zaehlender Teilton.
Zaehlende Teiltoene: "hoerbar" = sigma <= 100 1/s (Setzung aus Bericht 11), daneben 1-12 und alle bis 20 kHz.
Schreibt nichts.
"""
import sys
import pathlib
sys.dont_write_bytecode = True
import numpy as np

WB = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\archiv\ergebnisse\2026-11-17_21_umlauf_filter_nacharbeit")
sys.path.insert(0, str(WB))
import bank_voll as bv  # noqa: E402
import weg2  # noqa: E402

ue = bv.ue
FS, F, W = ue.FS, ue.F, ue.W
OG = dict(bv.VOLL, glatt=False, hf=False)      # ueber dem letzten Sollpunkt wird der Wert gehalten
P_LO, P_HI = 100 * (1 / 1.4309 - 1), 100 * (1 / 0.7923 - 1)      # -30,1 ; +26,2 %
CENT = 10.0


# ------------------------------------------------------------------ Verlust
def gewicht(g, art):
    w = 1 / (g ** 2 * np.log(g) ** 4) if art == "bv" else 1 / (g * np.log(g)) ** 2
    return np.sqrt(w / w.max())


def nenner(fk, g, Hd, N, art):
    sw = gewicht(g, art)
    x = 2 * np.pi * fk / FS
    E = np.exp(-1j * np.outer(x, np.arange(N + 1)))
    M = np.hstack([E, -Hd[:, None] * E[:, 1:]])
    Mr = np.vstack([(M * sw[:, None]).real, (M * sw[:, None]).imag])
    rr = np.concatenate([(Hd * sw).real, (Hd * sw).imag])
    p = np.linalg.lstsq(Mr, rr, rcond=None)[0]
    b, a = p[:N + 1], np.concatenate([[1.0], p[N + 1:]])
    r = np.roots(a)
    aus = np.abs(r) >= 1
    if np.any(aus):
        b = b / np.prod(np.abs(r[aus]))
        a = np.real(np.poly(np.where(aus, 1 / np.conj(r), r)))
    return b, a


def zaehler_passiv(fk, g, a, N, art, eps=1e-7, delta=1e-9):
    x = 2 * np.pi * fk / FS
    n = np.arange(N + 1)
    C = np.where(n == 0, 1.0, 2.0) * np.cos(np.outer(x, n))
    Ak2 = np.abs(np.exp(-1j * np.outer(x, n)) @ a) ** 2
    sw = gewicht(g, art) / (2 * g)
    E = C / Ak2[:, None] * sw[:, None]
    f = g ** 2 * sw
    Cg = np.where(n == 0, 1.0, 2.0) * np.cos(np.outer(weg2.WG, n))
    Ag2 = np.abs(np.exp(-1j * np.outer(weg2.WG, n)) @ a) ** 2
    r = weg2.lsi(E, f, np.vstack([Cg, -Cg]), np.concatenate([delta * Ag2, -(1 - eps) * Ag2]))
    return weg2.spektralfaktor(r)


def verlust(Gt, fn, sg, N, weg):
    """-> b, a, max|H| (vor der Rundungs-Skalierung bei A/R)"""
    fk, g, Hd, _, _ = bv.vorgabe(Gt, fn, sg, OG)
    art = "rel" if weg == "R" else "bv"
    b, a = nenner(fk, g, Hd, N, art)
    if weg != "W1":
        b = zaehler_passiv(fk, g, a, N, art)
    mx = bv.maxbetrag(b, a)[0]
    if weg != "W1" and mx > 1:
        b = b / mx * (1 - 1e-12)
    return b, a, mx


def p_verlust(b, a, Gt, fn):
    h = np.abs(ue.antwort_ba(b, a, 2 * np.pi * fn / FS))
    return 100 * (np.log(h) / np.log(np.interp(fn, F, Gt)) - 1)


# ------------------------------------------------------------------ Dispersion
def as_biquads(rest, f_d, beta=0.85):
    """Abel-Smith an der Restphase `rest` (auf F, positiv, waechst). -> alpha, cc"""
    w_d = 2 * np.pi * f_d / FS
    r_d = np.interp(f_d, F, rest)
    tau = np.gradient(rest, W)
    mu0 = np.interp(f_d, F, tau)
    nap = int(np.floor((r_d - mu0 * w_d) / (2 * np.pi)))
    if nap < 1:
        return np.array([]), np.array([])
    mu = (r_d - 2 * np.pi * nap) / w_d
    m = F <= f_d
    psi = np.maximum.accumulate(rest[m] - mu * W[m])
    theta = np.interp(np.pi * np.arange(1, 2 * nap, 2), psi, W[m])
    delta = np.diff(np.concatenate([[-theta[0]], theta])) / 2
    eta = (1 - beta * np.cos(delta)) / (1 - beta)
    return np.sqrt(eta ** 2 - 1) - eta, np.cos(theta)


def umlauf(Gt, ph, fn, b, a, disp, f_d):
    """-> |H_m|, Phase, Zahl der Multiplikationen der Dispersion, Bezeichnung"""
    hl = ue.antwort_ba(b, a)
    rest = ph - ue.phase_von(hl)
    f1, w1 = fn[0], 2 * np.pi * fn[0] / FS
    if disp == "AS":
        al, cc = as_biquads(rest, f_d)
        hd = ue.biquads(al, cc) if len(al) else np.ones_like(W, complex)
        mult, name = 2 * len(al), f"{len(al)}bq"
    else:
        K = int(disp[1:])
        while K > 0:
            a_ = ue.fit_ap(rest, fn, K, 5000.0)
            hd = ue.ap1(a_, K)
            if np.interp(f1, F, rest - ue.phase_von(hd)) / w1 >= 3:
                break
            K //= 2
        if K == 0:
            hd = np.ones_like(W, complex)
        mult, name = K, f"K{K}"
    rest2 = rest - ue.phase_von(hd)
    Dtot = np.interp(f1, F, rest2) / w1
    Nint = int(np.floor(Dtot - 0.5))
    soll = np.interp(f1, F, rest2) - w1 * Nint
    Dl = Dtot - Nint
    pf = lambda D: ue.phase_von(ue.antwort_ba(*ue.thiran(D, 1), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    hf = ue.antwort_ba(*ue.thiran(Dl, 1))
    return np.abs(hl * hd * hf), ue.phase_von(hl * hd * hf) + W * Nint, mult, name


def je_teilton(Gt, ph, Gm, phm):
    fe, se = ue.teiltoene(Gt, ph)
    if np.any(np.diff(phm) <= 0):
        return None
    fm, sm = ue.teiltoene(Gm, phm)
    n = min(len(fe), len(fm))
    return fe[:n], se[:n], 1200 * np.log2(fm[:n] / fe[:n]), 100 * (sm[:n] / se[:n] - 1)


def mx(v):
    return float(np.abs(v).max()) if len(v) else float("nan")


# ------------------------------------------------------------------ Lauf
if __name__ == "__main__":
    nur = sys.argv[1:]
    WEGE, ORD, DISP = ("W1", "A", "R"), (2, 4, 8), ("K16", "K32", "AS")
    sv = {}      # (inst, weg, N) -> Liste (max|H|, p1, p12, phoer, palle, anteil hoerbar in Schwelle)
    sd = {}      # (inst, disp) -> Liste (c12, choer, calle, in10 hoer, n hoer, mult, p hoer, in p hoer)
    print("Je Taste. Verlust allein: Weg N | max|H| | % Grundton, 1-12, hoerbar, alle | hoerbare in [-30,1;+26,2] %")
    print("Voller Umlauf (Verlust R, kleinste Ordnung aus 8/4/2 mit allen hoerbaren in der Schwelle, sonst 8): "
          "Disp | Mult | Cent 1-12, hoerbar, alle | hoerbare in 10 Cent | % hoerbar | hoerbare in der Schwelle")
    for inst, name, sa, L in weg2.tasten_umfang():
        if nur and inst not in nur:
            continue
        Gt, ph = ue.ziel(sa, L)
        fn, sg = ue.teiltoene(Gt, ph)
        m = fn < 0.45 * FS
        fk, sk = fn[m], sg[m]
        hoer = sk <= ue.SIG_REL
        B = np.pi ** 2 * sa["EI"] / (sa["T"] * L * L)
        print(f"\n### {inst} {name}: f1 {fn[0]:.2f} Hz, B {B:.3g}, Teiltoene {len(fk)}, hoerbar {int(hoer.sum())} (bis {fk[hoer][-1]:.0f} Hz)", flush=True)
        beste = None
        for weg in WEGE:
            for N in ORD:
                if 2 * N + 1 > 2 * len(fk):
                    continue
                try:
                    b, a, mxh = verlust(Gt, fn, sg, N, weg)
                except Exception as e:  # noqa: BLE001 - als Ergebnis gemeldet
                    print(f"  {weg:2s} {N} | gescheitert: {e}")
                    continue
                p = p_verlust(b, a, Gt, fk)
                drin = (p[hoer] >= P_LO) & (p[hoer] <= P_HI)
                z = (mxh, p[0], mx(p[:12]), mx(p[hoer]), mx(p), float(drin.mean()))
                sv.setdefault((inst, weg, N), []).append(z)
                print(f"  {weg:2s} {N} | {mxh:.6f} | {p[0]:+7.2f} {z[2]:7.2f} {z[3]:8.2f} {z[4]:9.2f} | {int(drin.sum()):3d}/{int(hoer.sum()):3d}")
                if weg == "R" and (beste is None or (drin.all() and N < beste[0]) or (not beste[3] and N > beste[0])):
                    beste = (N, b, a, bool(drin.all()))
        if beste is None:
            continue
        f_d = min(fk[hoer][-1] * 1.02, 19500.0)
        for d in DISP:
            Gm, phm, mult, dn = umlauf(Gt, ph, fn, beste[1], beste[2], d, f_d)
            r = je_teilton(Gt, ph, Gm, phm)
            if r is None:
                print(f"  {d:3s}: Phase nicht monoton")
                continue
            fe, se, c, p = r
            h = se <= ue.SIG_REL
            dc = np.abs(c[h]) <= CENT
            dp = (p[h] >= P_LO) & (p[h] <= P_HI)
            z = (mx(c[:12]), mx(c[h]), mx(c), int(dc.sum()), int(h.sum()), mult, mx(p[h]), int(dp.sum()))
            sd.setdefault((inst, d), []).append(z)
            print(f"  R{beste[0]} + {d:3s} ({dn:>5s}) | {mult:3d} | {z[0]:7.2f} {z[1]:8.2f} {z[2]:9.2f} | {z[3]:3d}/{z[4]:3d} | {z[6]:8.2f} | {z[7]:3d}/{z[4]:3d}", flush=True)

    print("\n\n===== Zusammenfassung Verlust allein (Median / Groesstwert ueber die Tasten) =====")
    print("Instrument Weg N | Tasten | ueber 1 | Grundton |%| | 1-12 % | hoerbar % | alle % | Tasten mit allen hoerbaren in der Schwelle")
    for (inst, weg, N), v in sv.items():
        v = np.array(v)
        print(f"{inst:8s} {weg:2s} {N} | {len(v):3d} | {int(np.sum(v[:, 0] > 1)):3d} | {np.median(np.abs(v[:, 1])):5.1f} / {np.abs(v[:, 1]).max():6.1f} | "
              f"{np.median(v[:, 2]):5.1f} / {v[:, 2].max():6.1f} | {np.median(v[:, 3]):5.1f} / {v[:, 3].max():7.1f} | "
              f"{np.median(v[:, 4]):6.1f} / {v[:, 4].max():8.1f} | {int(np.sum(v[:, 5] >= 1)):3d}")
    print("\n===== Zusammenfassung voller Umlauf =====")
    print("Instrument Disp | Tasten | Mult Median / max | Cent 1-12 Med / max | Cent hoerbar Med / max | Tasten: alle hoerbaren in 10 Cent | "
          "Tasten: alle hoerbaren im Abklingen in der Schwelle")
    for (inst, d), v in sd.items():
        v = np.array(v, float)
        print(f"{inst:8s} {d:3s} | {len(v):3d} | {np.median(v[:, 5]):4.0f} / {v[:, 5].max():4.0f} | {np.median(v[:, 0]):6.2f} / {v[:, 0].max():7.1f} | "
              f"{np.median(v[:, 1]):6.2f} / {v[:, 1].max():7.1f} | {int(np.sum(v[:, 3] == v[:, 4])):3d} | {int(np.sum(v[:, 7] == v[:, 4])):3d}")
