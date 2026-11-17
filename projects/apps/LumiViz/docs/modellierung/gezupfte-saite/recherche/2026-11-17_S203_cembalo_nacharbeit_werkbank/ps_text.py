"""Text aus einer dvips-PostScript-Datei grob ziehen: python ps_text.py <ps> <txt>
Liest die Zeichenketten in (...) je Seite (%%Page) und schreibt sie hintereinander.
Wortabstaende setzt dvips als Bewegungsbefehle; zwischen Zeichenketten steht deshalb ein Leerzeichen.
Nur zum Eingrenzen — Zahlen werden an der Seite als Bild nicht geprueft (kein Renderer erlaubt).
"""
import re
import sys

roh = open(sys.argv[1], "rb").read().decode("latin-1")
seiten = re.split(r"%%Page:", roh)
aus = []
str_re = re.compile(r"\((?:\\.|[^\\)])*\)")
for i, s in enumerate(seiten[1:], start=1):
    teile = []
    for m in str_re.finditer(s):
        t = m.group(0)[1:-1]
        t = re.sub(r"\\([0-7]{3})", lambda x: chr(int(x.group(1), 8)), t)
        t = t.replace("\\(", "(").replace("\\)", ")").replace("\\\\", "\\")
        teile.append(t)
    aus.append(f"\n===== Seite {i} =====\n" + " ".join(teile))
open(sys.argv[2], "w", encoding="utf-8").write("".join(aus))
print(len(seiten) - 1, "Seiten")
