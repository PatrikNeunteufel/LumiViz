"""Inhalt der lokalen Quellen feststellen: pdfinfo (Titel, Seiten) und Anfang von Seite 1."""
import subprocess
import pathlib

Q = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\lippe-rohr")
for pdf in sorted(Q.glob("*.pdf")):
    info = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True, encoding="utf-8", errors="replace").stdout
    zeilen = [z for z in info.splitlines() if z.split(":")[0] in ("Title", "Author", "Pages", "CreationDate")]
    txt = subprocess.run(["pdftotext", "-f", "1", "-l", "1", "-layout", str(pdf), "-"],
                         capture_output=True, text=True, encoding="utf-8", errors="replace").stdout
    kopf = " ".join(txt.split())[:500]
    print("=====", pdf.name)
    for z in zeilen:
        print("  ", z)
    print("   S.1:", kopf)
