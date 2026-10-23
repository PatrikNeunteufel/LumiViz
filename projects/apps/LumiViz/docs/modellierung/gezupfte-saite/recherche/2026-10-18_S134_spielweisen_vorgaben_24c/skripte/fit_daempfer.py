"""Passt je gedaempfter Note ein Punktdaempfer-Modell an die Abklingraten der Teiltoene an.

Modell (schwache Daempfung, Punkt-Dashpot R an der Stelle xi = x/L vom Steg):
    sigma_k = sigma_k,normal + 2 f1 rho sin^2(k pi xi),   rho = R / Z0
sigma in 1/s (Amplitude), aus der Rate in dB/s: sigma = |rate| / 8.686.
Teiltoene mit T60 < 0.15 s (Rate > 400 dB/s) gelten als an der Messgrenze und bleiben draussen.

    python -I fit_daempfer.py <ds2-ordner>
"""
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from messe_idmt import fenster, lies_noten, lies_wav, miss_note  # noqa: E402

SAITEN = ("E", "A", "D", "G", "B", "E1")
GRENZE = 400.0  # dB/s


def fit(f1, sig_m, sig_n, ks):
    best = None
    for xi in np.arange(0.01, 0.20, 0.0025):
        s2 = np.sin(np.array(ks) * np.pi * xi) ** 2
        d = np.array(sig_m) - np.array(sig_n)
        a = 2 * f1 * s2
        rho = max(0.0, float(np.dot(a, d) / np.dot(a, a)))
        rest = d - rho * a
        # relativer Fehler gewichtet
        err = float(np.sum((rest / (np.abs(d) + 1.0)) ** 2))
        if best is None or err < best[0]:
            best = (err, xi, rho)
    return best


def main():
    ds = Path(sys.argv[1])
    ergebnis = {i: [] for i in range(5)}
    print("| Gitarre | Saite | Stufe | xi (x/L) | x bei 648 mm | rho = R/Z0 | Teiltoene im Fit |")
    print("|---|---|---|---|---|---|---|")
    for g in ("AR", "FS", "LP"):
        for s in SAITEN:
            xp = lies_wav(ds / "audio" / f"{g}_{s}_fret_0-20.wav")
            np_ = lies_noten(ds / "annotation" / f"{g}_{s}_fret_0-20.xml")
            xm = lies_wav(ds / "audio" / f"{g}_{s}_muted5.wav")
            nm = lies_noten(ds / "annotation" / f"{g}_{s}_muted5.xml")
            ip = [i for i, n in enumerate(np_) if n["fret"] == nm[0]["fret"]][0]
            a, e = fenster(np_, ip, len(xp))
            rn = miss_note(xp, a, e, np_[ip]["pitch"])
            sig_n = {t[0]: max(0.0, -t[3] / 8.686) if not np.isnan(t[3]) else 0.0 for t in rn["teil"]}
            for i in range(len(nm)):
                a, e = fenster(nm, i, len(xm))
                r = miss_note(xm, a, e, nm[i]["pitch"])
                if r is None:
                    continue
                ks, sm, sn = [], [], []
                for k, fk, start, rate in r["teil"]:
                    if np.isnan(rate) or -rate > GRENZE or k not in sig_n:
                        continue
                    ks.append(k)
                    sm.append(-rate / 8.686)
                    sn.append(sig_n[k])
                if len(ks) < 4:
                    print(f"| {g} | {s} | M{i+1} | — | — | — | {len(ks)} (zu wenige) |")
                    continue
                # Mensur: die Note liegt am 5. Bund -> schwingende Laenge 648 * 2^(-5/12)
                L = 648 * 2 ** (-nm[i]["fret"] / 12)
                err, xi, rho = fit(r["f0"], sm, sn, ks)
                ergebnis[i].append((xi, rho, xi * L))
                print(f"| {g} | {s} | M{i+1} | {xi:.3f} | {xi * L:.0f} mm | {rho:.3f} | {len(ks)} |")
    print()
    print("| Stufe | n | xi Median (Spanne) | x Median mm (schwingende Laenge 485 mm) | rho Median (Spanne) |")
    print("|---|---|---|---|---|")
    for i in range(5):
        v = np.array(ergebnis[i])
        if len(v) == 0:
            continue
        print(f"| M{i+1} | {len(v)} | {np.median(v[:,0]):.3f} ({v[:,0].min():.3f}..{v[:,0].max():.3f}) | {np.median(v[:,2]):.0f} ({v[:,2].min():.0f}..{v[:,2].max():.0f}) | {np.median(v[:,1]):.3f} ({v[:,1].min():.3f}..{v[:,1].max():.3f}) |")


if __name__ == "__main__":
    main()
