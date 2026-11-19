"""Text aus einer abgelegten HTML-Datei, Stellen um einen regulären Ausdruck.
Aufruf: python lies.py <datei> <regex> [zeichen=800] [max_treffer=6]"""
import sys, re, html
pfad, muster = sys.argv[1], sys.argv[2]
n = int(sys.argv[3]) if len(sys.argv) > 3 else 800
k = int(sys.argv[4]) if len(sys.argv) > 4 else 6
s = open(pfad, "rb").read().decode("utf-8", errors="replace")
s = re.sub(r"(?is)<(script|style).*?</\1>", " ", s)
s = re.sub(r"(?s)<[^>]+>", " ", s)
s = html.unescape(s)
s = re.sub(r"\s+", " ", s)
for i, m in enumerate(re.finditer(muster, s, re.I)):
    if i >= k:
        break
    print(f"[{m.start()}] …{s[max(0, m.start() - 200):m.start() + n]}…\n")
print("Textlänge", len(s))
