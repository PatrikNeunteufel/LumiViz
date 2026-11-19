"""S209 - Familie „Balken am Rand" mit Steg gegen Lesart B (Luftmode 139, Deckenmode 203, dann 280 / 362 Hz).
Eine Zeile je Fassung: Lagen der Spitzen am Antwortpunkt 3 gegen 139 / 203 / 280 / 362, Z2, Z3, Basschor."""
import itertools
import sys

import oud_kombi as k
import oud_suche as s

B4 = (139.0, 203.0, 280.0, 362.0)
STEGE = {"ohne": None, "fichte17x15x10_12g": (0.17, 0.015, 0.010, 460.0, 10e9), "13x20x10_17g": (0.13, 0.020, 0.010, 650.0, 10e9),
         "17x20x10_22g": (0.17, 0.020, 0.010, 650.0, 10e9), "17x25x12_33g": (0.17, 0.025, 0.012, 650.0, 10e9)}


def zeile(e, kopf, ziele=B4):
    sp = e["spitzen_ap"]
    lag = [100 * (min(sp, key=lambda g: abs(g[0] - z))[0] / z - 1) for z in ziele]
    return (f"{kopf}\t{max(abs(x) for x in lag):5.1f}\t" + " ".join(f"{x:+5.1f}" for x in lag) + f"\t{e['z2']:+6.1f}\t{e['z2p']:+6.1f}\t{e['f_luft']:.0f}\t{e['f_haupt']:.0f}\t{e['z3']:.2f}"
            f"\t{e['luft_db']:+.1f}\t{e['tal_db']:+.1f}\t{e['t2']:+.1f}\t{e['t3']:+.1f}\t"
            + " ".join(f"{f:.0f}({a:.2f}|{o[0]:+.2f})" for f, a, o in zip(e["moden"][:6], e["luft"][:6], e["form"][:6]))
            + "\t" + " ".join(f"{p[0]:.0f}:{p[1] / max(q[1] for q in sp):.2f}" for p in sp))


if __name__ == "__main__":
    print("Abstand mm, Steg, Decke mm\tgroesste Lage %\tLagen 139 203 280 362 %\tZ2 Raster\tZ2 Punkt\tLuft\tHaupt\tZ3\tLuft dB\tTal dB\tT2\tT3\tModen (Luftanteil|Steg rel.)\tSpitzen am Antwortpunkt (Raster)")
    for ab, st, d in itertools.product((0.002, 0.003, 0.004), STEGE, (0.0015, 0.00175, 0.002)):
        e = s.rechne(k.fassung(dicke=d, abstand=ab, steg=STEGE[st]), "raster_b", zeigen=False)
        if e:
            print(zeile(e, f"{ab * 1000:.0f} {st} {d * 1000:.2f}"), flush=True)
