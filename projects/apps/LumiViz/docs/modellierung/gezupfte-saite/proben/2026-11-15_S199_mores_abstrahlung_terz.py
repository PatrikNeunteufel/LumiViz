"""KQ24 (S199): wie gut tragen Abstrahlgewichte je Mode den Schall an den Nahfeld-Mikrofonen — in Terzbaendern?

Aufruf: python 2026-11-15_S199_mores_abstrahlung_terz.py <mat> <gitarre> <npz mit f, s, a, b, c>

Wie `…_mores_abstrahlung.py` (je Mikrofon ein komplexes Gewicht G je Mode fuer alle drei Schlagorte, phi fest aus dem
Stegfit), aber verglichen wird der Pegel je Terzband (Leistungsmittel) statt Punkt fuer Punkt: Einbrueche zwischen
den Moden zaehlen dann nicht mehr als die Spitzen. Dazu: wie verschieden sind die drei Mikrofone untereinander, und
wie gut traegt ein Gewicht je Mode, das nur aus dem Betrag stammt (|G| aus dem Fit, Phase 0 — „Betrag je Band“).
"""
import importlib.util
import pathlib
import sys

import numpy as np

hier = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("mores_fit", hier / "2026-11-15_S199_mores_fit.py")
mf = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mf)

pfad, g = sys.argv[1], int(sys.argv[2])
z = np.load(sys.argv[3])
f, s, a, b, c = z["f"], z["s"], z["a"], z["b"], z["c"]
hz, H = mf.frfs(mf.lade(pfad, g))
sel = (hz >= mf.VON) & (hz <= mf.BIS)
w = 2 * np.pi * hz[sel]
B = mf.basis(hz[sel], f, s) * (1j * w[:, None])
orte = mf.ORTE
phi = np.array([a + b * mf.X_SCHLAG[o] + c * mf.X_SCHLAG[o] ** 2 for o in orte])
n = B.shape[0]
mitten = [63 * 2 ** (k / 3) for k in range(0, 20)]            # 63 ... 5080 Hz
mitten = [m for m in mitten if m * 2 ** (1 / 6) <= mf.BIS * 1.02]


def terz(y):
    return np.array([10 * np.log10(np.mean(np.abs(y[(hz[sel] >= m / 2 ** (1 / 6)) & (hz[sel] < m * 2 ** (1 / 6))]) ** 2)) for m in mitten])


bereiche = (("63-400", 60, 450), ("500-1000", 450, 1120), ("1250-2500", 1120, 2800), ("3150-5000", 2800, 5700))
mm = np.array(mitten)
print(f"g{g:02d}, {len(f)} Moden; Fehler des Terzpegels in dB (Effektivwert ueber Baender und drei Schlagorte; groesster)")
pegel = {}
for mik in ("mbass", "mtreble", "east"):
    Y = np.concatenate([H[(o, mik)][sel] for o in orte])
    gew = 1.0 / np.maximum(np.abs(Y), 1e-9)
    A = np.concatenate([B * phi[k][None, :] for k in range(3)])
    rest = np.zeros((3 * n, 6), dtype=complex)
    for k in range(3):
        rest[k * n:(k + 1) * n, 2 * k] = 1.0
        rest[k * n:(k + 1) * n, 2 * k + 1] = 1e4 / w ** 2
    M = np.concatenate([A, rest], axis=1)
    x = np.linalg.lstsq(M * gew[:, None], Y * gew, rcond=None)[0]
    fit = (M @ x).reshape(3, n)
    ohne_rest = (A @ x[:len(f)]).reshape(3, n)                   # nur die Moden, wie eine Koerperdatei sie traegt
    mess = Y.reshape(3, n)
    d = np.array([terz(fit[k]) - terz(mess[k]) for k in range(3)])
    d0 = np.array([terz(ohne_rest[k]) - terz(mess[k]) for k in range(3)])
    pegel[mik] = np.array([terz(mess[k]) for k in range(3)])
    zeile = f"  {mik:8s}"
    for name, lo, hi in bereiche:
        m = (mm >= lo) & (mm < hi)
        zeile += f" | {name}: {np.sqrt(np.mean(d[:, m] ** 2)):.1f} ({np.abs(d[:, m]).max():.1f}), nur Moden {np.sqrt(np.mean(d0[:, m] ** 2)):.1f} ({np.abs(d0[:, m]).max():.1f})"
    print(zeile)
    # geteilt: bis `grenze` das Gewicht je Mode aus dem Fit, darueber ein Betrag je Terzband (Vorzeichen gezogen,
    # Samen 1), so gestellt, dass der Terzpegel im Mittel ueber die drei Schlagorte stimmt
    for grenze in (500.0, 1000.0):
        G = x[:len(f)].copy()
        oben = f >= grenze
        rng = np.random.default_rng(1)
        G[oben] = rng.choice([-1.0, 1.0], size=int(oben.sum())) * np.median(np.abs(G[oben]))
        band_von = [bm for bm in mitten if bm * 2 ** (1 / 6) > grenze]
        for _ in range(12):
            syn = np.array([terz((B * phi[k][None, :]) @ G) for k in range(3)])
            for bm in band_von:
                ib = mitten.index(bm)
                jm = oben & (f >= bm / 2 ** (1 / 6)) & (f < bm * 2 ** (1 / 6))
                if jm.any():
                    diff = 10 * np.log10(np.mean(10 ** (pegel[mik][:, ib] / 10)) / np.mean(10 ** (syn[:, ib] / 10)))
                    G[jm] *= 10 ** (np.clip(diff, -6, 6) / 20)
        syn = np.array([terz((B * phi[k][None, :]) @ G) for k in range(3)])
        dg = syn - pegel[mik]
        zeile = f"     geteilt bei {grenze:.0f} Hz"
        for name, lo, hi in bereiche:
            m = (mm >= lo) & (mm < hi)
            zeile += f" | {name}: {np.sqrt(np.mean(dg[:, m] ** 2)):.1f} ({np.abs(dg[:, m]).max():.1f})"
        print(zeile)

print("\nUnterschied der Mikrofone im Terzpegel (Messung, Mittel ueber die Schlagorte), dB:")
for a_, b_ in (("mbass", "mtreble"), ("mbass", "east"), ("mtreble", "east")):
    d = pegel[a_].mean(axis=0) - pegel[b_].mean(axis=0)
    print(f"  {a_} - {b_}: " + ", ".join(f"{name} {d[(mm >= lo) & (mm < hi)].mean():+.1f} (Streuung {d[(mm >= lo) & (mm < hi)].std():.1f})" for name, lo, hi in bereiche))
print("\nTerzpegel der Messung, Mittel ueber Schlagorte (dB re 1 Pa/N):")
print("  Hz      " + " ".join(f"{m:6.0f}" for m in mitten))
for mik in pegel:
    print(f"  {mik:8s}" + " ".join(f"{v:6.1f}" for v in pegel[mik].mean(axis=0)))
