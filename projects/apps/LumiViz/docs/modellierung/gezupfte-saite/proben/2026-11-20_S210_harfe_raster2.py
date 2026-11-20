"""S210 - Harfe: Raster nach der Eichung. Dicke mit ihrem geeichten Holz (10 -> 3 mm / S197, 12 -> 2 mm / S210) x
Bassende (gestuetzt / eingespannt) x Drehfeder an den Seiten bis y = 0,6 m (0 / 1e3 / 3e3 / 1e4 N) x Muendung;
je Fassung ohne Saiten und mit Saiten c = 0,2. Eine Zeile je Fassung (Spalten wie `harfe_raster1.py` aus S209).

Aufruf vom Projekt-Root: python -X utf8 -B <diese datei> [muendung ...]     (Vorgabe 2.0 und der Wert der Basis)
"""
import importlib.util
import pathlib
import sys

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("kg", HIER / "2026-11-20_S210_harfe_kandidat_geeicht.py")
_argv, sys.argv = sys.argv, sys.argv[:1]
kg = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kg)
a2, r1, s = kg.a2, kg.r1, kg.s

if __name__ == "__main__":
    B = s.basis()
    mus = [float(v) for v in _argv[1:]] or [2.0, B["cavity"]["endCorrection"]]
    print("Fassung | Moden mit Luft (Anteil) | Lage S(1,2)..S(1,6) % | A Saite 33..22 Modell - Bell dB | halten")
    for dicke, holz in (((0.010, 0.003), "S197"), ((0.012, 0.002), "geeicht")):
        for mu in mus:
            for fest in (False, True):
                for dreh in (0.0, 1e3, 3e3, 1e4):
                    p = a2.dicke(B, *dicke)
                    p = kg.holz(p, holz)
                    p["cavity"]["endCorrection"] = mu
                    if dreh > 0:
                        p = a2.bassseiten(p, 0.6, dreh)
                    if fest:
                        p = s.bassende(p, 1e12, 1e7)
                    r1.beide(p, f"d{dicke[0] * 1000:.0f}-{dicke[1] * 1000:.0f} mu{mu} {'fest' if fest else 'gest'} dreh{dreh:g}")
