"""Gibt zu Zeilennummern einer pdftotext-Ausgabe die PDF-Seite aus (Seitenwechsel = Formfeed)."""
import sys

AUFTRAG = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_03_gitarre_fernfeld_zweite_richtung"

ABFRAGEN = {
    "scherrer2013.txt": [2541, 3797, 3803, 3822, 3864],
    "traube2004.txt": [2715, 2848, 6333, 9302, 9527],
    "kma38.txt": [1235, 1270, 1582, 2201],
    "woodhouse2012.txt": [170, 177, 256, 280, 437, 449],
    "elie2012.txt": [867, 1004],
    "bank2010.txt": [411, 423],
}

for datei, zeilen in ABFRAGEN.items():
    with open(AUFTRAG + "\\" + datei, encoding="utf-8", errors="replace") as f:
        text = f.read().split("\n")
    seite = 1
    seite_je_zeile = []
    for zeile in text:
        seite_je_zeile.append(seite)
        seite += zeile.count("\f")
    for z in zeilen:
        print(f"{datei}: Zeile {z} -> PDF-Seite {seite_je_zeile[z - 1]}")
sys.exit(0)
