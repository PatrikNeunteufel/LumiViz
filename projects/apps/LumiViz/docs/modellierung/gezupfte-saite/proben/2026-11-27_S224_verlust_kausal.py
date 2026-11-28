"""S224, VL6 a — der Verlust der Saite kausal, mit neu angepasster Biegesteife, an allen fuenf Instrumenten.

Stand vor VL7 a (S224): seit VL7 a nehmen `quelle` und `stegkraft` des Bezugs die kausale Wellenzahl (`wellen_kausal`);
die Zahlen dieser Probe stehen in der .txt daneben, ein neuer Lauf rechnet mit dem kausalen Verlust.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>

Quellen (gelesen S224): Lynch-Aird & Woodhouse 2017 (Materials 10, 497) S. 4, Gl. 17, 18: E_B aus der Biegesteife, die
aus den gemessenen Obertoenen kommt (f_n / n ~ a + c n^2, quadratisch angepasst); Woodhouse & Lynch-Aird 2019 (Acta
Acust. 105) S. 517: „The value of E relevant to the equations stated above is the high-frequency value called EB“,
S. 518: E komplex, E(1 + i eta_E). Die gemessenen Obertoene tragen also schon jede Dispersion der echten Saite; die
Dispersion nach Kramers-Kronig zum heutigen eta einfach dazu zaehlte sie doppelt (VL6 b).

Kausal UND mit denselben Obertoenen: e^(-i k x) = H_min(w) e^(-i k_B'(w) x) — H_min der minimalphasige Faktor zum
Verlust (|H| = e^(-alpha x), alpha = -Im k des heutigen Modells, Phase aus dem Cepstrum), k_B' die verlustfreie
Wellenzahl der steifen Saite mit neu angepasstem Zug T' und Biegesteife EI', so dass die Teiltoene 2 L Re k(f_n) = 2 pi n
des neuen Modells die heutigen treffen (kleinste Quadrate in Cent, Teiltoene bis 20 kHz, hoechstens 30).

Je Saite: (1) der Rest der Lagen nach der Anpassung (Cent, groesster Betrag) und die Aenderung von T und EI; (2) die
Verschiebung ohne Anpassung (wie `2026-11-27_S224_vorlaeufer_kausal.py`); (3) der Vorlaeufer der Quelle vor -2 / -20 ms
heute und kausal (Spitze / Energie dB, auf 16 s mit s0 0,5 wie `ursache` „Fenster 16 s“ — auf 4 s trug der Auslaeufer tiefer
Saiten einen Boden um -70 dB; die Dispersion an Re w genommen, die Verschiebung auf die Gerade w - i s0 ist
gegen die Lagen vernachlaessigt).
"""
import importlib.util
import pathlib

import numpy as np
import scipy.optimize as opt

P = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ursache", P / "2026-11-27_S224_vorlaeufer_ursache.py")
ur = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ur)
bz, ps = ur.bz, ur.ps
WELLEN = ps.saite_wellen
NFEIN = 1 << 21
FMIN = 1.0


def real_raster():
    f = np.fft.rfftfreq(NFEIN, 1 / ps.FS)
    return f, 2 * np.pi * np.maximum(f, FMIN)


def dispersion(sa):
    """Delta k(w) auf dem feinen Raster: Re k, die zum Verlust alpha(w) kausal gehoert (x = 1 m)."""
    f, w = real_raster()
    k, _ = WELLEN(sa, w.astype(complex))
    ln_betrag = k.imag                       # Im k = -alpha < 0: |H| = e^(-alpha x) = e^(Im k x)
    c = np.fft.irfft(ln_betrag, NFEIN)
    c[1:NFEIN // 2] *= 2
    c[NFEIN // 2 + 1:] = 0.0
    phase = np.fft.rfft(c).imag
    return f, -phase                         # e^(-i dk x) = e^(i phase)


def re_k_steif(T, m, EI, w):
    return np.sqrt((-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI))


def teiltoene(re_k, f, L, nmax=30):
    aus = []
    for n in range(1, nmax + 1):
        ziel = 2 * np.pi * n
        i = int(np.searchsorted(2 * L * re_k, ziel))
        if i <= 0 or i >= len(f) - 1 or f[i] > 20000:
            break
        aus.append(f[i - 1] + (ziel - 2 * L * re_k[i - 1]) / (2 * L * (re_k[i] - re_k[i - 1])) * (f[i] - f[i - 1]))
    return np.array(aus)


def anpassen(sa, L):
    f, w = real_raster()
    alt = teiltoene(WELLEN(sa, w.astype(complex))[0].real, f, L)
    _, dk = dispersion(sa)
    ohne = teiltoene(re_k_steif(sa["T"], sa["m"], sa["EI"], w) + dk, f, L)[:len(alt)]

    def rest(p):
        neu = teiltoene(re_k_steif(sa["T"] * p[0], sa["m"], sa["EI"] * p[1], w) + dk, f, L)
        k = min(len(neu), len(alt))
        r = 1200 * np.log2(neu[:k] / alt[:k])
        return np.concatenate([r, np.zeros(len(alt) - k) + 50.0])

    los = opt.least_squares(rest, [1.0, 1.0], x_scale=[1e-3, 1e-2], diff_step=[1e-7, 1e-5])
    r = rest(los.x)
    ohne_c = 1200 * np.log2(ohne / alt[:len(ohne)])
    return los.x, r, ohne_c - ohne_c[0], len(alt)


def kausal_wellen(sa, T2, EI2):
    f, wf = real_raster()
    _, dk = dispersion(sa)

    def wellen(saite, w):
        k_alt, _ = WELLEN(saite, w)
        wr = np.abs(w.real)
        d = np.interp(wr, 2 * np.pi * f, dk)
        kre = np.sqrt((-T2 + np.sqrt(T2 * T2 + 4 * EI2 * saite["m"] * w ** 2)) / (2 * EI2)) + d
        k = kre + 1j * k_alt.imag
        return k, saite["T"] * k / w
    return wellen


def main():
    faelle = [("gitarre_40", "gitarre", 40), ("gitarre_57", "gitarre", 57), ("gitarre_83", "gitarre", 83),
              ("laute_38", "laute", 38), ("laute_58", "laute", 58), ("laute_65", "laute", 65),
              ("oud_36", "oud", 36), ("oud_72", "oud", 72),
              ("harfe_36", "harfe", 36), ("harfe_60", "harfe", 60), ("harfe_91", "harfe", 91),
              ("cembalo_36", "cembalo", 36), ("cembalo_60", "cembalo", 60), ("cembalo_89", "cembalo", 89)]
    print("je Fall (erste Saite): Teiltoene bis 20 kHz | ohne Anpassung: groesste Verschiebung gegen Teilton 1 (Cent) | "
          "angepasst: Rest (Cent), T' / T, EI' / EI | Vorlaeufer vor -2 / -20 ms heute -> kausal (Spitze / Energie dB)")
    for name, instrument, taste in faelle:
        e = bz.eingaben(instrument, taste, 0.0)
        sa, L = e[0][0], e[1][0]
        (pT, pEI), rest, ohne, n = anpassen(sa, L)
        x0, fs = ur.quelle_zeit(e, instrument, dauer=16.0)   # 16 s, s0 0,5: der Auslaeufer faellt nicht als Boden auf den Anfang
        ps.saite_wellen = kausal_wellen(sa, sa["T"] * pT, sa["EI"] * pEI)
        try:
            x1, _ = ur.quelle_zeit(e, instrument, dauer=16.0)
        finally:
            ps.saite_wellen = WELLEN
        print(f"  {name:11s} {n:2d} | {np.abs(ohne).max():6.2f} | {np.abs(rest).max():6.3f}  {pT:.6f}  {pEI:.4f} | "
              f"{ur.vorlaeufer(x0, fs)}  ->  {ur.vorlaeufer(x1, fs)}", flush=True)


if __name__ == "__main__":
    main()
