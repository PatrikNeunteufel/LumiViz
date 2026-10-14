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


def glatt(punkte):
    """Ein geschlossener, glatter Pfad durch die Punkte: Catmull-Rom als kubische Kurven."""
    n = len(punkte)
    teile = [f"M {z(punkte[0][0])} {z(punkte[0][1])}"]
    for i in range(n):
        p0, p1, p2, p3 = punkte[i - 1], punkte[i], punkte[(i + 1) % n], punkte[(i + 2) % n]
        c1 = (p1[0] + (p2[0] - p0[0]) / 6, p1[1] + (p2[1] - p0[1]) / 6)
        c2 = (p2[0] - (p3[0] - p1[0]) / 6, p2[1] - (p3[1] - p1[1]) / 6)
        teile.append("C " + " ".join(z(w) for w in (*c1, *c2, *p2)))
    teile.append("Z")
    return " ".join(teile)


def ellipse(cx, cy, rx, ry, drehung=0.0, schritte=24):
    c, s = math.cos(math.radians(drehung)), math.sin(math.radians(drehung))
    punkte = []
    for i in range(schritte):
        w = 2 * math.pi * i / schritte
        x, y = rx * math.cos(w), ry * math.sin(w)
        punkte.append((cx + x * c - y * s, cy + x * s + y * c))
    return glatt(punkte)


def bogen(cx, cy, r, von, bis, schritte):
    """Punkte auf einem Kreisbogen, Winkel in Grad (y nach unten), beide Enden eingeschlossen."""
    return [
        (cx + r * math.cos(math.radians(von + (bis - von) * i / schritte)),
         cy + r * math.sin(math.radians(von + (bis - von) * i / schritte)))
        for i in range(schritte + 1)
    ]


def mond(cx, cy, r):
    """Eine Sichel: aussen ein Halbkreis nach links, innen ein flacherer Bogen zurueck."""
    aussen = bogen(cx, cy, r, -70, -290, 22)
    # der innere Bogen: ein groesserer Kreis rechts, der durch beide Spitzen geht
    a, b = aussen[0], aussen[-1]
    halbe = abs(b[1] - a[1]) / 2
    mx = cx + r * 0.62
    ri = math.hypot(a[0] - mx, halbe)
    w = math.degrees(math.atan2(halbe, a[0] - mx))
    innen = bogen(mx, cy, ri, w, 360 - w, 16)[1:-1]  # von der unteren Spitze ueber links zur oberen
    return vieleck(aussen + innen)


def tropfen(cx, cy, r):
    return glatt([(cx, cy - 2.1 * r)] + bogen(cx, cy, r, -25, 205, 10))


def blitz():
    return vieleck([(122, 14), (52, 112), (96, 112), (72, 188), (152, 82), (106, 82), (138, 14)])


def note():
    """Eine Achtelnote als ein Umriss: Kopf, Hals, Faehnchen."""
    cx, cy, rx, ry = 74, 152, 30, 21
    links = 96  # linke Kante des Halses, rechte = cx + rx
    w0 = math.degrees(math.acos((links - cx) / rx))  # hier trifft der Hals den Kopf oben
    kopf = [(cx + rx * math.cos(math.radians(w)), cy + ry * math.sin(math.radians(w)))
            for w in [i * (360 - w0) / 20 for i in range(21)]]  # von rechts unten herum bis an den Hals
    rechts = cx + rx
    teile = [f"M {z(kopf[0][0])} {z(kopf[0][1])}"]
    teile += [f"L {z(x)} {z(y)}" for x, y in kopf[1:]]
    teile += [f"L {z(links)} 26", f"L {z(rechts)} 26",
              f"C {z(rechts + 6)} 52 {z(rechts + 44)} 62 {z(rechts + 36)} 112",
              f"C {z(rechts + 30)} 86 {z(rechts + 18)} 74 {z(rechts)} 70",
              f"L {z(kopf[0][0])} {z(kopf[0][1])} Z"]
    return " ".join(teile)


def blume(cx, cy, r, blaetter=5):
    punkte = []
    for i in range(blaetter * 4):
        w = -math.pi / 2 + 2 * math.pi * i / (blaetter * 4)
        rr = r * (0.62 + 0.38 * math.cos(blaetter * (w + math.pi / 2)))
        punkte.append((cx + rr * math.cos(w), cy + rr * math.sin(w)))
    return glatt(punkte)


def zahnrad(cx, cy, aussen, innen, zaehne=10):
    punkte = []
    teil = 2 * math.pi / zaehne
    for i in range(zaehne):
        w = -math.pi / 2 + i * teil
        for anteil, r in ((-0.30, innen), (-0.18, aussen), (0.18, aussen), (0.30, innen)):
            punkte.append((cx + r * math.cos(w + anteil * teil), cy + r * math.sin(w + anteil * teil)))
    return vieleck(punkte)


def kreuz(cx, cy, arm, breite):
    a, b = arm, breite / 2
    return vieleck([(cx - b, cy - a), (cx + b, cy - a), (cx + b, cy - b), (cx + a, cy - b), (cx + a, cy + b),
                    (cx + b, cy + b), (cx + b, cy + a), (cx - b, cy + a), (cx - b, cy + b), (cx - a, cy + b),
                    (cx - a, cy - b), (cx - b, cy - b)])


def haus():
    return vieleck([(100, 22), (128, 46), (128, 30), (146, 30), (146, 62), (178, 92), (156, 92), (156, 178),
                    (44, 178), (44, 92), (22, 92)])


def unendlich(cx, cy, a):
    """Die Lemniskate, als Acht gezeichnet: der Pfad kreuzt sich in der Mitte."""
    punkte = []
    for i in range(24):
        t = 2 * math.pi * i / 24
        n = 1 + math.sin(t) ** 2
        punkte.append((cx + a * math.cos(t) / n, cy + a * math.sin(t) * math.cos(t) / n))
    return glatt(punkte)


def katze(cx, cy, r):
    """Ein runder Kopf mit zwei spitzen Ohren."""
    punkte = []
    for i in range(36):
        w = i * 10
        # die Ohren: je eine Spitze ueber einem Bogen von 30 Grad (220 bis 250, 290 bis 320)
        if w in (240, 300):
            continue
        if w in (230, 310):
            spitze = 238 if w == 230 else 302
            punkte.append((cx + 1.5 * r * math.cos(math.radians(spitze)), cy + 1.5 * r * math.sin(math.radians(spitze))))
            continue
        punkte.append((cx + r * math.cos(math.radians(w)), cy + r * math.sin(math.radians(w))))
    return vieleck(punkte)


def mund(cx, cy, r):
    """Ein laechelnder Mund: unten ein Bogen, oben ein flacherer zurueck."""
    unten = bogen(cx, cy, r, 20, 160, 14)
    oben = bogen(cx, cy - r * 0.55, r * 1.25, 150, 30, 12)[1:-1]
    return vieleck(unten + oben)


def yin_welle(cx, cy, r):
    """Die dunkle Haelfte des Yin-Yang: grosser Halbkreis links, zwei kleine Halbkreise in der Mitte."""
    gross = bogen(cx, cy, r, -90, -270, 18)                  # oben ueber links nach unten
    unten = bogen(cx, cy + r / 2, r / 2, 90, -90, 9)[1:]     # unten: kleiner Halbkreis rechts herum nach oben
    oben = bogen(cx, cy - r / 2, r / 2, 90, 270, 9)[1:-1]    # oben: kleiner Halbkreis links herum
    return glatt(gross + unten + oben)


def zeichnung(titel, konturen, nutzung="layers", spruenge="listOrder"):
    return {
        "format": "lumiviz-contour",
        "version": 1,
        "title": titel,
        "useAs": nutzung,
        "jumps": spruenge,
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
    # --- S123: vierzehn weitere, zehn mit einer Kontur und vier mit mehreren
    "moon": zeichnung("Moon", [("Moon", mond(100, 100, 82))]),
    "drop": zeichnung("Drop", [("Drop", tropfen(100, 120, 58))]),
    "bolt": zeichnung("Bolt", [("Bolt", blitz())]),
    "note": zeichnung("Note", [("Note", note())]),
    "flower": zeichnung("Flower", [("Flower", blume(100, 100, 86))]),
    "gear": zeichnung("Gear", [("Gear", zahnrad(100, 100, 84, 64))]),
    "cross": zeichnung("Cross", [("Cross", kreuz(100, 100, 80, 46))]),
    "house": zeichnung("House", [("House", haus())]),
    "infinity": zeichnung("Infinity", [("Infinity", unendlich(100, 100, 90))]),
    "cat": zeichnung("Cat", [("Cat", katze(100, 112, 62))]),
    "smiley": zeichnung("Smiley", [
        ("Face", kreis(100, 100, 84)),
        ("Left eye", ellipse(72, 76, 10, 15)),
        ("Right eye", ellipse(128, 76, 10, 15)),
        ("Mouth", mund(100, 108, 48)),
    ]),
    "yin_yang": zeichnung("Yin and yang", [
        ("Circle", kreis(100, 100, 84)),
        ("Wave", yin_welle(100, 100, 84)),
        ("Dot 1", kreis(100, 58, 12)),
        ("Dot 2", kreis(100, 142, 12)),
    ], nutzung="onePath", spruenge="shortest"),
    "rings": zeichnung("Rings", [
        (f"Ring {i + 1}", kreis(x, y, 42)) for i, (x, y) in enumerate([(56, 80), (152, 80), (248, 80), (104, 126), (200, 126)])
    ], nutzung="onePath", spruenge="shortest"),
    "atom": zeichnung("Atom", [
        ("Core", kreis(100, 100, 14)),
        ("Orbit 1", ellipse(100, 100, 90, 32, 0)),
        ("Orbit 2", ellipse(100, 100, 90, 32, 60)),
        ("Orbit 3", ellipse(100, 100, 90, 32, 120)),
    ]),
}

if __name__ == "__main__":
    for name, inhalt in FORMEN.items():
        (ORDNER / f"{name}.lvcontour").write_text(json.dumps(inhalt, indent=4) + "\n", encoding="utf-8")
        print(f"{name}.lvcontour: {len(inhalt['contours'])} Kontur(en)")
