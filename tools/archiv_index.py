"""Index der externen Ablagen (S182): was unter `cmake/samples` und `cmake/quellen` liegt — auf der Systemplatte und im
Archiv auf der Wechselplatte —, auch wenn das Archiv nicht eingesteckt ist.

    python tools/archiv_index.py [--archiv <Laufwerk>]     z. B.  --archiv E

Je Ablage zwei Abschnitte: **Systemplatte** (`…\\Visuals_Project\\cmake\\<ablage>`) und **Wechselplatte**
(`<Laufwerk>:\\samples`, die Quellen darunter in `<Laufwerk>:\\samples\\quellen` — S189, sonst wie frueher
`<Laufwerk>:\\quellen`; ohne Angabe wird D: bis Z: gesucht). Ein Ordner `quellen` direkt in der Ablage der Samples ist das
Archiv der Quellen und zaehlt nicht zu den Samples. Ist ein Ort nicht da, bleibt sein Abschnitt aus dem
vorigen Index stehen (mit dem Datum, an dem er geschrieben wurde). Schreibt
`projects/apps/LumiViz/docs/archiv/Index_samples.md` und `Index_quellen.md`.

Samples: je Quelle (ein Ordner mit `HERKUNFT.md`, sonst die Ordner der ersten Ebene) Zahl und Groesse der Dateien,
Endungen, die erste Ueberschrift und die Zeilen zu Lizenz und Herkunft aus `HERKUNFT.md`/`README.md`; die Hoerproben
(`Hoerproben\\<gruppe>\\<liste>`, AU3) je Liste mit Zustand — **abgeschlossen**, wenn eine `abgeschlossen.pass` darin
liegt (erst dann darf sie ins Archiv). Quellen: je Datei Name und Groesse, nach Ordner; eine `NOTIZ.md` im Ordner wird
mit ihrer ersten Zeile genannt. Liest nur, schreibt nur die beiden Index-Dateien.
"""

import argparse
import datetime
import re
import string
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
CMAKE = REPO.parent
ZIEL = REPO / "projects" / "apps" / "LumiViz" / "docs" / "archiv"
BEGLEIT = ("HERKUNFT.md", "README.md", "readme.txt", "Readme.txt")
MERKMAL = re.compile(r"lizenz|license|licence|herkunft|quelle|source|url|doi|zenodo|cc[- ]by|cc0|public domain", re.I)
OHNE = {"README.md"}  # die Beschreibung der Ablage selbst
QUELLEN_IN_SAMPLES = "quellen"  # S189: das Archiv der Quellen liegt unter dem der Samples
# wo eine Ablage auf der Wechselplatte liegt, in dieser Reihenfolge gesucht
ARCHIV = {"samples": ("samples",), "quellen": ("samples/" + QUELLEN_IN_SAMPLES, "quellen")}

KOPF = {
    "samples": ["# Index: Samples, Messdaten und Hörproben", "",
                "> Ablage `…\\Visuals_Project\\cmake\\samples`, aus Platzmangel archiviert auf der Wechselplatte",
                "> (`<Laufwerk>:\\samples`, der Buchstabe kann wechseln). **Vor Gebrauch Patrik bitten, sie verfügbar zu",
                "> machen.** Lizenz und Herkunft je Quelle stehen in deren `HERKUNFT.md`; hier nur die Zeilen dazu. Hörproben",
                "> bleiben auf der Systemplatte, bis ihr Test abgeschlossen ist (`abgeschlossen.pass`). Erzeugt von",
                "> `tools/archiv_index.py`."],
    "quellen": ["# Index: fremde Dokumente", "",
                "> Ablage `…\\Visuals_Project\\cmake\\quellen` — Paper, Bücher, Noten und andere Dokumente, die aus",
                "> urheberrechtlichen Gründen nicht ins Repo gehören; archiviert auf der Wechselplatte unter",
                "> `<Laufwerk>:\\samples\\quellen` (seit S189; früher `<Laufwerk>:\\quellen`, der Buchstabe kann wechseln).",
                "> **Vor Gebrauch Patrik bitten, sie verfügbar zu machen.** Zitiert wird in den Berichten unter",
                "> `docs/modellierung/` mit Seite. Erzeugt von `tools/archiv_index.py`."],
}
ORTE = (("system", "Systemplatte"), ("wechsel", "Wechselplatte"))


def groesse(n):
    for einheit in ("B", "KB", "MB", "GB", "TB"):
        if n < 1024 or einheit == "TB":
            return f"{n:.0f} {einheit}" if einheit == "B" else f"{n:.1f} {einheit}"
        n /= 1024


def dateien(ordner, ohne=None):
    """Alle Dateien unter `ordner`; `ohne` ein Unterordner, der nicht mitzaehlt."""
    return [p for p in ordner.rglob("*") if p.is_file() and (ohne is None or ohne not in p.parents)]


def zusammenfassung(ordner, ohne=None):
    fs = dateien(ordner, ohne)
    endungen = Counter((p.suffix.lower() or "(ohne)") for p in fs)
    return len(fs), sum(p.stat().st_size for p in fs), ", ".join(f"{e} {n}" for e, n in endungen.most_common(5))


def begleitzeilen(ordner):
    zeilen = []
    for name in BEGLEIT:
        p = ordner / name
        if not p.is_file():
            continue
        text = p.read_text(encoding="utf-8", errors="replace").splitlines()
        kopf = next((z.strip("# ").strip() for z in text if z.strip()), "")
        if kopf:
            zeilen.append(f"{name}: {kopf[:140]}")
        for z in text:
            z = z.strip().lstrip("-*> ").strip()
            if z and MERKMAL.search(z) and not z.startswith("#") and len(zeilen) < 6:
                zeilen.append(z[:160])
        break
    return zeilen


def eintraege_samples(wurzel):
    """Quellen: Ordner mit HERKUNFT.md; ohne solche ein Ordner der ersten Ebene als Ganzes (ohne die Hoerproben und das
    Archiv der Quellen)."""
    aus = []
    for oben in sorted(p for p in wurzel.iterdir()
                       if p.is_dir() and not p.name.startswith("$") and p.name not in ("Hoerproben", QUELLEN_IN_SAMPLES)):
        mit = sorted(oben.rglob("HERKUNFT.md"))
        if mit:
            aus.extend(h.parent for h in mit)
            aus.extend(sorted(p for p in oben.iterdir()
                              if p.is_dir() and not any(p == h.parent or p in h.parents for h in mit)))
        else:
            aus.append(oben)
    return aus


def hoerproben(wurzel):
    hp = wurzel / "Hoerproben"
    if not hp.is_dir():
        return []
    z = []
    for gruppe in sorted(p for p in hp.iterdir() if p.is_dir()):
        for liste in sorted(p for p in gruppe.iterdir() if p.is_dir()):
            wavs = [p for p in liste.rglob("*") if p.is_file() and p.suffix.lower() == ".wav"]
            if not wavs and gruppe.name != "_werkzeug":
                continue
            zustand = ("Werkzeug" if gruppe.name == "_werkzeug"
                       else "**abgeschlossen**" if (liste / "abgeschlossen.pass").is_file() else "offen")
            z.append(f"| `{gruppe.name}/{liste.name}` | {len(wavs)} | {groesse(zusammenfassung(liste)[1])} | {zustand} |")
    if not z:
        return []
    return ["### Hörproben", "", "| Liste | Klänge | Größe | Zustand |", "|---|---|---|---|"] + z + [""]


def abschnitt_samples(wurzel):
    z = []
    eintraege = eintraege_samples(wurzel)
    n_alle, b_alle, _ = zusammenfassung(wurzel, ohne=wurzel / QUELLEN_IN_SAMPLES)
    z += [f"**Gesamt:** {n_alle} Dateien, {groesse(b_alle)}"
          + (" (ohne das Archiv der Quellen in `quellen`)." if (wurzel / QUELLEN_IN_SAMPLES).is_dir() else "."), ""]
    if eintraege:
        z += ["| Quelle | Dateien | Größe | Endungen |", "|---|---|---|---|"]
        for e in eintraege:
            n, b, end = zusammenfassung(e)
            z.append(f"| `{e.relative_to(wurzel).as_posix()}` | {n} | {groesse(b)} | {end} |")
        z.append("")
    z += hoerproben(wurzel)
    for e in eintraege:
        z += [f"### {e.relative_to(wurzel).as_posix()}", ""]
        zeilen = begleitzeilen(e)
        z += [f"- {zl}" for zl in zeilen] or ["- *(keine HERKUNFT.md/README — Herkunft nicht vermerkt)*"]
        unter = sorted(p for p in e.iterdir() if p.is_dir())
        if unter:
            z.append("- Unterordner: " + ", ".join(f"`{u.name}` ({zusammenfassung(u)[0]})" for u in unter[:12])
                     + (f", … ({len(unter)} insgesamt)" if len(unter) > 12 else ""))
        z.append("")
    lose = sorted(p for p in wurzel.iterdir() if p.is_file() and p.name not in OHNE)
    if lose:
        z += ["### Dateien direkt in der Ablage", ""] + [f"- `{p.name}` ({groesse(p.stat().st_size)})" for p in lose] + [""]
    return z


def abschnitt_quellen(wurzel):
    fs = [p for p in dateien(wurzel) if p.relative_to(wurzel).as_posix() not in OHNE]
    if not fs:
        return ["*(leer)*", ""]
    z = [f"**Gesamt:** {len(fs)} Dateien, {groesse(sum(p.stat().st_size for p in fs))}.", ""]
    je = {}
    for p in fs:
        je.setdefault(p.parent, []).append(p)
    for ordner in sorted(je):
        z += [f"### {ordner.relative_to(wurzel).as_posix() or '.'}", ""]
        notiz = ordner / "NOTIZ.md"
        if notiz.is_file():
            erste = next((l.strip("# ").strip() for l in notiz.read_text(encoding="utf-8", errors="replace").splitlines() if l.strip()), "")
            z += [f"> {erste}", ""]
        z += [f"- `{p.name}` ({groesse(p.stat().st_size)})" for p in sorted(je[ordner]) if p.name != "NOTIZ.md"] + [""]
    return z


def marke(ort, teil):
    return f"<!-- {ort.upper()}:{teil} -->"


def alter_abschnitt(text, ort):
    a, e = marke(ort, "ANFANG"), marke(ort, "ENDE")
    if a in text and e in text:
        return text[text.index(a) + len(a):text.index(e)].strip("\n").splitlines()
    return None


def wurzeln(name, laufwerk):
    system = CMAKE / name
    wechsel = None
    for lw in ([laufwerk] if laufwerk else string.ascii_uppercase[3:]):
        for teil in ARCHIV[name]:
            p = Path(f"{lw}:/") / teil
            if p.is_dir() and any(p.iterdir()):
                wechsel = p
                break
        if wechsel is not None:
            break
    return {"system": system if system.is_dir() else None, "wechsel": wechsel}


def main():
    a = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    a.add_argument("--archiv", help="Laufwerksbuchstabe der Wechselplatte (ohne: D bis Z suchen)")
    args = a.parse_args()
    ZIEL.mkdir(parents=True, exist_ok=True)
    heute = datetime.date.today().isoformat()
    for name, bau in (("samples", abschnitt_samples), ("quellen", abschnitt_quellen)):
        datei = ZIEL / f"Index_{name}.md"
        vorher = datei.read_text(encoding="utf-8") if datei.is_file() else ""
        z = KOPF[name] + [""]
        for ort, titel in ORTE:
            wurzel = wurzeln(name, args.archiv)[ort]
            z.append(marke(ort, "ANFANG"))
            if wurzel is not None:
                z += [f"## {titel}: `{wurzel}`", "", f"*Stand {heute}.*", ""] + bau(wurzel)
                print(f"{name} / {titel}: {wurzel}")
            else:
                alt = alter_abschnitt(vorher, ort)
                z += alt if alt else [f"## {titel}", "", "*(nicht eingesteckt und noch nie indexiert)*", ""]
                print(f"{name} / {titel}: nicht da — {'voriger Abschnitt bleibt' if alt else 'leer'}")
            while z and z[-1] == "":
                z.pop()  # gleich, ob frisch geschrieben oder aus dem vorigen Index uebernommen
            z.append(marke(ort, "ENDE"))
            z.append("")
        datei.write_text("\n".join(z).rstrip("\n") + "\n", encoding="utf-8", newline="\n")
        print(f"  -> {datei.relative_to(REPO).as_posix()}")


if __name__ == "__main__":
    main()
