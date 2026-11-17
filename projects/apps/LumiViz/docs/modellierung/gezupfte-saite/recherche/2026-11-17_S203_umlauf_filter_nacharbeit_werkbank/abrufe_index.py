"""Schreibt abrufe/INDEX.md (Datei, URL, Abrufzeit, Groesse, SHA-256) fuer die roh gelesenen Dateien dieses Laufs."""
import hashlib
import pathlib
import datetime

D = pathlib.Path(__file__).parent / "abrufe"
URL = {
    "Rauhala_Valimaki_2006_DispersionTunableAllpass_DAFx06.pdf":
        "https://web.archive.org/web/20200722051908id_/http://www.dafx.de/paper-archive/2006/papers/p_071.pdf",
    "adf_m_wayback.txt": "https://web.archive.org/web/20220709004750id_/http://research.spa.aalto.fi/publications/papers/spl-adf/adf.m",
    "wayback_spl_adf_index.html": "https://web.archive.org/web/2022id_/http://research.spa.aalto.fi/publications/papers/spl-adf/",
    "wayback_cdx_dafx06.txt": "https://web.archive.org/cdx/search/cdx?url=dafx.de/paper-archive/2006/papers/*&fl=original&collapse=urlkey&filter=mimetype:application/pdf",
    "wayback_cdx_spl_adf.txt": "https://web.archive.org/cdx/search/cdx?url=acoustics.hut.fi/publications/papers/spl-*&fl=original,mimetype&collapse=urlkey",
    "wayback_cdx_spa_spl.txt": "https://web.archive.org/cdx/search/cdx?url=research.spa.aalto.fi/publications/papers/spl*&fl=original,mimetype&collapse=urlkey&limit=200",
    "s2_abel2010.json": "https://api.semanticscholar.org/graph/v1/paper/DOI:10.1109/LSP.2010.2040924?fields=title,openAccessPdf,externalIds,url",
    "s2_rauhala2006.json": "https://api.semanticscholar.org/graph/v1/paper/search?query=Tunable+dispersion+filter+design+for+piano+synthesis (HTTP 429)",
    "aalto_suche_allpass.html": "https://research.aalto.fi/en/publications/?search=allpass+dispersive+string+sound+synthesis (HTTP 403)",
    "stanford_abel_smith_2006.html": "https://ccrma.stanford.edu/papers/robust-design-of-very-high-order-allpass-dispersion-filters",
    "ccrma_wiki_dispersion_filter_design.html": "https://ccrma.stanford.edu/wiki/Dispersion_Filter_Design",
}
z = ["# Abrufe des Laufs 2026-11-17_21 (roh gelesen mit curl)", "",
     "| Datei | URL | Abrufzeit (Datei) | Bytes | SHA-256 |", "|---|---|---|---|---|"]
for f in sorted(D.iterdir()):
    if f.name == "INDEX.md" or not f.is_file():
        continue
    b = f.read_bytes()
    t = datetime.datetime.fromtimestamp(f.stat().st_mtime).strftime("%Y-%m-%d %H:%M")
    z.append(f"| `{f.name}` | {URL.get(f.name, '?')} | {t} | {len(b)} | `{hashlib.sha256(b).hexdigest()}` |")
(D / "INDEX.md").write_text("\n".join(z) + "\n", encoding="utf-8")
print("\n".join(z))
