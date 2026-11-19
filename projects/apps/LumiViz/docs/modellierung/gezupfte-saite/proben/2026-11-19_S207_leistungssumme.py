"""S207 - Lautheit je Taste mit dem Koerper am Steg wie im Modell, die Abstrahlung aber als Leistungssumme ueber die
Moden (|p/F|^2 = Summe |c_n G_n|^2 je Hoerort, Phase 0) statt als Summe am Hoerort. Naeherung fuer ein Mittel ueber
viele Richtungen, wenn die Moden verschieden abstrahlen. Kennzahlen nach KQ34 a fuer beide Fassungen.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> gitarre | harfe [von=..] [bis=..]
"""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
PS = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py"
spec = importlib.util.spec_from_file_location("ps", PS)
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

opts = dict(a.split("=") for a in sys.argv[1:] if "=" in a)
was = sys.argv[1]
w = ps.frequenzen()


def leistung(kp, punkt, tan=None):
    """sqrt(Summe |c_n G_n|^2) je Hoerort; mit tan auch fuer die Kraft parallel (zweite Rueckgabe)."""
    wn = 2 * np.pi * kp["f"]
    phi = kp["steg"][:, punkt]
    g2 = np.abs(kp["G"]) ** 2
    Hz = np.zeros((len(w), kp["G"].shape[1]))
    Hy = np.zeros_like(Hz)
    for a in range(0, len(w), 4096):
        ws = w[a:a + 4096, None]
        n2 = np.abs(-ws ** 2 / (wn[None, :] ** 2 - ws ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * ws)) ** 2
        Hz[a:a + 4096] = np.sqrt(n2 @ (phi[:, None] ** 2 * g2))
        if tan is not None:
            Hy[a:a + 4096] = np.sqrt(n2 @ ((tan * phi)[:, None] ** 2 * g2))
    return Hz.astype(complex), Hy.astype(complex)


def kennzahlen(name, tasten, l):
    paare = [(abs(l[i] - l[j]), tasten[i], tasten[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= 4]
    g = max(paare)
    print(f"{name:28s}: Spanne {max(l) - min(l):5.1f} LU, groesster Sprung {g[0]:5.1f} LU ({g[1]}/{g[2]}), "
          f"mittlerer {np.median([p[0] for p in paare]):.1f} LU")


zeilen = []
if was == "gitarre":
    kp = ps.lade_koerper("classical_guitar")
    tan = ps.richtung_von(kp)
    kurven = {}
    for taste in range(int(opts.get("von", 40)), int(opts.get("bis", 83)) + 1):
        s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste)
        bund = taste - ps.GITARRE[s]["taste"]
        punkt = kp["tasten"].index(ps.GITARRE[s]["taste"])
        if s not in kurven:
            kurven = {s: ps.koerper_kurven2(kp, punkt, w, tan) + leistung(kp, punkt, tan)}
        Y, Hz, Hy, Lz, Ly = kurven[s]
        L = ps.MENSUR * 2 ** (-bund / 12)
        xp = min(ps.ZUPFORT, 0.5 * L)
        a = ps.momentary_max(ps.zeit(ps.ton2(ps.GITARRE[s], L, xp, 0.002, 30.0, Y, Hz, Hy, w)))
        # Leistungssumme: senkrechte und parallele Kraft getrennt gerechnet und in der Leistung addiert
        fz = ps.ton2(ps.GITARRE[s], L, xp, 0.002, 30.0, Y, Lz, np.zeros_like(Ly), w)
        fy = ps.ton2(ps.GITARRE[s], L, xp, 0.002, 30.0, Y, np.zeros_like(Lz), Ly, w)
        b = 10 * np.log10(10 ** (ps.momentary_max(ps.zeit(fz)) / 10) + 10 ** (ps.momentary_max(ps.zeit(fy)) / 10))
        zeilen.append((taste, a, b))
        print(f"{taste:3d}  Hoerorte {a:6.1f}   Leistungssumme {b:6.1f}   Unterschied {b - a:+5.1f}", flush=True)
else:
    kp = ps.lade_koerper("concert_harp")
    tan = ps.richtung_von(kp)
    for taste in range(int(opts.get("von", 38)), int(opts.get("bis", 104)) + 1):
        sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        punkt = kp["tasten"].index(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [punkt])
        a = ps.momentary_max(ps.zeit(ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0]))
        Lz, Ly = leistung(kp, punkt, tan)
        teile = []
        for r, q in ((0, Lz), (1, Ly)):
            Hq = np.zeros_like(H)
            Hq[:, r, :] = q
            teile.append(ps.momentary_max(ps.zeit(ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, Hq, w, breite, tempo)[0])))
        b = 10 * np.log10(sum(10 ** (v / 10) for v in teile))
        zeilen.append((taste, a, b))
        print(f"{taste:3d}  Hoerort {a:6.1f}   Leistungssumme {b:6.1f}   Unterschied {b - a:+5.1f}", flush=True)

t = [z[0] for z in zeilen]
kennzahlen("Summe am Hoerort (Modell)", t, [z[1] for z in zeilen])
kennzahlen("Leistungssumme ueber Moden", t, [z[2] for z in zeilen])
