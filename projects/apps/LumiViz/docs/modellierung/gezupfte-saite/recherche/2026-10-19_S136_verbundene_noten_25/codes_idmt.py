"""Zaehlt in der Dateiliste von IDMT-SMT-Guitar V2 die Ordner und die Code-Gruppen der Dateinamen.

    python -I codes_idmt.py <guitar_liste.txt>
"""
import re
import sys
from collections import Counter

ordner = Counter()
codes = Counter()
ds = Counter()
for zeile in open(sys.argv[1], encoding="utf-8"):
    name = zeile.rsplit(" ", 1)[0]
    teile = name.split("/")
    if len(teile) > 3:
        ordner["/".join(teile[1:3])] += 1
    ds["/".join(teile[1:2])] += 1
    m = re.search(r"G(\d\d)-(\d{5})-(\d{4})-(\d{5})\.wav", name)
    if m:
        codes[(teile[2], m.group(3))] += 1
for k, v in sorted(ordner.items()):
    print("ORDNER", k, v)
for k, v in sorted(codes.items()):
    print("CODE", k, v)
for k, v in sorted(ds.items()):
    print("DS", k, v)
