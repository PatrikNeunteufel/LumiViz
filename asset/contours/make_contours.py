"""Die mitgelieferten Formen schreiben (Klang aus einer Form, Stufe 19a, Session 120).

Aufruf (normales Python, ohne weitere Pakete):

    python asset/contours/make_contours.py

Schreibt neben dieses Skript je Form eine `.lvcontour` (Fassung 1, das Format
aus `projects/apps/LumiViz/include/shape/Kontur.hpp`). Die Koordinaten sind die
der Zeichnung: y nach unten, eine Flaeche von rund 200 mal 200.

Wer eine Form dazulegt, traegt sie hier ein und stellt im Test die Zahl nach
(`test_ShapeKontur.cpp`).
"""
import json
import math
import pathlib

ORDNER = pathlib.Path(__file__).parent
KAPPA = 0.5522847498307936  # ein Viertelkreis als kubische Kurve
FARBEN = ["#00e5ff", "#ff4fa3", "#ffb020", "#7f77dd"]


def z(wert):
    """Eine Zahl kurz: drei Stellen hinter dem Komma, ohne ueberfluessige Nullen."""
    text = f"{round(wert, 3):.3f}".rstrip("0").rstrip(".")
    return "0" if text in ("-0", "") else text


def vieleck(punkte):
    teile = [f"M {z(punkte[0][0])} {z(punkte[0][1])}"]
    teile += [f"L {z(x)} {z(y)}" for x, y in punkte[1:]]
    teile.append(f"L {z(punkte[0][0])} {z(punkte[0][1])} Z")
    return " ".join(teile)


def kreis(cx, cy, r):
    k = KAPPA * r
    stuecke = [
        (cx + r, cy + k, cx + k, cy + r, cx, cy + r),
        (cx - k, cy + r, cx - r, cy + k, cx - r, cy),
        (cx - r, cy - k, cx - k, cy - r, cx, cy - r),
        (cx + k, cy - r, cx + r, cy - k, cx + r, cy),
    ]
    teile = [f"M {z(cx + r)} {z(cy)}"]
    teile += ["C " + " ".join(z(w) for w in s) for s in stuecke]
    teile.append("Z")
    return " ".join(teile)


def stern(cx, cy, aussen, innen, zacken=5):
    punkte = []
    for i in range(2 * zacken):
        winkel = -math.pi / 2 + i * math.pi / zacken
        r = aussen if i % 2 == 0 else innen
        punkte.append((cx + r * math.cos(winkel), cy + r * math.sin(winkel)))
    return vieleck(punkte)


def herz(cx, cy, groesse):
    """Vier Kurven; die Vorlage ist auf eine Breite von 88 und eine Hoehe von 108 gezeichnet."""
    f = groesse / 108.0
    vorlage = [
        (100, 28, 56, 24, 56, 58),
        (56, 90, 100, 104, 100, 132),
        (100, 104, 144, 90, 144, 58),
        (144, 24, 100, 28, 100, 60),
    ]
    def p(x, y):
        return z(cx + (x - 100) * f), z(cy + (y - 78) * f)
    teile = ["M {} {}".format(*p(100, 60))]
    for x1, y1, x2, y2, x, y in vorlage:
        teile.append("C {} {} {} {} {} {}".format(*p(x1, y1), *p(x2, y2), *p(x, y)))
    teile.append("Z")
    return " ".join(teile)


def zeichnung(titel, konturen, nutzung="layers"):
    return {
        "format": "lumiviz-contour",
        "version": 1,
        "title": titel,
        "useAs": nutzung,
        "jumps": "listOrder",
        "nextId": len(konturen) + 1,
        "contours": [
            {"id": i + 1, "name": name, "color": FARBEN[i % len(FARBEN)], "selected": True, "path": pfad}
            for i, (name, pfad) in enumerate(konturen)
        ],
        "bridges": [],
        "source": {"kind": "drawn", "file": "", "settings": {}},
    }


FORMEN = {
    "circle": zeichnung("Circle", [("Circle", kreis(100, 100, 80))]),
    "square": zeichnung("Square", [("Square", vieleck([(30, 30), (170, 30), (170, 170), (30, 170)]))]),
    "triangle": zeichnung("Triangle", [("Triangle", vieleck([(100, 24), (176, 160), (24, 160)]))]),
    "star": zeichnung("Star", [("Star", stern(100, 104, 86, 36))]),
    "heart": zeichnung("Heart", [("Heart", herz(100, 100, 160))]),
    "heart_and_star": zeichnung(
        "Heart and star", [("Heart", herz(100, 100, 140)), ("Star", stern(290, 102, 80, 33))], nutzung="onePath"
    ),
}

if __name__ == "__main__":
    for name, inhalt in FORMEN.items():
        (ORDNER / f"{name}.lvcontour").write_text(json.dumps(inhalt, indent=4) + "\n", encoding="utf-8")
        print(f"{name}.lvcontour: {len(inhalt['contours'])} Kontur(en)")
