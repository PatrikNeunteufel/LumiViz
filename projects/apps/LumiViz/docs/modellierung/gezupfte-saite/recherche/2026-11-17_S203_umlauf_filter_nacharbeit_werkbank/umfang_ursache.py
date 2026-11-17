"""LVR Auftrag 21: an den Tasten des ganzen Umfangs, an denen Weg 1 (voll) ueber 1 liegt: wo liegt das Maximum, und
verschwindet es ohne HF-Vorgabe / ohne Glaettung / mit Steiglitz-McBride? Liest die Faelle aus weg2_umfang.txt.
Aufruf: python -B umfang_ursache.py -> umfang_ursache.txt
"""
import sys
sys.dont_write_bytecode = True
import pathlib

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import bank_voll as bv  # noqa: E402
import weg2  # noqa: E402

V = bv.VOLL
VAR = [("voll", V), ("ohne HF", dict(V, hf=False)), ("ohne Glaettung", dict(V, glatt=False)), ("voll SM", dict(V, loeser="sm"))]

if __name__ == "__main__":
    faelle = []
    for z in (pathlib.Path(__file__).parent / "weg2_umfang.txt").read_text(encoding="utf-8").splitlines()[2:]:
        t = [s.strip() for s in z.split("|")]
        if "zu wenig" in t[2]:
            continue
        if float(t[2]) > 1:
            faelle.append((" ".join(t[0].split()), int(t[1])))
    tasten = {f"{i} {n}": (sa, L) for i, n, sa, L in weg2.tasten_umfang()}
    sys.stdout = bv.Tee("umfang_ursache.txt")
    print("Fall | je Variante: max|H| (bei Hz)")
    for name, N in faelle:
        sa, L = tasten[name]
        Gt, ph = bv.ue.ziel(sa, L)
        fn, sg = bv.ue.teiltoene(Gt, ph)
        s = []
        for vn, o in VAR:
            fk, g, Hd, gu, info = bv.vorgabe(Gt, fn, sg, o)
            b, a, _ = bv.wls(fk, g, Hd, N, o["loeser"])
            m, f = bv.maxbetrag(b, a)
            s.append(f"{vn} {m:.6f} ({f:.0f})")
        print(f"{name:20s} iir{N} | " + " | ".join(s), flush=True)
