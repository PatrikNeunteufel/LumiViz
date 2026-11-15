"""S199, 34k-6 — GZ5-1 a nachgerechnet: der Daempfer ueber 5 kHz an Gitarre, Laute, Oud und Harfe.

Aufruf (vom Projekt-Root): python -B <dieses Skript> [<ausgabe.txt>]

Je Instrument an jeder dritten Taste drei Fassungen der Saitenschleife (die Abstrahlung nimmt immer alle Moden):
  Datei            die Moden der Koerperdatei (GZ5 a)
  Datei + Daempfer dazu ueber 5 kHz Re Y = n(f) phi_rms^2 / 4 am Stegpunkt (GZ5-1 a), in der zweiten Richtung mal 0,16
                   (Mittel von tan^2 der Annahme), ohne Kopplung der Richtungen; Saiten eines Chors haengen am selben
                   Punkt und teilen den Daempfer
  alle             alle Moden samt den gezogenen
Gemessen: Momentary-Maximum und Pegel bei 1-2 s gegen den Anfang; groesster Betrag gegen `alle`.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.with_name("2026-11-15_S199_pruefstand_zupf.py"))
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)


def tasten(name):
    """Je Taste (Taste, Saiten, Laengen, Taste des Stegpunkts, Zupfort, Auslenkung, Winkel, Breite, Tempo)."""
    if name == "gitarre":
        for t in range(40, 84, 3):
            s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= t)
            L = ps.MENSUR * 2 ** (-(t - ps.GITARRE[s]["taste"]) / 12)
            yield t, [ps.GITARRE[s]], [L], ps.GITARRE[s]["taste"], min(ps.ZUPFORT, 0.5 * L), 0.002, 30.0, 0.0075, 0.0
    elif name in ("laute", "oud"):
        ch, mensur, cent = ps.choere(name)
        for t in range(ch[0][0], ch[-1][0] + 13, 3):
            c = max(i for i, (k, _) in enumerate(ch) if k <= t)
            saiten = ch[c][1]
            L = mensur * 2 ** (-(t - ch[c][0]) / 12)
            gleich = len(saiten) == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
            laengen = [L * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else L for i in range(len(saiten))]
            yield t, saiten, laengen, ch[c][0], min(ps.ZUPFORT, 0.5 * L), 0.002, 30.0, 0.0075, 0.0
    else:
        for t in range(25, 105, 3):
            a = ps.harfe_taste(t)
            if a:
                yield t, [a[0]], [a[1]], t, a[2], a[3], a[4], a[5], a[6]


if __name__ == "__main__":
    aus = open(sys.argv[1], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 1 else None

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    w = ps.frequenzen()
    f = np.abs(w.real) / (2 * np.pi)
    for name, koerper in (("gitarre", "classical_guitar"), ("laute", "lute"), ("oud", "oud"), ("harfe", "concert_harp")):
        kp = ps.lade_koerper(koerper)
        roh = json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))
        st, datei = roh["statistical"], len(roh["modes"])
        nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
        tan = ps.richtung_tan(kp["f"])
        dichte = np.where(f >= st["fromHz"], st["density"] + st["rise"] * (f - st["fromHz"]), 0.0)
        schreibe(f"{name}: Taste | Lautheit LUFS, Pegel 1-2 s dB | Datei | Datei + Daempfer | alle")
        zeilen = []
        for t, saiten, laengen, steg_taste, xp, d, winkel, breite, tempo in tasten(name):
            punkt = kp["tasten"].index(steg_taste)
            n = len(saiten)
            Yalle, H = ps.koerper_matrix(kp, w, tan, [punkt] * n)
            Yd, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt] * n)
            g = dichte * st["bridgeSpread"][punkt] ** 2 / 4
            Ym = Yd.copy()
            for i in range(n):
                for j in range(n):
                    Ym[:, 2 * i, 2 * j] += g
                    Ym[:, 2 * i + 1, 2 * j + 1] += 0.16 * g
            z = []
            for Y in (Yd, Ym, Yalle):
                x = ps.zeit(ps.ton_mit(saiten, laengen, list(range(n)), xp, d, winkel, Y, H, w, breite, tempo)[0])
                z += [ps.momentary_max(x), ps.rms_db(x, 1, 2) - ps.rms_db(x, ps.VORLAUF, ps.VORLAUF + 0.4)]
            zeilen.append(z)
            schreibe(f"{t:3d} | {z[0]:6.1f} {z[1]:6.1f} | {z[2]:6.1f} {z[3]:6.1f} | {z[4]:6.1f} {z[5]:6.1f}")
        a = np.array(zeilen)
        laut = a[:, 5] > -45                                 # der spaete Pegel nur, wo der Ton dann noch klingt
        schreibe(f"{name}, groesster Betrag gegen `alle`: Datei {np.abs(a[:, 0] - a[:, 4]).max():.2f} LU / "
                 f"{np.abs(a[laut, 1] - a[laut, 5]).max():.2f} dB; Datei + Daempfer {np.abs(a[:, 2] - a[:, 4]).max():.2f} LU / "
                 f"{np.abs(a[laut, 3] - a[laut, 5]).max():.2f} dB (Pegel nur an Tasten ueber -45 dB)")
        schreibe()
