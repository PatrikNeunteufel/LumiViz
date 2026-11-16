"""S201, 34k-6 Punkt (1) — Harfe: Admittanz und Schalldruck an den Saitenoesen bei der Frequenz der Saite,
Koerperdatei gegen Bell 1988 Abb. 7.22 / 7.23 (fertige Harfe, besaitet; PDF 309, am Bild gelesen, +-1 dB).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> [--eta-tief <eta>] [--bis <Hz>]

Anlass: im Pruefstand sind die Tasten 55-57 (196-220 Hz) 8-13 LU lauter als ihre Nachbarn und fallen mit 20-84 dB/s
ab; ihr Grundton liegt auf der Mode 208,4 Hz der Koerperdatei (S(1,1)+). Bell misst diese Mode als schwach (Abb. 7.9:
216 Hz ohne Saiten, hoechstens 82 dB; 207 Hz besaitet, 78-86 dB) und schreibt S. 152: „considerably weaker than the
S(1,1)-“. Abb. 7.22 gibt die Groesse, an der die Lautheit je Taste haengt: die Eingangsadmittanz an der Oese jeder
Saite bei deren Frequenz (Saiten 33 ... 10, 104 ... 988 Hz; 116 dB = 1 s/kg).

Gerechnet: Y(p, f) = i w phi_p^T [diag(w_n^2 (1 + i eta_n) - w^2) + i w C]^-1 phi_p an der Oese der Saite (Stegpunkt der
weissen Taste ueber der leeren Saite, KQ22 a), C = Summe ueber die Saiten last * Z_i phi(p_i) phi(p_i)^T (bei Bell sind
die Saiten mit Band gedaempft; last = 0, 0,25 und 1). Dazu der Druck je Kraft an den Hoerorten (Leistungssumme) - Abb.
7.23 ist ein Schalldruck in 1,5 m bei Anregung mit dem Schwinger, der Bezug ist nicht genannt: nur die FORM (Median 0).

`--eta-tief`: Verlustfaktor fuer die Moden unter `--bis` (Vorgabe 230 Hz) ersetzen - Probe, wie viel die Daempfung der
Moden S(1,1)- / S(1,1)+ ausmacht (Bell: Q 11 und Q 16 in der Harfe ohne Saiten).

Schreibt nichts.
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent


def modul(datei, name):
    spec = importlib.util.spec_from_file_location(name, HIER / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


ps = modul("2026-11-15_S199_pruefstand_zupf.py", "pruefstand")
ha = modul("2026-11-14_S198_harfe_admittanz.py", "harfe_admittanz")

# Bell Abb. 7.22 (Admittanz, dB; 116 dB = 1 s/kg) und 7.23 (Schalldruck, dB, Bezug unbekannt) je Saite 33 ... 10
BELL_Y = {33: 80.3, 32: 89.2, 31: 73.7, 30: 79.2, 29: 81.2, 28: 82.3, 27: 85.1, 26: 86.1, 25: 84.3, 24: 87.2, 23: 88.2,
          22: 89.2, 21: 94.8, 20: 85.1, 19: 89.2, 18: 89.2, 17: 91.2, 16: 90.1, 15: 93.3, 14: 93.3, 13: 99.2, 12: 102.7,
          11: 97.9, 10: 99.2}
BELL_P = {33: -26, 32: -24, 31: -31, 30: -32, 29: -34, 28: -28, 27: -10, 26: -8, 25: -16, 24: -12, 23: -5.5, 22: -8,
          21: -5, 20: -11, 19: -12, 18: -17, 17: -15.5, 16: -18, 15: -24, 14: -13, 13: -20, 12: -17, 11: -10.5, 10: -17.5}

args = sys.argv[1:]
eta_tief = float(args[args.index("--eta-tief") + 1]) if "--eta-tief" in args else None
bis = float(args[args.index("--bis") + 1]) if "--bis" in args else 230.0

kp = ps.lade_koerper("concert_harp")
if eta_tief is not None:
    kp["eta"] = np.where(kp["f"] < bis, eta_tief, kp["eta"])
saiten = {}                                      # Nr. -> (Hz, Stegpunkt)
for nr, hz, *_ in ha.NYLON_DARM + ha.METALL:
    taste = int(round(69 + 12 * np.log2(hz / 440.0))) + 1          # weisse Taste = die Saite um einen Halbton verkuerzt
    if taste in kp["tasten"]:
        saiten[int(nr) if nr not in ("00",) else -1] = (float(hz), kp["tasten"].index(taste))
z = ha.wellenwiderstaende()
zi = np.array([z["00" if nr == -1 else str(nr)] for nr in saiten])
phi_s = np.array([kp["steg"][:, p] for _, p in saiten.values()]).T          # (Moden, Saiten)
wn2 = (2 * np.pi * kp["f"]) ** 2

print(f"concert_harp: {len(kp['f'])} Moden, {len(saiten)} Saiten mit Stegpunkt; Moden unter 300 Hz (Hz / Q): "
      + ", ".join(f"{f:.1f} / {1 / e:.0f}" for f, e in zip(kp["f"], kp["eta"]) if f < 300))


def an_der_saite(nr, last):
    hz, p = saiten[nr]
    w = 2 * np.pi * hz
    A = np.diag(wn2 * (1 + 1j * kp["eta"]) - w * w).astype(complex)
    if last > 0:
        A += 1j * w * (phi_s * (last * zi)[None, :]) @ phi_s.T
    q = np.linalg.solve(A, kp["steg"][:, p])
    y = 1j * w * (kp["steg"][:, p] @ q)
    druck = -w * w * (q @ kp["G"])                                   # Pa/N je Hoerort
    return 20 * np.log10(abs(y)) + ha.BEZUG, 10 * np.log10(np.sum(np.abs(druck) ** 2))


nrn = sorted(BELL_Y, reverse=True)
erg = {last: np.array([an_der_saite(n, last) for n in nrn]) for last in (0.0, 0.25, 1.0)}
by = np.array([BELL_Y[n] for n in nrn])
bp = np.array([BELL_P[n] for n in nrn])
print("\nSaite  Hz   | Bell Y dB | Modell Y: ohne Last, 0,25 Z, 1 Z  (Abstand zu Bell bei 0,25 Z) | Bell p (Median 0) | Modell p, 0,25 Z (Median 0)")
p_mod = erg[0.25][:, 1] - np.median(erg[0.25][:, 1])
for i, n in enumerate(nrn):
    print(f"  {n:2d}  {saiten[n][0]:5.0f} |  {by[i]:5.1f}   |  {erg[0.0][i, 0]:5.1f}  {erg[0.25][i, 0]:5.1f}  {erg[1.0][i, 0]:5.1f}   ({erg[0.25][i, 0] - by[i]:+5.1f}) |"
          f"  {bp[i] - np.median(bp):+5.1f}  |  {p_mod[i]:+5.1f}")
for last in (0.0, 0.25, 1.0):
    d = erg[last][:, 0] - by
    print(f"Admittanz, Last {last}: Abstand Modell - Bell Median {np.median(d):+.1f} dB, Effektivwert um den Median {np.std(d):.1f} dB, "
          f"groesster {d[np.argmax(np.abs(d - np.median(d)))]:+.1f} dB an Saite {nrn[int(np.argmax(np.abs(d - np.median(d))))]}")
for name, v in (("Bell  ", by), ("Modell", erg[0.25][:, 0])):
    s = np.abs(np.diff(v))
    print(f"{name}: Schritt der Admittanz zwischen Nachbarsaiten groesster {s.max():.1f} dB (Saiten {nrn[int(np.argmax(s))]}/{nrn[int(np.argmax(s)) + 1]}), Mittel {s.mean():.1f} dB")
dp = p_mod - (bp - np.median(bp))
print(f"Schalldruck (Form): Effektivwert Modell - Bell {np.sqrt(np.mean(dp ** 2)):.1f} dB; Saiten 27 / 26 / 25 gegen den Median der Saiten 24-20: "
      f"Bell {np.mean(bp[6:9]) - np.median(bp[9:14]):+.1f} dB, Modell {np.mean(erg[0.25][6:9, 1]) - np.median(erg[0.25][9:14, 1]):+.1f} dB")
