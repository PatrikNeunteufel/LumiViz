"""S200, 34k-6 — an den VCSL-Cembali gemessen: der Tastenstoss und die Tonhoehe des Lautenzugs.

Aufruf (vom Projekt-Root): python -B <dieses Skript> <Ordner …/VCSL/Chordophones/Zithers> [<ausgabe.txt>]

Tastenstoss  Dieselbe Groesse wie am Modell (`2026-11-16_S200_cembalo_register.py stoss`): Pegel im Band 30-300 Hz
             (Butterworth 4. Ordnung) in den ersten 0,2 s nach dem Einsatz gegen den ganzen Ton in demselben Fenster —
             an Tasten ab C5, deren Saite unter 300 Hz nichts traegt. Dazu dasselbe Band 1,0-1,2 s nach dem Einsatz
             (dort ist der Stoss verklungen: was bleibt, ist Raum und Rauschen) und 0,2 s vor dem Einsatz, soweit die
             Datei so frueh beginnt. Modell: ohne Stoss -24 dB (C5), -39 dB (C6); mit 10 mN s -10 / -18 dB.
             Die Aufnahmen haben eine Staerke und einen Raum; die Namen liegen eine Oktave zu tief (S195).
Lautenzug    Englisches Cembalo, *Lute* gegen *Normal* an derselben Taste: Tonhoehe aus den Teiltoenen 1-4 (Median von
             f_n / n; Frequenzen wie in `2026-11-15_S199_ausklang_je_teilton.py`), Unterschied in Cent.
"""
import importlib.util
import pathlib
import re
import sys

import numpy as np
import scipy.io.wavfile as wavfile
import scipy.signal as sig

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("at", HIER.with_name("2026-11-15_S199_ausklang_je_teilton.py"))
at = importlib.util.module_from_spec(spec)
spec.loader.exec_module(at)

NOTEN = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}
GRUPPEN = [("englisch Normal", "Harpsichord, English/Sustains/Normal"), ("englisch Lute", "Harpsichord, English/Sustains/Lute"),
           ("franzoesisch", "Harpsichord, French/Sustains"), ("Unk", "Harpsichord, Unk/Sustains"),
           ("flaemisch Low", "Harpsichord, Flemish/Sustains/Low"), ("flaemisch High", "Harpsichord, Flemish/Sustains/High"),
           ("italienisch", "Harpsichord, Italian/Sustains/stop1")]


def lies(pfad):
    fs, x = wavfile.read(str(pfad))
    x = x.astype(np.float64)
    if x.ndim == 1:
        x = x[:, None]
    return x / max(1.0, np.abs(x).max()), fs


def taste_von(name):
    m = re.search(r"_([A-G])(#?)(\d)_", name)
    return 12 * (int(m.group(3)) + 1) + NOTEN[m.group(1)] + (1 if m.group(2) else 0) + 12      # + 1 Oktave (S195)


def stoss(x, fs):
    """(Band frueh, Band spaet, Band vor dem Einsatz) in dB gegen den ganzen Ton in den ersten 0,2 s."""
    mono = np.sqrt(np.sum(x ** 2, axis=1))
    ein = int(np.argmax(mono > 0.1 * mono.max()))
    sos = sig.butter(4, [30.0, 300.0], btype="bandpass", fs=fs, output="sos")
    tief = sig.sosfilt(sos, x, axis=0)
    n = int(0.2 * fs)
    ganz = np.mean(x[ein:ein + n] ** 2)
    db = lambda a: 10 * np.log10(np.mean(tief[a:a + n] ** 2) / ganz + 1e-30) if 0 <= a and a + n <= len(x) else float("nan")
    return db(ein), db(ein + int(1.0 * fs)), db(ein - n - int(0.01 * fs))


def tonhoehe(x, fs, taste):
    teile = at.teiltoene(x.mean(axis=1), fs, 440.0 * 2 ** ((taste - 69) / 12))
    f = [hz / n for n, hz, *_ in teile if n <= 4]
    return float(np.median(f)) if len(f) >= 2 else float("nan")


if __name__ == "__main__":
    wurzel = pathlib.Path(sys.argv[1])
    aus = open(sys.argv[2], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 2 else None

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    schreibe("Tastenstoss: Band 30-300 Hz gegen den ganzen Ton der ersten 0,2 s (dB): frueh / 1,0-1,2 s / vor dem Einsatz")
    hoehe = {}
    for name, ordner in GRUPPEN:
        zeilen = []
        for pfad in sorted((wurzel / ordner).glob("*rr1*.wav")) + sorted((wurzel / ordner).glob("*_1.wav")):
            taste = taste_von(pfad.name)
            x, fs = lies(pfad)
            if name.startswith("englisch"):
                hoehe.setdefault(taste, {})[name] = tonhoehe(x, fs, taste)
            sounding = taste + (12 if name == "flaemisch High" else 0) - (12 if name == "flaemisch High" else 0)
            if 440.0 * 2 ** ((sounding - 69) / 12) >= 500.0:
                zeilen.append((taste,) + stoss(x, fs))
        for z in zeilen:
            schreibe(f"  {name:16s} Taste {z[0]:3d}: {z[1]:6.1f} / {z[2]:6.1f} / {z[3]:6.1f}")
        a = np.array(zeilen)
        if len(a):
            for lage, lo, hi in (("C5-H5", 72, 84), ("C6-H6", 84, 96), ("ab C7", 96, 128)):
                g = a[(a[:, 0] >= lo) & (a[:, 0] < hi)]
                if len(g):
                    schreibe(f"  {name:16s} {lage}: Median frueh {np.median(g[:, 1]):6.1f} dB, spaet {np.nanmedian(g[:, 2]):6.1f} dB, "
                             f"vorher {np.nanmedian(g[:, 3]) if np.isfinite(g[:, 3]).any() else float('nan'):6.1f} dB ({len(g)} Toene)")
    schreibe()
    schreibe("Lautenzug: Tonhoehe Lute gegen Normal (englisches Cembalo), Cent")
    d = []
    for taste in sorted(hoehe):
        h = hoehe[taste]
        if len(h) == 2 and all(np.isfinite(list(h.values()))):
            c = 1200 * np.log2(h["englisch Lute"] / h["englisch Normal"])
            d.append((taste, c))
            schreibe(f"  Taste {taste:3d}: Normal {h['englisch Normal']:8.2f} Hz, Lute {h['englisch Lute']:8.2f} Hz, {c:+6.1f} Cent")
    a = np.array(d)
    schreibe(f"Lute - Normal: Median {np.median(a[:, 1]):+.1f} Cent, 10.-90. Perzentil {np.percentile(a[:, 1], 10):+.1f} ... {np.percentile(a[:, 1], 90):+.1f}; "
             f"ab C5 Median {np.median(a[a[:, 0] >= 72, 1]):+.1f} Cent ({len(a)} Tasten)")
