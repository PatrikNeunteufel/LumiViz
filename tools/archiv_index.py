"""Index der externen Ablagen (S182): was unter `cmake/samples` und `cmake/quellen` liegt, auch wenn das Archiv auf der
Wechselplatte nicht eingesteckt ist.

    python tools/archiv_index.py [--samples <ordner>] [--quellen <ordner>]

Ohne Angabe sucht das Werkzeug je Ablage zuerst den Ordner unter `…\\Visuals_Project\\cmake\\` und, ist er leer, ein
Archiv `<Laufwerk>:\\samples` bzw. `<Laufwerk>:\\quellen` (D: bis Z:). Es schreibt
`projects/apps/LumiViz/docs/archiv/Index_samples.md` und `Index_quellen.md` neu; eine Ablage, die nicht da ist, laesst
ihren Index stehen (er nennt den Stand, an dem er geschrieben wurde).

Samples: je Quelle (ein Ordner mit `HERKUNFT.md`, sonst die Ordner der ersten und zweiten Ebene) Zahl und Groesse der
Dateien, Endungen, die erste Ueberschrift und die Zeilen zu Lizenz und Herkunft aus `HERKUNFT.md`/`README.md`.
Quellen: je Datei Name und Groesse, nach Ordner; eine `NOTIZ.md` im Ordner wird mit ihrer ersten Zeile genannt.
Liest nur, schreibt nur die beiden Index-Dateien.
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


def groesse(n):
    for einheit in ("B", "KB", "MB", "GB", "TB"):
        if n < 1024 or einheit == "TB":
            return f"{n:.0f} {einheit}" if einheit == "B" else f"{n:.1f} {einheit}"
        n /= 1024


def leer(ordner):
    return not ordner.is_dir() or not any(ordner.iterdir())


def finde(name, angabe):
    if angabe:
        return Path(angabe)
    hier = CMAKE / name
    if not leer(hier):
        return hier
    for lw in string.ascii_uppercase[3:]:
        p = Path(f"{lw}:/") / name
        if not leer(p):
            return p
    return None


def dateien(ordner):
    return [p for p in ordner.rglob("*") if p.is_file()]


def zusammenfassung(ordner):
    fs = dateien(ordner)
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
    """Quellen: Ordner mit HERKUNFT.md; ohne solche ein Ordner der ersten Ebene als Ganzes."""
    aus = []
    for oben in sorted(p for p in wurzel.iterdir() if p.is_dir() and not p.name.startswith("$")):
        mit = sorted(p for p in oben.rglob("HERKUNFT.md"))
        if mit:
            for h in mit:
                aus.append(h.parent)
            rest = [p for p in oben.iterdir() if p.is_dir() and not any(p == h.parent or p in h.parents for h in mit)]
            aus.extend(sorted(rest))
        else:
            aus.append(oben)
    return aus


def index_samples(wurzel):
    z = [f"# Index: Samples und Messdaten", "",
         f"> Erzeugt von `tools/archiv_index.py` am {datetime.date.today().isoformat()} aus `{wurzel}`.",
         "> Ablage: `…\\Visuals_Project\\cmake\\samples`, archiviert auf der Wechselplatte (`<Laufwerk>:\\samples`, der",
         "> Buchstabe kann wechseln). **Vor Gebrauch Patrik bitten, sie verfügbar zu machen.** Neu schreiben, solange sie",
         "> eingesteckt ist. Lizenz und Herkunft je Quelle stehen in deren `HERKUNFT.md`; hier nur die Zeilen dazu.", ""]
    lose = sorted(p for p in wurzel.iterdir() if p.is_file())
    gesamt_n, gesamt_b, _ = zusammenfassung(wurzel)
    z.append(f"**Gesamt:** {gesamt_n} Dateien, {groesse(gesamt_b)}.")
    z.append("")
    z.append("| Quelle | Dateien | Größe | Endungen |")
    z.append("|---|---|---|---|")
    eintraege = eintraege_samples(wurzel)
    for e in eintraege:
        n, b, end = zusammenfassung(e)
        z.append(f"| `{e.relative_to(wurzel).as_posix()}` | {n} | {groesse(b)} | {end} |")
    z.append("")
    for e in eintraege:
        rel = e.relative_to(wurzel).as_posix()
        z.append(f"## {rel}")
        z.append("")
        zeilen = begleitzeilen(e)
        if not zeilen:
            z.append("- *(keine HERKUNFT.md/README — Herkunft nicht vermerkt)*")
        for zl in zeilen:
            z.append(f"- {zl}")
        unter = sorted(p for p in e.iterdir() if p.is_dir())
        if unter:
            z.append(f"- Unterordner: " + ", ".join(f"`{u.name}` ({zusammenfassung(u)[0]})" for u in unter[:12])
                     + (f", … ({len(unter)} insgesamt)" if len(unter) > 12 else ""))
        z.append("")
    if lose:
        z.append("## Dateien direkt in der Ablage")
        z.append("")
        for p in lose:
            z.append(f"- `{p.name}` ({groesse(p.stat().st_size)})")
        z.append("")
    return "\n".join(z) + "\n"


def index_quellen(wurzel):
    z = [f"# Index: fremde Dokumente", "",
         f"> Erzeugt von `tools/archiv_index.py` am {datetime.date.today().isoformat()} aus `{wurzel}`.",
         "> Ablage: `…\\Visuals_Project\\cmake\\quellen` — Paper, Bücher, Noten und andere Dokumente, die aus",
         "> urheberrechtlichen Gründen nicht ins Repo gehören; archiviert auf der Wechselplatte (`<Laufwerk>:\\quellen`).",
         "> **Vor Gebrauch Patrik bitten, sie verfügbar zu machen.** Zitiert wird in den Berichten unter",
         "> `docs/modellierung/` mit Seite; dort steht auch, wofür ein Dokument gebraucht wurde.", ""]
    fs = [p for p in dateien(wurzel) if p.relative_to(wurzel).as_posix() != "README.md"]
    if not fs:
        z.append("*(noch leer)*")
        return "\n".join(z) + "\n"
    z.append(f"**Gesamt:** {len(fs)} Dateien, {groesse(sum(p.stat().st_size for p in fs))}.")
    z.append("")
    je = {}
    for p in fs:
        je.setdefault(p.parent, []).append(p)
    for ordner in sorted(je):
        rel = ordner.relative_to(wurzel).as_posix() or "."
        z.append(f"## {rel}")
        z.append("")
        notiz = ordner / "NOTIZ.md"
        if notiz.is_file():
            erste = next((l.strip("# ").strip() for l in notiz.read_text(encoding="utf-8", errors="replace").splitlines() if l.strip()), "")
            z.append(f"> {erste}")
            z.append("")
        for p in sorted(je[ordner]):
            if p.name == "NOTIZ.md":
                continue
            z.append(f"- `{p.name}` ({groesse(p.stat().st_size)})")
        z.append("")
    return "\n".join(z) + "\n"


def main():
    a = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    a.add_argument("--samples")
    a.add_argument("--quellen")
    args = a.parse_args()
    ZIEL.mkdir(parents=True, exist_ok=True)
    for name, angabe, bau in (("samples", args.samples, index_samples), ("quellen", args.quellen, index_quellen)):
        wurzel = finde(name, angabe)
        datei = ZIEL / f"Index_{name}.md"
        if wurzel is None:
            print(f"{name}: nicht gefunden — {datei.name} bleibt, wie er ist")
            continue
        datei.write_text(bau(wurzel), encoding="utf-8", newline="\n")
        print(f"{name}: {wurzel} -> {datei.relative_to(REPO).as_posix()}")


if __name__ == "__main__":
    main()
