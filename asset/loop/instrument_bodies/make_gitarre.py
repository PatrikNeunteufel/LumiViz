"""Schreibt den gemessenen Koerper der klassischen Gitarre: `classical_guitar.lvinstbodyfit` (Beschreibung) und
`classical_guitar.lvinstbody` (Stufe 34k-5, S199; KQ3 a, KQ23 a, KQ24 a, KQ25 a, KQ26 a).

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

Der Fit in sechs Schritten (FIT_FASSUNG = `kFitFassung` in `InstrumentBody/Koerper.hpp`; hochzaehlen, wenn sich bei
gleicher Messung ein anderer Koerper ergaebe):
  1 Kurven       Accelerance A/F und Druck P/F je Schlagort aus 1 s, Exponentialfenster 3 1/s ab dem Schlag (wird von
                 der Daempfung abgezogen), die Kraft nur 5 ms um den Schlag
  2 Pole         ESPRIT ueber alle sechs Stegkurven gemeinsam, je Terzband 60 Hz - 5 kHz; Moden unter -25 dB gegen ihr
                 Band und doppelte Pole fallen heraus
  3 Paare        unter 300 Hz: ein Paar, dessen Abstand kleiner ist als die mittlere Breite, wird ein Pol (Frequenz und
                 Daempfung nachgestellt), wenn der Fehler im Band dadurch um hoechstens 0,2 dB steigt
  4 Steg         Normalmoden: R(Schlag, Aufnehmer) = phi(x_Schlag) phi(x_Aufnehmer), phi(x) = a + b x + c x^2 laengs
                 der Stegeinlage (starrer Stab mit Biegung), Gauss-Newton; daraus phi an jeder Saite
  5 Abstrahlung  je Hoerort (Mikrofon bass = left, treble = right, 10 cm ueber der Decke): bis zur Grenze ein komplexes
                 Gewicht je Mode aus dem Fit an P/F fuer alle drei Schlagorte; darueber ein Betrag je Terzband mit
                 gezogenem Vorzeichen (Samen 1), so gestellt, dass der Terzpegel im Mittel ueber die Schlagorte stimmt
                 (KQ24 a). Die Grenze (500 Hz oder 1 kHz) nach dem kleineren Fehler ueber beide Hoerorte.
  6 Statistik    der statistische Teil 5-20 kHz (KQ26 a), Groessen an der Messung: das Daempfungsgesetz
                 alpha(f) = a f^2 + b f + c aus dem Abklingen der Mikrofone je Terzband 2,5-10 kHz (Schroeder-Kurve,
                 -5 ... -25 dB); Dichte und Streuung am Steg aus den Polen 2,5-5 kHz wie `schaetze` in
                 `InstrumentBody/Statistik.hpp`; der Betrag der Abstrahlung je Hoerort so gestellt, dass der Terzpegel
                 6,3 / 8 / 10 kHz im Mittel stimmt - gerechnet mit den Moden, die `ziehe` daraus zieht (hier
                 nachgebaut: SplitMix64, Rayleigh, Box-Muller; Samen 1). Die Messung traegt bis rund 10 kHz
                 (Stoerabstand an den Mikrofonen 11-16 dB); darueber ist das Gesetz fortgesetzt, nicht geprueft.
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

FIT_FASSUNG = 2
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
PAARE = [("bass", "bass"), ("bass", "treble"), ("center", "bass"), ("center", "treble"), ("treble", "bass"),
         ("treble", "treble")]
SAITENABSTAND = 0.0116
X_SCHLAG = {"bass": -2 * SAITENABSTAND, "center": 0.0, "treble": 2 * SAITENABSTAND}
X_AUFN = {"bass": -0.036, "treble": 0.036}
X_SAITEN = [(k - 2.5) * SAITENABSTAND for k in range(6)]     # Saite 6 (E2) ... 1 (E4), Bass negativ
TASTEN = [40, 45, 50, 55, 59, 64]
SAITEN_NAMEN = ["6 (E2)", "5 (A2)", "4 (D3)", "3 (G3)", "2 (B3)", "1 (E4)"]
MIKROFONE = (("left", "mbass"), ("right", "mtreble"))
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
    """Wie `lumi::koerper::ziehe`: f, sigma, Formwerte (Moden x Saiten), Gewichte (Moden x Hoerorte)."""
    z = Zufall(st["seed"])
    a, b, c = st["damping"]
    f, fs_, ss_, steg, G = st["fromHz"], [], [], [], []
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
    return np.array(fs_), np.array(ss_), np.array(steg), np.array(G)


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


def statistik(daten, hz, H, f, s, steg, G):
    mitten = [1000 * 2 ** (k / 3) for k in range(4, 11)]                 # 2,5 ... 10 kHz
    alpha = []
    for m in mitten:
        werte = []
        for x in daten.values():
            ip = int(np.argmax(np.abs(x[:, 0])))
            werte += [abklingen(x[:, kanal], ip, m) for kanal in (4, 5)]
        alpha.append(float(np.median(werte)))
    fm, al = np.array(mitten), np.array(alpha)
    gesetz = nnls(np.stack([fm ** 2, fm, np.ones(len(fm))], axis=1) / al[:, None], np.ones(len(fm)))[0]

    # Dichte und Streuung am Steg wie `schaetze`: zehn Stuecke, eine Gerade durch die Mitten
    von, bis = STAT_BAND
    band = (f >= von) & (f < bis)
    breite = (bis - von) / 10
    x = von + (np.arange(10) + 0.5) * breite
    y = np.array([np.sum((f >= von + i * breite) & (f < von + (i + 1) * breite)) for i in range(10)]) / breite
    steigung, achse = np.polyfit(x, y, 1)
    st = {"fromHz": BIS, "toHz": STAT_BIS,
          "density": float(achse + steigung * BIS if steigung > 0 else y.mean()), "rise": float(max(0.0, steigung)),
          "bridgeSpread": [float(v) for v in np.sqrt(np.mean(steg[band] ** 2, axis=0))],
          "radiation": [float(v) for v in np.sqrt(np.mean(np.abs(G[band]) ** 2, axis=0))],
          "seed": STAT_SAMEN, "bandFromHz": von, "bandToHz": bis, "bandModes": int(band.sum()),
          "damping": [float(v) for v in gesetz]}

    # Betrag der Abstrahlung: der Terzpegel 6,3 / 8 / 10 kHz im Mittel wie gemessen
    ziel = mitten[4:]
    sel = (hz >= ziel[0] / 2 ** (1 / 6)) & (hz < ziel[-1] * 2 ** (1 / 6))
    w = 2 * np.pi * hz[sel]

    def terz(p):
        return np.array([10 * np.log10(np.mean(p[(hz[sel] >= m / 2 ** (1 / 6)) & (hz[sel] < m * 2 ** (1 / 6))])) for m in ziel])

    mess = [terz(np.mean([np.abs(H[(o, mik)][sel]) ** 2 for o in ORTE], axis=0)) for _, mik in MIKROFONE]

    def modell():
        fz, sz, stegz, gz = ziehe(st)
        fa, sa = np.concatenate([f, fz]), np.concatenate([s, sz])
        B = basis(hz[sel], fa, sa) * (1j * w[:, None])
        phi, g = np.concatenate([steg, stegz]), np.concatenate([G, gz])
        return [terz(np.mean([np.abs(B @ (phi[:, k] * g[:, h])) ** 2 for k in range(len(TASTEN))], axis=0))
                for h in range(len(MIKROFONE))], len(fz), fz

    for _ in range(8):
        syn, _, _ = modell()
        for h in range(len(MIKROFONE)):
            st["radiation"][h] *= float(10 ** (np.mean(mess[h] - syn[h]) / 20))
    syn, zahl, fz = modell()
    pruefung = {"decayBandsHz": [round(m) for m in mitten], "decayAlphaMeasured": [round(a, 1) for a in alpha],
                "decayAlphaLaw": [round(float(gesetz[0] * m * m + gesetz[1] * m + gesetz[2]), 1) for m in mitten],
                "levelBandsHz": [round(m) for m in ziel],
                "levelErrorDb": {name: [round(float(v), 2) for v in syn[h] - mess[h]] for h, (name, _) in enumerate(MIKROFONE)},
                "drawnModes": zahl, "drawnFirstHz": [float(v) for v in fz[:3]], "drawnLastHz": float(fz[-1]),
                "note": "level: third-octave level of pressure per force, model (mean over the six strings) minus "
                        "measured (mean over the three strikes); alpha in 1/s, energy ~ exp(-alpha t); drawnFirstHz "
                        "and drawnLastHz are the modes `ziehe` of InstrumentBody draws from the statistical part"}
    return st, pruefung


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
                "bridge": "normal modes: residue(strike, sensor) = phi(x_strike) phi(x_sensor), phi(x) = a + b x + c x^2; "
                          "Gauss-Newton on the six mobilities, error weighted by 1/|Y|",
                "radiation": "per listening point one complex weight per mode up to the split frequency, fitted to the "
                             "pressure per force of all three strikes; above it one magnitude per third-octave band "
                             "with a drawn sign (seed 1), set so that the band level matches on average over the strikes",
                "statistical": f"statistical part {BIS:.0f}-{STAT_BIS:.0f} Hz (KQ26 a), quantities from the measurement: "
                               "damping law alpha(f) = a f^2 + b f + c from the decay at the microphones per "
                               "third-octave band 2.5-10 kHz (Schroeder curve, -5 to -25 dB); density and bridge "
                               "spread from the poles 2.5-5 kHz as for the bodies from a plan; radiation magnitude "
                               "per listening point set so that the third-octave level at 6.3 / 8 / 10 kHz matches "
                               "on average. The measurement carries up to about 10 kHz (signal to noise at the "
                               "microphones 11-16 dB at 10 kHz, below 10 dB above); above that the law is continued "
                               "and not checked. Only the product of bridge value and radiation weight is determined "
                               "by the measurement there, not their split."},
        "bridgePoints": [{"key": t, "x": round(x, 5),
                          "source": f"open string {n}: phi at its place on the saddle from the fitted shape; the key is "
                                    "the open string (fretted notes use the played string)"}
                         for t, x, n in zip(TASTEN, X_SAITEN, SAITEN_NAMEN)],
        "listeningPoints": [{"name": "left", "source": "microphone 'bass': 10 cm above the top plate, 10 cm from the "
                                                       "bridge centre towards the bass side (Mores 2021, Method 2c) - near field"},
                            {"name": "right", "source": "microphone 'treble': 10 cm above the top plate, 10 cm from the "
                                                        "bridge centre towards the treble side - near field"}],
        "assumptions": ["scale of the guitar 662 mm; the strings after Woodhouse et al. 2012 are for 650 mm",
                        "only the motion normal to the top plate is measured; the second direction at the bridge is an assumption elsewhere",
                        "the listening points lie in the near field (10 cm); the other bodies listen at about 1 m",
                        "left = bass side, right = treble side: assigned"],
        "check": pruefung,
    }


def koerper(f, s, a, b, c, G, st, fit_datei, abdruck, sekunden):
    moden = []
    for j in range(len(f)):
        moden.append({"f": float(f[j]), "eta": float(2 * s[j] / (2 * np.pi * f[j])), "airShare": 0.0, "materialShares": [],
                      "bridge": [float(a[j] + b[j] * x + c[j] * x * x) for x in X_SAITEN],
                      "radiation": [[float(g[j].real), float(g[j].imag)] for g in G]})
    return {
        "format": "lumiviz-instrument-body",
        "version": 4,
        "title": "Classical guitar",
        "description": "Measured body: modal fit to guitar g34 of the Mores archive (Manuel Contreras, Madrid 1971, "
                       "cedar / Rio palisander, scale 662 mm). Six bridge points, one per open string; two listening "
                       "points 10 cm above the top plate (near field); above 5 kHz a statistical part fitted to the "
                       "measurement. See classical_guitar.lvinstbodyfit.",
        "units": "f in Hz; eta loss factor; bridge: mode shape at the bridge point, 1/sqrt(kg) (modal mass 1/value^2); "
                 "radiation: complex pressure per modal acceleration at the listening point, sqrt(kg)/m^2 (fitted to "
                 "the measured pressure per force)",
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
                        "bridgeSpread": st["bridgeSpread"], "radiation": st["radiation"], "seed": str(st["seed"]),
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
    a, b, c, fits_steg = stegfit(hz, mob, f, s, sel, R, rest)
    grenze, G, strahl = abstrahlung(hz, H, f, s, a, b, c, sel)
    steg = np.array([a + b * x + c * x * x for x in X_SAITEN]).T
    st, st_pruefung = statistik(daten, hz, H, f, s, steg, np.array(G).T)
    bereiche = (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000))
    m6 = (f > 85) & (f < 240)
    phi6 = a + b * X_SAITEN[0] + c * X_SAITEN[0] ** 2
    pruefung = {
        "modes": int(len(f)), "modesAbove1kHz": int((f > 1000).sum()),
        "bridgeErrorDb": {"free": {n: round(fehler(hz, sel, mob, fits_frei, lo, hi), 2) for n, lo, hi in bereiche},
                          "normalModes": {n: round(fehler(hz, sel, mob, fits_steg, lo, hi), 2) for n, lo, hi in bereiche},
                          "note": "rms of 20 lg |fit / measured| over the six bridge mobilities"},
        "pairs": paare,
        "effectiveMassString6Kg": {"value": round(float(1 / np.sum(phi6[m6] ** 2)), 4), "band": "85-240 Hz",
                                   "reference": "Wright 1996 Table 5.4: 0.062 kg (guitar BR2, sound hole sealed)"},
        "radiation": {"splitHz": grenze, "thirdOctaveErrorDb": strahl,
                      "note": "rms error of the third-octave level over bands and the three strikes, per split frequency"},
        "statistical": st_pruefung,
    }
    ziel.mkdir(parents=True, exist_ok=True)
    fit_datei = ziel / "classical_guitar.lvinstbodyfit"
    inhalt = (json.dumps(beschreibung(pruefung), indent=1) + "\n").encode("utf-8")
    fit_datei.write_bytes(inhalt)
    k = koerper(f, s, a, b, c, G, st, fit_datei.name, abdruck_von(inhalt), time.time() - start)
    (ziel / "classical_guitar.lvinstbody").write_bytes((json.dumps(k, indent=4) + "\n").encode("utf-8"))
    print(json.dumps(pruefung, indent=1))
    print(f"{fit_datei} und classical_guitar.lvinstbody: {len(f)} Moden bis {BIS:.0f} Hz, Grenze der Abstrahlung "
          f"{grenze:.0f} Hz, statistisch bis {STAT_BIS:.0f} Hz ({st_pruefung['drawnModes']} gezogene Moden), "
          f"{time.time() - start:.0f} s")
