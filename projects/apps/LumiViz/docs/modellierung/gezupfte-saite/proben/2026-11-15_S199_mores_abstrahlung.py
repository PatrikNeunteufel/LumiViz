"""Mores 2021: erklaert der Modensatz des Stegs (aus 2026-11-15_S199_mores_fit.py --steg --biegung) auch die drei Nahfeld-Mikrofone?

Aufruf: python mores_abstrahlung.py <mat> <gitarre> <stamm der npz> [--bild]

Modell je Mikrofon m und Schlagort h: P/F = sum_j phi_j(x_h) G_jm (-w^2) / (w_j^2 - w^2 + 2 i s_j w) + Rest,
phi_j(x_h) = a + b x + c x^2 fest aus dem Stegfit, G_jm komplex frei (die Form von `antwortSpektrumExakt`).
Ein Satz G je Mikrofon muss alle drei Schlagorte zugleich treffen — das prueft die Moden, nicht nur die Kurve.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np

hier = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("mores_fit", hier / "2026-11-15_S199_mores_fit.py")
mf = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mf)

pfad, g, stamm = sys.argv[1], int(sys.argv[2]), sys.argv[3]
z = np.load(f"{stamm}_steg_g{g:02d}.npz")
f, s, a, b, c = z["f"], z["s"], z["a"], z["b"], z["c"]
hz, H = mf.frfs(mf.lade(pfad, g))
sel = (hz >= mf.VON) & (hz <= mf.BIS)
w = 2 * np.pi * hz[sel]
B = mf.basis(hz[sel], f, s) * (1j * w[:, None])          # i w * Mobilitaetsbasis = -w^2 / (...)
orte = mf.ORTE
phi = np.array([a + b * mf.X_SCHLAG[o] + c * mf.X_SCHLAG[o] ** 2 for o in orte])   # (3, J)
aus = {"gitarre": g}
kurven = {}
for mik in ("mbass", "mtreble", "east"):
    Y = np.concatenate([H[(o, mik)][sel] for o in orte])
    gew = 1.0 / np.maximum(np.abs(Y), 1e-9)
    A = np.concatenate([B * phi[k][None, :] for k in range(3)])
    # Rest je Schlagort: konstanter und 1/w^2-Term
    n = B.shape[0]
    rest = np.zeros((3 * n, 6), dtype=complex)
    for k in range(3):
        rest[k * n:(k + 1) * n, 2 * k] = 1.0
        rest[k * n:(k + 1) * n, 2 * k + 1] = 1e4 / w ** 2
    M = np.concatenate([A, rest], axis=1) * gew[:, None]
    x = np.linalg.lstsq(M, Y * gew, rcond=None)[0]
    fit = np.concatenate([A, rest], axis=1) @ x
    d = 20 * np.log10(np.abs(fit) / np.abs(Y))
    hzz = np.tile(hz[sel], 3)
    aus[mik] = {name: round(float(np.sqrt(np.mean(d[(hzz >= lo) & (hzz < hi)] ** 2))), 2)
                for name, lo, hi in (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000))}
    kurven[mik] = (Y.reshape(3, n), fit.reshape(3, n))
print(json.dumps(aus))
if "--bild" in sys.argv:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, achsen = plt.subplots(3, 3, figsize=(16, 9), sharex=True, sharey=True)
    for i, mik in enumerate(kurven):
        for k, o in enumerate(orte):
            ax = achsen[i, k]
            ax.loglog(hz[sel], np.abs(kurven[mik][0][k]), "k-", lw=0.7)
            ax.loglog(hz[sel], np.abs(kurven[mik][1][k]), "r-", lw=0.7)
            ax.set_title(f"g{g:02d} mic {mik} / hit {o}  (Pa/N)", fontsize=9)
            ax.grid(True, which="both", alpha=0.3)
            ax.set_ylim(1e-3, 3)
    fig.tight_layout()
    fig.savefig(f"{stamm}_abstrahlung_g{g:02d}.png", dpi=75)
