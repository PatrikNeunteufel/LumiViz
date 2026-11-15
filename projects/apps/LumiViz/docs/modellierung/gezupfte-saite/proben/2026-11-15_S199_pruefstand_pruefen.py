"""S199, 34k-6 — der Pruefstand (2026-11-15_S199_pruefstand_zupf.py) gegen geschlossene Faelle.

Aufruf: python -B <dieses Skript>

1  Lautheit: Sinus 1 kHz, -23 dBFS, beide Kanaele -> -23,0 LUFS (EBU Tech 3341, Fall 1).
2  Starrer Steg, Ausgang = Kraft am Steg: je Teilton Frequenz gegen n f0 sqrt(1 + B n^2) (B = pi^2 EI / (T L^2)),
   Abklingen gegen sigma_n = eta_n w_n / 2 (Valette), Hoehe bei t = 0 gegen 2 F0 g |sin(n pi x_p / L)| / (w_n L dk/dw).
3  Zwei Polarisationen: bewegen alle Moden den Steg in derselben Richtung theta und wird unter theta losgelassen, ist
   das die Rechnung mit einer Polarisation und den Formwerten phi / cos(theta); bei Winkel 0 und tan = 0 dieselbe.
"""
import importlib.util
import pathlib

import numpy as np
from scipy.optimize import minimize_scalar

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.with_name("2026-11-15_S199_pruefstand_zupf.py"))
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
FS = ps.FS


def projektion(x, hz, t0, dauer=0.4):
    a, n = int(t0 * FS), int(dauer * FS)
    fen = np.hanning(n)
    t = np.arange(a, a + n) / FS
    return abs(np.sum(x[a:a + n] * fen * np.exp(-2j * np.pi * hz * t))) * 2 / fen.sum()


def fall3():
    kp = ps.lade_koerper("classical_guitar")
    w = ps.frequenzen()[::7]                    # ein duennes Raster genuegt fuer den Vergleich der Spektren
    sa, L = ps.GITARRE[3], ps.MENSUR
    theta = 20.0
    t = np.full(len(kp["f"]), np.tan(np.radians(theta)))
    zwei = ps.ton2(sa, L, 0.16, 0.002, theta, *ps.koerper_kurven2(kp, 3, w, t), w)
    kp1 = dict(kp)
    kp1["steg"] = kp["steg"] / np.cos(np.radians(theta))
    eins = ps.ton(sa, L, 0.16, 0.002, *ps.koerper_kurven(kp1, 3, w), w)
    null2 = ps.ton2(sa, L, 0.16, 0.002, 0.0, *ps.koerper_kurven2(kp, 3, w, 0 * t), w)
    null1 = ps.ton(sa, L, 0.16, 0.002, *ps.koerper_kurven(kp, 3, w), w)
    print(f"3  alle Moden unter {theta:.0f} Grad, Zupf unter {theta:.0f} Grad: groesster Unterschied zur Rechnung mit einer "
          f"Polarisation {np.abs(zwei - eins).max() / np.abs(eins).max():.1e} (relativ); Winkel 0, tan 0: "
          f"{np.abs(null2 - null1).max() / np.abs(null1).max():.1e}")


if __name__ == "__main__":
    fall3()
    t = np.arange(int(3 * FS)) / FS
    s = 10 ** (-23 / 20) * np.sin(2 * np.pi * 1000 * t)
    print(f"1  Sinus 1 kHz -23 dBFS stereo: {ps.momentary_max(np.stack([s, s], axis=1)):.2f} LUFS (Soll -23,0)")

    w = ps.frequenzen()
    print("2  starrer Steg: Saite, Teilton, Hz gemessen / Soll (Cent), sigma gemessen / Soll (1/s), Hoehe gemessen / Soll (N)")
    for si, bund in ((5, 0), (0, 0), (2, 7)):
        sa = ps.GITARRE[si]
        L = ps.MENSUR * 2 ** (-bund / 12)
        xp, d = 0.13, 0.002
        x = ps.zeit(ps.ton(sa, L, xp, d, np.zeros(len(w)), np.ones((len(w), 1)), w))[:, 0]
        T, m, EI = sa["T"], sa["m"], sa["EI"]
        f0 = np.sqrt(T / m) / (2 * L)
        B = np.pi ** 2 * EI / (T * L * L)
        F0 = T * d * L / (xp * (L - xp))
        for n in (1, 2, 5, 10, 20):
            soll = n * f0 * np.sqrt(1 + B * n * n)
            k = n * np.pi / L
            wn = 2 * np.pi * soll
            eta = (T * (sa["etaF"] + sa["etaA"] / wn) + EI * sa["etaB"] * k * k) / (T + EI * k * k)
            hz = minimize_scalar(lambda h: -projektion(x, h, 0.3), bounds=(soll - 2, soll + 2), method="bounded",
                                 options={"xatol": 1e-4}).x
            a1, a2 = projektion(x, hz, 0.3), projektion(x, hz, 1.3)
            sigma = np.log(a1 / a2) / 1.0
            dkdw = m * wn / (k * (T + 2 * EI * k * k))
            hoehe = 2 * F0 * np.exp(-(wn * 0.0075 / np.sqrt(T / m)) ** 2) * abs(np.sin(k * xp)) / (wn * L * dkdw)
            print(f"   Saite {6 - si} Bund {bund:2d} n={n:2d}: {hz:9.3f} / {soll:9.3f} ({1200 * np.log2(hz / soll):+.2f})   "
                  f"{sigma:7.3f} / {eta * wn / 2:7.3f}   {a1 * np.exp(sigma * (0.5 - ps.VORLAUF)):8.4f} / {hoehe:8.4f}")
