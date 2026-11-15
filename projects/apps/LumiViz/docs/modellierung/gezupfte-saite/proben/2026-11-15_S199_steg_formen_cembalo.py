"""S199, 34k-6 — Nachfrage zu GZ5 a: bis wohin muss der Koerper des Cembalos in die Saitenschleife?

Aufruf (vom Projekt-Root): python -B <dieses Skript> [<ausgabe.txt>]
                           python -B <dieses Skript> daempfer [<ausgabe.txt>]

daempfer: statt der gezogenen Moden steht ueber 5 kHz ihr mittlerer Wirkanteil in der Schleife - ein Daempfer
Re Y = n(f) phi_rms^2 / 4 (n Moden je Hz, phi_rms die Streuung am Stegpunkt; der Mittelwert des Realteils einer dichten
Folge von Moden), in der zweiten Richtung mal dem Mittel von tan^2 (0,16), ohne Kopplung der Richtungen. Tasten 64-89.

Die Abstrahlung nimmt immer alle Moden. In der Schleife stehen die Moden unter einer Grenze: 5 kHz (= die Moden der
Koerperdatei, GZ5 a), 7,5 / 10 / 15 kHz (dazu die gezogenen Moden des statistischen Teils bis dorthin) und alle (20 kHz).
Je Fassung an jeder fuenften Taste: Momentary-Maximum und Pegel bei 1-2 s gegen den Anfang, als Unterschied zu `alle`.
Cembalo wie im Pruefstand (Taskin 1769, ein 8-Fuss, Abloesekraft 2 N).
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

GRENZEN = [("bis 5 kHz (Datei)", 5000.0), ("bis 7,5 kHz", 7500.0), ("bis 10 kHz", 10000.0), ("bis 15 kHz", 15000.0), ("alle", 1e9)]

def daempfer(aus):
    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    kp = ps.lade_koerper("harpsichord")
    st = json.loads((ps.KOERPER / "harpsichord.lvinstbody").read_text(encoding="utf-8"))["statistical"]
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    f = np.abs(w.real) / (2 * np.pi)
    dichte = np.where(f >= st["fromHz"], st["density"] + st["rise"] * (f - st["fromHz"]), 0.0)
    m = kp["f"] < 5000.0
    teil = {"f": kp["f"][m], "eta": kp["eta"][m], "steg": kp["steg"][m], "G": kp["G"][m]}
    schreibe("Taste | Lautheit LUFS, Pegel 1-2 s gegen den Anfang dB | Datei | Datei + Daempfer | alle")
    zeilen = []
    for taste in range(64, 90, 5):
        sa, laenge, xp, d, winkel, breite, tempo = ps.cembalo_taste(taste)
        punkt = kp["tasten"].index(taste)
        Yalle, H = ps.koerper_matrix(kp, w, tan, [punkt])
        Yd, _ = ps.koerper_matrix(teil, w, tan[m], [punkt])
        g = dichte * st["bridgeSpread"][punkt] ** 2 / 4
        Ym = Yd.copy()
        Ym[:, 0, 0] += g
        Ym[:, 1, 1] += 0.16 * g
        z = []
        for Y in (Yd, Ym, Yalle):
            x = ps.zeit(ps.ton_mit([sa], [laenge], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
            z += [ps.momentary_max(x), ps.rms_db(x, 1, 2) - ps.rms_db(x, ps.VORLAUF, ps.VORLAUF + 0.4)]
        zeilen.append(z)
        schreibe(f"{taste:3d} | {z[0]:6.1f} {z[1]:6.1f} | {z[2]:6.1f} {z[3]:6.1f} | {z[4]:6.1f} {z[5]:6.1f}")
    a = np.array(zeilen)
    schreibe(f"gegen `alle`, groesster Betrag: Datei {np.abs(a[:, 0] - a[:, 4]).max():.2f} LU / {np.abs(a[:, 1] - a[:, 5]).max():.2f} dB; "
             f"Datei + Daempfer {np.abs(a[:, 2] - a[:, 4]).max():.2f} LU / {np.abs(a[:, 3] - a[:, 5]).max():.2f} dB")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "daempfer":
        daempfer(open(sys.argv[2], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 2 else None)
        sys.exit(0)
    aus = open(sys.argv[1], "w", encoding="utf-8", newline="\n") if len(sys.argv) > 1 else None

    def schreibe(z=""):
        print(z, flush=True)
        if aus:
            aus.write(z + "\n")

    kp = ps.lade_koerper("harpsichord")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    werte = {name: [] for name, _ in GRENZEN}
    zahl = {name: int((kp["f"] < g).sum()) for name, g in GRENZEN}
    schreibe("Taste | je Fassung: Lautheit LUFS, Pegel 1-2 s gegen den Anfang dB | " + " | ".join(n for n, _ in GRENZEN))
    for taste in range(29, 90, 5):
        sa, laenge, xp, d, winkel, breite, tempo = ps.cembalo_taste(taste)
        punkt = kp["tasten"].index(taste)
        _, H = ps.koerper_matrix(kp, w, tan, [punkt])
        zeile = f"{taste:3d} |"
        for name, g in GRENZEN:
            m = kp["f"] < g
            teil = {"f": kp["f"][m], "eta": kp["eta"][m], "steg": kp["steg"][m], "G": kp["G"][m]}
            Y, _ = ps.koerper_matrix(teil, w, tan[m], [punkt])
            x = ps.zeit(ps.ton_mit([sa], [laenge], 0, xp, d, winkel, Y, H, w, breite, tempo)[0])
            werte[name].append((ps.momentary_max(x), ps.rms_db(x, 1, 2) - ps.rms_db(x, ps.VORLAUF, ps.VORLAUF + 0.4)))
            zeile += f" {werte[name][-1][0]:6.1f} {werte[name][-1][1]:6.1f} |"
        schreibe(zeile)
    schreibe()
    schreibe("Fassung            Moden in der Schleife | Lautheit gegen `alle`: Median / groesster Betrag (LU) | Pegel 1-2 s: Median / groesster Betrag (dB)")
    bezug = np.array(werte["alle"])
    for name, _ in GRENZEN:
        v = np.array(werte[name])
        dl, dp = np.abs(v[:, 0] - bezug[:, 0]), np.abs(v[:, 1] - bezug[:, 1])
        schreibe(f"{name:18s} {zahl[name]:5d} | {np.median(dl):5.2f} / {dl.max():5.2f} | {np.median(dp):5.2f} / {dp.max():5.2f}")
