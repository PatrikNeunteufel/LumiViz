"""Laedt Edinburgher Dissertationen (Lippen-Messungen) bis 20 MB nach abrufe/; groessere nur melden."""
import hashlib
import os
import urllib.request

BASIS = "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/"
DATEIEN = {
    "Newton_Michael__PhDThesis_UniversityOfEdinburgh_2009.pdf": "Newton_2009_PhD_Edinburgh.pdf",
    "Stevenson_Samuel__PhDThesis_UniversityOfEdinburgh_2009.pdf": "Stevenson_2009_PhD_Edinburgh.pdf",
    "Bromage_Seona__PhDThesis_UniversityOfEdinburgh_2007.pdf": "Bromage_2007_PhD_Edinburgh.pdf",
}
ZIEL = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
for quelle, name in DATEIEN.items():
    url = BASIS + quelle
    kopf = urllib.request.Request(url, method="HEAD", headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(kopf, timeout=60) as r:
        groesse = int(r.headers.get("Content-Length", "0"))
    print(url, groesse, "Bytes")
    if groesse > 20 * 1024 * 1024 or groesse == 0:
        print("   nicht geladen (Groesse)")
        continue
    with urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"}), timeout=300) as r:
        daten = r.read()
    with open(os.path.join(ZIEL, name), "wb") as f:
        f.write(daten)
    print("   ->", name, len(daten), "SHA-256", hashlib.sha256(daten).hexdigest())
