"""S207 - Zerlegung des Sprungs je Taste an der Gitarre (Saite 1): Saitenkraft je Teilton gegen p/F des Koerpers.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> [von=64] [bis=83]
Je Taste: Lautheit (beide Hoerorte, nur links, nur rechts, starrer Steg), und je Teilton 1-8 am starren Steg
die Kraft am Steg (dB, K-bewertet), |p/F| je Hoerort (dB) und der Anteil an der K-bewerteten Energie.
Dazu die Lautheit, wenn |p/F| durch sein Terzmittel ersetzt wird (nur Betrag, Phase 0) - was die Saite allein traegt.
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

opts = dict(a.split("=") for a in sys.argv[1:] if "=" in a)
VON, BIS = int(opts.get("von", 64)), int(opts.get("bis", 83))
WINKEL = 30.0


def k_db(f):
    w, h = sig.sosfreqz(ps.k_bewertung(), worN=np.atleast_1d(f), fs=ps.FS)
    return 20 * np.log10(np.abs(h))


kp = ps.lade_koerper("classical_guitar")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
punkt = kp["tasten"].index(64)
Y, Hz, Hy = ps.koerper_kurven2(kp, punkt, w, tan)
hz = w.real / (2 * np.pi)

# Terzmittel von |Hz| (Energie), als glatte Kurve
glatt = np.empty_like(np.abs(Hz))
for o in range(Hz.shape[1]):
    e = np.abs(Hz[:, o]) ** 2
    c = np.concatenate([[0.0], np.cumsum(e)])
    lo = np.searchsorted(hz, hz / 2 ** (1 / 6))
    hi = np.maximum(np.searchsorted(hz, hz * 2 ** (1 / 6)), lo + 1)
    glatt[:, o] = np.sqrt((c[hi] - c[lo]) / (hi - lo))

sa = ps.GITARRE[5]
print("Taste Bund | LUFS beide / links / rechts | starr beide | p/F geglaettet (starr) | Teiltoene 1-8 am starren Steg: "
      "Kraft dBK, |p/F| links / rechts dB, Anteil %")
zeilen = []
for taste in range(VON, BIS + 1):
    bund = taste - 64
    L = ps.MENSUR * 2 ** (-bund / 12)
    xp = min(ps.ZUPFORT, 0.5 * L)
    null = np.zeros((len(w), 3))
    sp = ps.ton2(sa, L, xp, 0.002, WINKEL, Y, Hz, Hy, w)
    sp_starr = ps.ton2(sa, L, xp, 0.002, WINKEL, null, Hz, Hy, w)
    sp_glatt = ps.ton2(sa, L, xp, 0.002, WINKEL, null, glatt.astype(complex), np.zeros_like(Hy), w)
    # Kraft am starren Steg senkrecht: H = 1
    eins = np.ones((len(w), 1), dtype=complex)
    kraft = ps.ton2(sa, L, xp, 0.002, WINKEL, null, eins, np.zeros_like(eins), w)[:, 0]
    x, xs, xg = ps.zeit(sp), ps.zeit(sp_starr), ps.zeit(sp_glatt)
    lu = ps.momentary_max(x)
    li, re = ps.momentary_max(x[:, 0:1]), ps.momentary_max(x[:, 1:2])
    ls, lg = ps.momentary_max(xs), ps.momentary_max(xg)
    f0 = ps.hz_von(taste)
    teile = []
    for n in range(1, 9):
        m = (hz > n * f0 * 0.97) & (hz < n * f0 * 1.03)
        i = np.where(m)[0][np.argmax(np.abs(kraft[m]))]
        fk = 10 * np.log10(np.sum(np.abs(kraft[m]) ** 2)) + k_db(hz[i])[0]
        e = np.sum(np.abs(sp_starr[m]) ** 2) * 10 ** (k_db(hz[i])[0] / 10)
        teile.append((hz[i], fk, 20 * np.log10(np.abs(Hz[i, 0])), 20 * np.log10(np.abs(Hz[i, 1])), e))
    ges = sum(t[4] for t in teile)
    zeilen.append((taste, lu, li, re, ls, lg))
    print(f"{taste:3d} {bund:2d} | {lu:6.1f} / {li:6.1f} / {re:6.1f} | {ls:6.1f} | {lg:6.1f} | " + "  ".join(
        f"{t[0]:.0f}: {t[1]:.0f} {t[2]:.0f}/{t[3]:.0f} {100 * t[4] / ges:.0f}%" for t in teile), flush=True)

a = np.array(zeilen)
for name, sp_ in (("beide Hoerorte", 1), ("nur links", 2), ("nur rechts", 3), ("starr", 4), ("p/F geglaettet", 5)):
    l = a[:, sp_]
    paare = [(abs(l[i] - l[j]), int(a[i, 0]), int(a[j, 0])) for i in range(len(l)) for j in range(i + 1, len(l)) if a[j, 0] - a[i, 0] <= 4]
    g = max(paare)
    print(f"{name:15s}: Spanne {l.max() - l.min():5.1f} LU, groesster Sprung {g[0]:5.1f} LU ({g[1]}/{g[2]}), mittlerer {np.median([p[0] for p in paare]):.1f} LU")
