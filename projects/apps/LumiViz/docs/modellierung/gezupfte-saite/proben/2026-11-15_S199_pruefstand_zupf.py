"""S199, 34k-6 — Pruefstand ohne Kern: eine gezupfte Saite am Steg eines Koerpers, im Frequenzbereich gerechnet.

Aufruf (vom Projekt-Root): python -B <dieses Skript> gitarre [--je-taste] [--steg-neu] [--winkel <Grad>]
                           python -B <dieses Skript> gitarre-mit [--je-taste]
                           python -B <dieses Skript> laute | oud [--je-taste]
                           python -B <dieses Skript> harfe | cembalo [--je-taste]

harfe, cembalo: eine Saite je Taste, ohne Mitschwingen. Harfe: 46 Saiten nach Bell 1988 Tab. 2.1 / 2.2 (leer in Ces-Dur),
die Taste ueber die Pedale (weisse Taste = ihre Saite um einen Halbton verkuerzt, schwarze = die Saite darunter um zwei,
KQ22 a); Zupf nach Chadefaux u. a. 2012 (0,36 der Laenge vom Boden, 4 mm, 2 m/s beim Abloesen, 25 Grad). Cembalo: ein
8-Fuss-Register, Mensur und Zupfpunkt des Taskin 1769 an elf Tasten (dazwischen logarithmisch), Draht nach Di Veroli;
der Kiel loest bei einer festen Kraft aus (CEMBALO_KRAFT; mit --auslenkung stattdessen die Auslenkung nach Perng 2011
Tab. 2), 20 Grad.

laute, oud: Saiten in Choeren. Die Saiten eines Chors haengen am selben Stegpunkt und werden zusammen gezupft (im
Einklang um +-2 bzw. +-1,5 Cent gegeneinander verstimmt wie die Vorgabe des Kerns, oder in der Oktave). Monofile Saiten
nach Woodhouse & Lynch-Aird 2019 (Gl. 4, 11-17, Tab. I; am Seitenbild gegengeprueft): E aus der Spannung, Verlust =
Luft (Stokes, mit dem Zuschlag d + 0,2) + lambda * E''/E. Je Taste: Lautheit mit allen Moden und mit den Moden der
Koerperdatei allein in der Schleife (GZ5 a).

gitarre-mit: Mitschwingen je Note (GZ3 a) - die gespielte Saite und die uebrigen fuenf leer an einem Steg, jede an ihrem
Stegpunkt, zwei Polarisationen (30 Grad). Der Koerper koppelt die Stegpunkte: Y[(i,a),(j,b)] = Summe phi_ni phi_nj
d_na d_nb c_n(w). Mit u = Y F, F_i = Z0_i (2 v+_i - u_i) und v+_i = g_i - E_i (u_i - v+_i):
M = (I + Y Z)^-1 2 Y Z, (I + E (M - I)) v+ = g, F = Z (2 I - M) v+, p = Summe H_i F_i. Dazu der Anteil, den die nicht
gespielten Saiten abstrahlen (p ohne den Summanden der gespielten Saite).

--winkel: zwei Polarisationen. Die Saite wird unter diesem Winkel gegen die Senkrechte zur Decke losgelassen; jede
Mode des Koerpers bewegt den Steg in einer Richtung theta_n gegen die Senkrechte (Bank & Karjalainen 2010: Y = Summe
H_n d_n d_n^T, wie im Kern). Der Koerper gibt nur die Senkrechte; tan(theta_n) ist eine ANNAHME, abgelesen an Scherrer
2013 Abb. A.11 (andere Gitarre): 0,03 bis 200 Hz (Yyy 30 dB unter den tiefen Spitzen von Yzz), steigend auf 0,4 ab
rund 1,5 kHz (Yyy 5-10 dB unter Yzz), Vorzeichen gezogen (Samen 7).

--steg-neu: Formwert und Abstrahlgewicht des Gitarrenkoerpers ab 2 kHz nach `teile_neu` geteilt.

Das Modell ist linear, also laesst sich der Ton einer Note geschlossen je Frequenz rechnen — ohne Zeitschritt, ohne
Interpolation, ohne Stimmen. Der Pruefstand ist der Bezug, an dem der Kern (Rechenfassung 2, 34k-7) gemessen wird.

  Saite     steif, mit Daempfung nach Valette (Woodhouse u. a. 2012 Gl. 2-4): Wellenzahl k(w) aus
            w^2 m = T k^2 + EI k^4, Verlustfaktor eta = [T (eta_F + eta_A / w) + EI eta_B k^2] / [T + EI k^2];
            gegriffen: Laenge um den Bund verkuerzt. Sattel bzw. Bund starr.
  Zupf      Auslenkung d am Ort x_p (vom Steg), aus der Ruhe losgelassen = Kraftstufe F0 = T d L / (x_p (L - x_p)),
            die bei t = 0 wegfaellt; Breite des Fingers als Gauss-Tiefpass exp(-(w h / c)^2) (Woodhouse 2012, h = 7,5 mm).
  Steg      Punktadmittanz des Koerpers am Stegpunkt der Saite, senkrecht zur Decke:
            Y(w) = sum_n phi_n^2 i w / (w_n^2 - w^2 + i eta_n w_n w); die zweite Richtung starr (Annahme).
            Kraft am Steg: F_b = f (e^{-i k x_p} - e^{-i k (2L - x_p)}) / ((1 + Z0 Y) (1 + R_b e^{-2 i k L})),
            R_b = (Z0 Y - 1) / (Z0 Y + 1), Z0 = T k / w.
  Ausgang   Schalldruck an den Hoerorten: p = F_b sum_n phi_n G_n (-w^2) / (w_n^2 - w^2 + i eta_n w_n w) (KB1 a).
  Zeit      auf der Geraden w - i s0 gerechnet (Exponentialfenster), zurueck mit exp(s0 t): kein Umlauf langer Toene.
  Messung   Momentary-Maximum nach BS.1770 (400 ms, K-Bewertung), beide Hoerorte als Kanaele — die Groesse von LP3 a.

Schreibt nichts.
"""
import json
import math
import pathlib
import sys

import numpy as np
import scipy.signal as sig

HIER = pathlib.Path(__file__).resolve()
ROOT = HIER.parents[7]
KOERPER = ROOT / "asset/loop/instrument_bodies"
FS = 48000
DAUER = 4.0
S0 = 2.0
VORLAUF = 0.02


# ---------------------------------------------------------------- Koerper
class Zufall:
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


def lade_koerper(name):
    """Alle Moden wie `alleModen` in loop/Boden.cpp: die der Datei und die aus dem statistischen Teil gezogenen."""
    k = json.loads((KOERPER / f"{name}.lvinstbody").read_text(encoding="utf-8"))
    f = [m["f"] for m in k["modes"]]
    eta = [m["eta"] for m in k["modes"]]
    steg = [m["bridge"] for m in k["modes"]]
    G = [[complex(*g) for g in m["radiation"]] for m in k["modes"]]
    st = k["statistical"]
    if st["density"] > 0:
        z = Zufall(int(st["seed"]))
        d = st["damping"]
        verlust = [w["lossFactor"] for w in k["materials"]]
        x = st["fromHz"]
        while True:
            x += z.rayleigh() / (st["density"] + st["rise"] * (x - st["fromHz"]))
            if x >= st["toHz"]:
                break
            e = (d["a"] * x * x + d["b"] * x + d["c"]) / (2 * math.pi * x)
            e += sum(a * (v - e) for a, v in zip(st["materialShares"], verlust) if v > 0)
            f.append(x)
            eta.append(e)
            steg.append([s * z.normal() for s in st["bridgeSpread"]])
            G.append([b * complex(math.cos(p), math.sin(p)) for b in st["radiation"]
                      for p in [2.0 * math.pi * z.gleich()]])
    return {"tasten": k["bridgePoints"], "orte": k["listeningPoints"], "f": np.array(f), "eta": np.array(eta),
            "steg": np.array(steg), "G": np.array(G)}


def koerper_kurven(kp, punkt, w):
    """Y (Punktadmittanz, m/s/N) und H (Druck je Kraft, Pa/N je Hoerort) am Stegpunkt auf den (komplexen) w."""
    wn = 2 * np.pi * kp["f"]
    Y = np.zeros(len(w), dtype=complex)
    H = np.zeros((len(w), kp["G"].shape[1]), dtype=complex)
    phi = kp["steg"][:, punkt]
    for a in range(0, len(w), 4096):
        ws = w[a:a + 4096, None]
        nenner = 1.0 / (wn[None, :] ** 2 - ws ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * ws)
        Y[a:a + 4096] = (1j * ws * nenner) @ (phi ** 2)
        H[a:a + 4096] = (-ws ** 2 * nenner) @ (phi[:, None] * kp["G"])
    return Y, H


# Gemessene Uebertragung Schlagort -> Aufnehmer an g34 (Mittel der sechs Paare, Mobilitaet in dB re 1 m/s/N je
# Terzband; 2026-11-15_S199_gitarre_admittanz_an_den_saiten.py mit der Messdatei). Ueber 10 kHz traegt die Messung nicht:
# dort der Wert von 10 kHz.
GITARRE_UEBERTRAGUNG = {2016: -45.3, 2540: -44.3, 3200: -47.2, 4032: -47.4, 5080: -49.8, 6400: -51.8, 8063: -51.2, 10159: -50.1}


def teile_neu(kp, ziel=None, zuschlag=3.0):
    """Formwert am Steg und Abstrahlgewicht je Terzband neu geteilt (das Produkt, also p / F, bleibt): die
    Punktadmittanz im Mittel der Stegpunkte = gemessene Uebertragung + `zuschlag` dB (Punkt gegen Uebertragung in einem
    dichten Modenfeld: Faktor 2). Die Messung bestimmt ueber rund 2,5 kHz nur das Produkt (Bericht S199 Par. 11)."""
    ziel = ziel or GITARRE_UEBERTRAGUNG
    mitten = sorted(ziel) + [m for m in (12800, 16127, 20319)]
    werte = [ziel[m] for m in sorted(ziel)] + [ziel[max(ziel)]] * 3
    hz = np.arange(1500.0, 22000.0, 2.0)
    for _ in range(6):
        pegel = np.mean([np.abs(koerper_kurven(kp, s, 2 * np.pi * hz + 0j)[0]) ** 2 for s in range(kp["steg"].shape[1])], axis=0)
        for m, z in zip(mitten, werte):
            band = (hz >= m / 2 ** (1 / 6)) & (hz < m * 2 ** (1 / 6))
            lam = 10 ** ((10 * np.log10(np.mean(pegel[band])) - (z + zuschlag)) / 20)
            mo = (kp["f"] >= m / 2 ** (1 / 6)) & (kp["f"] < m * 2 ** (1 / 6))
            kp["steg"][mo] /= lam
            kp["G"][mo] *= lam
    return kp


def richtung_tan(f, samen=7):
    """tan(theta_n) je Mode: Annahme nach Scherrer 2013 Abb. A.11 (siehe Kopf)."""
    z = Zufall(samen)
    betrag = np.clip(0.03 * (f / 200.0) ** 1.27, 0.03, 0.4)
    return betrag * np.array([1.0 if z.gleich() < 0.5 else -1.0 for _ in f])


def koerper_kurven2(kp, punkt, w, tan):
    """Yzz, Yzy, Yyy und der Druck je Kraft senkrecht (Hz) und parallel (Hy) am Stegpunkt."""
    wn = 2 * np.pi * kp["f"]
    Y = np.zeros((len(w), 3), dtype=complex)
    Hz = np.zeros((len(w), kp["G"].shape[1]), dtype=complex)
    Hy = np.zeros_like(Hz)
    phi = kp["steg"][:, punkt]
    for a in range(0, len(w), 4096):
        ws = w[a:a + 4096, None]
        nenner = 1.0 / (wn[None, :] ** 2 - ws ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * ws)
        Y[a:a + 4096] = (1j * ws * nenner) @ np.stack([phi ** 2, tan * phi ** 2, tan ** 2 * phi ** 2], axis=1)
        Hz[a:a + 4096] = (-ws ** 2 * nenner) @ (phi[:, None] * kp["G"])
        Hy[a:a + 4096] = (-ws ** 2 * nenner) @ ((tan * phi)[:, None] * kp["G"])
    return Y, Hz, Hy


def staerke_faktor(w, stufe):
    """GZ6 a: die Stufe (dB, <= 0) setzt die Auslenkung (d bei Stufe 0) und kippt das Spektrum des Zupfs um 1 kHz —
    0,16 dB je Oktave je dB Stufe (angepasst an Iowa MIS: ff - pp 3,0 dB je Oktave bei 18,7 dB)."""
    if stufe == 0.0:
        return 1.0
    f = np.maximum(np.abs(w.real) / (2 * np.pi), 20.0)
    return 10 ** (stufe * (1 + 0.16 * np.log2(f / 1000.0)) / 20)


def ton2(saite, laenge, x_p, d, winkel_grad, Y, Hz, Hy, w, breite=0.0075, stufe=0.0, y_bis=None):
    """Zwei Polarisationen: (I + R_b E) v+ = g (cos a, sin a), R_b = (I + Z0 Y)^-1 (Z0 Y - I), F = 2 Z0 (I + Z0 Y)^-1 v+."""
    T, m, EI = saite["T"], saite["m"], saite["EI"]
    k2 = (-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI)
    k = np.sqrt(k2)
    eta = saiten_eta(saite, w, k2)
    k = k * (1 - 0.5j * eta)
    z0 = T * k / w
    f0 = T * d * laenge / (x_p * (laenge - x_p))
    f = -f0 / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2) * staerke_faktor(w, stufe)
    g = f / (2 * z0) * (np.exp(-1j * k * x_p) - np.exp(-1j * k * (2 * laenge - x_p)))
    E = np.exp(-2j * k * laenge)
    n = len(w)
    A = np.zeros((n, 2, 2), dtype=complex)          # I + Z0 Y
    A[:, 0, 0] = 1 + z0 * Y[:, 0]
    A[:, 0, 1] = A[:, 1, 0] = z0 * Y[:, 1]
    A[:, 1, 1] = 1 + z0 * Y[:, 2]
    Ainv = np.linalg.inv(A)
    eins = np.broadcast_to(np.eye(2), (n, 2, 2))
    Rb = Ainv @ (A - 2 * eins)
    a = np.radians(winkel_grad)
    rechts = g[:, None] * np.array([np.cos(a), np.sin(a)])[None, :]
    vplus = np.linalg.solve(eins + Rb * E[:, None, None], rechts[:, :, None])[:, :, 0]
    F = 2 * z0[:, None] * (Ainv @ vplus[:, :, None])[:, :, 0]
    return F[:, 0:1] * Hz + F[:, 1:2] * Hy


def saiten_eta(saite, w, k2):
    """Verlustfaktor der Saite. Mit `d` und `rho` (monofil): Woodhouse & Lynch-Aird 2019 Gl. 11, 12, 14, 15 mit dem
    Zuschlag (d + 0,2) auf die Luft; sonst Valette mit eta_F, eta_A (Woodhouse u. a. 2012 Gl. 4)."""
    T, EI = saite["T"], saite["EI"]
    biegung = EI * saite["etaB"] * k2 / (T + EI * k2)
    if "d" in saite:
        f = np.maximum(np.abs(w.real) / (2 * np.pi), 20.0)
        M = saite["d"] / 4 * np.sqrt(2 * np.pi * f / 1.5e-5)
        return (saite["d"] * 1000 + 0.2) * 1.2 / saite["rho"] * (2 * np.sqrt(2) * M + 1) / M ** 2 + biegung
    return T * (saite["etaF"] + saite["etaA"] / w) / (T + EI * k2) + biegung


# E0 GPa, Anstieg je GPa Spannung, E'' GPa, rho (Woodhouse & Lynch-Aird 2019 Tab. I; D = Darm)
WERKSTOFF = {"N": (4.5, 39.0, 0.25, 1070.0), "F": (3.2, 41.0, 0.18, 1800.0), "D": (6.0, 0.0, 0.24, 1320.0)}


def monofil(d_mm, hz, laenge, werkstoff):
    """Eine monofile Saite aus Durchmesser, Ton und Laenge (Woodhouse & Lynch-Aird 2019 Tab. I, Gl. 16, 17)."""
    e0, anstieg, e2, rho = WERKSTOFF[werkstoff]
    d = d_mm / 1000
    flaeche = np.pi * d * d / 4
    m = rho * flaeche
    T = 4 * laenge ** 2 * hz ** 2 * m
    E = (e0 + anstieg * T / flaeche / 1e9) * 1e9
    return {"T": T, "m": m, "EI": E * np.pi * d ** 4 / 64, "etaB": e2 * 1e9 / E, "d": d, "rho": rho, "hz": hz}


def umsponnen(zug_n, hz, laenge):
    """Eine umsponnene Saite aus Zug und Ton; Biegesteife und Verluste UEBERTRAGEN von den umsponnenen Gitarrensaiten
    (Woodhouse u. a. 2012 Tab. I, Saiten 4-6: EI 40-57e-6 N m^2, eta_F 2-7e-5, eta_B 2e-2, eta_A 0,9-1,2)."""
    return {"T": zug_n, "m": zug_n / (4 * laenge ** 2 * hz ** 2), "EI": 50e-6, "etaF": 5e-5, "etaB": 2.0e-2, "etaA": 1.1, "hz": hz}


def koerper_matrix(kp, w, tan, punkte=None):
    """Y (Frequenz x 2S x 2S) zwischen den Stegpunkten der Saiten und beiden Richtungen, H (Frequenz x 2S x Hoerorte).
    `punkte`: je Saite ihr Stegpunkt (mehrere Saiten duerfen am selben haengen); ohne Angabe je Stegpunkt eine Saite."""
    steg = kp["steg"] if punkte is None else kp["steg"][:, punkte]
    S = steg.shape[1]
    wn = 2 * np.pi * kp["f"]
    phi = np.empty((len(wn), 2 * S))
    phi[:, 0::2] = steg
    phi[:, 1::2] = steg * tan[:, None]
    Y = np.empty((len(w), 2 * S, 2 * S), dtype=complex)
    H = np.empty((len(w), 2 * S, kp["G"].shape[1]), dtype=complex)
    for a in range(0, len(w), 2048):
        ws = w[a:a + 2048, None]
        nenner = 1.0 / (wn[None, :] ** 2 - ws ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * ws)
        Y[a:a + 2048] = np.einsum("fn,na,nb->fab", 1j * ws * nenner, phi, phi, optimize=True)
        H[a:a + 2048] = np.einsum("fn,na,no->fao", -ws ** 2 * nenner, phi, kp["G"], optimize=True)
    return Y, H


def saite_wellen(saite, w):
    T, m, EI = saite["T"], saite["m"], saite["EI"]
    k2 = (-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI)
    k = np.sqrt(k2)
    eta = saiten_eta(saite, w, k2)
    k = k * (1 - 0.5j * eta)
    return k, T * k / w


def ton_mit(saiten, laengen, gespielt, x_p, d, winkel_grad, Y, H, w, breite=0.0075, tempo=0.0):
    """Alle Saiten an einem Steg; `gespielt` wird gezupft. Gibt (p, p der nicht gespielten Saiten)."""
    S, n = len(saiten), len(w)
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        k, z0 = saite_wellen(sa, w)
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = np.exp(-2j * k * L)
        if i == gespielt or (isinstance(gespielt, (list, tuple)) and i in gespielt):
            T, m = sa["T"], sa["m"]
            f = -(T * d * L / (x_p * (L - x_p))) / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2)
            if tempo:      # Geschwindigkeit beim Abloesen, zur Ruhelage hin: dieselbe Form, ueber die Zeit integriert
                f = f * (1 - (tempo / d) / (1j * w))
            gi = f / (2 * z0) * (np.exp(-1j * k * x_p) - np.exp(-1j * k * (2 * L - x_p)))
            a = np.radians(winkel_grad)
            g[:, 2 * i], g[:, 2 * i + 1] = gi * np.cos(a), gi * np.sin(a)
    p = np.empty((n, H.shape[2]), dtype=complex)
    rest = np.empty_like(p)
    eins = np.eye(2 * S)
    for a in range(0, n, 8192):
        b = slice(a, a + 8192)
        YZ = Y[b] * z[b][:, None, :]
        M = np.linalg.solve(eins[None] + YZ, 2 * YZ)
        vplus = np.linalg.solve(eins[None] + E[b][:, :, None] * (M - eins[None]), g[b][:, :, None])[:, :, 0]
        F = z[b] * (2 * vplus - (M @ vplus[:, :, None])[:, :, 0])
        p[b] = np.einsum("fa,fao->fo", F, H[b])
        for i in (gespielt if isinstance(gespielt, (list, tuple)) else [gespielt]):
            F[:, 2 * i:2 * i + 2] = 0
        rest[b] = np.einsum("fa,fao->fo", F, H[b])
    return p, rest


# ---------------------------------------------------------------- Saite und Ton
def ton(saite, laenge, x_p, d, Y, H, w, breite=0.0075):
    T, m, EI = saite["T"], saite["m"], saite["EI"]
    k2 = (-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI)
    k = np.sqrt(k2)
    eta = saiten_eta(saite, w, k2)
    k = k * (1 - 0.5j * eta)
    z0 = T * k / w
    f0 = T * d * laenge / (x_p * (laenge - x_p))
    c = np.sqrt(T / m)
    f = -f0 / (1j * w) * np.exp(-(w * breite / c) ** 2)
    rb = (z0 * Y - 1) / (z0 * Y + 1)
    fb = f * (np.exp(-1j * k * x_p) - np.exp(-1j * k * (2 * laenge - x_p))) / ((1 + z0 * Y) * (1 + rb * np.exp(-2j * k * laenge)))
    return fb[:, None] * H


def zeit(spektrum):
    """Der Ton beginnt bei VORLAUF: was vor dem Loslassen liegt (Vorschwinger der Bandgrenze und des Gauss-Tiefpasses),
    faellt in den Vorlauf statt ans Ende des Fensters, wo exp(s0 t) es um e^8 anhoebe. Das letzte Stueck faellt weg."""
    n = int(DAUER * FS)
    x = np.fft.irfft(spektrum * np.exp(-1j * frequenzen() * VORLAUF)[:, None], n, axis=0) * FS
    x = x * np.exp(S0 * np.arange(n) / FS)[:, None]     # die Verschiebung auf der Geraden w - i s0 traegt exp(-s0 VORLAUF) schon
    return x[:n - int(0.25 * FS)]


def k_bewertung():
    """BS.1770: Hochregal und Hochpass fuer 48 kHz."""
    b1 = [1.53512485958697, -2.69169618940638, 1.19839281085285]
    a1 = [1.0, -1.69065929318241, 0.73248077421585]
    b2 = [1.0, -2.0, 1.0]
    a2 = [1.0, -1.99004745483398, 0.99007225036621]
    return np.array([b1 + a1, b2 + a2])


def momentary_max(x):
    y = sig.sosfilt(k_bewertung(), x, axis=0)
    p = np.sum(y ** 2, axis=1)
    c = np.concatenate([[0.0], np.cumsum(p)])
    n = int(0.4 * FS)
    fen = (c[n::int(0.1 * FS)] - c[:-n:int(0.1 * FS)]) / n
    return -0.691 + 10 * np.log10(fen.max())


def abfall(x):
    """Wie die Messung der Aufnahmen in S195: Pegel in 20-ms-Bloecken; Abfall frueh (-5 ... -25 dB unter dem
    lautesten Block) und spaet (-25 ... -45 dB) in dB/s."""
    n = int(0.02 * FS)
    b = x[:len(x) // n * n].reshape(-1, n, x.shape[1])
    db = 10 * np.log10(np.mean(np.sum(b ** 2, axis=2), axis=1) + 1e-300)
    i0 = int(np.argmax(db))
    db, t = db[i0:] - db[i0], np.arange(len(db) - i0) * 0.02
    aus = []
    for oben, unten in ((-5, -25), (-25, -45)):
        ende = int(np.argmax(db < unten)) if (db < unten).any() else len(db)
        m = db[:ende] <= oben
        aus.append(float(-np.polyfit(t[:ende][m], db[:ende][m], 1)[0]) if m.sum() >= 3 else float("nan"))
    return aus


def frequenzen():
    n = int(DAUER * FS)
    return 2 * np.pi * np.fft.rfftfreq(n, 1 / FS) - 1j * S0


# ---------------------------------------------------------------- Gitarre
# Woodhouse u. a. 2012 Tab. I, Satz (a) Nylon (gegengelesen S195): T in N, m in kg/m, EI in N m^2, eta_F, eta_B, eta_A
GITARRE = [  # Saite 6 ... 1
    {"taste": 40, "T": 71.6, "m": 6.24e-3, "EI": 57e-6, "etaF": 2e-5, "etaB": 2.0e-2, "etaA": 1.2},
    {"taste": 45, "T": 73.9, "m": 3.61e-3, "EI": 40e-6, "etaF": 7e-5, "etaB": 2.5e-2, "etaA": 0.9},
    {"taste": 50, "T": 71.2, "m": 1.95e-3, "EI": 51e-6, "etaF": 5e-5, "etaB": 2.0e-2, "etaA": 1.2},
    {"taste": 55, "T": 58.3, "m": 0.90e-3, "EI": 310e-6, "etaF": 14e-5, "etaB": 2.0e-2, "etaA": 1.7},
    {"taste": 59, "T": 53.4, "m": 0.52e-3, "EI": 160e-6, "etaF": 40e-5, "etaB": 2.0e-2, "etaA": 1.2},
    {"taste": 64, "T": 70.3, "m": 0.38e-3, "EI": 130e-6, "etaF": 40e-5, "etaB": 2.4e-2, "etaA": 1.5},
]
MENSUR = 0.650
ZUPFORT = 0.16      # m vom Steg (Pavlidou 1997 nach Scherrer 2013 Tab. 2.1: Auslenkung 0,8-4 mm bei 16 cm)


def gitarre(je_taste, steg_neu, winkel=None):
    kp = lade_koerper("classical_guitar")
    if steg_neu:
        teile_neu(kp)
    w = frequenzen()
    kurven = {}
    zeilen = []
    for taste in range(40, 84, 1 if je_taste else 3):
        s = max(i for i, sa in enumerate(GITARRE) if sa["taste"] <= taste)
        bund = taste - GITARRE[s]["taste"]
        if bund > 19:
            continue
        punkt = kp["tasten"].index(GITARRE[s]["taste"])
        if s not in kurven:
            kurven[s] = koerper_kurven(kp, punkt, w) if winkel is None else koerper_kurven2(kp, punkt, w, richtung_tan(kp["f"]))
        laenge = MENSUR * 2 ** (-bund / 12)
        if winkel is None:
            x = zeit(ton(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, *kurven[s], w))
            steif = zeit(ton(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, np.zeros(len(w)), kurven[s][1], w))
        else:
            x = zeit(ton2(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, winkel, *kurven[s], w))
            steif = zeit(ton2(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, winkel, np.zeros((len(w), 3)),
                              kurven[s][1], kurven[s][2], w))
        frueh, spaet = abfall(x)
        zeilen.append((taste, 6 - s, bund, momentary_max(x), momentary_max(steif), frueh, spaet, abfall(steif)[0]))
        print(f"Taste {taste:3d}  Saite {6 - s}  Bund {bund:2d}  {zeilen[-1][3]:7.1f} LUFS   starrer Steg {zeilen[-1][4]:7.1f}   "
              f"Abfall frueh {frueh:6.1f} spaet {spaet:6.1f} dB/s   starr frueh {zeilen[-1][7]:6.1f}", flush=True)
    for name, sp in (("Koerper am Steg", 3), ("starrer Steg  ", 4)):
        l = np.array([z[sp] for z in zeilen])
        print(f"{name}: Spanne {l.max() - l.min():.1f} LU, groesster Sprung zwischen Nachbarn {np.abs(np.diff(l)).max():.1f} LU, "
              f"mittlerer Sprung {np.median(np.abs(np.diff(l))):.1f} LU")
    print("Abfall frueh / spaet je Oktave (Median, dB/s); Iowa ff: C2 5,5 / 0,7 - C3 14,8 / 1,4 - C4 11,1 / 1,1 - C5 11,6 / 0,9")
    for von in (36, 48, 60, 72):
        o = [z for z in zeilen if von <= z[0] < von + 12]
        if o:
            print(f"   C{von // 12 - 1}: {np.nanmedian([z[5] for z in o]):5.1f} / {np.nanmedian([z[6] for z in o]):5.1f}   "
                  f"(starrer Steg frueh {np.nanmedian([z[7] for z in o]):5.1f})")


def rms_db(x, von, bis):
    return 10 * np.log10(np.mean(np.sum(x[int(von * FS):int(bis * FS)] ** 2, axis=1)) + 1e-300)


def gitarre_mit(je_taste):
    kp = lade_koerper("classical_guitar")
    w = frequenzen()
    Y, H = koerper_matrix(kp, w, richtung_tan(kp["f"]))
    zeilen = []
    print("Taste Saite Bund | Lautheit ohne / mit (LUFS), Unterschied | Pegel 1-2 s gegen 0-0,4 s: ohne, mit, nur die uebrigen Saiten (dB)")
    for taste in range(40, 84, 1 if je_taste else 3):
        s = max(i for i, sa in enumerate(GITARRE) if sa["taste"] <= taste)
        bund = taste - GITARRE[s]["taste"]
        if bund > 19:
            continue
        laenge = MENSUR * 2 ** (-bund / 12)
        xp = min(ZUPFORT, 0.5 * laenge)
        j = 2 * s
        ohne = zeit(ton_mit([GITARRE[s]], [laenge], 0, xp, 0.002, 30.0, Y[:, j:j + 2, j:j + 2], H[:, j:j + 2], w)[0])
        p, rest = ton_mit(GITARRE, [laenge if i == s else MENSUR for i in range(6)], s, xp, 0.002, 30.0, Y, H, w)
        mit, uebrige = zeit(p), zeit(rest)
        v, e = VORLAUF, VORLAUF + 0.4
        zeilen.append((taste, momentary_max(ohne), momentary_max(mit), rms_db(ohne, 1, 2) - rms_db(ohne, v, e),
                       rms_db(mit, 1, 2) - rms_db(mit, v, e), rms_db(uebrige, 1, 2) - rms_db(mit, v, e)))
        z = zeilen[-1]
        print(f"{taste:3d}  {6 - s}  {bund:2d} | {z[1]:6.1f} / {z[2]:6.1f}  {z[2] - z[1]:+5.1f} | {z[3]:6.1f}  {z[4]:6.1f}  {z[5]:6.1f}", flush=True)
    a = np.array(zeilen)
    for name, sp in (("ohne Mitschwingen", 1), ("mit Mitschwingen ", 2)):
        l = a[:, sp]
        d3 = np.abs(l[3:] - l[:-3]) if je_taste else np.abs(np.diff(l))
        print(f"{name}: Spanne {l.max() - l.min():.1f} LU, groesster Sprung im Terzabstand {d3.max():.1f} LU, mittlerer {np.median(d3):.1f} LU")
    print(f"Lautheit mit - ohne: Median {np.median(a[:, 2] - a[:, 1]):+.1f} LU, von {np.min(a[:, 2] - a[:, 1]):+.1f} bis {np.max(a[:, 2] - a[:, 1]):+.1f} LU")
    print(f"Pegel 1-2 s gegen den Anfang, Median: ohne {np.median(a[:, 3]):.1f} dB, mit {np.median(a[:, 4]):.1f} dB, "
          f"nur die uebrigen Saiten {np.median(a[:, 5]):.1f} dB (von {a[:, 5].min():.1f} bis {a[:, 5].max():.1f})")


# ---------------------------------------------------------------- Laute und Oud
# Laute: Woodhouse & Lynch-Aird 2019 Tab. II (8 Choere in G, 0,625 m; S195 gegengelesen): Taste der leeren Saite,
# je Saite (Durchmesser mm, Werkstoff, Oktave ueber dem Chor?)
LAUTE_MENSUR = 0.625
LAUTE = [(38, [(1.60, "F", 0), (0.925, "N", 12)]), (41, [(1.40, "F", 0), (0.875, "N", 12)]), (43, [(1.25, "F", 0), (0.775, "N", 12)]),
         (48, [(0.91, "F", 0), (0.91, "F", 0)]), (53, [(0.900, "N", 0), (0.900, "N", 0)]), (57, [(0.725, "N", 0), (0.725, "N", 0)]),
         (62, [(0.52, "N", 0), (0.52, "N", 0)]), (67, [(0.475, "N", 0)])]
# Oud: D'Addario EJ95A (Hersteller; Durchmesser in Zoll, Zug in kg), arabische Stimmung C2 F2 A2 D3 G3 C4. Die Mensur
# nennt der Hersteller nicht: 0,60 m ANNAHME (die Tabelle passt zu 0,594 m). Chor C4: Zug auf der Seite "0.0" - aus
# Nylon und Durchmesser gerechnet. F2 0.032 ohne Artikelnummer: umsponnen (als Nylon waere die Masse ein Fuenftel).
OUD_MENSUR = 0.60
OUD = [(36, [("u", 3.659)]), (41, [("u", 2.86), ("u", 2.86)]), (45, [("u", 3.24), ("u", 3.24)]), (50, [("u", 4.034), ("u", 4.034)]),
       (55, [("n", 0.031 * 25.4), ("n", 0.031 * 25.4)]), (60, [("n", 0.026 * 25.4), ("n", 0.026 * 25.4)])]


def hz_von(taste):
    return 440.0 * 2 ** ((taste - 69) / 12)


def choere(name):
    """Je Chor (Taste der leeren Saiten, Liste der Saiten); die Mensur; die Verstimmung im Einklang in Cent."""
    if name == "laute":
        return [(t, [monofil(d, hz_von(t + okt), LAUTE_MENSUR, w) for d, w, okt in saiten]) for t, saiten in LAUTE], LAUTE_MENSUR, 4.0
    aus = []
    for t, saiten in OUD:
        aus.append((t, [umsponnen(wert * 9.80665, hz_von(t), OUD_MENSUR) if art == "u" else monofil(wert, hz_von(t), OUD_MENSUR, "N")
                        for art, wert in saiten]))
    return aus, OUD_MENSUR, 3.0


def chor_instrument(name, je_taste, hoechster_bund=12):
    kp = lade_koerper({"laute": "lute", "oud": "oud"}[name])
    datei = len(json.loads((KOERPER / f"{ {'laute': 'lute', 'oud': 'oud'}[name] }.lvinstbody").read_text(encoding="utf-8"))["modes"])
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    w = frequenzen()
    tan = richtung_tan(kp["f"])
    ch, mensur, cent = choere(name)
    print(f"{name}: {len(ch)} Choere, Mensur {mensur} m; Zug je Saite in N: " +
          " | ".join(" / ".join(f"{s['T']:.1f}" for s in saiten) for _, saiten in ch))
    print("Taste Chor Bund | Lautheit alle Moden / Moden der Datei (LUFS) | Pegel 1-2 s gegen den Anfang: alle / Datei (dB)")
    zeilen, kurven = [], {}
    tasten = range(ch[0][0], ch[-1][0] + hoechster_bund + 1, 1 if je_taste else 3)
    for taste in tasten:
        c = max(i for i, (t, _) in enumerate(ch) if t <= taste)
        bund = taste - ch[c][0]
        saiten = ch[c][1]
        punkt = kp["tasten"].index(ch[c][0])
        if c not in kurven:
            Y, H = koerper_matrix(kp, w, tan, [punkt] * len(saiten))
            Yd, _ = koerper_matrix(nur, w, tan[:datei], [punkt] * len(saiten))
            kurven = {c: (Y, Yd, H)}
        Y, Yd, H = kurven[c]
        laenge = mensur * 2 ** (-bund / 12)
        # im Einklang gegeneinander verstimmt: die Laenge der zweiten Saite um `cent` kuerzer bzw. laenger
        gleich = len(saiten) == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else laenge for i in range(len(saiten))]
        xp = min(ZUPFORT, 0.5 * laenge)
        alle = zeit(ton_mit(saiten, laengen, list(range(len(saiten))), xp, 0.002, 30.0, Y, H, w)[0])
        dat = zeit(ton_mit(saiten, laengen, list(range(len(saiten))), xp, 0.002, 30.0, Yd, H, w)[0])
        v, e = VORLAUF, VORLAUF + 0.4
        zeilen.append((taste, momentary_max(alle), momentary_max(dat), rms_db(alle, 1, 2) - rms_db(alle, v, e),
                       rms_db(dat, 1, 2) - rms_db(dat, v, e)))
        z = zeilen[-1]
        print(f"{taste:3d}  {c + 1}  {bund:2d} | {z[1]:6.1f} / {z[2]:6.1f} | {z[3]:6.1f} / {z[4]:6.1f}", flush=True)
    a = np.array(zeilen)
    l = a[:, 1]
    d3 = np.abs(l[3:] - l[:-3]) if je_taste else np.abs(np.diff(l))
    print(f"Spanne {l.max() - l.min():.1f} LU, groesster Sprung im Terzabstand {d3.max():.1f} LU (bei Taste {int(a[int(np.argmax(d3)), 0])}), "
          f"mittlerer {np.median(d3):.1f} LU")
    print(f"GZ5 a, Moden der Datei ({datei}) gegen alle ({len(kp['f'])}): Lautheit groesster Betrag {np.abs(a[:, 2] - a[:, 1]).max():.2f} LU, "
          f"Pegel 1-2 s groesster Betrag {np.abs(a[:, 4] - a[:, 3]).max():.2f} dB")


# ---------------------------------------------------------------- Harfe und Cembalo
# Bell 1988 Tab. 2.1 (Nylon 00-8, Darm 9-33): Hz, Durchmesser mm, Laenge m; Tab. 2.2 (Metall 34-44): Hz, Kern mm, gesamt mm,
# Laenge m (am Bild gelesen, S198)
HARFE_MONO = [(2960, 0.52, 0.071), (2637, 0.52, 0.082), (2489, 0.56, 0.088), (2218, 0.56, 0.095), (1976, 0.61, 0.105),
              (1864, 0.62, 0.113), (1661, 0.65, 0.123), (1480, 0.69, 0.132), (1318, 0.70, 0.143), (1244, 0.69, 0.154),
              (1109, 0.70, 0.164), (988, 0.73, 0.177), (932, 0.76, 0.192), (830, 0.80, 0.208), (740, 0.85, 0.225),
              (659, 0.90, 0.242), (622, 0.95, 0.265), (554, 1.00, 0.285), (494, 1.06, 0.327), (466, 1.12, 0.354),
              (415, 1.18, 0.385), (370, 1.24, 0.421), (330, 1.30, 0.455), (311, 1.36, 0.498), (277, 1.44, 0.542),
              (247, 1.52, 0.591), (233, 1.60, 0.644), (207, 1.68, 0.699), (185, 1.76, 0.753), (165, 1.84, 0.813),
              (155, 1.92, 0.880), (139, 2.00, 0.945), (123, 2.10, 0.999), (116, 2.20, 1.053), (104, 2.35, 1.103)]
HARFE_METALL = [(92, 0.80, 1.45, 1.150), (82, 0.80, 1.55, 1.180), (77, 0.80, 1.65, 1.222), (69, 0.85, 1.75, 1.257),
                (61, 0.90, 1.92, 1.299), (58, 0.95, 2.05, 1.330), (52, 1.00, 2.15, 1.359), (46, 1.10, 2.30, 1.397),
                (41, 1.10, 2.45, 1.421), (39, 1.25, 2.60, 1.445), (35, 1.30, 2.75, 1.475)]
# Taskin 1769 (Edinburgh 96075): Taste, Laenge mm, Zupfpunkt mm ab Sattel (F3: 151 statt des gedruckten 51, Schreibfehler)
TASKIN = [(29, 1765, 208), (36, 1681, 191), (41, 1443, 179), (48, 1178, 162), (53, 982, 151), (60, 711, 137), (65, 535, 127),
          (72, 357, 115), (77, 268, 107), (84, 186, 96), (89, 148, 91)]
# Besaitung 8 Fuss nach Di Veroli (Tab. 8, Durchmesser Tab. 7): bis Taste, Durchmesser mm, Dichte kg/m3, E GPa. Dichte und E
# sind ANNAHMEN (Rotmessing 8700 / 115, Gelbmessing 8500 / 110, Eisen 7800 / 170 - Malcolm Rose, moderne Werte).
TASKIN_DRAHT = [(31, 0.620, 8700, 115), (33, 0.572, 8700, 115), (36, 0.518, 8700, 115), (38, 0.463, 8700, 115), (40, 0.463, 8500, 110),
                (43, 0.410, 8500, 110), (45, 0.377, 8500, 110), (47, 0.339, 8500, 110), (51, 0.339, 7800, 170), (56, 0.300, 7800, 170),
                (64, 0.269, 7800, 170), (75, 0.244, 7800, 170), (89, 0.213, 7800, 170)]
# Perng 2011 Tab. 2: Zupfverhaeltnis -> Auslenkung beim Abloesen in mm (Modell, v = 1 m/s; Saite 0,5 m, T 92 bzw. 135 N).
# Umgerechnet in die Kraft beim Abloesen, F = T d L / (x (L - x)): 1,6-1,9 N (T 92 N) bzw. 2,4-2,8 N (T 135 N) ueber alle
# Zupfverhaeltnisse - der Kiel loest bei etwa gleicher Kraft aus, nicht bei gleicher Auslenkung. Chadefaux u. a. 2012
# nennen fuer das Plektrum des Cembalos 1,5 N. ANNAHME: 2 N fuer jede Taste.
CEMBALO_KRAFT = 2.0
PERNG = [(1 / 7, 1.2636), (1 / 6, 1.3719), (1 / 5, 1.5285), (1 / 4, 1.6858), (1 / 3, 1.8708), (1 / 2, 2.2236)]


def harfe_taste(taste):
    """(Saite, Laenge, Zupfort, Auslenkung, Winkel, Breite, Tempo) oder None."""
    offen = [int(round(69 + 12 * np.log2(hz / 440.0))) for hz, *_ in HARFE_MONO + HARFE_METALL]
    for versatz in (1, 2, 0):
        if taste - versatz in offen and (versatz or taste == min(offen)):
            i = offen.index(taste - versatz)
            if i < len(HARFE_MONO):
                hz, d, L = HARFE_MONO[i]
                sa = monofil(d, hz, L, "N" if i <= 9 else "D")
            else:
                hz, kern, gesamt, L = HARFE_METALL[i - len(HARFE_MONO)]
                # Stahlkern, Umwicklung Kupfer als voller Ring (Annahme wie S198); Biegung vom Kern, Verluste uebertragen
                m = 7850 * np.pi * (kern / 1000) ** 2 / 4 + 8900 * np.pi * ((gesamt / 1000) ** 2 - (kern / 1000) ** 2) / 4
                sa = {"T": 4 * L * L * hz * hz * m, "m": m, "EI": 200e9 * np.pi * (kern / 1000) ** 4 / 64, "etaF": 5e-5, "etaB": 2.0e-2,
                      "etaA": 1.1, "hz": hz}
            laenge = L * 2 ** (-versatz / 12)
            return sa, laenge, 0.36 * laenge, 0.004, 25.0, 0.0075, 2.0
    return None


def cembalo_taste(taste):
    t = [z[0] for z in TASKIN]
    L = float(np.exp(np.interp(taste, t, np.log([z[1] for z in TASKIN])))) / 1000
    zupf = float(np.exp(np.interp(taste, t, np.log([z[2] for z in TASKIN])))) / 1000
    d, rho, e = next((d, rho, e) for bis, d, rho, e in TASKIN_DRAHT if taste <= bis)
    d /= 1000
    m = rho * np.pi * d * d / 4
    # Verlust: Luft nach Stokes (wie monofil), Biegung mit eta_E = 0,001 - ANNAHME (fuer Draht keine Quelle gelesen)
    sa = {"T": 4 * L * L * hz_von(taste) ** 2 * m, "m": m, "EI": e * 1e9 * np.pi * d ** 4 / 64, "etaB": 1e-3, "d": d, "rho": rho,
          "hz": hz_von(taste)}
    if "--auslenkung" in sys.argv:
        aus = float(np.interp(zupf / L, [p[0] for p in PERNG], [p[1] for p in PERNG])) / 1000
    else:
        aus = CEMBALO_KRAFT * zupf * (L - zupf) / (sa["T"] * L)
    return sa, L, L - zupf, aus, 20.0, 0.003, 0.0


def tasten_instrument(name, je_taste):
    koerper, je = {"harfe": ("concert_harp", harfe_taste), "cembalo": ("harpsichord", cembalo_taste)}[name]
    kp = lade_koerper(koerper)
    datei = len(json.loads((KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))["modes"])
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    w = frequenzen()
    tan = richtung_tan(kp["f"])
    print(f"{name}: Koerper {koerper}, {datei} Moden in der Datei, {len(kp['f'])} mit dem statistischen Teil")
    print("Taste | Laenge m, Zug N, Zupfort m | Lautheit alle Moden / Moden der Datei (LUFS) | Pegel 1-2 s gegen den Anfang: alle / Datei (dB) "
          "| Abfall frueh dB/s")
    zeilen = []
    for taste in range(kp["tasten"][0], kp["tasten"][-1] + 1, 1 if je_taste else 3):
        angabe = je(taste)
        if angabe is None:
            continue
        sa, laenge, xp, d, winkel, breite, tempo = angabe
        punkt = kp["tasten"].index(taste)
        Y, H = koerper_matrix(kp, w, tan, [punkt])
        Yd, _ = koerper_matrix(nur, w, tan[:datei], [punkt])
        alle = zeit(ton_mit([sa], [laenge], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
        dat = zeit(ton_mit([sa], [laenge], 0, xp, d, winkel, Yd, H, w, breite, tempo)[0])
        v, e = VORLAUF, VORLAUF + 0.4
        zeilen.append((taste, momentary_max(alle), momentary_max(dat), rms_db(alle, 1, 2) - rms_db(alle, v, e),
                       rms_db(dat, 1, 2) - rms_db(dat, v, e), abfall(alle)[0]))
        z = zeilen[-1]
        print(f"{taste:3d} | {laenge:6.3f} {sa['T']:6.1f} {xp:6.3f} | {z[1]:6.1f} / {z[2]:6.1f} | {z[3]:6.1f} / {z[4]:6.1f} | {z[5]:6.1f}", flush=True)
    a = np.array(zeilen)
    l = a[:, 1]
    d3 = np.abs(l[3:] - l[:-3]) if je_taste else np.abs(np.diff(l))
    print(f"Spanne {l.max() - l.min():.1f} LU, groesster Sprung im Terzabstand {d3.max():.1f} LU (bei Taste {int(a[int(np.argmax(d3)), 0])}), "
          f"mittlerer {np.median(d3):.1f} LU")
    print(f"GZ5 a, Moden der Datei gegen alle: Lautheit groesster Betrag {np.abs(a[:, 2] - a[:, 1]).max():.2f} LU, "
          f"Pegel 1-2 s groesster Betrag {np.abs(a[:, 4] - a[:, 3]).max():.2f} dB")
    print("Abfall frueh je Oktave (Median, dB/s): " + "  ".join(
        f"C{von // 12 - 1} {np.nanmedian(a[(a[:, 0] >= von) & (a[:, 0] < von + 12), 5]):.1f}" for von in range(24, 108, 12)
        if ((a[:, 0] >= von) & (a[:, 0] < von + 12)).any()))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] in ("harfe", "cembalo"):
        tasten_instrument(sys.argv[1], "--je-taste" in sys.argv)
        sys.exit(0)
    if len(sys.argv) > 1 and sys.argv[1] in ("laute", "oud"):
        chor_instrument(sys.argv[1], "--je-taste" in sys.argv)
        sys.exit(0)
    if len(sys.argv) > 1 and sys.argv[1] == "gitarre-mit":
        gitarre_mit("--je-taste" in sys.argv)
        sys.exit(0)
    if len(sys.argv) < 2 or sys.argv[1] != "gitarre":
        sys.exit(__doc__)
    gitarre("--je-taste" in sys.argv, "--steg-neu" in sys.argv,
            float(sys.argv[sys.argv.index("--winkel") + 1]) if "--winkel" in sys.argv else None)
