"""Prüfsummen der geladenen Dateien und Bogenlänge der Becher-Mittellinie (Abb. 3b, eigene Ablesung)."""
import hashlib
import math
import pathlib

EINGANG = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\eingang\2026-11-16_07_alphorn_nachpruefung")

for pdf in sorted(EINGANG.glob("*.pdf")):
    h = hashlib.sha256(pdf.read_bytes()).hexdigest()
    print(f"{h}  {pdf.name}  ({pdf.stat().st_size} Bytes)")

# Mittellinie (x, y) in mm, abgelesen bei 300 dpi
punkte = [(0, 0), (100, 0.5), (200, 8), (300, 28), (350, 53), (400, 92), (450, 158), (492, 232)]
s = sum(math.dist(a, b) for a, b in zip(punkte, punkte[1:]))
print(f"Bogenlaenge Polygon: {s:.1f} mm")
# Empfindlichkeit: y-Werte ab x=300 um +-5 mm verschoben
for d in (-5, 5):
    p2 = [(x, y + (d * (x - 250) / 242 if x > 250 else 0)) for x, y in punkte]
    print(f"  y-Ende {d:+d} mm: {sum(math.dist(a, b) for a, b in zip(p2, p2[1:])):.1f} mm")

# Längenskalierung Fis 3,47 m gleichstufig
for name, halbtoene in [("Es", -3), ("E", -2), ("F", -1), ("Fis", 0), ("G", 1), ("Gis", 2), ("B", 4), ("C", 6)]:
    print(f"{name}: {3.47 * 2 ** (-halbtoene / 12):.3f} m")
print(f"Gis bei 442 statt 440 Hz: Faktor {440/442:.4f} -> {3.09*440/442:.3f} m")
print(f"51,9 Hz gegen Es1 (38,89 Hz): {1200*math.log2(51.9/38.891):.0f} Cent; gegen As1 (51,913 Hz): {1200*math.log2(51.9/51.913):.1f} Cent")
