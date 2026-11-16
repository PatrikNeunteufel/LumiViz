"""S201 — Gegenprobe zweier Entwurfsformeln aus dem Bericht der Research-Werkbank (Auftrag 01, Abschnitt 1a / 1b).

Aufruf: python -X utf8 -B <dieses Skript>

Der Bericht gibt beide Formeln nur als Code wieder, den ein Abrufwerkzeug zitiert hat (Marke [Werkzeug]):
  1a  Rauhala & Välimäki 2006, „tunable dispersion filter“ - M gleiche Allpaesse 1. Ordnung, der Wert a1 geschlossen aus
      B, Taste und M (Faust `piano_dispersion_filter`)
  1b  Abel, Välimäki & Smith 2010 - Kaskade von Biquad-Allpaessen aus der Soll-Gruppenlaufzeit (`adf.m`)

Hier werden die Zeilen so, wie sie im Bericht stehen, nachgerechnet: an der idealen steifen Saite
f_n = n f0 sqrt(1 + B n^2) liefert ein richtiger Entwurf Teiltoene nahe am Soll. Der Umlauf ist Kette + reiner Verzug,
der Verzug so, dass der Grundton stimmt; Fehler der Teiltoene in Cent bis zur Entwurfsbandbreite. Daneben derselbe
Fall mit K gleichen Allpaessen, deren Wert angepasst ist (wie `…_S201_umlauf_filter_entwurf.py`).
Stimmt eine Abschrift nicht (Vorzeichen, Reihenfolge der Beiwerte), zeigt sich das an Fehlern von Hunderten Cent.

Schreibt nichts.
"""
import numpy as np
import scipy.optimize as opt


def soll(f0, B, n):
    return n * f0 * np.sqrt(1 + B * n * n)


def teiltoene(phase, raster, anzahl):
    n = np.arange(1, anzahl + 1)
    return np.interp(2 * np.pi * n, phase, raster)


def ap1_phase(a, x, K):
    return -K * np.angle((a + np.exp(-1j * x)) / (1 + a * np.exp(-1j * x)))


def messe(name, f0, B, fs, band, kette_phase):
    """kette_phase(x) = Phasenverzug der Kette (positiv) bei x = 2 pi f / fs."""
    raster = np.arange(1.0, min(1.5 * band, 0.49 * fs), 0.25)
    x = 2 * np.pi * raster / fs
    ziel = soll(f0, B, np.arange(1, 400))
    ziel = ziel[ziel <= band]
    n = len(ziel)
    x1 = 2 * np.pi * ziel[0] / fs
    kette = kette_phase(x)                                         # auf dem Raster abgewickelt (ein Punkt allein waere es nicht)
    d = (2 * np.pi - np.interp(ziel[0], raster, kette)) / x1       # reiner Verzug: der Grundton stimmt
    phase = d * x + kette
    if np.any(np.diff(phase) <= 0):
        print(f"  {name}: Phase nicht monoton - Abschrift oder Entwurf unbrauchbar")
        return
    f = teiltoene(phase, raster, n)
    cent = 1200 * np.log2(f / ziel)
    ohne = 1200 * np.log2(np.arange(1, n + 1) * ziel[0] / ziel)    # nur Verzug, keine Steifheit
    print(f"  {name:38s}: reiner Verzug {d:8.2f} Takte | Teiltoene 1-{n}: groesster Fehler {np.abs(cent).max():6.1f} Cent, "
          f"1-12: {np.abs(cent[:12]).max():5.1f} Cent | ohne Steifheit waeren es {np.abs(ohne).max():6.1f} / {np.abs(ohne[:12]).max():5.1f} Cent")


def rauhala(f0, B, M, fs):
    """Wie im Bericht (Faust effect.lib): a1 aus B, Taste, M."""
    k1, k2, k3 = -0.00179, -0.0233, -2.93
    m1, m2, m3, m4 = 0.0126, 0.0606, -0.00825, 1.97
    lb = np.log(max(B, 1e-6))
    kd = np.exp(k1 * lb * lb + k2 * lb + k3)
    cd = np.exp((m1 * np.log(M) + m2) * lb + m3 * np.log(M) + m4)
    trt = 2 ** (1 / 12)
    ikey = np.log(f0 * trt / 27.5) / np.log(trt)
    D = np.exp(cd - ikey * kd)
    return (1 - D) / (1 + D)


def adf(f0, B, df, beta, fs):
    """Wie im Bericht (adf.m), fs als Groesse statt fest 44100. -> (alpha, cc) je Biquad"""
    tau0 = fs / f0
    pd0 = tau0 / np.sqrt(1 + B * (df / f0) ** 2)
    mu0 = pd0 / (1 + B * (df / f0) ** 2)
    phi0 = 2 * np.pi * (df / fs) * pd0 - mu0 * 2 * np.pi * df / fs
    nap = int(np.floor(phi0 / (2 * np.pi)))
    if nap < 1:
        return np.array([]), np.array([])
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
    return alpha, cc


def biquad_phase(alpha, cc):
    def phase(x):
        z1 = np.exp(-1j * x)
        h = np.ones_like(z1)
        for a, c in zip(alpha, cc):
            h = h * (a * a + 2 * a * c * z1 + z1 * z1) / (1 + 2 * a * c * z1 + a * a * z1 * z1)
        return -np.unwrap(np.angle(h))
    return phase


def angepasst(f0, B, K, fs, band):
    ziel = soll(f0, B, np.arange(1, 400))
    ziel = ziel[ziel <= band][:40]
    x = 2 * np.pi * ziel / fs
    n = np.arange(1, len(ziel) + 1)

    def fehler(a):
        d = (2 * np.pi - ap1_phase(a, x[0], K)) / x[0]
        return float(np.sum((1200 / np.log(2) * ((d * x + ap1_phase(a, x, K)) - 2 * np.pi * n) / (2 * np.pi * n)) ** 2))
    r = np.linspace(-0.97, 0.0, 195)
    i = int(np.argmin([fehler(v) for v in r]))
    return float(opt.minimize_scalar(fehler, bounds=(r[max(i - 1, 0)], r[min(i + 1, 194)]), method="bounded").x)


if __name__ == "__main__":
    print("1b  adf.m - die drei Beispiele aus dem Kopf des Codes (fs 44100, beta 0,85), dazu angepasste gleiche Allpaesse")
    for f0, B, df in ((32.702, 0.0002, 2100.0), (65.406, 0.0001, 2500.0), (130.81, 0.00015, 4300.0)):
        fs = 44100.0
        alpha, cc = adf(f0, B, df, 0.85, fs)
        print(f" f0 {f0} Hz, B {B}, Band {df:.0f} Hz: {len(alpha)} Biquads, |alpha| {np.abs(alpha).min():.3f} ... {np.abs(alpha).max():.3f}")
        if len(alpha):
            messe(f"adf, {len(alpha)} Biquads", f0, B, fs, df, biquad_phase(alpha, cc))
        for K in (2 * len(alpha), 16):
            if K:
                a = angepasst(f0, B, K, fs, df)
                messe(f"{K} gleiche Allpaesse, angepasst (a {a:+.3f})", f0, B, fs, df, lambda x, a=a, K=K: ap1_phase(a, x, K))
    print("1a  Rauhala & Välimäki (Faust) - Wert a1 geschlossen, gegen den angepassten Wert; fs 44100")
    for f0, B, M in ((32.702, 0.0002, 8), (65.406, 0.0001, 8), (130.81, 0.00015, 8), (65.406, 0.0001, 4), (261.63, 0.0003, 4)):
        fs, band = 44100.0, 3000.0
        a = rauhala(f0, B, M, fs)
        b = angepasst(f0, B, M, fs, band)
        print(f" f0 {f0} Hz, B {B}, M {M}: a1 nach Formel {a:+.3f}, angepasst {b:+.3f}")
        messe("Formel", f0, B, fs, band, lambda x, a=a, M=M: ap1_phase(a, x, M))
        messe("angepasst", f0, B, fs, band, lambda x, b=b, M=M: ap1_phase(b, x, M))
    print("48 kHz und unsere Saiten: G-Saite der Gitarre (EI 310e-6 N m^2, T 58,3 N, L 0,65 m -> B 1,24e-4), f0 196 Hz und Bund 12")
    for f0, B in ((196.0, 1.24e-4), (392.0, 4.96e-4)):
        fs, band = 48000.0, 5000.0
        alpha, cc = adf(f0, B, band, 0.85, fs)
        print(f" f0 {f0} Hz, B {B}: adf gibt {len(alpha)} Biquads")
        if len(alpha):
            messe(f"adf, {len(alpha)} Biquads", f0, B, fs, band, biquad_phase(alpha, cc))
        for K in (8, 16):
            a = angepasst(f0, B, K, fs, band)
            messe(f"{K} gleiche Allpaesse, angepasst", f0, B, fs, band, lambda x, a=a, K=K: ap1_phase(a, x, K))
        a = rauhala(f0, B, 8, fs)
        messe(f"Rauhala-Formel M 8 (a1 {a:+.3f})", f0, B, fs, band, lambda x, a=a: ap1_phase(a, x, 8))
