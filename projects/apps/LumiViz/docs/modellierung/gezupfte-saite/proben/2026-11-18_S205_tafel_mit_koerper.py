"""S205 - die Tafel je Taste nach UL3-1 a (Thiran 5, `2026-11-18_S205_tafel_je_taste.py`) im Pruefstand MIT Koerper.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> cembalo|harfe [tasten=64,71,76,78]

Rahmen und Messung wie `2026-11-17_S204_umlauf_mit_koerper.py` (gegen den genauen Umlauf: Lautheit LU | Pegel 1-2 s dB |
Teiltoene 1-12 Cent (Grundton) | Abfall frueh->spaet dB). Verglichen:
  UL3    die Fassung aus S204 (Thiran 1, einzeln angepasst)
  UL31   die Tafel aus S205 (Thiran 5 mit in der Anpassung, bis zur 1,5-fachen Grenze, mehrere Starts)
Mit `einzeln` als weiterem Argument: fuer UL31 je Teilton 1-12 Lage und Abfall gegen den genauen Umlauf.
Schreibt nichts.
"""
import sys
import json
import pathlib
import importlib.util
sys.dont_write_bytecode = True
import numpy as np

HIER = pathlib.Path(__file__).resolve()


def lade(datei, name):
    spec = importlib.util.spec_from_file_location(name, HIER.with_name(datei))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


um = lade("2026-11-17_S204_umlauf_mit_koerper.py", "um")
tj = lade("2026-11-18_S205_tafel_je_taste.py", "tj")
s200 = um.s200
FS = um.FS
_baue_s204 = um.baue


def baue(fassung, saite, L, f0):
    if fassung != "UL31":
        um.bq.RMAX = 0.98
        return _baue_s204(fassung, saite, L, f0)
    tj.bq.RMAX = 0.99
    e = tj.entwurf(saite, L)

    def E(w):
        zi = np.exp(-1j * w / FS)
        h = np.polyval(e["b"][::-1], zi) / np.polyval(e["a"][::-1], zi)
        for r, t in zip(e["r"], e["th"]):
            al, c = -r, np.cos(t)
            h = h * (al * al + 2 * al * c * zi + zi * zi) / (1 + 2 * al * c * zi + al * al * zi * zi)
        return h * np.polyval(e["bt"][::-1], zi) / np.polyval(e["at"][::-1], zi) * zi ** e["N"]
    E.info = f"T{e['glied'][1]}, {len(e['r'])} Biquads, starr {e['cent']:.2f} Cent / {e['abkl']:.1f} %"
    return E


s200.baue = baue
s200.FASSUNGEN = ("UL3", "UL31")

if __name__ == "__main__":
    ps = s200.ps
    was = sys.argv[1]
    opts = dict(a.split("=") for a in sys.argv[2:] if "=" in a)
    name, fn_, std = (("harpsichord", ps.cembalo_taste, "64,71,76,78") if was == "cembalo" else ("concert_harp", ps.harfe_taste, "48,96,103"))
    tasten = [int(x) for x in opts.get("tasten", std).split(",")]
    kp = ps.lade_koerper(name)
    datei = len(json.loads((ps.KOERPER / f"{name}.lvinstbody").read_text(encoding="utf-8"))["modes"])
    faelle = []
    for t in tasten:
        x = fn_(t)
        if x:
            sa, L, xp, d, winkel, breite, _ = x
            faelle.append((f"Taste {t}", sa, L, xp, d, winkel, breite, kp["tasten"].index(t), ps.hz_von(t)))
    s200.laufe(was.capitalize(), faelle, kp, datei)
    if "einzeln" in sys.argv:
        w = ps.frequenzen()
        tan = ps.richtung_tan(kp["f"])
        nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
        for titel, sa, L, xp, d, winkel, breite, punkt, f0 in faelle:
            Y, _, _ = ps.koerper_kurven2(nur, punkt, w, tan[:datei])
            k, _ = ps.saite_wellen(sa, w)
            t0 = s200.kd.teiltoene(ps.zeit(s200.kraft(sa, L, xp, d, winkel, Y, w, breite, np.exp(-2j * k * L)))[:, 0], f0)
            E = baue("UL31", sa, L, f0)
            t1 = s200.kd.teiltoene(ps.zeit(s200.kraft(sa, L, xp, d, winkel, Y, w, breite, E(w)))[:, 0], f0)
            print(f"\n{titel} UL31 [{E.info}]: Teilton | f Hz | Cent | genau frueh / spaet dB | UL31 frueh / spaet dB | Abfall-Differenz dB")
            for i, (a, b) in enumerate(zip(t1, t0)):
                print(f"  {i + 1:2d} | {b[0]:8.1f} | {1200 * np.log2(a[0] / b[0]):+6.2f} | {b[1]:7.1f} / {b[2]:7.1f} | {a[1]:7.1f} / {a[2]:7.1f} | {(a[1] - a[2]) - (b[1] - b[2]):+6.2f}")
