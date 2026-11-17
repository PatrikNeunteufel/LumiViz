"""Baut bericht.md aus teil_haupt.md: rechnet horn_rechnung.py neu und setzt Skripte und Ausgabe als Codebloecke ein."""
import os
import subprocess
import sys

ORDNER = os.path.dirname(os.path.abspath(__file__))
lauf = subprocess.run([sys.executable, os.path.join(ORDNER, "horn_rechnung.py"), ORDNER], capture_output=True, text=True,
                      encoding="utf-8")
if lauf.returncode != 0:
    print(lauf.stderr)
    sys.exit(1)
with open(os.path.join(ORDNER, "horn_rechnung_ausgabe.txt"), "w", encoding="utf-8") as f:
    f.write(lauf.stdout)


def lies(name):
    with open(os.path.join(ORDNER, name), encoding="utf-8") as f:
        return f.read().rstrip()


text = lies("teil_haupt.md")
for marke, datei in (("{{SKRIPT_RECHNUNG}}", "horn_rechnung.py"), ("{{SKRIPT_DIGI88}}", "digitalisiere_abb88.py"),
                     ("{{SKRIPT_KURVE}}", "digitalisiere_kurve.py")):
    text = text.replace(marke, "```python\n" + lies(datei) + "\n```")
text = text.replace("{{AUSGABE_RECHNUNG}}", "```text\n" + lauf.stdout.rstrip() + "\n```")
with open(os.path.join(ORDNER, "bericht.md"), "w", encoding="utf-8") as f:
    f.write(text + "\n")
print("bericht.md geschrieben,", len(text), "Zeichen")
