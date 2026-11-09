"""Umzug U3 (S182, Aufloesung_IC_Plan.md): die Doku von InstrumentsCalculation (IC) nach `docs/modellierung/…`.

    python tools/ic_doku_umzug_s182.py --trocken     # nur zeigen: Ziele, Links, ungeloeste Links
    python tools/ic_doku_umzug_s182.py               # kopieren und die Links richten

Quelle sind die in IC versionierten Markdown-Dateien (Stand `4409088`) ohne `rechnung/`, `hoertest/`,
`gemeinsam/vorbild-lumiviz/`, `gemeinsam/cloud-global/` und ohne die Begleitdateien unter `klassen/*/aufnahmen|daten/`
(die bringt U4). Ziele nach AU1 a und AU1-1 a, je in einem Unterordner `ic/`. Der Wortlaut bleibt; geaendert werden nur
relative Links: auf ein Dokument, das mitkommt oder in einem spaeteren Schritt kommt (U4 Begleitdateien, U5 Code nach
`harvest/`, U6 Hoerlisten), zeigt der Link auf den neuen Ort; auf eine Datei ausserhalb des Repos (PDF unter
`cmake\\quellen`, Klaenge unter `cmake\\samples`) wird der Link zu Text mit dem Ort in Backticks; auf etwas, das
entfaellt, zu Text mit Vermerk. In IC wird nichts geaendert. Ein Ziel, das es schon gibt, bricht ab.
"""

import argparse
import posixpath
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import quote, unquote

C = Path(__file__).resolve().parents[2]
R = C / "LumiViz"
IC = C / "InstrumentsCalculation"
MOD = "projects/apps/LumiViz/docs/modellierung"
ANB = f"{MOD}/anbindung-ic/ic"

KLASSE = {
    "Klasse 1 - Hammer-Saite": "hammer-saite", "Klasse 2 - Gezupfte Saite": "gezupfte-saite",
    "Klasse 3 - Bogen-Saite": "bogen-saite", "Klasse 4 - Platte und Stab": "platte-stab",
    "Klasse 5 - Membran": "membran", "Klasse 6 - Becken und Gong": "becken-gong",
    "Klasse 7 - Lippe und Rohr": "lippe-rohr", "Klasse 8 - Rohrblatt und Rohr": "rohrblatt-rohr",
    "Klasse 9 - Luftstrahl und Rohr": "luftstrahl-rohr", "Klasse 10 - Durchschlagende Zunge": "zunge",
    "Klasse 11 - Tonrad": "tonrad",
}
# AU1-1 a: die gemeinsamen Dokumente nach Thema
GEMEINSAM = {
    "koerper.md": "koerper", "recherche/2026-10-26_koerper_abstrahlung.md": "koerper",
    "rohr.md": "rohr", "recherche/2026-10-26_rohr_verfahren.md": "rohr",
    "recherche/2026-10-26_moden_und_waveguides_rohr.md": "rohr", "recherche/2026-10-26_aufnahmen_blaeser.md": "rohr",
    "messkette.md": "messung", "hoerschwellen.md": "messung", "recherche/2026-10-26_messkette_verfahren.md": "messung",
    "recherche/2026-10-26_aufnahmen_schlagwerk.md": "membran", "recherche/2026-10-26_spielweisen_schlagwerk.md": "membran",
    "recherche/2026-10-26_moden_und_waveguides_saite.md": "hammer-saite",
    "lehren-pruefung.md": "anbindung-ic", "recherche/2026-10-27_pruefung_lehren_k1-4.md": "anbindung-ic",
    "recherche/2026-10-27_pruefung_lehren_k5-8.md": "anbindung-ic", "recherche/2026-10-27_pruefung_lehren_k9-11_k0.md": "anbindung-ic",
    "README.md": "gemeinsam", "aufnahmen.md": "gemeinsam", "quellen.md": "gemeinsam", "offene-fragen.md": "gemeinsam",
}
QUELLEN_LISTE = C / "quellen" / "UMZUG_S182.tsv"
LINK = re.compile(r"(!?)\[([^\]]*)\]\(\s*(<[^>]+>|[^)\s]+)(\s+\"[^\"]*\")?\s*\)")


def ic_dateien():
    return subprocess.run(["git", "-C", str(IC), "ls-files"], capture_output=True, text=True, encoding="utf-8").stdout.splitlines()


def pdf_ziele():
    """IC-Pfad eines PDF -> Ort unter cmake\\quellen (aus U1)."""
    aus = {}
    for z in QUELLEN_LISTE.read_text(encoding="utf-8").splitlines()[1:]:
        h, ziel, quelle, _ = (z.split("\t") + [""])[:4]
        q = Path(quelle)
        if IC in q.parents:
            aus[q.relative_to(IC).as_posix()] = "…\\cmake\\quellen\\" + ziel.replace("/", "\\")
    return aus


PDF = None


def ziel(p):
    """IC-Pfad (Datei oder Ordner, POSIX, ohne fuehrendes /) -> ("repo", Pfad im LumiViz-Repo) | ("extern", Ort) |
    ("weg", Grund) | None (keine Regel)."""
    global PDF
    PDF = PDF if PDF is not None else pdf_ziele()
    p = p.rstrip("/")
    if p in PDF:
        return ("extern", PDF[p])
    teile = p.split("/") if p else []
    if not teile:
        return ("repo", f"{ANB}")
    kopf = teile[0]
    if kopf == "klassen":
        if len(teile) == 1:
            return ("repo", MOD)
        slug = KLASSE.get(teile[1])
        if slug is None:
            return None
        rest = teile[2:]
        if rest[:1] in (["aufnahmen"], ["daten"]):  # U4: Begleitdateien
            return ("repo", "/".join([MOD, slug] + rest))
        if rest[:1] == ["quellen"]:
            return ("extern", "…\\cmake\\quellen\\" + slug + ("\\" + "\\".join(rest[1:]) if rest[1:] else ""))
        return ("repo", "/".join([MOD, slug, "ic"] + rest))
    if kopf == "gemeinsam":
        rest = "/".join(teile[1:])
        if rest in GEMEINSAM:
            return ("repo", f"{MOD}/{GEMEINSAM[rest]}/ic/{rest}")
        if rest in ("", "recherche"):
            return ("repo", f"{MOD}/gemeinsam/ic" + ("/recherche" if rest else ""))
        if rest.startswith("quellen"):
            return ("extern", "…\\cmake\\quellen")
        if rest.startswith(("vorbild-lumiviz", "cloud-global")):
            return ("weg", "Kopie fuer die Cloud-Sitzung, entfaellt mit der Aufloesung (S182)")
        return None
    if kopf == "rechnung":  # U5
        return ("repo", "harvest/instruments-calculation/" + p)
    if kopf == "hoertest":  # U6, AU3/AU3-1
        if len(teile) >= 3 and teile[1] == "listen":
            if len(teile) >= 4 and teile[3] == "klaenge":
                return ("extern", "…\\cmake\\samples\\Hoerproben\\IC\\" + "\\".join(teile[2:]))
            return ("repo", "hoertest/listen/ic/" + "/".join(teile[2:]))
        return ("extern", "…\\cmake\\samples\\Hoerproben\\IC")
    if kopf == "pruefungen":
        return ("repo", f"{ANB}/" + p)
    if kopf == ".claude" and len(teile) >= 2 and teile[1] == "journal":
        return ("repo", f"{ANB}/journal" + ("/" + "/".join(teile[2:]) if teile[2:] else ""))
    if kopf == "dld":
        return ("extern", "…\\cmake\\quellen")
    if len(teile) == 1 and p.endswith(".md"):
        return ("repo", f"{ANB}/" + ("IC_CLAUDE.md" if p == "CLAUDE.md" else p))
    if p in (".gitignore", ".gitattributes") or kopf in (".claude", ".vs"):
        return ("weg", "Einstellung des IC-Repos, entfaellt")
    return None


def mitkommen(dateien):
    """Die Dokumente von U3: Markdown in IC, ohne Code, Hoertest, Cloud-Kopien und Begleitdateien (U4)."""
    for d in dateien:
        if not d.endswith(".md"):
            continue
        if d.startswith(("rechnung/", "hoertest/", "gemeinsam/vorbild-lumiviz/", "gemeinsam/cloud-global/")):
            continue
        if re.match(r"klassen/[^/]+/(aufnahmen|daten)/", d):
            continue
        yield d


def richte(text, quelle, ziel_rel, fehlt):
    """Die relativen Links in `text` (Datei `quelle` in IC, kuenftig `ziel_rel` im Repo) richten."""
    def ersetze(m):
        bild, wort, roh, titel = m.group(1), m.group(2), m.group(3), m.group(4) or ""
        pfad = roh[1:-1] if roh.startswith("<") else roh
        if re.match(r"^[a-z]+:", pfad) or pfad.startswith("#"):
            return m.group(0)
        pfad, _, anker = pfad.partition("#")
        voll = posixpath.normpath(posixpath.join(posixpath.dirname(quelle), unquote(pfad)))
        if voll.startswith("../"):  # ausserhalb von IC: nach LumiViz oder sonst unter cmake
            ausser = posixpath.normpath(posixpath.join("InstrumentsCalculation", voll))
            if ausser.startswith("LumiViz/"):
                neu = posixpath.relpath(ausser[len("LumiViz/"):], posixpath.dirname(ziel_rel))
                return f"{bild}[{wort}]({quote(neu)}{'#' + anker if anker else ''}{titel})"
            return f"{wort} (`…\\cmake\\{ausser.replace('/', chr(92))}`)"
        z = ziel(voll)
        if z is None or (z[0] == "repo" and not (IC / voll).exists() and not voll.startswith(("rechnung", "hoertest"))):
            fehlt.append((quelle, pfad))
            return m.group(0)
        art, ort = z
        if art == "repo":
            neu = posixpath.relpath(ort, posixpath.dirname(ziel_rel))
            return f"{bild}[{wort}]({quote(neu)}{'#' + anker if anker else ''}{titel})"
        if art == "extern":
            return f"{wort} (`{ort}`)"
        return f"{wort} (*{ort}*)"
    return LINK.sub(ersetze, text)


def main():
    a = argparse.ArgumentParser()
    a.add_argument("--trocken", action="store_true")
    trocken = a.parse_args().trocken
    dateien = ic_dateien()
    plan, fehlt, gerichtet = [], [], 0
    for d in mitkommen(dateien):
        z = ziel(d)
        if z is None or z[0] != "repo":
            raise SystemExit(f"keine Regel fuer {d}: {z}")
        plan.append((d, z[1]))
    ziele = [z for _, z in plan]
    if len(set(ziele)) != len(ziele):
        raise SystemExit("zwei Quellen, ein Ziel")
    for d, z in plan:
        if (R / z).exists():
            raise SystemExit(f"Ziel gibt es schon: {z}")
    for d, z in plan:
        roh = (IC / d).read_bytes()
        bom = roh.startswith(b"\xef\xbb\xbf")
        text = roh.decode("utf-8-sig")
        neu = richte(text, d, z, fehlt)
        gerichtet += sum(1 for x, y in zip(LINK.findall(text), LINK.findall(neu)) if x != y)
        if not trocken:
            ziel_p = R / z
            ziel_p.parent.mkdir(parents=True, exist_ok=True)
            ziel_p.write_bytes((b"\xef\xbb\xbf" if bom else b"") + neu.encode("utf-8"))
    je = {}
    for _, z in plan:
        je.setdefault(posixpath.dirname(z).replace(MOD + "/", ""), 0)
        je[posixpath.dirname(z).replace(MOD + "/", "")] += 1
    for o, n in sorted(je.items()):
        print(f"{n:4}  {o}")
    print(f"{len(plan)} Dokumente, {gerichtet} Links geaendert, {len(fehlt)} Links ohne Ziel")
    for q, p in fehlt:
        print(f"  OHNE ZIEL: {q} -> {p}")
    if trocken:
        print("(Trockenlauf — nichts geschrieben)")


if __name__ == "__main__":
    main()
