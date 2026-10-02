"""Icons bauen, die aus `lumiviz.ico` abgeleitet sind (Session 88).

Aufruf (normales Python mit Pillow):

    python asset/img/logo/icons/build_lvfx2_icons.py

Erzeugt vier .ico (16–256 px) neben diesem Skript:

- `lumiviz_lvfx2.ico`   „Stapel"  — das Icon vorn, zwei abgedunkelte Kopien
                        versetzt dahinter. Das Datei-Icon fuer `.lvfx2`
                        (Preset-Stapel; Entscheid Patrik S88).
- `lumiviz_marke.ico`   „Marke"   — das Icon ganz, unten rechts eine runde
                        Marke mit drei Ebenen in den Logofarben. Reserve.
- `lumiviz_karaoke.ico` „Marke" mit Mikrofon im Kreis — das Datei-Icon fuer
                        Karaoke-Presets `.lvkar` (Session 89). Die Marke ist
                        eine Familie: je Verwendung ein Zeichen im Kreis.
- `lumiviz_faecher.ico` „Faecher" — drei gleich grosse Icons diagonal
                        gestaffelt. Reserve.

Die Reserve-Icons sind noch ohne Verwendung (Ideen in `README.md`).
`lumiviz.ico` selbst ist Handarbeit und wird nur gelesen.
"""
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
    als_ico(faecher(icon), "lumiviz_faecher.ico")
    return 0


if __name__ == "__main__":
    sys.exit(main())
