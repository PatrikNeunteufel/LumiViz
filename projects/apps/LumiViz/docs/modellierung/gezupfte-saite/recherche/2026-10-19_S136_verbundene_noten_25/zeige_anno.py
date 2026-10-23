"""Zeigt Noten der dataset2-Annotationen als Tabelle; Filter auf Dateinamen und Ausdrucksstil.

    python -I zeige_anno.py <annotation-ordner> <dateimuster> [stil,stil]
"""
import sys
import xml.etree.ElementTree as ET
from collections import Counter
from pathlib import Path

ordner, muster = Path(sys.argv[1]), sys.argv[2]
stile = set(sys.argv[3].split(",")) if len(sys.argv) > 3 else None
zaehl = Counter()
for p in sorted(ordner.glob(muster)):
    w = ET.parse(p).getroot()
    ev = []
    for e in w.iter("event"):
        d = {c.tag: (c.text or "").strip() for c in e}
        ev.append(d)
        zaehl[(d.get("expressionStyle"), d.get("excitationStyle"))] += 1
    ev.sort(key=lambda d: float(d["onsetSec"]))
    zeigen = [d for d in ev if stile is None or d.get("expressionStyle") in stile]
    if not zeigen:
        continue
    print("==", p.name)
    for d in ev:
        mark = "*" if d in zeigen else " "
        print(mark, d["onsetSec"], d.get("offsetSec"), "p", d["pitch"], "s", d.get("stringNumber"),
              "f", d.get("fretNumber"), d.get("excitationStyle"), d.get("expressionStyle"),
              "mf", d.get("modulationFrequency"), "mr", d.get("modulationFrequencyRange"))
print(zaehl)
