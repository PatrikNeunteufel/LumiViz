"""S199, 34k-6 — in welcher Form kommt der Koerper in die Saitenschleife? (KQ27 a; Vorlage GZ5) — Gitarre.

Aufruf (vom Projekt-Root): python -B <dieses Skript> [<ausgabe.txt>]

Die Abstrahlung nimmt in jeder Fassung alle Moden (p = F_Steg * H, wie die Faltung beim Fluegel). Verschieden ist nur,
welche Moden die Saite am Steg spuert (die Admittanz in der Schleife):

  alle      alle Moden samt statistischem Teil (236)
  bis f     nur die Moden unter f (5 kHz, 2 kHz, 1 kHz, 500 Hz)
  bis f + R dazu der Rest der weggelassenen Moden als Feder: Y_Rest = i w Summe phi_n^2 d d^T / w_n^2 (ihr Grenzwert
            weit unter der Resonanz) — ein Glied mehr in der Schleife
  starr     keine (nur die Verluste der Saite)

Je Fassung an jeder zweiten Taste (2 mm bei 16 cm, 30 Grad, Stufe 0): Momentary-Maximum und Pegel bei 1-2 s gegen den
Anfang, als Unterschied zu `alle`; dazu Spanne und groesster Sprung zwischen den gerechneten Nachbarn (zwei Halbtoene).
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.with_name("2026-11-15_S199_pruefstand_zupf.py"))
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

FASSUNGEN = [("alle", None, False), ("bis 5 kHz", 5000.0, False), ("bis 2 kHz", 2000.0, False), ("bis 2 kHz + R", 2000.0, True),
             ("bis 1 kHz", 1000.0, False), ("bis 1 kHz + R", 1000.0, True), ("bis 500 Hz", 500.0, False),
             ("bis 500 Hz + R", 500.0, True), ("starr", 0.0, False)]


def admittanz(kp, punkt, w, tan, grenze, rest):
    if grenze is None:
        return ps.koerper_kurven2(kp, punkt, w, tan)[0], len(kp["f"])
    unten = kp["f"] < grenze
    teil = {"f": kp["f"][unten], "eta": kp["eta"][unten], "steg": kp["steg"][unten], "G": kp["G"][unten]}
    Y = ps.koerper_kurven2(teil, punkt, w, tan[unten])[0] if unten.any() else np.zeros((len(w), 3), dtype=complex)
    if rest:
        oben = ~unten
        phi2 = kp["steg"][oben, punkt] ** 2 / (2 * np.pi * kp["f"][oben]) ** 2
        t = tan[oben]
        Y = Y + 1j * w[:, None] * np.array([phi2.sum(), (t * phi2).sum(), (t * t * phi2).sum()])[None, :]
    return Y, int(unten.sum())


if __name__ == "__main__":
    aus = open(sys.argv[1], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 1 else None

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    kp = ps.lade_koerper("classical_guitar")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    werte = {name: [] for name, _, _ in FASSUNGEN}
    moden = {}
    kurven = {}
    for taste in range(40, 84, 2):
        s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste)
        bund = taste - ps.GITARRE[s]["taste"]
        if s not in kurven:
            _, Hz, Hy = ps.koerper_kurven2(kp, s, w, tan)
            kurven = {s: (Hz, Hy, {})}                      # nur eine Saite im Speicher
            for name, grenze, rest in FASSUNGEN:
                kurven[s][2][name], moden[name] = admittanz(kp, s, w, tan, grenze, rest)
        Hz, Hy, Ys = kurven[s]
        laenge = ps.MENSUR * 2 ** (-bund / 12)
        zeile = f"{taste:3d} {6 - s} {bund:2d} |"
        for name, _, _ in FASSUNGEN:
            x = ps.zeit(ps.ton2(ps.GITARRE[s], laenge, min(ps.ZUPFORT, 0.5 * laenge), 0.002, 30.0, Ys[name], Hz, Hy, w))
            werte[name].append((ps.momentary_max(x), ps.rms_db(x, 1, 2) - ps.rms_db(x, ps.VORLAUF, ps.VORLAUF + 0.4)))
            zeile += f" {werte[name][-1][0]:6.1f} {werte[name][-1][1]:6.1f} |"
        schreibe(zeile)
    schreibe()
    schreibe("Fassung          Moden in der Schleife | Lautheit gegen `alle`: Median / groesster Betrag (LU) | Pegel 1-2 s gegen `alle`: "
             "Median / groesster Betrag (dB) | Spanne, groesster Sprung (LU)")
    bezug = np.array(werte["alle"])
    for name, _, _ in FASSUNGEN:
        v = np.array(werte[name])
        dl, dp = v[:, 0] - bezug[:, 0], v[:, 1] - bezug[:, 1]
        schreibe(f"{name:16s} {moden[name]:4d} | {np.median(np.abs(dl)):5.2f} / {np.abs(dl).max():5.2f} | {np.median(np.abs(dp)):5.2f} / "
                 f"{np.abs(dp).max():5.2f} | {v[:, 0].max() - v[:, 0].min():5.1f}, {np.abs(np.diff(v[:, 0])).max():5.1f}")
