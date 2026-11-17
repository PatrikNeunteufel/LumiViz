"""LVR Auftrag 11: je Gleichung ein Zahlenbeispiel (G-Saite der Gitarre, leer, 196 Hz; Cembalo Taste 84; fs 48 kHz).
Aufruf: python -B beispiele.py"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ue", HIER / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
ps, FS = ue.ps, ue.FS
sa, L = ps.GITARRE[3], ps.MENSUR
T, m, EI = sa["T"], sa["m"], sa["EI"]
Gt, ph = ue.ziel(sa, L)
fn, se = ue.teiltoene(Gt, ph)
f1 = fn[0]
print(f"(Z1/Z2) G-Saite: f1 {f1:.3f} Hz (c/2L = {np.sqrt(T / m) / (2 * L):.3f} Hz), sigma_1 {se[0]:.4f} 1/s, sigma_8 {se[7]:.3f}, sigma_16 {se[15]:.3f}")
B = np.pi ** 2 * EI / (T * L * L)
print(f"(Z4) B = pi^2 EI/(T L^2) = {B:.3e}; f_n-Naeherung n f0 sqrt(1+B n^2) fuer n=12: {12 * np.sqrt(T / m) / (2 * L) * np.sqrt(1 + B * 144):.2f} Hz, exakt {fn[11]:.2f} Hz")
for n in (1, 8, 16):
    f = fn[n - 1]
    w = 2 * np.pi * f
    s3 = sa["etaA"] / 2 + np.pi * sa["etaF"] * f + sa["etaB"] * EI * m * w ** 3 / (2 * T * T)
    print(f"(Z3) Naeherung sigma(f_{n}) = eta_A/2 + pi eta_F f + eta_B EI m w^3/(2 T^2) = {s3:.3f} 1/s (exakt {se[n - 1]:.3f})")
print(f"(Z5) -ln G je Umlauf bei f1: {-np.log(np.interp(f1, ue.F, Gt)):.3e}; sigma_1/f1 = {se[0] / f1:.3e}")
b, a = ue.pol1_2p(Gt, f1, 8 * f1)
h = lambda f: abs(b / (1 + a * np.exp(-2j * np.pi * f / FS)))
print(f"(V2) Ein-Pol durch f1 und 8 f1: b {b:.6f}, a {a:+.6f}; |H(f1)| {h(f1):.6f} = Ziel {np.interp(f1, ue.F, Gt):.6f}; "
      f"|H(8f1)| {h(8 * f1):.6f} = Ziel {np.interp(8 * f1, ue.F, Gt):.6f}; |H(0)| = b/(1+a) = {b / (1 + a):.6f} <= 1")
g = np.interp(fn[:3], ue.F, Gt)
print(f"(V4) Bank & Valimaki: tau_k = -1/(f0 ln g_k): {[-1 / (f1 * np.log(x)) for x in g]} s; Gewicht 1/(1-g)^4 (rel. zu k=1): "
      f"{[(1 - g[0]) ** 4 / (1 - x) ** 4 for x in g]}")
o = ue.entwurf(sa, L, "pol1p", ("K", 16, 5000.0), ("thiran", 1))
info = o[4]
aap = info["a_ap"]
print(f"(D1/D5) K16, a {aap:+.4f}: Phasenverzug je Stufe bei 0 = (1-a)/(1+a) = {(1 - aap) / (1 + aap):.3f} Takte; K mal = {16 * (1 - aap) / (1 + aap):.1f}")
w1 = 2 * np.pi * f1 / FS
hl = ue.antwort_pol1(*info["koef"], np.array([w1]))
print(f"(S1) Umlauf {ph[np.searchsorted(ue.F, f1)] / w1:.3f} Takte = N {info['Nint']} + Delta {info['Delta']:.4f} + Filter; Thiran-1-Wert eta = (1-D)/(1+D) = {(1 - info['Delta']) / (1 + info['Delta']):+.4f}")
print(f"(D2) Rauhala-Formel M 8: a1 = {ue.gp.rauhala(f1, B, 8, FS):+.4f}; (D3) adf, df 5 kHz: nap = {len(ue.gp.adf(f1, B, 5000.0, 0.85, FS)[0])} Biquads")
for N in (1, 3, 5):
    hh, _ = ue.lagrange((N - 1) / 2 + 0.5, N)
    for f in (5000.0, 10000.0):
        x = 2 * np.pi * f / FS
        print(f"(S3) Lagrange {N}, Bruchteil 0,5: |H({f / 1000:.0f} kHz)| = {abs(np.sum(hh * np.exp(-1j * x * np.arange(N + 1)))):.4f}"
              + (f"  (cos(w/2) = {np.cos(x / 2):.4f})" if N == 1 else ""))
sc, Lc, *_ = ps.cembalo_taste(84)
G84, ph84 = ue.ziel(sc, Lc)
f84, s84 = ue.teiltoene(G84, ph84)
x = 2 * np.pi * 5000 / FS
print(f"(S4) Cembalo 84: -ln G(5 kHz) je Umlauf {-np.log(np.interp(5000, ue.F, G84)):.2e} gegen -ln|H_Lagrange1(Delta 0,5)| = {-np.log(np.cos(x / 2)):.2e}")
D0 = FS / ps.hz_von(64)
print(f"(B2) E4: Verzug {D0:.2f} Takte, +1 Halbton in 50 ms: {D0 * (1 - 2 ** (-1 / 12)) / (0.05 * FS):.5f} Takte je Takt")
print(f"(B3) fester Verlust, +1 / +2 Halbtoene: Grundton-Abklingen + {100 * (2 ** (1 / 12) - 1):.1f} / + {100 * (2 ** (2 / 12) - 1):.1f} % (Naeherung)")
for f in (5000, 8000, 12000, 16000):
    print(f"(M6) foh: Hoehe der Spitze 20 log sinc^2(f/fs) bei {f} Hz = {20 * np.log10(np.sinc(f / FS) ** 2):+.2f} dB")
