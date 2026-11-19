"""S209 - Z2 wie gemessen: Beschleunigung am Antwortpunkt 3 je Kraft, gemittelt ueber das 5-cm-Raster, MIT dem
Aufnehmer (5,8 g) am Antwortpunkt (Massenaenderung im Modenraum, Formen mitgedreht). Gegen Abdallah 2026 Abb. 6:
Hoehen rel. 0,51 / 1,00 / 0,43 / 0,37 bei 139 / 203 / 280 / 362 Hz, Tal 0,42-0,49 bei 149-160 Hz, 0,12 bei 251 Hz.
Aufruf: python -X utf8 -B oud_z2_gemessen.py <decke mm> <steghoehe mm> <stegbreite mm> <muendung> <rosette g>"""
import json
import sys

import numpy as np
from scipy.linalg import eigh

import oud_feinsuche6 as f6
import oud_suche as s

x = [float(v) for v in sys.argv[1:6]]
e = s.rechne(f6.plan(x), "z2_gemessen", to=1000, netz=2500, zeigen=False)
k = json.loads((s.HIER / "z2_gemessen.lvinstbody").read_text(encoding="utf-8"))
mf = np.array([m["f"] for m in k["modes"]])
eta0 = np.array([m["eta"] for m in k["modes"]])
w, knoten, ap = e["_w"], e["_knoten"], e["_antwort"]


def naechster(xx, yy):
    return int(np.argmin((knoten[:, 0] - xx) ** 2 + (knoten[:, 1] - yy) ** 2))


raster, mitte = [], []
for xr in range(-15, 16, 5):
    for yr in range(0, 41, 5):
        xx, yy = xr / 100.0, 0.46 - yr / 100.0
        kn = naechster(xx, yy)
        if np.hypot(knoten[kn, 0] - xx, knoten[kn, 1] - yy) < 0.012:
            raster.append(kn)
            if xr == 0:
                mitte.append(kn)


def kurven(masse):
    v = w[:, ap]
    lam, q = eigh(np.diag((2 * np.pi * mf) ** 2), np.eye(len(mf)) + masse * np.outer(v, v))
    hz = np.sqrt(lam) / (2 * np.pi)
    wn = q.T @ w
    eta = np.interp(hz, mf, eta0)
    f = np.arange(60.0, 420.01, 0.25)
    om = 2 * np.pi * f[:, None]
    omn = 2 * np.pi * hz[None, :]
    kern = -(om ** 2) / (omn ** 2 * (1 + 1j * eta[None, :]) - om ** 2)
    alle = np.array([np.abs(kern @ (wn[:, ap] * wn[:, r])) for r in raster])
    mi = np.array([np.abs(kern @ (wn[:, ap] * wn[:, r])) for r in mitte])
    return hz, f, np.sqrt((alle ** 2).mean(axis=0)), np.sqrt((mi ** 2).mean(axis=0)), np.abs(kern @ (wn[:, ap] ** 2))


for was, masse in (("ohne Aufnehmer", 0.0), ("mit Aufnehmer 5,8 g", 0.0058)):
    hz, f, mittel, mi, punkt = kurven(masse)
    print(f"{was}: Moden " + " / ".join(f"{v:.1f}" for v in hz[:6]))
    for name, a in (("Raster alle", mittel), ("Raster Mittellinie", mi), ("am Punkt", punkt)):
        sp = s.spitzen(f, a)
        um = [p for p in sp if 185 <= p[0] <= 225]
        an = [p for p in sp if not (185 <= p[0] <= 225)]
        bezug = max(p[1] for p in um) if um else a.max()
        z2 = 20 * np.log10(bezug / max(p[1] for p in an)) if um and an else float("nan")

        def tal(a0, a1):
            sel = (f >= a0) & (f <= a1)
            return a[sel].min() / bezug

        print(f"   {name:19s} Z2 {z2:+5.1f} dB   Spitzen rel.: " + "  ".join(f"{p[0]:.0f}:{p[1] / bezug:.2f}" for p in sp)
              + f"   Tal 145-200: {tal(145, 200):.2f}, Tal 225-270: {tal(225, 270):.2f}")
