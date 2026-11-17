# Abb. 7.6 (Bell 1988, PDF 293): Seite als Graustufe (PGM) ablegen und Gitterlinien/Ticks finden.
# Aufruf: python d1_abb76_geometrie.py
import subprocess, pathlib

PDF = r"C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\gezupfte-saite\Bell_1988_AcousticalInvestigationConcertHarp_PhD.pdf"
HIER = pathlib.Path(__file__).parent
ZIEL = HIER / "seiten" / "bell293_120"
DPI = 120


def lade_pgm(pfad):
    daten = pfad.read_bytes()
    # P5-Kopf: P5\n<w> <h>\n<max>\n
    teile = []
    i = 0
    while len(teile) < 4:
        while daten[i:i + 1].isspace():
            i += 1
        j = i
        while not daten[j:j + 1].isspace():
            j += 1
        teile.append(daten[i:j])
        i = j
    i += 1
    w, h = int(teile[1]), int(teile[2])
    pix = daten[i:i + w * h]
    return w, h, pix


def main():
    subprocess.run(["pdftoppm", "-r", str(DPI), "-f", "293", "-l", "293", "-gray", PDF, str(ZIEL)], check=True)
    pgm = sorted(ZIEL.parent.glob("bell293_120*.pgm"))[0]
    w, h, pix = lade_pgm(pgm)
    print("Datei", pgm.name, "Groesse", w, h)
    dunkel = lambda x, y: pix[y * w + x] < 128
    # horizontale Linien: Zeilen mit vielen dunklen Pixeln zwischen x 150 und 800
    for y in range(h):
        n = sum(1 for x in range(150, 800) if dunkel(x, y))
        if n > 300:
            print("Zeile", y, "dunkel", n)
    # Ticks unter Panel 1: Spalten mit dunklen Pixeln in einem Band knapp ueber der Achsenbeschriftung
    for yband in (range(445, 462),):
        cols = [x for x in range(130, 830) if any(dunkel(x, y) for y in yband)]
        print("Band", yband, "Spalten (gruppiert):")
        gruppe = []
        for x in cols:
            if gruppe and x - gruppe[-1] > 2:
                print("  ", gruppe[0], "-", gruppe[-1])
                gruppe = []
            gruppe.append(x)
        if gruppe:
            print("  ", gruppe[0], "-", gruppe[-1])


main()
