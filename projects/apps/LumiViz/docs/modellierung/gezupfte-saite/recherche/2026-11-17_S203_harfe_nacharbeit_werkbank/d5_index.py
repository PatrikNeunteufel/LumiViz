# Prueft Groesse und SHA-256 der roh gelesenen Dateien unter abrufe/ und gibt Zeilen fuer INDEX.md aus.
import hashlib, pathlib

ABRUFE = pathlib.Path(__file__).parent / "abrufe"
for p in sorted(ABRUFE.iterdir()):
    if p.is_file() and p.suffix.lower() in (".pdf", ".html"):
        h = hashlib.sha256(p.read_bytes()).hexdigest()
        print(f"| `{p.name}` | {p.stat().st_size} | {h} |")
