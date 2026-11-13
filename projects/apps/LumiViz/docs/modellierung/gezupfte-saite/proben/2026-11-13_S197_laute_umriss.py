"""Liest die Masse der Laute Sixtus Rauwolf 1598 (CL.93) aus der Messzeichnung (Santa Maria Bouquet 2020).

Die Zeichnung ist ein Vektor-PDF im Massstab 1:1 („All measurements in millimeters. Scale 1:1“):
    pdftocairo -svg SantaMariaBouquet_2020_LuteRauwolf1598_CL93.pdf rw.svg
    python laute_umriss.py rw.svg laute_umriss.json [kontrolle.png]

Gelesen werden (1 pt = 25,4/72 mm):
  - der Umriss der Decke (gefuelltes Band der Randeinlage, 475 mm lang) -> halbe Breite je Station
  - die Rose (Kreis 110 mm)
  - die sieben Querschnitte der Muschel (rot 5-7, blau 1-4) mit ihren Stationen in der Seitenansicht
  - das Seitenprofil (Tiefe 163,7 mm)
Laengsachse der Decke = x der Zeichnung, unteres Ende (Clasp end) links.
"""
import json
import re
import sys

import numpy as np

MM = 25.4 / 72.0
TOKEN = re.compile(r"[MLCHVZmlchvz]|-?\d+\.?\d*(?:e-?\d+)?")


def pfad_punkte(attr, schritte=24):
    d = re.search(r'\bd="([^"]*)"', attr).group(1)
    tr = re.search(r'transform="matrix\(([^)]*)\)"', attr)
    mat = [float(v) for v in tr.group(1).replace(",", " ").split()] if tr else [1, 0, 0, 1, 0, 0]
    tok = TOKEN.findall(d)
    i, bef, pos, start, teile, akt = 0, None, (0.0, 0.0), (0.0, 0.0), [], []

    def zahl():
        nonlocal i
        v = float(tok[i])
        i += 1
        return v

    while i < len(tok):
        if tok[i].isalpha():
            bef = tok[i]
            i += 1
            if bef in "Zz":
                if akt:
                    akt.append(start)
                    teile.append(akt)
                    akt = []
                pos = start
                continue
        if bef == "M":
            if akt:
                teile.append(akt)
            pos = (zahl(), zahl())
            start = pos
            akt = [pos]
            bef = "L"
        elif bef == "L":
            pos = (zahl(), zahl())
            akt.append(pos)
        elif bef == "H":
            pos = (zahl(), pos[1])
            akt.append(pos)
        elif bef == "V":
            pos = (pos[0], zahl())
            akt.append(pos)
        elif bef == "C":
            p1, p2, p3 = (zahl(), zahl()), (zahl(), zahl()), (zahl(), zahl())
            for k in range(1, schritte + 1):
                t = k / schritte
                a, b, c, e = (1 - t) ** 3, 3 * (1 - t) ** 2 * t, 3 * (1 - t) * t * t, t ** 3
                akt.append((a * pos[0] + b * p1[0] + c * p2[0] + e * p3[0], a * pos[1] + b * p1[1] + c * p2[1] + e * p3[1]))
            pos = p3
        else:
            raise ValueError("Befehl " + str(bef))
    if akt:
        teile.append(akt)
    aus = []
    for t in teile:
        arr = np.array(t)
        aus.append(np.column_stack([(mat[0] * arr[:, 0] + mat[2] * arr[:, 1] + mat[4]) * MM,
                                    (mat[1] * arr[:, 0] + mat[3] * arr[:, 1] + mat[5]) * MM]))
    return aus


def alle_pfade(svg):
    for m in re.finditer(r"<path\b([^>]*)>", svg):
        attr = m.group(1)
        if 'd="' not in attr:
            continue
        try:
            teile = pfad_punkte(attr)
        except (ValueError, IndexError):
            continue
        if not teile:
            continue
        p = np.vstack(teile)
        yield attr, teile, (p[:, 0].min(), p[:, 0].max(), p[:, 1].min(), p[:, 1].max())


def finde(pfade, bedingung):
    return [(a, t, b) for a, t, b in pfade if bedingung(a, b)]


def main():
    svg = open(sys.argv[1], encoding="utf-8").read()
    pfade = list(alle_pfade(svg))

    def b_h(b):
        return b[1] - b[0], b[3] - b[2]

    # Umriss: gefuelltes Band, 475 x 328,6 mm
    band = finde(pfade, lambda a, b: abs(b_h(b)[0] - 475) < 2 and abs(b_h(b)[1] - 328) < 4 and "fill" in a and "stroke" not in a)
    assert len(band) == 1, len(band)
    teile = band[0][1]
    bb = band[0][2]
    x_unten, mitte = bb[0], (bb[2] + bb[3]) / 2.0
    print(f"Band: {len(teile)} Teilpfade, Laenge {bb[1] - bb[0]:.1f}, Breite {bb[3] - bb[2]:.1f}, Mittellinie y {mitte:.1f}")
    alle = np.vstack(teile)
    # aeussere und innere Kante des Bandes je Station
    stationen = np.arange(0.0, bb[1] - bb[0] + 0.01, 2.5)
    umriss = []
    for s in stationen:
        x = x_unten + s
        ys = []
        for t in teile:
            for k in range(len(t) - 1):
                xa, xb = t[k, 0], t[k + 1, 0]
                if (xa - x) * (xb - x) <= 0 and xa != xb:
                    u = (x - xa) / (xb - xa)
                    ys.append(t[k, 1] + u * (t[k + 1, 1] - t[k, 1]))
        if len(ys) < 2:
            umriss.append((s, 0.0, 0.0, 0.0, 0.0))
            continue
        ys = np.array(sorted(ys)) - mitte
        neg, pos = ys[ys < 0], ys[ys >= 0]
        aussen_l = neg.min() if neg.size else 0.0
        aussen_r = pos.max() if pos.size else 0.0
        innen_l = neg.max() if neg.size > 1 else aussen_l
        innen_r = pos.min() if pos.size > 1 else aussen_r
        umriss.append((float(s), float(aussen_l), float(aussen_r), float(innen_l), float(innen_r)))

    # Rose
    rose = finde(pfade, lambda a, b: abs(b_h(b)[0] - 110) < 0.5 and abs(b_h(b)[1] - 110) < 0.5 and "stroke" in a)
    rb = rose[0][2]
    rose_x, rose_y, rose_d = (rb[0] + rb[1]) / 2 - x_unten, (rb[2] + rb[3]) / 2 - mitte, rb[1] - rb[0]
    print(f"Rose: Mitte {rose_x:.1f} mm ueber dem unteren Ende, quer {rose_y:+.1f}, Durchmesser {rose_d:.1f}")

    # Seitenprofil
    seite = finde(pfade, lambda a, b: abs(b_h(b)[0] - 497) < 3 and abs(b_h(b)[1] - 169) < 3)
    sb = seite[0][2]
    sp = np.vstack(seite[0][1])
    print(f"Seitenprofil: x {sb[0]:.1f}..{sb[1]:.1f}, y {sb[2]:.1f}..{sb[3]:.1f}")

    # Querschnitte: Halbboegen ueber der Grundlinie y = 191,2 / 191,3
    schnitte = finde(pfade, lambda a, b: abs(b[3] - 191.25) < 0.3 and b_h(b)[1] > 50 and b_h(b)[0] > 100)
    quer = []
    for a, t, b in schnitte:
        p = np.vstack(t)
        farbe = "rot" if "92.9" in a else "blau"
        # Flaeche unter dem Bogen (Trapez ueber x sortiert)
        o = np.argsort(p[:, 0])
        px, py = p[o, 0], b[3] - p[o, 1]
        flaeche = float(np.trapezoid(py, px))
        quer.append({"farbe": farbe, "breite": float(b[1] - b[0]), "tiefe": float(b[3] - b[2]), "flaeche_mm2": flaeche})
    # Stationen der Schnitte: senkrechte farbige Linien in der Seitenansicht
    linien = finde(pfade, lambda a, b: b_h(b)[0] < 0.3 and b_h(b)[1] > 50 and b[2] > 240 and b[3] < 560 and ("92.9" in a or "67.8" in a))
    stat = sorted(((b[0] + b[1]) / 2 - sb[0], b[3] - b[2], "rot" if "92.9" in a else "blau") for a, t, b in linien)
    print("Schnittlinien in der Seitenansicht (Abstand vom unteren Ende, Laenge, Farbe):")
    for s in stat:
        print(f"   {s[0]:6.1f}  {s[1]:6.1f}  {s[2]}")
    print("Querschnitte (Breite, Tiefe, Flaeche cm2):")
    for q in sorted(quer, key=lambda q: -q["breite"]):
        print(f"   {q['farbe']:5s} {q['breite']:6.1f} {q['tiefe']:6.1f} {q['flaeche_mm2'] / 100:7.1f}")

    json.dump({"quelle": "Santa Maria Bouquet 2020, Lute by Sixtus Rauwolf 1598 (CL.93), Musikmuseet Kopenhagen; aus dem Vektor-PDF gelesen (1:1, mm)",
               "einheit": "mm", "laenge": float(bb[1] - bb[0]), "breite": float(bb[3] - bb[2]),
               "umriss_spalten": ["station ab unterem Ende", "aussen links", "aussen rechts", "innen links", "innen rechts"],
               "umriss": [[round(v, 2) for v in u] for u in umriss],
               "rose": {"x": round(rose_x, 2), "quer": round(rose_y, 2), "durchmesser": round(rose_d, 2)},
               "schnittlinien": [[round(s[0], 1), round(s[1], 1), s[2]] for s in stat],
               "querschnitte": quer},
              open(sys.argv[2], "w", encoding="utf-8", newline="\n"), indent=1)
    if len(sys.argv) > 3:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(10, 7))
        u = np.array(umriss)
        ax.plot(u[:, 0], u[:, 1], "k-", u[:, 0], u[:, 2], "k-", lw=1)
        ax.plot(u[:, 0], u[:, 3], "r-", u[:, 0], u[:, 4], "r-", lw=0.6)
        w = np.linspace(0, 2 * np.pi, 100)
        ax.plot(rose_x + rose_d / 2 * np.cos(w), rose_y + rose_d / 2 * np.sin(w), "b-")
        ax.set_aspect("equal")
        ax.grid(alpha=0.3)
        fig.savefig(sys.argv[3], dpi=90)


if __name__ == "__main__":
    main()
