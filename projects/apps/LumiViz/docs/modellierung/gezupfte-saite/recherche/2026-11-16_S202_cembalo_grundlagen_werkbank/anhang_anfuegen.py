"""Haengt rechnung.py und ihre Ausgabe an bericht.md an (einmalig; bricht ab, wenn der Anhang schon gefuellt ist)."""
import pathlib
import subprocess
import sys

HIER = pathlib.Path(__file__).resolve().parent
bericht = HIER / "bericht.md"
skript = HIER / "rechnung.py"
text = bericht.read_text(encoding="utf-8")
if "```python" in text:
    sys.exit("Anhang schon vorhanden")
aus = subprocess.run([sys.executable, str(skript)], capture_output=True, text=True, encoding="utf-8", check=True).stdout
with bericht.open("a", encoding="utf-8", newline="\n") as f:
    f.write("\n### Skript\n\n```python\n" + skript.read_text(encoding="utf-8").rstrip() + "\n```\n")
    f.write("\n### Ausgabe (`python ergebnisse/2026-11-16_14_cembalo_grundlagen/rechnung.py`)\n\n```text\n" + aus.rstrip() + "\n```\n")
print("angehaengt:", len(aus), "Zeichen Ausgabe")
