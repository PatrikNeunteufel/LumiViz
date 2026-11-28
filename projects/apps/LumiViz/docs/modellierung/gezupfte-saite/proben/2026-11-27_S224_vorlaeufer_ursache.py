"""S224, VL4 a — die Ursache des Vorlaeufers der Quelle bei Stufe 0 (Laute 38, Cembalo 84 / 89, dazu Gitarre 57, Harfe 60).

Stand vor VL7 a (S224): seit VL7 a nehmen `quelle` und `stegkraft` des Bezugs die kausale Wellenzahl (`wellen_kausal`);
die Zahlen dieser Probe stehen in der .txt daneben, ein neuer Lauf rechnet mit dem kausalen Verlust.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>

Die Quelle g einer Saite (`quelle` des Bezugs `2026-11-26_S222_bezug_zupf.py`) mit 0,3 s Vorlauf, 4 s auf der Geraden
w - i s0 wie der Pruefstand; gemessen Spitze / Energie vor -2 / -20 ms gegen die ganze Quelle (dB). Je Kandidat
abgeschaltet:

  ohne Gauss       Fingerbreite 0 (kein Gauss-Tiefpass)
  ohne Verlust     Verlustfaktor der Saite 0 (`saiten_eta`)
  ohne Biegung     ohne Biegesteife (k^2 = w^2 m / T), der Verlust ohne Biegeanteil
  Fenster 16 s     dieselbe Quelle auf 16 s mit s0 0,5 (s0 * Dauer = 8 wie der Pruefstand): was vom Ende der Quelle
                   ueber das Fenster reicht, faellt um e^(-8) gedaempft auf den Anfang
  Rate x4          dieselbe Quelle bei 192 kHz (eine Bandgrenze bei 24 kHz zeigte sich als Vorlaeufer, der mit der Rate
                   faellt; gemessen an denselben Zeiten)
"""
import importlib.util
import pathlib

import numpy as np

P = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("bezug", P / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps = bz.ps

ETA = ps.saiten_eta
V = 0.3


WELLEN = ps.saite_wellen


def ideal_wellen(saite, w):
    """Wie `saite_wellen` ohne Biegesteife: k^2 = w^2 m / T, der Verlust wie die Vorgabe (ohne Biegeanteil)."""
    T, m = saite["T"], saite["m"]
    k2 = w ** 2 * m / T
    k = np.sqrt(k2)
    eta = ps.saiten_eta(dict(saite, EI=0.0), w, k2)
    k = k * (1 - 0.5j * eta)
    return k, T * k / w


def quelle_zeit(e, instrument, fs=48000.0, breite=None, dauer=None):
    saiten, laengen, stegtaste, xp, d, winkel, b, tempo, neigung = e
    n = int((dauer or ps.DAUER) * fs)
    s0 = ps.S0 * ps.DAUER / (dauer or ps.DAUER)
    w = 2 * np.pi * np.fft.rfftfreq(n, 1 / fs) - 1j * s0
    g = bz.quelle(saiten[0], laengen[0], xp, d, b if breite is None else breite, tempo, neigung, w, kipp=bz.KIPP[instrument])
    x = np.fft.irfft(g * np.exp(-1j * w * V), n) * fs
    return x * np.exp(s0 * np.arange(n) / fs), fs


def vorlaeufer(x, fs):
    spitze, energie = np.abs(x).max(), np.sum(x ** 2)
    aus = []
    for vor in (0.002, 0.02):
        s = x[:int((V - vor) * fs)]
        aus.append(f"{20 * np.log10(np.abs(s).max() / spitze + 1e-300):7.1f} / {10 * np.log10(np.sum(s ** 2) / energie + 1e-300):7.1f}")
    return "  ".join(aus)


def main():
    faelle = [("laute_38", "laute", 38), ("cembalo_84", "cembalo", 84), ("cembalo_89", "cembalo", 89), ("gitarre_57", "gitarre", 57),
              ("harfe_60", "harfe", 60)]
    print("Spitze / Energie vor -2 ms  vor -20 ms (dB gegen die ganze Quelle)")
    for name, instrument, taste in faelle:
        e = bz.eingaben(instrument, taste, 0.0)
        zeilen = [("Vorgabe", lambda: quelle_zeit(e, instrument))]
        zeilen.append(("ohne Gauss", lambda: quelle_zeit(e, instrument, breite=0.0)))

        def ohne_verlust():
            ps.saiten_eta = lambda saite, w, k2: 0.0 * w
            try:
                return quelle_zeit(e, instrument)
            finally:
                ps.saiten_eta = ETA
        zeilen.append(("ohne Verlust", ohne_verlust))

        def ohne_biegung():
            ps.saite_wellen = ideal_wellen
            try:
                return quelle_zeit(e, instrument)
            finally:
                ps.saite_wellen = WELLEN
        zeilen.append(("ohne Biegung", ohne_biegung))
        zeilen.append(("Rate x4", lambda: quelle_zeit(e, instrument, fs=192000.0)))
        zeilen.append(("Fenster 16 s (s0 0,5)", lambda: quelle_zeit(e, instrument, dauer=16.0)))

        def ohne_alles():
            ps.saiten_eta = lambda saite, w, k2: 0.0 * w
            try:
                ps.saite_wellen = ideal_wellen
                return quelle_zeit(e, instrument, breite=0.0)
            finally:
                ps.saiten_eta = ETA
                ps.saite_wellen = WELLEN
        zeilen.append(("ohne Gauss, Verlust, Biegung", ohne_alles))
        for art, fn in zeilen:
            x, fs = fn()
            print(f"  {name:11s} {art:30s} {vorlaeufer(x, fs)}", flush=True)


if __name__ == "__main__":
    main()
