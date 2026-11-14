"""Probe zu KQ18 (34k-5, S198): der Rand des Harfenbodens als Feder — ohne Eingriff in `InstrumentBody`.

Die Moden des Bodens mit Leisten im Vakuum (aus `InstrumentBodyTool --shapes`, bezueglich M normiert) dienen als
Ansatz; die Federn am Rand kommen im Modenraum dazu:

    A = diag(omega_k^2) + Phi^T K_Rand Phi,     K_Rand = kw * Int w w ds  +  kd * Int (dw/dn)(dw/dn) ds

    kw   Wegfeder je Laenge des Rands, N/m^2       (nur mit der Basis „Rand frei“)
    kd   Drehfeder je Laenge des Rands, N (= N m/rad je m)

Zwei Basen:
    --gestuetzt <formen.txt>   Basis mit w = 0 am Rand (`--supported-rim`): nur die Drehfeder; kd = 0 gibt die Basis
                               zurueck, kd -> unendlich muss den eingespannten Rand treffen.
    --frei <formen.txt>        Basis mit freiem Rand (`--free-rim --from 5`): Weg- und Drehfeder; die drei
                               Starrkoerperformen (1, x, y) kommen aus einer geschaetzten Massenbelegung dazu.
                               kw -> unendlich, kd = 0 muss den gestuetzten Rand treffen.

Die Neigung am Rand ist der Gradient des anliegenden Dreiecks (linear aus w an den Ecken) — eine Naeherung; die
beiden Grenzfaelle oben sind die Gegenprobe. Ein abgeschnittener Ansatz mit Federn liegt immer ueber dem wahren Wert.

    python …_harfe_rand_feder.py --gestuetzt <datei> [--frei <datei>] [--gesetz gleich|breite] [--bis <Hz>]
                                 [--kd <N> …] [--kw <N/m2> …] [--suche]

  --gesetz breite   kd(y) = kd * b(0) / b(y): die Drehfeder waechst zum Diskant wie 1 / Breite (Schale mit kleinerem Radius)
  --suche           sucht kd (und mit --frei auch kw) nach dem kleinsten mittleren Betrag der Abweichung je Ziel
"""
import importlib.util
import os
import sys

import numpy as np

HIER = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("harfe_moden", os.path.join(HIER, "2026-11-13_S197_harfe_moden.py"))
hm = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hm)

LAENGE, B0, B1 = 1.31, 0.41, 0.10
# Ziele: Familie S(1,n) = (0 Wechsel quer, n-1 Wechsel laengs), S(2,1) = (1, 0). Bell 1988.
ZIELE = {
    "F lose (Tab. 6.1)": {(0, 1): 250, (0, 2): 332, (0, 3): 421, (0, 4): 535, (0, 5): 660, (0, 6): 775, (0, 7): 970, (1, 0): 468},
    "G lose (Abb. 7.10-7.16)": {(0, 1): 221, (0, 2): 320, (0, 3): 420, (0, 4): 557, (0, 5): 702, (0, 6): 768, (0, 7): 912},
    "G besaitet (Abb. 7.10-7.16)": {(0, 1): 257, (0, 2): 334, (0, 3): 430, (0, 4): 555, (0, 5): 697, (0, 6): 753, (0, 7): 895},
}


def name(mn):
    return f"S({mn[0] + 1},{mn[1] + 1})"


def breite(y):
    return B0 + (B1 - B0) * y / LAENGE


def randkanten(dreiecke):
    zahl = {}
    for t, d in enumerate(dreiecke):
        for a, b in ((d[0], d[1]), (d[1], d[2]), (d[2], d[0])):
            zahl.setdefault((min(a, b), max(a, b)), []).append(t)
    return [(k, v[0]) for k, v in zahl.items() if len(v) == 1]


def dreiecke_aus(datei):
    z = open(datei, encoding="utf-8").read().split("\n")
    n = int(z[1].split()[1])
    t = int(z[2 + n].split()[1])
    return np.array([[int(v) for v in s.split()] for s in z[3 + n:3 + n + t]])


def rand_matrizen(knoten, dreiecke, phi, gesetz):
    """Phi^T (Int w w ds) Phi und Phi^T (Int g(y) dw/dn dw/dn ds) Phi; phi: Moden x Knoten."""
    m = phi.shape[0]
    weg, dreh = np.zeros((m, m)), np.zeros((m, m))
    for (a, b), t in randkanten(dreiecke):
        pa, pb = knoten[a], knoten[b]
        lang = np.linalg.norm(pb - pa)
        wa, wb = phi[:, a], phi[:, b]
        weg += lang / 6.0 * (2 * np.outer(wa, wa) + np.outer(wa, wb) + np.outer(wb, wa) + 2 * np.outer(wb, wb))
        d = dreiecke[t]
        p = knoten[d]
        flaeche2 = (p[1, 0] - p[0, 0]) * (p[2, 1] - p[0, 1]) - (p[2, 0] - p[0, 0]) * (p[1, 1] - p[0, 1])
        # Gradient der linearen Ansatzfunktionen
        gx = np.array([p[1, 1] - p[2, 1], p[2, 1] - p[0, 1], p[0, 1] - p[1, 1]]) / flaeche2
        gy = np.array([p[2, 0] - p[1, 0], p[0, 0] - p[2, 0], p[1, 0] - p[0, 0]]) / flaeche2
        tang = (pb - pa) / lang
        normal = np.array([tang[1], -tang[0]])
        neigung = phi[:, d] @ (normal[0] * gx + normal[1] * gy)
        faktor = B0 / breite(0.5 * (pa[1] + pb[1])) if gesetz == "breite" else 1.0
        dreh += faktor * lang * np.outer(neigung, neigung)
    return weg, dreh


def massen(knoten, dreiecke):
    """Geschaetzte Masse je Knoten: Platte 500 kg/m3 mal Dicke, die Leisten als Linien in der Mitte und im Viertel."""
    masse = np.zeros(len(knoten))
    for d in dreiecke:
        p = knoten[d]
        fl = 0.5 * abs((p[1, 0] - p[0, 0]) * (p[2, 1] - p[0, 1]) - (p[2, 0] - p[0, 0]) * (p[1, 1] - p[0, 1]))
        y = p[:, 1].mean()
        h = 0.010 + (0.003 - 0.010) * y / LAENGE + 0.001
        masse[d] += 500.0 * h * fl / 3.0
    return masse


def starr(knoten, masse, phi):
    """Drei Starrkoerperformen, bezueglich der geschaetzten Masse normiert und zu den elastischen Formen senkrecht."""
    roh = [np.ones(len(knoten)), knoten[:, 0].copy(), knoten[:, 1].copy()]
    fertig = []
    for r in roh:
        for f in fertig:
            r = r - (f * masse) @ r * f
        for f in phi:
            r = r - (f * masse) @ r * f
        fertig.append(r / np.sqrt((r * masse) @ r))
    return np.array(fertig)


def loese(hz, phi, federn):
    a = np.diag((2 * np.pi * hz) ** 2) + federn
    wert, vek = np.linalg.eigh(a)
    f = np.sqrt(np.maximum(wert, 0.0)) / (2 * np.pi)
    zahl = int(np.searchsorted(f, 1300.0)) + 1  # die Formen nur, soweit sie zugeordnet werden
    return f[:zahl], (vek[:, :zahl].T @ phi)


_LINIEN = {}


def linien(knoten):
    """Die Abtastlinien von `harfe_moden.indizes` als Knotennummern, einmal je Netz gesucht."""
    if id(knoten) not in _LINIEN:
        def naechster(x, y):
            return int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))
        quer = np.array([[naechster(x, y) for x in np.linspace(-breite(y) / 2 + 0.002, breite(y) / 2 - 0.002, 21)]
                         for y in np.linspace(0.03, LAENGE - 0.03, 60)])
        laengs = np.array([[naechster(anteil * breite(y), y) for y in np.linspace(0.005, LAENGE - 0.005, 200)] for anteil in (0.0, 0.45)])
        _LINIEN[id(knoten)] = (quer, laengs)
    return _LINIEN[id(knoten)]


def indizes(knoten, w):
    """Wie `harfe_moden.indizes` (S197), mit vorab gesuchten Knoten."""
    quer, laengs = linien(knoten)
    gross = np.abs(w).max()
    zahlen = []
    for reihe in quer:
        linie = w[reihe]
        a = np.abs(linie).max()
        if a >= 0.3 * gross:
            zahlen.append(hm.wechsel(linie, 0.2 * a))
    m = int(np.bincount(zahlen).argmax()) if zahlen else -1
    beste, n = 0.0, 0
    for reihe in laengs:
        linie = w[reihe]
        a = np.abs(linie).max()
        if a > beste:
            beste, n = a, hm.wechsel(linie, 0.1 * a)
    return m, n


def familien(knoten, f, formen, bis):
    gefunden = {}
    for k in range(len(f)):
        if f[k] > bis:
            break
        if f[k] < 20.0:
            continue
        gefunden.setdefault(indizes(knoten, formen[k]), []).append(f[k])
    return gefunden


def abstand(gefunden, ziel):
    ab = []
    for mn, soll in ziel.items():
        if mn not in gefunden:
            ab.append(None)
            continue
        ab.append(100 * (min(gefunden[mn], key=lambda v: abs(v - soll)) / soll - 1))
    return ab


def zeile(gefunden, ziel):
    teile, betrag = [], []
    for (mn, soll), a in zip(ziel.items(), abstand(gefunden, ziel)):
        if a is None:
            teile.append(f"{name(mn)} -")
        else:
            teile.append(f"{name(mn)} {soll * (1 + a / 100):.0f} ({a:+.0f} %)")
            betrag.append(abs(a))
    return " | ".join(teile) + (f" | Mittel |d| {np.mean(betrag):.1f} %, groesste {np.max(betrag):.0f} %" if betrag else "")


def werte(args, name_, vorgabe):
    if name_ not in args:
        return vorgabe
    i = args.index(name_) + 1
    aus = []
    while i < len(args) and not args[i].startswith("--"):
        aus.append(float(args[i]))
        i += 1
    return aus


if __name__ == "__main__":
    args = sys.argv[1:]
    gesetz = args[args.index("--gesetz") + 1] if "--gesetz" in args else "gleich"
    bis = werte(args, "--bis", [1100.0])[0]
    if "--gestuetzt" in args:
        datei = args[args.index("--gestuetzt") + 1]
        knoten, hz, phi = hm.formen(datei)
        if "--moden" in args:  # Gegenprobe der Konvergenz: nur die ersten n Moden der Basis
            zahl = int(werte(args, "--moden", [len(hz)])[0])
            hz, phi = hz[:zahl], phi[:zahl]
        dreiecke = dreiecke_aus(datei)
        _, dreh = rand_matrizen(knoten, dreiecke, phi, gesetz)
        print(f"Basis Rand gestuetzt: {len(hz)} Moden bis {hz[-1]:.0f} Hz, Gesetz der Drehfeder: {gesetz}")
        for kd in werte(args, "--kd", [0.0, 30, 100, 300, 1000, 3000, 1e4, 1e5, 1e7]):
            f, formen = loese(hz, phi, kd * dreh)
            gef = familien(knoten, f, formen, bis)
            print(f"  kd = {kd:9.0f} N")
            for titel, ziel in ZIELE.items():
                print(f"      {titel}: {zeile(gef, ziel)}")
        if "--suche" in args:
            for titel, ziel in ZIELE.items():
                beste = None
                for kd in np.logspace(1, 4.5, 36):
                    f, formen = loese(hz, phi, kd * dreh)
                    ab = [abs(a) for a in abstand(familien(knoten, f, formen, bis), {k: v for k, v in ziel.items() if k[0] == 0}) if a is not None]
                    if beste is None or np.mean(ab) < beste[0]:
                        beste = (np.mean(ab), kd)
                f, formen = loese(hz, phi, beste[1] * dreh)
                print(f"  Suche nur S(1,n), {titel}: kd = {beste[1]:.0f} N -> {zeile(familien(knoten, f, formen, bis), ziel)}")
    if "--frei" in args:
        datei = args[args.index("--frei") + 1]
        knoten, hz, phi = hm.formen(datei)
        dreiecke = dreiecke_aus(datei)
        masse = massen(knoten, dreiecke)
        st = starr(knoten, masse, phi)
        hz = np.concatenate([np.zeros(3), hz])
        phi = np.vstack([st, phi])
        weg, dreh = rand_matrizen(knoten, dreiecke, phi, gesetz)
        print(f"Basis Rand frei: {len(hz)} Formen (3 starr, Masse der Platte geschaetzt {masse.sum():.3f} kg) bis {hz[-1]:.0f} Hz, Gesetz: {gesetz}")
        kws = werte(args, "--kw", [1e12, 1e9, 1e8, 3e7, 1e7, 3e6])
        kds = werte(args, "--kd", [0.0])
        for kw in kws:
            for kd in kds:
                f, formen = loese(hz, phi, kw * weg + kd * dreh)
                gef = familien(knoten, f, formen, bis)
                print(f"  kw = {kw:.1e} N/m2, kd = {kd:.0f} N")
                for titel, ziel in ZIELE.items():
                    print(f"      {titel}: {zeile(gef, ziel)}")
        if "--suche" in args:
            for titel, ziel in ZIELE.items():
                beste = None
                for kw in np.logspace(5.5, 9, 15):
                    for kd in np.logspace(1.5, 4.5, 16):
                        f, formen = loese(hz, phi, kw * weg + kd * dreh)
                        ab = [abs(a) for a in abstand(familien(knoten, f, formen, bis), ziel) if a is not None]
                        if len(ab) == len(ziel) and (beste is None or np.mean(ab) < beste[0]):
                            beste = (np.mean(ab), kw, kd)
                if beste:
                    f, formen = loese(hz, phi, beste[1] * weg + beste[2] * dreh)
                    print(f"  Suche, {titel}: kw = {beste[1]:.1e} N/m2, kd = {beste[2]:.0f} N -> {zeile(familien(knoten, f, formen, bis), ziel)}")
