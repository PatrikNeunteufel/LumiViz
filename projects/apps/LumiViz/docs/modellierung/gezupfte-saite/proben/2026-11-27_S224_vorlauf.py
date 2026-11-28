"""S224, VL1 a — wie weit die Quelle des Zupfs vor dem Loslassen reicht, und die Neigung je Stufe minimalphasig.

Stand vor VL7 a (S224): seit VL7 a nehmen `quelle` und `stegkraft` des Bezugs die kausale Wellenzahl (`wellen_kausal`);
die Zahlen dieser Probe stehen in der .txt daneben, ein neuer Lauf rechnet mit dem kausalen Verlust.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>

Die Probe nach VL1 a (Kern: `test_InstrumentModelsZupfBezug` „Vorlauf der Note (VL1 a)“, `LUMIVIZ_VL1_MESSUNG=1`) zeigte:
die Neigung je Stufe (GZ6 a) als reeller Faktor 10^(Stufe k lb(f / 1 kHz) / 20) ist nullphasig, mit dem Knick bei 20 Hz
reicht die Quelle bei leisen Stufen weit vor das Loslassen (Gitarre Stufe -40: vor -2 ms noch -23 dB der Spitze, vor
-200 ms -86 dB). Hier je Fall und Stufe:

  (1) Vorlaeufer der Quelle g (eine Saite, Polarisation senkrecht ohne Winkel) mit der Neigung nullphasig und
      minimalphasig (derselbe Betrag, Phase aus dem Cepstrum): Spitze und Energie vor -V gegen die ganze Quelle,
      V = 2 / 20 / 200 ms;
  (2) die Zielgroessen des Bezugs (`2026-11-26_S222_bezug_zupf.py`, `rechne`) mit der Neigung minimalphasig gegen
      nullphasig: Teiltoene der Stegkraft (Pegel frueh / spaet), Pegel im Band (PK1 a), Lautheit am Hoerort und Pegel
      1-2 s gegen 0-0,4 s.

Stand vor VL5 a: die Probe tauscht `neigung_faktor` des Bezugs; seit VL5 a nimmt die Quelle `quellfilter` (Neigung und
Bandgrenze), die Probe laeuft so nicht mehr wie damals — ihre Zahlen stehen in `2026-11-27_S224_vorlauf.txt`.

Die Minimalphase: log|H| auf einem feinen Raster der reellen Achse (2^21 Werte), reelles Cepstrum gefaltet, h(t) kausal;
auf der Geraden w - i s0 dann H = FFT(h(t) e^(-s0 t)) wie die Quelle selbst.
"""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path(__file__).resolve().parents[7]
P = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("bezug", P / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps = bz.ps

NULLPHASIG = bz.neigung_faktor
NFEIN = 1 << 21


def minimalphasig_zeit(stufe, kipp):
    """h(t) der Neigung minimalphasig, kausal, auf NFEIN Werten bei FS."""
    f = np.fft.rfftfreq(NFEIN, 1 / ps.FS)
    betrag = 10 ** (stufe * kipp * np.log2(np.maximum(f, 20.0) / 1000.0) / 20)
    c = np.fft.irfft(np.log(betrag), NFEIN)
    falte = np.zeros(NFEIN)
    falte[0] = c[0]
    falte[1:NFEIN // 2] = 2 * c[1:NFEIN // 2]
    falte[NFEIN // 2] = c[NFEIN // 2]
    H = np.exp(np.fft.rfft(falte))
    return np.fft.irfft(H, NFEIN)


_merk = {}


def minimalphasig(w, stufe, kipp):
    """Wie `neigung_faktor`, minimalphasig: an den Punkten w (der Geraden w - i s0 des Pruefstands)."""
    if stufe == 0.0 or kipp == 0.0:
        return 1.0
    schluessel = (len(w), stufe, kipp)
    if schluessel not in _merk:
        n = len(w) * 2 - 2 if len(w) > 1 else 1
        h = minimalphasig_zeit(stufe, kipp)[:n]
        t = np.arange(n) / ps.FS
        _merk[schluessel] = np.fft.rfft(h * np.exp(-ps.S0 * t), n)   # h je Abtastwert: Summe h e^(-i w t) ist der Faktor
    return _merk[schluessel]


def quelle_zeit(fall_eingaben, instrument, neigung_fn, vorlauf):
    saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = fall_eingaben
    bz.neigung_faktor = neigung_fn
    w = ps.frequenzen()
    g = bz.quelle(saiten[0], laengen[0], xp, d, breite, tempo, neigung, w, kipp=bz.KIPP[instrument])
    bz.neigung_faktor = NULLPHASIG
    n = int(ps.DAUER * ps.FS)
    x = np.fft.irfft(g * np.exp(-1j * w * vorlauf), n) * ps.FS
    return x * np.exp(ps.S0 * np.arange(n) / ps.FS)


def vorlaeufer(x, vorlauf):
    spitze, energie = np.abs(x).max(), np.sum(x ** 2)
    aus = []
    for V in (0.002, 0.02, 0.2):
        s = x[:int((vorlauf - V) * ps.FS)]
        aus.append((20 * np.log10(np.abs(s).max() / spitze + 1e-300), 10 * np.log10(np.sum(s ** 2) / energie + 1e-300)))
    return aus


def zeilen(name, instrument, taste, stufe, geladen, neigung_fn):
    bz.neigung_faktor = neigung_fn
    aus = []
    bz.rechne(name, instrument, taste, stufe, *geladen, ps.frequenzen(), aus)
    bz.neigung_faktor = NULLPHASIG
    werte = {"teilton": []}
    for z in aus:
        t = z.split()
        if t[0] == "teilton":
            werte["teilton"].append([float(v) for v in t[2:5]])
        elif t[0] in ("kraftband", "hoerort"):
            werte[t[0]] = [float(v) for v in t[1:]]
    return werte


def main():
    faelle = [("gitarre_57", "gitarre", 57, 0.0), ("gitarre_57_pp", "gitarre", 57, -18.0), ("gitarre_57_m40", "gitarre", 57, -40.0),
              ("gitarre_40_m40", "gitarre", 40, -40.0), ("gitarre_83_m40", "gitarre", 83, -40.0),
              ("laute_38", "laute", 38, 0.0), ("cembalo_84", "cembalo", 84, 0.0), ("cembalo_89", "cembalo", 89, 0.0),
              ("harfe_60", "harfe", 60, 0.0)]
    geladen = {}
    print("(1) Vorlaeufer der Quelle: Spitze / Energie vor -2 / -20 / -200 ms gegen die ganze Quelle (dB), Vorlauf 0,3 s")
    for name, instrument, taste, stufe in faelle:
        e = bz.eingaben(instrument, taste, stufe)
        teile = []
        for art, fn in (("nullphasig", NULLPHASIG), ("minimalphasig", minimalphasig)):
            x = quelle_zeit(e, instrument, fn, 0.3)
            teile.append(f"{art} " + "  ".join(f"{a:7.1f} / {b:7.1f}" for a, b in vorlaeufer(x, 0.3)))
        print(f"  {name:16s} " + " | ".join(teile), flush=True)
    print("(2) Zielgroessen des Bezugs, minimalphasig minus nullphasig (nur Faelle mit Neigung)")
    for name, instrument, taste, stufe in faelle:
        if stufe == 0.0 or bz.KIPP[instrument] == 0.0:
            continue
        if instrument not in geladen:
            koerper = bz.KOERPER[instrument]
            import json
            geladen[instrument] = (ps.lade_koerper(koerper), json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8")))
        a = zeilen(name, instrument, taste, stufe, geladen[instrument], NULLPHASIG)
        b = zeilen(name, instrument, taste, stufe, geladen[instrument], minimalphasig)
        staerkster = max(t[1] for t in a["teilton"])
        dfr = [abs(y[1] - x[1]) for x, y in zip(a["teilton"], b["teilton"]) if x[1] >= staerkster - 60]
        dsp = [abs(y[2] - x[2]) for x, y in zip(a["teilton"], b["teilton"]) if x[1] >= staerkster - 60 and x[2] > x[1] - 60]
        dct = [abs(1200 * np.log2(y[0] / x[0])) for x, y in zip(a["teilton"], b["teilton"]) if x[1] >= staerkster - 60]
        print(f"  {name:16s} Teiltoene (bis 60 dB unter dem staerksten): Lage {max(dct):.3f} Cent, frueh {max(dfr):.3f} dB, "
              f"spaet {max(dsp) if dsp else float('nan'):.3f} dB | Band " + " / ".join(f"{y - x:+.3f}" for x, y in zip(a["kraftband"], b["kraftband"]))
              + f" dB | Hoerort {b['hoerort'][0] - a['hoerort'][0]:+.3f} LU, spaet/frueh {b['hoerort'][1] - a['hoerort'][1]:+.3f} dB", flush=True)


if __name__ == "__main__":
    main()
