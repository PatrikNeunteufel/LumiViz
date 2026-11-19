"""Schritt 1 der Ablesung von Değirmenli 2023, Abb. 3 (S. 531): Lage von Rahmen und Gitterlinien in Pixeln."""
import sys
from PIL import Image
pfad = sys.argv[1]
im = Image.open(pfad).convert("RGB")
w, h = im.size
px = im.load()
print("Größe", w, h)
def grau(c):
    r, g, b = c
    return abs(r - g) < 12 and abs(g - b) < 12 and 150 < r < 235
def dunkel(c):
    return sum(c) < 200
spalten = []
for x in range(w):
    n = sum(1 for y in range(h) if grau(px[x, y]))
    d = sum(1 for y in range(h) if dunkel(px[x, y]))
    if n > h * 0.35 or d > h * 0.6:
        spalten.append((x, n, d))
print("Spalten (x, grau, dunkel):", spalten)
zeilen = []
for y in range(h):
    n = sum(1 for x in range(w) if grau(px[x, y]))
    d = sum(1 for x in range(w) if dunkel(px[x, y]))
    if n > w * 0.35 or d > w * 0.6:
        zeilen.append((y, n, d))
print("Zeilen (y, grau, dunkel):", zeilen)
