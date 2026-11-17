"""Fuegt bericht.md aus den Teilen, den Skripten und ihren Ausgaben zusammen."""
import pathlib
import subprocess
import sys

D = pathlib.Path(__file__).parent


def lies(n):
    return (D / n).read_text(encoding="utf-8")


def lauf(skript):
    r = subprocess.run([sys.executable, str(D / skript)], capture_output=True, text=True, encoding="utf-8",
                       cwd=str(D))
    if r.returncode != 0:
        raise SystemExit(f"{skript} fehlgeschlagen:\n{r.stderr}")
    return r.stdout


teile = [lies("teil_kopf.md"), lies("teil_gleichungen.md"), lies("teil_haupt.md")]
anhang = ["## Anhang: Skripte und Ausgaben\n"]
for skript in ("tmm.py", "pruefung.py", "rechnung.py", "zusatz.py"):
    anhang.append(f"### `{skript}`\n\n```python\n{lies(skript).rstrip()}\n```\n")
anhang.append("### Ausgabe `python pruefung.py`\n\n```text\n" + lauf("pruefung.py").rstrip() + "\n```\n")
anhang.append("### Ausgabe `python zusatz.py`\n\n" + lauf("zusatz.py").rstrip() + "\n")
anhang.append("### Ausgabe `python rechnung.py alles` (Datei `ausgabe_alles.md`, Lauf vom 2026-11-17, ≈ 15 min)\n\n"
              + lies("ausgabe_alles.md").rstrip() + "\n")
(D / "bericht.md").write_text("\n\n".join(teile + anhang), encoding="utf-8")
print("bericht.md geschrieben:", (D / "bericht.md").stat().st_size, "Bytes")
