"""Icons bauen, die aus `lumiviz.ico` abgeleitet sind (Session 88).

Aufruf (normales Python mit Pillow):

    python asset/img/logo/icons/build_lvfx2_icons.py

Erzeugt vierundzwanzig .ico (16–256 px) neben diesem Skript:

- `lumiviz_lvfx2.ico`   „Stapel"  — das Icon vorn, zwei abgedunkelte Kopien
                        versetzt dahinter. Das Datei-Icon fuer `.lvfx2`
                        (Preset-Stapel; Entscheid Patrik S88).
- `lumiviz_marke.ico`   „Marke"   — das Icon ganz, unten rechts eine runde
                        Marke mit drei Ebenen in den Logofarben. Reserve.
- `lumiviz_karaoke.ico` „Marke" mit Mikrofon im Kreis — das Datei-Icon fuer
                        Karaoke-Presets `.lvkar` (Session 89). Die Marke ist
                        eine Familie: je Verwendung ein Zeichen im Kreis.
- `lumiviz_trail.ico`   „Marke" mit Schweif im Kreis — das Datei-Icon fuer
                        Trail-Presets `.lvtrail` (Session 92, Entscheid
                        Patrik: der Komet bleibt auch in 32 px als Bewegung
                        lesbar). In der App traegt es jede Effect List, die
                        einen Lyrics Trail enthaelt.
- acht weitere aus der Familie „Marke" (Session 120), je `lumiviz_<endung>.ico`:
                        `lvloop` (Schritt-Raster mit Bogen zurueck), `lvinst`
                        (Klaviertasten), `lvcontour` (Kontur mit Stift),
                        `lvdata` (Ablage mit Pfeil), `lvfade` (Keil), `lvshape`
                        (Huegel), `lvmute` (Lautsprecher mit Kreuz), `grad`
                        (Verlauf).
- `lumiviz_lvkit.ico`   „Marke" mit einer Snare und zwei gekreuzten Sticks —
                        das Datei-Icon fuer Kits `.lvkit` (Session 124;
                        Entscheide Patrik I1 a, I2 a: am Vorschaublatt aus
                        drei Fassungen gewaehlt).
- `lumiviz_lvchord.ico` „Marke" mit einem Griffbild — das Datei-Icon fuer Akkorde
                        `.lvchord` (Session 137, Entscheid S-T3 a).
- `lumiviz_lvinstbodyplan.ico` „Marke" mit dem Umriss eines Resonanzbodens, Rippen und
                        Steg — das Datei-Icon fuer Bauplaene `.lvinstbodyplan`;
  `lumiviz_lvinstbody.ico` derselbe Umriss mit einer Mode — fuer Koerper
                        `.lvinstbody` (Session 147, Entscheid IC1 a). Der Umriss
                        kommt aus `asset/loop/instrument_bodies/steinway_d.lvinstbodyplan`.
- `lumiviz_lvpattern.ico`, `lumiviz_lvclip.ico`, `lumiviz_lvtrack.ico` „Marke" mit Noten
                        einer Piano-Roll, einem Clip mit Wiederholung, einer Zeile mit drei
                        Clips — die Datei-Icons fuer Muster, Clips und Spuren (Session 167,
                        Loop Stufe 33).
- `lumiviz_lvchords.ico` „Marke" mit drei Griffbildern hintereinander — das Datei-Icon fuer
                        die Sammlungen der mitgelieferten Akkorde `.lvchords` (Session 167, I1 a).
- `lumiviz_lvp.ico`     „Marke" mit fuenf Equalizer-Balken — das Datei-Icon fuer die Presets
                        der klassischen Visualizer `.lvp` (Session 167, I1 a).
- `lumiviz_faecher.ico` „Faecher" — drei gleich grosse Icons diagonal
                        gestaffelt. Reserve.
- `lumiviz_lvcomp.ico`  der Faecher mit Zeitstrahl und Abspielmarke — das
                        Datei-Icon fuer Kompositionen des Composers `.lvcomp`
                        (Session 100).
- `lumiviz_app.ico`     das Icon der App: `lumiviz.ico` auf ein Quadrat gesetzt,
                        in den ueblichen Stufen — fuer die Icon-Ressource der
                        Exe (Session 100).

Die Reserve-Icons sind noch ohne Verwendung (Ideen in `README.md`).
`lumiviz.ico` selbst ist Handarbeit und wird nur gelesen.
"""
import json
import math
import pathlib
import sys

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

ORDNER = pathlib.Path(__file__).parent
GROESSEN = (16, 24, 32, 48, 64, 128, 256)
K = 1024  # Arbeitsgroesse; die Stufen werden daraus heruntergerechnet


def quelle() -> Image.Image:
    ico = Image.open(ORDNER / "lumiviz.ico")
    ico.size = max(ico.info["sizes"])  # die groesste enthaltene Stufe
    ico.load()
    return ico.convert("RGBA")


def quadrat(img: Image.Image, kante: int) -> Image.Image:
    """Unverzerrt auf ein Quadrat setzen (das Original ist nicht quadratisch)."""
    f = kante / max(img.size)
    klein = img.resize((round(img.width * f), round(img.height * f)), Image.LANCZOS)
    out = Image.new("RGBA", (kante, kante), (0, 0, 0, 0))
    out.alpha_composite(klein, ((kante - klein.width) // 2, (kante - klein.height) // 2))
    return out


def gedimmt(img: Image.Image, hell: float, satt: float) -> Image.Image:
    r, g, b, a = img.split()
    rgb = Image.merge("RGB", (r, g, b))
    rgb = ImageEnhance.Color(rgb).enhance(satt)
    rgb = ImageEnhance.Brightness(rgb).enhance(hell)
    return Image.merge("RGBA", (*rgb.split(), a))


def mit_schatten(ziel: Image.Image, lage: Image.Image, x: int, y: int, radius: int,
                 deck: float) -> None:
    """`lage` bei (x, y) auflegen, darunter ein weicher Schatten nach rechts unten."""
    alpha = lage.split()[3].filter(ImageFilter.GaussianBlur(radius))
    schatten = Image.new("RGBA", lage.size, (0, 0, 0, 0))
    schatten.putalpha(alpha.point(lambda v: int(v * deck)))
    sx = min(x + radius // 2, ziel.width - lage.width)
    sy = min(y + radius // 2, ziel.height - lage.height)
    ziel.alpha_composite(schatten, (sx, sy))
    ziel.alpha_composite(lage, (x, y))


def stapel(icon: Image.Image) -> Image.Image:
    out = Image.new("RGBA", (K, K), (0, 0, 0, 0))
    teil = quadrat(icon, int(K * 0.74))
    schritt = int(K * 0.125)
    # hinten -> vorn: (Stufe, Helligkeit, Saettigung)
    for i, hell, satt in ((2, 0.42, 0.55), (1, 0.66, 0.8), (0, 1.0, 1.0)):
        mit_schatten(out, gedimmt(teil, hell, satt), i * schritt,
                     K - teil.height - i * schritt, 18, 0.55)
    return out


CYAN, MAGENTA, BERNSTEIN = (0, 229, 255), (255, 79, 163), (255, 176, 32)  # Logofarben
MARKE_GRUND = (24, 27, 33, 255)


def zeichen_ebenen(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Drei Rauten uebereinander (Ebenen), in den Farben des Logos."""
    b, h = int(r * 0.62), int(r * 0.26)
    for i, farbe in enumerate((CYAN, MAGENTA, BERNSTEIN)):
        my = cy - int(r * 0.36) + i * int(r * 0.36)
        raute = [(cx, my - h), (cx + b, my), (cx, my + h), (cx - b, my)]
        d.polygon(raute, fill=MARKE_GRUND)
        d.line(raute + [raute[0]], fill=(*farbe, 255), width=16, joint="curve")


def zeichen_mikrofon(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Mikrofon: Kapsel, Buegel, Stiel und Fuss — gefuellt, damit es klein lesbar bleibt."""
    kb = int(r * 0.24)                    # halbe Kapselbreite
    oben, unten = cy - int(r * 0.66), cy + int(r * 0.10)
    d.rounded_rectangle((cx - kb, oben, cx + kb, unten), radius=kb, fill=(*CYAN, 255))
    bb = int(r * 0.44)                    # halbe Buegelbreite
    by = cy - int(r * 0.08)               # Mitte des Buegel-Bogens
    d.arc((cx - bb, by - bb, cx + bb, by + bb), start=0, end=180, fill=(*MAGENTA, 255), width=18)
    fuss = cy + int(r * 0.66)
    d.line([(cx, by + bb - 4), (cx, fuss)], fill=(*BERNSTEIN, 255), width=18)
    d.line([(cx - int(r * 0.30), fuss), (cx + int(r * 0.30), fuss)], fill=(*BERNSTEIN, 255),
           width=18)


def zeichen_schweif(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Komet: ein Kopf rechts, drei Striche ziehen nach links weg (Nachwehen)."""
    kopf = int(r * 0.22)
    kx = cx + int(r * 0.40)
    dicke = int(r * 0.13)
    for dy, laenge, farbe in ((-0.34, 0.55, BERNSTEIN), (0.0, 0.85, MAGENTA),
                              (0.34, 0.55, BERNSTEIN)):
        y = cy + int(r * dy)
        d.rounded_rectangle((kx - int(r * laenge) - kopf, y - dicke // 2, kx - kopf // 2,
                             y + dicke // 2), radius=dicke // 2, fill=(*farbe, 255))
    d.ellipse((kx - kopf, cy - kopf, kx + kopf, cy + kopf), fill=(*CYAN, 255))


# --- acht weitere Zeichen (Session 120, Entscheid Patrik I1 a: jede Dateiart
# ohne Icon bekommt die Marke mit ihrem Zeichen; die Zeichen an vier
# Vorschaublaettern gewaehlt). Gefuellte Flaechen statt duenner Striche, damit
# bei 32 px etwas stehen bleibt. ---------------------------------------------

LEER = (70, 76, 88, 255)  # ein nicht gesetzter Schritt im Raster


def zeichen_loop(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Schritt-Raster wie im Drum-Editor (zwei Spuren, vier Schritte), darueber
    ein Bogen vom letzten Schritt zurueck zum ersten."""
    z, luecke = int(r * 0.24), int(r * 0.07)
    breite = 4 * z + 3 * luecke
    x0, y0 = cx - breite // 2, cy + int(r * 0.04)
    for zeile, (farbe, schritte) in enumerate(((CYAN, (1, 0, 1, 1)), (MAGENTA, (0, 1, 0, 1)))):
        for spalte, an in enumerate(schritte):
            x, y = x0 + spalte * (z + luecke), y0 + zeile * (z + luecke)
            d.rounded_rectangle((x, y, x + z, y + z), radius=int(z * 0.22),
                                fill=(*farbe, 255) if an else LEER)
    rx, ry = (breite - z) // 2, int(r * 0.42)
    yc = y0 - int(r * 0.24)               # Mitte des Bogens; die Spitze endet ueber dem Raster
    d.arc((cx - rx, yc - ry, cx + rx, yc + ry), start=190, end=360, fill=(*BERNSTEIN, 255),
          width=int(r * 0.15))
    s = int(r * 0.18)
    d.polygon([(cx - rx - s, yc - int(r * 0.08)), (cx - rx + s, yc - int(r * 0.08)),
               (cx - rx, yc + int(r * 0.13))], fill=(*BERNSTEIN, 255))


def zeichen_tasten(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Drei Klaviertasten in den Logofarben, zwei schwarze dazwischen."""
    b, h, luecke = int(r * 0.30), int(r * 0.62), int(r * 0.06)
    x0 = cx - (3 * b + 2 * luecke) // 2
    for i, farbe in enumerate((CYAN, MAGENTA, BERNSTEIN)):
        x = x0 + i * (b + luecke)
        d.rounded_rectangle((x, cy - h, x + b, cy + h), radius=int(b * 0.25), fill=(*farbe, 255))
    sb = int(r * 0.20)
    for i in (1, 2):
        x = x0 + i * (b + luecke) - luecke // 2
        d.rectangle((x - sb // 2, cy - h - 2, x + sb // 2, cy + int(h * 0.15)), fill=MARKE_GRUND)


def zeichen_kontur(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Eine geschlossene Form mit kraeftigem Rand, darauf der Stift als Punkt."""
    radius = r * 0.50
    punkte = []
    for i in range(121):
        t = 2 * math.pi * i / 120
        rad = radius * (1 + 0.28 * math.cos(3 * t + 0.6))
        punkte.append((cx + rad * math.cos(t), cy + rad * math.sin(t)))
    d.polygon(punkte, fill=(150, 46, 96, 255))  # Magenta, abgedunkelt
    d.line(punkte, fill=(*CYAN, 255), width=34, joint="curve")
    px, py = punkte[58]
    k = int(r * 0.20)
    d.ellipse((px - k - 8, py - k - 8, px + k + 8, py + k + 8), fill=MARKE_GRUND)
    d.ellipse((px - k, py - k, px + k, py + k), fill=(*BERNSTEIN, 255))


def zeichen_ablage(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Eine gefuellte Ablage, ein Pfeil faellt hinein (Export der eigenen Daten)."""
    b = int(r * 0.58)
    d.rounded_rectangle((cx - b, cy + int(r * 0.10), cx + b, cy + int(r * 0.58)),
                        radius=int(r * 0.10), fill=(*BERNSTEIN, 255))
    spitze, s, schaft = cy + int(r * 0.40), int(r * 0.34), int(r * 0.12)
    oben = cy - int(r * 0.68)
    # erst dunkel und etwas groesser (der Rand ueber der Ablage), dann der Pfeil
    for rand, farbe in ((int(r * 0.07), MARKE_GRUND), (0, (*CYAN, 255))):
        d.rectangle((cx - schaft - rand, oben - rand, cx + schaft + rand, spitze - s + 2),
                    fill=farbe)
        d.polygon([(cx - s - rand * 2, spitze - s - rand), (cx + s + rand * 2, spitze - s - rand),
                   (cx, spitze + rand * 2)], fill=farbe)


def zeichen_blende(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Die Flaeche unter einer ansteigenden Kurve: ein Keil auf einer Grundlinie."""
    b, h = int(r * 0.60), int(r * 0.44)
    punkte = []
    for i in range(41):
        u = i / 40
        punkte.append((cx - b + 2 * b * u, cy + h - 2 * h * u * u * (3 - 2 * u)))
    d.polygon(punkte + [(cx + b, cy + h), (cx - b, cy + h)], fill=(*CYAN, 255))
    d.line([(cx - b, cy + h), (cx + b, cy + h)], fill=(*BERNSTEIN, 255), width=24)


def zeichen_stueck(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Huegel mitten auf der Grundlinie: ein Stueck der Pegelkurve."""
    b, h = int(r * 0.62), int(r * 0.44)
    punkte = []
    for i in range(61):
        u = i / 60
        v = 0.0 if u < 0.2 or u > 0.8 else 0.5 * (1 - math.cos(2 * math.pi * (u - 0.2) / 0.6))
        punkte.append((cx - b + 2 * b * u, cy + h - 2 * h * v))
    d.polygon(punkte, fill=(*MAGENTA, 255))
    d.line([(cx - b, cy + h), (cx + b, cy + h)], fill=(*BERNSTEIN, 255), width=24)


def zeichen_stumm(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Lautsprecher, daneben ein Kreuz."""
    x = cx - int(r * 0.58)
    k, g = int(r * 0.18), int(r * 0.46)
    d.polygon([(x, cy - k), (x + int(r * 0.24), cy - k), (x + int(r * 0.56), cy - g),
               (x + int(r * 0.56), cy + g), (x + int(r * 0.24), cy + k), (x, cy + k)],
              fill=(*CYAN, 255))
    a, m = int(r * 0.22), cx + int(r * 0.36)
    d.line([(m - a, cy - a), (m + a, cy + a)], fill=(*MAGENTA, 255), width=22)
    d.line([(m - a, cy + a), (m + a, cy - a)], fill=(*MAGENTA, 255), width=22)


def zeichen_verlauf(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Balken, der von Cyan ueber Magenta nach Bernstein laeuft."""
    b, h = int(r * 0.62), int(r * 0.30)
    for x in range(-b, b + 1):
        u = (x + b) / (2 * b)
        von, nach, v = (CYAN, MAGENTA, u * 2) if u < 0.5 else (MAGENTA, BERNSTEIN, u * 2 - 1)
        farbe = tuple(round(von[i] + (nach[i] - von[i]) * v) for i in range(3))
        d.line([(cx + x, cy - h), (cx + x, cy + h)], fill=(*farbe, 255), width=1)
    d.rectangle((cx - b, cy - h, cx + b, cy + h), outline=(235, 235, 235, 255), width=10)


def zeichen_trommel(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Eine Snare von der Seite — Kessel in Magenta, das Fell als Ellipse in Cyan —,
    darueber zwei gekreuzte Sticks in Bernstein (Session 124, I2 a: Fassung A)."""
    b = int(r * 0.58)
    oben, unten = cy - int(r * 0.02), cy + int(r * 0.50)
    e = int(r * 0.16)
    d.rectangle((cx - b, oben, cx + b, unten), fill=(*MAGENTA, 255))
    d.ellipse((cx - b, unten - e, cx + b, unten + e), fill=(*MAGENTA, 255))
    d.ellipse((cx - b, oben - e, cx + b, oben + e), fill=(*CYAN, 255))
    w = int(r * 0.12)
    for vz in (-1, 1):
        x0, y0 = cx - vz * int(r * 0.20), oben - int(r * 0.08)
        d.line([(x0, y0), (cx + vz * int(r * 0.62), cy - int(r * 0.66))], fill=(*BERNSTEIN, 255), width=w)
        k = int(w * 0.85)  # die Spitze des Sticks
        d.ellipse((x0 - k, y0 - k, x0 + k, y0 + k), fill=(*BERNSTEIN, 255))


def zeichen_griffbild(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Griffbild (Session 137, Entscheid S-T3 a; Wunsch Patrik: „das Motiv ein
    Griffbild"): vier Saiten, oben der Sattel, drei Buende, darauf drei Finger in den
    Farben des Logos — nur vier Saiten, damit es bei 32 px lesbar bleibt."""
    b, h = int(r * 0.56), int(r * 0.62)
    links, rechts, oben, unten = cx - b, cx + b, cy - h, cy + h
    hell = (235, 235, 235, 255)
    d.rectangle((links - 6, oben - int(r * 0.07), rechts + 6, oben + int(r * 0.07)), fill=hell)  # der Sattel
    for i in range(1, 4):
        y = oben + (unten - oben) * i // 3
        d.line([(links, y), (rechts, y)], fill=(150, 156, 168, 255), width=12)
    for i in range(4):
        x = links + (rechts - links) * i // 3
        d.line([(x, oben), (x, unten)], fill=hell, width=14)
    k = int(r * 0.17)
    for saite, bund, farbe in ((1, 0, CYAN), (2, 1, MAGENTA), (3, 1, BERNSTEIN)):
        x = links + (rechts - links) * saite // 3
        y = oben + (unten - oben) * (2 * bund + 1) // 6
        d.ellipse((x - k, y - k, x + k, y + k), fill=(*farbe, 255))


def zeichen_balken(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Das Preset eines klassischen Visualizers (Session 167, `.lvp`, nach I1 a): fuenf
    Balken eines Equalizers in den Farben des Logos, auf einer gemeinsamen Grundlinie."""
    hoehen = (0.45, 0.80, 1.00, 0.62, 0.35)
    farben = (CYAN, CYAN, MAGENTA, BERNSTEIN, BERNSTEIN)
    b, luecke = int(r * 0.20), int(r * 0.07)
    x0 = cx - (5 * b + 4 * luecke) // 2
    unten = cy + int(r * 0.50)
    for i, (hoehe, farbe) in enumerate(zip(hoehen, farben)):
        x = x0 + i * (b + luecke)
        d.rounded_rectangle((x, unten - int(r * 1.00 * hoehe), x + b, unten), radius=int(b * 0.25),
                            fill=(*farbe, 255))


def zeichen_griffbilder(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Eine Sammlung von Akkorden (Session 167, `.lvchords`, nach I1 a): drei kleine
    Griffbilder hintereinander, das vordere mit Fingern in den Farben des Logos."""
    hell = (235, 235, 235, 255)
    b, h = int(r * 0.30), int(r * 0.38)
    for stufe, (dx, dy) in enumerate(((-0.22, -0.24), (0.0, -0.02), (0.22, 0.20))):
        x0, y0 = cx + int(r * dx) - b, cy + int(r * dy) - h
        vorn = stufe == 2
        d.rectangle((x0 - 8, y0 - 8, x0 + 2 * b + 8, y0 + 2 * h + 8), fill=MARKE_GRUND)
        farbe = hell if vorn else (120, 126, 138, 255)
        d.rectangle((x0, y0 - 10, x0 + 2 * b, y0 + 10), fill=farbe)  # der Sattel
        for i in range(1, 3):
            y = y0 + 2 * h * i // 3
            d.line([(x0, y), (x0 + 2 * b, y)], fill=(150, 156, 168, 255), width=9)
        for i in range(4):
            x = x0 + 2 * b * i // 3
            d.line([(x, y0), (x, y0 + 2 * h)], fill=farbe, width=10)
        if vorn:
            k = int(r * 0.11)
            for saite, bund, f in ((1, 0, CYAN), (2, 1, MAGENTA), (3, 1, BERNSTEIN)):
                x = x0 + 2 * b * saite // 3
                y = y0 + 2 * h * (2 * bund + 1) // 6
                d.ellipse((x - k, y - k, x + k, y + k), fill=(*f, 255))


# --- Muster, Clip und Spur als Dateien (Session 167, Loop Stufe 33, Wunsch Patrik) ------------

def zeichen_muster(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Muster, wie es im Editor steht: vier Noten als Balken einer Piano-Roll,
    versetzt in Zeit und Hoehe, in den Farben des Logos."""
    h = int(r * 0.22)
    links, rechts = cx - int(r * 0.62), cx + int(r * 0.62)
    breite = rechts - links
    for zeile, (von, bis, farbe) in enumerate(((0.00, 0.40, CYAN), (0.30, 0.62, MAGENTA),
                                                (0.55, 1.00, BERNSTEIN), (0.00, 0.22, MAGENTA))):
        y = cy - int(r * 0.58) + zeile * int(r * 0.32)
        d.rounded_rectangle((links + int(breite * von), y, links + int(breite * bis), y + h),
                            radius=int(h * 0.35), fill=(*farbe, 255))


def zeichen_clip(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Ein Clip wie auf der Taktachse: ein Block, in der Mitte die Naht zur Wiederholung,
    darin dieselben zwei Noten zweimal."""
    links, rechts = cx - int(r * 0.70), cx + int(r * 0.70)
    oben, unten = cy - int(r * 0.42), cy + int(r * 0.42)
    d.rounded_rectangle((links, oben, rechts, unten), radius=int(r * 0.14), fill=(40, 120, 160, 255),
                        outline=(*CYAN, 255), width=14)
    mitte = cx
    for y in range(oben + 18, unten - 18, 44):  # die Naht gestrichelt
        d.line([(mitte, y), (mitte, y + 24)], fill=(235, 235, 235, 255), width=12)
    h = int(r * 0.13)
    for start in (links, mitte):
        x = start + int(r * 0.12)
        d.rounded_rectangle((x, cy - int(r * 0.24), x + int(r * 0.30), cy - int(r * 0.24) + h),
                            radius=int(h * 0.4), fill=(*BERNSTEIN, 255))
        d.rounded_rectangle((x + int(r * 0.22), cy + int(r * 0.08), x + int(r * 0.50), cy + int(r * 0.08) + h),
                            radius=int(h * 0.4), fill=(*MAGENTA, 255))


def zeichen_spur(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Eine Spur: eine Zeile der Taktachse mit drei Clips hintereinander."""
    links, rechts = cx - int(r * 0.78), cx + int(r * 0.78)
    oben, unten = cy - int(r * 0.30), cy + int(r * 0.30)
    d.rectangle((links, oben - 10, rechts, unten + 10), fill=LEER)
    luecke = int(r * 0.06)
    teile = ((0.00, 0.38, CYAN), (0.38, 0.62, MAGENTA), (0.62, 1.00, BERNSTEIN))
    breite = rechts - links
    for von, bis, farbe in teile:
        d.rounded_rectangle((links + int(breite * von) + luecke // 2, oben,
                             links + int(breite * bis) - luecke // 2, unten),
                            radius=int(r * 0.08), fill=(*farbe, 255))


BAUPLAN = ORDNER.parents[2] / "loop" / "instrument_bodies" / "steinway_d.lvinstbodyplan"
HELL = (235, 235, 235, 255)


def boden(cx: int, cy: int, r: int):
    """Umriss, Steg und Rippen des Steinway D aus dem Bauplan, in Pixel des Kreises
    (der Boden in 70 % des Durchmessers, mittig)."""
    plan = json.loads(BAUPLAN.read_text(encoding="utf-8"))
    umriss = plan["plate"]["outline"]
    x0, x1 = min(p[0] for p in umriss), max(p[0] for p in umriss)
    y0, y1 = min(p[1] for p in umriss), max(p[1] for p in umriss)
    s = 2 * r * 0.70 / (y1 - y0)
    mx, my = (x0 + x1) / 2, (y0 + y1) / 2

    def t(p):
        return (cx + (p[0] - mx) * s, cy - (p[1] - my) * s)

    return umriss, plan["bridges"][0]["line"], [rippe["line"] for rippe in plan["ribs"]], t


def zeichen_bauplan(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Der Bauplan eines Resonanzbodens (Session 147, IC1 a, Fassung P1): der Umriss
    dunkel mit hellem Rand, jede dritte Rippe in Cyan, der Steg in Bernstein."""
    umriss, steg, rippen, t = boden(cx, cy, r)
    pts = [t(p) for p in umriss]
    d.polygon(pts, fill=(60, 66, 78, 255))
    d.line(pts + [pts[0]], fill=HELL, width=14, joint="curve")
    for rippe in rippen[1::3]:
        d.line([t(rippe[0]), t(rippe[-1])], fill=(*CYAN, 255), width=16)
    d.line([t(p) for p in steg], fill=(*BERNSTEIN, 255), width=24, joint="curve")


def zeichen_koerper(d: ImageDraw.ImageDraw, cx: int, cy: int, r: int) -> None:
    """Der gerechnete Koerper (Session 147, IC1 a, Fassung K1): derselbe Umriss mit
    einer Mode — die Baeuche wie sin(2πx)·sin(3πy) in Cyan und Magenta."""
    umriss, _, _, t = boden(cx, cy, r)
    x0, x1 = min(p[0] for p in umriss), max(p[0] for p in umriss)
    y0, y1 = min(p[1] for p in umriss), max(p[1] for p in umriss)

    def innen(x, y):
        drin = False
        for k in range(len(umriss)):
            (ax, ay), (bx, by) = umriss[k], umriss[k - 1]
            if (ay > y) != (by > y) and x < (bx - ax) * (y - ay) / (by - ay) + ax:
                drin = not drin
        return drin

    schritt = 0.025
    y = y0
    while y < y1:
        x = x0
        while x < x1:
            if innen(x + schritt / 2, y + schritt / 2):
                v = math.sin(2 * math.pi * (x - x0) / (x1 - x0)) * math.sin(3 * math.pi * (y - y0) / (y1 - y0))
                a, b = t((x, y + schritt)), t((x + schritt, y))
                d.rectangle((a[0], a[1], b[0] + 1, b[1] + 1), fill=(*(CYAN if v >= 0 else MAGENTA), 255))
            x += schritt
        y += schritt
    pts = [t(p) for p in umriss]
    d.line(pts + [pts[0]], fill=HELL, width=14, joint="curve")


def marke(icon: Image.Image, zeichen=zeichen_ebenen) -> Image.Image:
    """Das Icon ganz, unten rechts ein Kreis mit einem Zeichen — eine Familie:
    je Verwendung ein eigenes Zeichen im Kreis (Idee Patrik S89)."""
    out = quadrat(icon, K)
    d = ImageDraw.Draw(out)
    r = int(K * 0.21)
    cx = cy = K - r - int(K * 0.02)
    d.ellipse((cx - r - 14, cy - r - 14, cx + r + 14, cy + r + 14), fill=(8, 9, 12, 255))
    d.ellipse((cx - r, cy - r, cx + r, cy + r), fill=MARKE_GRUND,
              outline=(235, 235, 235, 255), width=14)
    zeichen(d, cx, cy, r)
    return out


def faecher(icon: Image.Image) -> Image.Image:
    out = Image.new("RGBA", (K, K), (0, 0, 0, 0))
    teil = quadrat(icon, int(K * 0.6))
    schritt = (K - teil.width) // 2
    for i in range(3):  # links oben -> rechts unten, alle voll farbig
        mit_schatten(out, teil, i * schritt, i * schritt, 16, 0.6)
    return out


def komposition(icon: Image.Image) -> Image.Image:
    """Der Faecher, darueber ein Zeitstrahl mit Abspielmarke — das Datei-Icon
    fuer Kompositionen des Composers `.lvcomp` (Session 100, Entscheide Patrik:
    der Faecher, mit Zeitstrahl und Marke — die rote Marke bleibt auch klein
    als eigener Farbpunkt stehen und trennt das Icon vom Stapel)."""
    out = Image.new("RGBA", (K, K), (0, 0, 0, 0))
    band = int(K * 0.20)  # oben frei fuer den Zeitstrahl
    hoehe = K - band
    teil = quadrat(icon, int(hoehe * 0.62))
    sx = (K - teil.width) // 2
    sy = (hoehe - teil.height) // 2
    for i in range(3):  # links oben -> rechts unten, alle voll farbig
        mit_schatten(out, teil, i * sx, band + i * sy, 16, 0.6)
    d = ImageDraw.Draw(out)
    hell, rot = (235, 235, 235, 255), (255, 90, 70, 255)
    x0, x1, y = int(K * 0.04), int(K * 0.96), int(K * 0.10)
    d.line([(x0, y), (x1, y)], fill=hell, width=26)
    striche = 8
    for i in range(striche + 1):
        x = x0 + (x1 - x0) * i // striche
        lang = int(K * 0.075) if i % 2 == 0 else int(K * 0.045)
        d.line([(x, y), (x, y + lang)], fill=hell, width=26)
    # die Abspielmarke: ein Dreieck ueber dem Strahl, ein Strich hindurch
    mx = x0 + (x1 - x0) * 3 // striche
    r = int(K * 0.05)
    d.polygon([(mx - r, y - int(r * 1.9)), (mx + r, y - int(r * 1.9)), (mx, y - 6)], fill=rot)
    d.line([(mx, y), (mx, y + int(K * 0.10))], fill=rot, width=22)
    return out


def als_ico(bild: Image.Image, name: str) -> None:
    stufen = [bild.resize((k, k), Image.LANCZOS) for k in GROESSEN]
    stufen[-1].save(ORDNER / name, format="ICO", sizes=[(k, k) for k in GROESSEN],
                    append_images=stufen[:-1])
    print(f"{name}: {len(GROESSEN)} Groessen")


def main() -> int:
    icon = quelle()
    als_ico(stapel(icon), "lumiviz_lvfx2.ico")
    als_ico(marke(icon), "lumiviz_marke.ico")
    als_ico(marke(icon, zeichen_mikrofon), "lumiviz_karaoke.ico")
    als_ico(marke(icon, zeichen_schweif), "lumiviz_trail.ico")
    # Session 120: die Marke fuer die Dateiarten, die noch kein Icon hatten
    als_ico(marke(icon, zeichen_loop), "lumiviz_lvloop.ico")
    als_ico(marke(icon, zeichen_tasten), "lumiviz_lvinst.ico")
    als_ico(marke(icon, zeichen_kontur), "lumiviz_lvcontour.ico")
    als_ico(marke(icon, zeichen_ablage), "lumiviz_lvdata.ico")
    als_ico(marke(icon, zeichen_blende), "lumiviz_lvfade.ico")
    als_ico(marke(icon, zeichen_stueck), "lumiviz_lvshape.ico")
    als_ico(marke(icon, zeichen_stumm), "lumiviz_lvmute.ico")
    als_ico(marke(icon, zeichen_verlauf), "lumiviz_grad.ico")
    # Session 124: Kits (.lvkit)
    als_ico(marke(icon, zeichen_trommel), "lumiviz_lvkit.ico")
    # Session 137: Akkorde (.lvchord)
    als_ico(marke(icon, zeichen_griffbild), "lumiviz_lvchord.ico")
    # Session 147: Bauplan und Koerper eines Instruments (.lvinstbodyplan, .lvinstbody)
    als_ico(marke(icon, zeichen_bauplan), "lumiviz_lvinstbodyplan.ico")
    als_ico(marke(icon, zeichen_koerper), "lumiviz_lvinstbody.ico")
    # Session 167: Muster, Clip und Spur als Dateien (.lvpattern, .lvclip, .lvtrack)
    als_ico(marke(icon, zeichen_muster), "lumiviz_lvpattern.ico")
    als_ico(marke(icon, zeichen_clip), "lumiviz_lvclip.ico")
    als_ico(marke(icon, zeichen_spur), "lumiviz_lvtrack.ico")
    # Session 167: die Sammlungen der mitgelieferten Akkorde (.lvchords, nach I1 a)
    als_ico(marke(icon, zeichen_griffbilder), "lumiviz_lvchords.ico")
    # Session 167: die Presets der klassischen Visualizer (.lvp, nach I1 a)
    als_ico(marke(icon, zeichen_balken), "lumiviz_lvp.ico")
    als_ico(faecher(icon), "lumiviz_faecher.ico")
    als_ico(komposition(icon), "lumiviz_lvcomp.ico")
    # Das Icon der App (Session 100): dasselbe Bild wie `lumiviz.ico`, auf ein
    # Quadrat gesetzt und in den sieben ueblichen Stufen — so nimmt es der
    # Ressourcen-Compiler fuer die Exe (`projects/apps/LumiViz/main/LumiViz.rc`).
    als_ico(quadrat(icon, K), "lumiviz_app.ico")
    return 0


if __name__ == "__main__":
    sys.exit(main())
