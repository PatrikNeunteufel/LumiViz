"""Verlustfreie Waveguide-Saite mit Punktdaempfer (Widerstand rho = R/Z0) an der Stelle xi vom Steg.

Streuung am Daempfer nach Pakarinen 2005, Gl. (3) mit Z1 = Z2 = 1, Z3 = rho:
    Reflexion r = -rho / (2 + rho), Durchgang t = 2 / (2 + rho).
Misst je rho: Abklingen (dB je Periode des Grundtons der leeren Saite) des Teiltons k=1 und, bei xi=1/2,
k=2 (Knoten am Daempfer); fuer Palm mute (xi klein) die Abklingrate je Teilton.

    python -I sim_daempfer.py
"""
import numpy as np


def simuliere(M, xi, rho, perioden=40, beta=0.2, ausgabe=0.1):
    """M = Abtastwerte je Saitenlaenge (Grundton = fs / 2M). Liefert Auslenkung an 'ausgabe' (Anteil vom Steg)."""
    r = -rho / (2 + rho)
    t = 2 / (2 + rho)
    # rechts laufend (zum Steg hin) und links laufend, Index 0 = Sattel, M = Steg
    x = np.arange(M + 1) / M  # Anteil vom Sattel
    # Dreieck mit Spitze bei beta vom Steg
    p = 1 - beta
    y0 = np.where(x < p, x / p, (1 - x) / (1 - p))
    rechts = y0[:-1] / 2
    links = y0[1:] / 2
    jd = int(round((1 - xi) * M))  # Daempferstelle vom Sattel
    ja = int(round((1 - ausgabe) * M))
    n = perioden * 2 * M
    out = np.empty(n)
    for i in range(n):
        out[i] = rechts[min(ja, M - 1)] + links[min(ja, M - 1)]
        # Endreflexionen
        steg = rechts[-1]
        sattel = links[0]
        rechts = np.roll(rechts, 1)
        links = np.roll(links, -1)
        rechts[0] = -sattel
        links[-1] = -steg
        # Streuung am Daempfer zwischen Zelle jd-1 und jd
        a1 = rechts[jd]      # kommt von links an
        a2 = links[jd - 1]   # kommt von rechts an
        rechts[jd] = t * a1 + r * a2
        links[jd - 1] = r * a1 + t * a2
    return out


def teilton_verlauf(sig, M, k, fs=1.0):
    per = 2 * M
    N = per
    werte = []
    for st in range(0, len(sig) - N, per):
        X = np.fft.rfft(sig[st:st + N])
        werte.append(20 * np.log10(abs(X[k]) + 1e-300))  # Bin k = k-ter Teilton, 1 Periode Rechteck
    return np.array(werte)


def steigung(v, bis=40):
    """dB je Periode ab Periode 1, bis 120 dB gefallen oder 'bis' Perioden."""
    j1 = min(len(v), bis)
    u = np.where(v < v[1] - 120)[0]
    if len(u):
        j1 = min(j1, max(int(u[0]), 3))
    return float(np.polyfit(np.arange(1, j1), v[1:j1], 1)[0])


def main():
    M = 100
    print("## Flageolett: Daempfer bei xi = 1/2 (12. Bund)")
    print("| rho = R/Z0 | k=1 dB je Periode | k=2 dB je Periode | k=3 dB je Periode | k=1 nach 1 / 3 Perioden dB | k=2 Pegel nach 20 Perioden dB (gegen ohne Daempfer) |")
    print("|---|---|---|---|---|---|")
    ref = teilton_verlauf(simuliere(M, 0.5, 0.0, perioden=25), M, 2)
    for rho in (0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 4.0, 10.0, 100.0, 1e6):
        sig = simuliere(M, 0.5, rho, perioden=25)
        z = []
        for k in (1, 2, 3):
            z.append(steigung(teilton_verlauf(sig, M, k), bis=12))
        v1 = teilton_verlauf(sig, M, 1)
        v2 = teilton_verlauf(sig, M, 2)
        print(f"| {rho:g} | {z[0]:.2f} | {z[1]:.2f} | {z[2]:.2f} | {v1[1]-v1[0]:.1f} / {v1[3]-v1[0]:.1f} | {v2[20]-ref[20]:+.2f} |")
    print("\n## Palm mute: Daempfer bei xi = 40/648 (40 mm vom Steg, 648 mm)")
    print("| rho | k=1 | k=2 | k=4 | k=6 | k=8 | k=10 | dB je Periode des Grundtons; Formel k=1 schwach: 8.686*2*rho*sin^2(pi xi) |")
    print("|---|---|---|---|---|---|---|---|")
    xi = 40 / 648
    for rho in (0.01, 0.03, 0.1, 0.3, 1.0):
        sig = simuliere(M, xi, rho, perioden=60, ausgabe=0.3)
        z = []
        for k in (1, 2, 4, 6, 8, 10):
            z.append(steigung(teilton_verlauf(sig, M, k), bis=50))
        formel = -8.686 * 2 * rho * np.sin(np.pi * xi) ** 2
        print(f"| {rho} | " + " | ".join(f"{q:.3f}" for q in z) + f" | {formel:.3f} |")


if __name__ == "__main__":
    main()
