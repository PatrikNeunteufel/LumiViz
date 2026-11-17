"""LVR Auftrag 21, Punkt 3: Dispersionsformeln gegen die Originale.
(1) Rauhala & Valimaki, DAFx-06 (Seitenbild): Gl. (3)-(8), Tab. 2. Gl. (7) wie gedruckt
    Cd = exp((m1 ln M + m2) ln M + m3 ln M + m4)  gegen  Lesart Cd = exp((m1 ln M + m2) ln B + m3 ln M + m4)
    (= Gl. (6) mit C1 = m1 ln M + m2, C2 = m3 ln M + m4, Abb. 2). Vergleich mit Werten, die an Abb. 3 und 4 abgelesen sind.
(2) Abb. 5 (a)-(c) nachgerechnet: Abweichung der Teiltoene in Cent fuer M = 1, 5, 10, 20, 30, 40, fs 44,1 und 48 kHz.
(3) adf.m (roh aus der Wayback Machine, abrufe/adf_m_wayback.txt) neu abgeschrieben, gegen die Abschrift der S201-
    Gegenprobe verglichen; Beispiel 1 (32,702 Hz, B 2e-4, df 2100 Hz) je Teilton, mit zwei Stimmungen.
Aufruf: python -B dispersion_original.py -> dispersion_original.txt
"""
import sys
sys.dont_write_bytecode = True
import pathlib

import numpy as np

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import bank_voll as bv  # noqa: E402  (nur fuer Tee und den Import der S201-Gegenprobe ueber ue)

gp = bv.ue.gp
K = dict(k1=-0.00179, k2=-0.0233, k3=-2.93, m1=0.0126, m2=0.0606, m3=-0.00825, m4=1.97)   # Tab. 2, am Seitenbild


def ikey(f0):
    t = 2 ** (1 / 12)
    return np.log(f0 * t / 27.5) / np.log(t)                                  # Gl. (4)


def a1(f0, B, M, gedruckt=False):
    lb, lm = np.log(B), np.log(M)
    kd = np.exp(K["k1"] * lb * lb + K["k2"] * lb + K["k3"])                  # Gl. (5)
    if gedruckt:
        cd = np.exp((K["m1"] * lm + K["m2"]) * lm + K["m3"] * lm + K["m4"])  # Gl. (7) wie gedruckt
    else:
        cd = np.exp((K["m1"] * lm + K["m2"]) * lb + K["m3"] * lm + K["m4"])  # Gl. (7), Lesart ln B
    D = np.exp(cd - ikey(f0) * kd)                                            # Gl. (3)
    return (1 - D) / (D + 1)                                                  # Gl. (8)


def ap1_lag(a, x, M):
    return -M * np.unwrap(np.angle((a + np.exp(-1j * x)) / (1 + a * np.exp(-1j * x))))


def abweichung(f0, B, lag, fs, fmax, stimmung="grundton", d_fest=None):
    """Teiltoene einer Schleife 'reiner Verzug d + Kette', Abweichung gegen f_k = k f0 sqrt(1 + B k^2) (Gl. 1) in Cent."""
    f = np.arange(1.0, 0.49 * fs, 0.25)
    x = 2 * np.pi * f / fs
    kette = lag(x)
    soll = f0 * np.arange(1, 400) * np.sqrt(1 + B * np.arange(1, 400) ** 2)
    soll = soll[soll <= fmax]
    if stimmung == "grundton":
        x1 = 2 * np.pi * soll[0] / fs
        d = (2 * np.pi - np.interp(soll[0], f, kette)) / x1
    else:
        d = d_fest
    ph = d * x + kette
    n = np.arange(1, len(soll) + 1)
    fm = np.interp(2 * np.pi * n, ph, f)
    return soll, 1200 * np.log2(fm / soll), d


def adf_roh(f0, B, df, beta, fs=44100.0):
    """adf.m Zeile fuer Zeile (abrufe/adf_m_wayback.txt, Z. 37-62)."""
    tau0 = fs / f0
    pd0 = tau0 / np.sqrt(1 + B * (df / f0) ** 2)
    mu0 = pd0 / (1 + B * (df / f0) ** 2)
    phi0 = 2 * np.pi * (df / fs) * pd0 - mu0 * 2 * np.pi * df / fs
    nap = int(np.floor(phi0 / (2 * np.pi)))
    phik = np.pi * np.arange(1, 2 * nap, 2)
    etan = np.arange(nap) / (1.2 * nap) * df
    pdn = tau0 / np.sqrt(1 + B * (etan / f0) ** 2)
    taun = pdn / (1 + B * (etan / f0) ** 2)
    phin = 2 * np.pi * (etan / fs) * pdn
    theta = fs / (2 * np.pi) * (phik - phin + (2 * np.pi / fs) * etan * taun) / (taun - mu0)
    delta = np.diff(np.concatenate([[-theta[0]], theta])) / 2
    cc = np.cos(theta * 2 * np.pi / fs)
    eta = (1 - beta * np.cos(delta * 2 * np.pi / fs)) / (1 - beta)
    alpha = np.sqrt(eta ** 2 - 1) - eta
    temp = np.column_stack([np.ones(nap), 2 * alpha * cc, alpha ** 2])
    sos = np.hstack([temp[:, ::-1], temp])                                   # [b0 b1 b2 a0 a1 a2]
    return sos, dict(tau0=tau0, pd0=pd0, mu0=mu0, phi0=phi0, nap=nap, theta=theta)


def sos_lag(sos):
    def lag(x):
        z = np.exp(-1j * x)
        h = np.ones_like(z)
        for b0, b1, b2, a0, a1_, a2 in sos:
            h = h * (b0 + b1 * z + b2 * z * z) / (a0 + a1_ * z + a2 * z * z)
        return -np.unwrap(np.angle(h))
    return lag


if __name__ == "__main__":
    sys.stdout = bv.Tee("dispersion_original.txt")
    print("(1) a1 nach Gl. (3)-(8), Tab. 2 - gedruckte Gl. (7) gegen Lesart 'ln B'; daneben an der Abbildung abgelesen (+-0,02)")
    print("  Abb. 3 (f0 65,406 Hz, Taste 16): M | B | Lesart ln B | gedruckt | abgelesen")
    abb3 = {(40, 1e-6): -0.35, (20, 1e-6): -0.45, (10, 1e-6): -0.53, (5, 1e-6): -0.61, (1, 1e-6): -0.82,
            (40, 1e-3): -0.84, (20, 1e-3): -0.87, (10, 1e-3): -0.89, (5, 1e-3): -0.92, (1, 1e-3): -0.955}
    for (M, B), ab in abb3.items():
        print(f"    {M:2d} | {B:.0e} | {a1(65.406, B, M):+.3f} | {a1(65.406, B, M, True):+.3f} | {ab:+.3f}")
    print("  Abb. 4 (M 8): Taste | B | Lesart ln B | gedruckt | abgelesen")
    abb4 = {(4, 1e-6): -0.73, (16, 1e-6): -0.57, (28, 1e-6): -0.29, (40, 1e-6): -0.01,
            (4, 1e-3): -0.95, (16, 1e-3): -0.88, (28, 1e-3): -0.80, (40, 1e-3): -0.65}
    for (key, B), ab in abb4.items():
        f0 = 27.5 * 2 ** ((key - 1) / 12)
        print(f"    {key:2d} ({f0:7.3f} Hz) | {B:.0e} | {a1(f0, B, 8):+.3f} | {a1(f0, B, 8, True):+.3f} | {ab:+.3f}")
    print("  Gegen die Faust-Abschrift (S201 gp.rauhala, 'ln B'): groesste Differenz ueber Abb. 3/4-Punkte "
          f"{max(abs(a1(65.406, B, M) - gp.rauhala(65.406, B, M, 44100)) for (M, B) in abb3):.2e}")

    print("\n(2) Abb. 5 nachgerechnet (Lesart ln B): Abweichung in Cent, Stimmung am Grundton. Je M: groesster Wert und bei Hz,"
          " erste Frequenz mit < -8,39 Cent (0,5 %-Grenze), Wert bei 700 / 1300 Hz (a), 1500 / 2800 Hz (b), 2500 / 3800 Hz (c)")
    for tag, f0, B, fmax, pr in (("a", 32.703, 0.0002, 1800, (700, 1300)), ("b", 65.406, 0.0001, 3000, (1500, 2800)),
                                 ("c", 130.82, 0.00015, 5000, (2500, 3800))):
        for fs in (44100.0, 48000.0):
            print(f"  ({tag}) f0 {f0} Hz, B {B}, fs {fs:.0f}")
            for M in (1, 5, 10, 20, 30, 40):
                a = a1(f0, B, M)
                soll, c, d = abweichung(f0, B, lambda x, a=a, M=M: ap1_lag(a, x, M), fs, fmax)
                i = int(np.argmax(c))
                unter = soll[c < -8.39]
                w = np.interp(pr, soll, c)
                print(f"    M {M:2d}: a1 {a:+.4f} | max {c[i]:+6.2f} bei {soll[i]:6.0f} Hz | < -8,39 ab {unter[0] if len(unter) else np.nan:6.0f} Hz"
                      f" | {w[0]:+7.1f} / {w[1]:+7.1f}")

    print("\n(3) adf.m roh gegen die Abschrift in der S201-Gegenprobe (gp.adf), fs 44100, beta 0,85")
    for f0, B, df in ((32.702, 0.0002, 2100.0), (65.406, 0.0001, 2500.0), (130.81, 0.00015, 4300.0)):
        sos, inf = adf_roh(f0, B, df, 0.85)
        al, cc = gp.adf(f0, B, df, 0.85, 44100.0)
        dif = max(np.abs(sos[:, 4] - 2 * al * cc).max(), np.abs(sos[:, 5] - al ** 2).max())
        print(f"  f0 {f0}, B {B}, df {df:.0f}: nap {inf['nap']} (S201 {len(al)}), groesste Differenz der Beiwerte {dif:.1e}; "
              f"phi0/2pi {inf['phi0'] / 2 / np.pi:.3f}, mu0 {inf['mu0']:.2f}, tau0 {inf['tau0']:.2f}; "
              f"Pole bei {inf['theta'][0]:.0f} ... {inf['theta'][-1]:.0f} Hz")
        lag = sos_lag(sos)
        for stimm, dd in (("grundton", None), ("Verzug mu0", inf["mu0"])):
            soll, c, d = abweichung(f0, B, lag, 44100.0, df, stimm, dd)
            n = np.arange(1, len(soll) + 1)
            gut = soll[np.abs(c) <= 8.39]
            print(f"    Stimmung {stimm:10s} (Verzug {d:7.2f}): Teiltoene 1-{len(soll)}; |Cent| 1-12 max {np.abs(c[:12]).max():6.1f}, "
                  f"alle max {np.abs(c).max():6.1f} (Teilton {n[int(np.argmax(np.abs(c)))]}, {soll[int(np.argmax(np.abs(c)))]:.0f} Hz); "
                  f"innerhalb 8,39 Cent bis {gut.max() if len(gut) else 0:.0f} Hz zusammenhaengend bis "
                  f"{soll[np.argmax(np.abs(c) > 8.39)] if np.any(np.abs(c) > 8.39) else soll[-1]:.0f} Hz")
        if f0 < 40:
            soll, c, _ = abweichung(f0, B, lag, 44100.0, df)
            print("    je Teilton (Stimmung Grundton), Cent: " + " ".join(f"{v:+.1f}" for v in c))
        # (4) Polfrequenzen exakt statt mit einem Newton-Schritt: psi(theta) = 2 pi theta/fs (pd(theta) - mu0) = (2k-1) pi
        import scipy.optimize as so
        fs = 44100.0
        pd = lambda f: inf["tau0"] / np.sqrt(1 + B * (f / f0) ** 2)
        psi = lambda f: 2 * np.pi * f / fs * (pd(f) - inf["mu0"])
        th = np.array([so.brentq(lambda f, k=k: psi(f) - (2 * k - 1) * np.pi, 1e-6, df) for k in range(1, inf["nap"] + 1)])
        delta = np.diff(np.concatenate([[-th[0]], th])) / 2
        eta = (1 - 0.85 * np.cos(delta * 2 * np.pi / fs)) / (1 - 0.85)
        al = np.sqrt(eta ** 2 - 1) - eta
        sos2 = np.column_stack([al ** 2, 2 * al * np.cos(th * 2 * np.pi / fs), np.ones(len(th)),
                                np.ones(len(th)), 2 * al * np.cos(th * 2 * np.pi / fs), al ** 2])
        soll, c, d = abweichung(f0, B, sos_lag(sos2), fs, df)
        print(f"    (4) Pole exakt (Newton-Schritt ersetzt): groesste Verschiebung {np.abs(th - inf['theta']).max():.1f} Hz; "
              f"|Cent| 1-12 max {np.abs(c[:12]).max():.1f}, alle max {np.abs(c).max():.1f}; Fehler bis zum letzten Pol "
              f"({th[-1]:.0f} Hz) max {np.abs(c[soll <= th[-1]]).max():.1f} Cent")
        # (5) Welches Teiltongesetz steckt in adf.m? pd(f) = tau0 / sqrt(1 + B (f/f0)^2) und f pd(f) / fs = n
        #     => f_n = n f0 / sqrt(1 - B n^2). Gegen Gl. (1) Rauhala: f_n = n f0 sqrt(1 + B n^2).
        n = np.arange(1, len(soll) + 1)
        g2 = n * f0 / np.sqrt(1 - B * n ** 2)
        diff = 1200 * np.log2(g2 / (n * f0 * np.sqrt(1 + B * n ** 2)))
        for name_, sos_ in (("adf.m", sos), ("Pole exakt", sos2)):
            f = np.arange(1.0, 0.49 * fs, 0.25)
            x = 2 * np.pi * f / fs
            kette = sos_lag(sos_)(x)
            d = (2 * np.pi - np.interp(g2[0], f, kette)) / (2 * np.pi * g2[0] / fs)
            fm = np.interp(2 * np.pi * n, d * x + kette, f)
            cc_ = 1200 * np.log2(fm / g2)
            mm = g2 <= df
            print(f"    (5) {name_:10s} gegen f_n = n f0/sqrt(1 - B n^2) bis df: |Cent| 1-12 max {np.abs(cc_[:12]).max():.1f}, "
                  f"alle max {np.abs(cc_[mm]).max():.1f}; beide Gesetze unterscheiden sich am Teilton {n[mm][-1]} um {diff[mm][-1]:.1f} Cent, "
                  f"am Teilton 12 um {diff[11]:.2f} Cent")
