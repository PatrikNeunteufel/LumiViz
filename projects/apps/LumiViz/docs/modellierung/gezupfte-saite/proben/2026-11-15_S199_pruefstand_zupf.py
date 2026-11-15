"""S199, 34k-6 — Pruefstand ohne Kern: eine gezupfte Saite am Steg eines Koerpers, im Frequenzbereich gerechnet.

Aufruf (vom Projekt-Root): python -B <dieses Skript> gitarre [--je-taste] [--steg-neu] [--winkel <Grad>]
                           python -B <dieses Skript> gitarre-mit [--je-taste]

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


def ton2(saite, laenge, x_p, d, winkel_grad, Y, Hz, Hy, w, breite=0.0075):
    """Zwei Polarisationen: (I + R_b E) v+ = g (cos a, sin a), R_b = (I + Z0 Y)^-1 (Z0 Y - I), F = 2 Z0 (I + Z0 Y)^-1 v+."""
    T, m, EI = saite["T"], saite["m"], saite["EI"]
    k2 = (-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI)
    k = np.sqrt(k2)
    eta = (T * (saite["etaF"] + saite["etaA"] / w) + EI * saite["etaB"] * k2) / (T + EI * k2)
    k = k * (1 - 0.5j * eta)
    z0 = T * k / w
    f0 = T * d * laenge / (x_p * (laenge - x_p))
    f = -f0 / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2)
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


def koerper_matrix(kp, w, tan):
    """Y (Frequenz x 2S x 2S) zwischen allen Stegpunkten und beiden Richtungen, H (Frequenz x 2S x Hoerorte)."""
    S = kp["steg"].shape[1]
    wn = 2 * np.pi * kp["f"]
    phi = np.empty((len(wn), 2 * S))
    phi[:, 0::2] = kp["steg"]
    phi[:, 1::2] = kp["steg"] * tan[:, None]
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
    eta = (T * (saite["etaF"] + saite["etaA"] / w) + EI * saite["etaB"] * k2) / (T + EI * k2)
    k = k * (1 - 0.5j * eta)
    return k, T * k / w


def ton_mit(saiten, laengen, gespielt, x_p, d, winkel_grad, Y, H, w, breite=0.0075):
    """Alle Saiten an einem Steg; `gespielt` wird gezupft. Gibt (p, p der nicht gespielten Saiten)."""
    S, n = len(saiten), len(w)
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        k, z0 = saite_wellen(sa, w)
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = np.exp(-2j * k * L)
        if i == gespielt:
            T, m = sa["T"], sa["m"]
            f = -(T * d * L / (x_p * (L - x_p))) / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2)
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
        F[:, 2 * gespielt:2 * gespielt + 2] = 0
        rest[b] = np.einsum("fa,fao->fo", F, H[b])
    return p, rest


# ---------------------------------------------------------------- Saite und Ton
def ton(saite, laenge, x_p, d, Y, H, w, breite=0.0075):
    T, m, EI = saite["T"], saite["m"], saite["EI"]
    k2 = (-T + np.sqrt(T * T + 4 * EI * m * w ** 2)) / (2 * EI)
    k = np.sqrt(k2)
    eta = (T * (saite["etaF"] + saite["etaA"] / w) + EI * saite["etaB"] * k2) / (T + EI * k2)
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


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "gitarre-mit":
        gitarre_mit("--je-taste" in sys.argv)
        sys.exit(0)
    if len(sys.argv) < 2 or sys.argv[1] != "gitarre":
        sys.exit(__doc__)
    gitarre("--je-taste" in sys.argv, "--steg-neu" in sys.argv,
            float(sys.argv[sys.argv.index("--winkel") + 1]) if "--winkel" in sys.argv else None)
