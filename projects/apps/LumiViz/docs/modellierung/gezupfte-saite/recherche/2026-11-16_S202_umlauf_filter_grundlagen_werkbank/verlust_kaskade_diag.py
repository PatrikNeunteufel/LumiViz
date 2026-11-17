"""Diagnose zu verlust_kaskade.py an Gitarre Saite 3 Bund 0: Ziel -ln G je Teilton, Anpassung, Fehler je Teilton;
dazu ein dichter Neustart der Anpassung (Raster der Startwerte). Aufruf: python -B verlust_kaskade_diag.py"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np
import scipy.optimize as opt

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ue", HIER / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
FS = ue.FS
titel, sa, L = ue.tasten()[1]
Gt, ph = ue.ziel(sa, L)
fn, se = ue.teiltoene(Gt, ph)
sel = se <= 100
x = 2 * np.pi * fn[sel] / FS
z = -np.log(np.interp(fn[sel], ue.F, Gt))
print(titel, "Ziel -ln G an Teilton 1, 2, 4, 8, 16, 32:", [f"{z[i]:.2e}" for i in (0, 1, 3, 7, 15, 31) if i < len(z)])
print("Verhaeltnis zu Teilton 1:", [f"{z[i] / z[0]:.2f}" for i in (0, 1, 3, 7, 15, 31) if i < len(z)])


def mlog(g, a, xx):
    return -np.log(g) + sum(0.5 * np.log((1 + 2 * am * np.cos(xx) + am * am) / (1 + am) ** 2) for am in a)


for M in (1, 2, 3):
    best = None
    for start in np.random.default_rng(1).uniform(-0.999, 0.0, size=(300, M)):
        def res(p):
            a = np.clip(p[1:], -0.999999, 0.0)
            g = np.exp(-abs(p[0]))
            return mlog(g, a, x) / z - 1
        r = opt.least_squares(res, np.r_[z[0] * 0.5, start], bounds=(np.r_[0, [-0.999999] * M], np.r_[1.0, [0.0] * M]))
        if best is None or r.cost < best.cost:
            best = r
    e = res(best.x) * 100
    print(f"M {M}: -ln g {best.x[0]:.2e}, a {np.round(best.x[1:], 4)}, Fehler % Teilton 1-12 max {np.abs(e[:12]).max():.1f}, alle hoerbaren max {np.abs(e).max():.1f}")
    print("   je Teilton 1-12:", " ".join(f"{v:+.0f}" for v in e[:12]))
