"""Liest aus der abgelegten DepositOnce-Seite (doi:10.14279/depositonce-5861.2) Titel, Lizenz, Beschreibung und die
Dateiliste (Name, Groesse) - nur die Beschreibung, keine Daten laden. Gibt Fundstellen zu 'guitar' mit Kontext aus."""
import html
import pathlib
import re

T = (pathlib.Path(__file__).parent / "abrufe" / "depositonce_5861.2.html").read_text(encoding="utf-8")
klar = html.unescape(re.sub(r"<[^>]+>", " ", T))
klar = re.sub(r"\\u002F", "/", klar)
klar = re.sub(r"\s+", " ", klar)

for name in ("citation_title", "citation_author", "citation_publication_date", "citation_date", "DC.rights",
             "citation_license"):
    for m in re.finditer(r'<meta name="%s" content="([^"]*)"' % name, T):
        print(name, ":", html.unescape(m.group(1)))
for m in re.finditer(r'(creativecommons\.org/[^"\s]*)', T):
    print("Lizenzlink:", m.group(1))
    break
print()
for m in re.finditer(r"guitar", klar, re.I):
    a = max(0, m.start() - 300)
    print("...", klar[a:m.end() + 300], "...\n")
print("Dateien mit Groesse (Auszug):")
gesehen = set()
for m in re.finditer(r'"name":"([^"]+)","[^{}]{0,400}?"sizeBytes":(\d+)', T):
    if m.group(1) not in gesehen:
        gesehen.add(m.group(1))
        print(f"  {m.group(1)}  {int(m.group(2)) / 1e6:.1f} MB")
for m in re.finditer(r"([A-Za-z0-9_\-]+\.(?:zip|sofa|daff|tar|7z))\s*\(?\s*([\d.,]+\s*[MG]B)", klar):
    if m.group(1) not in gesehen:
        gesehen.add(m.group(1))
        print(f"  {m.group(1)}  {m.group(2)}")
