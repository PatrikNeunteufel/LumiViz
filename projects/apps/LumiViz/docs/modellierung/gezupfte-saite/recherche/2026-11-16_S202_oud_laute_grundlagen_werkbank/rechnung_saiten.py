"""Saitensaetze Oud und Laute als Rechenwerte (Auftrag 2026-11-16_15). Schreibt nichts.

Gleichungen: Woodhouse & Lynch-Aird 2019 (W&LA) Gl. 3, 4, 8, 11-17 (S. 517-520, am Seitenbild geprueft);
Valette nach Woodhouse u. a. 2012 Gl. 4 (S. 477). Umsponnene Saiten: Aufbau-Modell [hergeleitet].
"""
import math

RHO_A, NU_A = 1.2, 1.5e-5
MAT = {"N": (4.5, 39.0, 0.25, 1070.0), "F": (3.2, 41.0, 0.18, 1800.0)}  # W&LA Tab. I: E' = a + b*sigma (GPa), E'' GPa, rho


def hz(taste):
    return 440.0 * 2 ** ((taste - 69) / 12)


def monofil(d_mm, f, L, mat):
    a, b, e2, rho = MAT[mat]
    d = d_mm / 1000
    A = math.pi * d * d / 4
    mu = rho * A
    T = 4 * L * L * f * f * mu                      # Gl. 3
    sigma = T / A / 1e9                              # GPa
    E = (a + b * sigma) * 1e9                         # Gl. 16 / 17
    EI = E * math.pi * d ** 4 / 64
    etaE = e2 * 1e9 / E
    out = {"d": d_mm, "f": f, "mu": mu, "T": T, "sigma_GPa": sigma, "E_GPa": E / 1e9, "EI": EI, "etaE": etaE,
           "Z0": math.sqrt(T * mu)}
    for n in (1, 10):
        fn = n * f
        lam = E * math.pi ** 2 * d * d * n * n / (64 * rho * L ** 4 * f * f + E * math.pi ** 2 * d * d * n * n)  # Gl. 4
        M = d / 4 * math.sqrt(2 * math.pi * fn / NU_A)                                                        # Gl. 12
        eta_air = RHO_A / rho * (2 * math.sqrt(2) * M + 1) / M ** 2                                            # Gl. 11
        eta_air_k = eta_air * (d * 1000 + 0.2)                                                                 # Zuschlag S. 518
        out[n] = {"lam": lam, "eta_air": eta_air_k, "eta_bend": lam * etaE, "eta": eta_air_k + lam * etaE}       # Gl. 14, 15
    return out


print("=== Gegenprobe Gl. 3 gegen W&LA S. 524: 'the 2nd, 3rd and 4th courses ... same tension, around 30 N'")
LAUTE = [(1, 392.0, [(0.475, "N", 1)]), (2, 293.7, [(0.52, "N", 1), (0.52, "N", 1)]), (3, 220.0, [(0.725, "N", 1), (0.725, "N", 1)]),
         (4, 174.6, [(0.900, "N", 1), (0.900, "N", 1)]), (5, 130.8, [(0.91, "F", 1), (0.91, "F", 1)]),
         (6, 98.0, [(1.25, "F", 1), (0.775, "N", 2)]), (7, 87.3, [(1.40, "F", 1), (0.875, "N", 2)]), (8, 73.4, [(1.60, "F", 1), (0.925, "N", 2)])]
L_LAUTE = 0.625
print("Laute (W&LA Tab. II, L = 0,625 m)")
print("Chor Saite  f Hz   d mm  W  mu g/m   T N   sigma MPa  E GPa  EI 1e-6 Nm2  lam1     eta_air1  eta_bend1  eta1    Z0 Ns/m  lam10   eta10")
for c, f, saiten in LAUTE:
    for k, (d, m, okt) in enumerate(saiten):
        s = monofil(d, f * okt, L_LAUTE, m)
        print(f"{c:4d} {k + 1:4d} {s['f']:6.1f} {d:6.3f} {m}  {s['mu'] * 1e3:6.3f} {s['T']:6.1f} {s['sigma_GPa'] * 1e3:8.1f} {s['E_GPa']:6.2f} "
              f"{s['EI'] * 1e6:10.2f}  {s[1]['lam']:.2e} {s[1]['eta_air']:.2e} {s[1]['eta_bend']:.2e} {s[1]['eta']:.2e} {s['Z0']:6.3f}  "
              f"{s[10]['lam']:.3f}  {s[10]['eta']:.2e}")

print()
print("=== Oud, Diskant (D'Addario EJ95A: G3 .031 in, C4 .026 in Nylon), drei Mensuren")
for L in (0.585, 0.60, 0.615):
    for t, d in ((55, 0.031 * 25.4), (60, 0.026 * 25.4)):
        s = monofil(d, hz(t), L, "N")
        print(f"L {L:.3f}  Taste {t}  f {s['f']:6.1f}  d {d:.3f} mm  mu {s['mu'] * 1e3:.3f} g/m  T {s['T']:5.1f} N ({s['T'] / 9.80665:.2f} kgf)  "
              f"E {s['E_GPa']:.2f} GPa  EI {s['EI'] * 1e6:.1f}e-6  lam1 {s[1]['lam']:.1e}  eta1 {s[1]['eta']:.2e}  lam10 {s[10]['lam']:.3f}  eta10 {s[10]['eta']:.2e}  Z0 {s['Z0']:.3f}")

print()
print("=== Oud, umsponnen: mu aus Zug (EJ95A, kgf) und mu aus dem Aufbau")
# EJ95A: Chor, Taste, Aussendurchmesser in (Herstellerangabe, S195 O2), Zug kgf
UMSP = [("D3", 50, 0.024, 4.034), ("A2", 45, 0.028, 3.24), ("F2", 41, 0.032, 2.86), ("C2", 36, 0.043, 3.659)]
RHO_KERN, RHO_CU = 1140.0, 8960.0     # Annahmen: Nylon-Faserkern (Polyamid 6.6), Kupfer (Silberauflage vernachlaessigt)
FUELL = 0.80                           # Annahme: Fuellgrad des Faserkerns (Fasern mit Luft dazwischen)
for name, t, d_in, kg in UMSP:
    D = d_in * 25.4e-3
    f = hz(t)
    for L in (0.585, 0.60, 0.615):
        T = kg * 9.80665
        mu = T / (4 * L * L * f * f)                 # Gl. 3, nach mu umgestellt
        rho_eff = mu / (math.pi * D * D / 4)
        # Aufbau: ein Lagen-Rundlitze (Draht w) dicht an dicht um den Kern d_c, D = d_c + 2 w.
        # Masse je Laenge: Kern rho_k*FUELL*pi d_c^2/4 + Draht rho_cu * (pi w^2/4) * pi (d_c + w) / w   [hergeleitet]
        best = None
        for i in range(1, 2000):
            w = D / 2 * i / 2000
            dc = D - 2 * w
            if dc <= 0:
                break
            m = RHO_KERN * FUELL * math.pi * dc * dc / 4 + RHO_CU * math.pi * w * w / 4 * math.pi * (dc + w) / w
            if best is None or abs(m - mu) < abs(best[2] - mu):
                best = (w, dc, m)
        w, dc, m = best
        sigma_k = T / (math.pi * dc * dc / 4 * FUELL) / 1e9
        E_k = (4.5 + 39 * sigma_k) * 1e9            # Gl. 16 fuer den Kern - UEBERTRAGUNG auf Faserkern, obere Grenze
        EI_massiv = E_k * math.pi * dc ** 4 / 64
        lam1 = EI_massiv * (math.pi / L) ** 2 / (T + EI_massiv * (math.pi / L) ** 2)
        lam1_g = 50e-6 * (math.pi / L) ** 2 / (T + 50e-6 * (math.pi / L) ** 2)
        if L == 0.60:
            print(f"{name} f {f:6.1f}  D {D * 1e3:.3f} mm  T {T:5.1f} N  mu {mu * 1e3:.3f} g/m  rho_eff {rho_eff:5.0f} kg/m3  "
                  f"Aufbau: Draht w {w * 1e3:.3f} mm, Kern {dc * 1e3:.3f} mm; Kern-Spannung {sigma_k * 1e3:.0f} MPa; "
                  f"EI Kern massiv {EI_massiv * 1e6:.1f}e-6 (lam1 {lam1:.1e}) gegen uebertragen 50e-6 (lam1 {lam1_g:.1e}); Z0 {math.sqrt(T * mu):.3f}")
        else:
            print(f"   L {L:.3f}: mu {mu * 1e3:.3f} g/m, rho_eff {rho_eff:5.0f}")

print()
print("=== Valette (Woodhouse u. a. 2012 Gl. 4) mit den uebertragenen Werten, Grundton und 10. Teilton, L = 0,60")
for name, t, d_in, kg in UMSP:
    T = kg * 9.80665
    f = hz(t)
    for n in (1, 10):
        w_ = 2 * math.pi * n * f
        kk = n * math.pi / 0.60
        eta = (T * (5e-5 + 1.1 / w_) + 50e-6 * 2.0e-2 * kk * kk) / (T + 50e-6 * kk * kk)
        print(f"{name} n={n:2d} f={n * f:7.1f}  eta {eta:.2e}  (Q {1 / eta:5.0f}; Abklingzeit T60 {6.91 / (eta * math.pi * n * f):5.2f} s ohne Koerper)")
