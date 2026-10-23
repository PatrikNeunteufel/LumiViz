"""Liest einzelne Dateien aus einem ZIP im Netz ueber HTTP-Range, ohne das ganze Archiv zu laden.

    python -I fernzip.py <url> list [muster]
    python -I fernzip.py <url> get <name-im-zip> <zielordner>
"""
import io
import sys
import time
import urllib.error
import urllib.request
import zipfile
from pathlib import Path


class FernDatei(io.RawIOBase):
    def __init__(self, url):
        self.url = url
        req = urllib.request.Request(url, headers={"Range": "bytes=0-0"})
        with urllib.request.urlopen(req) as r:
            self.groesse = int(r.headers["Content-Range"].split("/")[1])
        self.pos = 0
        self.cache = {}

    def seekable(self):
        return True

    def readable(self):
        return True

    def tell(self):
        return self.pos

    def seek(self, off, whence=0):
        if whence == 0:
            self.pos = off
        elif whence == 1:
            self.pos += off
        else:
            self.pos = self.groesse + off
        return self.pos

    def read(self, n=-1):
        if n is None or n < 0:
            n = self.groesse - self.pos
        if n == 0 or self.pos >= self.groesse:
            return b""
        ende = min(self.pos + n, self.groesse) - 1
        req = urllib.request.Request(self.url, headers={"Range": f"bytes={self.pos}-{ende}"})
        for versuch in range(20):
            try:
                with urllib.request.urlopen(req) as r:
                    daten = r.read()
                break
            except urllib.error.HTTPError as e:
                if e.code != 429:
                    raise
                warte = int(e.headers.get("retry-after", "30")) + 2
                print("429, warte", warte, file=sys.stderr)
                time.sleep(warte)
        self.pos += len(daten)
        return daten

    def readinto(self, b):
        d = self.read(len(b))
        b[: len(d)] = d
        return len(d)


def main():
    url, befehl = sys.argv[1], sys.argv[2]
    f = io.BufferedReader(FernDatei(url), buffer_size=1 << 20)
    z = zipfile.ZipFile(f)
    if befehl == "list":
        muster = sys.argv[3].lower() if len(sys.argv) > 3 else ""
        for i in z.infolist():
            if muster in i.filename.lower():
                print(i.filename, i.file_size)
    elif befehl == "get":
        name, ziel = sys.argv[3], Path(sys.argv[4])
        ziel.mkdir(parents=True, exist_ok=True)
        daten = z.read(name)
        (ziel / Path(name).name).write_bytes(daten)
        print("ok", name, len(daten))
    elif befehl == "getliste":
        # Liste: je Zeile "<name-im-zip>\t<zielordner>"
        for zeile in Path(sys.argv[3]).read_text(encoding="utf-8").splitlines():
            if not zeile.strip():
                continue
            name, ziel = zeile.split("\t")
            ziel = Path(ziel)
            ziel.mkdir(parents=True, exist_ok=True)
            pfad = ziel / Path(name).name
            if pfad.exists():
                continue
            try:
                pfad.write_bytes(z.read(name))
                print("ok", name, flush=True)
            except KeyError:
                print("FEHLT", name, flush=True)


if __name__ == "__main__":
    main()
