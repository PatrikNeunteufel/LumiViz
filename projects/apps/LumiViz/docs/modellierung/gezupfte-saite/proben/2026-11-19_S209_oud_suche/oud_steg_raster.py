"""S209 - Raster ueber den Steg (Laenge, Breite, Hoehe, Dichte) und die Deckendicke; eine Zeile je Fassung."""
import itertools
import sys

import numpy as np

import oud_kombi as k
import oud_suche as s

FUENF = (101.0, 139.0, 203.0, 280.0, 362.0)


def zeile(e, kopf):
    sp = e["spitzen_ap"]
    lag = []
    for z in FUENF:
        q = min(sp, key=lambda g: abs(g[0] - z))
        lag.append(100 * (q[0] / z - 1))
    gr = max(abs(x) for x in lag)
    return (f"{kopf}\t{gr:5.1f}\t" + " ".join(f"{x:+5.1f}" for x in lag) + f"\t{e['z2']:+6.1f}\t{e['z2p']:+6.1f}\t{e['f_luft']:.0f}\t{e['f_haupt']:.0f}\t{e['z3']:.2f}"
            f"\t{e['luft_db']:+.1f}\t{e['tal_db']:+.1f}\t{e['t2']:+.1f}\t{e['t3']:+.1f}\t" + " ".join(f"{f:.0f}({a:.2f})" for f, a in zip(e["moden"][:7], e["luft"][:7])))


if __name__ == "__main__":
    print("Steg L mm, b mm, h mm, Dichte, Decke mm\tgroesste Lage %\tLagen 101 139 203 280 362 %\tZ2 Raster\tZ2 Punkt\tLuft\tHaupt\tZ3\tLuft dB\tTal dB\tT2\tT3\tModen (Luftanteil)")
    for L, b, h, rho, d in itertools.product((0.13, 0.17), (0.015, 0.020, 0.025), (0.008, 0.010, 0.012), (650.0, 900.0), (0.0014, 0.0015, 0.0016)):
        e = s.rechne(k.fassung(dicke=d, steg=(L, b, h, rho, 10e9)), "raster", zeigen=False)
        if e:
            print(zeile(e, f"{L * 1000:.0f} {b * 1000:.0f} {h * 1000:.0f} {rho:.0f} {d * 1000:.1f} ({1000 * L * b * h * rho:.0f} g)"), flush=True)
