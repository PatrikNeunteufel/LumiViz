"""S209 - Z3 so, wie der Test es rechnet: Schalldruck am Hoerort `front` je Kraft am Steg, quadratisches Mittel
ueber die Stegpunkte der Tasten 36 / 50 / 60, ungeglaettet, 60-400 Hz in 0,5 Hz.
Aufruf: python -X utf8 -B oud_z3_wie_test.py <oud.lvinstbody>"""
import json
import sys

import numpy as np

k = json.load(open(sys.argv[1], encoding="utf-8"))
m = k["modes"]
hz = np.array([x["f"] for x in m])
eta = np.array([x["eta"] for x in m])
G = np.array([complex(*x["radiation"][0]) for x in m])
f = np.arange(60.0, 400.01, 0.5)
om = 2 * np.pi * f[:, None]
omn = 2 * np.pi * hz[None, :]
kern = -(om ** 2) / (omn ** 2 * (1 + 1j * eta[None, :]) - om ** 2)
p2 = np.mean([np.abs(kern @ (np.array([x["bridge"][k["bridgePoints"].index(t)] for x in m]) * G)) ** 2 for t in (36, 50, 60)], axis=0)
db = 10 * np.log10(p2)
a = (f >= 120) & (f <= 160)
b = (f >= 170) & (f <= 400)
fl, fh = f[a][np.argmax(db[a])], f[b][np.argmax(db[b])]
print(f"Luftmode {fl:.1f} Hz ({db[a].max():.1f} dB), Hauptspitze {fh:.1f} Hz ({db[b].max():.1f} dB), Verhaeltnis {fh / fl:.3f}")
tal = (f > fl) & (f < fh)
print(f"Tal dazwischen {db[tal].min() - db[b].max():.1f} dB, Luftmode {db[a].max() - db[b].max():.1f} dB gegen die Hauptspitze")
for t in (131.0, 196.0):
    print(f"   bei {t:.0f} Hz: {db[np.argmin(np.abs(f - t))] - db[f < 300].max():.1f} dB unter dem Maximum unter 300 Hz")
