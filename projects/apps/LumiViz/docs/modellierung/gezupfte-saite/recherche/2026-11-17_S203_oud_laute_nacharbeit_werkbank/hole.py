"""Laedt Quellen roh nach abrufe/ und schreibt Zeilen fuer INDEX.md. Aufruf: python hole.py <name>=<url> ..."""
import hashlib
import os
import sys
import time
import urllib.request

ZIEL = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-17_25_oud_laute_nacharbeit\abrufe"
os.makedirs(ZIEL, exist_ok=True)
for arg in sys.argv[1:]:
    name, url = arg.split("=", 1)
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (research client)"})
        with urllib.request.urlopen(req, timeout=60) as r:
            roh = r.read(20_000_001)
        if len(roh) > 20_000_000:
            print("ZU GROSS", name)
            continue
        pfad = os.path.join(ZIEL, name)
        with open(pfad, "wb") as f:
            f.write(roh)
        h = hashlib.sha256(roh).hexdigest()
        z = time.strftime("%Y-%m-%d %H:%M")
        print(f"| {name} | {url} | {z} | {len(roh)} | {h} |")
    except Exception as e:
        print("FEHLER", name, url, repr(e)[:150])
