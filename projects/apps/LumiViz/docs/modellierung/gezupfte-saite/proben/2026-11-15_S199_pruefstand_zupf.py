"""S199, 34k-6 — Pruefstand ohne Kern: eine gezupfte Saite am Steg eines Koerpers, im Frequenzbereich gerechnet.

Aufruf (vom Projekt-Root): python -B <dieses Skript> gitarre [--je-taste]

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


def gitarre(je_taste):
    kp = lade_koerper("classical_guitar")
    w = frequenzen()
    kurven = {}
    zeilen = []
    for taste in range(40, 84, 1 if je_taste else 3):
        s = max(i for i, sa in enumerate(GITARRE) if sa["taste"] <= taste)
        bund = taste - GITARRE[s]["taste"]
        if bund > 19:
            continue
        if s not in kurven:
            kurven[s] = koerper_kurven(kp, kp["tasten"].index(GITARRE[s]["taste"]), w)
        laenge = MENSUR * 2 ** (-bund / 12)
        x = zeit(ton(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, *kurven[s], w))
        steif = zeit(ton(GITARRE[s], laenge, min(ZUPFORT, 0.5 * laenge), 0.002, np.zeros(len(w)), kurven[s][1], w))
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


if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1] != "gitarre":
        sys.exit(__doc__)
    gitarre("--je-taste" in sys.argv)
