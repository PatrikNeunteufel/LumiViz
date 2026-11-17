"""Sprungmasse der Laute aus der Pruefstand-Ausgabe S199; Luftreibung umsponnener Saiten nach Stokes (W&LA Gl. 11-12)
gegen die Uebertragung (Valette, eta_A = 1,1 1/s). Aufruf: python -I <dieses Skript>"""
import math

import numpy as np

LAUTE = [-16.7, -16.4, -15.8, -16.7, -16.4, -16.5, -15.0, -13.2, -10.0, -8.7, -8.4, -7.9, -9.0, -9.8, -10.9, -15.1, -16.2, -15.2,
         -12.0, -12.2, -7.6, -7.0, -5.0, -4.1, -7.6, -8.1, -7.9, -15.9, -16.2, -18.0, -17.3, -14.8, -11.1, -15.2, -15.8, -12.5,
         -15.7, -16.8, -13.5, -13.4, -16.1, -17.3]          # Tasten 38-79, 2026-11-15_S199_pruefstand_laute_je_taste.txt
l = np.array(LAUTE)
d1 = np.abs(np.diff(l))
d3 = np.abs(l[3:] - l[:-3])
paare = [abs(l[i + k] - l[i]) for k in range(1, 5) for i in range(len(l) - k)]
print(f"Laute: Spanne {l.max() - l.min():.1f}; Nachbarn max {d1.max():.1f} Median {np.median(d1):.1f} Mittel {d1.mean():.1f}; "
      f"Terz max {d3.max():.1f} Median {np.median(d3):.1f}; Paare <= 4 HT max {max(paare):.1f} Mittel {np.mean(paare):.1f} Median {np.median(paare):.1f}")

print()
print("Umsponnene Oud-Saiten: Luftreibung Stokes am Aussendurchmesser (rho = mu / Querschnitt) gegen Valette-Uebertragung")
UMSP = [("D3", 146.83, 0.610, 1.274e-3), ("A2", 110.0, 0.711, 1.824e-3), ("F2", 87.31, 0.813, 2.555e-3), ("C2", 65.41, 1.092, 5.825e-3)]
for name, f, D_mm, mu in UMSP:
    D = D_mm / 1000
    rho = mu / (math.pi * D * D / 4)
    for n in (1, 10):
        fn = n * f
        M = D / 4 * math.sqrt(2 * math.pi * fn / 1.5e-5)
        stokes = 1.2 / rho * (2 * math.sqrt(2) * M + 1) / M ** 2
        valette = 5e-5 + 1.1 / (2 * math.pi * fn)
        print(f"   {name} n={n:2d}: Stokes {stokes:.2e} (mit Zuschlag d+0,2: {stokes * (D_mm + 0.2):.2e})   Valette eta_F + eta_A/w: {valette:.2e}")
