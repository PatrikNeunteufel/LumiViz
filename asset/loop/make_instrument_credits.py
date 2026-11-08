"""Schreibt die Nennungen der Quellen in `instruments/CREDITS.md` (Instrumente_IC_Konzept.md IK2-12 a, WZ1 a, WZ2 a;
Stufe 34f-2, S177).

Liest das Register `instrument_quellen.json` und jede mitgelieferte `instruments/*.lvinst`: welche Schluessel die
Kurven einer Kern-Schicht in `sources` nennen. Ab der Marke schreibt es die Nennung jeder benutzten Quelle mit
Pflicht zur Nennung, dazu die benutzten ohne Pflicht; was vor der Marke steht, bleibt (von Hand). Bricht ab, wenn ein
Schluessel nicht im Register steht oder aus einer Quelle stammt, deren Werte nicht ausgeliefert werden duerfen — wie
der Waechter `*Instrument Quellen*` in den Unit-Tests.

Aufruf (vom Ordner `asset/loop` aus oder mit Pfad): python make_instrument_credits.py [--pruefe]
`--pruefe` schreibt nichts und meldet nur, ob die Datei so stimmt (Rueckgabe 1, wenn nicht).

Nicht sichtbar fuer dieses Skript sind Quellen der Vorgaben, die keine Kurve sind (`ParameterAngabe::quellen` in der
Tafel eines Kerns); der Waechter in C++ sieht sie und meldet, wenn eine davon Pflicht ist und hier fehlte.
"""
import json
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
REGISTER = HIER / "instrument_quellen.json"
ZIEL = HIER / "instruments" / "CREDITS.md"
MARKE = "<!-- Instrument sources: from here on written by make_instrument_credits.py -->"
KOPF = """# Instruments — sources and credits

The built-in instruments under `instruments/` are LumiViz's own work. Some of their values are taken from published
measurements and models, or were set by measuring recordings through LumiViz's own model. The recordings themselves are
not shipped — only the values. Each value names its sources in the file (`sources`); the register of all sources is
`asset/loop/instrument_quellen.json` in the source tree.

"""


def benutzte(register):
    """Je Schluessel die Instrumente (und Kurven), die ihn nennen; dazu die Fehler."""
    bekannt = {q["key"]: q for q in register["sources"]}
    nutzung, fehler = {}, []
    for datei in sorted((HIER / "instruments").glob("*.lvinst")):
        daten = json.loads(datei.read_text(encoding="utf-8"))
        for schicht in daten.get("layers", []):
            if "core" not in schicht:
                continue
            for name, wert in sorted(schicht.get("coreParams", {}).items()):
                if not isinstance(wert, dict) or "points" not in wert:
                    continue
                quellen = wert.get("sources", [])
                if not quellen:
                    fehler.append(f"{datei.stem}: {name} nennt keine Quelle")
                for key in quellen:
                    if key not in bekannt:
                        fehler.append(f"{datei.stem}: {name} nennt {key!r}, das nicht im Register steht")
                    elif not bekannt[key]["shippable"]:
                        fehler.append(f"{datei.stem}: {name} stammt aus {key!r} — daraus darf nichts ausgeliefert werden")
                    nutzung.setdefault(key, {}).setdefault(datei.stem, []).append(name)
    return bekannt, nutzung, fehler


def text(bekannt, nutzung):
    pflicht = [k for k in sorted(nutzung) if bekannt.get(k, {}).get("creditRequired")]
    frei = [k for k in sorted(nutzung) if k in bekannt and not bekannt[k].get("creditRequired")]
    zeilen = [MARKE, "", "## Credits", "", "| Source | License | Used in |", "|---|---|---|"]
    for k in pflicht:
        q = bekannt[k]
        wo = "; ".join(f"`{inst}` ({', '.join(sorted(kurven))})" for inst, kurven in sorted(nutzung[k].items()))
        zeilen.append(f"| {q['credit']} | {q['license']} | {wo} |")
    if frei:
        zeilen += ["", "Also used, no credit required: " + ", ".join(f"{bekannt[k]['credit']} ({bekannt[k]['license']})" for k in frei) + "."]
    return "\n".join(zeilen) + "\n"


def main():
    register = json.loads(REGISTER.read_text(encoding="utf-8"))
    bekannt, nutzung, fehler = benutzte(register)
    if fehler:
        raise SystemExit("Quellen stimmen nicht:\n  " + "\n  ".join(fehler))
    alt = ZIEL.read_text(encoding="utf-8") if ZIEL.exists() else KOPF
    vorne = alt.split(MARKE, 1)[0] if MARKE in alt else alt
    neu = vorne + text(bekannt, nutzung)
    if "--pruefe" in sys.argv:
        if neu != alt:
            print(f"{ZIEL.name} ist veraltet — make_instrument_credits.py neu laufen lassen")
            return 1
        print(f"{ZIEL.name} stimmt")
        return 0
    ZIEL.write_text(neu, encoding="utf-8", newline="\n")
    print(f"{ZIEL.name}: {len(nutzung)} Quellen benutzt")
    return 0


if __name__ == "__main__":
    sys.exit(main())
