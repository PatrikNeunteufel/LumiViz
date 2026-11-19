"""S207 - Zerlegung des Sprungs je Taste an der Harfe: Saitenkraft je Teilton gegen p/F des Koerpers am Stegpunkt der Taste.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> [von=72] [bis=84]
"""
import importlib.util
import json
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
VON, BIS = int(opts.get("von", 72)), int(opts.get("bis", 84))


def k_db(f):
    return 20 * np.log10(np.abs(sig.sosfreqz(ps.k_bewertung(), worN=np.atleast_1d(f), fs=ps.FS)[1]))[0]


roh = json.loads((ps.KOERPER / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
print("Hoerorte:", roh["listeningPoints"])
kp = ps.lade_koerper("concert_harp")
tan = ps.richtung_von(kp)
w = ps.frequenzen()
hz = w.real / (2 * np.pi)
wn = 2 * np.pi * kp["f"]
no = kp["G"].shape[1]
print(f"{len(kp['f'])} Moden, {no} Hoerorte")
print("Taste | Laenge, Zug | LUFS alle Orte / je Ort | starr | p/F Terzmittel (starr) | Teiltoene 1-4 am starren Steg: Hz: Kraft dBK, "
      "|p/F| je Ort dB, Anteil % | staerkste Moden am Grundton je Ort")
zeilen = []
for taste in range(VON, BIS + 1):
    sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
    punkt = kp["tasten"].index(taste)
    Y, H = ps.koerper_matrix(kp, w, tan, [punkt])
    null = np.zeros_like(Y)
    glatt = np.zeros_like(H)
    for o in range(no):
        e = np.abs(H[:, 0, o]) ** 2
        c = np.concatenate([[0.0], np.cumsum(e)])
        lo = np.searchsorted(hz, hz / 2 ** (1 / 6))
        hi = np.maximum(np.searchsorted(hz, hz * 2 ** (1 / 6)), lo + 1)
        glatt[:, 0, o] = np.sqrt((c[hi] - c[lo]) / (hi - lo))
    eins = np.zeros((len(w), 2, 1), dtype=complex)
    eins[:, 0, 0] = 1
    sp = ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0]
    sp_starr = ps.ton_mit([sa], [L], 0, xp, d, winkel, null, H, w, breite, tempo)[0]
    sp_glatt = ps.ton_mit([sa], [L], 0, xp, d, winkel, null, glatt, w, breite, tempo)[0]
    kraft = ps.ton_mit([sa], [L], 0, xp, d, winkel, null, eins, w, breite, tempo)[0][:, 0]
    x = ps.zeit(sp)
    lu = ps.momentary_max(x)
    je = [ps.momentary_max(x[:, o:o + 1]) for o in range(no)]
    ls, lg = ps.momentary_max(ps.zeit(sp_starr)), ps.momentary_max(ps.zeit(sp_glatt))
    f0 = ps.hz_von(taste)
    teile = []
    for n in range(1, 5):
        m = (hz > n * f0 * 0.97) & (hz < n * f0 * 1.03)
        i = np.where(m)[0][np.argmax(np.abs(kraft[m]))]
        kd = k_db(hz[i])
        e = np.sum(np.abs(sp_starr[m]) ** 2) * 10 ** (kd / 10)
        teile.append((hz[i], 10 * np.log10(np.sum(np.abs(kraft[m]) ** 2)) + kd, [20 * np.log10(np.abs(H[i, 0, o])) for o in range(no)], e))
    ges = sum(t[3] for t in teile)
    wf = 2 * np.pi * f0
    c = -wf * wf / (wn ** 2 - wf * wf + 1j * kp["eta"] * wn * wf) * kp["steg"][:, punkt]
    moden = []
    for o in range(no):
        b = c * kp["G"][:, o]
        j = np.argsort(-np.abs(b))[:3]
        moden.append(f"{20 * np.log10(abs(b.sum())):+.0f} [" + ", ".join(
            f"{kp['f'][q]:.0f}: {20 * np.log10(abs(b[q])):+.0f} {np.degrees(np.angle(b[q])):+.0f}" for q in j) + "]")
    zeilen.append([taste, lu, ls, lg] + je)
    print(f"{taste:3d} | {L:.3f} {sa['T']:5.1f} | {lu:6.1f} / " + " ".join(f"{v:6.1f}" for v in je) + f" | {ls:6.1f} | {lg:6.1f} | " + "  ".join(
        f"{t[0]:.0f}: {t[1]:.0f} " + "/".join(f"{v:.0f}" for v in t[2]) + f" {100 * t[3] / ges:.0f}%" for t in teile)
          + " | " + "  ".join(moden), flush=True)

a = np.array(zeilen)
for name, sp_ in [("alle Hoerorte", 1), ("starr", 2), ("p/F Terzmittel", 3)] + [(f"nur Ort {o}", 4 + o) for o in range(no)]:
    l = a[:, sp_]
    paare = [(abs(l[i] - l[j]), int(a[i, 0]), int(a[j, 0])) for i in range(len(l)) for j in range(i + 1, len(l)) if a[j, 0] - a[i, 0] <= 4]
    g = max(paare)
    print(f"{name:15s}: Spanne {l.max() - l.min():5.1f} LU, groesster Sprung {g[0]:5.1f} LU ({g[1]}/{g[2]}), mittlerer {np.median([p[0] for p in paare]):.1f} LU")
