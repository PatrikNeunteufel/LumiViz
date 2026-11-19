"""S207 - |p/F| der Gitarre an den Grundtoenen der Saite 1: Modell (Koerperdatei, am Schlagort `treble` der Messung)
gegen die Messung g34 (Mores) an den Mikrofonen mbass (= left), mtreble (= right) und east (nicht im Koerper).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <qualified_selected_impulses.mat>
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.io as sio

KOERPER = pathlib.Path("asset/loop/instrument_bodies")
spec = importlib.util.spec_from_file_location("mg", KOERPER / "make_gitarre.py")
mg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mg)

d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"][mg.GITARRE - 1]
daten = {ort: d[o * mg.FS:(o + 1) * mg.FS, :] for o, ort in enumerate(mg.ORTE)}
hz, H = mg.frfs(daten)

k = json.loads((KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))
moden = [m for m in k["modes"] if abs(m.get("bridgeAngle", 0.0)) != 90.0]
f = np.array([m["f"] for m in moden])
eta = np.array([m["eta"] for m in moden])
steg = np.array([m["bridge"] for m in moden]) * np.cos(np.radians([m.get("bridgeAngle", 0.0) for m in moden]))[:, None]
G = np.array([[complex(*g) for g in m["radiation"]] for m in moden])
xs = np.array(mg.X_SAITEN)
abc = np.array([np.polyfit(xs, z, 2) for z in steg])


def modell(freq, x):
    """p/F an beiden Hoerorten; die Messung traegt das Fenster exp(-3 t): dieselbe Verbreiterung (s -> s + 3)."""
    w = 2 * np.pi * np.asarray(freq) - 1j * mg.FENSTER_S
    wn = 2 * np.pi * f
    phi = np.array([np.polyval(q, x) for q in abc])
    c = -w[:, None] ** 2 / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * eta[None, :] * wn[None, :] * w[:, None])
    return c @ (phi[:, None] * G)


db = lambda v: 20 * np.log10(np.abs(v))        # noqa: E731
print("Schlagort treble (x = +23 mm; Saite 1 bei +29 mm). |p/F| in dB am Grundton:")
print("Taste     Hz | Messung mbass / mtreble / east | Modell left / right | Modell - Messung left / right")
zeilen = []
for taste in range(64, 84):
    f0 = 440.0 * 2 ** ((taste - 69) / 12)
    i = int(round(f0))
    m = [db(H[("treble", mik)][i]) for mik in ("mbass", "mtreble", "east")]
    mo = db(modell([float(i)], mg.X_SCHLAG["treble"])[0])
    zeilen.append((taste, *m, *mo))
    print(f"{taste:3d} {f0:7.1f} | {m[0]:+6.1f} / {m[1]:+6.1f} / {m[2]:+6.1f} | {mo[0]:+6.1f} / {mo[1]:+6.1f} | {mo[0] - m[0]:+5.1f} / {mo[1] - m[1]:+5.1f}")
a = np.array(zeilen)
print(f"Modell - Messung ueber die 20 Grundtoene: left Mittel {np.mean(a[:, 4] - a[:, 1]):+.1f}, Streuung {np.std(a[:, 4] - a[:, 1]):.1f} dB; "
      f"right Mittel {np.mean(a[:, 5] - a[:, 2]):+.1f}, Streuung {np.std(a[:, 5] - a[:, 2]):.1f} dB")


def summe(sp):
    return 10 * np.log10(np.mean(10 ** (a[:, sp] / 10), axis=1))


for name, l in (("Messung mbass + mtreble", summe([1, 2])), ("Messung alle drei", summe([1, 2, 3])), ("Messung east allein", a[:, 3]),
                ("Modell left + right", summe([4, 5]))):
    paare = [(abs(l[i] - l[j]), int(a[i, 0]), int(a[j, 0])) for i in range(20) for j in range(i + 1, 20) if a[j, 0] - a[i, 0] <= 4]
    g = max(paare)
    print(f"{name:24s}: 68 -> 71 {l[7] - l[4]:+5.1f} dB; groesster Sprung der Grundtoene {g[0]:.1f} dB ({g[1]}/{g[2]}), mittlerer {np.median([p[0] for p in paare]):.1f} dB")

print("\nKurve 380-520 Hz in 5-Hz-Schritten: Messung mbass / mtreble / east | Modell left / right")
for i in range(380, 521, 5):
    mo = db(modell([float(i)], mg.X_SCHLAG["treble"])[0])
    print(f"  {i}: {db(H[('treble', 'mbass')][i]):+6.1f} / {db(H[('treble', 'mtreble')][i]):+6.1f} / {db(H[('treble', 'east')][i]):+6.1f} | {mo[0]:+6.1f} / {mo[1]:+6.1f}")
