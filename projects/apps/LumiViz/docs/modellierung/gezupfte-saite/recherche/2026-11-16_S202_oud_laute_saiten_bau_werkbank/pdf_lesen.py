"""Holt ein PDF in eine temporäre Datei, gibt Text (ganz oder um Begriffe) seitenweise aus und löscht die Datei.

Kein Ablegen im Projekt. Aufruf:
  python pdf_lesen.py <url> [--seiten 1-5] [--kontext 300] [--max 10] [--alles] -- <begriff> ...
"""
import os
import re
import subprocess
import sys
import tempfile
import urllib.request


def main() -> None:
    a = sys.argv[1:]
    if "--" in a:
        i = a.index("--")
        opts, begriffe = a[:i], a[i + 1:]
    else:
        opts, begriffe = a, []
    url = opts[0]
    kontext = int(opts[opts.index("--kontext") + 1]) if "--kontext" in opts else 300
    maxn = int(opts[opts.index("--max") + 1]) if "--max" in opts else 10
    seiten = opts[opts.index("--seiten") + 1] if "--seiten" in opts else None
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (research client)"})
    with urllib.request.urlopen(req, timeout=60) as r:
        daten = r.read()
    if not daten.startswith(b"%PDF"):
        print("Kein PDF erhalten; Anfang:", daten[:300])
        return
    fd, pfad = tempfile.mkstemp(suffix=".pdf")
    os.write(fd, daten)
    os.close(fd)
    try:
        info = subprocess.run(["pdfinfo", pfad], capture_output=True, text=True, encoding="utf-8", errors="replace").stdout
        n = int(re.search(r"Pages:\s+(\d+)", info).group(1))
        print(f"# {url}  ({len(daten)} Bytes, {n} Seiten)")
        erste, letzte = 1, n
        if seiten:
            teile = seiten.split("-")
            erste, letzte = int(teile[0]), int(teile[-1])
        if "--bild" in opts:
            ziel = opts[opts.index("--bild") + 1]
            dpi = opts[opts.index("--dpi") + 1] if "--dpi" in opts else "110"
            os.makedirs(os.path.dirname(ziel), exist_ok=True)
            subprocess.run(["pdftoppm", "-r", dpi, "-f", str(erste), "-l", str(letzte), "-png", pfad, ziel], check=True)
            print("Bilder:", ziel, "-<seite>.png")
            return
        for s in range(erste, min(letzte, n) + 1):
            t =subprocess.run(["pdftotext", "-layout", "-f", str(s), "-l", str(s), pfad, "-"],
                               capture_output=True, text=True, encoding="utf-8", errors="replace").stdout
            if "--alles" in opts:
                print(f"\n===== Seite {s} =====\n{t}")
                continue
            tn = re.sub(r"\s+", " ", t.replace("ﬁ", "fi").replace("ﬂ", "fl"))
            for b in begriffe:
                treffer = [m.start() for m in re.finditer(re.escape(b), tn, re.I)][:maxn]
                for p in treffer:
                    print(f"--- S.{s} '{b}': {tn[max(0, p - kontext):p + kontext]}")
    finally:
        os.remove(pfad)


if __name__ == "__main__":
    main()
