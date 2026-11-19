"""S207 - Moden der Gitarre 300-1100 Hz und |p/F| an Saite 1 an den Grundtoenen 64-83, je Mode ihr Beitrag.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>
"""
import importlib.util
import json
import pathlib

import numpy as np

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

k = json.loads((ps.KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))
print("Hoerorte:", k["listeningPoints"])
print("origin:", k.get("origin"), k.get("measurement"))
p = k["bridgePoints"].index(64)
print("\nModen 80-1100 Hz: f, Q, Formwert an Saite 1, Winkel, |G| links / rechts, Phase links / rechts (Grad)")
moden = [m for m in k["modes"] if 80 <= m["f"] <= 1100 and abs(m.get("bridgeAngle", 0)) != 90]
for m in moden:
    g = [complex(*x) for x in m["radiation"]]
    print(f"  {m['f']:7.1f}  Q {1 / m['eta']:5.1f}  phi {m['bridge'][p]:+8.4f}  {m.get('bridgeAngle', 0):+6.1f}  "
          f"|G| {abs(g[0]):.4f} / {abs(g[1]):.4f}  {np.degrees(np.angle(g[0])):+6.0f} / {np.degrees(np.angle(g[1])):+6.0f}")

kp = ps.lade_koerper("classical_guitar")
punkt = kp["tasten"].index(64)
print("\n|p/F| senkrecht an den Grundtoenen (dB): links / rechts; die drei groessten Beitraege je Hoerort (Mode Hz: dB, Phase)")
wn = 2 * np.pi * kp["f"]
for taste in range(64, 84):
    f0 = ps.hz_von(taste)
    w = 2 * np.pi * f0
    c = -w * w / (wn ** 2 - w * w + 1j * kp["eta"] * wn * w) * kp["steg"][:, punkt]
    text = []
    for o in range(2):
        b = c * kp["G"][:, o]
        i = np.argsort(-np.abs(b))[:3]
        text.append(f"{20 * np.log10(abs(b.sum())):+6.1f} [" + ", ".join(
            f"{kp['f'][j]:.0f}: {20 * np.log10(abs(b[j])):+.0f} {np.degrees(np.angle(b[j])):+.0f}" for j in i) + "]")
    print(f"  {taste} {f0:6.1f} Hz: " + "   ".join(text))

hz = np.arange(300.0, 1100.0, 2.0)
Y, H = ps.koerper_kurven(kp, punkt, 2 * np.pi * hz + 0j)
print("\n|p/F| 300-1100 Hz in 20-Hz-Schritten (dB), links / rechts:")
for i in range(0, len(hz), 10):
    print(f"  {hz[i]:5.0f}: {20 * np.log10(abs(H[i, 0])):+6.1f} / {20 * np.log10(abs(H[i, 1])):+6.1f}")
