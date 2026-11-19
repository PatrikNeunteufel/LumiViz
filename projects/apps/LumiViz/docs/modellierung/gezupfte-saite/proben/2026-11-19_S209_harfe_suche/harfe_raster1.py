"""S209 - Harfe: Raster Dicke am Bass x Muendungskorrektur x Bassende (gestuetzt / eingespannt), Dicke am Diskant 2 mm;
je Fassung ohne Saiten und mit Saiten (c = 0,2). Eine Zeile je Fassung.
Aufruf: python -X utf8 -B harfe_raster1.py"""
import copy
import json

import harfe_abtasten2 as a2
import harfe_saiten as sa
import harfe_suche as s


def zeile(name, e):
    ml = e["mitluft"]
    ab = e["abw"]
    return (f"{name:34s} | {ml[0][0]:5.1f} ({ml[0][1]:.2f}) {ml[1][0]:5.1f} ({ml[1][1]:.2f}) | "
            + " ".join(f"{100 * (v / s.S1N[n] - 1):+5.1f}" for n, v in e["lagen"].items()) + " | "
            + " ".join(f"{ab[k]:+5.1f}" for k in sorted(ab, reverse=True)) + f" | {sum(abs(v) <= 6 for v in ab.values()):2d}/12 max {max(abs(v) for v in ab.values()):4.1f}")


def beide(plan, name, c=0.2):
    e = s.rechne(plan, name, zeigen=False)
    if e is None:
        return
    print(zeile(name, e), flush=True)
    print(zeile(name + f" + Saiten {c}", s.messe(sa.besaitet(e["k"], c), plan, zeigen=False)), flush=True)


if __name__ == "__main__":
    import sys
    sys.argv = sys.argv[:1]
    print("Fassung | Moden mit Luft (Anteil) | Lage S(1,2)..S(1,6) % | A Saite 33..22 Modell - Bell dB | halten")
    B = s.basis()
    for bass in (0.012, 0.013, 0.014):
        for mu in (1.7, 2.0, 2.3):
            for fest in (False, True):
                p = a2.dicke(B, bass, 0.002)
                p["cavity"]["endCorrection"] = mu
                if fest:
                    p = s.bassende(p, 1e12, 1e7)
                beide(p, f"d{bass * 1000:.0f}-2 mu{mu} {'bass fest' if fest else 'bass gest.'}")
