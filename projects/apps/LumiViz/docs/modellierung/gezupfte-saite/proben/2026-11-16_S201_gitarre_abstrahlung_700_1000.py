"""S201, 34k-6 Punkt (20) — die Abstrahlung der Gitarre g34 in 700-1000 Hz: Koerperdatei gegen die gemessenen Kurven.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <qualified_selected_impulses.mat> [weitere Gitarren ...]

Frage: der Pruefstand gibt an den Tasten 81 / 82 / 83 (880 / 932 / 988 Hz, erste Saite) einen Sprung von 13,8 LU; er
folgt dem Druck je Kraft des Koerpers am Stegpunkt der ersten Saite. Steht dieser Gang in der Messung?

  1  Messung |P / F| an den drei Schlagorten (bass -23,2 mm, center 0, treble +23,2 mm) und drei Mikrofonen gegen das
     Modell der Koerperdatei am selben Ort (Formwert phi(x) = a + b x + c x^2, aus den sechs Stegpunkten zurueckgerechnet)
     und am Stegpunkt der ersten Saite (+29 mm, ausserhalb der Schlagorte). Je Grundton der Tasten 76-84 der Wert an der
     Frequenz und das Leistungsmittel im Halbtonband.
  2  Der Gang 988 gegen 880 Hz je Schlagort: Messung, Modell, und die Fortsetzung der Messung auf +29 mm (Gerade durch
     center und treble in dB - nur ein Anhalt).
  3  Fuer jede genannte Gitarre: der groesste Schritt des gemessenen Pegels zwischen Nachbar-Halbtoenen (Halbtonband,
     Schlag treble, beide Nahfeld-Mikrofone als Leistungssumme, und das Mikrofon east) ueber 82-1000 Hz - wie gross
     Spruenge von Halbton zu Halbton im Nahfeld ueberhaupt sind.

Schreibt nichts.
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent


def modul(datei):
    spec = importlib.util.spec_from_file_location(datei.replace("-", "_"), HIER / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


mf = modul("2026-11-15_S199_mores_fit.py")
ps = modul("2026-11-15_S199_pruefstand_zupf.py")

SAITENABSTAND = 0.0116
X_SCHLAG = {"bass": -2 * SAITENABSTAND, "center": 0.0, "treble": 2 * SAITENABSTAND}
X_SAITEN = np.array([(k - 2.5) * SAITENABSTAND for k in range(6)])
MIK = (("left", "mbass"), ("right", "mtreble"))


def hz_von(taste):
    return 440.0 * 2 ** ((taste - 69) / 12)


def band(hz, y, mitte):
    m = (hz >= mitte * 2 ** (-1 / 24)) & (hz < mitte * 2 ** (1 / 24))
    return 10 * np.log10(np.mean(np.abs(y[m]) ** 2))


def punkt(hz, y, f):
    return 20 * np.log10(np.abs(y[int(round(f))]))


def modell(kp, x, hz):
    """Druck je Kraft (Pa/N je Hoerort) der Koerperdatei bei Kraft am Ort x laengs des Stegs."""
    koeff = np.polyfit(X_SAITEN, kp["steg"].T, 2)           # (3, Moden): die Form a + b x + c x^2 ist genau
    phi = koeff[0] * x * x + koeff[1] * x + koeff[2]
    w = 2 * np.pi * hz[:, None]
    wn = 2 * np.pi * kp["f"][None, :]
    nenner = 1.0 / (wn ** 2 - w ** 2 + 1j * kp["eta"][None, :] * wn * w)
    return (-w ** 2 * nenner) @ (phi[:, None] * kp["G"])


pfad = sys.argv[1]
weitere = [int(a) for a in sys.argv[2:]]
kp = ps.lade_koerper("classical_guitar")
hz, H = mf.frfs(mf.lade(pfad, 34))
tasten = list(range(76, 85))

print("g34 - |P / F| in dB re 1 Pa/N: an der Frequenz des Grundtons / Leistungsmittel im Halbtonband")
mod = {x: modell(kp, x, hz) for x in list(X_SCHLAG.values()) + [float(X_SAITEN[5])]}
for o, (ort, mik) in enumerate(MIK):
    print(f"\nHoerort {ort} (Mikrofon {mik[1:]})")
    print("Taste   Hz  | " + " | ".join(f"Schlag {s:6s} Messung  Modell" for s in X_SCHLAG) + " | Modell Saite 1")
    for t in tasten:
        f = hz_von(t)
        zeile = f"{t:4d} {f:6.0f} |"
        for s, x in X_SCHLAG.items():
            zeile += f" {punkt(hz, H[(s, mik)], f):6.1f}/{band(hz, H[(s, mik)], f):6.1f}  {punkt(hz, mod[x][:, o], f):6.1f}/{band(hz, mod[x][:, o], f):6.1f} |"
        x1 = float(X_SAITEN[5])
        print(zeile + f" {punkt(hz, mod[x1][:, o], f):6.1f}/{band(hz, mod[x1][:, o], f):6.1f}")

print("\nMikrofon east (im Koerper nicht verwendet), Messung an der Frequenz / im Halbtonband")
for t in tasten:
    f = hz_von(t)
    print(f"{t:4d} {f:6.0f} | " + " | ".join(f"{s:6s} {punkt(hz, H[(s, 'east')], f):6.1f}/{band(hz, H[(s, 'east')], f):6.1f}" for s in X_SCHLAG))

print("\nGang Taste 83 gegen 81 (988 gegen 880 Hz), Halbtonband, dB: Messung / Modell je Schlagort; Modell an Saite 1;")
print("Messung auf +29 mm fortgesetzt (Gerade in dB durch center und treble - Anhalt)")
for o, (ort, mik) in enumerate(MIK):
    g_mess, g_mod = {}, {}
    for s, x in X_SCHLAG.items():
        g_mess[s] = band(hz, H[(s, mik)], hz_von(83)) - band(hz, H[(s, mik)], hz_von(81))
        g_mod[s] = band(hz, mod[x][:, o], hz_von(83)) - band(hz, mod[x][:, o], hz_von(81))
    x1 = float(X_SAITEN[5])
    saite1 = band(hz, mod[x1][:, o], hz_von(83)) - band(hz, mod[x1][:, o], hz_von(81))
    fort = g_mess["treble"] + (g_mess["treble"] - g_mess["center"]) * 0.25
    print(f"  {ort:5s}: " + ", ".join(f"{s} {g_mess[s]:+5.1f} / {g_mod[s]:+5.1f}" for s in X_SCHLAG)
          + f"; Modell Saite 1 {saite1:+5.1f}; Messung fortgesetzt {fort:+5.1f}")
g_e = {s: band(hz, H[(s, "east")], hz_von(83)) - band(hz, H[(s, "east")], hz_von(81)) for s in X_SCHLAG}
print("  east : " + ", ".join(f"{s} {g_e[s]:+5.1f}" for s in X_SCHLAG) + " (nur Messung)")

print("\nSchritte des gemessenen Pegels von Halbton zu Halbton, 82-1000 Hz (Tasten 40-83), Schlag treble, Halbtonband:")
print("Gitarre | Nahfeld (bass + treble, Leistungssumme): groesster, 90. Perzentil, Median, Ort des groessten | east: groesster, Median")
for g in [34] + weitere:
    hz_g, Hg = (hz, H) if g == 34 else mf.frfs(mf.lade(pfad, g))
    nah = np.array([10 * np.log10(10 ** (band(hz_g, Hg[("treble", "mbass")], hz_von(t)) / 10)
                                  + 10 ** (band(hz_g, Hg[("treble", "mtreble")], hz_von(t)) / 10)) for t in range(40, 84)])
    east = np.array([band(hz_g, Hg[("treble", "east")], hz_von(t)) for t in range(40, 84)])
    d, de = np.abs(np.diff(nah)), np.abs(np.diff(east))
    i = int(np.argmax(d))
    print(f"  g{g:02d}   | {d.max():4.1f}  {np.percentile(d, 90):4.1f}  {np.median(d):4.1f}  Tasten {40 + i}/{41 + i} | {de.max():4.1f}  {np.median(de):4.1f}")
