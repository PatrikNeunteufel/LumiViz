"""S202: Gegenprobe zu Bericht 11 der Research-Werkbank (Umlauf-Filter, Grundlagen).

Unabhaengig vom Code des Laufs nachgerechnet:
  (1) Gl. (M5) foh-Diskretisierung einer Mode gegen eine feine Zeitintegration mit stueckweise linearer Kraft;
      Gl. (M6) Hoehe der Spitze sinc^2.
  (2) Passivitaet je Mode (min Re Y_d / max Re Y) fuer ii, trapez, bilinear, foh an Q 1,5 bei 10 kHz.
  (3) Gl. (V2) Ein-Pol durch zwei Sollwerte (Algebra), Gl. (S3) Lagrange-Betrag, Gl. (S4) Beispiel Cembalo 84,
      Gl. (D1) Verzug je Allpass-Stufe, Gl. (B3), Thiran 1.
  (4) Befund V6 (Bank-Entwurf nicht passiv): Beiwerte aus dem Code des Laufs, Betrag mit scipy.signal.freqz
      unabhaengig ausgewertet; dazu der Fehler der Spalte "Umlauf Takte" (Raster statt f1).
Aufruf: python -X utf8 -B 2026-11-17_S202_gegenprobe_bericht11.py
"""
import importlib.util
import pathlib

import numpy as np
from scipy import signal

FS = 48000.0
T = 1 / FS
HIER = pathlib.Path(__file__).resolve().parent
LAUF = HIER.parent / "recherche" / "2026-11-16_S202_umlauf_filter_grundlagen_werkbank"


def mode(fn, Q):
    wn = 2 * np.pi * fn
    eta = 1 / Q
    lam = -eta * wn / 2 + 1j * wn * np.sqrt(1 - eta * eta / 4)
    R = lam / (2j * lam.imag)
    return lam, R


def y_kont(f, lam, R):
    s = 2j * np.pi * f
    return R / (s - lam) + np.conj(R) / (s - np.conj(lam))


def foh_koef(lam, R):
    p = np.exp(lam * T)
    x = lam * T
    b0 = R * (p - 1 - x) / (lam * x)
    b1 = R * (1 - p + x * p) / (lam * x)
    return p, b0, b1


def y_diskret(f, art, lam, R):
    z1 = np.exp(-2j * np.pi * f / FS)
    p = np.exp(lam * T)
    if art == "ii":
        h = T * R / (1 - p * z1)
    elif art == "trapez":
        h = T * R / (1 - p * z1) - T * R.real / 2 * np.ones_like(z1)   # h(0)/2 je Haelfte: 2 Re R / 2
        h = T * R / (1 - p * z1) - T * 0.5 * R                          # je konj. Haelfte T*R/2
    elif art == "foh":
        _, b0, b1 = foh_koef(lam, R)
        h = (b0 + b1 * z1) / (1 - p * z1)
    elif art == "bilinear":
        s = 2 / T * (1 - z1) / (1 + z1)
        return R / (s - lam) + np.conj(R) / (s - np.conj(lam))
    return h + np.conj(np.conj(h)) * 0 + _konj(f, art, lam, R)


def _konj(f, art, lam, R):
    lc, Rc = np.conj(lam), np.conj(R)
    z1 = np.exp(-2j * np.pi * f / FS)
    p = np.exp(lc * T)
    if art == "ii":
        return T * Rc / (1 - p * z1)
    if art == "trapez":
        return T * Rc / (1 - p * z1) - T * 0.5 * Rc
    _, b0, b1 = foh_koef(lc, Rc)
    return (b0 + b1 * z1) / (1 - p * z1)


print("(1) foh gegen feine Integration, Kraft stueckweise linear (Mode 8 kHz, Q 10)")
lam, R = mode(8000.0, 10.0)
rng = np.random.default_rng(1)
F = rng.standard_normal(400)
# exakt: dx/dt = lam x + R F(t), Ausgang y = 2 Re x (Geschwindigkeit je Kraft), F linear zwischen Abtastwerten
sub = 2000
x = 0j
y_exakt = []
dt = T / sub
for n in range(len(F)):
    y_exakt.append(2 * x.real)
    f0, f1 = F[n], F[n + 1] if n + 1 < len(F) else 0.0
    for m in range(sub):                       # RK4 auf der linearen Kraft
        t0 = m / sub
        def g(xx, tt):
            return lam * xx + R * (f0 + (f1 - f0) * tt)
        k1 = g(x, t0)
        k2 = g(x + dt / 2 * k1, t0 + 0.5 / sub)
        k3 = g(x + dt / 2 * k2, t0 + 0.5 / sub)
        k4 = g(x + dt * k3, t0 + 1 / sub)
        x = x + dt / 6 * (k1 + 2 * k2 + 2 * k3 + k4)
y_exakt = np.array(y_exakt)
p, b0, b1 = foh_koef(lam, R)
# Bericht M5: H(z) = (b0 + b1 z^-1)/(1 - p z^-1); Ausgang y[n] = 2 Re v[n]
v = 0j
fv = 0.0
y_foh = []
for n in range(len(F)):
    # Zustand nach Kraft bis n: v[n] = p v[n-1] + b0 F[n] + b1 F[n-1]
    v = p * v + b0 * F[n] + b1 * fv
    fv = F[n]
    y_foh.append(2 * v.real)
y_foh = np.array(y_foh)
# Versatz: y_exakt[n] ist der Zustand zum Zeitpunkt n (vor Kraft n..n+1) -> mit foh-Ausgang um einen Takt verschoben?
# Das Filter nimmt vor n = 0 eine Rampe von 0 auf F[0] an, die Integration einen Sprung: Anlauf abwarten (Mode klingt in
# rund 20 Takten ab), dann muss es bis auf die Genauigkeit von RK4 gleich sein.
a, b = y_exakt[200:], y_foh[200:]
print(f"   ab Takt 200: groesster Fehler relativ {np.abs(a - b).max() / np.abs(a).max():.2e}")

print("    (M6) sinc^2 bei 5/8/12/16 kHz [dB]:",
      " / ".join(f"{20 * np.log10(np.sinc(f / FS) ** 2):.2f}" for f in (5000, 8000, 12000, 16000)))
f = np.linspace(10, 23990, 24000)
for fn, Q in ((8000.0, 10.0), (16000.0, 10.0)):
    lam, R = mode(fn, Q)
    yk = np.abs(y_kont(f, lam, R))
    yd = np.abs(y_diskret(f, "foh", lam, R))          # foh-Beiwerte tragen T schon (Einheit s)
    print(f"    Spitze foh/kontinuierlich {fn:.0f} Hz Q {Q:.0f}: {20 * np.log10(yd.max() / yk.max()):+.2f} dB")

print("\n(2) Passivitaet: min Re Y_d / max Re Y, Mode 10 kHz Q 1,5 (Bericht: ii 0,50, trapez 0,066 der Spitze fern der Mode)")
lam, R = mode(10000.0, 1.5)
for art in ("ii", "trapez", "foh", "bilinear"):
    yd = y_diskret(f, art, lam, R)
    if art in ("ii", "trapez"):
        yd = yd / T
    re = yd.real
    tief = re[f < 500].max() / re.max()
    print(f"   {art:8s}: min Re/max Re {re.min() / re.max():+.2e}; Re Y unter 500 Hz / Spitze {tief:.2e}")

print("\n(3) Einzelformeln")
# V2 algebraisch: zufaellige G1>G2, Ergebnis trifft beide Betraege
G1, G2, f1, f2 = 0.995223, 0.988381, 195.793, 8 * 195.793
c1, c2 = np.cos(2 * np.pi * f1 / FS), np.cos(2 * np.pi * f2 / FS)
r = G1 ** 2 / G2 ** 2
pp = (c2 - r * c1) / (1 - r)
for a in (-pp + np.sqrt(pp * pp - 1), -pp - np.sqrt(pp * pp - 1)):
    if abs(a) < 1:
        b = G1 * np.sqrt(1 + 2 * a * c1 + a * a)
        h = lambda ff: abs(b / (1 + a * np.exp(-2j * np.pi * ff / FS)))
        print(f"   (V2) a {a:+.6f} b {b:.6f}: |H(f1)| {h(f1):.6f}, |H(8f1)| {h(f2):.6f}, |H(0)| {b / (1 + a):.6f}"
              f"  (Bericht: a -0,210212, b 0,786102, |H(0)| 0,995333)")
for N, dl in ((1, 0.5), (3, 1.5), (5, 2.5)):
    h = np.ones(N + 1)
    for k in range(N + 1):
        for m in range(N + 1):
            if m != k:
                h[k] *= (dl - m) / (k - m)
    w, H = signal.freqz(h, worN=[5000, 10000], fs=FS)
    print(f"   (S3) Lagrange {N}, Bruchteil 0,5: |H| 5 / 10 kHz = {abs(H[0]):.4f} / {abs(H[1]):.4f}")
print(f"   (S4) -ln|H_L1(0,5)| bei 5 kHz = {-np.log(np.cos(np.pi * 5000 / FS)):.3e} (Bericht 5,45e-2)")
a = -0.4828
print(f"   (D1) (1-a)/(1+a) = {(1 - a) / (1 + a):.3f}, x16 = {16 * (1 - a) / (1 + a):.1f} (Bericht 2,867 / 45,9)")
print(f"   (B3) 2^(1/12)-1 = {100 * (2 ** (1 / 12) - 1):.2f} %, 2^(2/12)-1 = {100 * (2 ** (2 / 12) - 1):.2f} %")
D = 1.0370
print(f"   Thiran 1: eta = (1-D)/(1+D) = {(1 - D) / (1 + D):+.4f}; Tiefton-Verzug des Allpasses = {(1 - (1 - D) / (1 + D)) / (1 + (1 - D) / (1 + D)):.4f}")

print("\n(4) Befund V6 und Spalte 'Umlauf Takte' mit dem Code des Laufs")
spec = importlib.util.spec_from_file_location("ue", LAUF / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
for name, sa, L in ue.tasten():
    Gt, ph = ue.ziel(sa, L)
    fn, se = ue.teiltoene(Gt, ph)
    f1 = fn[0]
    falsch = ph[np.searchsorted(ue.F, f1)] / (2 * np.pi * f1 / FS)
    zeile = f"   {name:30s} fs/f1 {FS / f1:8.2f}  Spalte {falsch:8.2f} ({100 * (falsch * f1 / FS - 1):+.2f} %)"
    if any(k in name for k in ("Harfe Taste 46", "Harfe Taste 103", "Cembalo Taste 29", "Harfe Taste 60")):
        for N in (2, 4, 8):
            b, a = ue.iir_bank(Gt, fn, N)
            vorher = ue.iir_bank.vorher
            # die skalierte Fassung zurueckrechnen und mit freqz unabhaengig pruefen
            b0 = b * vorher / (1 - 1e-9) if vorher > 1 else b
            _, H = signal.freqz(b0, a, worN=65536)
            zeile += f"\n        iir{N}: max|H| (freqz) {np.abs(H).max():.5f}  (Lauf {vorher:.5f})"
    print(zeile)
