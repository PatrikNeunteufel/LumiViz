"""Gegenprobe S203 zu Bericht 21, Abschnitte 1c/1d/2: Betrag der Entwuerfe unabhaengig mit scipy.signal.freqz.
Vorgabe und Entwurf aus den Skripten des Laufs (bank_voll.vorgabe, wls; weg2.weg2a), Betrag selbst gemessen."""
import sys
import pathlib
sys.dont_write_bytecode = True
import numpy as np
from scipy.signal import freqz

W = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\archiv\ergebnisse\2026-11-17_21_umlauf_filter_nacharbeit")
sys.path.insert(0, str(W))
import bank_voll as bv  # noqa: E402
import weg2  # noqa: E402

ue = bv.ue
FS = bv.FS


def maxh(b, a):
    w, h = freqz(b, a, worN=1 << 18)
    i = int(np.argmax(np.abs(h)))
    return abs(h[i]), w[i] * FS / (2 * np.pi)


def fehler(b, a, fk, g):
    _, h = freqz(b, a, worN=2 * np.pi * fk / FS)
    return 100 * np.max(np.abs(np.log(np.abs(h)) / np.log(g) - 1))


ziele = ("Gitarre", "Harfe")
for titel, sa, L in ue.tasten():
    if not any(t in titel for t in ("Saite 6 Bund 0", "Taste 60 (Darm)", "Taste 46", "Saite 1 Bund 12")):
        continue
    Gt, ph = ue.ziel(sa, L)
    fn, sig = ue.teiltoene(Gt, ph)
    print(f"\n### {titel}")
    for vname, o in [("voll", bv.VOLL), ("ohne HF", dict(bv.VOLL, hf=False)),
                     ("unter f1 Ziel", dict(bv.VOLL, unter="ziel"))]:
        fk, g, Hd, gu, info = bv.vorgabe(Gt, fn, sig, o)
        for N in (4, 8):
            if 2 * N + 1 > 2 * len(fk):
                continue
            b, a, ng = bv.wls(fk, g, Hd, N)
            m, fm = maxh(b, a)
            z = f"  {vname:14s} iir{N}: W1 max|H| {m:.5f} bei {fm:7.0f} Hz (gespiegelt {ng})"
            if vname == "voll":
                b2, a2 = weg2.weg2a(fk, g, Hd, N)
                m2, fm2 = maxh(b2, a2)
                # |B|^2 gegen R pruefen: R aus b2 selbst zurueck und gegen Ziel g an den Sollpunkten
                z += f" | 2a max|H| {m2:.6f} bei {fm2:7.0f} Hz, Fehler an Sollpunkten W1 {fehler(b,a,fk,g):.1f} % / 2a {fehler(b2,a2,fk,g):.1f} %"
            print(z)
