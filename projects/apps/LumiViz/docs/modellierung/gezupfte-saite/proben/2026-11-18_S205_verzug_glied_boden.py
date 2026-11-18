"""S205 - Boden der Teiltonlage aus dem Glied fuer den gebrochenen Verzug (Beleg zu UL3-1 a).

Frage: Wie weit verschiebt das Glied fuer den gebrochenen Verzug die zaehlenden Teiltoene (UL1 a), wenn die Kette der
Biquad-Allpaesse laengs der Saite glatt ist und den Bruchteil deshalb nicht ausgleichen kann?

Rechnung: je Saite und Bund die Lage jedes zaehlenden Teiltons ueber 40 Bruchteile des Verzugs (Grundton gestimmt);
ausgegeben wird die halbe Streuung, groesster Wert ueber die Teiltoene, in Cent. Thiran N mit Verzug in [N-0,5; N+0,5),
Lagrange 5 (nur die Phase; sein Betrag < 1 geht ins Abklingen und ist hier nicht bewertet).

Aufruf:  python -X utf8 -B 2026-11-18_S205_verzug_glied_boden.py
Braucht 2026-11-17_S204_dispersion_biquads.py im selben Ordner (und dessen Module aus dem Werkbank-Archiv).
"""
import sys
import pathlib
import importlib.util
from math import comb
sys.dont_write_bytecode = True
import numpy as np

PROBEN = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("bq", PROBEN / "2026-11-17_S204_dispersion_biquads.py")
bq = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bq)
uu, ue = bq.uu, bq.ue
ps = ue.ps
FS = uu.FS
KF = 1200 / np.log(2)
GLIEDER = (("T", 1), ("T", 2), ("T", 3), ("T", 4), ("T", 5), ("L", 5))


def thiran_phase(D, N, w):
    """Phasenverzug eines Thiran-Allpasses der Ordnung N mit Verzug D an den Kreisfrequenzen w"""
    a = np.ones(N + 1)
    for k in range(1, N + 1):
        pr = 1.0
        for n in range(N + 1):
            pr *= (D - N + n) / (D - N + k + n)
        a[k] = (-1) ** k * comb(N, k) * pr
    A = np.exp(-1j * np.outer(w, np.arange(N + 1))) @ a
    return N * w + 2 * np.unwrap(np.angle(A))


def lagrange_phase(D, N, w):
    """Phasenverzug eines Lagrange-Interpolators der Ordnung N (FIR) mit Verzug D"""
    h = np.ones(N + 1)
    for k in range(N + 1):
        for m in range(N + 1):
            if m != k:
                h[k] *= (D - m) / (k - m)
    H = np.exp(-1j * np.outer(w, np.arange(N + 1))) @ h
    return -np.unwrap(np.angle(H))


def boden(fz, art, N):
    wn = 2 * np.pi * fz / FS
    n = np.arange(1, len(fz) + 1)
    lo = N - 0.5 if art == "T" else (N - 1) / 2
    w0 = np.concatenate([[1e-9], wn])
    C = []
    for Dl in lo + np.linspace(0, 1, 41)[:-1]:
        ph = (thiran_phase(Dl, N, w0) if art == "T" else lagrange_phase(Dl, N, w0))[1:]
        dev = ph - wn * ph[0] / wn[0]
        C.append(-KF * dev / (2 * np.pi * n))
    C = np.array(C)
    halb = 0.5 * (C.max(axis=0) - C.min(axis=0))
    return halb.max(), int(np.argmax(halb)) + 1


def zaehlend(sa, L):
    Gt, ph = ue.ziel(sa, L)
    fn, sg = ue.teiltoene(Gt, ph)
    m = fn < 0.45 * FS
    return fn[m][sg[m] <= ue.SIG_REL]


if __name__ == "__main__":
    print("Halbe Streuung der Teiltonlage ueber den Bruchteil des Verzugs, groesster Wert ueber die zaehlenden Teiltoene [Cent]")
    print("Saite Bund | f1 Hz | zaehlend bis Hz | " + " | ".join(("Thiran " if a == "T" else "Lagrange ") + str(N) for a, N in GLIEDER))
    for s in range(6):
        for k in (0, 5, 12, 19):
            fz = zaehlend(ps.GITARRE[s], ps.MENSUR * 2 ** (-k / 12))
            z = f"S{6 - s} B{k:2d} | {fz[0]:7.1f} | {len(fz):3d} bis {fz[-1]:6.0f} |"
            for art, N in GLIEDER:
                c, nn = boden(fz, art, N)
                z += f" {c:6.2f} (n {nn:3d}) |"
            print(z, flush=True)
