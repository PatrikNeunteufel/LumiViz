"""S199, 34k-5 — die Gitarre (g34, Mores 2021) ueber 5 kHz: was traegt die Messung, was gibt der Koerper heute.

Aufruf: python -B 2026-11-15_S199_mores_ueber_5khz.py <qualified_selected_impulses.mat> <classical_guitar.lvinstbody>

Teil 1  Anregung und Stoerabstand je Terzband bis 20 kHz: Pegel der Hammerkraft gegen 1 kHz; Signal (0-150 ms nach
        dem Schlag) gegen Rauschen (die letzten 150 ms der Sekunde und die Zeit vor dem Schlag) je Kanal.
Teil 2  Gemessener Terzpegel von p / F (Mikrofone bass, treble) und a / F (Aufnehmer) von 1 bis 20 kHz.
Teil 3  Der Koerper von heute (75 Moden bis 5 kHz) an denselben Orten: Terzpegel und Abstand zur Messung.

Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.io as sio

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("make_gitarre", HIER.parents[7] / "asset/loop/instrument_bodies/make_gitarre.py")
mg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mg)

FS = mg.FS
MITTEN = [1000 * 2 ** (k / 3) for k in range(0, 14)]          # 1 kHz ... 20 kHz


def terz(hz, y):
    return np.array([10 * np.log10(np.mean(np.abs(y[(hz >= m / 2 ** (1 / 6)) & (hz < m * 2 ** (1 / 6))]) ** 2) + 1e-300)
                     for m in MITTEN])


def zeile(name, werte, fmt="{:6.1f}"):
    print(f"{name:<26}" + " ".join(fmt.format(v) for v in werte))


def modell(koerper, hz):
    """p / F an den drei Schlagorten und beiden Hoerorten aus der Koerperdatei."""
    f = np.array([m["f"] for m in koerper["modes"]])
    eta = np.array([m["eta"] for m in koerper["modes"]])
    steg = np.array([m["bridge"] for m in koerper["modes"]])                     # Moden x 6
    G = np.array([[complex(*g) for g in m["radiation"]] for m in koerper["modes"]])
    k = np.array([np.polyfit(mg.X_SAITEN, steg[j], 2) for j in range(len(f))])   # c, b, a
    w = 2 * np.pi * hz[:, None]
    wn = 2 * np.pi * f[None, :]
    B = -w ** 2 / (wn ** 2 - w ** 2 + 1j * eta[None, :] * wn * w)
    aus = {}
    for ort in mg.ORTE:
        phi = np.polyval(k.T, mg.X_SCHLAG[ort])
        for h, (_, mik) in enumerate(mg.MIKROFONE):
            aus[(ort, mik)] = B @ (phi * G[:, h])
    return aus


if __name__ == "__main__":
    d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"][mg.GITARRE - 1]
    koerper = json.loads(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8"))
    daten = {ort: d[o * FS:(o + 1) * FS, :] for o, ort in enumerate(mg.ORTE)}
    print("Terzmitten Hz             " + " ".join(f"{m:6.0f}" for m in MITTEN))

    print("\nTeil 1 - Anregung und Stoerabstand")
    n = int(0.15 * FS)
    hzk = np.fft.rfftfreq(n, 1 / FS)
    fen = np.hanning(n)
    for ort, x in daten.items():
        ip = int(np.argmax(np.abs(x[:, 0])))
        F = np.fft.rfft(x[ip - 48:ip - 48 + 240, 0] * mg.sig.windows.tukey(240, 0.3), FS)
        t = terz(np.fft.rfftfreq(FS, 1 / FS), F)
        zeile(f"{ort}: Kraft dB re 1 kHz", t - t[0])
        print(f"   Schlag bei Abtastwert {ip}")
        for kanal, name in ((1, "Aufn. treble"), (2, "Aufn. bass"), (4, "Mikro treble"), (5, "Mikro bass")):
            sig_ = terz(hzk, np.fft.rfft(x[ip:ip + n, kanal] * fen))
            ende = terz(hzk, np.fft.rfft(x[-n:, kanal] * fen))
            zeile(f"   {name} S/N Ende", sig_ - ende)
            if ip > n + 100:
                vor = terz(hzk, np.fft.rfft(x[ip - 100 - n:ip - 100, kanal] * fen))
                zeile(f"   {name} S/N davor", sig_ - vor)

    hz, H = mg.frfs(daten)
    print("\nTeil 2 - gemessen, Terzpegel (dB; p/F re 1 Pa/N, a/F re 1 m/s2/N)")
    mess = {}
    for ort in mg.ORTE:
        for name in ("mbass", "mtreble", "bass", "treble"):
            mess[(ort, name)] = terz(hz, H[(ort, name)])
            zeile(f"{ort} {name}", mess[(ort, name)])

    print("\nTeil 3 - der Koerper von heute (Moden bis 5 kHz), Terzpegel und Abstand zur Messung")
    syn = modell(koerper, hz)
    for _, mik in mg.MIKROFONE:
        ab = []
        for ort in mg.ORTE:
            s = terz(hz, syn[(ort, mik)])
            zeile(f"{ort} {mik} Modell", s)
            ab.append(s - mess[(ort, mik)])
        zeile(f"   {mik} Modell - Messung", np.mean(ab, axis=0))
