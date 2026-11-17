"""Laedt eine URL roh (OpenSSL ueber Python statt schannel/curl) und legt sie unter abrufe/<name> ab.
Aufruf: python hole.py <url> <name>"""
import hashlib
import os
import sys
import urllib.request

url, name = sys.argv[1], sys.argv[2]
ziel = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe", name)
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
try:
    with urllib.request.urlopen(req, timeout=120) as r:
        daten = r.read(20 * 1024 * 1024 + 1)
        print("Status", r.status, r.headers.get("Content-Type"))
except Exception as e:  # Fehler melden, nichts umgehen
    print("Fehler:", repr(e))
    sys.exit(2)
if len(daten) > 20 * 1024 * 1024:
    print("groesser als 20 MB - nicht abgelegt")
    sys.exit(3)
with open(ziel, "wb") as f:
    f.write(daten)
print(ziel, len(daten), "Bytes, SHA-256", hashlib.sha256(daten).hexdigest())
