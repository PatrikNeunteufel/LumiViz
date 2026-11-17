"""Seite roh holen, unter abrufe/ ablegen, INDEX.md ergaenzen, Treffer im Text zeigen.

Aufruf: python holen.py <url> <dateiname> [begriff ...]
"""
import hashlib
import html
import re
import sys
import urllib.request
from datetime import datetime
from pathlib import Path

ZIEL = Path(__file__).resolve().parent / "abrufe"
ZIEL.mkdir(exist_ok=True)
UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120 Safari/537.36"


def holen(url, name, begriffe):
    req = urllib.request.Request(url, headers={"User-Agent": UA, "Accept-Language": "de,en;q=0.8"})
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            daten = r.read()
            status = r.status
    except urllib.error.HTTPError as e:
        print(f"HTTP {e.code} {url}")
        hdr = dict(e.headers)
        for k in ("Cf-Mitigated", "Server"):
            if k in hdr:
                print(f"  {k}: {hdr[k]}")
        return
    except Exception as e:  # noqa: BLE001
        print(f"FEHLER {type(e).__name__}: {e} {url}")
        return
    if len(daten) > 20 * 1024 * 1024:
        print("zu gross, nicht abgelegt")
        return
    pfad = ZIEL / name
    pfad.write_bytes(daten)
    sha = hashlib.sha256(daten).hexdigest()
    zeit = datetime.now().strftime("%Y-%m-%d %H:%M")
    idx = ZIEL / "INDEX.md"
    if not idx.exists():
        idx.write_text("| Datei | URL | Abrufzeit | Größe | SHA-256 |\n|---|---|---|---|---|\n", encoding="utf-8")
    with idx.open("a", encoding="utf-8") as f:
        f.write(f"| {name} | {url} | {zeit} | {len(daten)} | {sha} |\n")
    print(f"HTTP {status}, {len(daten)} Bytes -> {pfad.name}")
    if name.lower().endswith(".pdf"):
        return
    text = daten.decode("utf-8", errors="replace")
    text = re.sub(r"(?is)<(script|style)[^>]*>.*?</\1>", " ", text)
    text = re.sub(r"(?s)<[^>]+>", " ", text)
    text = html.unescape(text)
    text = re.sub(r"\s+", " ", text)
    for b in begriffe:
        treffer = [m.start() for m in re.finditer(re.escape(b), text, re.I)]
        print(f"--- {b}: {len(treffer)} Treffer")
        gezeigt = -1000
        for t in treffer[:12]:
            if t - gezeigt < 250:
                continue
            gezeigt = t
            print("   …" + text[max(0, t - 200): t + 250] + "…")


if __name__ == "__main__":
    holen(sys.argv[1], sys.argv[2], sys.argv[3:])
