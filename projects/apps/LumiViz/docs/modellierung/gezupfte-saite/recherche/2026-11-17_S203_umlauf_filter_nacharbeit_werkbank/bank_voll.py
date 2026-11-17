"""LVR Auftrag 21, Punkt 1: Verlustfilter nach Bank & Valimaki 2003 (IEEE SPL 10(1), S. 18-19) vollstaendig.

Baut auf `umlauf_entwurf.py` aus Bericht 11 auf (Archiv, per importlib); ersetzt nur `iir_bank`.
Schritte nach der Quelle (S. 19, Abschnitte IV-VI, am Seitenbild gelesen):
  1. Abklingzeiten tau_k mit [0,25 0,5 0,25] falten (Glaettung)            (VI)
  2. g_k = exp(-1/(f0 tau_k)) an den Teiltoenen                              (III)
  3. Ein-Pol-Entwurf an die Vorgabe; dessen Betrag ueber dem letzten Sollpunkt;
     die letzten fuenf Sollpunkte linear in den Ein-Pol uebergeblendet        (V, VI)
  4. Betrag auf dichtem linearem Raster, kubisch interpoliert                (VI)
  5. Phase ueber Hilbert-Transformation (Mindestphase), an den Sollfrequenzen
     wieder abgetastet                                                       (V, VI)
  6. Gewicht w_k = 1/(g_k^2 (ln g_k)^4)  (Gl. 2)                             (IV)
  7. gewichtete kleinste Quadrate wie invfreqz: Gleichungsfehler (invfreqz ohne `iter`);
     wahlweise mit Steiglitz-McBride-Schritten (dann "SM")                   (VI)
Aufruf: python -B bank_voll.py            -> bank_voll.txt (18 Tasten, Ordnungen 2/4/8/16, Varianten)
"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib
import time

import numpy as np
import scipy.optimize as opt
from scipy.interpolate import PchipInterpolator, CubicSpline

ARCHIV = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\archiv\ergebnisse\2026-11-16_11_umlauf_filter_grundlagen")
_spec = importlib.util.spec_from_file_location("ue", ARCHIV / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(ue)

FS, F, W = ue.FS, ue.F, ue.W
NFFT = 1 << 16
FU = np.linspace(0, FS / 2, NFFT // 2 + 1)        # dichtes lineares Raster 0..fs/2 (0,73 Hz)
WD = np.linspace(0, np.pi, 1 << 16 | 1)            # Pruefraster fuer max |H|

# Voreinstellung des vollstaendigen Verfahrens; Varianten setzen einzelne Schalter um
VOLL = dict(glatt=True, hf=True, interp="pchip", loeser="eq", fmax=0.45 * FS, soll="alle", ueberblend=5)


def maxbetrag(b, a):
    """Groesster Betrag auf 0..pi: dichtes Raster, dann an der Spitze nachgesucht. -> (max, f_max in Hz)"""
    h = np.abs(ue.antwort_ba(b, a, WD))
    i = int(np.argmax(h))
    lo, hi = WD[max(i - 1, 0)], WD[min(i + 1, len(WD) - 1)]
    r = opt.minimize_scalar(lambda w: -abs(ue.antwort_ba(b, a, np.array([w]))[0]), bounds=(lo, hi), method="bounded",
                            options={"xatol": 1e-12})
    m = max(h[i], -r.fun)
    return float(m), float((r.x if -r.fun >= h[i] else WD[i]) * FS / (2 * np.pi))


def einpol(fk, gk):
    """Ein-Pol H = g (1 + a) / (1 + a z^-1) (Valimaki u. a. 1996, Form wie Valimaki u. a. 2004 Gl. 2 mit r = 0),
    angepasst an |H(f_k)| = g_k mit Gewicht 1/(1 - g_k) (das 'ad hoc'-Gewicht aus [3], Bank & Valimaki S. 19).
    0 < g <= 1, -1 < a <= 0 -> |H| <= g <= 1."""
    x = 2 * np.pi * fk / FS
    sw = 1 / np.sqrt(np.maximum(1 - gk, 1e-12))

    def res(p):
        g, a = p
        return sw * (g * (1 + a) / np.abs(1 + a * np.exp(-1j * x)) - gk)
    best = None
    for a0 in (-0.05, -0.3, -0.6):
        r = opt.least_squares(res, [min(gk[0] * 1.0005, 1.0), a0], bounds=([0.0, -0.9999], [1.0, 0.0]))
        if best is None or r.cost < best.cost:
            best = r
    return best.x


def einpol_betrag(p, f):
    g, a = p
    return g * (1 + a) / np.abs(1 + a * np.exp(-2j * np.pi * np.asarray(f) / FS))


def vorgabe(Gl, fn, sig=None, o=VOLL):
    """Schritte 1-5. -> fk, gk (Sollbetrag), Hd (komplex, Mindestphase), Betrag auf FU, info"""
    m = fn < o["fmax"]
    if o["soll"] == "hoerbar" and sig is not None:
        m &= sig <= ue.SIG_REL
    fk = fn[m]
    f0 = fn[0]
    g = np.minimum(np.interp(fk, F, Gl), 1 - 1e-12)
    info = {"K": len(fk)}
    tau = -1 / (f0 * np.log(g))
    if o["glatt"] and len(tau) >= 3:
        tp = np.pad(tau, 1, mode="edge")                    # Rand: Wert wiederholt (Quelle sagt nichts)
        ts = 0.25 * tp[:-2] + 0.5 * tp[1:-1] + 0.25 * tp[2:]
        if o.get("rand") == "ungeglaettet":
            ts[0], ts[-1] = tau[0], tau[-1]
        tau = ts
    g = np.exp(-1 / (f0 * tau))
    if o["hf"]:
        p = einpol(fk, g)
        info["einpol"] = p
        n5 = min(o["ueberblend"], len(fk))
        lam = np.zeros(len(fk))
        lam[len(fk) - n5:] = np.arange(1, n5 + 1) / n5     # 1/5 ... 5/5: der letzte Sollpunkt ganz Ein-Pol
        g = (1 - lam) * g + lam * einpol_betrag(p, fk)
    # Betrag auf dem dichten Raster: unter f_1 der Wert bei f_1 gehalten; zwischen den Sollpunkten kubisch;
    # ueber dem letzten Sollpunkt der Ein-Pol (mit hf) bzw. der letzte Wert gehalten (ohne hf)
    ip = PchipInterpolator(fk, g) if o["interp"] == "pchip" else CubicSpline(fk, g)
    gu = np.empty_like(FU)
    lo, hi = FU < fk[0], FU > fk[-1]
    mid = ~(lo | hi)
    gu[lo] = g[0] if o.get("unter", "halten") == "halten" else np.interp(FU[lo], F, Gl)
    gu[mid] = ip(FU[mid])
    gu[hi] = einpol_betrag(info["einpol"], FU[hi]) if o["hf"] else g[-1]
    gu = np.clip(gu, 1e-6, None)
    # Mindestphase (Hilbert-Transformation des ln-Betrags ueber das reelle Cepstrum)
    c = np.fft.irfft(np.log(gu), NFFT)
    c[1:NFFT // 2] *= 2
    c[NFFT // 2 + 1:] = 0
    ph = np.unwrap(np.angle(np.exp(np.fft.rfft(c, NFFT))))
    Hd = g * np.exp(1j * np.interp(fk, FU, ph))
    info["max_vorgabe"] = float(gu.max())
    return fk, g, Hd, gu, info


def wls(fk, g, Hd, N, loeser="eq", iters=10):
    """Gewichtete kleinste Quadrate im Gleichungsfehler: min sum w_k |B_k - Hd_k A_k|^2 (wie invfreqz ohne iter);
    loeser 'sm': danach Steiglitz-McBride, Gewicht w_k/|A_alt,k|^2."""
    w = 1 / (g ** 2 * np.log(g) ** 4)                         # Gl. (2), linke Seite
    sw = np.sqrt(w / w.max())
    x = 2 * np.pi * fk / FS
    E = np.exp(-1j * np.outer(x, np.arange(N + 1)))
    M = np.hstack([E, -Hd[:, None] * E[:, 1:]])
    A_prev = np.ones(len(fk))
    for _ in range(iters if loeser == "sm" else 1):
        s = sw / A_prev
        Mr = np.vstack([(M * s[:, None]).real, (M * s[:, None]).imag])
        rr = np.concatenate([(Hd * s).real, (Hd * s).imag])
        p = np.linalg.lstsq(Mr, rr, rcond=None)[0]
        b, a = p[:N + 1], np.concatenate([[1.0], p[N + 1:]])
        A_prev = np.abs(E @ a)
    # instabile Pole spiegeln; der Betrag bleibt erhalten (b mit 1/|r| je gespiegeltem Pol)
    r = np.roots(a)
    aus = np.abs(r) >= 1
    if np.any(aus):
        b = b / np.prod(np.abs(r[aus]))
        r = np.where(aus, 1 / np.conj(r), r)
        a = np.real(np.poly(r))
    return b, a, int(aus.sum())


ZUSTAND = {"o": VOLL, "sig": None, "skalieren": False, "letzt": {}}


def iir_bank_voll(Gl, fn, N, iters=None):
    """Ersatz fuer ue.iir_bank: gleiche Schnittstelle (Gl auf F, fn, N) -> (b, a)."""
    o = ZUSTAND["o"]
    t0 = time.perf_counter()
    fk, g, Hd, gu, info = vorgabe(Gl, fn, ZUSTAND["sig"], o)
    b, a, ngesp = wls(fk, g, Hd, N, o["loeser"])
    mx, fmx = maxbetrag(b, a)
    info.update(vorher=mx, f_max=fmx, gespiegelt=ngesp, zeit=time.perf_counter() - t0)
    ZUSTAND["letzt"] = info
    iir_bank_voll.vorher = mx
    if ZUSTAND["skalieren"] and mx > 1:
        b = b / mx * (1 - 1e-9)
    return b, a


iir_bank_voll.vorher = 0.0
ue.iir_bank = iir_bank_voll


def lauf(sa, L, ordnung, o=VOLL, skalieren=False):
    """Voller Entwurf des Umlaufs wie Bericht 11, Tabelle 2c (Dispersion K16 5 kHz, Thiran 1, nachgestimmt)."""
    Gt, ph = ue.ziel(sa, L)
    fn, sig = ue.teiltoene(Gt, ph)
    ZUSTAND.update(o=o, sig=sig, skalieren=skalieren)
    Gt_, ph_, Gm, phm, info = ue.entwurf(sa, L, f"iir{ordnung}", ("K", 16, 5000.0), ("thiran", 1))
    r = ue.bewerte(Gt_, ph_, Gm, phm)
    return r, dict(ZUSTAND["letzt"]), info


def zielform(sa, L):
    """Form von -ln|H_ziel| ueber f: Werte bei 0+, f1, 2 f1 und Kruemmung; Anteil konstant/linear (Z3)."""
    Gt, ph = ue.ziel(sa, L)
    fn, sig = ue.teiltoene(Gt, ph)
    y = -np.log(Gt)
    return {"f1": fn[0], "y0": y[0], "y1": np.interp(fn[0], F, y), "y2": np.interp(2 * fn[0], F, y),
            "n": int(np.sum(fn < 0.45 * FS)), "nr": int(np.sum(sig <= ue.SIG_REL))}


VARIANTEN = [
    ("voll", VOLL),
    ("voll SM", dict(VOLL, loeser="sm")),
    ("ohne Glaettung", dict(VOLL, glatt=False)),
    ("ohne HF-Vorgabe", dict(VOLL, hf=False)),
    ("ohne beides", dict(VOLL, glatt=False, hf=False)),
    ("Spline statt pchip", dict(VOLL, interp="spline")),
    ("Soll nur hoerbar", dict(VOLL, soll="hoerbar")),
]

class Tee:
    """Ausgabe auf die Konsole und in eine Datei neben dem Skript."""
    def __init__(self, name):
        self.f = open(pathlib.Path(__file__).with_name(name), "w", encoding="utf-8")
        self.o = sys.stdout

    def write(self, s):
        self.f.write(s)
        self.o.write(s)

    def flush(self):
        self.f.flush()
        self.o.flush()


if __name__ == "__main__":
    nur = sys.argv[1:]
    if not nur:
        sys.stdout = Tee("bank_voll.txt")
    print("Bank & Valimaki 2003 vollstaendig. Je Zeile: Variante, Ordnung | max|H| vor Skalierung (bei f Hz) | gespiegelte Pole |"
          " Abklingen % Grundton, 1-12, <=5k, hoerbar (unskaliert) | 1-12 / hoerbar nach Skalierung (nur wenn max|H|>1) | ms")
    for titel, sa, L in ue.tasten():
        if nur and not any(s in titel for s in nur):
            continue
        z = zielform(sa, L)
        print(f"\n### {titel}: f1 {z['f1']:.2f} Hz, Sollpunkte (< 0,45 fs) {z['n']}, hoerbar {z['nr']}; "
              f"-ln|H_ziel|: f->0 {z['y0']:.3e}, f1 {z['y1']:.3e}, 2f1 {z['y2']:.3e}", flush=True)
        for vname, o in VARIANTEN:
            for N in ((2, 4, 8, 16) if vname.startswith("voll") else (2, 4, 8)):
                if 2 * N + 1 > 2 * (z["nr"] if o["soll"] == "hoerbar" else z["n"]):
                    print(f"  {vname:19s} iir{N:<2d} | -- mehr Unbekannte ({2 * N + 1}) als reelle Gleichungen; nicht gerechnet")
                    continue
                r, inf, _ = lauf(sa, L, N, o)
                zus = ""
                if inf["vorher"] > 1:
                    rs, _, _ = lauf(sa, L, N, o, skalieren=True)
                    zus = f"{rs['p12']:9.1f} {rs['pr']:9.1f}"
                print(f"  {vname:19s} iir{N:<2d} | {inf['vorher']:.6f} ({inf['f_max']:7.0f}) | {inf['gespiegelt']} | "
                      f"{r['p1']:+7.2f} {r['p12']:7.2f} {r['p5k']:8.2f} {r['pr']:8.2f} | {zus:19s} | {1000 * inf['zeit']:6.1f}", flush=True)
