"""Holt eine URL roh, legt sie unter abrufe/ ab und trägt sie in abrufe/INDEX.md ein.
Aufruf: python hole.py <url> <dateiname> [--text]
--text: gibt bei HTML den sichtbaren Text (gekürzt) aus, bei PDF nur Größe/Kopf."""
import sys, os, hashlib, datetime, urllib.request, ssl, re, html

ORDNER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
os.makedirs(ORDNER, exist_ok=True)

def main():
    url, name = sys.argv[1], sys.argv[2]
    text = "--text" in sys.argv
    maxzeichen = 12000
    for a in sys.argv:
        if a.startswith("--max="):
            maxzeichen = int(a[6:])
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (research client)"})
    ctx = ssl.create_default_context()
    try:
        with urllib.request.urlopen(req, timeout=60, context=ctx) as r:
            daten = r.read(20 * 1024 * 1024 + 1)
            typ = r.headers.get("Content-Type", "")
            endurl = r.geturl()
    except Exception as e:
        print("FEHLER", url, repr(e))
        return 1
    if len(daten) > 20 * 1024 * 1024:
        print("ZU GROSS, nicht abgelegt")
        return 1
    pfad = os.path.join(ORDNER, name)
    with open(pfad, "wb") as f:
        f.write(daten)
    sha = hashlib.sha256(daten).hexdigest()
    zeit = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
    idx = os.path.join(ORDNER, "INDEX.md")
    neu = not os.path.exists(idx)
    with open(idx, "a", encoding="utf-8") as f:
        if neu:
            f.write("# Abrufe des Laufs (hole.py)\n\n| Datei | URL | Zeit | Bytes | SHA-256 |\n|---|---|---|---|---|\n")
        f.write(f"| {name} | {url} | {zeit} | {len(daten)} | {sha} |\n")
    print(f"OK {name} {len(daten)} B, Typ {typ}, End-URL {endurl}")
    if text and daten[:4] != b"%PDF":
        s = daten.decode("utf-8", errors="replace")
        s = re.sub(r"(?is)<(script|style).*?</\1>", " ", s)
        s = re.sub(r"(?s)<[^>]+>", " ", s)
        s = html.unescape(s)
        s = re.sub(r"\s+", " ", s)
        such = [a[8:] for a in sys.argv if a.startswith("--suche=")]
        if such:
            for m in re.finditer(such[0], s, re.I):
                print("…", s[max(0, m.start() - 300):m.start() + maxzeichen], "…\n")
        else:
            print(s[:maxzeichen])
    return 0

if __name__ == "__main__":
    sys.exit(main())
