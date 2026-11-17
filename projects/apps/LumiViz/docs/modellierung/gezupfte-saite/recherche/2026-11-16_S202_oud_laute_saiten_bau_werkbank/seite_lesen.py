"""Liest eine Webseite (HTML) und gibt Text um Suchbegriffe aus. Speichert nichts.

Aufruf: python seite_lesen.py <url> [--kontext 300] [--max 15] [--alles] -- <begriff> [<begriff> ...]
"""
import html
import re
import sys
import urllib.request


def text_von(url: str) -> str:
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (research client)"})
    with urllib.request.urlopen(req, timeout=40) as r:
        roh = r.read()
        cs = r.headers.get_content_charset() or "utf-8"
    t = roh.decode(cs, errors="replace")
    t = re.sub(r"(?is)<(script|style)[^>]*>.*?</\1>", " ", t)
    t = re.sub(r"(?i)<br\s*/?>|</p>|</tr>|</li>|</h\d>|</div>", "\n", t)
    t = re.sub(r"(?i)</td>|</th>", " | ", t)
    t = re.sub(r"<[^>]+>", " ", t)
    t = html.unescape(t)
    t = re.sub(r"[ \t\r\f\v]+", " ", t)
    t = re.sub(r"\n\s*\n+", "\n", t)
    return t


def main() -> None:
    a = sys.argv[1:]
    if "--" in a:
        i = a.index("--")
        opts, begriffe = a[:i], a[i + 1:]
    else:
        opts, begriffe = a, []
    url = opts[0]
    kontext = int(opts[opts.index("--kontext") + 1]) if "--kontext" in opts else 300
    maxn = int(opts[opts.index("--max") + 1]) if "--max" in opts else 15
    t = text_von(url)
    print(f"# {url}  ({len(t)} Zeichen Text)")
    if "--alles" in opts or not begriffe:
        print(t[:kontext * 40])
        return
    for b in begriffe:
        treffer = [m.start() for m in re.finditer(re.escape(b), t, re.I)]
        print(f"\n## '{b}': {len(treffer)} Treffer")
        letzte = -10**9
        n = 0
        for p in treffer:
            if p - letzte < kontext:
                continue
            letzte = p
            print("---", t[max(0, p - kontext):p + kontext].replace("\n", " / "))
            n += 1
            if n >= maxn:
                break


if __name__ == "__main__":
    main()
