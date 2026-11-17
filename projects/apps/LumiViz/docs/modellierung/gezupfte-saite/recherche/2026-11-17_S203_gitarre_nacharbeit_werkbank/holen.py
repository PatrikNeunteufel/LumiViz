"""Roh lesen mit Ablage (CLAUDE.md, Downloads): python holen.py <url> <dateiname>
Legt unter abrufe/ ab, prueft Groesse (<= 20 MB), schreibt eine Zeile in abrufe/INDEX.md."""
import datetime
import hashlib
import pathlib
import sys
import urllib.request

ORDNER = pathlib.Path(__file__).parent / "abrufe"
GRENZE = 20 * 1024 * 1024
UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0 Safari/537.36"


def main():
    url, name = sys.argv[1], sys.argv[2]
    ORDNER.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": UA, "Accept": "*/*"})
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            laenge = r.headers.get("Content-Length")
            if laenge and int(laenge) > GRENZE:
                print(f"ABBRUCH: {laenge} Bytes > 20 MB")
                return
            daten = r.read(GRENZE + 1)
            typ = r.headers.get("Content-Type")
            end_url = r.geturl()
    except Exception as e:  # HTTP-Fehler melden, nichts ablegen
        print(f"FEHLER: {e}")
        return
    if len(daten) > GRENZE:
        print("ABBRUCH: > 20 MB")
        return
    ziel = ORDNER / name
    ziel.write_bytes(daten)
    sha = hashlib.sha256(daten).hexdigest()
    zeit = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
    index = ORDNER / "INDEX.md"
    if not index.exists():
        index.write_text("# Abrufe (vom Lauf roh gelesen)\n\n| Datei | URL | Abrufzeit | Bytes | SHA-256 |\n|---|---|---|---|---|\n",
                         encoding="utf-8")
    with index.open("a", encoding="utf-8") as f:
        f.write(f"| `{name}` | <{end_url}> | {zeit} | {len(daten)} | `{sha}` |\n")
    print(f"OK {name} {len(daten)} Bytes, {typ}, {end_url}, sha256 {sha[:16]}")


main()
