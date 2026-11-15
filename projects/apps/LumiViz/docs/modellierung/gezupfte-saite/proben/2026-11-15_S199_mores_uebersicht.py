"""Mores 2021: Uebersicht ueber alle 65 Gitarren — Guete der Rohdaten und tiefe Moden je Schlagort.

Aufruf: python -I mores_uebersicht.py <qualified_selected_impulses.mat> <ziel.tsv> [<bild.png> g08,g09,...]

Je Gitarre und Schlagort (bass, center, treble) mit dem Aufnehmer derselben Seite (center: beide):
  Kraftspitze N, Zweitschlag dB, Grenze des Hammers (-10 dB der Kraft), Rauschabstand der Beschleunigung,
  A0 (70-140 Hz) und T1 (150-270 Hz): Frequenz, |Y| in m/Ns, Guete aus der -3-dB-Breite.
Einheiten nach batch_extract_bridge_mobility_archive.m (Empfindlichkeiten der Datenblaetter).
"""
import sys

import numpy as np
import scipy.io as sio

FS = 48000
N_VOLL = (10000 / 92.90) * 4.4482       # N je Vollausschlag
A_VOLL = (10000 / 10.64) * 9.80665      # m/s2 je Vollausschlag
ORTE = ("bass", "center", "treble")


def frf(x, kanal, n=FS):
    """Mobilitaet Y = A / (i w F) aus einem Schlag; Fenster: halber Kosinus ueber 1 s (wie Mores)."""
    fenster = 0.5 * np.cos(np.arange(1, n + 1) * np.pi / n) + 0.5
    f = np.fft.rfft(x[:n, 0] * fenster * N_VOLL)
    a = np.fft.rfft(x[:n, kanal] * fenster * A_VOLL)
    hz = np.fft.rfftfreq(n, 1 / FS)
    with np.errstate(divide="ignore", invalid="ignore"):
        y = a / (1j * 2 * np.pi * hz * f)
    y[0] = 0
    return hz, y, f, a


def spitze(hz, y, von, bis):
    sel = (hz >= von) & (hz <= bis)
    i = np.argmax(np.abs(y[sel])) + np.argmax(sel)
    p = np.abs(y[i])
    # -3-dB-Breite, linear interpoliert
    def rand(schritt):
        k = i
        while 0 < k + schritt < len(y) - 1 and np.abs(y[k + schritt]) > p / np.sqrt(2):
            k += schritt
        a, b = np.abs(y[k]), np.abs(y[k + schritt])
        t = (a - p / np.sqrt(2)) / (a - b) if a != b else 0.0
        return hz[k] + schritt * t * (hz[1] - hz[0])
    breite = rand(1) - rand(-1)
    return hz[i], p, (hz[i] / breite if breite > 0 else float("nan"))


def main():
    d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"]
    zeilen = ["g\tort\tkanal\tkraft_N\tzweit_dB\thammer_-10dB_Hz\trausch_dB\tA0_Hz\tA0_Y\tA0_Q\tT1_Hz\tT1_Y\tT1_Q\tY_1-4k_dB"]
    kurven = {}
    for g in range(d.shape[0]):
        for o, ort in enumerate(ORTE):
            x = d[g, o * FS:(o + 1) * FS, :]
            kraft = x[:, 0] * N_VOLL
            ip = int(np.argmax(np.abs(kraft)))
            fp = abs(kraft[ip])
            # Zweitschlag: groesster Betrag der Kraft ab 3 ms nach der Spitze
            nach = np.abs(kraft[ip + int(0.003 * FS):ip + int(0.2 * FS)])
            zweit = 20 * np.log10(nach.max() / fp) if len(nach) and fp > 0 else float("nan")
            for kanal, name in ((1, "treble"), (2, "bass")):
                if ort != "center" and name != ort:
                    continue
                hz, y, f, a = frf(x, kanal)
                fa = np.abs(f)
                bezug = np.median(fa[(hz > 100) & (hz < 500)])
                unter = np.where((hz > 500) & (fa < bezug * 10 ** (-10 / 20)))[0]
                grenze = hz[unter[0]] if len(unter) else float("nan")
                # Rauschabstand: Beschleunigung in den ersten 0,5 s gegen die letzten 0,1 s
                sig = x[:, kanal]
                rausch = 20 * np.log10(np.sqrt(np.mean(sig[ip:ip + FS // 10] ** 2)) / (np.sqrt(np.mean(sig[-FS // 10:] ** 2)) + 1e-30))
                a0 = spitze(hz, y, 70, 140)
                t1 = spitze(hz, y, 150, 270)
                hoch = 10 * np.log10(np.mean(np.abs(y[(hz >= 1000) & (hz <= 4000)]) ** 2))
                zeilen.append(f"g{g + 1:02d}\t{ort}\t{name}\t{fp:.1f}\t{zweit:.1f}\t{grenze:.0f}\t{rausch:.1f}\t"
                              f"{a0[0]:.0f}\t{a0[1]:.4f}\t{a0[2]:.1f}\t{t1[0]:.0f}\t{t1[1]:.4f}\t{t1[2]:.1f}\t{hoch:.1f}")
                kurven[(g + 1, ort, name)] = (hz, np.abs(y))
    open(sys.argv[2], "w", encoding="utf-8", newline="\n").write("\n".join(zeilen) + "\n")
    print(len(zeilen) - 1, "Zeilen ->", sys.argv[2])

    if len(sys.argv) > 4:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        wahl = [int(s.strip("g")) for s in sys.argv[4].split(",")]
        fig, achsen = plt.subplots(len(wahl), 1, figsize=(11, 2.6 * len(wahl)), sharex=True)
        for ax, g in zip(np.atleast_1d(achsen), wahl):
            for (ort, name), stil in ((("bass", "bass"), "b-"), (("treble", "treble"), "r-"), (("center", "treble"), "r:"),
                                      (("center", "bass"), "b:")):
                hz, y = kurven[(g, ort, name)]
                ax.loglog(hz, y, stil, lw=0.8, label=f"hit {ort} / sensor {name}")
            ax.set_xlim(60, 6000)
            ax.set_ylim(1e-3, 1)
            ax.grid(True, which="both", alpha=0.3)
            ax.set_ylabel(f"g{g:02d}  |Y| m/Ns")
        np.atleast_1d(achsen)[0].legend(fontsize=7, ncol=4)
        np.atleast_1d(achsen)[-1].set_xlabel("Hz")
        fig.tight_layout()
        fig.savefig(sys.argv[3], dpi=90)
        print("Bild ->", sys.argv[3])


main()
