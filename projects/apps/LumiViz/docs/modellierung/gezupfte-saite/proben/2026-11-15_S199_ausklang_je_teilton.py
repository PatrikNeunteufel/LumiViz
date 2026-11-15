"""S199, 34k-6 — Ausklang je Teilton: die Iowa-Gitarre (ff, je Saite) gegen den Pruefstand, mit derselben Messung.

Aufruf (vom Projekt-Root):
    python -B <dieses Skript> pruefe
    python -B <dieses Skript> iowa <Ordner Iowa_MIS_Guitar_mono2496> [<ausgabe.txt>]

Messung je Teilton n (bis 12, hoechstens 6 kHz): die Frequenz in +-2 % um n f1 an der groessten Projektion gesucht
(0,05-0,45 s nach dem Einsatz), dann die Huellkurve aus Projektionen (Hann 80 ms, unter 120 Hz 160 ms; Schritt 20 ms).
Abfall in dB/s als Gerade durch die Huellkurve ab 40 ms nach ihrer Spitze: `frueh` bis 15 dB unter der Spitze, `ganz` bis
40 dB darunter — beide nur bis 8 dB ueber dem Boden der Huellkurve (Median ihres letzten Zehntels) und hoechstens 4 s.

Iowa MIS (Univ. of Iowa Electronic Music Studios): Nylongitarre Raimundo 118, schalltot, 24 Bit / 96 kHz, chromatische
Laeufe je Saite, die Toene durch digitale Stille getrennt. Verglichen wird mit dem Pruefstand an derselben Saite und
demselben Bund (Saiten nach Woodhouse 2012, Koerper g34, zwei Polarisationen unter 30 Grad) und mit den Verlusten der
Saite allein (Valette, starrer Steg). Der Abfall eines Teiltons haengt nicht vom Zupfort ab.
"""
import aifc
import importlib.util
import pathlib
import re
import sys
import warnings

import numpy as np
from scipy.optimize import minimize_scalar

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.with_name("2026-11-15_S199_pruefstand_zupf.py"))
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

N_MAX = 12
NOTEN = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}
SAITEN = {"sulE": 0, "sulA": 1, "sulD": 2, "sulG": 3, "sulB": 4, "sul_E": 5}


def projektion(x, fs, hz, a, n):
    fen = np.hanning(n)
    t = np.arange(a, a + n) / fs
    return abs(np.sum(x[a:a + n] * fen * np.exp(-2j * np.pi * hz * t))) * 2 / fen.sum()


def gerade(t, db, bis, boden):
    i0 = int(np.argmax(db))
    start = i0 + 2
    grenze = max(db[i0] - bis, boden + 8.0)
    unter = np.nonzero(db[start:] < grenze)[0]
    ende = start + (int(unter[0]) if len(unter) else len(db) - start)
    ende = min(ende, start + int(4.0 / (t[1] - t[0])))
    if ende - start < 5:
        return float("nan")
    return float(-np.polyfit(t[start:ende], db[start:ende], 1)[0])


def teiltoene(x, fs, f_soll):
    """Je Teilton (n, Hz, Pegel der Spitze in dB, Abfall frueh, Abfall ganz)."""
    ein = int(np.argmax(np.abs(x) > 0.1 * np.abs(x).max()))
    x = x[ein:]
    nw = int((0.16 if f_soll < 120 else 0.08) * fs)
    a0, n0 = int(0.05 * fs), int(0.4 * fs)
    if len(x) < a0 + n0 + nw:
        return []
    f1 = minimize_scalar(lambda h: -projektion(x, fs, h, a0, n0), bounds=(0.97 * f_soll, 1.03 * f_soll), method="bounded",
                         options={"xatol": 1e-3}).x
    aus = []
    schritt = int(0.02 * fs)
    starts = np.arange(0, len(x) - nw, schritt)
    t = starts / fs
    for n in range(1, N_MAX + 1):
        if n * f1 > 6000:
            break
        hz = minimize_scalar(lambda h: -projektion(x, fs, h, a0, n0), bounds=(0.98 * n * f1, 1.02 * n * f1), method="bounded",
                             options={"xatol": 1e-3}).x
        fen = np.hanning(nw)
        tt = np.arange(nw) / fs
        traeger = fen * np.exp(-2j * np.pi * hz * tt)
        db = 20 * np.log10(np.array([abs(np.sum(x[a:a + nw] * traeger)) for a in starts]) * 2 / fen.sum() + 1e-300)
        boden = float(np.median(db[-max(3, len(db) // 10):]))
        aus.append((n, hz, float(db.max()), gerade(t, db, 15.0, boden), gerade(t, db, 40.0, boden)))
    return aus


def lies_aiff(pfad):
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        with aifc.open(str(pfad), "rb") as f:
            fs, breite, n = f.getframerate(), f.getsampwidth(), f.getnframes()
            roh = np.frombuffer(f.readframes(n), dtype=np.uint8).reshape(-1, breite)
    wert = np.zeros(len(roh), dtype=np.int64)
    for b in range(breite):
        wert = (wert << 8) | roh[:, b]
    wert = np.where(wert >= 1 << (8 * breite - 1), wert - (1 << (8 * breite)), wert)
    return wert / float(1 << (8 * breite - 1)), fs


def stuecke(x, fs):
    """Wie `toeneDesLaufs` (S195): 10-ms-Bloecke ueber -90 dBFS, mindestens 1 s, Pausen unter 30 ms trennen nicht."""
    b = fs // 100
    e = 10 * np.log10(np.mean(x[:len(x) // b * b].reshape(-1, b) ** 2, axis=1) + 1e-30)
    laut = e > -90
    aus, i = [], 0
    while i < len(laut):
        if not laut[i]:
            i += 1
            continue
        von, still, ende = i, 0, i
        while i < len(laut) and still < 3:
            still = 0 if laut[i] else still + 1
            ende = i if laut[i] else ende
            i += 1
        if (ende - von) * b >= fs:
            aus.append((von * b, (ende + 1) * b))
    return aus


def midi(name):
    m = re.fullmatch(r"([A-G])(b?)(\d)", name)
    return 12 * (int(m.group(3)) + 1) + NOTEN[m.group(1)] - (1 if m.group(2) else 0)


def saite_allein(sa, laenge, n):
    """Abfall des Teiltons n am starren Steg in dB/s (Valette)."""
    T, m, EI = sa["T"], sa["m"], sa["EI"]
    k = n * np.pi / laenge
    wn = np.sqrt((T * k * k + EI * k ** 4) / m)
    eta = (T * (sa["etaF"] + sa["etaA"] / wn) + EI * sa["etaB"] * k * k) / (T + EI * k * k)
    return 20 * np.log10(np.e) * eta * wn / 2


def pruefe():
    fs = 96000
    t = np.arange(int(5 * fs)) / fs
    rng = np.random.default_rng(1)
    f0, soll = 110.3, {}
    x = 1e-5 * rng.standard_normal(len(t))
    for n in range(1, 13):
        rate = 3.0 + 1.5 * n
        soll[n] = rate
        x += (1.0 / n) * np.exp(-rate / 8.686 * t) * np.sin(2 * np.pi * n * f0 * (1 + 2e-5 * n * n) * t + n)
    x = np.concatenate([np.zeros(4800), x])
    print("synthetisch: n, Hz gemessen / Soll, Abfall frueh / ganz / Soll (dB/s)")
    for n, hz, _, fr, ga in teiltoene(x, fs, 110.0):
        print(f"  {n:2d}  {hz:9.3f} / {n * f0 * (1 + 2e-5 * n * n):9.3f}   {fr:6.2f} / {ga:6.2f} / {soll[n]:6.2f}")


def iowa(ordner, aus):
    kp = ps.lade_koerper("classical_guitar")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    kurven = {}
    zeilen = []

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    schreibe("Saite Bund Taste | n: Abfall frueh in dB/s  Iowa / Modell / Saite allein")
    for pfad in sorted(pathlib.Path(ordner).glob("Guitar.ff.*.aif")):
        m = re.fullmatch(r"Guitar\.ff\.(sul_?[A-G])\.([A-G]b?\d)([A-G]b?\d)?\.mono\.aif", pfad.name)
        s = SAITEN[m.group(1)]
        von, bis = midi(m.group(2)), midi(m.group(3) or m.group(2))
        x, fs = lies_aiff(pfad)
        st = stuecke(x, fs)
        if len(st) != bis - von + 1:
            schreibe(f"# {pfad.name}: {len(st)} Stuecke, erwartet {bis - von + 1} - ausgelassen")
            continue
        for (a, e), taste in zip(st, range(von, bis + 1)):
            bund = taste - ps.GITARRE[s]["taste"]
            if bund not in (0, 2, 3, 5, 7, 9, 10, 12):
                continue
            gem = teiltoene(x[a:e], fs, 440.0 * 2 ** ((taste - 69) / 12))
            if not gem:
                continue
            if s not in kurven:
                kurven[s] = ps.koerper_kurven2(kp, s, w, tan)
            laenge = ps.MENSUR * 2 ** (-bund / 12)
            y = ps.zeit(ps.ton2(ps.GITARRE[s], laenge, min(ps.ZUPFORT, 0.5 * laenge), 0.002, 30.0, *kurven[s], w))
            mod = {n: (fr, ga) for n, _, _, fr, ga in teiltoene(y[:, 0] + y[:, 1], ps.FS, np.sqrt(ps.GITARRE[s]["T"] / ps.GITARRE[s]["m"]) / (2 * laenge))}
            teile = []
            for n, hz, _, fr, ga in gem[:8]:
                allein = saite_allein(ps.GITARRE[s], laenge, n)
                teile.append(f"{n}: {fr:5.1f} / {mod.get(n, (float('nan'),))[0]:5.1f} / {allein:5.1f}")
                zeilen.append((s, bund, n, fr, mod.get(n, (float("nan"), float("nan")))[0], allein, ga, mod.get(n, (float("nan"), float("nan")))[1]))
            schreibe(f"{6 - s}  {bund:2d}  {taste:3d} | " + "   ".join(teile))
    z = np.array(zeilen, dtype=float)
    schreibe()
    schreibe("Median je Saite und Teiltongruppe: Abfall frueh Iowa / Modell / Saite allein (dB/s); Verhaeltnis Modell : Iowa; ganz Iowa / Modell")
    for s in range(6):
        for name, lo, hi in (("n = 1", 1, 1), ("n = 2-4", 2, 4), ("n = 5-8", 5, 8)):
            g = z[(z[:, 0] == s) & (z[:, 2] >= lo) & (z[:, 2] <= hi) & np.isfinite(z[:, 3]) & np.isfinite(z[:, 4])]
            if len(g):
                schreibe(f"  Saite {6 - s}  {name:8s}  {np.median(g[:, 3]):5.1f} / {np.median(g[:, 4]):5.1f} / {np.median(g[:, 5]):5.1f}   "
                         f"{np.median(g[:, 4] / g[:, 3]):4.2f}   {np.nanmedian(g[:, 6]):5.1f} / {np.nanmedian(g[:, 7]):5.1f}   ({len(g)} Werte)")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "pruefe":
        pruefe()
    elif len(sys.argv) > 2 and sys.argv[1] == "iowa":
        datei = open(sys.argv[3], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 3 else None
        iowa(sys.argv[2], datei)
    else:
        sys.exit(__doc__)
