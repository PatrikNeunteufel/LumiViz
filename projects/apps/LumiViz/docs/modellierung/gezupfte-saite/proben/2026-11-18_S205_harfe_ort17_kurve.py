"""S205 - KQ31 a: Harfe ohne Saiten an Bells Messort 17, die ganze Kurve statt 23 Stuetzstellen.

Bell 1988 Abb. 7.6 (PDF 293), Feld 2 „completed harp (without strings)“ und Feld 1 „isolated soundbox“, spaltenweise
abgegriffen (Geometrie wie `recherche/2026-11-17_S203_harfe_nacharbeit_werkbank/d1…`, `d2…`: 120 dpi, 100 Hz bei
x 178,5 px, 800 Hz bei 798 px; 116 dB = 1 s/kg), gegen die Koerperdatei `concert_harp` an den Oesen der Saiten 20 / 21.

Aufruf (vom Projekt-Root):
  python -X utf8 -B <dieses Skript> <bell293_120-293.pgm> [koerper=<datei.lvinstbody>] [bild=<datei.png>] [glatt=<Hz>]
Das Graubild entsteht mit `pdftoppm -r 120 -gray -f 293 -l 293 <Bell_1988….pdf> bell293_120` (liegt nicht im Repo).
Gibt je Terzband das energetische Mittel (Bell, Modell, Abstand), Taeler und Spitzen aus; schreibt nur das Bild.
"""
import importlib.util
import pathlib
import sys

import numpy as np

P = pathlib.Path(__file__).parent


def modul(datei, name):
    spec = importlib.util.spec_from_file_location(name, P / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def lade_pgm(pfad):
    daten = pathlib.Path(pfad).read_bytes()
    teile, i = [], 0
    while len(teile) < 4:
        while daten[i:i + 1].isspace():
            i += 1
        j = i
        while not daten[j:j + 1].isspace():
            j += 1
        teile.append(daten[i:j])
        i = j
    w, h = int(teile[1]), int(teile[2])
    return np.frombuffer(daten[i + 1:i + 1 + w * h], dtype=np.uint8).reshape(h, w)


X100, X800 = 178.5, 798.0
FELDER = {  # y oben, y unten, (y, dB), (y, dB), Gitterzeilen
    "lose": (140, 442, (202.5, 90), (380.5, 70), (202, 203, 291, 292, 380, 381)),
    "ohne": (505, 822, (558.5, 90), (735.5, 70), (558, 559, 647, 648, 735, 736)),
    "mit": (850, 1033, (857.0, 90), (947.5, 80), (857, 947, 948)),
}


def spur(bild, name):
    """-> f (Hz), dB (Median der dunklen Zeilen je Spalte), dB oben, dB unten"""
    y0, y1, (ya, da), (yb, db), gitter = FELDER[name]
    f, v, o, u = [], [], [], []
    for x in range(int(X100), int(X800) + 1):
        ys = [y for y in range(y0, y1) if bild[y, x] < 140 and all(abs(y - g) > 1 for g in gitter)]
        if not ys:
            continue
        z = lambda y: da + (y - ya) * (db - da) / (yb - ya)  # noqa: E731
        f.append(100 + (x - X100) * 700 / (X800 - X100))
        v.append(z(ys[len(ys) // 2])); o.append(z(ys[0])); u.append(z(ys[-1]))
    return np.array(f), np.array(v), np.array(o), np.array(u)


def e_mittel(db):
    return 10 * np.log10(np.mean(10 ** (np.asarray(db) / 10)))


def modell(kp, punkte, hz, feder=0.0):
    """|Y|^2 gemittelt ueber die Stegpunkte, in Bells dB; feder > 0: dazu eine Feder in Reihe am Messpunkt (i w / k)"""
    w = 2 * np.pi * hz
    wn = 2 * np.pi * kp["f"]
    n = 1.0 / (wn[None, :] ** 2 * (1 + 1j * kp["eta"][None, :]) - w[:, None] ** 2)
    zus = 1j * w / feder if feder > 0 else 0.0
    y2 = np.mean([np.abs((1j * w[:, None] * n) @ (kp["steg"][:, p] ** 2) + zus) ** 2 for p in punkte], axis=0)
    return 10 * np.log10(y2) + 116


if __name__ == "__main__":
    opts = dict(a.split("=", 1) for a in sys.argv[2:])
    bild = lade_pgm(sys.argv[1])
    ps = modul("2026-11-15_S199_pruefstand_zupf.py", "pruefstand")
    ha = modul("2026-11-14_S198_harfe_admittanz.py", "harfe_admittanz")
    if "koerper" in opts:
        ps.KOERPER = pathlib.Path(opts["koerper"]).parent
        kp = ps.lade_koerper(pathlib.Path(opts["koerper"]).stem)
    else:
        kp = ps.lade_koerper("concert_harp")
    punkte = [kp["tasten"].index(int(round(69 + 12 * np.log2(hz / 440.0))) + 1) for nr, hz, *_ in ha.NYLON_DARM if nr in ("20", "21")]
    fein = np.arange(100.0, 800.01, 0.25)
    mod = modell(kp, punkte, fein)
    glatt = float(opts.get("glatt", 0))
    if glatt > 0:  # Mittel von |Y|^2 ueber +- glatt/2 (Bandbreite eines Analysators)
        k = int(round(glatt / 0.25)) | 1
        mod_g = 10 * np.log10(np.convolve(10 ** (mod / 10), np.ones(k) / k, mode="same"))
    else:
        mod_g = mod
    fb, ohne, *_ = spur(bild, "ohne")
    fl, lose, *_ = spur(bild, "lose")
    print(f"Bell ohne Saiten: {len(fb)} Spalten, lose: {len(fl)} Spalten; Modell an {len(punkte)} Stegpunkten (Oesen 20 / 21)"
          f"{', geglaettet ueber %g Hz' % glatt if glatt else ''}")
    print("Terzband | Bell ohne Saiten | Bell lose | Modell | Modell - Bell ohne (energetisch, dB)")
    for fm in (125, 160, 200, 250, 315, 400, 500, 630, 800):
        lo, hi = fm / 2 ** (1 / 6), min(fm * 2 ** (1 / 6), 795.0)
        b, l, m = (e_mittel(v[(f >= lo) & (f < hi)]) for f, v in ((fb, ohne), (fl, lose), (fein, mod)))
        print(f"  {fm:4d} Hz ({lo:5.0f}-{hi:5.0f}) | {b:5.1f} | {l:5.1f} | {m:5.1f} | {m - b:+5.1f}")
    for lo, hi in ((140, 200), (200, 400), (400, 790), (200, 790)):
        b, m = e_mittel(ohne[(fb >= lo) & (fb < hi)]), e_mittel(mod[(fein >= lo) & (fein < hi)])
        bm, mm = np.median(ohne[(fb >= lo) & (fb < hi)]), np.median(mod_g[(fein >= lo) & (fein < hi)])
        b10, m10 = np.percentile(ohne[(fb >= lo) & (fb < hi)], 10), np.percentile(mod_g[(fein >= lo) & (fein < hi)], 10)
        b90, m90 = np.percentile(ohne[(fb >= lo) & (fb < hi)], 90), np.percentile(mod_g[(fein >= lo) & (fein < hi)], 90)
        print(f"Band {lo}-{hi} Hz: energetisch Bell {b:5.1f} / Modell {m:5.1f} ({m - b:+5.1f}) | Median {bm:5.1f} / {mm:5.1f} ({mm - bm:+5.1f}) | "
              f"10. Perz. {b10:5.1f} / {m10:5.1f} ({m10 - b10:+5.1f}) | 90. Perz. {b90:5.1f} / {m90:5.1f} ({m90 - b90:+5.1f})")
    federn = [float(x) for x in opts["feder"].split(",")] if "feder" in opts else []
    kurven = {}
    if federn:
        print("\nFeder in Reihe am Messpunkt (Y + i w / k): Abstand Modell - Bell ohne Saiten")
        print("  k N/m   | Terzbaender 200 250 315 400 500 630 800 (energetisch) | 200-790 Hz: energetisch, Median, 10. Perz., 90. Perz. | Taeler Hz")
        for k in federn:
            mk = modell(kp, punkte, fein, k)
            kurven[k] = mk
            t = []
            for fm in (200, 250, 315, 400, 500, 630, 800):
                lo, hi = fm / 2 ** (1 / 6), min(fm * 2 ** (1 / 6), 795.0)
                t.append(e_mittel(mk[(fein >= lo) & (fein < hi)]) - e_mittel(ohne[(fb >= lo) & (fb < hi)]))
            a, b = mk[(fein >= 200) & (fein < 790)], ohne[(fb >= 200) & (fb < 790)]
            fa = fein[(fein >= 200) & (fein < 790)]
            tal = [fa[i] for i in range(8, len(a) - 8) if a[i] == a[i - 8:i + 9].min() and a[i] < np.median(a) - 3]
            print(f"  {k:8.2e} | " + " ".join(f"{x:+5.1f}" for x in t) + f" | {e_mittel(a) - e_mittel(b):+5.1f} {np.median(a) - np.median(b):+5.1f} "
                  f"{np.percentile(a, 10) - np.percentile(b, 10):+5.1f} {np.percentile(a, 90) - np.percentile(b, 90):+5.1f} | " + " ".join(f"{x:.0f}" for x in tal))
        bo = ohne[(fb >= 200) & (fb < 790)]
        fo = fb[(fb >= 200) & (fb < 790)]
        print("  Bell: Taeler bei " + " ".join(f"{fo[i]:.0f}" for i in range(8, len(bo) - 8) if bo[i] == bo[i - 8:i + 9].min() and bo[i] < np.median(bo) - 3)
              + " Hz; Spitzen bei " + " ".join(f"{fo[i]:.0f}" for i in range(8, len(bo) - 8) if bo[i] == bo[i - 8:i + 9].max() and bo[i] > np.median(bo) + 3) + " Hz")
    print("Moden der Datei 100-800 Hz: " + " ".join(f"{x:.0f}" for x in kp["f"][(kp["f"] > 100) & (kp["f"] < 800)]))
    if "bild" in opts:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(13, 6))
        ax.plot(fb, ohne, "k", lw=1.6, label="Bell Abb. 7.6 Feld 2: Harfe ohne Saiten, Ort 17")
        ax.plot(fl, lose, color="0.6", lw=1.0, label="Bell Feld 1: loser Kasten")
        ax.plot(fein, mod, "r", lw=1.0, label="Modell, Oesen 20 / 21")
        if glatt:
            ax.plot(fein, mod_g, "b", lw=1.0, label=f"Modell, |Y|² ueber {glatt:g} Hz gemittelt")
        for k, c in zip(list(kurven)[:3], ("b", "g", "m")):
            ax.plot(fein, kurven[k], c, lw=1.0, label=f"Modell mit Feder {k:.2g} N/m in Reihe")
        ax.set_xlim(100, 800); ax.set_ylim(40, 100); ax.grid(True, alpha=0.3)
        ax.set_xlabel("Hz"); ax.set_ylabel("dB (116 dB = 1 s/kg)"); ax.legend(loc="lower left")
        fig.tight_layout(); fig.savefig(opts["bild"], dpi=110)
        print("Bild:", opts["bild"])
