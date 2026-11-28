"""S222, 34k-7d-1 — die Bezugsdatei des Pruefstands fuer den Kern `pluckedString`, Rechenfassung 2.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> [<ausgabe>]
    Ohne Angabe schreibt es `projects/apps/LumiViz/tests/unit/UnitTests/werte/zupf_pruefstand.txt`.
                           python -X utf8 -B <dieses Skript> zeitschritt <fall> [<fall> ...]
    Die Pruefung von 7d-1: die Faelle (eine Saite) im Zeitschritt wie der Kandidat S200 gegen den Bezug.

Was der Kern in 34k-7d rechnen soll, hier geschlossen je Frequenz (Physik des Pruefstands
`2026-11-15_S199_pruefstand_zupf.py`, dessen Funktionen es nimmt; die Vorgaben stehen HIER, nach den Entscheiden, und
weichen an zwei Stellen von den festen Zahlen des Pruefstands ab, deren Ausgaben gueltig bleiben sollen):

  Saite(n)   je Fall die Werte der gespielten Saite bzw. der Saiten eines Chors (Laute, Oud: am selben Stegpunkt,
             zusammen gezupft; im Einklang die Laenge um -+halbe Verstimmung, Laute 4, Oud 3 Cent; Oktavsaite gleiche
             Laenge). Umlauf genau exp(-2 i k L) (der Kern: das Umlauf-Filter, UL2 a).
  Zupf       Kraftstufe F0 = T d L / (x_p (L - x_p)) je Saite, Fingerbreite als Gauss-Tiefpass, Winkel teilt auf die
             zwei Polarisationen (ZU1 a: der Kern rechnet die Quelle genau so, mit einer FFT). Vorgaben:
               Gitarre  d = 4 mm * 10^(Stufe/20), Neigung 0,16 dB/Okt je dB Stufe um 1 kHz (GZ6 a), 16 cm fest
                        (ZW1 a), 30 Grad, Breite 7,5 mm        [der Pruefstand rechnet 2 mm und mit Kappe]
                        Den Pegel traegt die Auslenkung, die Neigung kommt als eigener Faktor
                        10^(Stufe k lb(f / 1 kHz) / 20) dazu (`neigung_faktor`, k je Instrument in `KIPP`: Gitarre, Laute,
                        Oud 0,16 nach GZ6 a / ZD2 a, Harfe 0 nach ZD1-1 a, Cembalo 0), kausal (minimalphasig, VL2 a,
                        S224 - nullphasig reichte die Quelle leiser Stufen weit vor das Loslassen). Bis S224 nahm die Quelle
                        `staerke_faktor` des Pruefstands, der den Pegel 10^(Stufe/20) schon enthaelt - mit der geschwaechten
                        Auslenkung zaehlte die Stufe doppelt (gitarre_57_pp: -36 statt -18 dB bei 1 kHz).
               Laute, Oud  d = 2 mm, 16 cm fest, 30 Grad, 7,5 mm
               Harfe    0,36 L, d = 4 mm, 2 m/s beim Abloesen, 25 Grad, 7,5 mm (Chadefaux u. a. 2012); die
                        Geschwindigkeit als verteilter Kraftstoss ueber das Dreieck (`quelle`, `impuls`)
                                                         [der Pruefstand: eine Rampe, die nie endet — an den Teiltoenen
                                                          gleich, `2026-11-26_S222_harfe_tempo.py`]
               Cembalo  Kraft 1,07 N an C4 bei ff, -2,8 dB je Oktave (KQ30 a, GZ13 a), Zupfpunkt Taskin, 20 Grad,
                        3 mm                                     [der Pruefstand rechnet 2 N fest]
  Steg       in der Schleife die Moden der Koerperdatei (GZ5 a) mit ihrer Richtung (`bridgeAngle`, sonst die Annahme
             nach Scherrer wie der Pruefstand: `richtung_von`), dazu der Daempfer ueber 5 kHz als sieben breite Moden
             (Q 1,5; Mitten 6 ... 23 kHz) NUR senkrecht, Gewichte phi^2 >= 0 mit NNLS an
             Re Y = n(f) phi_rms^2 / 4 angepasst (GZ5-1 a; phi_rms senkrecht = bridgeSpread * cos(bridgeAngle) des
             statistischen Teils). Wie der Kandidat S200 (`daempfer_moden`), dort mit einer zweiten Richtung (0,4) —
             der `Steg` aus 34k-7b traegt keine; ueber 5 kHz strahlt nur die Senkrechte.
  Ausgang    die Stegkraft senkrecht zur Decke (Summe ueber die Saiten am Punkt) — das gibt der Kern aus (GZ5 a).
             Daraus am Hoerort p = Fz * Hz mit ALLEN Moden (samt gezogenem statistischem Teil wie `alleModen`).

Je Fall stehen in der Datei: die Eingaben (zum Pruefen der Vorgaben des Kerns), die Gewichte des Daempfers, je Teilton
1-12 der Stegkraft Frequenz und Pegel frueh (0,05-0,45 s) / spaet (1-2 s) wie der Kandidat (`teiltoene`), der Pegel der
Stegkraft in 0-0,1 / 0-1 / 1-3 s (dB re 1 N, Effektivwert), derselbe Pegel im Band 20 Hz - 20 kHz (S224, PK1 a: ueber das
ganze Band traegt ihn die statische Last unter 20 Hz — an der Gitarre 1-3 s 21-40 dB ueber dem Band; `kraftband`), die Lautheit am Hoerort (Momentary-Maximum,
BS.1770) und der Pegel 1-2 s gegen 0-0,4 s. Zahlen mit 17 Stellen.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.optimize as opt

ROOT = pathlib.Path.cwd()
P = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"
spec = importlib.util.spec_from_file_location("pruefstand", P / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
spec = importlib.util.spec_from_file_location("kandidat", P / "2026-11-16_S200_zeitbereich_kandidat.py")
kd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(kd)

ZUPFSTELLE = 0.16                      # ZW1 a

# ---------------------------------------------------------------- VL7 a: der Verlust der Saite kausal (S224)
# Wie `umlauf::kausalerVerlust`, `kausalAngepasst` und `wellenzahl` des Kerns Schritt fuer Schritt: Im k ohne Dispersion
# (`ps.saite_wellen`) auf VERLUST_FEIN Werten der reellen Achse (w mindestens 2 pi * 1 Hz), sein reelles Cepstrum gefaltet,
# dk = -Im rfft; jeder VERLUST_ABLAGE. Wert gemerkt (je Saite). Zug und Biegesteife der verlustfreien Wellenzahl k' je Saite
# und Laenge so, dass die Teiltoene 2 L (Re k'(w_n) + dk(w_n)) = 2 pi n denen der Vorgabe gleichen (f_n = sqrt((T q^2 +
# EI q^4) / m) / 2 pi, q = n pi / L, n = 1 ... 30 bis 20 kHz; Cent; Gauss-Newton, Differenzen 1e-7, 30 Schritte; die
# Teiltoene des neuen Modells mit 60 Halbierungen in [0,9 ; 1,1] w_n). Die Wellenzahl: k'(w) + dk + i Im k, auf der Geraden
# w = Re w - i s0 als eine analytische Funktion fortgesetzt: das gefaltete Cepstrum c ist kausal, G = rfft(c) traegt auf der
# reellen Achse Im k - i dk, also dk + i Im k = i G, und auf der Geraden G = rfft(c exp(-s0 q / FS)) (S224, nach VL7 a: bis
# dahin dk an |Re w| zur fortgesetzten k', auf der Geraden fehlte -i s0 d(dk)/dw, die Laute 38 klang rund 10 % langsamer
# ab, `2026-11-28_S224_laute38_kopplung.txt`); z0 = T k / w mit dem Zug der Saite. Quellen und Zahlen: `2026-11-27_S224_verlust_kausal.py` / `.txt` (E_B aus den
# gemessenen Obertoenen, Lynch-Aird & Woodhouse 2017); ueber die Laenge gemeinsam angepasst blieben an der Darmsaite der
# Laute 38 10,6 Cent.
VERLUST_FEIN = 1 << 20
VERLUST_ABLAGE = 16
WELLEN_OHNE_DISPERSION = ps.saite_wellen
_KAUSAL = {}
_ANGEPASST = {}
_AUF_GERADE = {}


def verlustfrei(T, m, EI, w):
    if EI > 0:
        return np.sqrt((-T + np.sqrt(T * T + 4 * EI * m * w * w)) / (2 * EI))
    return w * np.sqrt(m / T)


def gefaltetes_cepstrum(sa):
    """Im k ohne Dispersion auf der reellen Achse (w mindestens 2 pi * 1 Hz), sein reelles Cepstrum gefaltet (kausal)."""
    N = VERLUST_FEIN
    f = np.arange(N // 2 + 1) * (ps.FS / N)
    k, _ = WELLEN_OHNE_DISPERSION(sa, (2 * np.pi * np.maximum(f, 1.0)).astype(complex))
    c = np.fft.irfft(k.imag, N)
    c[1:N // 2] *= 2
    c[N // 2 + 1:] = 0.0
    return c


def kausal_verlust(sa):
    """(Schritt in Hz, dk ab 0 Hz) einer Saite — je Satz Werte einmal."""
    schluessel = tuple(sorted(sa.items()))
    if schluessel not in _KAUSAL:
        N = VERLUST_FEIN
        df = ps.FS / N
        dk_voll = -np.fft.rfft(gefaltetes_cepstrum(sa)).imag
        idx = np.minimum(np.arange(len(dk_voll) // VERLUST_ABLAGE + 1) * VERLUST_ABLAGE, len(dk_voll) - 1)
        _KAUSAL[schluessel] = (df * VERLUST_ABLAGE, dk_voll[idx])
    return _KAUSAL[schluessel]


def kausal_verlust_auf(sa, s0):
    """(Schritt in Hz, dk + i Im k ab 0 Hz) auf der Geraden w - i s0 — je Satz Werte und s0 einmal."""
    schluessel = (tuple(sorted(sa.items())), float(s0))
    if schluessel not in _AUF_GERADE:
        N = VERLUST_FEIN
        c = gefaltetes_cepstrum(sa) * np.exp(-s0 * np.arange(N) / ps.FS)
        G = 1j * np.fft.rfft(c)
        idx = np.minimum(np.arange(len(G) // VERLUST_ABLAGE + 1) * VERLUST_ABLAGE, len(G) - 1)
        _AUF_GERADE[schluessel] = (ps.FS / N * VERLUST_ABLAGE, G[idx])
    return _AUF_GERADE[schluessel]


def dk_bei(schritt, dk, w):
    x = np.abs(np.real(w)) / (2 * np.pi) / schritt
    i = np.minimum(np.floor(x).astype(int), len(dk) - 2)
    t = x - i
    return np.where(np.floor(x) + 1 >= len(dk), dk[-1], dk[i] + t * (dk[i + 1] - dk[i]))


def kausal_angepasst(sa, L):
    """(T'/T, EI'/EI, Rest in Cent) einer Saite an der Laenge L — je Saite und Laenge einmal."""
    schluessel = (tuple(sorted(sa.items())), L)
    if schluessel in _ANGEPASST:
        return _ANGEPASST[schluessel]
    schritt, dk = kausal_verlust(sa)
    T, m, EI = sa["T"], sa["m"], sa["EI"]
    n = np.arange(1, 31)
    q = n * np.pi / L
    alt = np.sqrt((T * q * q + EI * q ** 4) / m)
    alt = alt[alt <= 2 * np.pi * 20000.0]
    ziel = np.pi * np.arange(1, len(alt) + 1)

    def neu(pT, pE):
        lo, hi = 0.9 * alt, 1.1 * alt
        for _ in range(60):
            mitte = 0.5 * (lo + hi)
            g = L * (verlustfrei(T * pT, m, EI * pE, mitte) + dk_bei(schritt, dk, mitte)) - ziel
            lo, hi = np.where(g < 0, mitte, lo), np.where(g < 0, hi, mitte)
        return 0.5 * (lo + hi)

    def rest(pT, pE):
        return 1200 * np.log2(neu(pT, pE) / alt)

    steif = EI > 0
    p = [1.0, 1.0]
    for _ in range(30):
        r0 = rest(*p)
        h = 1e-7
        j1 = (rest(p[0] + h, p[1]) - r0) / h
        j2 = (rest(p[0], p[1] + h) - r0) / h if steif else 0 * r0
        a11, a12, a22 = np.dot(j1, j1), np.dot(j1, j2), np.dot(j2, j2)
        b1, b2 = np.dot(j1, r0), np.dot(j2, r0)
        det = a11 * a22 - a12 * a12
        if steif and len(r0) > 1 and det > 0:
            d1, d2 = (a22 * b1 - a12 * b2) / det, (a11 * b2 - a12 * b1) / det
        else:
            d1, d2 = b1 / a11, 0.0
        p = [p[0] - d1, p[1] - d2]
        if abs(d1) < 1e-13 and abs(d2) < 1e-13:
            break
    _ANGEPASST[schluessel] = (p[0], p[1], float(np.abs(rest(*p)).max()))
    return _ANGEPASST[schluessel]


def wellen_kausal(sa, L, w):
    """Die Wellenzahl der Saite mit der Laenge L kausal (VL7 a) und z0 = T k / w."""
    w = np.asarray(w, dtype=complex)
    s0 = -float(np.imag(w).flat[0])
    assert np.all(np.imag(w) == -s0), "wellen_kausal: alle w auf einer Geraden w - i s0"
    schritt, wert = kausal_verlust_auf(sa, s0)
    pT, pE, _ = kausal_angepasst(sa, L)
    T, m, EI = sa["T"], sa["m"], sa["EI"]
    k = verlustfrei(T * pT, m, EI * pE, w) + dk_bei(schritt, wert, w)
    return k, T * k / w
DAEMPFER_MITTEN = np.array([6000.0, 7500.0, 9500.0, 12000.0, 15000.0, 19000.0, 23000.0])
DAEMPFER_ETA = 1 / 1.5


def daempfer(st, punkt):
    """GZ5-1 a als sieben breite Moden, wie `daempfer_moden` des Kandidaten S200 (dasselbe Raster und Gewicht)."""
    streuung = st["bridgeSpread"][punkt] * np.cos(np.radians(st.get("bridgeAngle", 0.0)))
    f = np.concatenate([np.arange(500.0, 4501.0, 250.0), np.arange(5250.0, 20001.0, 250.0)])
    ziel = np.where(f >= st["fromHz"], (st["density"] + st["rise"] * (f - st["fromHz"])) * streuung ** 2 / 4, 0.0)
    w, wn = 2 * np.pi * f[:, None], 2 * np.pi * DAEMPFER_MITTEN[None, :]
    A = np.real(1j * w / (wn ** 2 - w ** 2 + 1j * DAEMPFER_ETA * wn * w))
    gew = np.where(f < st["fromHz"], 3.0, 1.0)
    phi2, _ = opt.nnls(A * gew[:, None], ziel * gew)
    return streuung, phi2


def dreieck_integral(kappa, a, L, d):
    """Integral_0^L y0(x) e^(-i kappa x) dx fuer das Dreieck y0 (Spitze d bei a, null an beiden Enden), geschlossen."""
    Ea, EL = np.exp(-1j * kappa * a), np.exp(-1j * kappa * L)
    return d * (Ea - 1) / (a * kappa ** 2) - d * (EL - Ea) / ((L - a) * kappa ** 2)


KIPP = {"gitarre": 0.16, "laute": 0.16, "oud": 0.16, "harfe": 0.0, "cembalo": 0.0}


NFEIN = 1 << 21                        # VL2 a: Raster der reellen Achse fuer das Cepstrum (wie `kNeigungFein` des Kerns)
_NEIGUNG = {}


def neigung_kausal(p):
    """VL2 a: h(t) je Takt des minimalphasigen Filters mit dem Betrag 10^(p lb(max(f, 20 Hz) / 1 kHz) / 20) — reelles
    Cepstrum auf NFEIN Werten, gefaltet, h = irfft(exp(rfft(gefaltet))) (wie `neigungKausal` des Kerns)."""
    if p not in _NEIGUNG:
        f = np.fft.rfftfreq(NFEIN, 1 / ps.FS)
        c = np.fft.irfft(p * np.log2(np.maximum(f, 20.0) / 1000.0) / 20 * np.log(10.0), NFEIN)
        c[1:NFEIN // 2] *= 2
        c[NFEIN // 2 + 1:] = 0.0
        _NEIGUNG[p] = np.fft.irfft(np.exp(np.fft.rfft(c)), NFEIN)
    return _NEIGUNG[p]


BAND_DURCHLASS = 20000.0              # VL5 a: die Bandgrenze der Quelle kausal (wie `kBandDurchlass`, `kBandBodenDb`)
BAND_BODEN_DB = -120.0
_QUELLFILTER = {}


def quellfilter_kausal(p):
    """VL2 a, VL5 a: h(t) je Takt des minimalphasigen Filters mit dem Betrag der Neigung (p wie `neigung_kausal`) mal der
    Bandgrenze (1 bis BAND_DURCHLASS, darueber mit einem halben Kosinus bis FS / 2 auf BAND_BODEN_DB) — wie
    `quellfilterKausal` des Kerns."""
    if p not in _QUELLFILTER:
        f = np.fft.rfftfreq(NFEIN, 1 / ps.FS)
        x = np.clip((f - BAND_DURCHLASS) / (ps.FS / 2 - BAND_DURCHLASS), 0.0, 1.0)
        db = p * np.log2(np.maximum(f, 20.0) / 1000.0) + BAND_BODEN_DB * 0.5 * (1 - np.cos(np.pi * x))
        c = np.fft.irfft(db / 20 * np.log(10.0), NFEIN)
        c[1:NFEIN // 2] *= 2
        c[NFEIN // 2 + 1:] = 0.0
        _QUELLFILTER[p] = np.fft.irfft(np.exp(np.fft.rfft(c)), NFEIN)
    return _QUELLFILTER[p]


def quellfilter(w, stufe, kipp):
    """Das Quellfilter (Neigung je Stufe und Bandgrenze, kausal; VL2 a, VL5 a) an den Punkten w der Geraden w - i s0 des
    Pruefstands. Seit VL5 a (S224) immer, auch bei Stufe 0: nullphasig begrenzt schwang der Kraftsprung des Loslassens
    nach beiden Seiten nach (Cembalo vor -2 ms -72 dB; `2026-11-27_S224_vorlaeufer_kausal.txt`)."""
    n = 2 * (len(w) - 1)
    s0 = -float(w[0].imag)
    assert n <= NFEIN and abs(w[1].real - 2 * np.pi * ps.FS / n) < 1e-9, "nur auf einem Raster der FFT bei FS"
    h = quellfilter_kausal(stufe * kipp)[:n]
    return np.fft.rfft(h * np.exp(-s0 * np.arange(n) / ps.FS), n)


def neigung_faktor(w, stufe, kipp):
    """Die Neigung allein (die Quelle nimmt seit VL5 a `quellfilter`, das die Bandgrenze mitfuehrt; diese Funktion bleibt
    fuer die Proben S224). Die Neigung je Stufe um 1 kHz ohne Pegel (GZ6 a): Betrag 10^(Stufe k lb(f / 1 kHz) / 20), f mindestens 20 Hz; seit
    VL2 a (S224) kausal (minimalphasig, `neigung_kausal`), an den Punkten w der Geraden w - i s0 des Pruefstands
    (`ps.frequenzen()`): H = Summe h(t) e^(-s0 t) e^(-i w t). Bis S224 reell (nullphasig): mit dem Knick bei 20 Hz reichte
    die Quelle bei leisen Stufen weit vor das Loslassen (`2026-11-27_S224_vorlauf.txt`)."""
    if stufe == 0.0 or kipp == 0.0:
        return 1.0
    n = 2 * (len(w) - 1)
    s0 = -float(w[0].imag)
    assert n == int(ps.DAUER * ps.FS) and abs(w[1].real - 2 * np.pi * ps.FS / n) < 1e-9, "nur auf dem Raster des Pruefstands"
    h = neigung_kausal(stufe * kipp)[:n]
    return np.fft.rfft(h * np.exp(-s0 * np.arange(n) / ps.FS), n)


def quelle(sa, L, xp, d, breite, tempo, stufe, w, geschwindigkeit="impuls", kipp=0.16):
    """Die Quelle g einer Saite am Steg (ZU1 a: so rechnet der Kern sie, mit einer FFT): Kraftstufe am Zupfpunkt,
    Fingerbreite, Neigung je Stufe (ohne Pegel, den traegt d), dazu die Geschwindigkeit beim Abloesen (siehe `stegkraft`)."""
    k, z0 = wellen_kausal(sa, L, w)                  # VL7 a
    T, m = sa["T"], sa["m"]
    breit = np.exp(-(w * breite / np.sqrt(T / m)) ** 2) * quellfilter(w, stufe, kipp)     # VL2 a, VL5 a
    f = -(T * d * L / (xp * (L - xp))) / (1j * w) * breit
    if tempo and geschwindigkeit == "rampe":
        f = f * (1 - (tempo / d) / (1j * w))
    g = f / (2 * z0) * (np.exp(-1j * k * xp) - np.exp(-1j * k * (2 * L - xp)))
    if tempo and geschwindigkeit == "impuls":
        q = -m * (tempo / d) * breit                          # Kraftstoss je Laenge und Auslenkung: mu v0(x) = q y0(x)
        g = g + q / (2 * z0) * (dreieck_integral(k, xp, L, d) - np.exp(-2j * k * L) * dreieck_integral(-k, xp, L, d))
    return g


def stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, stufe, Y, w, geschwindigkeit="impuls", kipp=0.16):
    """Die Kraft am Steg je Saite und Richtung (Frequenz x 2S), wie `ton_mit` des Pruefstands, mit der Neigung je Stufe
    (`staerke_faktor`, GZ6 a) und der Kraftstufe je Saite aus derselben Auslenkung.

    Die Geschwindigkeit beim Abloesen (`tempo`, Harfe): `impuls` (Vorgabe, S222) — die Anfangsgeschwindigkeit
    v0(x) = -(tempo / d) y0(x) (zur Ruhelage hin, dieselbe Form wie die Auslenkung) als verteilter Kraftstoss
    mu v0(x) delta(t), ueber das Dreieck geschlossen integriert (`dreieck_integral`); `rampe` — wie der Pruefstand
    f (1 - (tempo/d) / (i w)): gleich an den Teiltoenen, dazu eine Rampe aus der statischen Last, die nie endet
    (`2026-11-26_S222_harfe_tempo.py`)."""
    S, n = len(saiten), len(w)
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    a = np.radians(winkel)
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        k, z0 = wellen_kausal(sa, L, w)              # VL7 a
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = np.exp(-2j * k * L)
        gi = quelle(sa, L, xp, d, breite, tempo, stufe, w, geschwindigkeit, kipp)
        g[:, 2 * i], g[:, 2 * i + 1] = gi * np.cos(a), gi * np.sin(a)
    F = np.empty((n, 2 * S), dtype=complex)
    eins = np.eye(2 * S)
    for b0 in range(0, n, 8192):
        b = slice(b0, b0 + 8192)
        YZ = Y[b] * z[b][:, None, :]
        M = np.linalg.solve(eins[None] + YZ, 2 * YZ)
        vplus = np.linalg.solve(eins[None] + E[b][:, :, None] * (M - eins[None]), g[b][:, :, None])[:, :, 0]
        F[b] = z[b] * (2 * vplus - (M @ vplus[:, :, None])[:, :, 0])
    return F


KOERPER = {"gitarre": "classical_guitar", "laute": "lute", "oud": "oud", "harfe": "concert_harp", "cembalo": "harpsichord"}


def faelle():
    """(Name, Instrument, Taste, Stufe): Stichtasten tief / mitte / hoch je Instrument, an der Gitarre dazu Bund 19
    (ZW1 a) und eine leise Stufe (Neigung, GZ6 a). Die Laute ohne Chor 8 (UL2 a: Ausnahme Chor 8.1)."""
    return [("gitarre_40", "gitarre", 40, 0.0), ("gitarre_57", "gitarre", 57, 0.0), ("gitarre_57_pp", "gitarre", 57, -18.0),
            ("gitarre_76", "gitarre", 76, 0.0), ("gitarre_83", "gitarre", 83, 0.0),
            ("laute_38", "laute", 38, 0.0), ("laute_53", "laute", 53, 0.0), ("laute_65", "laute", 65, 0.0),
            ("oud_36", "oud", 36, 0.0), ("oud_45", "oud", 45, 0.0), ("oud_62", "oud", 62, 0.0),
            ("harfe_36", "harfe", 36, 0.0), ("harfe_60", "harfe", 60, 0.0), ("harfe_91", "harfe", 91, 0.0),
            ("cembalo_36", "cembalo", 36, 0.0), ("cembalo_60", "cembalo", 60, 0.0), ("cembalo_84", "cembalo", 84, 0.0)]


def eingaben(instrument, taste, stufe):
    """(Saiten, Laengen, Stegpunkt-Taste, x_p, d, Winkel, Breite, Tempo, Stufe der Neigung, Kraft F0 je Saite)."""
    if instrument == "gitarre":
        s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste)
        sa = dict(ps.GITARRE[s])
        L = ps.MENSUR * 2 ** (-(taste - sa["taste"]) / 12)
        d = 0.004 * 10 ** (stufe / 20)
        return [sa], [L], sa["taste"], ZUPFSTELLE, d, 30.0, 0.0075, 0.0, stufe
    if instrument in ("laute", "oud"):
        ch, mensur, cent = ps.choere(instrument)
        c = max(i for i, (t, _) in enumerate(ch) if t <= taste)
        saiten = ch[c][1]
        laenge = mensur * 2 ** (-(taste - ch[c][0]) / 12)
        gleich = len(saiten) == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else laenge for i in range(len(saiten))]
        return saiten, laengen, ch[c][0], ZUPFSTELLE, 0.002, 30.0, 0.0075, 0.0, 0.0
    if instrument == "harfe":
        sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        return [sa], [L], taste, xp, d, winkel, breite, tempo, 0.0
    sa, L, xp, _, winkel, breite, _ = ps.cembalo_taste(taste)
    kraft = 1.07 * 10 ** (-2.8 / 20 * (taste - 60) / 12)          # KQ30 a, GZ13 a
    d = kraft * xp * (L - xp) / (sa["T"] * L)
    return [sa], [L], taste, xp, d, winkel, breite, 0.0, 0.0


def zahl(x):
    return repr(float(x))


def band_db(x, von, bis):
    """Der Pegel der Kraft im Fenster von ... bis (s) im Band 20 Hz - 20 kHz (S224, PK1 a): das Stueck ohne seinen
    Mittelwert, mit Hann-Fenster (die Leistung auf das Mittel des Fensters im Quadrat bezogen), eine FFT ueber das Stueck,
    ausserhalb des Bands null. Die statische Kraft und ihre Auslaeufer zaehlen so nicht: sie bleibt im Bezug nach dem
    Loslassen stehen (Harfe 91 -4,9 N), im Kern klingt sie langsam ab (sein Umlauf-Filter ist bei 0 Hz nicht genau 1) —
    ueber den ganzen Ton gefiltert trugen ihre Spruenge, im Rechteckfenster ihr Gefaelle bei leisem Ausklang das Band
    (`proben/2026-11-27_S224_bezug_statik.txt`)."""
    s = x[int(von * ps.FS):int(bis * ps.FS)]
    w = np.hanning(len(s))
    s = (s - np.mean(s)) * w
    X = np.fft.rfft(s)
    hz = np.fft.rfftfreq(len(s), 1 / ps.FS)
    X[(hz < 20.0) | (hz > 20000.0)] = 0.0
    y = np.fft.irfft(X, len(s))
    return 10 * np.log10(np.mean(y ** 2) / np.mean(w ** 2) + 1e-300)


def rechne(name, instrument, taste, stufe, kp, roh, w, aus):
    saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = eingaben(instrument, taste, stufe)
    punkt = kp["tasten"].index(stegtaste)
    datei = len(roh["modes"])
    tan = ps.richtung_von(kp)
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    S = len(saiten)
    Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt] * S)
    streuung, phi2 = daempfer(roh["statistical"], punkt)
    wn = 2 * np.pi * DAEMPFER_MITTEN
    yd = (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
    for i in range(S):
        for j in range(S):
            Y[:, 2 * i, 2 * j] += yd
    F = stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, neigung, Y, w, kipp=KIPP[instrument])
    Fz = F[:, 0::2].sum(axis=1)
    _, Hz, _ = ps.koerper_kurven2(kp, punkt, w, tan)                # Abstrahlung: alle Moden, nur die Senkrechte
    kraft = ps.zeit(Fz[:, None])[:, 0]
    p = ps.zeit(Fz[:, None] * Hz)
    f0 = 440.0 * 2 ** ((taste - 69) / 12)
    tt = kd.teiltoene(kraft, f0)
    v, e = ps.VORLAUF, ps.VORLAUF + 0.4

    def rms(von, bis):
        x = kraft[int(von * ps.FS):int(bis * ps.FS)]
        return 10 * np.log10(np.mean(x ** 2) + 1e-300)

    aus.append(f"fall {name} {instrument} taste {taste} stufe {zahl(stufe)} stegpunkt {stegtaste} saiten {S}")
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        werte = " ".join(f"{k} {zahl(sa[k])}" for k in ("T", "m", "EI", "etaB", "etaF", "etaA", "d", "rho") if k in sa)
        f0n = sa["T"] * d * L / (xp * (L - xp))
        aus.append(f"saite {i} laenge {zahl(L)} {werte} kraft {zahl(f0n)}")
    aus.append(f"zupf ort {zahl(xp)} auslenkung {zahl(d)} winkel {zahl(winkel)} breite {zahl(breite)} tempo {zahl(tempo)} "
               f"neigung {zahl(neigung)} kipp {zahl(KIPP[instrument])}")
    aus.append(f"daempfer streuung {zahl(streuung)} " + " ".join(zahl(x) for x in phi2))
    for n, (hz, frueh, spaet) in enumerate(tt, 1):
        aus.append(f"teilton {n} {zahl(hz)} {zahl(frueh)} {zahl(spaet)}")
    aus.append(f"kraft {zahl(rms(v, v + 0.1))} {zahl(rms(v, v + 1.0))} {zahl(rms(1.0, 3.0))}")
    aus.append(f"kraftband {zahl(band_db(kraft, v, v + 0.1))} {zahl(band_db(kraft, v, v + 1.0))} {zahl(band_db(kraft, 1.0, 3.0))}")
    aus.append(f"hoerort {zahl(ps.momentary_max(p))} {zahl(ps.rms_db(p, 1, 2) - ps.rms_db(p, v, e))}")
    print(f"{name}: {S} Saite(n), Stegpunkt {stegtaste}, x_p {xp:.4f} m, F0 {saiten[0]['T'] * d * laengen[0] / (xp * (laengen[0] - xp)):.4f} N, "
          f"{len(tt)} Teiltoene, Grundton {tt[0][0]:.3f} Hz, Hoerort {ps.momentary_max(p):.2f} LUFS", flush=True)


def gegen_zeitschritt(namen):
    """Pruefung 7d-1: derselbe Fall im Zeitschritt wie der Kandidat S200 (`schleife`: Moden „genau fuer stueckweise lineare
    Kraft“ mit Durchgriff, Umlauf genau als FIR, die Quelle g genau) — nur eine Saite. Gibt je Fall die Abweichung der
    Teiltoene (Cent, Pegel frueh / spaet), der Lautheit am Hoerort und des Pegels der Stegkraft."""
    w = ps.frequenzen()
    for name, instrument, taste, stufe in [f for f in faelle() if f[0] in namen]:
        saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = eingaben(instrument, taste, stufe)
        if len(saiten) != 1:
            print(f"{name}: Chor - der Kandidat rechnet eine Saite, ausgelassen")
            continue
        koerper = KOERPER[instrument]
        kp = ps.lade_koerper(koerper)
        roh = json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))
        punkt = kp["tasten"].index(stegtaste)
        datei = len(roh["modes"])
        tan = ps.richtung_von(kp)
        phi = kp["steg"][:datei, punkt]
        f, eta = kp["f"][:datei], kp["eta"][:datei]
        e = np.stack([phi, phi * tan[:datei]], axis=1)
        nur = {"f": f, "eta": eta, "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
        Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt])
        _, phi2 = daempfer(roh["statistical"], punkt)
        wn = 2 * np.pi * DAEMPFER_MITTEN
        Y[:, 0, 0] += (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
        f = np.concatenate([f, DAEMPFER_MITTEN])
        eta = np.concatenate([eta, np.full(len(DAEMPFER_MITTEN), DAEMPFER_ETA)])
        e = np.concatenate([e, np.stack([np.sqrt(phi2), 0 * phi2], axis=1)])
        # Bezug: geschlossen
        F = stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, neigung, Y, w, kipp=KIPP[instrument])
        bezug = ps.zeit(F[:, 0:1])[:, 0]
        # die Quelle g wie in `stegkraft`, in die Zeit
        sa, L = saiten[0], laengen[0]
        T, m = sa["T"], sa["m"]
        g1 = quelle(sa, L, xp, d, breite, tempo, neigung, w, kipp=KIPP[instrument])
        a = np.radians(winkel)
        g = ps.zeit(g1[:, None] * np.array([np.cos(a), np.sin(a)])[None, :])
        h, _ = kd.umlauf_fir(sa, L)
        z0 = float(np.sqrt(T * m))
        pol, B0, B1 = kd.moden_diskret(f, eta, e, "foh")
        M = np.einsum("n,na,nb->ab", B0.real, e, e)
        Fzeit = kd.schleife(np.ascontiguousarray(g), h, z0, pol, B0, B1, np.ascontiguousarray(e), M)[:, 0]
        _, Hz, _ = ps.koerper_kurven2(kp, punkt, w, tan)
        pb = ps.zeit(kd.spektrum_s0(bezug[:, None]) * Hz)
        pz = ps.zeit(kd.spektrum_s0(Fzeit[:, None]) * Hz)
        f0 = 440.0 * 2 ** ((taste - 69) / 12)
        tb, tz = kd.teiltoene(bezug, f0), kd.teiltoene(Fzeit, f0)
        cent = [1200 * np.log2(x[0] / y[0]) for x, y in zip(tz, tb)]
        dfr = [x[1] - y[1] for x, y in zip(tz, tb)]
        dsp = [x[2] - y[2] for x, y in zip(tz, tb) if y[2] > y[1] - 60]
        v = ps.VORLAUF

        def rms(x, von, bis):
            s = x[int(von * ps.FS):int(bis * ps.FS)]
            return 10 * np.log10(np.mean(s ** 2) + 1e-300)

        def baender(x, von, bis):
            s = x[int(von * ps.FS):int(bis * ps.FS)]
            S2 = np.abs(np.fft.rfft(s)) ** 2
            hz = np.fft.rfftfreq(len(s), 1 / ps.FS)
            return [10 * np.log10(S2[(hz >= lo) & (hz < hi)].sum() + 1e-300)
                    for lo, hi in ((0, 20), (20, 5000), (5000, 12000), (12000, 20000), (20000, 24001))]

        print(f"{name}: je Teilton n: Hz, Pegel frueh / spaet Bezug, Zeitschritt minus Bezug (dB)")
        for n, (x, y) in enumerate(zip(tz, tb), 1):
            print(f"    {n:2d} {y[0]:9.2f} Hz  {y[1]:7.1f} / {y[2]:7.1f}   {x[1] - y[1]:+6.2f} / {x[2] - y[2]:+6.2f}")
        bb, bz = baender(bezug, 1, 3), baender(Fzeit, 1, 3)
        print(f"{name}: 1-3 s je Band <20 / 20-5k / 5-12k / 12-20k / >20k Hz: Bezug " + " / ".join(f"{x:6.1f}" for x in bb) + "  Zeitschritt "
              + " / ".join(f"{x:6.1f}" for x in bz) + " dB")
        print(f"{name}: Lautheit am Hoerort {ps.momentary_max(pz) - ps.momentary_max(pb):+.3f} LU | Teiltoene 1-{len(cent)}: "
              f"groesster Betrag {np.abs(cent).max():.3f} Cent (Grundton {cent[0]:+.3f}), Pegel frueh {np.abs(dfr).max():.2f} dB, "
              f"spaet {np.abs(dsp).max() if dsp else float('nan'):.2f} dB | Pegel der Stegkraft 0-0,1 / 0-1 / 1-3 s "
              f"{rms(Fzeit, v, v + 0.1) - rms(bezug, v, v + 0.1):+.3f} / {rms(Fzeit, v, v + 1) - rms(bezug, v, v + 1):+.3f} / "
              f"{rms(Fzeit, 1, 3) - rms(bezug, 1, 3):+.3f} dB", flush=True)


def main():
    ziel = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "projects/apps/LumiViz/tests/unit/UnitTests/werte/zupf_pruefstand.txt"
    w = ps.frequenzen()
    aus = ["# Bezug des Kerns pluckedString, Rechenfassung 2 (34k-7d-1, S222) - nicht von Hand aendern",
           "# geschrieben von docs/modellierung/gezupfte-saite/proben/2026-11-26_S222_bezug_zupf.py (dort die Vorgaben und die Rechnung)",
           f"# Pruefstand: {ps.FS} Hz, {ps.DAUER} s, s0 {ps.S0} 1/s, Vorlauf {ps.VORLAUF} s; Daempfer-Mitten "
           + " ".join(f"{m:.0f}" for m in DAEMPFER_MITTEN) + f" Hz, eta {DAEMPFER_ETA!r}",
           "# fall <name> <instrument> taste <t> stufe <dB> stegpunkt <taste> saiten <n>",
           "# saite <i> laenge <m> T m EI etaB [etaF etaA | d rho] kraft <F0 N>",
           "# zupf ort <m> auslenkung <m> winkel <grad> breite <m> tempo <m/s> neigung <stufe dB> kipp <dB je Oktave je dB Stufe>",
           "# daempfer streuung <phi_rms senkrecht> <phi^2 je Mitte, 7 Werte>",
           "# teilton <n> <Hz> <Pegel frueh dB> <Pegel spaet dB>   (Stegkraft senkrecht)",
           "# kraft <dB re 1 N: 0-0,1 s> <0-1 s> <1-3 s>   (ab dem Vorlauf)",
           "# kraftband <dieselben im Band 20 Hz - 20 kHz: je Fenster das Stueck ohne Mittelwert mit Hann-Fenster, FFT, ausserhalb null>",
           "# hoerort <Momentary-Maximum LUFS> <Pegel 1-2 s gegen 0-0,4 s dB>"]
    geladen = {}
    for name, instrument, taste, stufe in faelle():
        if instrument not in geladen:
            koerper = KOERPER[instrument]
            geladen[instrument] = (ps.lade_koerper(koerper),
                                   json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8")))
        rechne(name, instrument, taste, stufe, *geladen[instrument], w, aus)
    ziel.write_text("\n".join(aus) + "\n", encoding="utf-8", newline="\n")
    print(f"geschrieben: {ziel}")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "zeitschritt":
        gegen_zeitschritt(sys.argv[2:])
    else:
        main()
