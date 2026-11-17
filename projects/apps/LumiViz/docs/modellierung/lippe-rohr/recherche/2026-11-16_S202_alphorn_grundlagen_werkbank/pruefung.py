"""Kontrollen des Rechenwegs (Abschnitt 3 des Berichts): K1-K5."""
import numpy as np
from scipy.optimize import brentq
import tmm

print("== K5 Zahlenbeispiel Verluste: (2.73)/(2.74) in (2.75)/(2.76) -> Konstanten von (2.77)/(2.78)")
k_v = 1 / (np.sqrt(2) * 632.8) + 0.4 / (np.sqrt(2) * 532.8)
print(f"  1/(sqrt2*632.8) + 0.4/(sqrt2*532.8) = {k_v:.4e}   (Hendrie 2.77: 1.65e-3)")
print(f"  2*pi*{k_v:.4e}/345 = {2*np.pi*k_v/345:.3e}            (Hendrie 2.78: 3e-5, mit c = 345 m/s S. 25)")
L300 = tmm.luft(26.85)
print(f"  G1 bei 20 C: c = {tmm.luft(20)['c']:.2f} m/s, rho = {tmm.luft(20)['rho']:.4f} kg/m3;"
      f" bei 300 K: c = {L300['c']:.2f} m/s")

print("\n== K1 verlustfreier Zylinder r = 20 mm, L = 1 m, ideal offen (Z_L = 0): Spitzen bei (2n-1) c / 4L")
lu = tmm.luft(20)
seg = [(1000.0, 20.0, 20.0)]
sp = tmm.spitzen(seg, lu, verlust=False, abstrahlung="offen", fmax=1000, n_max=5)
for n, (f, z, q) in enumerate(sp, 1):
    soll = (2 * n - 1) * lu["c"] / 4
    print(f"  n={n}: TMM {f:9.4f} Hz  geschlossen {soll:9.4f} Hz  Diff {tmm.cent(f, soll):+.4f} Cent")

print("\n== K2 verlustfreier Kegelstumpf r 5 -> 50 mm, L = 1 m, ideal offen: Hendrie (2.103) -> sin(kL + atan(k x1)) = 0")
seg = [(1000.0, 5.0, 50.0)]
x1 = 5.0 * 1000.0 / 45.0 * 1e-3
sp = tmm.spitzen(seg, lu, verlust=False, abstrahlung="offen", fmax=1000, n_max=5)
for n, (f, z, q) in enumerate(sp, 1):
    g = lambda ff: np.sin(2 * np.pi * ff / lu["c"] * 1.0 + np.arctan(2 * np.pi * ff / lu["c"] * x1))
    soll = brentq(g, f - 15, f + 15)
    print(f"  n={n}: TMM {f:9.4f} Hz  (2.103) {soll:9.4f} Hz  Diff {tmm.cent(f, soll):+.4f} Cent")

print("\n== K3 Hendrie (2.102) mit Last (ungeflanscht, G8) gegen eigene Kegelmatrix, Kegel r 5 -> 50 mm, L = 1 m")
f = np.array([50.0, 137.0, 300.0, 555.0])
k = 2 * np.pi * f / lu["c"]
r1, r2, L = 5e-3, 50e-3, 1.0
x1 = r1 * L / (r2 - r1); x2 = x1 + L
t1, t2 = np.arctan(k * x1), np.arctan(k * x2)
Zc1 = lu["rho"] * lu["c"] / (np.pi * r1 ** 2); Zc2 = lu["rho"] * lu["c"] / (np.pi * r2 ** 2)
ZL = tmm.z_abstrahlung(f, r2, lu, "unflanged")
num = 1j * ZL * np.sin(k * L - t2) / np.sin(t2) + Zc2 * np.sin(k * L)
den_druck = ZL * np.sin(k * L + t1 - t2) - 1j * Zc2 * np.sin(k * L + t1) / np.sin(t1)
den_FR = ZL * np.sin(k * L + t1 - t2) / (np.sin(t1) * np.sin(t2)) - 1j * Zc2 * np.sin(k * L + t1) / np.sin(t1)
Z_druck = Zc1 * num / den_druck
Z_FR = Zc1 * num / den_FR
Z_tmm = tmm.zin([(1000.0, 5.0, 50.0)], f, lu, verlust=False, abstrahlung="unflanged")
for i in range(len(f)):
    print(f"  f={f[i]:6.1f}: |Z_TMM|={abs(Z_tmm[i]):.4e}  |(2.102) wie gedruckt|={abs(Z_druck[i]):.4e}"
          f"  rel.Abw {abs(Z_druck[i]-Z_tmm[i])/abs(Z_tmm[i]):.2e}  |mit 1/(sin t1 sin t2)|={abs(Z_FR[i]):.4e}"
          f"  rel.Abw {abs(Z_FR[i]-Z_tmm[i])/abs(Z_tmm[i]):.2e}")

print("\n== K4 Hendrie Abb. 2.12: verlustbehafteter Zylinder r = 2 cm, L = 1 m, ungeflanscht, c = 345 m/s")
lu345 = tmm.luft(26.85, c=345.0)   # Hendrie S. 25: c = 345 m/s; Verlustformeln bei 300 K
seg = [(1000.0, 20.0, 20.0)]
sp = tmm.spitzen(seg, lu345, verlust=True, abstrahlung="unflanged", fmax=2000, n_max=12)
Zc = lu345["rho"] * 345.0 / (np.pi * 0.02 ** 2)
for n, (f, z, q) in enumerate(sp, 1):
    print(f"  n={n:2d}: {f:8.2f} Hz  |Z/Zc| = {z*1e6/Zc:6.1f}  Q = {q:6.1f}   (2.92): {(2*n-1)*345/(4*(1+0.6*0.02)):8.2f} Hz")

print("\n== K4b wie K4, aber so wie Hendrie Abb. 2.12 rechnet: (2.92) = ideal offen bei L + 0.6 r, nur Wandverluste")
abgelesen = [None, 40, 31, 27, 24, 22, 20, 18, 17, 16, 15.5, 15]   # Abb. 2.12 oben, |Z/Zc|; 1. Spitze abgeschnitten (>50)
sp = tmm.spitzen([(1012.0, 20.0, 20.0)], lu345, verlust=True, abstrahlung="offen", fmax=2000, n_max=12)
for n, (f, z, q) in enumerate(sp, 1):
    zz = z * 1e6 / Zc
    print(f"  n={n:2d}: {f:8.2f} Hz  |Z/Zc| = {zz:5.1f} ({20*np.log10(zz):4.1f} dB)  abgelesen {abgelesen[n-1]}")
