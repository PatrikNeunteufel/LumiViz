"""Diagnose Laute Chor 1.1 / 8.1: je Teilton Lage und Abklingen. Aufruf: laute_diag.py <proben> <Chorname> <Lagen> [T5]"""
import sys
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np

P = pathlib.Path(sys.argv[1])
spec = importlib.util.spec_from_file_location("gs", P / "2026-11-18_S205_gebunden_stuetzstellen.py")
gs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gs)
bq, ue, uu = gs.bq, gs.ue, gs.uu
FS, F, W = gs.FS, gs.F, gs.W
chor = sys.argv[2]
lagen = [float(x) for x in sys.argv[3].split(",")]
g = ("T", int(sys.argv[4][1:])) if len(sys.argv) > 4 else ("T", 5)
name, sa, mens, nb = [x for x in gs.saiten("Laute") if x[0] == "Chor " + chor][0]
print(name, {k: v for k, v in sa.items() if not hasattr(v, "__len__") or isinstance(v, str)})


def genau(s, r, th):
    hd = ue.biquads(-r, np.cos(th))
    rest2 = s["rest"] - ue.phase_von(hd)
    f1 = s["fn"][0]
    w1 = 2 * np.pi * f1 / FS
    tot = np.interp(f1, F, rest2)
    lo = gs.unten(g)
    Nint = int(np.floor(tot / w1 - lo))
    soll = tot - w1 * Nint
    Dl = tot / w1 - Nint
    pf = lambda D: ue.phase_von(ue.antwort_ba(*gs.glied(g, D), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    hf = ue.antwort_ba(*gs.glied(g, Dl))
    return uu.je_teilton(s["Gt"], s["ph"], np.abs(s["hl"] * hd * hf), ue.phase_von(s["hl"] * hd * hf) + W * Nint), Nint, Dl


bund = [gs.stelle(sa, mens, float(k), nb, None) for k in range(nb + 1)]
halb = [gs.stelle(sa, mens, k + 0.5, nb, bund) for k in range(nb)]
st = sorted(bund + halb, key=lambda s: s["u"])
print("Ordnung Verlust je Bund:", [len(s["b"]) - 1 for s in bund])
print("Teiltoene unter 0,45 fs / zaehlend je Bund:", [(int((s["fn"] < 0.45 * FS).sum()), len(s["fz"])) for s in bund])
r0, th0 = bq.start(bund[0]["rest"], min(bund[0]["fz"][-1] * 1.02, 19500.0), 8)
f, runden = gs.gebunden(st, 8, 3, r0, th0, g)
for u in lagen:
    s = gs.stelle(sa, mens, u, nb, None if u == int(u) else bund)
    r, th = f(s["x"])
    (fe, se, c, p), Nint, Dl = genau(s, r, th)
    pv = uu.p_verlust(s["b"], s["a"], s["Gt"], fe)
    print(f"\nLage {u}: M {Nint}, Glied {Dl:.3f}; Radien {np.round(r, 3)}; Winkel Hz {np.round(th * FS / 2 / np.pi)}")
    print("  n | f Hz | Rate 1/s | zaehlt | Cent | Abklingen % | Abklingen nur Verlustfilter %")
    for i in range(len(fe)):
        if se[i] <= 3 * ue.SIG_REL:
            print(f"  {i + 1:3d} | {fe[i]:8.1f} | {se[i]:7.1f} | {'ja' if se[i] <= ue.SIG_REL else '  '} | {c[i]:+7.2f} | {p[i]:+7.1f} | {pv[i]:+7.1f}")
