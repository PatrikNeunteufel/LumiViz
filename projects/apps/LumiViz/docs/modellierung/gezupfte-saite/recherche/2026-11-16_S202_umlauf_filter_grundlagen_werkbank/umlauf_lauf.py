"""LVR Auftrag 11, Punkt 2: Lauf ueber die Tasten. Volle Tabellen nach umlauf_entwurf.txt, Uebersicht auf die Konsole.
Aufruf: python -B umlauf_lauf.py"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ue", HIER / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)

DISP = [("ohne", None), ("K8 5k", ("K", 8, 5000.0)), ("K16 5k", ("K", 16, 5000.0)), ("K16 20k", ("K", 16, 20000.0)),
        ("K32 5k", ("K", 32, 5000.0)), ("Rauhala M8", ("rauhala", 8)), ("adf 5k", ("adf", 5000.0))]
VERL = ["pol1", "pol1p", "iir2", "iir4", "iir8"]
FD = [("thiran", 1), ("thiran", 3), ("lagrange", 1), ("lagrange", 3), ("lagrange", 5)]

aus = open(HIER / "umlauf_entwurf.txt", "w", encoding="utf-8")


def p(s=""):
    aus.write(s + "\n")


uebersicht = []
for titel, sa, L in ue.tasten():
    B = np.pi ** 2 * sa["EI"] / (sa["T"] * L * L)
    Gt, ph = ue.ziel(sa, L)
    fn, se = ue.teiltoene(Gt, ph)
    p(f"\n### {titel}: f1 {fn[0]:.2f} Hz, L {L:.3f} m, T {sa['T']:.1f} N, EI {sa['EI']:.3g} N m2, B {B:.3g}, "
      f"Umlauf {ph[np.searchsorted(ue.F, fn[0])] / (2 * np.pi * fn[0] / ue.FS):.1f} Takte, Teiltoene bis 20 kHz {np.sum(fn <= 20000)}, "
      f"davon mit sigma <= 100 1/s {np.sum(se <= 100)} (bis {fn[se <= 100].max():.0f} Hz), sigma_1 {se[0]:.3f} 1/s")
    zeile = {"titel": titel, "f1": fn[0], "B": B, "nr": int(np.sum(se <= 100)), "n": int(np.sum(fn <= 20000))}
    p("  A Dispersion (Verlust pol1p, Thiran 1, nachgestimmt) | K | Cent: Grundton, 1-12, <=5k, <=20k | in +-10 Cent alle | dphi rad <=5k / <=20k | hoerbar: Cent max, in +-10")
    for name, d in DISP:
        o = ue.entwurf(sa, L, "pol1p", d, ("thiran", 1))
        r = ue.bewerte(*o[:4])
        zeile["A " + name] = r["cr"]
        p(f"    {name:11s} | {str(o[4]['K']):>4s} | {r['c1']:+6.2f} {r['c12']:7.1f} {r['c5k']:7.1f} {r['c20']:8.1f} | {r['in10']:3d}/{r['n']:3d} | "
          f"{r['dphi5']:6.2f} / {r['dphi20']:7.2f} | {r['cr']:7.1f} {r['in10r']:3d}/{r['nr']:3d}")
    p("  B Verlust (Dispersion K16 5k, Thiran 1) | max|H_loss| (vor Skalierung) | Abklingen %: Grundton, 1-12, <=5k, <=20k | in 5 % alle | max |d ln|H|| <=20k | hoerbar: % max, in 5 %")
    for v in VERL:
        o = ue.entwurf(sa, L, v, ("K", 16, 5000.0), ("thiran", 1))
        r = ue.bewerte(*o[:4])
        info = o[4]
        kl = (" skaliert" if v.startswith("iir") else " geklemmt") if info.get("geklemmt") else ""
        vorher = info.get("vorher", info["max_loss"])
        zeile["B " + v] = r["pr"]
        zeile["B1 " + v] = r["p1"]
        p(f"    {v:6s} | {vorher:.6f}{kl:9s} | {r['p1']:+7.1f} {r['p12']:7.1f} {r['p5k']:7.1f} {r['p20']:8.1f} | {r['in5']:3d}/{r['n']:3d} | {r['dln20']:.2e} | "
          f"{r['pr']:7.1f} {r['in5r']:3d}/{r['nr']:3d}")
    p("  C Bruchteil-Verzug (Verlust iir4 mit Ausgleich des Betrags, K16 5k) | Grundton Cent ohne / mit Nachstimmen | hoerbar: Cent max, Abklingen % max | Raster-Frequenzen <=20k, an denen der Verlust > 1 sein muesste")
    for fd in FD:
        o0 = ue.entwurf(sa, L, "iir4", ("K", 16, 5000.0), fd, nachstimmen=False)
        r0 = ue.bewerte(*o0[:4])
        o = ue.entwurf(sa, L, "iir4", ("K", 16, 5000.0), fd)
        r = ue.bewerte(*o[:4])
        zeile[f"C {fd[0][0]}{fd[1]}"] = (r0["c1"], r["pr"], o[4]["unmoeglich"])
        p(f"    {fd[0]:8s} {fd[1]} | {r0['c1']:+8.3f} / {r['c1']:+7.3f} | {r['cr']:7.1f} {r['pr']:7.1f} | {o[4]['unmoeglich']}")
    uebersicht.append(zeile)
    aus.flush()
    print(titel, "fertig", flush=True)
aus.close()

print("\nUebersicht (hoerbare Teiltoene, sigma <= 100 1/s): Cent max je Dispersion | Abklingen % max je Verlust | Grundton-Cent ohne Nachstimmen T1/L3/L5")
for z in uebersicht:
    print(f"{z['titel'][:26]:26s} f1 {z['f1']:7.1f} B {z['B']:.2e} n {z['nr']:3d}/{z['n']:3d} | "
          + " ".join(f"{z['A ' + n]:6.1f}" for n, _ in DISP) + " | "
          + " ".join(f"{z['B ' + v]:6.1f}" for v in VERL) + " | "
          + " ".join(f"{z[k][0]:+.2f}/{z[k][1]:.0f}%/{z[k][2]}" for k in ("C t1", "C l3", "C l5")))
