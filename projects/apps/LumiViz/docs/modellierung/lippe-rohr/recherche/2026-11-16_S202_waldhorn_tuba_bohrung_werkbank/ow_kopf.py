"""Liest Textdateien aus dem OpenWInD-Repo (nur in den Speicher) und zeigt Kopf, Ende und Umfang."""
import sys
import urllib.parse
import urllib.request

BASIS = "https://gitlab.inria.fr/api/v4/projects/openwind%2Fopenwind/repository/files/{}/raw?ref=master"

for pfad in sys.argv[1:]:
    url = BASIS.format(urllib.parse.quote(pfad, safe=""))
    text = urllib.request.urlopen(url, timeout=60).read().decode("utf-8", "replace")
    zeilen = text.splitlines()
    print(f"##### {pfad}: {len(zeilen)} Zeilen, {len(text)} Zeichen")
    for z in zeilen[:40]:
        print("  " + z)
    if len(zeilen) > 55:
        print("  …")
        for z in zeilen[-15:]:
            print("  " + z)
    elif len(zeilen) > 40:
        for z in zeilen[40:]:
            print("  " + z)
