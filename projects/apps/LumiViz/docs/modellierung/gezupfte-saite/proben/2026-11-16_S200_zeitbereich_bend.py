"""S200, 34k-6 — Probe Bend und Slide (Haltepunkt GZ1-1 a): haelt eine Saite mit laufender Tonhoehe +-10 Cent?

Aufruf (vom Projekt-Root): python -B <dieses Skript>

Der Zeitbereichs-Kandidat (`2026-11-16_S200_zeitbereich_kandidat.py`: Steg mit allen Moden der Gitarre, zwei
Richtungen, Diskretisierung foh) mit einem Umlauf, wie ihn ein Kern bei laufender Tonhoehe rechnen kann:
  Verzug     D(t) Takte, gelesen mit Lagrange 5. Ordnung (wie `leseLagrange` der Saiten-Schicht)
  Verlust    ein Tiefpass 1. Ordnung b / (1 - a z^-1), an den Umlauf des Pruefstands bei f0 und 8 f0 angepasst
             (Betrag), fest ueber den Ton; sein Phasenverzug bei der laufenden Tonhoehe wird von D abgezogen
  Steifheit  keine (Nylon; der Pruefstand zeigt am Grundton unter 0,1 Cent)
  Anregung   ein kurzer Kraftstoss am Zupfort (die Tonhoehe haengt nicht an der Form der Anregung)
Die Tonhoehe laeuft nach 0,5 s: Bend um 2 Halbtoene in 0,3 s und zurueck, oder Slide ueber 5 Buende in 0,5 s.

Gemessen: die Frequenz des Grundtons der Stegkraft (komplexe Demodulation am Soll-Verlauf, 40 ms gemittelt) gegen das
Soll in Cent — vor dem Lauf (der Steg verstimmt die Saite: das ist Physik, der Stimmer gleicht es aus), waehrend und
danach; einmal ohne Ausgleich, einmal mit dem Ausgleich, den der Stimmer an der Anfangsnote findet (ein Faktor auf D),
einmal mit einem Ausgleich je Tonhoehe (Faktor an Anfangs- und Zielnote gemessen, dazwischen in der Tonhoehe linear).

Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve()


def lade(name):
    spec = importlib.util.spec_from_file_location(name, HIER.parent / f"{name}.py")
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


kd = lade("2026-11-16_S200_zeitbereich_kandidat")
ps = kd.ps
FS = ps.FS


def verlust(saite, L, f0):
    """b, a des Tiefpasses: |H| = |exp(-2 i k L)| bei f0 und 8 f0."""
    w = 2 * np.pi * np.array([f0, 8 * f0]) + 0j
    k, _ = ps.saite_wellen(saite, w)
    g1, g8 = np.abs(np.exp(-2j * k * L))
    w1, w8 = 2 * np.pi * f0 / FS, 2 * np.pi * 8 * f0 / FS
    betrag = lambda a, x: 1 / np.sqrt(1 + a * a - 2 * a * np.cos(x))
    lo, hi = 0.0, 0.95
    for _ in range(60):
        mi = 0.5 * (lo + hi)
        lo, hi = (mi, hi) if betrag(mi, w8) / betrag(mi, w1) > g8 / g1 else (lo, mi)
    a = 0.5 * (lo + hi)
    return g1 / betrag(a, w1), a


def phasenverzug(b, a, f):
    x = 2 * np.pi * f / FS
    return -np.angle(b / (1 - a * np.exp(-1j * x))) / x


def laufe(soll, faktor, saite, L0, f, eta, e):
    """soll: Frequenz je Takt; faktor: Ausgleich auf D je Takt. Gibt die Stegkraft senkrecht zurueck."""
    n = len(soll)
    z0 = float(np.sqrt(saite["T"] * saite["m"]))
    b, a = verlust(saite, L0, soll[0])
    pol, B0, B1 = kd.moden_diskret(f, eta, e, "foh")
    M = np.einsum("n,na,nb->ab", B0.real, e, e)
    A = np.linalg.inv(np.eye(2) + z0 * M)
    zM = 2.0 * z0 * M
    eT = np.ascontiguousarray(e.T)
    D = (FS / soll - phasenverzug(b, a, soll)) * faktor
    puffer = int(D.max()) + 8
    aus = np.zeros((n + puffer, 2))
    z = np.zeros(len(pol), dtype=complex)
    fv = np.zeros(2)
    y1 = np.zeros(2)
    F = np.zeros(n)
    # Anregung: Gauss-Stoss 0,3 ms bei 5 ms, 30 Grad, direkt und ueber den Sattel (Zupfort 1/4 des Umlaufs)
    t = np.arange(n) / FS
    stoss = np.exp(-0.5 * ((t - 0.005) / 0.0003) ** 2)
    spaet = np.exp(-0.5 * ((t - 0.005 - 0.75 * D[0] / FS) / 0.0003) ** 2)
    g = (stoss - spaet)[:, None] * np.array([np.cos(np.radians(30.0)), np.sin(np.radians(30.0))])[None, :] * 0.01
    for i in range(n):
        d = D[i]
        k = int(d)
        mu = d - k
        # Lagrange 5. Ordnung um den Verzug d: Stuetzstellen k - 2 ... k + 3
        x = mu + 2.0
        gew = np.ones(6)
        for p in range(6):
            for q in range(6):
                if p != q:
                    gew[p] *= (x - q) / (p - q)
        idx = puffer + i - (k - 2) - np.arange(6)
        gelesen = gew @ aus[idx]
        y1 = b * gelesen + a * y1
        v = g[i] - y1
        z = z * pol + B1 * (e @ fv)
        u = A @ (eT @ z.real + zM @ v)
        fv = z0 * (2.0 * v - u)
        z += B0 * (e @ fv)
        aus[puffer + i] = u - v
        F[i] = fv[0]
    return F


def cent_teilton(F, soll, nr):
    phase = 2 * np.pi * np.cumsum(nr * soll) / FS
    zz = F * np.exp(-1j * phase)
    m = int(0.04 * FS)
    zz = np.convolve(zz, np.hanning(m) / np.hanning(m).sum(), mode="same")
    df = np.gradient(np.unwrap(np.angle(zz))) * FS / (2 * np.pi)
    return 1200 * np.log2(np.clip((nr * soll + df) / (nr * soll), 0.5, 2.0))


def cent_verlauf(F, soll):
    """Die Tonhoehe aus den Teiltoenen 1-4 (Median je Takt) — wie die zweite Messung `InstrumentMeasure/Grundton`: ein
    Grundton nahe einer Koerpermode (A0 98 Hz, T1 200 Hz) ist allein kein Mass fuer die Tonhoehe."""
    return np.median([cent_teilton(F, soll, nr) for nr in (1, 2, 3, 4)], axis=0)


def fall(name, saite, L_leer, taste_leer, von_bund, bis_bund, dauer, zurueck, f, eta, e):
    n = int(2.2 * FS)
    t = np.arange(n) / FS
    halb = np.clip((t - 0.5) / dauer, 0, 1)
    if zurueck:
        halb = halb - np.clip((t - 0.5 - dauer - 0.3) / dauer, 0, 1)
    glatt = 0.5 - 0.5 * np.cos(np.pi * halb)
    bund = von_bund + (bis_bund - von_bund) * glatt
    soll = ps.hz_von(taste_leer) * 2 ** (bund / 12)
    L0 = L_leer * 2 ** (-von_bund / 12)
    eins = np.ones(n)

    def messe(faktor):
        c = cent_verlauf(laufe(soll, faktor, saite, L0, f, eta, e), soll)
        s = lambda a, b: c[int(a * FS):int(b * FS)]
        lauf = s(0.5, 0.5 + dauer)
        oben = s(0.5 + dauer + 0.05, 0.5 + dauer + 0.25)
        return float(np.mean(s(0.2, 0.45))), float(lauf[np.argmax(np.abs(lauf))]), float(np.mean(oben))

    # Ausgleich des Stimmers: an gehaltenen Noten gemessen (Anfang und Ziel), als Faktor auf D
    def gehalten(b):
        fest = np.full(int(0.6 * FS), ps.hz_von(taste_leer) * 2 ** (b / 12))
        Fg = laufe(fest, np.ones(len(fest)), saite, L0, f, eta, e)
        c = cent_verlauf(Fg, fest)
        grund = cent_teilton(Fg, fest, 1)
        s = slice(int(0.2 * FS), int(0.5 * FS))
        return float(np.mean(c[s])), float(np.mean(grund[s]))

    (c0, g0), (c1, g1) = gehalten(von_bund), gehalten(bis_bund)
    f0_, f1_ = 2 ** (c0 / 1200), 2 ** (c1 / 1200)
    ohne = messe(eins)
    fest = messe(eins * f0_)
    je = messe(f0_ + (f1_ - f0_) * glatt)
    print(f"{name}: gehalten am Anfang {c0:+.2f} Cent, am Ziel {c1:+.2f} Cent aus den Teiltoenen 1-4 (ohne Stimmer); "
          f"der Grundton allein {g0:+.2f} / {g1:+.2f} Cent")
    for titel, m in (("ohne Ausgleich", ohne), ("Ausgleich der Anfangsnote", fest), ("Ausgleich je Tonhoehe", je)):
        print(f"   {titel:26s}: vor dem Lauf {m[0]:+6.2f} Cent | im Lauf groesster Betrag {m[1]:+6.2f} Cent | am Ziel {m[2]:+6.2f} Cent", flush=True)


if __name__ == "__main__":
    kp = ps.lade_koerper("classical_guitar")
    datei = len(json.loads((ps.KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))["modes"])
    tan = ps.richtung_tan(kp["f"])
    for s, von, bis, dauer, zurueck, was in ((5, 7, 9, 0.3, True, "Bend 2 Halbtoene"), (5, 5, 10, 0.5, False, "Slide 5 Buende"),
                                             (0, 3, 5, 0.3, True, "Bend 2 Halbtoene"), (2, 2, 7, 0.5, False, "Slide 5 Buende"),
                                             (5, 12, 14, 0.3, True, "Bend 2 Halbtoene")):
        sa = ps.GITARRE[s]
        phi = kp["steg"][:datei, kp["tasten"].index(sa["taste"])]
        e = np.stack([phi, phi * tan[:datei]], axis=1)
        if "--starr" in sys.argv:       # Gegenprobe: derselbe Lauf am (fast) starren Steg — was bleibt, ist das Rechenverfahren
            e = e * 1e-4
            was += ", starrer Steg"
        fall(f"Saite {6 - s}, {was} ab Bund {von}", sa, ps.MENSUR, sa["taste"], von, bis, dauer, zurueck, kp["f"][:datei], kp["eta"][:datei], e)
