"""Vergleicht zwei Bezugsdateien des Pruefstands je Fall: Teiltoene (Cent, Pegel frueh / spaet, Abfall), Kraft, Hoerort."""
import math
import sys


def lies(datei):
    faelle, akt = {}, None
    for z in open(datei, encoding="utf-8"):
        t = z.split()
        if not t or t[0].startswith("#"):
            continue
        if t[0] == "fall":
            akt = faelle.setdefault(t[1], {"teilton": []})
        elif t[0] == "teilton":
            akt["teilton"].append(tuple(float(x) for x in t[2:5]))
        elif t[0] in ("kraft", "hoerort"):
            akt[t[0]] = [float(x) for x in t[1:]]
    return faelle


a, b = lies(sys.argv[1]), lies(sys.argv[2])
print("Fall            | Cent max | frueh max dB | spaet max dB | Abfall max dB | Kraft 0-0,1 / 0-1 / 1-3 dB | LUFS | 1-2 s dB")
for n in a:
    x, y = a[n], b[n]
    cent = max(abs(1200 * math.log2(q[0] / p[0])) for p, q in zip(x["teilton"], y["teilton"]))
    fr = max(abs(q[1] - p[1]) for p, q in zip(x["teilton"], y["teilton"]))
    sp = max(abs(q[2] - p[2]) for p, q in zip(x["teilton"], y["teilton"]))
    ab = max(abs((q[1] - q[2]) - (p[1] - p[2])) for p, q in zip(x["teilton"], y["teilton"]))
    k = [q - p for p, q in zip(x["kraft"], y["kraft"])]
    h = [q - p for p, q in zip(x["hoerort"], y["hoerort"])]
    print(f"{n:15s} | {cent:8.3f} | {fr:12.2f} | {sp:12.2f} | {ab:13.2f} | {k[0]:+6.2f} / {k[1]:+6.2f} / {k[2]:+6.2f} | {h[0]:+5.2f} | {h[1]:+6.2f}")
