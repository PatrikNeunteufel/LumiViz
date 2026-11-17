"""LVR Auftrag 21, Punkt 1 (Ursache): Varianten, die zwischen Bericht 11 und dem vollstaendigen Verfahren liegen.
  A  'unter f1 Ziel'   : Betrag unter f1 auf dem dichten Raster = wahres Ziel |H_ziel| (wie Bericht 11) statt g_1 gehalten
  B  'Rand ungeglaettet': Glaettung [0,25 0,5 0,25] nur im Innern, erster und letzter Sollpunkt unveraendert
Aufruf: python -B bank_diag.py -> bank_diag.txt
"""
import sys
sys.dont_write_bytecode = True
import pathlib

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import bank_voll as bv  # noqa: E402

V = bv.VOLL
VAR = [("ohne beides, unter f1 Ziel", dict(V, glatt=False, hf=False, unter="ziel")),
       ("ohne beides, unter f1 Ziel, SM", dict(V, glatt=False, hf=False, unter="ziel", loeser="sm")),
       ("voll, unter f1 Ziel", dict(V, unter="ziel")),
       ("voll, Rand ungeglaettet", dict(V, rand="ungeglaettet")),
       ("voll SM, Rand ungeglaettet", dict(V, rand="ungeglaettet", loeser="sm"))]

if __name__ == "__main__":
    sys.stdout = bv.Tee("bank_diag.txt")
    print("Variante, Ordnung | max|H| (bei Hz) | gespiegelt | Abklingen % Grundton, 1-12, <=5k, hoerbar (unskaliert)")
    for titel, sa, L in bv.ue.tasten():
        z = bv.zielform(sa, L)
        print(f"\n### {titel}: Sollpunkte {z['n']}, -ln|H_ziel| f->0 / f1 = {z['y0'] / z['y1']:.3f}", flush=True)
        for vname, o in VAR:
            for N in (2, 4, 8):
                if 2 * N + 1 > 2 * z["n"]:
                    continue
                r, inf, _ = bv.lauf(sa, L, N, o)
                print(f"  {vname:31s} iir{N} | {inf['vorher']:.6f} ({inf['f_max']:6.0f}) | {inf['gespiegelt']} | "
                      f"{r['p1']:+7.2f} {r['p12']:7.2f} {r['p5k']:8.2f} {r['pr']:8.2f}", flush=True)
