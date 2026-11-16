"""S200, 34k-6 — der Umlauf der Saite mit Filtern niedriger Ordnung statt des genauen FIR.

Aufruf (vom Projekt-Root): python -B <dieses Skript> gitarre | cembalo | harfe

Der Zeitbereichs-Kandidat (`2026-11-16_S200_zeitbereich_kandidat.py`) nahm den Umlauf Steg -> Sattel -> Steg genau
(exp(-2 i k L) als FIR mit 250-930 Werten). Ein Kern braucht ihn als Verzug plus wenige Filterwerte. Fuer einen
gehaltenen Ton ist auch das linear: der Ton mit einem Filter-Umlauf laesst sich im Pruefstand rechnen, indem
exp(-2 i k L) durch die Antwort der Filter ersetzt wird (z = exp(i w / fs), auf der Geraden w - i s0 fortgesetzt). Der
Zeitschritt am Steg ist getrennt geprueft; hier geht es nur um den Umlauf. Die Quelle des Zupfs bleibt genau.

Fassungen des Umlaufs (der Verzug D in Takten jeweils so, dass die Phase am Grundton stimmt; D als idealer Verzug —
die gebrochene Stelle ist ein eigener Punkt):
  pol1      Verlust b / (1 - a z^-1), Betrag bei f0 und 8 f0 getroffen (wie der Kern heute); keine Steifheit
  fir5      Verlust als symmetrisches FIR mit 5 Werten (kein Phasengang), an den Verlust je Umlauf bei den Teiltoenen bis
            12 kHz angepasst (kleinste Quadrate des relativen Fehlers); keine Steifheit
  fir5+ap   dazu die Steifheit als K gleiche Allpaesse 1. Ordnung (K = 4), Wert an die Phase bei den Teiltoenen angepasst
  fir9+ap   Verlust mit 9 Werten, Steifheit mit K = 8

Je Fassung gegen den genauen Umlauf: Lautheit, Pegel 1-2 s gegen den Anfang, je Teilton 1-12 die Frequenz (Cent) und
der Abfall frueh -> spaet (dB). Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.optimize as opt

HIER = pathlib.Path(__file__).resolve()


def lade(name):
    spec = importlib.util.spec_from_file_location(name, HIER.with_name(name + ".py"))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


kd = lade("2026-11-16_S200_zeitbereich_kandidat")
ps = kd.ps
FS = ps.FS


def genau(saite, L, f):
    """Betrag und Phase (abgewickelt, positiv) des genauen Umlaufs bei reellen Frequenzen f."""
    k, _ = ps.saite_wellen(saite, 2 * np.pi * np.asarray(f, dtype=float) + 0j)
    return np.exp(2 * k.imag * L), 2 * k.real * L


def teilton_frequenzen(saite, L, f0, bis=12000.0):
    """Die Frequenzen der Teiltoene am starren Steg (Phase des Umlaufs = 2 pi n)."""
    # die Phase waechst monoton: auf einem feinen Raster rechnen und umkehren
    raster = np.arange(1.0, bis + 2.0, 0.25)
    phase = genau(saite, L, raster)[1]
    n = np.arange(1, int(phase[-1] / (2 * np.pi)) + 1)
    return np.interp(2 * np.pi * n, phase, raster)


def baue(fassung, saite, L, f0):
    """Gibt E(w) als Funktion (w komplex) zurueck."""
    fn = teilton_frequenzen(saite, L, f0)
    g, phase = genau(saite, L, fn)
    x = 2 * np.pi * fn / FS
    if fassung == "pol1":
        g1, g8 = genau(saite, L, [fn[0], 8 * fn[0]])[0]
        w1, w8 = x[0], 8 * x[0]
        betrag = lambda a, y: 1 / np.sqrt(1 + a * a - 2 * a * np.cos(y))
        lo, hi = 0.0, 0.95
        for _ in range(60):
            mi = 0.5 * (lo + hi)
            lo, hi = (mi, hi) if betrag(mi, w8) / betrag(mi, w1) > g8 / g1 else (lo, mi)
        a = 0.5 * (lo + hi)
        b = g1 / betrag(a, w1)
        H = lambda z: b / (1 - a / z)
        ap = lambda z: 1.0
    else:
        M = 2 if fassung.startswith("fir5") else 4
        # Verlust je Umlauf 1 - H(x) = l0 + sum_m l_m (1 - cos m x): relativer Fehler bei den Teiltoenen
        # angepasst an den ersten 40 Teiltoenen, Gewicht 1 / n (die tiefen tragen Lautheit und Ausklang)
        nn = np.arange(1, len(x) + 1)
        gew = np.where(nn <= 40, 1.0 / nn, 0.0)
        A = np.stack([np.ones_like(x)] + [1 - np.cos(m * x) for m in range(1, M + 1)], axis=1)
        ziel = 1 - g
        l, _ = opt.nnls(A / ziel[:, None] * gew[:, None], gew)
        c0 = 1 - l[0] - l[1:].sum()
        H = lambda z: (c0 + sum(l[m] * 0.5 * (z ** m + z ** -m) for m in range(1, M + 1))) * z ** -M
        if "+ap" in fassung:
            K = 4 if fassung.startswith("fir5") else 8
            # Steifheit: die Phase des genauen Umlaufs ohne ihren linearen Teil am Grundton; K Allpaesse (a + z^-1)/(1 + a z^-1)
            def ap_phase(a, y):
                return -K * np.angle((a + np.exp(-1j * y)) / (1 + a * np.exp(-1j * y)))

            def fehler(a):
                # Verzug so, dass der Grundton stimmt; Fehler der Phase je Teilton in Cent
                d = (phase[0] - ap_phase(a, x[0])) / x[0]
                dphi = (d * x + ap_phase(a, x)) - phase
                n = np.arange(1, len(x) + 1)
                return np.sum(((1200 / np.log(2) * dphi / (2 * np.pi * n)) ** 2 / n)[:40])
            # der Wert ueber ein Raster vorgesucht (die Zielfunktion hat Nebenminima), dann verfeinert
            raster = np.linspace(-0.95, 0.0, 96)
            i = int(np.argmin([fehler(r) for r in raster]))
            a = opt.minimize_scalar(fehler, bounds=(raster[max(i - 1, 0)], raster[min(i + 1, 95)]), method="bounded").x
            ap = lambda z: ((a + 1 / z) / (1 + a / z)) ** K
        else:
            ap = lambda z: 1.0
    z1 = np.exp(1j * x[0])
    d = (phase[0] + np.angle(H(z1) * ap(z1))) / x[0]      # Verzug, der die Phase am Grundton trifft
    return lambda w: H(np.exp(1j * w / FS)) * ap(np.exp(1j * w / FS)) * np.exp(-1j * w * d / FS)


def kraft(saite, L, xp, d, winkel, Y, w, breite, E):
    """Wie `stegkraft_frequenz`, der Umlauf E vorgegeben (die Quelle bleibt genau)."""
    T, m = saite["T"], saite["m"]
    k, z0 = ps.saite_wellen(saite, w)
    f = -(T * d * L / (xp * (L - xp))) / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2)
    g1 = f / (2 * z0) * (np.exp(-1j * k * xp) - np.exp(-1j * k * (2 * L - xp)))
    n = len(w)
    A = np.zeros((n, 2, 2), dtype=complex)
    A[:, 0, 0] = 1 + z0 * Y[:, 0]
    A[:, 0, 1] = A[:, 1, 0] = z0 * Y[:, 1]
    A[:, 1, 1] = 1 + z0 * Y[:, 2]
    Ainv = np.linalg.inv(A)
    eins = np.broadcast_to(np.eye(2), (n, 2, 2))
    Rb = Ainv @ (A - 2 * eins)
    a = np.radians(winkel)
    g = g1[:, None] * np.array([np.cos(a), np.sin(a)])[None, :]
    vplus = np.linalg.solve(eins + Rb * E[:, None, None], g[:, :, None])[:, :, 0]
    return 2 * z0[:, None] * (Ainv @ vplus[:, :, None])[:, :, 0]


FASSUNGEN = ("pol1", "fir5", "fir5+ap", "fir9+ap")


def laufe(name, faelle, kp, datei):
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    summe = {f: [] for f in FASSUNGEN}
    print(f"{name}: je Taste und Fassung gegen den genauen Umlauf: Lautheit LU | Pegel 1-2 s dB | Teiltoene: groesster Betrag Cent "
          "(Grundton) | Abfall frueh->spaet groesster Betrag dB")
    for titel, sa, L, xp, d, winkel, breite, punkt, f0 in faelle:
        Y, _, _ = ps.koerper_kurven2(nur, punkt, w, tan[:datei])
        _, Hz, Hy = ps.koerper_kurven2(kp, punkt, w, tan)
        k, _ = ps.saite_wellen(sa, w)
        v, en = ps.VORLAUF, ps.VORLAUF + 0.4

        def messe(E):
            F = kraft(sa, L, xp, d, winkel, Y, w, breite, E)
            p = ps.zeit(F[:, 0:1] * Hz + F[:, 1:2] * Hy)
            return ps.momentary_max(p), ps.rms_db(p, 1, 2) - ps.rms_db(p, v, en), kd.teiltoene(ps.zeit(F)[:, 0], f0)

        l0, n0, t0 = messe(np.exp(-2j * k * L))
        zeile = f"  {titel:28s}"
        for fassung in FASSUNGEN:
            l, nk, t = messe(baue(fassung, sa, L, f0)(w))
            cent = [1200 * np.log2(a[0] / b[0]) for a, b in zip(t, t0)]
            abkl = [(a[1] - a[2]) - (b[1] - b[2]) for a, b in zip(t, t0) if b[2] > b[1] - 60]
            m = (l - l0, nk - n0, float(np.abs(cent).max()), cent[0], float(np.abs(abkl).max()) if abkl else float("nan"))
            summe[fassung].append(m)
            zeile += f" | {fassung}: {m[0]:+5.2f} {m[1]:+6.2f} {m[2]:5.1f} ({m[3]:+4.1f}) {m[4]:5.1f}"
        print(zeile, flush=True)
    print("  groesster Betrag je Fassung: Lautheit LU / Pegel 1-2 s dB / Teiltoene Cent / Grundton Cent / Abfall dB")
    for fassung in FASSUNGEN:
        a = np.abs(np.array(summe[fassung]))
        print(f"    {fassung:8s}: {a[:, 0].max():5.2f} / {a[:, 1].max():5.2f} / {a[:, 2].max():5.1f} / {a[:, 3].max():4.1f} / {np.nanmax(a[:, 4]):5.1f}")


if __name__ == "__main__":
    was = sys.argv[1] if len(sys.argv) > 1 else ""
    if was == "gitarre":
        kp = ps.lade_koerper("classical_guitar")
        datei = len(json.loads((ps.KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))["modes"])
        faelle = []
        for s, bund in ((0, 0), (0, 7), (1, 3), (2, 5), (3, 2), (4, 0), (4, 8), (5, 0), (5, 5), (5, 12)):
            sa = ps.GITARRE[s]
            L = ps.MENSUR * 2 ** (-bund / 12)
            faelle.append((f"Saite {6 - s} Bund {bund}", sa, L, min(ps.ZUPFORT, 0.5 * L), 0.002, 30.0, 0.0075,
                           kp["tasten"].index(sa["taste"]), ps.hz_von(sa["taste"] + bund)))
        laufe("Gitarre", faelle, kp, datei)
    elif was == "cembalo":
        kp = ps.lade_koerper("harpsichord")
        datei = len(json.loads((ps.KOERPER / "harpsichord.lvinstbody").read_text(encoding="utf-8"))["modes"])
        faelle = []
        for taste in (29, 36, 43, 50, 57, 64, 71, 78, 84, 89):
            sa, L, xp, d, winkel, breite, _ = ps.cembalo_taste(taste)
            faelle.append((f"Taste {taste}", sa, L, xp, d, winkel, breite, kp["tasten"].index(taste), ps.hz_von(taste)))
        laufe("Cembalo", faelle, kp, datei)
    elif was == "harfe":
        kp = ps.lade_koerper("concert_harp")
        datei = len(json.loads((ps.KOERPER / "concert_harp.lvinstbody").read_text(encoding="utf-8"))["modes"])
        faelle = []
        for taste in (26, 31, 36, 43, 48, 55, 60, 67, 72, 79, 84, 91, 96, 103):
            a = ps.harfe_taste(taste)
            if a:
                sa, L, xp, d, winkel, breite, _ = a
                faelle.append((f"Taste {taste}", sa, L, xp, d, winkel, breite, kp["tasten"].index(taste), ps.hz_von(taste)))
        laufe("Harfe", faelle, kp, datei)
    else:
        sys.exit(__doc__)
