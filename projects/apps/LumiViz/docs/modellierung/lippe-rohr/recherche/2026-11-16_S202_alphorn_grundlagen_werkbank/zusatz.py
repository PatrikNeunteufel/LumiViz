"""Zusatz: V1 mit den Schablonenlaengen aus Abb. 3a (L2 = 940, L3 = 825 mm, Nachpruefung 1.2, [abgelesen])
statt Tab. 3 (844,91 / 754,84) - als Hypothese, warum Tab. 3 + Becher kuerzer ist als ein Fis-Horn."""
import numpy as np
import tmm
import rechnung as R

R.TAB3["H3S"] = ([539.92, 940.0, 825.0, 444.94], [16.0, 28.5, 44.0])
R.TAB1_MEAS["H3S"] = R.TAB1_MEAS["H3"]
print("| Variante | Länge (mm) | f2 | Cent f2 (A) | Mittel 2–16 (A) | Spanne 3–16 (A) | Tab.1 n4..n10 | Mittel Tab.1 |")
print("|---|---|---|---|---|---|---|---|")
for mp in ("a", "d"):
    seg, lu, g = R.bohrung("V1", "H3S", mp)
    sp = R.rechne(seg, lu)
    f, n, cA, cB, sel = R.kennzahlen(sp)
    t1 = [tmm.cent(f[k - 1], R.TAB1_MEAS["H3"][k - 4]) for k in range(4, 11)]
    print(f"| V1 Schablone MP-{mp} | {g:.0f} | {f[1]:.2f} | {cA[1]:+.0f} | {np.mean(cA[sel]):+.0f} | "
          f"{np.max(cA[2:16]) - np.min(cA[2:16]):.0f} | " + " ".join(f"{x:+.0f}" for x in t1) + f" | {np.mean(t1):+.0f} |")
