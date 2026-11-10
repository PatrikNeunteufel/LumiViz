"""Gesamtlautstaerke der Loops mit Kern (S185; Entscheid Patrik LV1 a).

Jeder Loop unter `modelled_instruments/` (die Loops der Nuancen, das Ensemble und die Beispiele je Klasse, ohne
`full/nocturne_demo` — NP2 a) bekommt die hoechste Lautstaerke bis 1,0, bei der seine Spitze nicht ueber −1 dBFS
geht: der Begrenzer am Ausgang (Decke −0,1 dBFS, LL7 a) arbeitet dann nicht, der Loop klingt Bit fuer Bit, wie der
Klangerzeuger ihn rechnet — wie jeder uebrige mitgelieferte Loop. Der Waechter `Loop Spitze LV1 a` in
`test_LoopWerte.cpp` prueft es fuer alle mitgelieferten Loops.

`LAUT` haelt je Loop die gemessene Lautstaerke; die Skripte `make_modelled_loops.py`, `make_fluegel_loops.py`,
`make_stab_loops.py`, `make_membran_loops.py`, `make_klasse1_loops.py` lesen sie mit `laut()`. Ein Loop, der noch
nicht in der Tafel steht, bekommt die Vorgabe des Skripts.

Ablauf — nach einer Klangaenderung an einem Instrument mit Kern oder einem neuen Loop dort:

1. messen (aus dem Projekt-Root, Release-clang):
       LUMIVIZ_LOOP_SPITZEN_DATEI=<datei.tsv> LumiViz.UnitTests --test-case="Loop Spitze LV1 a*"
2. ``python asset/loop/loop_lautstaerke.py --messung <datei.tsv>``
   rechnet je Loop der Tafel ``neu = min(1, laut heute * 10^((ZIEL - spitze) / 20))``, abgerundet auf vier Stellen,
   schreibt `LAUT` hier hinein und die Lautstaerke in die Loop-Dateien (nur dieses Feld).
3. ``loop_werte.txt`` nur in den Zeilen der geaenderten Loops nachziehen.

`ZIEL` liegt 0,05 dB unter der Grenze des Waechters: zwei Compiler runden nicht gleich. Die Spitze waechst mit der
Lautstaerke linear (sie wirkt nach den Effekten der Clips, `Loop::laut`), eine Messung genuegt.

Schreibende Laeufe zuerst in einer Kopie: ``--wurzel <kopie von asset/loop>``.
"""

import argparse
import json
import math
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

ZIEL_DBFS = -1.05
LAUT_MAX = 1.0

# --- LAUT: geschrieben von --messung; Loop (ohne .lvloop, relativ zu asset/loop): Lautstaerke ---
LAUT = {
    "modelled_instruments/bars_demo": 1.0,
    "modelled_instruments/bass_drum": 0.7543,
    "modelled_instruments/bongo": 0.6082,
    "modelled_instruments/celesta": 1.0,
    "modelled_instruments/clavinet_modal": 0.4262,
    "modelled_instruments/conga": 0.5184,
    "modelled_instruments/electric_grand_piano": 1.0,
    "modelled_instruments/glockenspiel": 1.0,
    "modelled_instruments/grand_piano": 1.0,
    "modelled_instruments/grand_piano_demo": 1.0,
    "modelled_instruments/honky_tonk_piano": 0.9952,
    "modelled_instruments/kalimba": 1.0,
    "modelled_instruments/keys_demo": 0.7628,
    "modelled_instruments/low_tom": 0.617,
    "modelled_instruments/marimba": 1.0,
    "modelled_instruments/membrane_demo": 0.3475,
    "modelled_instruments/modelled_ensemble": 0.2725,
    "modelled_instruments/reed_electric_piano": 1.0,
    "modelled_instruments/snare_drum": 0.6729,
    "modelled_instruments/tabla": 0.7114,
    "modelled_instruments/timpani": 0.8465,
    "modelled_instruments/tine_electric_piano": 1.0,
    "modelled_instruments/tubular_bells": 1.0,
    "modelled_instruments/upright_piano": 1.0,
    "modelled_instruments/vibraphone": 1.0,
    "modelled_instruments/xylophone": 1.0,
}
# --- Ende LAUT ---


def laut(name, vorgabe):
    """Die Lautstaerke des Loops `name` (z. B. ``modelled_instruments/kalimba``): aus `LAUT`, sonst `vorgabe`."""
    return LAUT.get(name, vorgabe)


def lies_messung(tsv):
    werte = {}
    for zeile in Path(tsv).read_text(encoding="utf-8").splitlines():
        if zeile.startswith("#") or not zeile.strip():
            continue
        name, spitze, volume = zeile.split("\t")
        werte[name.removesuffix(".lvloop")] = (float(spitze), float(volume))
    return werte


def gehoert_dazu(name):
    """Die Loops mit Kern nach LV1 a: alles unter modelled_instruments/, ohne die ganzen Stuecke (full/)."""
    return name.startswith("modelled_instruments/") and not name.startswith("modelled_instruments/full/")


def schreibe_laut(tafel, datei=Path(__file__)):
    text = datei.read_text(encoding="utf-8")
    zeilen = "".join(f"    {json.dumps(n)}: {v!r},\n" for n, v in sorted(tafel.items()))
    neu, anzahl = re.subn(r"(# --- LAUT: [^\n]*\nLAUT = \{\n).*?(\}\n# --- Ende LAUT ---)",
                          lambda m: m.group(1) + zeilen + m.group(2), text, flags=re.S)
    if anzahl != 1:
        raise SystemExit("Marken von LAUT nicht gefunden")
    datei.write_text(neu, encoding="utf-8", newline="\n")


def setze_in_datei(pfad, wert):
    """Nur das Feld `volume` auf oberster Ebene; der Rest der Datei bleibt Zeichen fuer Zeichen."""
    text = pfad.read_text(encoding="utf-8")
    neu, anzahl = re.subn(r'^(    "volume": )[-0-9.eE+]+(,?)$', lambda m: f"{m.group(1)}{json.dumps(wert)}{m.group(2)}", text,
                          flags=re.M)
    if anzahl != 1:
        raise SystemExit(f"{pfad}: Lautstaerke des Loops {anzahl}-mal gefunden, erwartet einmal")
    if json.loads(neu)["volume"] != wert:
        raise SystemExit(f"{pfad}: Lautstaerke nicht gesetzt")
    pfad.write_text(neu, encoding="utf-8", newline="\n")


def messung(tsv, wurzel):
    gemessen = lies_messung(tsv)
    tafel = dict(LAUT)
    geaendert = 0
    for name, (spitze, volume) in sorted(gemessen.items()):
        if not gehoert_dazu(name):
            continue
        neu = min(LAUT_MAX, math.floor(volume * 10 ** ((ZIEL_DBFS - spitze) / 20) * 10000) / 10000)
        tafel[name] = neu
        if neu != volume:
            setze_in_datei(wurzel / (name + ".lvloop"), neu)
            geaendert += 1
            print(f"  {name}: {volume} -> {neu} ({20 * math.log10(neu / volume):+.2f} dB, Spitze {spitze:.2f} -> "
                  f"{spitze + 20 * math.log10(neu / volume):.2f} dBFS)")
    fehlt = sorted(n for n in tafel if n not in gemessen)
    if fehlt:
        raise SystemExit(f"in der Tafel, aber nicht gemessen: {', '.join(fehlt)}")
    if wurzel == HIER:
        schreibe_laut(tafel)
    else:
        kopie = wurzel / Path(__file__).name
        if kopie.exists():
            schreibe_laut(tafel, kopie)
    print(len(tafel), "Loops in der Tafel,", geaendert, "geaendert")


def main():
    teile = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    teile.add_argument("--messung", required=True, metavar="TSV", help="die Spitzen aus `Loop Spitze LV1 a`")
    teile.add_argument("--wurzel", default=str(HIER), help="Ordner asset/loop (oder eine Kopie)")
    argumente = teile.parse_args()
    messung(argumente.messung, Path(argumente.wurzel).resolve())


if __name__ == "__main__":
    sys.exit(main())
