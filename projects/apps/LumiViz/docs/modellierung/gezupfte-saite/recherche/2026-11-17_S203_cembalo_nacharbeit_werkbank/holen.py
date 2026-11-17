"""Roh lesen mit Ablage: python holen.py <url> <dateiname>
Legt die Datei unter abrufe/<dateiname> ab, traegt sie in abrufe/INDEX.md ein,
und schreibt bei HTML zusaetzlich einen Textauszug <dateiname>.txt (nur zum Lesen).
"""
import datetime
import hashlib
import html
import pathlib
import re
import sys
import urllib.request
import urllib.error

ORDNER = pathlib.Path(__file__).resolve().parent / "abrufe"
MAX = 20 * 1024 * 1024


def text_aus_html(roh: bytes) -> str:
    s = roh.decode("utf-8", errors="replace")
    s = re.sub(r"(?is)<(script|style)[^>]*>.*?</\1>", " ", s)
    s = re.sub(r"(?i)<br\s*/?>|</p>|</tr>|</h\d>|</li>|</div>", "\n", s)
    s = re.sub(r"(?i)</t[dh]>", " | ", s)
    s = re.sub(r"<[^>]+>", " ", s)
    s = html.unescape(s)
    s = re.sub(r"[ \t\r\f\v]+", " ", s)
    s = re.sub(r"\n\s*\n+", "\n", s)
    return s


def main() -> int:
    url, name = sys.argv[1], sys.argv[2]
    ORDNER.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) research-reader",
        "Accept": "*/*",
    })
    try:
        with urllib.request.urlopen(req, timeout=90) as r:
            typ = r.headers.get("Content-Type", "")
            laenge = r.headers.get("Content-Length")
            if laenge and int(laenge) > MAX:
                print(f"ZU GROSS: {laenge} Bytes")
                return 2
            daten = r.read(MAX + 1)
            endurl = r.geturl()
            status = r.status
    except urllib.error.HTTPError as e:
        print(f"HTTP {e.code} {e.reason} fuer {url}")
        return 3
    except Exception as e:  # noqa: BLE001
        print(f"FEHLER {type(e).__name__}: {e}")
        return 4
    if len(daten) > MAX:
        print("ZU GROSS (> 20 MB), nicht abgelegt")
        return 2
    ziel = ORDNER / name
    ziel.write_bytes(daten)
    sha = hashlib.sha256(daten).hexdigest()
    zeit = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
    idx = ORDNER / "INDEX.md"
    if not idx.exists():
        idx.write_text("| Datei | URL | Abrufzeit | Größe (Bytes) | SHA-256 |\n|---|---|---|---|---|\n",
                       encoding="utf-8")
    with idx.open("a", encoding="utf-8") as f:
        f.write(f"| {name} | {endurl} | {zeit} | {len(daten)} | {sha} |\n")
    print(f"HTTP {status}, {typ}, {len(daten)} Bytes -> {ziel}")
    if daten[:5] == b"%PDF-":
        print("PDF")
    elif b"<html" in daten[:2000].lower() or "html" in typ:
        t = text_aus_html(daten)
        (ORDNER / (name + ".txt")).write_text(t, encoding="utf-8")
        print(f"Text: {len(t)} Zeichen -> {name}.txt")
        if re.search(r"(?i)cloudflare|captcha|just a moment", t[:3000]):
            print("WARNUNG: Abfrageseite (Cloudflare/Captcha)?")
    return 0


if __name__ == "__main__":
    sys.exit(main())
