"""Verlustfilter laengs der Saite: Entwurf an Buenden und auf halbem Weg (Schritt 0,5), dazwischen b, a linear.
Aufruf: laute_verlust2.py <proben> <Instrument> <Name der Saite> [schritt=0.5]"""
import sys
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np

P = pathlib.Path(sys.argv[1])
spec = importlib.util.spec_from_file_location("gs", P / "2026-11-18_S205_gebunden_stuetzstellen.py")
gs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gs)
ue, uu = gs.ue, gs.uu
name, sa, mens, nb = [x for x in gs.saiten(sys.argv[2]) if x[0] == sys.argv[3]][0]
dx = float(sys.argv[4]) if len(sys.argv) > 4 else 0.5
ug = np.arange(0, nb + 1e-9, dx)
ent = [gs.stelle(sa, mens, float(u), nb, None) for u in ug]
print(f"{sys.argv[2]} {name}, Entwurf im Abstand {dx} Buende; groesster Polradius je Entwurf:")
print("  " + " ".join(f"{np.abs(np.roots(s['a'])).max():.4f}" for s in ent))
schl, aus, n = 0.0, 0, 0
for u in np.arange(0, nb + 1e-9, 0.0625):
    k = min(int(np.floor(u / dx + 1e-9)), len(ug) - 2)
    t = (u - ug[k]) / dx
    b = (1 - t) * ent[k]["b"] + t * ent[k + 1]["b"]
    a = (1 - t) * ent[k]["a"] + t * ent[k + 1]["a"]
    Gt, ph = ue.ziel(sa, mens * 2 ** (-u / 12))
    fe, se = ue.teiltoene(Gt, ph)
    h = se <= ue.SIG_REL
    pv = uu.p_verlust(b, a, Gt, fe[h])
    n += 1
    draussen = pv.min() < uu.P_LO or pv.max() > uu.P_HI
    aus += draussen
    schl = max(schl, float(np.abs(pv).max()))
    if draussen:
        print(f"  Lage {u:6.3f}: {pv[np.argmax(np.abs(pv))]:+.1f} % (Teilton {int(np.argmax(np.abs(pv))) + 1}), |H| max {np.abs(ue.antwort_ba(b, a)).max():.6f}")
print(f"  groesster Fehler {schl:.1f} %, ausserhalb {aus}/{n}")
