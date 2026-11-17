"""Text aus abgelegter HTML-Datei, Suche nach Begriffen. python lies.py <datei> <kontext> <begriff>..."""
import html
import re
import sys

d, k, bs = sys.argv[1], int(sys.argv[2]), sys.argv[3:]
t = open(d, "rb").read().decode("utf-8", errors="replace")
t = re.sub(r"(?is)<(script|style)[^>]*>.*?</\1>", " ", t)
t = re.sub(r"(?i)<br\s*/?>|</p>|</tr>|</li>|</h\d>|</div>", "\n", t)
t = re.sub(r"<[^>]+>", " ", t)
t = html.unescape(t)
t = re.sub(r"[ \t\r\f\v]+", " ", t)
t = re.sub(r"\n\s*\n+", "\n", t)
for b in bs:
    last = -10**9
    n = 0
    for m in re.finditer(re.escape(b), t, re.I):
        if m.start() - last < k:
            continue
        last = m.start()
        print(f"--- [{b}]", t[max(0, m.start() - k):m.start() + k].replace("\n", " / "))
        n += 1
        if n >= 3:
            break
    if n == 0:
        print(f"--- [{b}] kein Treffer")
