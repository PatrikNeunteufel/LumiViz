# -*- coding: utf-8 -*-
"""Skript-Lint fuer Asset-Chains: geschrieben, aber nie gelesen? (S62)

Anlass war domain.lvfx: das Preset baute eine Beat-Richtungsumkehr ueber
`dx`/`dy` — Variablen, die der domainWarp-Vertrag gar nicht liest. Der Flip
war jahrelang ein stummer No-op (MAE mit/ohne Beats: exakt 0,0000). Die
Feld-Sonden koennen das nicht finden: sie pruefen die MODULE, nicht die
handgeschriebenen Chains in asset/.

Die Frage dieses Lints, je Knoten einer Chain:

> Wird jede Variable, die ein Skript-Slot SCHREIBT, auch irgendwo GELESEN —
> vom Modul (skriptvars-Vertrag aus inventory_docs.json) oder von einem
> Skript-Slot desselben Knotens?

Wenn nein, ist die Zuweisung ein stummer Schreibzugriff: entweder tot oder —
wie bei domain.lvfx — ein Vertragsirrtum (Vokabular eines anderen Knotentyps).

Grenzen (bewusst):
- KEINE transitive Analyse: `a=...; b=a;` mit ungelesenem b meldet nur b.
- `reg00..reg99` / `q1..` sind knotenuebergreifende Globals — nie ein Befund.
- Gelesen heisst: das Token kommt ausserhalb einer Zuweisungs-LINKEN Seite
  vor. `x=x+1` liest x.
- EEL ist CASE-INSENSITIV: `X` und `x` sind dieselbe Variable — alles wird
  vor dem Vergleich kleingeschrieben (Fehlalarm-Quelle des Erstlaufs).

Aufruf:
  python lint_chain_scripts.py                      # effectchain+examples+composits
  python lint_chain_scripts.py pfad/zur/datei.lvfx  # einzelne Dateien/Ordner
Exit 1 bei Befunden.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HIER = Path(__file__).parent
REPO = HIER.parents[2]
INVENTORY = HIER / "inventory_docs.json"
DEFAULT_DIRS = [REPO / "asset/effectchain", REPO / "asset/examples",
                REPO / "asset/composits"]

# Knotenuebergreifende Globals (ScriptContext): nie als stumm melden.
GLOBAL_RE = re.compile(r"^(reg\d{2}|q\d+)$")

IDENT_RE = re.compile(r"\$?[A-Za-z_][A-Za-z0-9_]*")
# Zuweisung: Ident am Ausdrucksanfang, dann '=' (nicht '==', '<=', '>=', '!=').
ASSIGN_RE = re.compile(r"(?:^|[;(])\s*(\$?[A-Za-z_][A-Za-z0-9_]*)\s*=(?![=])")


def contract_vars() -> dict[str, tuple[set[str], set[str]]]:
    """typkey -> (Skriptfeld-Namen, vom Modul gelesene Variablen)."""
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    result: dict[str, tuple[set[str], set[str]]] = {}
    for typ in data["typen"]:
        slots: set[str] = set()
        gelesen: set[str] = set()
        for feld in typ["felder"]:
            if feld.get("helfer") == "addScript" or (
                    feld.get("panel") == "skript" and feld.get("art") == "text"):
                slots.add(feld["name"])
                gelesen.update(v.lower() for v in (feld.get("skriptvars") or []))
        result[typ["typkey"]] = (slots, gelesen)
    return result


def strip_comments(code: str) -> str:
    code = re.sub(r"/\*.*?\*/", " ", code, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", " ", code)


def written_and_read(code: str) -> tuple[set[str], set[str]]:
    """(geschriebene, gelesene) Variablen eines EEL-Texts.

    Funktionsaufrufe (`sin(`) zaehlen weder als Schreiben noch als
    Variablen-Lesen; ein Ident auf der rechten Seite liest.
    """
    code = strip_comments(code)
    written = {m.group(1).lower() for m in ASSIGN_RE.finditer(code)}
    read: set[str] = set()
    for m in IDENT_RE.finditer(code):
        name = m.group(0).lower()
        rest = code[m.end():]
        if re.match(r"\s*\(", rest):
            continue  # Funktionsname
        if re.match(r"\s*=(?![=])", rest):
            # linke Seite einer Zuweisung? Nur wenn am Ausdrucksanfang.
            davor = code[:m.start()].rstrip()
            if davor == "" or davor.endswith((";", "(")):
                continue
        read.add(name)
    return written, read


def walk_nodes(node: dict, path: str = "root"):
    yield path, node
    for i, child in enumerate(node.get("children") or []):
        name = child.get("name") or child.get("type") or str(i)
        yield from walk_nodes(child, f"{path}/{name}")


def lint_file(lvfx: Path, contracts) -> list[str]:
    try:
        doc = json.loads(lvfx.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as e:
        return [f"UNLESBAR: {e}"]
    root = doc.get("root")
    if not isinstance(root, dict):
        return []

    findings = []
    for path, node in walk_nodes(root):
        typ = node.get("type", "")
        slots, modul_liest = contracts.get(typ, (set(), set()))
        if typ not in contracts:
            # Unbekannter Typ: generisch alle *Code-Stringfelder linten.
            slots = {k for k, v in node.items()
                     if isinstance(v, str) and k.endswith("Code")}
        written_all: set[str] = set()
        read_all: set[str] = set()
        wo: dict[str, list[str]] = {}
        for slot in sorted(slots):
            code = node.get(slot)
            if not isinstance(code, str) or code.strip() == "":
                continue
            w, r = written_and_read(code)
            for name in w:
                wo.setdefault(name, []).append(slot)
            written_all |= w
            read_all |= r
        stumm = {name for name in written_all
                 if name not in read_all
                 and name not in modul_liest
                 and not GLOBAL_RE.match(name)}
        for name in sorted(stumm):
            findings.append(f"{path} [{typ}]: '{name}' geschrieben "
                            f"({', '.join(wo[name])}), nie gelesen")
    return findings


def main() -> int:
    contracts = contract_vars()
    args = [Path(a) for a in sys.argv[1:]]
    roots = args or DEFAULT_DIRS
    files: list[Path] = []
    for r in roots:
        if r.is_dir():
            files += sorted(r.rglob("*.lvfx"))
        elif r.suffix == ".lvfx":
            files.append(r)
    if not files:
        print("FEHLER: keine .lvfx gefunden")
        return 2

    gesamt = 0
    for f in files:
        findings = lint_file(f, contracts)
        if findings:
            gesamt += len(findings)
            rel = f.relative_to(REPO) if f.is_relative_to(REPO) else f
            print(f"\n{rel}")
            for zeile in findings:
                print(f"  BEFUND  {zeile}")
    print(f"\n{len(files)} Dateien geprueft, {gesamt} Befund(e)")
    return 1 if gesamt else 0


if __name__ == "__main__":
    sys.exit(main())
