"""Schreibt den gemessenen Koerper der klassischen Gitarre: `classical_guitar.lvinstbodyfit` (Beschreibung) und
`classical_guitar.lvinstbody` (Stufe 34k-5, S199; KQ3 a, KQ23 a, KQ24 a, KQ25 a, KQ26 a; Schritt 4 nach KQ28 a, S201;
Schritte 7 und 8 nach KQ35-1 a und KQ35-2 a, S206).

Aufruf (vom Projekt-Root; braucht numpy und scipy):
    python -B asset/loop/instrument_bodies/make_gitarre.py <qualified_selected_impulses.mat> [--ziel <ordner>]

Die Messdatei liegt ausserhalb des Repos (Archiv, Wechselplatte):
`...\\cmake\\samples\\Klasse 2 - Gezupfte Saite\\Mores_2021_Zenodo4604577\\qualified_selected_impulses.mat`.
**Massgebend sind die Dateien im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Laufzeit rund 5 min.

Quelle: R. Mores, „Archive for the acoustical documentation of classical Spanish guitars, flamenco guitars and romantic
guitars - bridge mobility“, Zenodo 4604577 (2021), CC BY 4.0 — Gitarre g34 (Manuel Contreras 1971; KQ23 a).
Weg nach Ducceschi, Russo & Webb, DAFx26 (2026) Par. 3.4, mit Abweichungen. Herleitung und Zahlen:
`projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-15_S199_gitarre_modalfit.md`.

Der Fit in acht Schritten (FIT_FASSUNG = `kFitFassung` in `InstrumentBody/Koerper.hpp`; hochzaehlen, wenn sich bei
gleicher Messung ein anderer Koerper ergaebe):
  1 Kurven       Accelerance A/F und Druck P/F je Schlagort aus 1 s, Exponentialfenster 3 1/s ab dem Schlag (wird von
                 der Daempfung abgezogen), die Kraft nur 5 ms um den Schlag
  2 Pole         ESPRIT ueber alle sechs Stegkurven gemeinsam, je Terzband 60 Hz - 5 kHz; Moden unter -25 dB gegen ihr
                 Band und doppelte Pole fallen heraus
  3 Paare        unter 300 Hz: ein Paar, dessen Abstand kleiner ist als die mittlere Breite, wird ein Pol (Frequenz und
                 Daempfung nachgestellt), wenn der Fehler im Band dadurch um hoechstens 0,2 dB steigt
  4 Steg         Normalmoden mit einer Form phi(x) = a + b x + c x^2 laengs der Stegeinlage (starrer Stab mit
                 Biegung). Zuerst an den sechs Stegkurven allein (R(Schlag, Aufnehmer) = phi(x_Schlag)
                 phi(x_Aufnehmer), Gauss-Newton) - das ist der Start. Dann an den Stegkurven und den neun Schallkurven
                 (drei Schlagorte, drei Mikrofone: P/F = Summe phi(x_Schlag) G) gemeinsam, beide mit gleichem Gewicht,
                 abwechselnd die Gewichte je Mikrofon und die Form, acht Runden (KQ28 a): die zwei Aufnehmer an den
                 Enden der Stegeinlage allein legen die Form zwischen 500 und 1000 Hz nicht fest. Daraus phi an jeder
                 Saite
  5 Abstrahlung  je Hoerort (Mikrofon bass = left, treble = right, 10 cm ueber der Decke): bis zur Grenze ein komplexes
                 Gewicht je Mode aus dem Fit an P/F fuer alle drei Schlagorte; darueber ein Betrag je Terzband mit
                 gezogenem Vorzeichen (Samen 1), so gestellt, dass der Terzpegel im Mittel ueber die Schlagorte stimmt
                 (KQ24 a). Die Grenze (500 Hz oder 1 kHz) nach dem kleineren Fehler ueber beide Hoerorte.
  6 Daempfung    das Daempfungsgesetz alpha(f) = a f^2 + b f + c aus dem Abklingen der Mikrofone je Terzband
                 2,5-10 kHz (Schroeder-Kurve, -5 ... -25 dB) - fuer die gezogenen Moden (KQ26 a)
  7 Dichte       (KQ35 a, S205/S206) ESPRIT findet ueber 2,2 kHz nur die Moden, die an den Messpunkten stark sind.
                 2,2-5 kHz: die gefitteten Moden bleiben; dazu gezogene Moden bis zur Dichte der Gitarre vom
                 Gitarrenbauer nach Elie u. a. 2011 Abb. 4b (0,027 / 0,026 / 0,0235 /Hz bis 3 / 4 / 5 kHz; Samen 1).
                 Sind die Formwerte normalverteilt und ist nur der Anteil r der Moden gefunden, fehlen die mit
                 |z| < t, P(|z| > t) = r; ihr Anteil an der Energie ist m = erf(t / sqrt 2) - t sqrt(2 / pi)
                 exp(-t^2 / 2). Eine gezogene Mode ist eine zufaellig gewaehlte gefittete Mode desselben Bands (Form
                 laengs der Stegeinlage und Abstrahlgewicht bleiben beisammen), ihr Formwert mal
                 sqrt((m / (1 - r)) / ((1 - m) / r)), Vorzeichen gezogen, Daempfung nach dem Gesetz; die Phase des
                 Abstrahlgewichts gezogen, sein Betrag je Band so gestellt, dass der Terzpegel Druck je Kraft wieder
                 liegt wie mit den gefitteten Moden allein (hoechstens der Betrag der kopierten Mode).
                 Ab 5 kHz der statistische Teil bis 20 kHz: Dichte 0,0215 /Hz (Elie 5-6 kHz, darueber fortgesetzt -
                 Annahme), Streuung am Steg = die der gefitteten Moden 2,5-5 kHz mal sqrt(r / (1 - m)) (die mittlere
                 Admittanz bleibt); der Betrag der Abstrahlung je Hoerort so gestellt, dass der Terzpegel 6,3 / 8 /
                 10 kHz im Mittel stimmt - gerechnet mit den Moden, die `ziehe` daraus zieht (hier nachgebaut:
                 SplitMix64, Rayleigh, Box-Muller; Samen 1). Die Messung traegt bis rund 10 kHz (Stoerabstand an den
                 Mikrofonen 11-16 dB); darueber ist das Gesetz fortgesetzt, nicht geprueft.
  8 Richtung     (KQ35-1 a, KQ35-2 a) die Messung gibt nur die Bewegung senkrecht zur Decke. Je Mode ein Winkel
                 gegen die Senkrechte: |tan| = 10^((Yzy - Yzz) / 20) des Bands nach Elie 2012 Abb. 3.17 (Yamaha
                 CG131S, 183 Hz - 4 kHz; unter 150 Hz die Werte von 150-300 Hz, ueber 4 kHz die von 1-4 kHz - Annahme),
                 Vorzeichen gezogen (Samen 7). Dazu je Mode eine zweite, die den Steg nur parallel zur Decke bewegt
                 (Winkel 90 Grad): auf halbem Weg zur naechsten Mode, Daempfung wie die Mode, ohne Abstrahlung,
                 Formwert = kappa^(1/4) mal der senkrechte Formwert der Mode, kappa = 10^(dYyy / 10) -
                 10^(2 dYzy / 10) des Bands - sie traegt, was Yyy ueber einem Winkel je Mode liegt. Zahl, Lage und
                 Daempfung dieser Moden sind Annahme. Der statistische Teil traegt Winkel und Anteil fuer die
                 gezogenen Moden. In der Datei steht je Mode der Formwert laengs ihrer Richtung (`bridge`) und der
                 Winkel (`bridgeAngle`): senkrecht bridge cos, parallel bridge sin.
"""
import hashlib
import json
import math
import pathlib
import sys
import time

import numpy as np
import scipy.io as sio
import scipy.signal as sig
from scipy.optimize import least_squares, minimize, nnls

FIT_FASSUNG = 4
KOERPER_FASSUNG = 7                      # `kKoerperFassung` in InstrumentBody/Koerper.hpp
GITARRE = 34
MAT_SHA256 = "8a016fab0b3f086a49b2e740c2588fc61938fd09251aab5314413ea4bab72fb6"
FS = 48000
N_VOLL = (10000 / 92.90) * 4.4482        # N je Vollausschlag (Hammer 92,90 mV/lbf, +-10 V)
A_VOLL = (10000 / 10.64) * 9.80665       # m/s2 je Vollausschlag (Aufnehmer 10,64 mV/g)
P_VOLL = 10000 / 50.0                    # Pa je Vollausschlag (Mikrofon 50 mV/Pa)
ORTE = ("bass", "center", "treble")
FENSTER_S = 3.0
VON, BIS = 60.0, 5000.0
STAT_BIS = 20000.0                       # Ende des statistischen Teils
STAT_BAND = (2500.0, 5000.0)             # woraus Dichte und Streuung am Steg geschaetzt werden
STAT_SAMEN = 1
# Schritt 7 (KQ35 a): Soll der Modendichte nach Elie u. a. 2011 Abb. 4b (Gitarre vom Gitarrenbauer; abgelesen in
# Bericht 23 der Research-Werkbank): von, bis, Moden je Hz; ab 5 kHz der Wert von 5-6 kHz
SOLL_DICHTE = [(2200.0, 3000.0, 0.027), (3000.0, 4000.0, 0.026), (4000.0, 5000.0, 0.0235)]
SOLL_DICHTE_AB_5K = 0.0215
FUELL_SAMEN = 1
# Betrag der Abstrahlung der gezogenen Moden: je Auffuellband die Terzbaender (2,5; 3,15; 4; 5 kHz), an denen er haengt
# - das Band 4 kHz reicht in zwei Auffuellbaender -, Schritt je Runde in dB mal FUELL_RELAX, hoechstens der Betrag der
# kopierten Mode. S206 ueber acht Samen verglichen: mit ([0], [1, 2], [3]), 3 und ohne Deckel (Probe S205) liegt das
# Band 4 kHz je Samen bis 8,6 dB ueber der Messung, so hoechstens 1,3 dB.
FUELL_TERZ = ([0], [1, 2], [2, 3])
FUELL_RELAX = 1.0
FUELL_MAX = 1.0
# Schritt 8 (KQ35-1 a): Elie 2012 Abb. 3.17 (Bericht 23 Tab. 2.2): von, bis, Yzy - Yzz, Yyy - Yzz in dB
ELIE = [(150, 300, -14.2, -22.1), (300, 500, -18.3, -23.7), (500, 1000, -13.5, -10.4), (1000, 1500, -19.3, -19.2),
        (1500, 2000, -9.5, -11.7), (2000, 2500, -13.5, -15.1), (2500, 3000, -16.9, -16.2), (3000, 3500, -12.9, -18.9),
        (3500, 4000, -9.2, -14.2)]
ELIE_GESAMT = (-12.9, -15.6)             # 1-4 kHz
RICHTUNG_SAMEN = 7
PAARE = [("bass", "bass"), ("bass", "treble"), ("center", "bass"), ("center", "treble"), ("treble", "bass"),
         ("treble", "treble")]
SAITENABSTAND = 0.0116
X_SCHLAG = {"bass": -2 * SAITENABSTAND, "center": 0.0, "treble": 2 * SAITENABSTAND}
X_AUFN = {"bass": -0.036, "treble": 0.036}
X_SAITEN = [(k - 2.5) * SAITENABSTAND for k in range(6)]     # Saite 6 (E2) ... 1 (E4), Bass negativ
TASTEN = [40, 45, 50, 55, 59, 64]
SAITEN_NAMEN = ["6 (E2)", "5 (A2)", "4 (D3)", "3 (G3)", "2 (B3)", "1 (E4)"]
MIKROFONE = (("left", "mbass"), ("right", "mtreble"))
FORM_MIKROFONE = ("mbass", "mtreble", "east")           # Schritt 4: alle drei Mikrofone tragen die Form am Schlagort
GEMEINSAM_RUNDEN = 8
QUELLE = ("R. Mores, Archive for the acoustical documentation of classical Spanish guitars, flamenco guitars and "
          "romantic guitars - bridge mobility, Zenodo 4604577 (2021), CC BY 4.0; guitar g34: Manuel Contreras, Madrid "
          "1971, classical Spanish, cedar / Rio palisander, scale 662 mm, 1670 g, measured in the free-field laboratory "
          "of the Hamburg University of Applied Sciences on 3 Dec. 2018")


# ---------------------------------------------------------------- 1 Kurven
def frfs(daten):
    aus = {}
    hz = np.fft.rfftfreq(FS, 1 / FS)
    for ort, x in daten.items():
        ip = int(np.argmax(np.abs(x[:, 0])))
        start = max(0, ip - 48)
        t = (np.arange(FS) - start) / FS
        w = np.where(t >= 0, np.exp(-FENSTER_S * t), 0.0)
        fw = np.zeros(FS)
        fw[start:start + 240] = sig.windows.tukey(240, 0.3)
        F = np.fft.rfft(x[:, 0] * fw * N_VOLL)
        for kanal, name in ((1, "treble"), (2, "bass")):
            aus[(ort, name)] = np.fft.rfft(x[:, kanal] * w * A_VOLL) / F
        for kanal, name in ((3, "east"), (4, "mtreble"), (5, "mbass")):
            aus[(ort, name)] = np.fft.rfft(x[:, kanal] * w * P_VOLL) / F
    return hz, aus


# ---------------------------------------------------------------- 2 Pole
def esprit(x, k):
    kan, n = x.shape
    l = n // 2
    h = np.concatenate([np.lib.stride_tricks.sliding_window_view(x[c], l).T for c in range(kan)], axis=1)
    u, _, _ = np.linalg.svd(h, full_matrices=False)
    k = min(k, u.shape[1] - 1)
    us = u[:, :k]
    return np.linalg.eigvals(np.linalg.lstsq(us[:-1], us[1:], rcond=None)[0])


def pole_im_band(acc, fl, fh, ordnung):
    fc, bw = 0.5 * (fl + fh), fh - fl
    dez = max(1, int(FS // max(4.0 * bw, 200.0)))
    rate = FS / dez
    t = np.arange(FS) / FS
    kan = []
    for y in acc:
        hb = np.fft.irfft(y, FS) * np.exp(-2j * np.pi * fc * t)
        if dez > 1:
            hb = sig.lfilter(sig.firwin(8 * dez + 1, 0.8 / dez), 1.0, hb)[4 * dez::dez]
        n = int(min(len(hb), 0.35 * rate))
        kan.append(hb[2:n + 2])
    z = esprit(np.array(kan), ordnung)
    f = fc + np.angle(z) * rate / (2 * np.pi)
    s = -np.log(np.abs(z)) * rate - FENSTER_S
    gut = (f >= fl) & (f < fh) & (s > 0.5) & (s < 0.25 * 2 * np.pi * f)
    return f[gut], s[gut]


def basis(hz, f, s):
    w = 2 * np.pi * hz[:, None]
    wj = 2 * np.pi * f[None, :]
    return 1j * w / (wj ** 2 - w ** 2 + 2j * s[None, :] * w)


def residuen(hz, y, f, s, sel):
    w = 2 * np.pi * hz[sel]
    b = np.concatenate([basis(hz[sel], f, s), (1 / (1j * w))[:, None], (1j * w)[:, None] * 1e-8], axis=1)
    gew = 1.0 / np.maximum(np.abs(y[sel]), 1e-6)
    a = np.concatenate([(b * gew[:, None]).real, (b * gew[:, None]).imag])
    r = np.concatenate([(y[sel] * gew).real, (y[sel] * gew).imag])
    x = np.linalg.lstsq(a, r, rcond=None)[0]
    return x, b @ x


def frei(hz, mob, f, s, sel):
    xs, fits = [], []
    for y in mob:
        x, yf = residuen(hz, y, f, s, sel)
        xs.append(x)
        fits.append(yf)
    xs = np.array(xs)
    return xs[:, :len(f)], xs[:, len(f):], fits


def fehler(hz, sel, mob, fits, von, bis):
    m = (hz[sel] >= von) & (hz[sel] < bis)
    d = [20 * np.log10(np.abs(yf[m]) / np.abs(y[sel][m])) for y, yf in zip(mob, fits)]
    return float(np.sqrt(np.mean(np.concatenate(d) ** 2)))


def pole(hz, acc, mob, sel):
    kanten = [VON * 2 ** (k / 3) for k in range(int(np.ceil(3 * np.log2(BIS / VON))) + 1)]
    kanten[-1] = BIS
    fs_, ss_ = [], []
    for fl, fh in zip(kanten, kanten[1:]):
        rand = 0.08 * (fh - fl)
        f, s = pole_im_band(acc, fl - rand, fh + rand, ordnung=int(np.clip(6 + 0.02 * (fh - fl) * 3, 8, 40)))
        innen = (f >= fl) & (f < fh)
        fs_.extend(f[innen])
        ss_.extend(s[innen])
    f, s = np.array(fs_), np.array(ss_)
    ordn = np.argsort(f)
    f, s = f[ordn], s[ordn]
    for _ in range(3):
        R, _, _ = frei(hz, mob, f, s, sel)
        spitze = np.abs(R) / (2 * s[None, :])
        band = np.array([[np.sqrt(np.mean(np.abs(y[(hz > fj / 1.12) & (hz < fj * 1.12)]) ** 2)) for fj in f] for y in mob])
        bleibt = (spitze / band).max(axis=0) > 10 ** (-25 / 20)
        for i in range(len(f) - 1):
            if bleibt[i] and bleibt[i + 1] and (f[i + 1] - f[i]) < 0.25 * (s[i] + s[i + 1]) / (2 * np.pi):
                bleibt[i if spitze[:, i].max() < spitze[:, i + 1].max() else i + 1] = False
        if bleibt.all():
            break
        f, s = f[bleibt], s[bleibt]
    return f, s


# ---------------------------------------------------------------- 3 Paare
def lege_paare_zusammen(hz, mob, f, s, sel, bericht):
    i = 0
    while i < len(f) - 1:
        if f[i + 1] < 300.0 and (f[i + 1] - f[i]) < 0.5 * (s[i] + s[i + 1]) / np.pi:
            von, bis = 0.85 * f[i], 1.15 * f[i + 1]
            vorher = fehler(hz, sel, mob, frei(hz, mob, f, s, sel)[2], von, bis)
            f1, s1 = np.delete(f, i + 1), np.delete(s, i + 1)

            def ziel(p):
                ff, ss = f1.copy(), s1.copy()
                ff[i], ss[i] = p[0], abs(p[1])
                return fehler(hz, sel, mob, frei(hz, mob, ff, ss, sel)[2], von, bis)

            los = minimize(ziel, [0.5 * (f[i] + f[i + 1]), 0.5 * (s[i] + s[i + 1])], method="Nelder-Mead",
                           options={"xatol": 0.01, "fatol": 1e-3, "maxiter": 300})
            nimmt = los.fun <= vorher + 0.2
            bericht.append({"pair": [round(float(f[i]), 2), round(float(f[i + 1]), 2)], "errorPairDb": round(vorher, 2),
                            "errorSingleDb": round(float(los.fun), 2), "merged": bool(nimmt),
                            "single": [round(float(los.x[0]), 2), round(float(np.pi * los.x[0] / abs(los.x[1])), 1)]})
            if nimmt:
                f1[i], s1[i] = los.x[0], abs(los.x[1])
                f, s = f1, s1
                continue        # dieselbe Stelle noch einmal gegen den naechsten Pol pruefen
        i += 1
    ordn = np.argsort(f)
    return f[ordn], s[ordn]


# ---------------------------------------------------------------- 4 Steg
def stegfit(hz, mob, f, s, sel, R, rest):
    J = len(f)
    B = basis(hz[sel], f, s)
    w = 2 * np.pi * hz[sel]
    rest_b = np.stack([1 / (1j * w), 1j * w * 1e-8], axis=1)
    xh = np.array([X_SCHLAG[p[0]] for p in PAARE])
    xs = np.array([X_AUFN[p[1]] for p in PAARE])
    Y = np.array([y[sel] for y in mob])
    gew = 1.0 / np.maximum(np.abs(Y), 1e-6)
    ib, it = PAARE.index(("bass", "bass")), PAARE.index(("treble", "treble"))
    a0, b0 = np.zeros(J), np.zeros(J)
    for j in range(J):
        best = None
        pb, pt = np.sqrt(abs(R[ib, j])), np.sqrt(abs(R[it, j]))
        for vz in (1.0, -1.0):
            b = (vz * pt - pb) / (X_AUFN["treble"] - X_AUFN["bass"])
            a = pb - b * X_AUFN["bass"]
            fehl = np.sum(((a + b * xh) * (a + b * xs) - R[:, j]) ** 2)
            if best is None or fehl < best[0]:
                best = (fehl, a, b)
        a0[j], b0[j] = best[1], best[2]
    skala = np.array([1.0, 30.0, 900.0])
    ph = np.stack([xh ** k * skala[k] for k in range(3)])
    ps = np.stack([xs ** k * skala[k] for k in range(3)])
    p0 = np.concatenate([a0, b0 / skala[1], np.zeros(J), rest.ravel()])
    nk, n = len(PAARE), B.shape[0]

    def formen(p):
        k = p[:3 * J].reshape(3, J)
        return k.T @ ph, k.T @ ps

    def modell(p):
        fh, fs_ = formen(p)
        return (fh * fs_).T @ B.T + p[3 * J:].reshape(nk, 2) @ rest_b.T

    def res(p):
        d = (modell(p) - Y) * gew
        return np.concatenate([d.real.ravel(), d.imag.ravel()])

    def jac(p):
        fh, fs_ = formen(p)
        Jm = np.zeros((2, nk, n, 3 * J + 2 * nk))
        for q in range(nk):
            gb = gew[q][:, None] * B
            for k in range(3):
                block = gb * (ph[k, q] * fs_[:, q] + ps[k, q] * fh[:, q])[None, :]
                Jm[0, q, :, k * J:(k + 1) * J] = block.real
                Jm[1, q, :, k * J:(k + 1) * J] = block.imag
            rb = gew[q][:, None] * rest_b
            Jm[0, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = rb.real
            Jm[1, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = rb.imag
        return Jm.reshape(2 * nk * n, -1)

    los = least_squares(res, p0, jac=jac, method="trf", max_nfev=200, x_scale="jac")
    k = los.x[:3 * J].reshape(3, J) * skala[:, None]
    return k[0], k[1], k[2], list(modell(los.x))


def gemeinsam(hz, H, mob, f, s, sel, a, b, c):
    """KQ28 a: die Form je Mode an den sechs Stegkurven UND den neun Schallkurven gemeinsam. Abwechselnd die Gewichte je
    Mikrofon (linear, ohne Restterm - die Koerperdatei traegt keinen) und a, b, c je Mode mit dem Rest der Stegkurven
    (Gauss-Newton auf jeder zweiten Frequenz); Fehler relativ, beide Kurvenarten mit gleichem Gewicht."""
    J = len(f)
    hzs = hz[sel]
    w = 2 * np.pi * hzs
    Bm = basis(hzs, f, s)
    Bp = Bm * (1j * w[:, None])
    rest_b = np.stack([1 / (1j * w), 1j * w * 1e-8], axis=1)
    xh_p = np.array([X_SCHLAG[p[0]] for p in PAARE])
    xs_p = np.array([X_AUFN[p[1]] for p in PAARE])
    xh = np.array([X_SCHLAG[o] for o in ORTE])
    Ysteg = np.array([y[sel] for y in mob])
    gew_steg = 1.0 / np.maximum(np.abs(Ysteg), 1e-6)
    schall = [(h, mik) for h in range(len(ORTE)) for mik in FORM_MIKROFONE]
    Ymik = np.array([H[(ORTE[h], mik)][sel] for h, mik in schall])
    gew_mik = 1.0 / np.maximum(np.abs(Ymik), 1e-9)
    skala = np.array([1.0, 30.0, 900.0])
    nk, n = len(PAARE), len(hzs)
    ph_h = np.stack([xh_p ** e * skala[e] for e in range(3)])
    ph_s = np.stack([xs_p ** e * skala[e] for e in range(3)])
    pm_h = np.stack([xh ** e * skala[e] for e in range(3)])
    i2 = slice(None, None, 2)
    n2 = len(hzs[i2])

    def gewichte(k):
        fm = k.T @ pm_h                                             # (J, Schlagorte)
        G = {}
        for mik in FORM_MIKROFONE:
            zeilen = [q for q, (_, m) in enumerate(schall) if m == mik]
            A = np.concatenate([Bp * fm[:, schall[q][0]][None, :] * gew_mik[q][:, None] for q in zeilen])
            G[mik] = np.linalg.lstsq(A, np.concatenate([Ymik[q] * gew_mik[q] for q in zeilen]), rcond=None)[0]
        return G

    def steg_rest(k):
        kern = ((k.T @ ph_h) * (k.T @ ph_s)).T @ Bm.T
        rest = np.zeros((nk, 2))
        for q in range(nk):
            A = rest_b * gew_steg[q][:, None]
            r = (Ysteg[q] - kern[q]) * gew_steg[q]
            rest[q] = np.linalg.lstsq(np.concatenate([A.real, A.imag]), np.concatenate([r.real, r.imag]), rcond=None)[0]
        return rest

    def teile(p):
        return p[:3 * J].reshape(3, J), p[3 * J:].reshape(nk, 2)

    k = np.stack([a, b, c]) / skala[:, None]
    verlauf = []
    for _ in range(GEMEINSAM_RUNDEN):
        G = gewichte(k)
        Gq = np.array([G[m] for _, m in schall])                    # (Schallkurven, J)

        def res(p):
            kk, rest = teile(p)
            ds = (((kk.T @ ph_h) * (kk.T @ ph_s)).T @ Bm[i2].T + rest @ rest_b[i2].T - Ysteg[:, i2]) * gew_steg[:, i2]
            fm = kk.T @ pm_h
            dm = np.array([(Bp[i2] @ (fm[:, h] * Gq[q]) - Ymik[q][i2]) * gew_mik[q][i2] for q, (h, _) in enumerate(schall)])
            return np.concatenate([ds.real.ravel(), ds.imag.ravel(), dm.real.ravel(), dm.imag.ravel()])

        def jac(p):
            kk, _ = teile(p)
            fh, fs_ = kk.T @ ph_h, kk.T @ ph_s
            Js = np.zeros((2, nk, n2, 3 * J + 2 * nk))
            for q in range(nk):
                gb = gew_steg[q][i2][:, None] * Bm[i2]
                for e in range(3):
                    block = gb * (ph_h[e, q] * fs_[:, q] + ph_s[e, q] * fh[:, q])[None, :]
                    Js[0, q, :, e * J:(e + 1) * J] = block.real
                    Js[1, q, :, e * J:(e + 1) * J] = block.imag
                rb = gew_steg[q][i2][:, None] * rest_b[i2]
                Js[0, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = rb.real
                Js[1, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = rb.imag
            Jm = np.zeros((2, len(schall), n2, 3 * J + 2 * nk))
            for q, (h, _) in enumerate(schall):
                gb = gew_mik[q][i2][:, None] * Bp[i2] * Gq[q][None, :]
                for e in range(3):
                    Jm[0, q, :, e * J:(e + 1) * J] = (gb * pm_h[e, h]).real
                    Jm[1, q, :, e * J:(e + 1) * J] = (gb * pm_h[e, h]).imag
            return np.concatenate([Js.reshape(2 * nk * n2, -1), Jm.reshape(2 * len(schall) * n2, -1)])

        los = least_squares(res, np.concatenate([k.ravel(), steg_rest(k).ravel()]), jac=jac, method="trf", max_nfev=25,
                            x_scale="jac")
        k = teile(los.x)[0]
        verlauf.append(round(float(np.sqrt(np.mean(los.fun ** 2))), 4))
    fits = list(((k.T @ ph_h) * (k.T @ ph_s)).T @ Bm.T + steg_rest(k) @ rest_b.T)
    k = k * skala[:, None]
    return k[0], k[1], k[2], fits, verlauf


# ---------------------------------------------------------------- 5 Abstrahlung
def abstrahlung(hz, H, f, s, a, b, c, sel):
    w = 2 * np.pi * hz[sel]
    B = basis(hz[sel], f, s) * (1j * w[:, None])              # -w^2 / (w_j^2 - w^2 + 2 i s_j w)
    phi = np.array([a + b * X_SCHLAG[o] + c * X_SCHLAG[o] ** 2 for o in ORTE])
    n = B.shape[0]
    mitten = [m for m in (63 * 2 ** (k / 3) for k in range(20)) if m * 2 ** (1 / 6) <= BIS * 1.02]
    mm = np.array(mitten)

    def terz(y):
        return np.array([10 * np.log10(np.mean(np.abs(y[(hz[sel] >= m / 2 ** (1 / 6)) & (hz[sel] < m * 2 ** (1 / 6))]) ** 2))
                         for m in mitten])

    je_grenze = {}
    for grenze in (500.0, 1000.0):
        gewichte, fehl = [], []
        for _, mik in MIKROFONE:
            Y = np.concatenate([H[(o, mik)][sel] for o in ORTE])
            gew = 1.0 / np.maximum(np.abs(Y), 1e-9)
            A = np.concatenate([B * phi[k][None, :] for k in range(3)])
            rest = np.zeros((3 * n, 6), dtype=complex)
            for k in range(3):
                rest[k * n:(k + 1) * n, 2 * k] = 1.0
                rest[k * n:(k + 1) * n, 2 * k + 1] = 1e4 / w ** 2
            M = np.concatenate([A, rest], axis=1)
            G = np.linalg.lstsq(M * gew[:, None], Y * gew, rcond=None)[0][:len(f)]
            mess = np.array([terz(Y.reshape(3, n)[k]) for k in range(3)])
            oben = f >= grenze
            G[oben] = np.random.default_rng(1).choice([-1.0, 1.0], size=int(oben.sum())) * np.median(np.abs(G[oben]))
            for _ in range(12):
                syn = np.array([terz((B * phi[k][None, :]) @ G) for k in range(3)])
                for ib, bm in enumerate(mitten):
                    jm = oben & (f >= bm / 2 ** (1 / 6)) & (f < bm * 2 ** (1 / 6))
                    if bm * 2 ** (1 / 6) > grenze and jm.any():
                        diff = 10 * np.log10(np.mean(10 ** (mess[:, ib] / 10)) / np.mean(10 ** (syn[:, ib] / 10)))
                        G[jm] *= 10 ** (np.clip(diff, -6, 6) / 20)
            syn = np.array([terz((B * phi[k][None, :]) @ G) for k in range(3)])
            d = (syn - mess)[:, mm >= 95]                       # ab dem Band 100 Hz (die tiefste Saite liegt bei 82 Hz)
            gewichte.append(G)
            fehl.append({"all": float(np.sqrt(np.mean(d ** 2))),
                         "bands": {name: round(float(np.sqrt(np.mean((syn - mess)[:, (mm >= lo) & (mm < hi)] ** 2))), 2)
                                   for name, lo, hi in (("100-400", 95, 450), ("500-1000", 450, 1120),
                                                        ("1250-2500", 1120, 2800), ("3150-5000", 2800, 5700))}})
        je_grenze[grenze] = (gewichte, fehl, float(np.sqrt(np.mean([e["all"] ** 2 for e in fehl]))))
    grenze = min(je_grenze, key=lambda g: je_grenze[g][2])
    return grenze, je_grenze[grenze][0], {str(int(g)): {"rmsDb": round(v[2], 2), "left": v[1][0]["bands"], "right": v[1][1]["bands"]}
                                          for g, v in je_grenze.items()}


# ---------------------------------------------------------------- 6 Statistik
class Zufall:
    """SplitMix64 und die Umrechnungen wie `lumi::koerper::Zufall` (InstrumentBody/Statistik.hpp)."""
    M64 = (1 << 64) - 1

    def __init__(self, samen):
        self.z = samen

    def naechste(self):
        self.z = (self.z + 0x9E3779B97F4A7C15) & self.M64
        z = self.z
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & self.M64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & self.M64
        return z ^ (z >> 31)

    def gleich(self):
        return ((self.naechste() >> 11) + 0.5) * (1.0 / 9007199254740992.0)

    def rayleigh(self):
        return math.sqrt(-4.0 * math.log(self.gleich()) / math.pi)

    def normal(self):
        u = self.gleich()
        v = self.gleich()
        return math.sqrt(-2.0 * math.log(u)) * math.cos(2.0 * math.pi * v)


def ziehe(st):
    """Wie `lumi::koerper::ziehe`: f, sigma, Formwerte laengs der Richtung (Moden x Saiten), Gewichte (Moden x
    Hoerorte), Winkel in Grad. Mit `bridgeAngle` traegt jede Mode den Winkel mit gezogenem Vorzeichen; mit
    `parallelFactor` kommt je Mode eine zweite nur parallel zur Decke dazu (KQ35-1 a, KQ35-2 a)."""
    z = Zufall(st["seed"])
    a, b, c = st["damping"]
    winkel0, anteil = st.get("bridgeAngle", 0.0), st.get("parallelFactor", 0.0)
    f, fs_, ss_, steg, G, wi = st["fromHz"], [], [], [], [], []
    while True:
        f += z.rayleigh() / (st["density"] + st["rise"] * (f - st["fromHz"]))
        if f >= st["toHz"]:
            break
        fs_.append(f)
        ss_.append(0.5 * (a * f * f + b * f + c))
        steg.append([s * z.normal() for s in st["bridgeSpread"]])
        zeile = []
        for betrag in st["radiation"]:
            phase = 2.0 * math.pi * z.gleich()
            zeile.append(complex(betrag * math.cos(phase), betrag * math.sin(phase)))
        G.append(zeile)
        wi.append(0.0 if winkel0 == 0.0 else (winkel0 if z.gleich() < 0.5 else -winkel0))
    fs_, ss_, steg, G, wi = np.array(fs_), np.array(ss_), np.array(steg), np.array(G), np.array(wi)
    if not anteil > 0.0 or len(fs_) < 2:
        return fs_, ss_, steg, G, wi
    n = len(fs_)
    fy = 0.5 * (fs_[:-1] + fs_[1:])
    alle = lambda eigen, dazu: np.concatenate([eigen, dazu])[np.argsort(np.concatenate([2 * np.arange(n), 2 * np.arange(n - 1) + 1]))]  # noqa: E731
    return (alle(fs_, fy), alle(ss_, ss_[:-1] * fy / fs_[:-1]),                 # derselbe Verlustfaktor wie die Mode
            alle(steg, anteil * steg[:-1] * np.cos(np.radians(wi[:-1]))[:, None]),
            alle(G, np.zeros_like(G[:-1])), alle(wi, np.full(n - 1, 90.0)))


def abklingen(x, ip, mitte):
    """alpha (1/s, Energie ~ exp(-alpha t)) aus der Schroeder-Kurve des Terzbands, -5 ... -25 dB, Rauschen abgezogen."""
    sos = sig.butter(4, [mitte / 2 ** (1 / 6), mitte * 2 ** (1 / 6)], btype="band", fs=FS, output="sos")
    y = sig.sosfilt(sos, x)[ip:] ** 2
    n = int(0.25 * FS)
    e = np.cumsum((y[:n] - np.mean(y[-4800:]))[::-1])[::-1]
    db = 10 * np.log10(np.maximum(e, 1e-300) / e[0])
    ende = int(np.argmax(db < -25)) if (db < -25).any() else n
    m = db[:ende] <= -5
    return float(-np.polyfit((np.arange(n) / FS)[:ende][m], db[:ende][m], 1)[0] / (10 * np.log10(np.e)))


def daempfungsgesetz(daten):
    """Schritt 6: alpha(f) = a f^2 + b f + c aus dem Abklingen an den Mikrofonen je Terzband 2,5 ... 10 kHz."""
    mitten = [1000 * 2 ** (k / 3) for k in range(4, 11)]
    alpha = []
    for m in mitten:
        werte = []
        for x in daten.values():
            ip = int(np.argmax(np.abs(x[:, 0])))
            werte += [abklingen(x[:, kanal], ip, m) for kanal in (4, 5)]
        alpha.append(float(np.median(werte)))
    fm, al = np.array(mitten), np.array(alpha)
    gesetz = nnls(np.stack([fm ** 2, fm, np.ones(len(fm))], axis=1) / al[:, None], np.ones(len(fm)))[0]
    return mitten, alpha, [float(v) for v in gesetz]


def abgeschnitten(r):
    """t mit P(|z| > t) = r und der Energieanteil m der Moden mit |z| < t (z normalverteilt)."""
    lo, hi = 0.0, 8.0
    for _ in range(60):
        t = 0.5 * (lo + hi)
        lo, hi = (t, hi) if math.erfc(t / math.sqrt(2)) > r else (lo, t)
    return t, math.erf(t / math.sqrt(2)) - t * math.sqrt(2 / math.pi) * math.exp(-t * t / 2)


def form(q, x):
    """phi(x) = a + b x + c x^2 je Mode (Zeilen von q) an den Orten x."""
    return q[:, 0:1] + q[:, 1:2] * x[None, :] + q[:, 2:3] * x[None, :] ** 2


def stegfehler(hz, mob, fa, sa, qa, von, bis):
    """rms von 20 lg |Modell / Messung| ueber die sechs Stegkurven im Band, mit zwei Resttermen je Kurve."""
    sel = (hz >= von) & (hz < bis)
    w = 2 * np.pi * hz[sel]
    B = basis(hz[sel], fa, sa)
    rest = np.stack([1 / (1j * w), 1j * w * 1e-8], axis=1)
    xh = np.array([X_SCHLAG[p[0]] for p in PAARE])
    xa = np.array([X_AUFN[p[1]] for p in PAARE])
    aus = []
    for q, y in enumerate(mob):
        yf = B @ (form(qa, xh[q:q + 1])[:, 0] * form(qa, xa[q:q + 1])[:, 0])
        gew = 1.0 / np.maximum(np.abs(y[sel]), 1e-6)
        A = np.concatenate([(rest * gew[:, None]).real, (rest * gew[:, None]).imag])
        r = np.concatenate([((y[sel] - yf) * gew).real, ((y[sel] - yf) * gew).imag])
        yf = yf + rest @ np.linalg.lstsq(A, r, rcond=None)[0]
        aus.append(20 * np.log10(np.abs(yf) / np.abs(y[sel])))
    return float(np.sqrt(np.mean(np.concatenate(aus) ** 2)))


def elie_band(x):
    """Yzy - Yzz, Yyy - Yzz in dB fuer eine Mode bei x Hz."""
    return next(((a, b) for lo, hi, a, b in ELIE if lo <= x < hi), ELIE[0][2:] if x < 150 else ELIE_GESAMT)


def parallel_anteil(dzy, dyy):
    """Formwert der Mode nur parallel zur Decke gegen den senkrechten Formwert ihrer Mode (KQ35-1 a)."""
    return max(0.0, 10 ** (dyy / 10) - 10 ** (2 * dzy / 10)) ** 0.25


def dichte_und_statistik(hz, H, mob, f, s, abc, G, mitten, alpha, gesetz):
    """Schritt 7: fuellt 2,2-5 kHz auf und stellt den statistischen Teil. -> f, sigma, abc, G je Mode (gefittete
    zuerst, dann die gezogenen), der statistische Teil, die Pruefzahlen."""
    xs = np.array(X_SAITEN)
    steg = form(abc, xs)
    w = 2 * np.pi * np.maximum(hz, 1e-9)
    selp = (hz >= mitten[0] / 2 ** (1 / 6)) & (hz < mitten[-1] * 2 ** (1 / 6))

    def terz(p):
        return np.array([10 * np.log10(np.mean(p[(hz[selp] >= m / 2 ** (1 / 6)) & (hz[selp] < m * 2 ** (1 / 6))])) for m in mitten])

    mess = [terz(np.mean([np.abs(H[(o, mik)][selp]) ** 2 for o in ORTE], axis=0)) for _, mik in MIKROFONE]

    def pegel(fa, sa, phi, g):
        B = basis(hz[selp], fa, sa) * (1j * w[selp, None])
        return [terz(np.mean([np.abs(B @ (phi[:, i] * g[:, h])) ** 2 for i in range(len(TASTEN))], axis=0))
                for h in range(len(MIKROFONE))]

    def mit_statistik(fa, sa, qa, ga, st):
        fz, sz, stegz, gz, wz = ziehe(st)
        return (np.concatenate([fa, fz]), np.concatenate([sa, sz]),
                np.concatenate([form(qa, xs), stegz * np.cos(np.radians(wz))[:, None]]), np.concatenate([ga, gz]))

    baender = (("1000-2200", 1000, 2200), ("2200-3000", 2200, 3000), ("3000-4000", 3000, 4000), ("4000-5000", 4000, 5000),
               ("2500-5000", 2500, 5000))
    fehler_vorher = {n: round(stegfehler(hz, mob, f, s, abc, lo, hi), 2) for n, lo, hi in baender}

    # 2,2-5 kHz: je Band der Anteil r der gefundenen Moden und der Formwert der fehlenden
    plan = []
    for lo, hi, soll in SOLL_DICHTE:
        im = np.where((f >= lo) & (f < hi))[0]
        r = len(im) / (soll * (hi - lo))
        t, m = abgeschnitten(r)
        plan.append((lo, hi, soll, im, r, m, (m / (1 - r)) / ((1 - m) / r)))
    z = Zufall(FUELL_SAMEN)
    fn, sn, qn, gn, bn = [], [], [], [], []
    for nr, (lo, hi, soll, im, r, m, rho) in enumerate(plan):
        x = lo
        zus = soll - len(im) / (hi - lo)
        while True:
            x += z.rayleigh() / zus
            if x >= hi:
                break
            j = im[int(z.gleich() * len(im)) % len(im)]
            vz = 1.0 if z.gleich() < 0.5 else -1.0
            fn.append(x)
            sn.append(0.5 * (gesetz[0] * x * x + gesetz[1] * x + gesetz[2]))
            qn.append(abc[j] * vz * math.sqrt(rho))
            ph = 2 * math.pi * z.gleich()
            gn.append(G[j] * complex(math.cos(ph), math.sin(ph)))
            bn.append(nr)
    fa, sa = np.concatenate([f, fn]), np.concatenate([s, sn])
    qa, gn, bn = np.concatenate([abc, np.array(qn)]), np.array(gn), np.array(bn)
    # der Betrag der Abstrahlung der gezogenen Moden je Band: der Terzpegel wie mit den gefitteten allein
    nur = pegel(f, s, steg, G)
    skala = np.ones(len(plan))
    for _ in range(12):
        syn = pegel(fa, sa, form(qa, xs), np.concatenate([G, gn * skala[bn][:, None]]))
        ab = np.mean([syn[h] - nur[h] for h in range(len(MIKROFONE))], axis=0)
        for b, tb in enumerate(FUELL_TERZ):
            skala[b] = min(FUELL_MAX, max(0.02, skala[b] * 10 ** (-FUELL_RELAX * np.mean(ab[tb]) / 20)))
    ga = np.concatenate([G, gn * skala[bn][:, None]])

    # ab 5 kHz: Dichte nach Elie, Streuung am Steg so, dass die mittlere Admittanz bleibt
    von, bis = STAT_BAND
    band = (f >= von) & (f < bis)
    r_st = band.sum() / sum(soll * (min(hi, bis) - max(lo, von)) for lo, hi, soll in SOLL_DICHTE)
    _, m_st = abgeschnitten(r_st)
    faktor = math.sqrt(r_st / (1 - m_st))
    winkel = math.degrees(math.atan(10 ** (ELIE_GESAMT[0] / 20)))
    st = {"fromHz": BIS, "toHz": STAT_BIS, "density": SOLL_DICHTE_AB_5K, "rise": 0.0,
          # in der Datei der Formwert laengs der Richtung: senkrecht ist er um cos kleiner
          "bridgeSpread": [float(v) for v in np.sqrt(np.mean(steg[band] ** 2, axis=0)) * faktor / math.cos(math.radians(winkel))],
          "bridgeAngle": winkel, "parallelFactor": parallel_anteil(*ELIE_GESAMT),
          "radiation": [float(v) for v in np.sqrt(np.mean(np.abs(G[band]) ** 2, axis=0))],
          "seed": STAT_SAMEN, "bandFromHz": von, "bandToHz": bis, "bandModes": int(band.sum()),
          "damping": list(gesetz)}
    ziel_b = slice(4, 7)                                                       # 6,3 / 8 / 10 kHz
    for _ in range(12):
        syn = pegel(*mit_statistik(fa, sa, qa, ga, st))
        for h in range(len(MIKROFONE)):
            st["radiation"][h] *= float(10 ** (np.mean((mess[h] - syn[h])[ziel_b]) / 20))
    syn = pegel(*mit_statistik(fa, sa, qa, ga, st))
    fz = ziehe(st)[0]
    pruefung = {
        "decayBandsHz": [round(m) for m in mitten], "decayAlphaMeasured": [round(a, 1) for a in alpha],
        "decayAlphaLaw": [round(float(gesetz[0] * m * m + gesetz[1] * m + gesetz[2]), 1) for m in mitten],
        "fill": {"seed": FUELL_SAMEN,
                 "bands": [{"fromHz": lo, "toHz": hi, "targetPerHz": soll, "fitted": int(len(im)), "drawn": int((bn == nr).sum()),
                            "foundShare": round(r, 3), "missingEnergyShare": round(m, 4),
                            "shapeSquaredDrawnDb": round(10 * math.log10(rho), 1),
                            "radiationDrawnDb": round(20 * math.log10(skala[nr]), 1)}
                           for nr, (lo, hi, soll, im, r, m, rho) in enumerate(plan)],
                 "note": "drawn: modes added up to the target density; shapeSquaredDrawnDb: squared bridge value of a drawn "
                         "mode against the fitted mode it copies; radiationDrawnDb: magnitude of its radiation weight "
                         "against that mode (at most 0 dB), set so that the third-octave level stays as with the fitted "
                         "modes alone"},
        "bridgeErrorDb": {"fitted": fehler_vorher,
                          "filled": {n: round(stegfehler(hz, mob, fa, sa, qa, lo, hi), 2) for n, lo, hi in baender},
                          "note": "rms of 20 lg |model / measured| over the six bridge mobilities with two residual terms "
                                  "per curve; fitted modes alone and with the drawn modes"},
        "bridgeSpreadFactor": round(faktor, 3),
        "levelBandsHz": [round(m) for m in mitten],
        "levelErrorDb": {name: [round(float(v), 2) for v in syn[h] - mess[h]] for h, (name, _) in enumerate(MIKROFONE)},
        "drawnModes": int(len(fz)), "drawnFirstHz": [float(v) for v in fz[:3]], "drawnLastHz": float(fz[-1]),
        "note": "level: third-octave level of pressure per force, model (mean over the six strings, all modes) minus "
                "measured (mean over the three strikes); the radiation of the statistical part is set on the bands "
                "6.3 / 8 / 10 kHz; alpha in 1/s, energy ~ exp(-alpha t); drawnModes, drawnFirstHz and drawnLastHz are "
                "the modes `ziehe` of InstrumentBody draws from the statistical part, including the in-plane modes"}
    return fa, sa, qa, ga, len(f), st, pruefung


def richtung(fa, sa, qa, ga, gefittet, st):
    """Schritt 8: je Mode der Winkel und die Moden nur parallel zur Decke. -> die Moden der Datei (aufsteigend), die
    Pruefzahlen. `gefittet`: so viele Moden am Anfang sind gefittet, die uebrigen gezogen."""
    xs = np.array(X_SAITEN)
    ordnung = np.argsort(fa, kind="stable")
    f, s, senkrecht, g = fa[ordnung], sa[ordnung], form(qa, xs)[ordnung], ga[ordnung]
    herkunft = np.where(ordnung < gefittet, "fitted", "drawn")
    eta = s / (np.pi * f)
    z = Zufall(RICHTUNG_SAMEN)
    tan = np.array([(1.0 if z.gleich() < 0.5 else -1.0) * 10 ** (elie_band(x)[0] / 20) for x in f])
    winkel = np.degrees(np.arctan(tan))
    moden = [{"f": float(f[j]), "eta": float(eta[j]), "airShare": 0.0, "springShare": 0.0, "tubeShare": 0.0, "materialShares": [],
              "bridge": [float(v) for v in senkrecht[j] / math.cos(math.radians(winkel[j]))], "bridgeAngle": float(winkel[j]),
              "radiation": [[float(v.real), float(v.imag)] for v in g[j]]} for j in range(len(f))]
    # je Mode eine zweite nur parallel zur Decke, auf halbem Weg zur naechsten; die letzte gegen die erste gezogene
    # Mode des statistischen Teils, wenn sie noch im Fenster liegt
    naechste = np.append(f[1:], ziehe(st)[0][0])
    parallel = 0
    for j in range(len(f)):
        fy = 0.5 * (f[j] + naechste[j])
        if fy >= BIS:
            continue
        moden.append({"f": float(fy), "eta": float(eta[j]), "airShare": 0.0, "springShare": 0.0, "tubeShare": 0.0, "materialShares": [],
                      "bridge": [float(v) for v in parallel_anteil(*elie_band(f[j])) * senkrecht[j]], "bridgeAngle": 90.0,
                      "radiation": [[0.0, 0.0] for _ in MIKROFONE]})
        parallel += 1
    moden.sort(key=lambda m: m["f"])

    # Pruefung an der Saite 1 (E4) gegen Elie: Yzy - Yzz und Yyy - Yzz je Band, alle Moden (auch die gezogenen)
    fz, sz, stegz, _, wz = ziehe(st)
    fn = np.concatenate([[m["f"] for m in moden], fz])
    en = np.concatenate([[m["eta"] for m in moden], sz / (np.pi * fz)])
    phi = np.concatenate([[m["bridge"][-1] for m in moden], stegz[:, -1]])
    wi = np.radians(np.concatenate([[m["bridgeAngle"] for m in moden], wz]))
    pz, py = phi * np.cos(wi), phi * np.sin(wi)
    hz = np.arange(150.0, 4000.0, 0.5)
    w = 2 * np.pi * hz[:, None]
    wn = 2 * np.pi * fn[None, :]
    kern = 1j * w / (wn ** 2 - w ** 2 + 1j * en[None, :] * wn * w)
    Yzz, Yzy, Yyy = kern @ (pz * pz), kern @ (pz * py), kern @ (py * py)
    e = lambda y, m: 10 * np.log10(np.mean(np.abs(y[m]) ** 2))                # noqa: E731
    zeilen = []
    for lo, hi, dzy, dyy in ELIE + [(1000, 4000) + ELIE_GESAMT]:
        m = (hz >= lo) & (hz < hi)
        zeilen.append({"fromHz": lo, "toHz": hi, "elieDb": [dzy, dyy],
                       "bodyDb": [round(float(e(Yzy, m) - e(Yzz, m)), 1), round(float(e(Yyy, m) - e(Yzz, m)), 1)]})
    pruefung = {"modesFitted": int((herkunft == "fitted").sum()), "modesDrawn": int((herkunft == "drawn").sum()),
                "modesInPlane": parallel, "seed": RICHTUNG_SAMEN,
                "statisticalAngleDeg": round(st["bridgeAngle"], 3), "statisticalParallelFactor": round(st["parallelFactor"], 4),
                "string1Bands": zeilen,
                "note": "string1Bands: band levels of the cross term and of the in-plane admittance against the normal "
                        "admittance at string 1 (E4), in dB: [Yzy - Yzz, Yyy - Yzz]; elieDb after Elie 2012 Fig. 3.17 "
                        "(Yamaha CG131S), bodyDb from all modes of this body including the drawn ones"}
    return moden, pruefung


# ---------------------------------------------------------------- Dateien
def beschreibung(pruefung):
    return {
        "format": "lumiviz-instrument-body-fit",
        "version": 1,
        "fitVersion": FIT_FASSUNG,
        "title": "Classical guitar",
        "description": "Modal fit to the measured bridge mobility and near-field sound pressure of a classical Spanish "
                       "guitar from the Mores archive (KQ3 a, KQ23 a to KQ25 a). The body file carries, per mode, the "
                       "frequency, the loss factor, the mode shape at each of the six strings and a radiation weight "
                       "for two listening points. Method after Ducceschi, Russo & Webb (DAFx26, 2026, Sec. 3.4) with "
                       "the full one-second response instead of 62.5 ms, poles from ESPRIT over all six bridge "
                       "responses, and normal modes with one shape along the saddle (rigid bar with bending).",
        "source": QUELLE,
        "dataset": {"doi": "10.5281/zenodo.4604577", "license": "CC BY 4.0", "file": "qualified_selected_impulses.mat",
                    "sha256": MAT_SHA256, "guitar": GITARRE, "sampleRate": FS,
                    "channels": "1 hammer, 2 accelerometer treble, 3 accelerometer bass, 4 microphone upper east, "
                                "5 microphone treble, 6 microphone bass; three strikes of 1 s: bass, center, treble",
                    "sensitivities": "hammer 92.90 mV/lbf, accelerometers 10.64 mV/g, microphones 50 mV/Pa, full "
                                     "scale 10 V (batch_extract_bridge_mobility_archive.m of the archive)"},
        "positions": {"units": "m along the saddle from its centre, bass side negative",
                      "strikes": X_SCHLAG, "accelerometers": X_AUFN, "stringSpacing": SAITENABSTAND,
                      "source": "Mores 2021, Method Fig. 3: strikes between strings 6/5, 4/3 and 2/1, accelerometers at "
                                "the ends of the saddle just outside strings 6 and 1; string spacing 11.6 mm assumed "
                                "(usual size), accelerometers +-36 mm estimated from the photograph (+-3 mm)"},
        "fit": {"fromHz": VON, "toHz": BIS, "window": f"exponential {FENSTER_S} 1/s from the strike, subtracted from the damping",
                "poles": "multichannel ESPRIT per third-octave band over the six bridge accelerances; modes below -25 dB "
                         "of their band and double poles dropped",
                "pairs": "below 300 Hz a pair closer than its mean width becomes one pole (frequency and damping "
                         "re-fitted) if the error in its band rises by at most 0.2 dB",
                "bridge": "normal modes with one shape phi(x) = a + b x + c x^2 along the saddle: first Gauss-Newton on "
                          "the six mobilities alone (residue(strike, sensor) = phi(x_strike) phi(x_sensor)), then "
                          "together with the nine curves of sound pressure per force (three strikes, three "
                          "microphones: sum of phi(x_strike) G), equal weight, error weighted by 1/|Y|, alternating "
                          "between the weights per microphone and the shape, eight rounds (KQ28 a)",
                "radiation": "per listening point one complex weight per mode up to the split frequency, fitted to the "
                             "pressure per force of all three strikes; above it one magnitude per third-octave band "
                             "with a drawn sign (seed 1), set so that the band level matches on average over the strikes",
                "statistical": f"statistical part {BIS:.0f}-{STAT_BIS:.0f} Hz (KQ26 a), quantities from the measurement: "
                               "damping law alpha(f) = a f^2 + b f + c from the decay at the microphones per "
                               "third-octave band 2.5-10 kHz (Schroeder curve, -5 to -25 dB); density and bridge "
                               "spread see `density`; radiation magnitude "
                               "per listening point set so that the third-octave level at 6.3 / 8 / 10 kHz matches "
                               "on average. The measurement carries up to about 10 kHz (signal to noise at the "
                               "microphones 11-16 dB at 10 kHz, below 10 dB above); above that the law is continued "
                               "and not checked. Only the product of bridge value and radiation weight is determined "
                               "by the measurement there, not their split.",
                "density": "KQ35 a: ESPRIT finds above 2.2 kHz only the modes that are strong at the measuring points. "
                           "2.2-5 kHz: the fitted modes stay; modes are drawn (seed 1) up to the modal density of the "
                           "luthier's guitar in Elie, Gautier & David, Acta Acustica united with Acustica 97 (2011) "
                           "Fig. 4b (0.027 / 0.026 / 0.0235 per Hz up to 3 / 4 / 5 kHz, read off the figure). A drawn "
                           "mode copies a fitted mode of its band (shape along the saddle and radiation weight stay "
                           "together) with a smaller bridge value: with normally distributed shape values and the "
                           "share r of the modes found, the missing ones are those with |z| < t, P(|z| > t) = r; "
                           "damping after the law, phase of the radiation drawn, its magnitude per band set so that "
                           "the third-octave level of pressure per force stays as with the fitted modes alone (at most "
                           "the magnitude of the copied mode). From "
                           "5 kHz the statistical part with 0.0215 per Hz (Elie 5-6 kHz, continued above - "
                           "assumption) and the bridge spread of the fitted modes 2.5-5 kHz times sqrt(r / (1 - m)) "
                           "(the mean admittance stays).",
                "direction": "KQ35-1 a, KQ35-2 a: the measurement gives the motion normal to the top plate only. Per "
                             "mode an angle against the normal with |tan| = 10^((Yzy - Yzz) / 20) of its band after "
                             "B. Elie, thesis Universite du Maine 2012, Fig. 3.17 (Yamaha CG131S at the E4 string, "
                             "183 Hz - 4 kHz; below 150 Hz the values of 150-300 Hz, above 4 kHz those of 1-4 kHz - "
                             "assumption), sign drawn (seed 7). In addition per mode a second one that moves the bridge "
                             "in the plane of the top only (angle 90): halfway to the next mode, damping of the mode, "
                             "no radiation, bridge value = kappa^(1/4) times the normal bridge value of the mode with "
                             "kappa = 10^(dYyy / 10) - 10^(2 dYzy / 10) of the band - it carries what Yyy exceeds one "
                             "angle per mode. Number, place and damping of these modes are assumptions. The "
                             "statistical part carries the angle and the factor of 1-4 kHz for the drawn modes. In "
                             "the body file `bridge` is the value along the direction of `bridgeAngle`.",
                "directionBandsDb": [{"fromHz": lo, "toHz": hi, "YzyMinusYzz": a, "YyyMinusYzz": b} for lo, hi, a, b in ELIE]
                                    + [{"fromHz": 1000, "toHz": 4000, "YzyMinusYzz": ELIE_GESAMT[0], "YyyMinusYzz": ELIE_GESAMT[1]}]},
        "bridgePoints": [{"key": t, "x": round(x, 5),
                          "source": f"open string {n}: phi at its place on the saddle from the fitted shape; the key is "
                                    "the open string (fretted notes use the played string)"}
                         for t, x, n in zip(TASTEN, X_SAITEN, SAITEN_NAMEN)],
        "listeningPoints": [{"name": "left", "source": "microphone 'bass': 10 cm above the top plate, 10 cm from the "
                                                       "bridge centre towards the bass side (Mores 2021, Method 2c) - near field"},
                            {"name": "right", "source": "microphone 'treble': 10 cm above the top plate, 10 cm from the "
                                                        "bridge centre towards the treble side - near field"}],
        "assumptions": ["scale of the guitar 662 mm; the strings after Woodhouse et al. 2012 are for 650 mm",
                        "only the motion normal to the top plate is measured on this guitar; the direction at the bridge "
                        "follows band levels of another guitar (Elie 2012), the in-plane modes are assumed in number, "
                        "place and damping",
                        "the modal density above 2.2 kHz follows another guitar (Elie 2011); the modes drawn for it are not measured",
                        "the listening points lie in the near field (10 cm); the other bodies listen at about 1 m",
                        "left = bass side, right = treble side: assigned"],
        "check": pruefung,
    }


def koerper(moden, st, fit_datei, abdruck, sekunden):
    return {
        "format": "lumiviz-instrument-body",
        "version": KOERPER_FASSUNG,
        "title": "Classical guitar",
        "description": "Measured body: modal fit to guitar g34 of the Mores archive (Manuel Contreras, Madrid 1971, "
                       "cedar / Rio palisander, scale 662 mm). Six bridge points, one per open string; two listening "
                       "points 10 cm above the top plate (near field); above 5 kHz a statistical part fitted to the "
                       "measurement. Above 2.2 kHz modes are drawn up to the modal density after Elie 2011; the "
                       "direction at the bridge and the in-plane modes follow Elie 2012. See classical_guitar.lvinstbodyfit.",
        "units": "f in Hz; eta loss factor; bridge: mode shape at the bridge point along the direction of bridgeAngle, "
                 "1/sqrt(kg) (modal mass 1/value^2); bridgeAngle: direction in which the mode moves the bridge, degrees "
                 "from the normal of the top plate, across the strings (normal component bridge * cos, in-plane "
                 "component bridge * sin; 90 = in-plane only); radiation: complex pressure per modal acceleration at "
                 "the listening point, sqrt(kg)/m^2 (fitted to the measured pressure per force)",
        "origin": "measurement",
        "plan": {"file": "", "fingerprint": ""},
        "measurement": {"file": fit_datei, "fingerprint": abdruck, "fitVersion": FIT_FASSUNG},
        "computation": {"version": 0, "mesher": "", "solver": "ESPRIT, Gauss-Newton (make_gitarre.py)", "order": 0,
                        "meshForHz": 0, "fromHz": VON, "toHz": BIS, "unknowns": 0, "seconds": round(sekunden, 1)},
        "bridgePoints": TASTEN,
        "listeningPoints": [name for name, _ in MIKROFONE],
        "air": {"density": 1.29, "speedOfSound": 340},
        "damping": {"law": "alpha(f) = a f^2 + b f + c in 1/s, f in Hz; eta = alpha / (2 pi f)",
                    "a": st["damping"][0], "b": st["damping"][1], "c": st["damping"][2],
                    "source": "statistical part only: fitted to the measured decay at the microphones, third-octave "
                              "bands 2.5-10 kHz (KQ26 a); the loss factor of each fitted mode is measured",
                    "perMaterial": "not used: no materials"},
        "materials": [],
        "modes": moden,
        "statistical": {"fromHz": st["fromHz"], "toHz": st["toHz"], "density": st["density"], "rise": st["rise"],
                        "bridgeSpread": st["bridgeSpread"], "bridgeAngle": st["bridgeAngle"],
                        "parallelFactor": st["parallelFactor"], "radiation": st["radiation"], "seed": str(st["seed"]),
                        "bandFromHz": st["bandFromHz"], "bandToHz": st["bandToHz"], "bandModes": st["bandModes"],
                        "materialShares": [],
                        "damping": {"a": st["damping"][0], "b": st["damping"][1], "c": st["damping"][2]}},
    }


def abdruck_von(inhalt: bytes) -> str:
    """Wie `messungAbdruck` in `loop/Koerper`: SHA-256 ueber die Bytes ohne Wagenruecklauf."""
    return hashlib.sha256(inhalt.replace(b"\r", b"")).hexdigest()


if __name__ == "__main__":
    args = sys.argv[1:]
    if not args or args[0].startswith("--"):
        sys.exit(__doc__)
    ziel = pathlib.Path(args[args.index("--ziel") + 1]) if "--ziel" in args else pathlib.Path(__file__).resolve().parent
    start = time.time()
    roh = pathlib.Path(args[0]).read_bytes()
    if hashlib.sha256(roh).hexdigest() != MAT_SHA256:
        sys.exit("die Messdatei ist nicht die des Archivs (SHA-256 weicht ab)")
    d = sio.loadmat(args[0])["qualified_selected_impulses"][GITARRE - 1]
    daten = {ort: d[o * FS:(o + 1) * FS, :] for o, ort in enumerate(ORTE)}
    hz, H = frfs(daten)
    sel = (hz >= VON) & (hz <= BIS)
    w = 2 * np.pi * np.maximum(hz, 1e-9)
    acc = [H[p] for p in PAARE]
    mob = [y / (1j * w) for y in acc]

    f, s = pole(hz, acc, mob, sel)
    paare = []
    f, s = lege_paare_zusammen(hz, mob, f, s, sel, paare)
    R, rest, fits_frei = frei(hz, mob, f, s, sel)
    a, b, c, fits_allein = stegfit(hz, mob, f, s, sel, R, rest)
    a, b, c, fits_steg, verlauf = gemeinsam(hz, H, mob, f, s, sel, a, b, c)
    grenze, G, strahl = abstrahlung(hz, H, f, s, a, b, c, sel)
    mitten, alpha, gesetz = daempfungsgesetz(daten)
    fa, sa, qa, ga, gefittet, st, st_pruefung = dichte_und_statistik(hz, H, mob, f, s, np.stack([a, b, c], axis=1),
                                                                     np.array(G).T, mitten, alpha, gesetz)
    moden, richtung_pruefung = richtung(fa, sa, qa, ga, gefittet, st)
    bereiche = (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000))
    m6 = (f > 85) & (f < 240)
    phi6 = a + b * X_SAITEN[0] + c * X_SAITEN[0] ** 2
    pruefung = {
        "modes": len(moden), "modesFitted": int(len(f)), "modesAbove1kHz": int((f > 1000).sum()),
        "bridgeErrorDb": {"free": {n: round(fehler(hz, sel, mob, fits_frei, lo, hi), 2) for n, lo, hi in bereiche},
                          "normalModesBridgeOnly": {n: round(fehler(hz, sel, mob, fits_allein, lo, hi), 2) for n, lo, hi in bereiche},
                          "normalModes": {n: round(fehler(hz, sel, mob, fits_steg, lo, hi), 2) for n, lo, hi in bereiche},
                          "jointRounds": verlauf,
                          "note": "rms of 20 lg |fit / measured| over the six bridge mobilities; normalModesBridgeOnly: "
                                  "shape from the bridge mobilities alone (start), normalModes: shape from bridge "
                                  "mobilities and sound pressure together (KQ28 a, in the body); jointRounds: rms of "
                                  "the relative error over all fifteen curves after each round"},
        "pairs": paare,
        "effectiveMassString6Kg": {"value": round(float(1 / np.sum(phi6[m6] ** 2)), 4), "band": "85-240 Hz",
                                   "reference": "Wright 1996 Table 5.4: 0.062 kg (guitar BR2, sound hole sealed)"},
        "radiation": {"splitHz": grenze, "thirdOctaveErrorDb": strahl,
                      "note": "rms error of the third-octave level over bands and the three strikes, per split frequency"},
        "statistical": st_pruefung,
        "direction": richtung_pruefung,
    }
    ziel.mkdir(parents=True, exist_ok=True)
    fit_datei = ziel / "classical_guitar.lvinstbodyfit"
    inhalt = (json.dumps(beschreibung(pruefung), indent=1) + "\n").encode("utf-8")
    fit_datei.write_bytes(inhalt)
    k = koerper(moden, st, fit_datei.name, abdruck_von(inhalt), time.time() - start)
    (ziel / "classical_guitar.lvinstbody").write_bytes((json.dumps(k, indent=4) + "\n").encode("utf-8"))
    print(json.dumps(pruefung, indent=1))
    print(f"{fit_datei} und classical_guitar.lvinstbody: {len(moden)} Moden bis {BIS:.0f} Hz ({len(f)} gefittet, "
          f"{richtung_pruefung['modesDrawn']} gezogen, {richtung_pruefung['modesInPlane']} nur parallel zur Decke), Grenze der Abstrahlung "
          f"{grenze:.0f} Hz, statistisch bis {STAT_BIS:.0f} Hz ({st_pruefung['drawnModes']} gezogene Moden), "
          f"{time.time() - start:.0f} s")
