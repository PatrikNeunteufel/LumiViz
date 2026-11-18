"""S205 - KQ31 a: Profile der Admittanz laengs der Deckleiste, je Mode der Koerperdatei unter 1 kHz bei deren Frequenz,
an den 46 Oesen (Ort nach Bell Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m) - zum Vergleich mit Bell 1988 Abb. 7.8-7.16
(Zahl und Lage ALLER Baeuche, nicht nur des staerksten). 116 dB = 1 s/kg.

Aufruf: python -X utf8 -B <dieses Skript> <bild.png> [<koerper.lvinstbody>]     (ohne Datei: der mitgelieferte Koerper)
Gibt je Mode den Pegel an Ort 17 und den staerksten Bauch aus; schreibt nur das Bild.
"""
import importlib.util, json, pathlib, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben")
def modul(d, n):
    s = importlib.util.spec_from_file_location(n, P / d); m = importlib.util.module_from_spec(s); s.loader.exec_module(m); return m
ps = modul("2026-11-15_S199_pruefstand_zupf.py", "ps")
ha = modul("2026-11-14_S198_harfe_admittanz.py", "ha")
if len(sys.argv) > 2:
    ps.KOERPER = pathlib.Path(sys.argv[2]).parent
    kp = ps.lade_koerper(pathlib.Path(sys.argv[2]).stem)
else:
    kp = ps.lade_koerper("concert_harp")
orte = {s["name"]: s["ort"] for s in json.load(open(P / "2026-11-14_S198_harfe_saitenorte.json", encoding="utf-8"))["saiten"]}
alle = ha.NYLON_DARM + [x for n in dir(ha) if n.isupper() and n not in ("NYLON_DARM", "LAENGE", "BEZUG", "HIER") for x in getattr(ha, n) if isinstance(getattr(ha, n), list)]
pk, y = [], []
for nr, hz, *_ in alle:
    t = int(round(69 + 12 * np.log2(hz / 440.0))) + 1
    if t in kp["tasten"] and nr in orte:
        pk.append(kp["tasten"].index(t)); y.append(orte[nr])
y = np.array(y); o = 40 - (y + 0.057) / 0.035
i = np.argsort(o); o = o[i]; pk = np.array(pk)[i]
print(len(pk), "Oesen, Ort", o.min(), o.max())
wn = 2 * np.pi * kp["f"]
def Y(hz):
    w = 2 * np.pi * hz
    n = 1.0 / (wn ** 2 * (1 + 1j * kp["eta"]) - w * w)
    return np.array([20 * np.log10(abs(1j * w * np.sum(kp["steg"][:, p] ** 2 * n))) + 116 for p in pk])
fm = [f for f in kp["f"] if 100 < f < 1000]
fig, ax = plt.subplots(3, 4, figsize=(18, 10))
for a, f in zip(ax.flat, fm):
    v = Y(f)
    a.plot(o, v, "r.-"); a.set_xlim(40, 0); a.set_ylim(55, 110); a.grid(True, alpha=0.3); a.axvline(17, color="k", lw=0.5)
    a.set_title(f"Modell {f:.0f} Hz, Q {1 / kp['eta'][list(kp['f']).index(f)]:.0f}")
    print(f"{f:6.1f} Hz: Ort 17 {np.interp(17, o, v):5.1f} dB, max {v.max():5.1f} an Ort {o[v.argmax()]:.0f}")
fig.tight_layout(); fig.savefig(sys.argv[1], dpi=80)
