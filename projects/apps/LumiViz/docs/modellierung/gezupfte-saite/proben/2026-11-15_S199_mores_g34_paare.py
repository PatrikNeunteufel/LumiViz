"""KQ23 a (S199): sind die dichten Polpaare an A0 und T1 der Gitarre g34 echte Moden oder ein geteilter Pol?

Aufruf: python 2026-11-15_S199_mores_g34_paare.py <qualified_selected_impulses.mat> [gitarre=34]

Vergleicht den Fit mit den Paaren gegen einen Fit, in dem jedes Paar (Abstand kleiner als die mittlere Breite, unter
300 Hz) durch einen Pol ersetzt ist; Frequenz und Daempfung des einen Pols werden nachgestellt (Nelder-Mead auf den
Fehler des freien Fits im Band). Fehler = Effektivwert von 20 lg |Fit / Messung| ueber die sechs Stegkurven, im Band
85-240 Hz und in 60-400 Hz, frei und mit dem Steg als Stab mit Biegung.
"""
import importlib.util
import pathlib
import sys

import numpy as np
from scipy.optimize import minimize

hier = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("mores_fit", hier / "2026-11-15_S199_mores_fit.py")
mf = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mf)

pfad = sys.argv[1]
g = int(sys.argv[2]) if len(sys.argv) > 2 else 34
sys.argv.append("--biegung")


def fehler(e, fitk, von, bis):
    hz, sel = e["hz"], e["sel"]
    m = (hz[sel] >= von) & (hz[sel] < bis)
    d = [20 * np.log10(np.abs(yf[m]) / np.abs(y[sel][m])) for y, yf in zip(e["mob"], fitk)]
    return float(np.sqrt(np.mean(np.concatenate(d) ** 2)))


def frei(e, f, s):
    """Freie Residuen zu gegebenen Polen -> e mit fit, R, rest."""
    aus = dict(e)
    fitk, Rk = [], []
    for y in e["mob"]:
        x, yf = mf.residuen(e["hz"], y, f, s, e["sel"])
        Rk.append(x)
        fitk.append(yf)
    aus.update(f=f, s=s, fit=fitk, R=np.array(Rk)[:, :len(f)], rest=np.array(Rk)[:, len(f):])
    return aus


e2 = mf.fit(pfad, g)
f, s = e2["f"], e2["s"]
paare = [i for i in range(len(f) - 1) if f[i + 1] < 300 and (f[i + 1] - f[i]) < 0.5 * (s[i] + s[i + 1]) / np.pi]
print(f"g{g:02d}: {len(f)} Moden; Paare unter 300 Hz:",
      [(round(float(f[i]), 1), round(float(f[i + 1]), 1)) for i in paare])

# ein Pol je Paar, f und s nachgestellt
behalten = np.ones(len(f), dtype=bool)
for i in paare:
    behalten[i + 1] = False
f1, s1 = f[behalten].copy(), s[behalten].copy()
stellen = [int(np.sum(behalten[:i])) for i in paare]
for k, i in zip(stellen, paare):
    f1[k], s1[k] = 0.5 * (f[i] + f[i + 1]), 0.5 * (s[i] + s[i + 1])


def ziel(p):
    ff, ss = f1.copy(), s1.copy()
    for n, k in enumerate(stellen):
        ff[k], ss[k] = p[2 * n], abs(p[2 * n + 1])
    return fehler(e2, frei(e2, ff, ss)["fit"], 85, 240)


if stellen:
    p0 = np.concatenate([[f1[k], s1[k]] for k in stellen])
    los = minimize(ziel, p0, method="Nelder-Mead", options={"xatol": 0.01, "fatol": 1e-3, "maxiter": 400})
    for n, k in enumerate(stellen):
        f1[k], s1[k] = los.x[2 * n], abs(los.x[2 * n + 1])
        print(f"   ein Pol: {f1[k]:.1f} Hz, Q {np.pi * f1[k] / s1[k]:.1f}")
e1 = frei(e2, f1, s1)
e2 = frei(e2, f, s)

print("\nFehler dB                 85-240 Hz   60-400 Hz")
print(f"frei, mit Paaren          {fehler(e2, e2['fit'], 85, 240):6.2f}     {fehler(e2, e2['fit'], 60, 400):6.2f}")
print(f"frei, ein Pol je Paar     {fehler(e1, e1['fit'], 85, 240):6.2f}     {fehler(e1, e1['fit'], 60, 400):6.2f}")
st2, st1 = mf.stegfit(e2), mf.stegfit(e1)
print(f"Steg, mit Paaren          {fehler(e2, st2['fit'], 85, 240):6.2f}     {fehler(e2, st2['fit'], 60, 400):6.2f}")
print(f"Steg, ein Pol je Paar     {fehler(e1, st1['fit'], 85, 240):6.2f}     {fehler(e1, st1['fit'], 60, 400):6.2f}")
for name, e, st in (("mit Paaren", e2, st2), ("ein Pol", e1, st1)):
    m = (e["f"] > 85) & (e["f"] < 240)
    x6 = mf.X_SAITEN[0]
    phi = st["a"] + st["b"] * x6 + st["c"] * x6 ** 2
    print(f"{name}: wirksame Masse an Saite 6 ueber 85-240 Hz {1 / np.sum(phi[m] ** 2):.3f} kg; Moden",
          [(round(float(a), 1), round(float(np.pi * a / b), 1), round(float(1 / p ** 2), 3)) for a, b, p in zip(e["f"][m], e["s"][m], phi[m])])
np.savez(f"g{g:02d}_ein_pol.npz", f=e1["f"], s=e1["s"], a=st1["a"], b=st1["b"], c=st1["c"])
