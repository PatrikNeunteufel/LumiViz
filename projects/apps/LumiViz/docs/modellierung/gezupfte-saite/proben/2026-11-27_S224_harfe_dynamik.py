"""S224, ZD1 a — die Staerke je Stufe an der Harfe: f gegen mf an der VCSL-Konzertharfe, je Teilton.

Aufruf (vom Projekt-Root, das Archiv eingesteckt): python -X utf8 -B <dieses Skript> [<ordner der Aufnahmen>]
    Vorgabe: E:/archive/LumiViz/samples/Klasse 2 - Gezupfte Saite/VCSL/Chordophones/Composite Chordophones/Concert Harp
Ergebnis: `2026-11-27_S224_harfe_dynamik.txt` (die Ausgabe dieses Skripts).

Wie `2026-11-15_S199_dynamik_je_teilton` an der Iowa-Gitarre (GZ6 a): je Note, die in beiden Staerken vorliegt, der Pegel
der Teiltoene 1-12 frueh (0,05-0,45 s nach dem Einsatz, an der gemessenen Frequenz demoduliert mit Hann-Fenster) und daraus
Delta = L(f) - L(mf). Gezaehlt werden Teiltoene bis 50 dB unter dem staerksten der Note (beide Staerken) und unter 12 kHz.
Angepasst ueber alle Teiltoene:
  nur Auslenkung       Delta = A
  mit Neigung          Delta = A + C lb(f / 1 kHz)      (GZ6 a: Neigung je dB Stufe k = C / A; Gitarre 0,16)
Rest = Median von |gemessen - Gesetz|, Streuung = Median von |gemessen - Median|. Die Unsicherheit von C und k aus
2000 Ziehungen der Noten mit Zuruecklegen (5.-95. Perzentil). Die Aufnahmen sind nicht je Datei normiert (Spitze f
4-14 dB ueber mf), A ist also der Abstand der Staerken in dB.
"""
import pathlib
import re
import sys

import numpy as np
from scipy.io import wavfile

ORDNER = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path(
    "E:/archive/LumiViz/samples/Klasse 2 - Gezupfte Saite/VCSL/Chordophones/Composite Chordophones/Concert Harp")
NAMEN = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def midi(name):
    m = re.match(r"([A-G])(#|b)?(\d)", name)
    n = NAMEN[m.group(1)] + (1 if m.group(2) == "#" else -1 if m.group(2) == "b" else 0)
    return 12 * (int(m.group(3)) + 1) + n


def lies(p):
    import warnings
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        fs, x = wavfile.read(p)
    voll = {np.dtype("int16"): 32768.0, np.dtype("int32"): 2147483648.0}.get(x.dtype, 1.0)
    x = x.astype(np.float64) / voll
    if x.ndim > 1:
        x = x.mean(axis=1)
    return fs, x


def teiltoene(x, fs, f0, anzahl=12):
    """(Hz, Pegel frueh dB) je Teilton ab dem Einsatz."""
    einsatz = int(np.argmax(np.abs(x) > 0.1 * np.max(np.abs(x))))
    x = x[einsatz:]
    a, e = int(0.1 * fs), min(int(1.1 * fs), len(x))
    S = np.abs(np.fft.rfft(x[a:e] * np.hanning(e - a), 1 << 20))
    hz = np.fft.rfftfreq(1 << 20, 1 / fs)
    aus = []
    for n in range(1, anzahl + 1):
        if f0 * n > 12000:
            break
        band = (hz > f0 * (n - 0.3)) & (hz < f0 * (n + 0.3))
        i = int(np.argmax(np.where(band, S, 0)))
        y0, y1, y2 = np.log(S[i - 1] + 1e-300), np.log(S[i] + 1e-300), np.log(S[i + 1] + 1e-300)
        fn = hz[i] + 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2) * (hz[1] - hz[0])
        s = slice(int(0.05 * fs), int(0.45 * fs))
        t = np.arange(s.start, s.stop) / fs
        z = x[s] * np.exp(-2j * np.pi * fn * t) * np.hanning(s.stop - s.start)
        aus.append((fn, 20 * np.log10(np.abs(np.mean(z)) + 1e-300)))
    return aus


def anpassen(f, d):
    X = np.stack([np.ones_like(f), np.log2(f / 1000.0)], axis=1)
    (A, C), *_ = np.linalg.lstsq(X, d, rcond=None)
    return A, C


def main():
    dateien = {}
    for p in sorted(ORDNER.glob("KSHarp_*.wav")):
        m = re.match(r"KSHarp_([A-G][#b]?\d)_([a-z]+)\d\.wav", p.name)
        if m:
            dateien.setdefault(m.group(1), {})[m.group(2)] = p
    paare = {k: v for k, v in dateien.items() if "f" in v and "mf" in v}
    print(f"{len(paare)} Noten in f und mf: " + " ".join(sorted(paare, key=midi)))
    je_note = []
    for name in sorted(paare, key=midi):
        f0 = 440.0 * 2 ** ((midi(name) - 69) / 12)
        fs1, xf = lies(paare[name]["f"])
        fs2, xm = lies(paare[name]["mf"])
        tf, tm = teiltoene(xf, fs1, f0), teiltoene(xm, fs2, f0)
        oben = max(max(l for _, l in tf), max(l for _, l in tm))
        zeilen = [(hf, lf - lm) for (hf, lf), (_, lm) in zip(tf, tm) if min(lf, lm) > oben - 50.0]
        je_note.append((name, f0, zeilen))
        if zeilen:
            print(f"  {name:4s} {f0:7.1f} Hz: {len(zeilen):2d} Teiltoene, Delta Median {np.median([d for _, d in zeilen]):6.2f} dB, "
                  f"Grundton {zeilen[0][1] if zeilen[0][0] < 1.5 * f0 else float('nan'):6.2f} dB")
    f = np.array([h for _, _, z in je_note for h, _ in z])
    d = np.array([x for _, _, z in je_note for _, x in z])
    print(f"{len(d)} Teiltoene")
    streuung = np.median(np.abs(d - np.median(d)))
    A0 = np.median(d)
    print(f"nur Auslenkung:  A = {A0:.2f} dB; Rest {np.median(np.abs(d - A0)):.2f} dB, Streuung {streuung:.2f} dB")
    A, C = anpassen(f, d)
    rest = np.median(np.abs(d - (A + C * np.log2(f / 1000.0))))
    print(f"mit Neigung:     A = {A:.2f} dB, C = {C:.2f} dB je Oktave, k = C / A = {C / A:.3f} je dB Stufe; Rest {rest:.2f} dB")
    rng = np.random.default_rng(7)
    ks, cs = [], []
    for _ in range(2000):
        wahl = rng.integers(0, len(je_note), len(je_note))
        ff = np.array([h for i in wahl for h, _ in je_note[i][2]])
        dd = np.array([x for i in wahl for _, x in je_note[i][2]])
        a, c = anpassen(ff, dd)
        cs.append(c)
        ks.append(c / a)
    print(f"Ziehung der Noten (2000): C 5.-95. Perzentil {np.percentile(cs, 5):.2f} ... {np.percentile(cs, 95):.2f} dB je Oktave, "
          f"k {np.percentile(ks, 5):.3f} ... {np.percentile(ks, 95):.3f}")
    print("Je Band der Teiltoene: Median Delta (Zahl)")
    for lo, hi in ((0, 300), (300, 600), (600, 1200), (1200, 2400), (2400, 6000), (6000, 12000)):
        w = (f >= lo) & (f < hi)
        print(f"  {lo:5d}-{hi:5d} Hz: {np.median(d[w]) if w.any() else float('nan'):6.2f} dB ({int(w.sum())})")
    print("Zum Vergleich die Iowa-Gitarre (S199, ff - mf): A 7,89 dB, C 0,84 dB je Oktave, k 0,106; ff - pp: 17,20 / 3,00, k 0,174")


if __name__ == "__main__":
    main()
