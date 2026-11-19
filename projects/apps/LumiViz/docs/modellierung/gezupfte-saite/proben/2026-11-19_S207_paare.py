"""Die groessten Spruenge (Paare bis 4 Halbtoene) in Spalte 2 einer tsv (MIDI, Leistungsmittel, ...). Aufruf: <tsv> [ab=..]"""
import sys

import numpy as np

opts = dict(a.split("=") for a in sys.argv[2:] if "=" in a)
z = np.array([[float(v) for v in s.split("\t")[:2]] for s in open(sys.argv[1], encoding="utf-8") if s.strip()])
z = z[z[:, 0] >= int(opts.get("ab", 0))]
z = z[np.argsort(z[:, 0])]
p = sorted(((abs(a[1] - b[1]), int(a[0]), int(b[0]), a[1], b[1]) for i, a in enumerate(z) for b in z[i + 1:] if 0 < b[0] - a[0] <= 4), reverse=True)
for d, t1, t2, l1, l2 in p[:8]:
    print(f"{t1}/{t2}: {d:.1f} LU ({l1:.1f} / {l2:.1f})")
