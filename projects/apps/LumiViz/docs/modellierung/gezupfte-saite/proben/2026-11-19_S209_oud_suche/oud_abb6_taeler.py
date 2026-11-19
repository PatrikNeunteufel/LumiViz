"""S209 - Taeler und Verlauf der gemessenen Kurve (Abdallah 2026 Abb. 6, blau; in S196 aus dem Bild gelesen)."""
import numpy as np

m = np.loadtxt("projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-13_S196_oud_abb6_kurve.txt")
f, a = m[:, 0], m[:, 1]
for lo, hi in ((60, 100), (102, 138), (140, 200), (212, 279), (282, 360)):
    s = (f > lo) & (f < hi)
    i, j = np.argmin(a[s]), np.argmax(a[s])
    print(f"{lo}-{hi} Hz: kleinster Wert {a[s][i]:.2f} bei {f[s][i]:.1f} Hz, groesster {a[s][j]:.2f} bei {f[s][j]:.1f} Hz")
print(" ".join(f"{x}:{a[np.argmin(abs(f - x))]:.2f}" for x in (80, 90, 96, 99, 101, 103, 106, 110, 120, 130, 136, 139, 142, 146, 150, 155, 160, 170, 180, 190, 203, 230, 242, 260, 280, 300, 330, 362)))
