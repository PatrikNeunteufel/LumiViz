"""Energieanteil der Teiltoene mit Knoten am Finger (k Vielfaches von n) bei einem Dreieck-Anschlag an beta.

Modenenergie E_k ~ sin^2(k pi beta) / k^2 (ideale Saite, Dreieck, Auslenkung a_k ~ sin(k pi beta)/k^2, E_k ~ k^2 a_k^2).
Ausgegeben: 10 lg(Summe k=n,2n,.. / Summe alle) in dB, bis k = 200.
"""
import numpy as np

k = np.arange(1, 201)
print("| beta (Anschlag vom Steg) | n=2 | n=3 | n=4 | n=5 |")
print("|---|---|---|---|---|")
for beta in (0.1, 0.15, 0.2, 0.25):
    e = np.sin(k * np.pi * beta) ** 2 / k ** 2
    z = [10 * np.log10(e[k % n == 0].sum() / e.sum()) for n in (2, 3, 4, 5)]
    print(f"| {beta} | " + " | ".join(f"{v:.1f} dB" for v in z) + " |")
