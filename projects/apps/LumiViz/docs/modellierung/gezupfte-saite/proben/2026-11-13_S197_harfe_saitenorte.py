"""Liest die Lage der 46 Saiten am Boden der Salvi „Orchestra“ aus Bell 1988, Abb. 2.9 (PDF-Seite 67).

    pdftoppm -f 67 -l 67 -r 200 -png Bell_1988_….pdf s67
    python harfe_saitenorte.py s67-067.png <ziel.json> [kontrolle.png]

Die Abbildung zeigt den Boden von vorn (Diskant oben), in der Mitte die Deckleiste mit einem Querstrich je Saite
(00, 0, 1 … 44), dazu einen Massstab 10 cm waagrecht und senkrecht. Gesucht: je Saite der Abstand vom Bassende.
Der Massstab wird am Boden selbst genommen (131 cm lang, Abb. 2.4 a) und gegen den gezeichneten geprueft.
"""
import json
import sys

import numpy as np
from PIL import Image
from scipy import ndimage

im = np.asarray(Image.open(sys.argv[1]).convert("L")).astype(int)
dunkel = im < 110
h, w = dunkel.shape
print("Bild", w, "x", h)
# der Boden: die langen waagrechten Linien oben (Diskant) und unten (Bass) des Trapezes
zeilen = dunkel[:, int(0.2 * w):int(0.65 * w)].sum(axis=1)
# Kandidaten: Zeilen mit einem langen waagrechten Lauf
lang = []
for y in range(h):
    lauf, best = 0, 0
    for v in dunkel[y, int(0.2 * w):int(0.65 * w)]:
        lauf = lauf + 1 if v else 0
        best = max(best, lauf)
    lang.append(best)
lang = np.array(lang)
oben_kand = [y for y in range(int(0.07 * h), int(0.15 * h)) if lang[y] > 0.08 * w]
unten_kand = [y for y in range(int(0.85 * h), int(0.97 * h)) if lang[y] > 0.25 * w]
y_oben, y_unten = int(np.median(oben_kand)), int(np.median(unten_kand))
print("Boden: oben (Diskant) y", y_oben, " unten (Bass) y", y_unten, " Laenge px", y_unten - y_oben)
px_je_cm = (y_unten - y_oben) / 131.0
print(f"Massstab aus dem Boden (131 cm): {px_je_cm:.2f} px je cm")
# die Mittellinien der Deckleiste: senkrechte Linien nahe der Bildmitte des Trapezes
spalten = dunkel[y_oben + 20:y_unten - 20].sum(axis=0)
senkrecht = [x for x in range(int(0.3 * w), int(0.5 * w)) if spalten[x] > 0.8 * (y_unten - y_oben - 40)]
gruppen = []
for x in senkrecht:
    if gruppen and x - gruppen[-1][-1] <= 2:
        gruppen[-1].append(x)
    else:
        gruppen.append([x])
mitten = [float(np.mean(g)) for g in gruppen]
print("senkrechte Linien der Deckleiste bei x:", [round(m, 1) for m in mitten])
x_links = min(mitten)
# Querstriche: links der linken Linie ragen sie heraus — dunkle Laeufe in einem Streifen links davon
streifen = dunkel[y_oben + 5:y_unten - 5, int(x_links) - 22:int(x_links) - 5]
profil = streifen.sum(axis=1)
treffer = np.where(profil >= 0.6 * streifen.shape[1])[0] + y_oben + 5
striche = []
for y in treffer:
    if striche and y - striche[-1][-1] <= 2:
        striche[-1].append(y)
    else:
        striche.append([y])
ys = [float(np.mean(s)) for s in striche]
print("Querstriche:", len(ys))
namen = ["00", "0"] + [str(k) for k in range(1, 45)]
orte = [(y_unten - y) / px_je_cm / 100.0 for y in ys]
if len(ys) == len(namen):
    for n, y, o in zip(namen, ys, orte):
        print(f"  Saite {n:>2}: y {y:7.1f} px, {o * 100:6.1f} cm vom Bassende")
    json.dump({"quelle": "Bell 1988, Fig. 2.9 (PDF p. 67), read from the figure at 200 dpi; scale from the length of the soundboard (131 cm, Fig. 2.4 a)",
               "einheit": "m vom Bassende des Bodens, auf der Mittellinie",
               "px_je_cm": round(px_je_cm, 3),
               "saiten": [{"name": n, "ort": round(o, 4)} for n, o in zip(namen, orte)]},
              open(sys.argv[2], "w", encoding="utf-8", newline="\n"), indent=1)
else:
    print("Zahl der Striche passt nicht zu 46 Saiten - y:", [round(y) for y in ys])
if len(sys.argv) > 3:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(8, 14))
    ax.imshow(im, cmap="gray")
    for y in ys:
        ax.plot([x_links - 60, x_links - 25], [y, y], "r-", lw=0.7)
    ax.axhline(y_oben, color="b", lw=0.6)
    ax.axhline(y_unten, color="b", lw=0.6)
    ax.set_xlim(x_links - 300, x_links + 300)
    fig.savefig(sys.argv[3], dpi=80)
