"""Liest die Geometrie des Dulcken-Bodens aus Abb. 2 von Bader, Plath & Kontopidis 2025 (arXiv 2511.09037).

    python dulcken_geometrie.py <abb2.png> <ziel.json> [kontrolle.png]

Abb. 2 ist die Eingabe des FDTD-Modells der Autoren: Boden (farbig), Stegstifte 8' und 4' (weiss), Anhangstifte
(schwarz; die des 8' liegen ausserhalb des Bodens an der Wand), Anhangleiste des 4' (kraeftiges Orange),
Querleiste und vier Rippen (grau).

Massstab: die Stufen des Umrisses sind 14 px breit (1 cm im Modell der Autoren) -> 14 px je cm. Damit ist der Boden
166,4 x 79,7 cm; das Museum nennt fuer das Instrument 2212 x 847 mm (Sammlung online MK&G, 2000.526), in Abb. 1
nimmt der Boden 93,6 % der Breite des Instruments ein -> 79,3 cm. (Der Text der Arbeit nennt ein Gitter von
72 x 178 Punkten; das passt nicht zum Bild.)

Koordinaten der Ausgabe: X laengs vom Schwanz (im Bild links) in m, Y quer von der geraden Wand (im Bild unten) in m.
"""
import json
import sys

import numpy as np
from PIL import Image
from scipy import ndimage

PX = 14.0   # px je cm
X0, Y0 = 92.0, 1191.0   # Bildpunkt der Ecke Schwanz / gerade Wand (linke untere Ecke des Bodens)


def nach_m(x, y):
    return (x - X0) / PX / 100.0, (Y0 - y) / PX / 100.0


def flecken(maske, klein=4, gross=400):
    lab, n = ndimage.label(maske)
    aus = []
    for k, sl in enumerate(ndimage.find_objects(lab), 1):
        teil = lab[sl] == k
        f = int(teil.sum())
        if klein <= f <= gross:
            cy, cx = ndimage.center_of_mass(teil)
            aus.append((cx + sl[1].start, cy + sl[0].start))
    return aus


def reihe(punkte, start, max_schritt=75.0):
    rest = [tuple(p) for p in punkte]
    akt = min(rest, key=lambda p: (p[0] - start[0]) ** 2 + (p[1] - start[1]) ** 2)
    rest.remove(akt)
    aus = [akt]
    while rest:
        n = min(rest, key=lambda p: (p[0] - akt[0]) ** 2 + (p[1] - akt[1]) ** 2)
        if np.hypot(n[0] - akt[0], n[1] - akt[1]) > max_schritt:
            break
        if len(aus) >= 3:
            v = np.array(aus[-1]) - np.array(aus[-3])
            u = np.array(n) - np.array(akt)
            if np.dot(u, v) / (np.linalg.norm(u) * np.linalg.norm(v) + 1e-9) < 0.5:
                break
        rest.remove(n)
        aus.append(n)
        akt = n
    return aus, rest


def vereinfache(p, tol):
    """Douglas-Peucker auf einer offenen Linie."""
    p = np.asarray(p, dtype=float)
    if len(p) < 3:
        return p.tolist()
    a, b = p[0], p[-1]
    d = b - a
    n = np.hypot(*d)
    ab = np.abs((p[:, 0] - a[0]) * d[1] - (p[:, 1] - a[1]) * d[0]) / (n + 1e-12)
    k = int(np.argmax(ab))
    if ab[k] <= tol:
        return [a.tolist(), b.tolist()]
    return vereinfache(p[:k + 1], tol)[:-1] + vereinfache(p[k:], tol)


def main():
    im = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(float)
    r, g, b = im[..., 0], im[..., 1], im[..., 2]
    weiss = (r > 240) & (g > 240) & (b > 240)
    schwarz = (r < 60) & (g < 60) & (b < 60)
    grau = (abs(r - g) < 14) & (abs(g - b) < 14) & (r > 90) & (r < 180)
    farbig = ~weiss & ~schwarz & ~grau
    boden = ndimage.binary_fill_holes(ndimage.binary_closing(farbig | grau, iterations=3))
    lab, n = ndimage.label(boden)
    boden = lab == (np.argmax(ndimage.sum(boden, lab, range(1, n + 1))) + 1)
    ys, xs = np.where(boden)
    x_min, x_max, y_min, y_max = xs.min(), xs.max(), ys.min(), ys.max()
    print(f"Boden px: x {x_min}..{x_max}, y {y_min}..{y_max} -> {(x_max - x_min + 1) / PX:.1f} x {(y_max - y_min + 1) / PX:.1f} cm")
    flaeche = boden.sum() / PX ** 2 / 1e4
    print(f"Flaeche {flaeche:.4f} m2")

    # Umriss: je Spalte die Oberkante (gebogene Wand und Schwanz); die Treppe des Modells durch ihre Mitte geglaettet
    spalten = np.arange(x_min, x_max + 1)
    ober = np.array([np.where(boden[:, x])[0].min() for x in spalten], dtype=float)
    glatt = ndimage.uniform_filter1d(ober, size=29, mode="nearest")
    linie = vereinfache(np.column_stack([spalten, glatt]), 3.0)       # 3 px = 2 mm
    oben = [nach_m(x, y) for x, y in linie]
    # geschlossen: unten links -> unten rechts -> rechts hinauf -> Oberkante zurueck nach links
    umriss = [(0.0, 0.0), nach_m(x_max, Y0), nach_m(x_max, linie[-1][1])] + oben[::-1][1:]
    # der erste Punkt der Oberkante liegt am Schwanz ueber der Ecke: die Kante dazwischen ist der Schwanz
    print("Umriss:", len(umriss), "Punkte")

    stifte_w = flecken(weiss & ndimage.binary_dilation(boden, iterations=6))
    stifte_s = flecken(schwarz)
    steg8, rest = reihe(stifte_w, (300, 1110))
    steg4, rest_w = reihe(rest, (1110, 1100))
    anh4, rest = reihe(stifte_s, (805, 1088))
    anh8, rest_s = reihe(rest, (65, 1108))
    print(f"Steg 8' {len(steg8)} Flecken, Steg 4' {len(steg4)}, Anhang 4' {len(anh4)}, Anhang 8' {len(anh8)}; uebrig {len(rest_w)} / {len(rest_s)}")
    for name, rr in (("Steg 8'", steg8), ("Steg 4'", steg4), ("Anhang 4'", anh4)):
        yy = np.array([p[1] for p in rr])
        print(f"   {name}: quer von {(Y0 - yy[0]) / PX:.1f} bis {(Y0 - yy[-1]) / PX:.1f} cm; Abstand quer je Stift im Mittel {(yy[0] - yy[-1]) / (len(rr) - 1) / PX * 10:.2f} mm")

    # Anhangleiste des 4': die Pixel, die roeter sind als die Dickenskala (R hoch, G unter 132)
    leiste = farbig & (r > 232) & (g < 132)
    leiste = ndimage.binary_opening(leiste, iterations=2)
    lab, n = ndimage.label(leiste)
    leiste = lab == (np.argmax(ndimage.sum(leiste, lab, range(1, n + 1))) + 1)
    ly, lx = np.where(leiste)
    # Mittellinie je Spalte, Breite senkrecht zur Linie aus Flaeche durch Laenge
    lsp = np.arange(lx.min(), lx.max() + 1)
    mitte = np.array([np.where(leiste[:, x])[0].mean() if leiste[:, x].any() else np.nan for x in lsp])
    gut = np.isfinite(mitte)
    lm = vereinfache(np.column_stack([lsp[gut], ndimage.uniform_filter1d(mitte[gut], size=41, mode="nearest")]), 4.0)
    laenge = sum(np.hypot(lm[k + 1][0] - lm[k][0], lm[k + 1][1] - lm[k][1]) for k in range(len(lm) - 1))
    breite = leiste.sum() / laenge
    print(f"Anhangleiste 4': Laenge {laenge / PX:.1f} cm, mittlere Breite {breite / PX:.2f} cm, von x {lx.min()} bis {lx.max()}")

    # graue Leisten: Querleiste (schraeg) und vier Rippen (fast senkrecht im Bild)
    gy, gx = np.where(grau & ndimage.binary_dilation(boden, iterations=3))
    # Rippen: Spalten mit vielen grauen Pixeln
    unten = grau[int(Y0) - 60:int(Y0) - 20]
    spalten_grau = np.where(unten.sum(axis=0) > 30)[0]
    gruppen = []
    for x in spalten_grau:
        if gruppen and x - gruppen[-1][-1] <= 3:
            gruppen[-1].append(x)
        else:
            gruppen.append([x])
    print("graue Laeufe nahe der geraden Wand (x px, Breite px):", [(round(float(np.mean(gr)), 1), len(gr)) for gr in gruppen])
    # Querleiste: graue Pixel links der ersten Rippe -> Gerade
    erste = min(np.mean(gr) for gr in gruppen if np.mean(gr) > 1500)
    m = gx < erste - 30
    k1, k0 = np.polyfit(gx[m], gy[m], 1)
    print(f"Querleiste: y = {k1:.4f} x + {k0:.1f}; grau von x {gx.min()} bis {gx.max()}; Dicke der Linie senkrecht {m.sum() / (np.hypot(1, k1) * (gx[m].max() - gx[m].min())):.1f} px")
    rippen = []
    for gr in gruppen:
        xu = float(np.mean(gr))
        if xu < 1500:
            continue
        # Rippe: graue Pixel in einem Streifen um die Spalte, oberhalb der geraden Wand bis zur Querleiste
        streifen = (np.abs(gx - xu) < 45) & (gy > k1 * gx + k0 + 12)
        if streifen.sum() < 50:
            continue
        a1, a0 = np.polyfit(gy[streifen], gx[streifen], 1)          # x = a1 y + a0
        y_oben = (k1 * a0 + k0) / (1.0 - k1 * a1)                    # Schnitt mit der Querleiste
        rippen.append(((a1 * Y0 + a0, Y0), (a1 * y_oben + a0, y_oben), len(gr)))
    x_links = (Y0 - k0) / k1
    quer = ((x_links, Y0), (float(gx.max()), k1 * gx.max() + k0))
    print("Querleiste von", [round(v, 3) for v in nach_m(*quer[0])], "bis", [round(v, 3) for v in nach_m(*quer[1])], "m")
    for rp in rippen:
        print("   Rippe von", [round(v, 3) for v in nach_m(*rp[0])], "bis", [round(v, 3) for v in nach_m(*rp[1])], "m; Linie", rp[2], "px breit")

    def m_liste(pp):
        return [[round(v, 4) for v in nach_m(x, y)] for x, y in pp]

    json.dump({"quelle": "Bader, Plath & Kontopidis 2025 (arXiv 2511.09037, CC BY), Fig. 2: input of the FDTD model of the Dulcken 1755 soundboard; read from the figure at 14 px per cm",
               "einheit": "m; X laengs vom Schwanz, Y quer von der geraden Wand",
               "laenge": round((x_max - x_min + 1) / PX / 100, 4), "breite": round((y_max - y_min + 1) / PX / 100, 4),
               "flaeche": round(float(flaeche), 4),
               "umriss": [[round(v, 4) for v in p] for p in umriss],
               "steg8": m_liste(steg8), "steg4": m_liste(steg4), "anhang4": m_liste(anh4), "anhang8": m_liste(anh8),
               "anhangleiste4": {"linie": m_liste(lm), "breite": round(float(breite / PX / 100), 4)},
               "querleiste": m_liste(quer), "rippen": [m_liste(rp[:2]) for rp in rippen]},
              open(sys.argv[2], "w", encoding="utf-8", newline="\n"), indent=1)
    if len(sys.argv) > 3:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(18, 9))
        ax.imshow(im.astype(np.uint8), alpha=0.55)
        u = np.array([(X0 + p[0] * 100 * PX, Y0 - p[1] * 100 * PX) for p in umriss + [umriss[0]]])
        ax.plot(u[:, 0], u[:, 1], "b-", lw=1.2)
        for rr, f in ((steg8, "m"), (steg4, "c"), (anh4, "g")):
            ax.plot([p[0] for p in rr], [p[1] for p in rr], f + "-", lw=0.8)
        ax.plot([p[0] for p in lm], [p[1] for p in lm], "r-", lw=1.5)
        ax.plot([quer[0][0], quer[1][0]], [quer[0][1], quer[1][1]], "k-", lw=1.5)
        for rp in rippen:
            ax.plot([rp[0][0], rp[1][0]], [rp[0][1], rp[1][1]], "k-", lw=1.5)
        fig.savefig(sys.argv[3], dpi=80)


if __name__ == "__main__":
    main()
