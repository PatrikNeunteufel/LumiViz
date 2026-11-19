"""S207 - Oud am Basschor: Zerlegung der Lautheit je Taste (Pruefkoerper mit 16 Hoerorten in 2,06 m und `front` in
0,5 m). Je Taste: Lautheit im Mittel der 16 Orte und an `front`, mit starrem Steg, und je Teilton 1-8 am starren Steg
die Kraft senkrecht zur Decke (Summe der Saiten des Chors, dB, K-bewertet), |p/F| im Leistungsmittel der 16 Orte und
an `front` (dB) und der Anteil an der K-bewerteten Energie (16 Orte).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <ordner mit oud.lvinstbody (17 Hoerorte)> [von=36] [bis=48]
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


def k_db(f):
    return 20 * np.log10(np.abs(sig.sosfreqz(ps.k_bewertung(), worN=np.atleast_1d(f), fs=ps.FS)[1]))[0]


kp = ps.lade_koerper("oud")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
hz = w.real / (2 * np.pi)
ch, mensur, cent = ps.choere("oud")
print("Choere (Taste, je Saite T N, m g/m, EI): " + " | ".join(
    f"{t}: " + ", ".join(f"{s['T']:.1f} N {1000 * s['m']:.2f} g/m {s['EI']:.1e}" for s in saiten) for t, saiten in ch))
print("Moden bis 420 Hz (Hz, Q, Formwert an Chor 1 / Chor 2, |G| front, |G| Mittel der 16):")
for i in np.argsort(kp["f"]):
    if kp["f"][i] > 420:
        break
    print(f"  {kp['f'][i]:6.1f}  Q {1 / kp['eta'][i]:5.1f}  {kp['steg'][i, 0]:+7.3f} / {kp['steg'][i, 1]:+7.3f}  "
          f"{abs(kp['G'][i, 0]):.4f}  {np.sqrt(np.mean(np.abs(kp['G'][i, 1:]) ** 2)):.4f}")
print("\nTaste Chor Bund | LUFS 16 Orte / front | starr 16 / front | Teiltoene 1-8: Hz: Kraft dBK, |p/F| 16 / front dB, Anteil %")
kurven = {}
for taste in range(int(opts.get("von", 36)), int(opts.get("bis", 48)) + 1):
    c = max(i for i, (t, _) in enumerate(ch) if t <= taste)
    saiten = ch[c][1]
    S = len(saiten)
    if c not in kurven:
        kurven = {c: ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(ch[c][0])] * S)}
    Y, H = kurven[c]
    laenge = mensur * 2 ** (-(taste - ch[c][0]) / 12)
    gleich = S == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
    laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else laenge for i in range(S)]
    xp = min(ps.ZUPFORT, 0.5 * laenge)
    alle = list(range(S))
    eins = np.zeros((len(w), 2 * S, 1), dtype=complex)
    eins[:, 0::2, 0] = 1
    x = ps.zeit(ps.ton_mit(saiten, laengen, alle, xp, 0.002, 30.0, Y, H, w)[0])
    sp_starr = ps.ton_mit(saiten, laengen, alle, xp, 0.002, 30.0, np.zeros_like(Y), H, w)[0]
    kraft = ps.ton_mit(saiten, laengen, alle, xp, 0.002, 30.0, np.zeros_like(Y), eins, w)[0][:, 0]
    xs = ps.zeit(sp_starr)
    n16 = 10 * np.log10(x.shape[1] - 1)
    f0 = ps.hz_von(taste)
    teile = []
    for n in range(1, 9):
        m = (hz > n * f0 * 0.97) & (hz < n * f0 * 1.03)
        i = np.where(m)[0][np.argmax(np.abs(kraft[m]))]
        kd = k_db(hz[i])
        e = np.sum(np.abs(sp_starr[m][:, 1:]) ** 2) * 10 ** (kd / 10)
        teile.append((hz[i], 10 * np.log10(np.sum(np.abs(kraft[m]) ** 2)) + kd, 10 * np.log10(np.mean(np.abs(H[i, 0, 1:]) ** 2)),
                      20 * np.log10(np.abs(H[i, 0, 0])), e))
    ges = sum(t[4] for t in teile)
    print(f"{taste:3d} {c + 1} {taste - ch[c][0]:2d} | {ps.momentary_max(x[:, 1:]) - n16:6.1f} / {ps.momentary_max(x[:, 0:1]):6.1f} | "
          f"{ps.momentary_max(xs[:, 1:]) - n16:6.1f} / {ps.momentary_max(xs[:, 0:1]):6.1f} | " + "  ".join(
              f"{t[0]:.0f}: {t[1]:.0f} {t[2]:.0f}/{t[3]:.0f} {100 * t[4] / ges:.0f}%" for t in teile), flush=True)
