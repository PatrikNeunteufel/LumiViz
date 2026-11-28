"""S224, VL4 a — die zwei Ursachen des Vorlaeufers bei Stufe 0 kausal gemacht, mit Zahlen fuer die Vorlage VL5.

Stand vor VL7 a (S224): seit VL7 a nehmen `quelle` und `stegkraft` des Bezugs die kausale Wellenzahl (`wellen_kausal`);
die Zahlen dieser Probe stehen in der .txt daneben, ein neuer Lauf rechnet mit dem kausalen Verlust.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>

Ursachen (`2026-11-27_S224_vorlaeufer_ursache.txt`): am Cembalo die Bandgrenze bei 24 kHz (bei 192 kHz faellt der
Vorlaeufer um 40 dB), an Laute, Gitarre, Harfe der Verlust der Saite (ohne ihn Laute 38 -68,5 -> -85,4 dB).

(1) Bandgrenze minimalphasig: die Quelle mal ein kausaler Tiefpass, Betrag 1 bis `F_DURCH`, darueber mit einem halben
    Kosinus bis 24 kHz auf `BODEN_DB` (Phase aus dem Cepstrum wie die Neigung, VL2 a). Gemessen: Vorlaeufer vor -2 / -20 ms
    (Spitze / Energie gegen die ganze Quelle) und die Aenderung am Betrag der Quelle bis 16 / 20 kHz.
(2) Verlust kausal (Kramers-Kronig): der Umlauf E = exp(-2 i k L) hat den Betrag exp(-2 L Im k); seine minimalphasige
    Phase (Hilbert von ln|E| ueber das Cepstrum) ist die Dispersion, die zu diesem Verlust gehoert und im heutigen Modell
    fehlt. Daraus je Teilton n die Verschiebung der Lage: d f_n = -d phi(f_n) / (2 pi tau_g(f_n)), tau_g die Laufzeit
    des Umlaufs; berichtet in Cent gegen Teilton 1 (der Grundton wird ohnehin gestimmt).
"""
import importlib.util
import pathlib

import numpy as np

P = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ursache", P / "2026-11-27_S224_vorlaeufer_ursache.py")
ur = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ur)
bz, ps = ur.bz, ur.ps

F_DURCH = 20000.0
BODEN_DB = -120.0
NFEIN = 1 << 21


def minimalphasig(log_betrag, n):
    """h (n Werte) des kausalen Filters zu ln|H| auf NFEIN Werten der reellen Achse."""
    c = np.fft.irfft(log_betrag, NFEIN)
    c[1:NFEIN // 2] *= 2
    c[NFEIN // 2 + 1:] = 0.0
    return np.fft.irfft(np.exp(np.fft.rfft(c)), NFEIN)[:n]


def tiefpass_h(n, fs=48000.0):
    f = np.fft.rfftfreq(NFEIN, 1 / fs)
    x = np.clip((f - F_DURCH) / (fs / 2 - F_DURCH), 0.0, 1.0)
    db = BODEN_DB * 0.5 * (1 - np.cos(np.pi * x))
    return minimalphasig(db / 20 * np.log(10.0), n)


def bandgrenze():
    print(f"(1) Bandgrenze minimalphasig (Betrag 1 bis {F_DURCH:.0f} Hz, bis 24 kHz auf {BODEN_DB:.0f} dB)")
    print("    Vorlaeufer Spitze / Energie vor -2 ms  vor -20 ms (dB) | Betrag der Quelle: groesste Aenderung bis 16 / 20 kHz (dB)")
    for name, instrument, taste in [("cembalo_59", "cembalo", 59), ("cembalo_84", "cembalo", 84), ("cembalo_89", "cembalo", 89),
                                    ("laute_38", "laute", 38), ("harfe_60", "harfe", 60)]:
        e = bz.eingaben(instrument, taste, 0.0)
        saiten, laengen, stegtaste, xp, d, winkel, b, tempo, neigung = e
        n = int(ps.DAUER * ps.FS)
        w = 2 * np.pi * np.fft.rfftfreq(n, 1 / ps.FS) - 1j * ps.S0
        g = bz.quelle(saiten[0], laengen[0], xp, d, b, tempo, neigung, w, kipp=bz.KIPP[instrument])
        h = tiefpass_h(n)
        T = np.fft.rfft(h * np.exp(-ps.S0 * np.arange(n) / ps.FS), n)
        aus = []
        for art, G in (("Vorgabe", g), ("Tiefpass", g * T)):
            x = np.fft.irfft(G * np.exp(-1j * w * ur.V), n) * ps.FS * np.exp(ps.S0 * np.arange(n) / ps.FS)
            aus.append(f"{art} {ur.vorlaeufer(x, ps.FS)}")
        f = np.fft.rfftfreq(n, 1 / ps.FS)
        d16 = np.abs(20 * np.log10(np.abs(T[(f > 20) & (f <= 16000)]))).max()
        d20 = np.abs(20 * np.log10(np.abs(T[(f > 20) & (f <= 20000)]))).max()
        print(f"  {name:11s} " + " | ".join(aus) + f" | {d16:.4f} / {d20:.4f}", flush=True)


def dispersion():
    print("(2) Verlust kausal: Verschiebung der Teiltoene 2-12 gegen Teilton 1 (Cent), dazu der groesste Betrag")
    for name, instrument, taste in [("laute_38", "laute", 38), ("laute_58", "laute", 58), ("gitarre_57", "gitarre", 57),
                                    ("harfe_36", "harfe", 36), ("harfe_60", "harfe", 60), ("oud_72", "oud", 72),
                                    ("cembalo_60", "cembalo", 60), ("cembalo_89", "cembalo", 89)]:
        e = bz.eingaben(instrument, taste, 0.0)
        sa, L = e[0][0], e[1][0]
        f = np.fft.rfftfreq(NFEIN, 1 / ps.FS)
        w = 2 * np.pi * np.maximum(f, 0.5)
        k, _ = ps.saite_wellen(sa, w.astype(complex))
        ln_betrag = 2 * L * k.imag                # Im k < 0 (k (1 - i eta / 2)): |E| = exp(2 L Im k)
        c = np.fft.irfft(ln_betrag, NFEIN)
        c[1:NFEIN // 2] *= 2
        c[NFEIN // 2 + 1:] = 0.0
        phase = np.fft.rfft(c).imag              # minimalphasige Phase zu ln|E|
        k_re = k.real
        tau = 2 * L * np.gradient(k_re, w)       # Laufzeit des Umlaufs
        f0 = 440.0 * 2 ** ((taste - 69) / 12)
        # die Teiltoene des Modells: 2 L Re k(f_n) = 2 pi n
        fn = []
        for nn in range(1, 13):
            ziel = 2 * np.pi * nn
            i = int(np.searchsorted(2 * L * k_re, ziel))
            if i >= len(f) - 1 or f[i] > 20000:
                break
            fn.append(f[i - 1] + (ziel - 2 * L * k_re[i - 1]) / (2 * L * (k_re[i] - k_re[i - 1])) * (f[i] - f[i - 1]))
        cent = []
        for x in fn:
            i = int(round(x * NFEIN / ps.FS))
            df = phase[i] / (2 * np.pi * tau[i])   # die Phase dreht wie eine laengere Laufzeit: Teilton tiefer bei positiver Phase
            cent.append(1200 * np.log2(1 - df / x))
        rel = [c_ - cent[0] for c_ in cent]
        print(f"  {name:11s} Grundton {fn[0]:8.2f} Hz (Soll {f0:8.2f}) | " + " ".join(f"{r:+.2f}" for r in rel[1:])
              + f" | groesster {max(abs(r) for r in rel):.2f} Cent", flush=True)


if __name__ == "__main__":
    bandgrenze()
    dispersion()
