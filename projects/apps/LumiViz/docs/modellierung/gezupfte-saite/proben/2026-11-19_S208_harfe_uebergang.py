"""S208 - Zerlegung am Uebergang Metall/Darm der Harfe (Tasten 38-56) am Pruefkoerper mit 16 Hoerorten.

Je Taste: Saite (Art, Zug, Laenge), Kraft F0 der Kraftstufe, Lautheit im Leistungsmittel der 16 Orte (mit Koerper am Steg,
starrer Steg), Kraft je Teilton am starren Steg (dBK), |p/F|^2 im Leistungsmittel der 16 Orte je Teilton, Anteil.
Varianten: (a) wie Pruefstand, (b) Taste 44 auf der As-Saite (Darm 104 Hz, flat), (c) Metallsaiten mit Umwicklung zu 80 %
der Masse eines vollen Rings, (d) gleiche Kraft F0 je Taste wie die Darmsaite 45 (statt gleicher Auslenkung).

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner mit concert_harp.lvinstbody> [von=38] [bis=56]
Der Ordner wie in `2026-11-19_S208_harfe_mitschwingen_16.py` (Pruefkoerper mit 16 Hoerorten).
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.signal as sig

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
ps.KOERPER = pathlib.Path(sys.argv[1])
opts = dict(a.split("=") for a in sys.argv[2:] if "=" in a)
VON, BIS = int(opts.get("von", 38)), int(opts.get("bis", 56))

kp = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
hz = w.real / (2 * np.pi)
no = kp["G"].shape[1]
print(f"{len(kp['f'])} Moden, {no} Hoerorte (Ort 0 = front, Mittel ueber 1..{no - 1})")


def k_db(f):
    return 20 * np.log10(np.abs(sig.sosfreqz(ps.k_bewertung(), worN=np.atleast_1d(f), fs=ps.FS)[1]))[0]


def mittel(x):
    return ps.momentary_max(x[:, 1:]) - 10 * np.log10(x.shape[1] - 1)


def metall(hz0, kern, gesamt, L, anteil=1.0):
    ring = 8900 * np.pi * ((gesamt / 1000) ** 2 - (kern / 1000) ** 2) / 4
    m = 7850 * np.pi * (kern / 1000) ** 2 / 4 + anteil * ring
    return {"T": 4 * L * L * hz0 * hz0 * m, "m": m, "EI": 200e9 * np.pi * (kern / 1000) ** 4 / 64, "etaF": 5e-5, "etaB": 2.0e-2,
            "etaA": 1.1, "hz": hz0}


def taste_mit(taste, variante):
    sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
    art = "Darm" if "d" in sa or sa.get("etaA") is None else "Metall"
    if variante == "b" and taste == 44:
        hz0, dm, L0 = ps.HARFE_MONO[-1]          # As-Saite (104 Hz) leer
        sa, L, art = ps.monofil(dm, hz0, L0, "D"), L0, "Darm (As)"
        xp = 0.36 * L
    if variante == "c" and art == "Metall":
        i = [z[0] for z in ps.HARFE_METALL].index(round(sa["hz"]))
        sa = metall(*ps.HARFE_METALL[i], anteil=0.8)
    return sa, L, xp, d, winkel, breite, tempo, art


def f0_von(sa, L, xp, d):
    return sa["T"] * d * L / (xp * (L - xp))


ref = taste_mit(45, "a")
F_REF = f0_von(ref[0], ref[1], ref[2], ref[3])

ergebnis = {}
for variante in ("a", "b", "c", "d"):
    print(f"\n== Variante {variante}")
    zeilen = []
    for taste in range(VON, BIS + 1):
        sa, L, xp, d, winkel, breite, tempo, art = taste_mit(taste, variante)
        if variante == "d":
            d = d * F_REF / f0_von(sa, L, xp, d)
        F0 = f0_von(sa, L, xp, d)
        # As-Saite: Stegpunkt an ihrer eigenen Oese (= der Punkt der Taste 45, A an derselben Saite)
        punkt = kp["tasten"].index(45 if art == "Darm (As)" else taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [punkt])
        null = np.zeros_like(Y)
        eins = np.zeros((len(w), 2, 1), dtype=complex)
        eins[:, 0, 0] = 1
        sp = ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0]
        sp_starr = ps.ton_mit([sa], [L], 0, xp, d, winkel, null, H, w, breite, tempo)[0]
        lu, ls = mittel(ps.zeit(sp)), mittel(ps.zeit(sp_starr))
        zeile = f"{taste:3d} {art:9s} T {sa['T']:5.0f} N  L {L:.3f}  F0 {F0:6.1f} N  | {lu:6.1f} LUFS (starr {ls:6.1f})"
        if variante == "a":
            kraft = ps.ton_mit([sa], [L], 0, xp, d, winkel, null, eins, w, breite, tempo)[0][:, 0]
            f0 = ps.hz_von(taste)
            teile = []
            for n in range(1, 5):
                m = (hz > n * f0 * 0.97) & (hz < n * f0 * 1.03)
                i = np.where(m)[0][np.argmax(np.abs(kraft[m]))]
                kd = k_db(hz[i])
                pf = 10 * np.log10(np.mean(np.abs(H[i, 0, 1:]) ** 2))
                e = np.sum(np.abs(sp_starr[m][:, 1:]) ** 2) * 10 ** (kd / 10)
                teile.append((hz[i], 10 * np.log10(np.sum(np.abs(kraft[m]) ** 2)) + kd, pf, e))
            ges = sum(t[3] for t in teile)
            zeile += " | " + "  ".join(f"{t[0]:5.0f}: F {t[1]:5.1f} p/F {t[2]:5.1f} {100 * t[3] / ges:3.0f}%" for t in teile)
        print(zeile, flush=True)
        zeilen.append((taste, lu))
    a = np.array(zeilen)
    l = a[:, 1]
    paare = [(abs(l[i] - l[j]), int(a[i, 0]), int(a[j, 0])) for i in range(len(l)) for j in range(i + 1, len(l)) if a[j, 0] - a[i, 0] <= 4]
    g = sorted(paare, reverse=True)[:3]
    print(f"Variante {variante}: groesste Paare " + ", ".join(f"{p[0]:.1f} ({p[1]}/{p[2]})" for p in g)
          + f"; mittlerer {np.median([p[0] for p in paare]):.1f} LU (nur Tasten {VON}-{BIS})")
    ergebnis[variante] = a
np.save(pathlib.Path(sys.argv[1]) / "uebergang.npy", np.array([ergebnis[v] for v in "abcd"]))
