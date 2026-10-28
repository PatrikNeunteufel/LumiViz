"""Pegel der mitgelieferten Instrumente (S139, Stufe 27c; docs/audio/Loop_Konzept.md §4n,
Entscheide Patrik LL2 a, LL6 a, LL7 a, LL8 a).

Jedes mitgelieferte Instrument ausser den Drums (Art `drums` — sie kommen mit den Kits,
Stufe 27d) soll mit der Standardprobe (`loop/Pegel.hpp`) gleich laut messen: `BEZUG_LUFS`,
Short-term-Maximum bei Loop 1. Derselbe Wert steht als `kInstrumentBezugLufs` im Programm;
`test_LoopPegel.cpp` vergleicht beide.

`PEGEL` haelt je Instrument `(alt, neu)`: `alt` ist das `level`, mit dem die Skripte es bauen
und mit dem die mitgelieferten Loops gemischt sind; `neu` bringt es auf den Bezug.
`instrument_katalog.ergaenze` setzt `neu` — so bleibt es, auch wenn ein Skript das Instrument
neu schreibt. Die Datei behaelt ihre Fassung: das Programm liest `level` bis +24 dB in jeder
Fassung, und eine Fassung 13 behauptete Felder, die eine von einem Skript gebaute Datei nicht
traegt (das Programm selbst schreibt immer 13).

Ablauf — wer ein Instrument im Klang aendert, misst neu:

1. messen (aus dem Projekt-Root, Release-clang):
       LUMIVIZ_PEGEL_DATEI=<datei.tsv> LumiViz.UnitTests --test-case="Loop Pegel: Messung*"
2. ``python asset/loop/instrument_pegel.py --messung <datei.tsv>``
   rechnet ``neu = level heute * 10^((BEZUG - gemessen) / 20)``, schreibt `PEGEL` hier hinein
   und alle Instrumente neu (ueber `instrument_katalog.main`).
3. ``python asset/loop/instrument_pegel.py --loops``
   zieht jeden mitgelieferten Loop nach, der es noch nicht ist (Fassung unter 11): je
   Instrument-Muster ``volume * alt / neu``; was ueber 1 kaeme, geht in den Fader des Kanals
   (`gain` in dB, LL8 a). Der Loop klingt danach so laut wie vorher. Ein Loop-Skript, das neu
   laeuft, schreibt eine Fassung unter 11 — danach (3) wieder laufen lassen.

**Ein neues Instrument** (S157, `grand_piano`): `--messung` traegt es als `(level vor der Messung, neu)` ein —
richtig fuer Loops, die schon mit dem alten Level gemischt sind. Sind seine Loops fuer den Bezug geschrieben (die
Regel fuer jedes neue Instrument), setzt man danach von Hand `alt = neu`, sonst zieht (3) sie um den ganzen
Unterschied leiser nach (S157: 12,4 dB).

Schreibende Laeufe zuerst in einer Kopie: ``--wurzel <kopie von asset/loop>``.
"""

import argparse
import json
import math
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

BEZUG_LUFS = -13.4
LEVEL_MAX = 10 ** (24 / 20)  # +24 dB, `kInstrumentLautMax`

# --- PEGEL: geschrieben von --messung; Name: (alt, neu) ---
PEGEL = {
    "acid_bass": (1.0, 2.423),
    "alien_choir": (0.6, 0.4009),
    "alto_flute": (0.85, 0.4644),
    "alto_sax": (0.85, 0.5215),
    "angel_harp": (0.75, 0.5827),
    "bass": (1.0, 1.4082),
    "bass_808": (0.8, 0.5325),
    "bell": (0.7, 0.5021),
    "brass": (0.7, 0.6164),
    "broken_radio": (0.45, 0.3341),
    "cascade": (0.6, 0.5748),
    "chord_stab": (1.0, 2.5293),
    "clave": (0.49, 1.012),
    "clavinet": (0.705, 1.1452),
    "clavinet_old": (0.6, 0.9427),
    "clean_guitar": (0.398, 1.387),
    "clean_guitar_old": (0.7, 2.2392),
    "conga": (0.85, 1.5773),
    "cowbell": (0.64, 1.0833),
    "drop_d_guitar": (0.795, 0.4987),
    "dulcimer": (0.9, 1.0063),
    "female_choir": (0.8, 0.5392),
    "fifth_pad": (0.55, 0.5772),
    "finger_bass": (1.0, 1.1852),
    "flute": (0.8, 0.5298),
    "ghost": (0.7, 0.4897),
    "grand_piano": (4.2418, 4.2418),  # S160 (HU4-1 a, Boden mit c = 4,5): -13,833 LUFS bei 4,0355 -> 4,2418; die Loops sind fuer den Bezug geschrieben, darum alt = neu (S159: 4,1788 -> 4,0355)
    "growl_bass": (0.55, 0.3081),
    "guitar": (1.0, 2.0941),
    "guitar_old": (1.0, 2.2333),
    "harp": (0.85, 0.6098),
    "hoover": (0.45, 0.346),
    "house_organ": (1.0, 0.802),
    "house_piano": (1.0, 1.2222),
    "hum_pad": (0.85, 0.5331),
    "jazz_guitar": (0.51, 1.3465),
    "kalimba": (0.8, 0.9013),
    "koto": (0.8, 0.5774),
    "lead": (1.0, 0.7343),
    "marimba": (0.85, 1.0858),
    "muted_guitar": (1.0, 4.5798),
    "muted_guitar_old": (0.8, 2.7136),
    "nylon_guitar": (0.638, 2.1107),
    "nylon_guitar_old": (0.75, 1.8968),
    "organ": (1.0, 0.539),
    "overdrive_guitar": (0.568, 0.3769),
    "overdrive_guitar_old": (0.5, 0.5334),
    "piano": (1.0, 0.6294),
    "piezo_guitar": (0.505, 1.2388),
    "pizzicato_violin": (1.0, 5.559),
    "pluck": (1.0, 2.2722),
    "reese_bass": (0.6, 0.5013),
    "reverse_swell": (0.7, 0.6194),
    "rolling_bass": (0.7, 0.9255),
    "round_bass": (0.85, 0.7313),
    "sax": (0.65, 0.2874),
    "shakuhachi": (0.85, 0.8518),
    "shape_atom_bell": (0.55, 0.4807),
    "shape_bolt_pluck": (0.9, 3.8578),
    "shape_cat": (0.75, 0.7265),
    "shape_flower_flute": (0.7, 0.6216),
    "shape_gear_bass": (0.6, 0.3551),
    "shape_heart_pad": (0.55, 0.6922),
    "shape_infinity_drone": (0.5, 0.7073),
    "shape_moon_bass": (0.65, 1.0065),
    "shape_rings_choir": (0.5, 0.7216),
    "shape_smiley_keys": (0.5, 0.5651),
    "shape_star_lead": (0.5, 0.4385),
    "siren": (0.5, 0.3268),
    "slap_bass": (0.938, 0.9635),
    "slap_bass_old": (0.75, 0.8127),
    "steel_drum": (0.7, 0.8557),
    "strings": (1.0, 0.9377),
    "sub_bass": (1.0, 0.4745),
    "super_saw": (0.5, 0.3773),
    "thumb_piano": (0.8, 0.3443),
    "tine_piano": (0.75, 0.4932),
    "trance_lead": (0.5, 0.4769),
    "trance_pluck": (0.75, 1.8557),
    "ukulele": (0.878, 2.8112),
    "upright_bass": (0.954, 0.9867),
    "upright_bass_old": (0.85, 0.9781),
    "vibraphone": (0.8, 0.6336),
    "violin": (1.0, 0.7915),
    "vocal_chop": (0.9, 1.1367),
    "vocal_lead": (0.85, 0.5367),
    "warm_pad": (1.0, 1.3063),
    "wind_voices": (0.8, 0.526),
}
# --- Ende PEGEL ---


def level(name, gebaut):
    """Das `level` eines mitgelieferten Instruments: aus `PEGEL`, sonst wie gebaut."""
    eintrag = PEGEL.get(name)
    return eintrag[1] if eintrag else gebaut


def fassung(level_wert, version):
    """Die Fassung bleibt, wie gebaut (s. oben) — auch ueber 1."""
    return version


def lies_messung(tsv):
    werte = {}
    for zeile in Path(tsv).read_text(encoding="utf-8").splitlines():
        teile = zeile.split("\t")
        if len(teile) < 5 or teile[0] != "instrument" or teile[4] == "drums":
            continue
        werte[teile[1]] = (float(teile[2]), float(teile[3]))  # gemessen, Volume dabei
    return werte


def schreibe_pegel(pegel, datei=Path(__file__)):
    text = datei.read_text(encoding="utf-8")
    zeilen = "".join(f"    {json.dumps(n)}: ({a!r}, {b!r}),\n" for n, (a, b) in sorted(pegel.items()))
    neu, anzahl = re.subn(r"(# --- PEGEL: [^\n]*\nPEGEL = \{\n).*?(\}\n# --- Ende PEGEL ---)",
                          lambda m: m.group(1) + zeilen + m.group(2), text, flags=re.S)
    if anzahl != 1:
        raise SystemExit("Marken von PEGEL nicht gefunden")
    datei.write_text(neu, encoding="utf-8", newline="\n")


def messung(tsv):
    gemessen = lies_messung(tsv)
    pegel = dict(PEGEL)
    for name, (lufs, volume) in sorted(gemessen.items()):
        alt = pegel[name][0] if name in pegel else volume
        neu = volume * 10 ** ((BEZUG_LUFS - lufs) / 20)
        if neu > LEVEL_MAX:
            raise SystemExit(f"{name}: braeuchte {neu:.2f}, mehr als +24 dB")
        pegel[name] = (alt, round(neu, 4))
    schreibe_pegel(pegel)
    PEGEL.clear()
    PEGEL.update(pegel)
    print(len(gemessen), "Instrumente gemessen;", sum(1 for _, n in pegel.values() if n > 1.0), "ueber 1")
    # alle Instrumente neu schreiben: `ergaenze` liest PEGEL frisch aus dieser Datei
    sys.path.insert(0, str(HIER))
    from instrument_katalog import main as katalog
    katalog()


def mitgelieferte_loops(wurzel):
    return sorted({p for muster in ("genres/*.lvloop", "vergleich/*.lvloop", "*_demo.lvloop") for p in wurzel.glob(muster)})


def ziehe_nach(daten):
    """Einen Loop nachziehen (s. oben, 3); gibt zurueck, wie viele Kanaele einen Fader bekamen."""
    faktor = {name: alt / neu for name, (alt, neu) in PEGEL.items() if neu > 0}
    noetig = {}
    for m in daten.get("patterns", []):
        if m.get("kind") == "instrument" and m.get("instrument") in faktor:
            noetig[m["name"]] = m.get("volume", 1.0) * faktor[m["instrument"]]
        for spur in m.get("tracks", []):
            if spur.get("instrument") in faktor:
                raise SystemExit(f"Drum-Spur mit Instrument {spur['instrument']}: nicht vorgesehen")
    fader_von = {}
    mit_fader = 0
    for k in daten.get("channels", []):
        namen = {c["pattern"] for c in k.get("clips", [])}
        groesstes = max([noetig.get(n, 0.0) for n in namen] + [1.0])
        # auf zwei Stellen aufgerundet, damit keine Lautstaerke ueber 1 kommt
        gain = math.ceil(20 * math.log10(groesstes) * 100) / 100 if groesstes > 1.0 else 0.0
        k["gain"] = gain  # immer geschrieben (Fassung 11)
        if gain > 0.0:
            mit_fader += 1
        for n in namen:
            if fader_von.setdefault(n, gain) != gain:
                raise SystemExit(f"Muster {n} in Kanaelen mit verschiedenem Fader")
    for m in daten.get("patterns", []):
        if m["name"] in noetig:
            fader = 10 ** (fader_von.get(m["name"], 0.0) / 20)
            m["volume"] = min(1.0, round(noetig[m["name"]] / fader, 4))
    daten["version"] = 11
    return mit_fader


def loops(wurzel):
    sys.path.insert(0, str(HIER))
    from make_examples import als_text
    gezogen = uebrig = 0
    for pfad in mitgelieferte_loops(wurzel):
        daten = json.loads(pfad.read_text(encoding="utf-8"))
        if int(daten.get("version", 1)) >= 11:
            uebrig += 1
            continue
        mit_fader = ziehe_nach(daten)
        pfad.write_text(als_text(daten), encoding="utf-8", newline="\n")
        gezogen += 1
        if mit_fader:
            print(f"  {pfad.relative_to(wurzel).as_posix()}: {mit_fader} Kanaele mit Fader")
    print(gezogen, "Loops nachgezogen,", uebrig, "waren es schon")


def main():
    teile = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    teile.add_argument("--messung", help="TSV aus test_LoopPegel (LUMIVIZ_PEGEL_DATEI)")
    teile.add_argument("--loops", action="store_true", help="die mitgelieferten Loops nachziehen")
    teile.add_argument("--wurzel", default=str(HIER), help="Ordner asset/loop (oder eine Kopie)")
    args = teile.parse_args()
    if args.messung:
        messung(args.messung)
    if args.loops:
        loops(Path(args.wurzel))


if __name__ == "__main__":
    main()
