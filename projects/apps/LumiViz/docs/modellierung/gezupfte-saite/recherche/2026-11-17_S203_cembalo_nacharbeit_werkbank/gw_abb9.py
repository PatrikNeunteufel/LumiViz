"""Giordano & Winans (Vorabdruck), Abb. 9 (Plektrumkraft ueber der Zeit) aus den PS-Koordinaten lesen.
Achsen aus derselben Seite: y = 113.727 -> 0 N, 298.754 -> 1.0 N, 483.781 -> 2.0 N;
x = 122.146 -> 0 s, 245.497 -> 0.02 s.
Die Datenkurve ist der erste lange moveto/lineto-Zug nach 'setdash'.
"""
import re
import sys

roh = open(sys.argv[1], "rb").read().decode("latin-1")
s = re.split(r"%%Page:", roh)[35]
teil = s.split("[] 0 setdash", 1)[1]
pkt = [(float(a), float(b)) for a, b in re.findall(r"([\d.]+) ([\d.]+) (?:moveto|lineto)", teil.split("stroke", 1)[0])]
y0, y1 = 113.727, 298.754
x0, x1 = 122.146, 245.497
t = [(x - x0) / (x1 - x0) * 0.02 for x, _ in pkt]
f = [(y - y0) / (y1 - y0) * 1.0 for _, y in pkt]
print("Punkte:", len(pkt))
i = max(range(len(f)), key=lambda k: f[k])
print(f"Maximum: {f[i]:.3f} N bei t = {t[i]*1000:.1f} ms")
grund = sorted(f[:20])[10]
print(f"Grundlinie (Median der ersten 20 Punkte): {grund:.3f} N")
ueber = [k for k in range(len(f)) if f[k] - grund > 0.1 * (f[i] - grund)]
print(f"ueber 10 % des Maximums: {t[ueber[0]]*1000:.1f} ... {t[ueber[-1]]*1000:.1f} ms")
# Verlauf grob, alle 2 ms
for ms in range(0, int(t[-1] * 1000) + 1, 2):
    k = min(range(len(t)), key=lambda j: abs(t[j] * 1000 - ms))
    print(f"{ms:3d} ms  {f[k]:.3f} N")
