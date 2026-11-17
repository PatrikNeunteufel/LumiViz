"""Zieht aus den lokalen Edinburgh-St-Cecilia's-Seiten Titel und die Abschnitte um 'pluck' als Klartext."""
import glob
import html
import re

ORDNER = r"C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\gezupfte-saite"

for pfad in sorted(glob.glob(ORDNER + r"\Edinburgh_StCecilias_*.html")):
    roh = open(pfad, encoding="utf-8", errors="replace").read()
    titel = re.search(r"<title>(.*?)</title>", roh, re.S)
    text = re.sub(r"<script.*?</script>|<style.*?</style>", " ", roh, flags=re.S)
    text = re.sub(r"<br\s*/?>|</p>|</tr>|</li>", "\n", text)
    text = html.unescape(re.sub(r"<[^>]+>", " ", text))
    text = re.sub(r"[ \t]+", " ", text)
    print("=" * 30, pfad.rsplit("\\", 1)[-1], "|", titel.group(1).strip() if titel else "")
    for m in re.finditer(r"(?i)pluck", text):
        print("---", text[max(0, m.start() - 300): m.start() + 1500].strip())
