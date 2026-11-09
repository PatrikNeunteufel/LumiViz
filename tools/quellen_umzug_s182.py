"""Umzug U1 (S182, Aufloesung_IC_Plan.md): fremde Dokumente aus LumiViz und IC nach `…\\cmake\\quellen\\<thema>\\`.

    python tools/quellen_umzug_s182.py kopiere     # kopieren, je Datei SHA-256 pruefen, Liste schreiben
    python tools/quellen_umzug_s182.py loesche     # die Quellen in LumiViz loeschen (nur, wenn das Ziel gleich ist)

`kopiere` schreibt `…\\cmake\\quellen\\UMZUG_S182.tsv` (sha256, Ziel relativ zu `quellen`, Quelle, Vermerk) — die
Grundlage fuer U2 (Links richten). Doppelte (gleiche Bytes) kommen einmal an, die weiteren Quellen stehen mit Vermerk
`doppelt`. Ein Ziel, das schon mit anderem Inhalt existiert, bricht ab. In IC wird nie etwas geloescht (nur lesen bis
zur Archivierung). Namen bleiben, wie sie sind.
"""

import hashlib
import shutil
import sys
from pathlib import Path

C = Path(__file__).resolve().parents[2]
R = C / "LumiViz"
IC = C / "InstrumentsCalculation"
ZIEL = C / "quellen"
LISTE = ZIEL / "UMZUG_S182.tsv"
MOD = R / "projects/apps/LumiViz/docs/modellierung"

KLASSE = {"Klasse 1 - Hammer-Saite": "hammer-saite", "Klasse 2 - Gezupfte Saite": "gezupfte-saite",
          "Klasse 3 - Bogen-Saite": "bogen-saite", "Klasse 7 - Lippe und Rohr": "lippe-rohr",
          "Klasse 8 - Rohrblatt und Rohr": "rohrblatt-rohr"}
IC_DLD = {  # nach dem Titel der ersten Seite (pdftotext, S182)
    "1611.01025v1.pdf": "lippe-rohr",            # Resch et al.: trumpet mouthpiece shank, 3D
    "2207.05395v3.pdf": "lippe-rohr",            # Mattéoli et al.: ghost notes in tubas, euphoniums, saxhorns
    "2506.16220v3.pdf": "rohrblatt-rohr",        # modal parameters, saxophone intonation
    "Ernoult et al. - 2021 - Full waveform inversion for bore reconstruction of.pdf": "rohr",
    "Gabrielli.pdf": "hammer-saite",             # Gabrielli et al.: Clavinet, waveguide (andere Fassung als Klasse 1)
    "Hendrie DA thesis 07.pdf": "lippe-rohr",    # bore reconstruction, brass (IC Klasse 7 quellen.md)
    "oboes_paper_revised_submitted.pdf": "rohrblatt-rohr",
    "real-time-emulation-of-the-clavinet.pdf": "hammer-saite",
}
GEMEINSAM = {"Weinzierl": "koerper", "Mores": "gezupfte-saite"}


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def plan():
    """(Quelle, Ziel-Ordner, loeschbar) — loeschbar nur, was in LumiViz liegt."""
    aus = []
    for q in sorted(MOD.glob("*/quellen")):
        aus += [(p, q.parent.name, True) for p in sorted(q.iterdir()) if p.is_file()]
    for p in sorted((R / "dld").glob("*")):
        aus.append((p, "noten/imslp" if p.name.startswith("IMSLP") else "hammer-saite", True))
    aus += [(p, "noten/tabs", True) for p in sorted((R / "asset/free_Tabs").glob("*")) if p.is_file()]
    for k, thema in KLASSE.items():
        aus += [(p, thema, False) for p in sorted((IC / "klassen" / k / "quellen").glob("*")) if p.is_file() and p.name != "SHA256SUMS"]
    for p in sorted((IC / "gemeinsam/quellen").glob("*")):
        aus.append((p, next(t for n, t in GEMEINSAM.items() if p.name.startswith(n)), False))
    for p in sorted((IC / "dld").glob("*")):
        aus.append((p, IC_DLD.get(p.name), False))  # None: ein Doppel, das schon kommt (geprueft unten)
    return aus


def kopiere():
    angekommen = {}  # sha -> Ziel
    zeilen = ["sha256\tziel\tquelle\tvermerk"]
    for quelle, thema, _ in plan():
        h = sha(quelle)
        if h in angekommen:
            zeilen.append(f"{h}\t{angekommen[h]}\t{quelle}\tdoppelt")
            continue
        if thema is None:
            raise SystemExit(f"keine Zuordnung und kein Doppel: {quelle}")
        ziel = ZIEL / thema / quelle.name
        if ziel.exists() and sha(ziel) != h:
            raise SystemExit(f"Ziel existiert mit anderem Inhalt: {ziel}")
        ziel.parent.mkdir(parents=True, exist_ok=True)
        if not ziel.exists():
            shutil.copy2(quelle, ziel)
        if sha(ziel) != h:
            raise SystemExit(f"Pruefsumme weicht ab: {ziel}")
        rel = ziel.relative_to(ZIEL).as_posix()
        angekommen[h] = rel
        zeilen.append(f"{h}\t{rel}\t{quelle}\t")
    LISTE.write_text("\n".join(zeilen) + "\n", encoding="utf-8", newline="\n")
    n = sum(1 for z in zeilen[1:] if not z.endswith("doppelt"))
    print(f"{len(zeilen) - 1} Quellen, {n} Dateien angekommen, {len(zeilen) - 1 - n} doppelt; Liste {LISTE}")


def loesche():
    zeilen = LISTE.read_text(encoding="utf-8").splitlines()[1:]
    weg = 0
    for z in zeilen:
        h, rel, quelle, _ = (z.split("\t") + [""])[:4]
        q = Path(quelle)
        if R not in q.parents or not q.exists():
            continue
        if sha(ZIEL / rel) != h or sha(q) != h:
            raise SystemExit(f"nicht gleich, nichts geloescht ab: {q}")
        q.unlink()
        weg += 1
    for leer in list(MOD.glob("*/quellen")) + [R / "dld", R / "asset/free_Tabs"]:
        if leer.is_dir() and not any(leer.iterdir()):
            leer.rmdir()
    print(f"{weg} Dateien in LumiViz geloescht")


if __name__ == "__main__":
    {"kopiere": kopiere, "loesche": loesche}[sys.argv[1]]()
