"""Trägt die mit curl abgelegten Dateien in abrufe/INDEX.md ein (Datei, URL, Zeit der Datei, Bytes, SHA-256, Vermerk)."""
import os, hashlib, datetime
ORDNER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
EINTRAEGE = [
    ("Degirmenli_2023_TinisalNitelikler_Idil104.pdf", "https://www.idildergisi.com/makale/pdf/1684998615.pdf", "PDF, gelesen"),
    ("daddario_arabic_oud.html", "https://www.daddario.com/products/guitar/more-instruments/arabic-oud/ (weitergeleitet auf https://www.daddario.com/products/arabic-oud-strings-ej95a)", "HTML, Zugtabelle EJ95A"),
    ("DAddario_TensionChart_13934.pdf", "https://www.daddario.com/globalassets/pdfs/accessories/tension_chart_13934.pdf", "KEIN PDF: der Server lieferte eine HTML-Seite; nicht lesbar"),
    ("dergipark_3966895.pdf", "https://dergipark.org.tr/en/download/article-file/3966895", "PDF, Sahinkayasi u. a. 2024 (Baglama), nicht einschlägig"),
    ("dergipark_1751498.pdf", "https://dergipark.org.tr/en/download/article-file/1751498", "PDF, Übersichtsartikel zur Oud, ohne Messwerte"),
    ("dergipark_433700.pdf", "https://dergipark.org.tr/en/download/article-file/433700", "PDF, nicht einschlägig (Signalverarbeitung)"),
    ("emirdegirmenli_soundboard_brace_tuning.html", "https://www.emirdegirmenli.com/en/oud-workshop/soundboard-brace-tuning/", "HTTP 404, Fehlerseite"),
    ("actaacustica_aacus250198_abs.html", "https://acta-acustica.edpsciences.org/articles/aacus/abs/2026/01/aacus250198/aacus250198.html", "HTTP 403, gesperrt"),
    ("yok_tez_degirmenli.html", "https://tez.yok.gov.tr/UlusalTezMerkezi/tezSorguSonucYeni.jsp", "nur die Suchmaske der Tez Merkezi, kein Treffer"),
]
idx = os.path.join(ORDNER, "INDEX.md")
with open(idx, "a", encoding="utf-8") as f:
    f.write("\n## Mit curl abgelegt (Nachtrag durch index_nachtrag.py)\n\n| Datei | URL | Zeit (Datei) | Bytes | SHA-256 | Vermerk |\n|---|---|---|---|---|---|\n")
    for name, url, vermerk in EINTRAEGE:
        p = os.path.join(ORDNER, name)
        if not os.path.exists(p):
            f.write(f"| {name} | {url} | — | — | — | fehlt |\n")
            continue
        d = open(p, "rb").read()
        zeit = datetime.datetime.fromtimestamp(os.path.getmtime(p)).strftime("%Y-%m-%d %H:%M")
        f.write(f"| {name} | {url} | {zeit} | {len(d)} | {hashlib.sha256(d).hexdigest()} | {vermerk} |\n")
        print(name, len(d))
