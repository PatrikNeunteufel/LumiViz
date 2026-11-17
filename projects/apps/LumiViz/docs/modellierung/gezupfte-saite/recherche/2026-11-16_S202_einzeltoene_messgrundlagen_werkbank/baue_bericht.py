"""Fuehrt gegenrechnung.py ganz aus, legt die Ausgabe in gegenrechnung_ausgabe.txt ab und setzt Skript und Ausgabe
an den Platzhaltern {{SKRIPT}} und {{AUSGABE}} in bericht.md ein (je genau einmal)."""
import os
import pathlib
import subprocess
import sys

hier = pathlib.Path(__file__).resolve().parent
umgebung = dict(os.environ, PYTHONIOENCODING="utf-8")
lauf = subprocess.run([sys.executable, str(hier / "gegenrechnung.py")], capture_output=True, text=True,
                      encoding="utf-8", cwd=str(hier), env=umgebung)
if lauf.returncode != 0:
    print(lauf.stderr)
    sys.exit(1)
ausgabe = lauf.stdout.rstrip("\n")
(hier / "gegenrechnung_ausgabe.txt").write_text(ausgabe + "\n", encoding="utf-8")
skript = (hier / "gegenrechnung.py").read_text(encoding="utf-8").rstrip("\n")
bericht = (hier / "bericht.md").read_text(encoding="utf-8")
for marke, text in (("{{SKRIPT}}", skript), ("{{AUSGABE}}", ausgabe)):
    if bericht.count(marke) != 1:
        print(f"Platzhalter {marke} nicht genau einmal")
        sys.exit(1)
    bericht = bericht.replace(marke, text)
(hier / "bericht.md").write_text(bericht, encoding="utf-8")
print(f"eingesetzt: Skript {len(skript.splitlines())} Zeilen, Ausgabe {len(ausgabe.splitlines())} Zeilen")
