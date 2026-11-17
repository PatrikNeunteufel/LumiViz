"""Findet die Masszahlen am Steg der Rauwolf-Zeichnung (Lage der Woerter) und schneidet den Bereich als Bild aus."""
import re
import subprocess
import sys

PDF = r"C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\gezupfte-saite\SantaMariaBouquet_2020_LuteRauwolf1598_CL93.pdf"
ZIEL = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_15_oud_laute_grundlagen\seiten"
AUS = ZIEL + r"\rauwolf_bbox.html"
r = subprocess.run(["pdftotext", "-bbox", PDF, AUS], capture_output=True, text=True)
print(r.returncode, r.stderr[:300])
html = open(AUS, encoding="utf-8", errors="replace").read()
print(len(html))
woerter = re.findall(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', html)
ziele = {"139", "177", "122.5", "89.0", "12", "8", "10", "7", "6"}
for x0, y0, x1, y1, w in woerter:
    if w in {"139", "177", "122.5", "89.0"}:
        print(w, x0, y0)
# Umgebung des Wortes 177 ausgeben
anker = [(float(a), float(b)) for a, b, _, _, w in woerter if w == "177"]
if anker:
    ax, ay = anker[0]
    for x0, y0, x1, y1, w in woerter:
        if abs(float(x0) - ax) < 250 and abs(float(y0) - ay) < 200:
            print("  nah:", w, round(float(x0) - ax), round(float(y0) - ay))
    dpi = 100
    s = dpi / 72.0
    x, y = int((ax - 260) * s), int((ay - 220) * s)
    subprocess.run(["pdftoppm", "-r", str(dpi), "-x", str(x), "-y", str(y), "-W", str(int(520 * s)), "-H", str(int(440 * s)),
                    "-png", "-singlefile", PDF, ZIEL + r"\rauwolf_steg"], check=True)
    print("Bild:", ZIEL + r"\rauwolf_steg.png")
