"""S215 (KQ63 a gegen b) - Probe: Abstrahlung an den Saiten 33-22 und Gueten ohne Saiten aus Koerperdateien.

Wie der Fall „Luftraum die mitgelieferte Harfe“ (KQ60 a): |p/F| an der Oese bei der Frequenz der leeren Saite, Kraft
senkrecht zum Boden, Leistungsmittel ueber die 16 Hoerorte k01-k16 (KQ36 a), die Saiten als Daempfer (0,67 Z0 phi^2 / w,
Z0 aus proben/2026-11-23_S214_harfe_saitendaempfung.py), bezogen auf den Median der zwoelf Abweichungen gegen Bell
Abb. 7.23. Dazu ohne Saiten die Gueten der Moden 150-760 Hz.

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner> ...   (je Ordner concert_harp.lvinstbody mit 17 Hoerorten,
concert_harp.lvinstbodyplan und, falls da, ohne.lvinstbody)
"""
import json
import math
import pathlib
import sys

import numpy as np

Z0 = [5.314, 5.181, 4.773, 4.624, 4.45, 4.417, 3.978, 3.61, 3.482, 3.148, 2.998, 1.314, 1.226, 1.124, 1.089, 1.043, 0.9417,
      0.8947, 0.8468, 0.7965, 0.6993, 0.6455, 0.594, 0.5261, 0.4966, 0.4613, 0.4291, 0.3763, 0.3274, 0.3084, 0.2678, 0.2494,
      0.2291, 0.2143, 0.1932, 0.1848, 0.1533, 0.1552, 0.1563, 0.1451, 0.1361, 0.1298, 0.1111, 0.1154, 0.09827, 0.09551]
BELL = [(33, 104, -26), (32, 116, -24), (31, 123, -31), (30, 139, -32), (29, 155, -34), (28, 165, -28.5), (27, 185, -10),
        (26, 207, -8), (25, 233, -16), (24, 247, -12), (23, 277, -5.5), (22, 311, -8)]


def auswerten(o):
    k = json.loads((o / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
    p = json.loads((o / "concert_harp.lvinstbodyplan").read_text(encoding="utf-8"))
    tasten = k["bridgePoints"]
    orte = [i for i, n in enumerate(k["listeningPoints"]) if n.startswith("k")]
    assert len(orte) == 16
    ys = [b["y"] for b in p["bridgePoints"]]
    oese = []
    for i, y in enumerate(ys):
        if all(ys[j] != y for j in oese):
            oese.append(i)
    oese.sort(key=lambda i: ys[i])
    assert len(oese) == 46
    f = np.array([m["f"] for m in k["modes"]])
    eta = np.array([m["eta"] for m in k["modes"]])
    steg = np.array([m["bridge"] for m in k["modes"]])
    G = np.array([[complex(*g) for g in m["radiation"]] for m in k["modes"]])
    d = 0.67 * (steg[:, oese] ** 2 * np.array(Z0)[None, :]).sum(axis=1) / (2 * math.pi * f)
    eta_d = eta + d

    def pf(taste, hz):
        i = tasten.index(taste)
        om = 2 * math.pi * hz
        omn = 2 * math.pi * f
        h = steg[:, i] * (-om * om) / (omn * omn * (1 + 1j * eta_d) - om * om)
        p_ = (h[:, None] * G[:, orte]).sum(axis=0)
        return 10 * math.log10(np.mean(np.abs(p_) ** 2))

    taste = lambda hz: int(round(69 + 12 * math.log2(hz / 440.0))) + 1
    abw = np.array([pf(taste(hz), hz) - db for _, hz, db in BELL])
    a = abw - np.median(abw)
    halten = int(np.sum(np.abs(a) <= 6))
    print(f"{o.name}: Abstrahlung 33-22 (Median, Saiten als Daempfer): " + " ".join(f"{s}:{x:+.1f}" for (s, _, _), x in zip(BELL, a))
          + f" | halten {halten} / 12, groesste {np.max(np.abs(a)):.1f} dB")
    besaitet = [(m["f"], m["airShare"], 1 / m["eta"]) for m in k["modes"] if 150 < m["f"] < 760]
    print("   besaitet: " + "; ".join(f"{x:.1f} L{l:.2f} Q{q:.0f}" for x, l, q in besaitet))
    if (o / "ohne.lvinstbody").exists():
        ko = json.loads((o / "ohne.lvinstbody").read_text(encoding="utf-8"))
        print("   ohne Saiten: " + "; ".join(f"{m['f']:.1f} L{m['airShare']:.2f} Q{1 / m['eta']:.0f}" for m in ko["modes"] if 150 < m["f"] < 760))


for arg in sys.argv[1:]:
    auswerten(pathlib.Path(arg))
