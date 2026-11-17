"""Zieht aus der abgelegten Google-Patents-Seite (abrufe/googlepatents_US7161077.html) Titel, Erfinder und alle
Saetze mit Massangaben (inch, mm, %) — Wortlaut, ohne Umformung."""
import html
import os
import re

pfad = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe", "googlepatents_US7161077.html")
roh = open(pfad, encoding="utf-8", errors="replace").read()
for feld in ("DC.title", "DC.contributor", "DC.date"):
    for m in re.finditer(r'<meta name="%s" content="([^"]*)"' % re.escape(feld), roh):
        print(feld, ":", html.unescape(m.group(1)))
text = re.sub(r"<script.*?</script>|<style.*?</style>", " ", roh, flags=re.S)
text = html.unescape(re.sub(r"<[^>]+>", " ", text))
text = re.sub(r"\s+", " ", text)
start = text.find("Description")
saetze = re.split(r"(?<=[.;])\s+", text[start:])
gesehen = set()
for s in saetze:
    if re.search(r"\d\s*(inch|inches|mm|%|\")", s) and s not in gesehen:
        gesehen.add(s)
        print("-", s[:400])
