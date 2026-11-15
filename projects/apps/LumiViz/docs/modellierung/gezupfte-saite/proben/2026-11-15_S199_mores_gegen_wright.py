"""Gegenprobe der tiefen Moden gegen Wright 1996 Tab. 5.4 (Gitarre BR2, Schallloch verschlossen, Antrieb an der
tiefen E-Saite: T(1,1) 165 Hz, wirksame Masse 0,062 kg, Q 36; T(2,1) 204 Hz, 0,201 kg, Q 66).

Bei offenem Schallloch teilt sich T(1,1) in A0 und T1 (und den Anteil des Bodens); die Summe der Residuen
phi^2 = 1/m bleibt erhalten. Verglichen wird deshalb 1 / sum(phi(x6)^2) ueber die Moden 85-240 Hz.
"""
import sys

import numpy as np

X6 = -0.029
for g in sys.argv[2:]:
    z = np.load(f"{sys.argv[1]}_steg_g{int(g):02d}.npz")
    f, s, a, b, c = z["f"], z["s"], z["a"], z["b"], z["c"]
    phi = a + b * X6 + c * X6 ** 2
    q = np.pi * f / s
    print(f"g{int(g):02d}")
    sel = (f > 85) & (f < 300)
    for fj, qj, pj, aj in zip(f[sel], q[sel], phi[sel], a[sel]):
        if pj ** 2 / (2 * np.pi * fj / qj) > 0.002:      # Spitze der Mobilitaet ueber 0,002 m/Ns
            print(f"   {fj:6.1f} Hz  Q {qj:5.1f}  M(Saite 6) {1 / pj ** 2:7.3f} kg  M(Mitte) {1 / aj ** 2 if abs(aj) > 1e-9 else float('inf'):7.3f} kg")
    m = (f > 85) & (f < 240)
    print(f"   Summe 85-240 Hz: M(Saite 6) {1 / np.sum(phi[m] ** 2):.3f} kg, M(Mitte) {1 / np.sum(a[m] ** 2):.3f} kg")
