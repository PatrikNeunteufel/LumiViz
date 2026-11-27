"""S224, 34k-7d-4 (PK1 a) - die statische Kraft im Bezug: die Stegkraft senkrecht 1-3 s je Band, mit und ohne den\nGleichanteil (Mittel 3-3,75 s), dazu der Gleichanteil selbst.\n\nAufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <fall> [<fall> ...]   (Namen wie in `faelle()` des Bezugs)\nErgebnis: `2026-11-27_S224_bezug_statik.txt` (dort auch die Schleife des Kerns zum Vergleich).\n"""
import importlib.util, json, pathlib, sys
import numpy as np
P = pathlib.Path.cwd() / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"
spec = importlib.util.spec_from_file_location("bz", P / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps = bz.ps
BAND_N = 1 << 18  # die Baender ueber den ganzen Ton, eine FFT auf 2^18 Werten (wie der erste Entwurf von PK1 a)
BAENDER = [(20, 100), (100, 300), (300, 1000), (1000, 5000), (5000, 20000)]
w = ps.frequenzen()


def baender(x):
    X = np.fft.rfft(x, BAND_N)
    hz = np.fft.rfftfreq(BAND_N, 1 / ps.FS)
    aus = []
    for lo, hi in BAENDER:
        y = np.fft.irfft(np.where((hz >= lo) & (hz < hi), X, 0), BAND_N)[:len(x)][int(1.0 * ps.FS):int(3.0 * ps.FS)]
        aus.append(10 * np.log10(np.mean(y ** 2) + 1e-300))
    return " / ".join(f"{lo}-{hi}: {v:.2f}" for (lo, hi), v in zip(BAENDER, aus))


for name, instrument, taste, stufe in [f for f in bz.faelle() if f[0] in sys.argv[1:]]:
    saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = bz.eingaben(instrument, taste, stufe)
    koerper = bz.KOERPER[instrument]
    kp = ps.lade_koerper(koerper)
    roh = json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))
    punkt = kp["tasten"].index(stegtaste)
    datei = len(roh["modes"])
    tan = ps.richtung_von(kp)
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    S = len(saiten)
    Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt] * S)
    _, phi2 = bz.daempfer(roh["statistical"], punkt)
    wn = 2 * np.pi * bz.DAEMPFER_MITTEN
    yd = (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * bz.DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
    for i in range(S):
        for j in range(S):
            Y[:, 2 * i, 2 * j] += yd
    F = bz.stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, neigung, Y, w)
    kraft = ps.zeit(F[:, 0::2].sum(axis=1)[:, None])[:, 0]
    gleich = float(np.mean(kraft[int(3.0 * ps.FS):]))
    print(f"{name}: Gleichanteil 3-3,75 s {gleich:.6g} N, Mittel 1-2 s {float(np.mean(kraft[int(1.0 * ps.FS):int(2.0 * ps.FS)])):.6g} N")
    print(f"    mit       {baender(kraft)} dB")
    print(f"    ohne      {baender(kraft - gleich)} dB")
