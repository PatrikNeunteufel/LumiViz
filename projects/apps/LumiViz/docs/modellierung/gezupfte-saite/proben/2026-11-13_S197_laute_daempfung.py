"""Daempfung der Laute aus der gemessenen Kurve (Woodhouse 2021 Abb. 1): Halbwertsbreiten der Spitzen und die
Daempfung des Bauplans (alpha = b f + c, Guete der Luft), mit der die gerechnete Kurve - mit Saitenlast - die
Hoehen der vier Spitzen trifft.

    python laute_daempfung.py <kurve.txt> <formen.txt> <werkzeug.log> <x> <y> [bild.png]
"""
import re
import sys

import numpy as np
from scipy.signal import find_peaks

SAITEN = 2.47
ZIELE = [(132.2, -28.0), (277.1, -17.4), (340.6, -19.6), (475.6, -11.8)]


def formen(datei):
    z = open(datei, encoding="utf-8").read().split("\n")
    n = int(z[1].split()[1])
    knoten = np.array([[float(v) for v in s.split()] for s in z[2:2 + n]])
    t = int(z[2 + n].split()[1])
    m = int(z[3 + n + t].split()[1])
    hz, w = [], []
    for k in range(m):
        hz.append(float(z[4 + n + t + 2 * k].split()[1]))
        w.append(np.array([float(v) for v in z[5 + n + t + 2 * k].split()]))
    return knoten, np.array(hz), np.array(w)


def breite(f, d, i):
    """-3-dB-Breite um die Spitze i (linear interpoliert); None, wenn eine Flanke nicht 3 dB faellt."""
    ziel = d[i] - 3.0
    l = i
    while l > 0 and d[l] > ziel:
        if d[l - 1] > d[l] + 0.5 and d[l] < d[i] - 0.5:
            return None
        l -= 1
    r = i
    while r < len(d) - 1 and d[r] > ziel:
        if d[r + 1] > d[r] + 0.5 and d[r] < d[i] - 0.5:
            return None
        r += 1
    if d[l] > ziel or d[r] > ziel:
        return None
    fl = f[l] + (ziel - d[l]) / (d[l + 1] - d[l]) * (f[l + 1] - f[l])
    fr = f[r - 1] + (ziel - d[r - 1]) / (d[r] - d[r - 1]) * (f[r] - f[r - 1])
    return fr - fl


def kurve(hz, phi2, luft, f, b, c, q):
    eta_platte = (b * hz + c) / (2.0 * np.pi * hz)   # wie InstrumentBody: eta = alpha / (2 pi f)
    eta = eta_platte * (1.0 - luft) + luft / q
    om = 2.0 * np.pi * f[:, None]
    omn = 2.0 * np.pi * hz[None, :]
    y = (1j * om / (omn ** 2 * (1.0 + 1j * eta[None, :]) - om ** 2)) @ phi2
    y = y / (1.0 + SAITEN * y)
    return 20.0 * np.log10(np.abs(y))


def hoehen(f, d):
    aus = []
    i, _ = find_peaks(d, prominence=1.5)
    for zf, zd in ZIELE:
        nah = [k for k in i if abs(f[k] / zf - 1.0) <= 0.08]
        if not nah:
            aus.append((np.nan, np.nan, None))
            continue
        k = max(nah, key=lambda k: d[k])
        aus.append((f[k], d[k], breite(f, d, k)))
    return aus


def main():
    mess = np.loadtxt(sys.argv[1])
    mf, md = mess[:, 0], mess[:, 2]
    print("gemessen (Oberkante der Linie):")
    i, _ = find_peaks(md, prominence=3.0)
    for k in i:
        if 100 < mf[k] < 600:
            br = breite(mf, md, k)
            print(f"  {mf[k]:6.1f} Hz {md[k]:6.1f} dB  Breite " + (f"{br:5.1f} Hz  eta {br / mf[k] * 100:4.1f} %" if br else "  -  (nicht frei stehend)"))
    knoten, hz, w = formen(sys.argv[2])
    luft = np.zeros(len(hz))
    for m in re.finditer(r"coupled\s+([\d.]+) Hz\s+air share ([\d.]+)", open(sys.argv[3], encoding="utf-8", errors="replace").read()):
        k = int(np.argmin(np.abs(hz - float(m.group(1)))))
        luft[k] = float(m.group(2))
    x, y = float(sys.argv[4]), float(sys.argv[5])
    p = int(np.argmin((knoten[:, 0] - x) ** 2 + (knoten[:, 1] - y) ** 2))
    phi2 = w[:, p] ** 2
    f = np.arange(70.0, 1000.0, 0.2)
    bestes = None
    for b in (0.0, 0.03, 0.06, 0.1, 0.15, 0.2, 0.3):
        for c in (10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 100.0, 120.0):
            for q in (8.0, 10.0, 12.0, 15.0, 20.0, 30.0):
                h = hoehen(f, kurve(hz, phi2, luft, f, b, c, q))
                if any(np.isnan(v[1]) for v in h):
                    continue
                fehler = [v[1] - z[1] for v, z in zip(h, ZIELE)]
                wert = float(np.sqrt(np.mean(np.square(fehler))))
                if bestes is None or wert < bestes[0]:
                    bestes = (wert, b, c, q, h, fehler)
    wert, b, c, q, h, fehler = bestes
    print(f"beste Daempfung: alpha = {b} f + {c}, Guete der Luft {q}  (Abstand der Hoehen {wert:.1f} dB quadratisch gemittelt)")
    for (zf, zd), v, e in zip(ZIELE, h, fehler):
        print(f"  Ziel {zf:6.1f} Hz {zd:6.1f} dB -> {v[0]:6.1f} Hz ({100 * (v[0] / zf - 1):+.1f} %) {v[1]:6.1f} dB ({e:+.1f} dB)"
              + (f"  Breite {v[2]:.1f} Hz" if v[2] else ""))
    for name, (bb, cc, qq) in {"Oud-Gesetz 0,06 f + 23, Q 30": (0.06, 23.0, 30.0)}.items():
        h2 = hoehen(f, kurve(hz, phi2, luft, f, bb, cc, qq))
        print(f"zum Vergleich {name}: " + "  ".join(f"{v[1] - z[1]:+.1f} dB" for v, z in zip(h2, ZIELE)))
    if len(sys.argv) > 6:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(11, 5.5))
        ax.semilogx(mf, md, "b-", lw=1.6, label="gemessen: Woodhouse 2021 Abb. 1 (Laute), aus dem Bild gelesen")
        ax.semilogx(f, kurve(hz, phi2, luft, f, b, c, q), "r-", lw=1.0,
                    label=f"gerechnet am Messpunkt, mit Saitenlast {SAITEN} N s/m; alpha = {b} f + {c}, Q Luft {q:.0f}")
        ax.set_xlim(70, 1000)
        ax.set_ylim(-70, 0)
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=8)
        ax.set_xlabel("Hz")
        ax.set_ylabel("Stegadmittanz, dB re 1 m/s/N")
        fig.savefig(sys.argv[6], dpi=110)
        print(sys.argv[6])


if __name__ == "__main__":
    main()
