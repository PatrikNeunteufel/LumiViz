"""Schreibt abrufe/INDEX.md (Datei, URL, Abrufzeit, Groesse, SHA-256) fuer die roh geladenen Dateien dieses Laufs
und druckt je PDF den Anfang der Titelseite (Textebene) zur Pruefung von Autor/Jahr/Titel."""
import datetime
import hashlib
import os
import subprocess

ORDNER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
URL = {
    "Matteoli_2021_MinimalBlowingPressure_arXiv2112.08751.pdf": "https://arxiv.org/pdf/2112.08751",
    "Matteoli_2021_MinimalBlowingPressure_ActaAcustica.pdf": "https://acta-acustica.edpsciences.org/10.1051/aacus/2021049/pdf (403-Seite, kein PDF)",
    "Matteoli_2023_OnePedalNote_FA2023_000530.pdf": "https://dael.euracoustics.org/confs/fa2023/data/articles/000530.pdf",
    "FA2025_000091.pdf": "https://dael.euracoustics.org/confs/fa2025/data/articles/000091.pdf",
    "FA2023_000490.pdf": "https://dael.euracoustics.org/confs/fa2023/data/articles/000490.pdf",
    "ATR_TR-H-0177.pdf": "https://results.atr.jp/35th/techreport/TR-H-0177/TR-H-0177.pdf",
    "Myers_1998_PhD_Edinburgh.pdf": "https://era.ed.ac.uk/server/api/core/bitstreams/2a943b41-3bb4-4f23-82eb-bcd61e984d3b/content",
    "Logie_2012_PhD_Edinburgh.pdf": "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Logie_Shona__PhDThesis_UniversityOfEdinburgh_2012.pdf",
    "Harrison_2018_PhD_Edinburgh.pdf": "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Harrison_Reginald__PhDThesis_UniversityOfEdinburgh_2018.pdf",
    "Newton_2009_PhD_Edinburgh.pdf": "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Newton_Michael__PhDThesis_UniversityOfEdinburgh_2009.pdf",
    "Stevenson_2009_PhD_Edinburgh.pdf": "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Stevenson_Samuel__PhDThesis_UniversityOfEdinburgh_2009.pdf",
    "Bromage_2007_PhD_Edinburgh.pdf": "https://www.acoustics.ed.ac.uk/wp-content/uploads/Theses/Bromage_Seona__PhDThesis_UniversityOfEdinburgh_2007.pdf",
    "googlepatents_US7161077.html": "https://patents.google.com/patent/US7161077B1/en",
    "iwk_hbs_slurs.html": "https://iwk.mdw.ac.at/hbs/",
    "edinburgh_acoustics_phd_theses.html": "https://www.acoustics.ed.ac.uk/?p=387",
    "edinburgh_acoustics_theses.html": "https://www.acoustics.ed.ac.uk/theses/ (404)",
    "ERA_Norman_2013_AppendixB_Antwort401.json": "https://era.ed.ac.uk/server/api/core/bitstreams/7bc115d6-d92a-480c-8aa1-41b26243fcd4/content (401 Authentication is required)",
    "historicbrass_www_Widholm2016_Antwort404.html": "https://www.historicbrass.org/edocman/hbj-2016/HBSJ_2016_JL01_010_Widholm.pdf (404)",
    "ioa_Campbell_Antwort404.html": "https://www.ioa.org.uk/system/files/publications/DM%20CAMPBELL%20INPUT%20IMPEDANCE%20MEASUREMENTS%20ON%20HISTORIC%20BRASS%20INSTRUMENTS.pdf (404)",
    "hal_Velut2017_Antwort_Botpruefung.html": "https://hal.science/hal-01245846/document (Anubis-Botpruefung, nicht umgangen)",
    "hal_Matteoli2023_Antwort_Botpruefung.html": "https://hal.science/hal-04281802/document (Anubis-Botpruefung, nicht umgangen)",
    "kyushu_7178835_Antwort202_leer.html": "https://catalog.lib.kyushu-u.ac.jp/opac_download_md/7178835/7178835.pdf (202, leer)",
    "kyushu_25703_Antwort202_leer.html": "https://catalog.lib.kyushu-u.ac.jp/opac_download_md/25703/1925_765.pdf (202, leer)",
    "github_oscilos_brass_baum.json": "https://api.github.com/repos/MorgansLab/OSCILOS_brass/git/trees/HEAD?recursive=1",
}
zeilen = ["# Abrufe des Laufs 2026-11-17_28_waldhorn_tuba_nacharbeit (vom Lauf geschrieben)", "",
          "Abrufzeit = Aenderungszeit der Datei (Ortszeit). Such-JSONs (zenodo_suche_*, github_suche_*, hal_suche_*) "
          "stammen aus `repo_suche.py` bzw. `hal_suche.py` (URLs dort).", "",
          "| Datei | URL | Abrufzeit | Bytes | SHA-256 |", "|---|---|---|---|---|"]
for name in sorted(os.listdir(ORDNER)):
    p = os.path.join(ORDNER, name)
    if not os.path.isfile(p) or name == "INDEX.md":
        continue
    d = open(p, "rb").read()
    zeit = datetime.datetime.fromtimestamp(os.path.getmtime(p)).strftime("%Y-%m-%d %H:%M")
    zeilen.append(f"| `{name}` | {URL.get(name, 'siehe Skript')} | {zeit} | {len(d)} | `{hashlib.sha256(d).hexdigest()}` |")
    if name.endswith(".pdf") and d[:4] == b"%PDF":
        t = subprocess.run(["pdftotext", "-l", "2", p, "-"], capture_output=True, text=True, encoding="utf-8",
                           errors="replace").stdout
        print("=====", name, "\n", " ".join(t.split())[:350])
with open(os.path.join(ORDNER, "INDEX.md"), "w", encoding="utf-8") as f:
    f.write("\n".join(zeilen) + "\n")
print("INDEX.md:", len(zeilen) - 6, "Dateien")
