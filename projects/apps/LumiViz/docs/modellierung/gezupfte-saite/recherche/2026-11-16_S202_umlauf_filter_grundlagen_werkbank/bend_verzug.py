"""LVR Auftrag 11, Punkt 3: Saite mit laufender Laenge, +1 Halbton in 50 ms, gehalten, -1 Halbton in 50 ms (fs 48 kHz).

Schleife (starrer Steg, eine Polarisation): v[n] = Verlust( lese(v, n - D(n)) ) + Anregung[n]
  Verlust   Ein-Pol H = b/(1 + a z^-1) nach Gl. (V2)/(V3), an der Anfangsnote entworfen und fest (wie S200)
  D(n)      fs / f(n) - Phasenverzug des Verlusts bei f(n): die Stimmung folgt der Soll-Tonhoehe Takt fuer Takt
  lese      lagrange N (N = 1, 3, 5; Fenster um den Bruchteil zentriert), thiran1 (Allpass 1. Ordnung, Delta in
            [0,5; 1,5), Beiwert jeden Takt neu, beim Wechsel des ganzzahligen Teils ohne Korrektur), Bezug: Sinc mit
            Kaiser-Fenster, 64 Werte (beta 8)
Gemessen gegen den Bezug (gleiche Physik, gleiche Messung): Tonhoehe aus den Teiltoenen 1-3 (komplexe Demodulation am
Soll-Verlauf, 10-ms-Hann, Median) in Cent; Huellkurve (5-ms-RMS) in dB; Energie der Differenz ueber 10 kHz in den Rampen
relativ zum Signal (dB). Dazu statisch: Dispersions- und Verlustfilter ueber +-2 Halbtoene fest gegen neu entworfen.
Aufruf: python -B bend_verzug.py
"""
import sys
sys.dont_write_bytecode = True
import importlib.util
import pathlib

import numpy as np
import scipy.signal as sg

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ue", HIER / "umlauf_entwurf.py")
ue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ue)
ps, FS = ue.ps, ue.FS

N_T = int(0.6 * FS)
t = np.arange(N_T) / FS


def verlauf():
    hoch = np.clip((t - 0.15) / 0.05, 0, 1) - np.clip((t - 0.35) / 0.05, 0, 1)
    return hoch        # Halbtoene, linear in Cent


def gewichte(art, d):
    """-> (Basisverzug, Gewichte ueber Verzuege base .. base + len - 1)"""
    if art.startswith("lagrange"):
        N = int(art[8:])
        base = int(np.floor(d)) - (N - 1) // 2
        x = d - base
        h = np.ones(N + 1)
        for k in range(N + 1):
            for m in range(N + 1):
                if m != k:
                    h[k] *= (x - m) / (k - m)
        return base, h
    if art == "sinc":
        halb = min(32, int(np.floor(d)) - 1)            # kausal: kein Wert mit Verzug < 1
        base = int(np.floor(d)) - (halb - 1)
        m = np.arange(2 * halb)
        x = d - base - m
        fenster = np.i0(8.0 * np.sqrt(np.clip(1 - (x / halb) ** 2, 0, 1))) / np.i0(8.0)   # Kaiser am Abstand x
        return base, np.sinc(x) * fenster
    raise ValueError(art)


def laufe(sa, L0, f0, art, bend=True):
    Gt, ph = ue.ziel(sa, L0)
    fn, _ = ue.teiltoene(Gt, ph)
    b, a, _ = ue.pol1_passiv(Gt, fn[0], min(8 * fn[0], 0.45 * FS))
    soll = f0 * 2 ** ((verlauf() if bend else 0 * t) / 12)
    x = 2 * np.pi * soll / FS
    tau_l = -np.angle(b / (1 + a * np.exp(-1j * x))) / x
    D = FS / soll - tau_l
    puf = np.zeros(N_T + 200)
    O = 100
    anr = np.exp(-0.5 * ((t - 0.002) / 0.00008) ** 2)
    anr -= np.exp(-0.5 * ((t - 0.002 - 0.3 / soll[0]) / 0.00008) ** 2)      # Zupf bei 0,15 der Laenge (Rueckweg)
    v = np.zeros(N_T)
    y = 0.0
    x1 = 0.0
    yap = 0.0
    for n in range(N_T):
        d = D[n]
        if art == "thiran1":
            M = int(np.floor(d - 0.5))
            De = d - M
            eta = (1 - De) / (1 + De)
            xin = puf[O + n - M]
            xprev = puf[O + n - M - 1]
            yap = eta * xin + xprev - eta * yap
            gel = yap
        else:
            base, h = gewichte(art, d)
            idx = O + n - base - np.arange(len(h))
            gel = h @ puf[idx]
        y = b * gel - a * y
        v[n] = y + anr[n]
        puf[O + n] = v[n]
    return v, soll


def cent(v, soll, nr):
    phase = 2 * np.pi * np.cumsum(nr * soll) / FS
    zz = v * np.exp(-1j * phase)
    m = int(0.010 * FS)
    zz = np.convolve(zz, np.hanning(m) / np.hanning(m).sum(), mode="same")
    df = np.gradient(np.unwrap(np.angle(zz))) * FS / (2 * np.pi)
    return 1200 * np.log2(np.clip((nr * soll + df) / (nr * soll), 0.5, 2.0))


def tonhoehe(v, soll):
    return np.median([cent(v, soll, k) for k in (1, 2, 3)], axis=0)


def huelle(v):
    m = int(0.005 * FS)
    return 10 * np.log10(np.convolve(v * v, np.ones(m) / m, mode="same") + 1e-30)


HP = sg.butter(8, 10000, "highpass", fs=FS, output="sos")


def fall(name, sa, L0, f0):
    ref, soll = laufe(sa, L0, f0, "sinc")
    ref_fest, _ = laufe(sa, L0, f0, "sinc", bend=False)
    c_ref = tonhoehe(ref, soll)
    h_ref = huelle(ref)
    s = lambda a_, b_: slice(int(a_ * FS), int(b_ * FS))
    print(f"\n{name}: f0 {f0:.1f} Hz, Verzug {FS / f0:.1f} -> {FS / f0 / 2 ** (1 / 12):.1f} Takte in 50 ms "
          f"(Aenderung {FS / f0 * (1 - 2 ** (-1 / 12)) / (0.05 * FS):.4f} Takte je Takt)")
    print(f"  Bezug (Sinc 64): Tonhoehe gegen Soll gehalten {np.mean(c_ref[s(0.08, 0.14)]):+.2f} Cent, in der Rampe hoch groesster Betrag "
          f"{np.abs(c_ref[s(0.15, 0.2)]).max():.1f} Cent (Messung mit 10-ms-Fenster laeuft nach), oben {np.mean(c_ref[s(0.25, 0.34)]):+.2f} Cent; "
          f"Pegel nach dem Bend gegen ohne Bend {np.mean(h_ref[s(0.45, 0.55)]) - np.mean(huelle(ref_fest)[s(0.45, 0.55)]):+.2f} dB")
    print("  Art       | Cent gegen Bezug: gehalten / Rampe hoch max / oben / Rampe runter max / danach | Huelle dB gegen Bezug: max in Rampen / danach Mittel | Differenz >10 kHz in den Rampen dB re Signal")
    for art in ("lagrange1", "lagrange3", "lagrange5", "thiran1"):
        v, _ = laufe(sa, L0, f0, art)
        c = tonhoehe(v, soll) - c_ref
        h = huelle(v) - h_ref
        rm = np.zeros(N_T, bool)
        rm[s(0.15, 0.205)] = True
        rm[s(0.35, 0.405)] = True
        e = sg.sosfilt(HP, v - ref)
        hf = 10 * np.log10(np.sum(e[rm] ** 2) / np.sum(ref[rm] ** 2) + 1e-30)
        print(f"  {art:9s} | {np.mean(c[s(0.08, 0.14)]):+6.2f} / {np.abs(c[s(0.15, 0.21)]).max():5.2f} / {np.mean(c[s(0.25, 0.34)]):+6.2f} / "
              f"{np.abs(c[s(0.35, 0.41)]).max():5.2f} / {np.mean(c[s(0.45, 0.55)]):+6.2f} | {np.abs(h[rm]).max():5.2f} / {np.mean(h[s(0.45, 0.55)]):+6.2f} | {hf:7.1f}")


def statisch(name, sa, L0):
    """Dispersion K16 und Verlust pol1p ueber +-2 Halbtoene: fest (Entwurf bei L0) gegen neu entworfen."""
    print(f"\n{name}: Filter fest (Entwurf an der Anfangsnote) gegen je Laenge neu entworfen, Teiltoene mit sigma <= 100 1/s")
    a0 = ue.entwurf(sa, L0, "pol1p", ("K", 16, 5000.0), ("thiran", 1))[4]
    for ht in (-2, -1, 1, 2):
        L = L0 * 2 ** (-ht / 12)
        neu = ue.entwurf(sa, L, "pol1p", ("K", 16, 5000.0), ("thiran", 1))
        rn = ue.bewerte(*neu[:4])
        # fest: dieselben Filterwerte wie an L0, nur der Verzug neu (Grundton gestimmt)
        Gt, ph = ue.ziel(sa, L)
        fn, _ = ue.teiltoene(Gt, ph)
        hl = ue.antwort_pol1(*a0["koef"])
        hd = ue.ap1(a0["a_ap"], a0["K"])
        rest = ph * 0 + ue.phase_von(hl * hd)
        w1 = 2 * np.pi * fn[0] / FS
        Dtot = (np.interp(fn[0], ue.F, ph) - np.interp(fn[0], ue.F, rest)) / w1
        phm = rest + ue.W * Dtot
        rf = ue.bewerte(Gt, ph, np.abs(hl * hd), phm)
        print(f"  {ht:+d} Halbtoene: a_ap neu {neu[4]['a_ap']:+.4f} (fest {a0['a_ap']:+.4f}) | Cent max neu {rn['cr']:5.1f} / fest {rf['cr']:5.1f} | "
              f"Abklingen % max neu {rn['pr']:5.1f} / fest {rf['pr']:5.1f} | Grundton-Abklingen neu {rn['p1']:+5.1f} / fest {rf['p1']:+5.1f} %")


if __name__ == "__main__":
    g1 = ps.GITARRE[5]
    fall("Gitarre Saite 1 leer (E4)", g1, ps.MENSUR, ps.hz_von(64))
    g3 = ps.GITARRE[3]
    fall("Gitarre Saite 3 leer (G3, EI 310e-6)", g3, ps.MENSUR, ps.hz_von(55))
    sa, L, *_ = ps.harfe_taste(60)
    fall("Harfe Taste 60 (Darm, C4)", sa, L, ps.hz_von(60))
    sa96, L96, *_ = ps.harfe_taste(96)
    fall("Harfe Taste 96 (Nylon, C7)", sa96, L96, ps.hz_von(96))
    statisch("Gitarre Saite 3", g3, ps.MENSUR)
    statisch("Harfe Taste 60", sa, L)
