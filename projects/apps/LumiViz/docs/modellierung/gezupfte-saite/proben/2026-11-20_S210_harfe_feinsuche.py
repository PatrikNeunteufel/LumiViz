"""S210 - Harfe: Feinsuche nach KQ50 a. Boden 10 -> 3 mm, Holz aus S197, Bassende gestuetzt; Drehfeder an den Seiten
vom Bassende bis y = `bis` (Randabschnitt, Auslenkung gehalten) x Wert der Feder x Muendungskorrektur.
Je Fassung ohne Saiten und mit Saiten c = 0,2; dazu ein Mass: Wurzel aus dem Mittel der Quadrate der sieben Lagen
ohne Saiten (zwei Moden mit Luft gegen 170 / 216 Hz, S(1,2)-S(1,6)), in Prozent, und die groesste.

Aufruf vom Projekt-Root:
    python -X utf8 -B <diese datei> raster [bis=0.35,0.6,0.85,1.1] [dreh=5e3,1e4,2e4,1e5] [mu=2.0,2.3,2.6]
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("kg", HIER / "2026-11-20_S210_harfe_kandidat_geeicht.py")
_argv, sys.argv = sys.argv, sys.argv[:1]
kg = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kg)
a2, r1, s, sa = kg.a2, kg.r1, kg.s, kg.r1.sa


def fassung(B, bis, dreh, mu):
    p = a2.bassseiten(B, bis, dreh) if dreh > 0 else B
    p = __import__("copy").deepcopy(p)
    p["cavity"]["endCorrection"] = mu
    return p


def lagen(e):
    ml = e["mitluft"]
    v = [100 * (ml[0][0] / 170 - 1), 100 * (ml[1][0] / 216 - 1)] + [100 * (f / s.S1N[n] - 1) for n, f in e["lagen"].items()]
    return np.array(v)


def liste(args, name, vorgabe):
    for a in args:
        if a.startswith(name + "="):
            return [float(v) for v in a.split("=")[1].split(",")]
    return vorgabe


if __name__ == "__main__":
    args = _argv[2:]
    B = s.basis()
    print("Fassung | Moden mit Luft (Anteil) | Lage S(1,2)..S(1,6) % | A Saite 33..22 Modell - Bell dB | halten || Mass, groesste Lage")
    for mu in liste(args, "mu", [2.0, 2.3, 2.6]):
        for bis in liste(args, "bis", [0.35, 0.6, 0.85, 1.1]):
            for dreh in liste(args, "dreh", [5e3, 1e4, 2e4, 1e5]):
                name = f"mu{mu} bis{bis} dreh{dreh:g}"
                e = s.rechne(fassung(B, bis, dreh, mu), name, zeigen=False)
                if e is None:
                    continue
                if len(e["mitluft"]) < 2:
                    print(f"{name:34s} | weniger als zwei Moden mit Luftanteil ueber 0,08", flush=True)
                    continue
                v = lagen(e)
                print(r1.zeile(name, e) + f" || {np.sqrt(np.mean(v ** 2)):.2f} {np.abs(v).max():.1f}", flush=True)
                es = s.messe(sa.besaitet(e["k"], 0.2), fassung(B, bis, dreh, mu), zeigen=False)
                print(r1.zeile(name + " + Saiten 0.2", es), flush=True)
