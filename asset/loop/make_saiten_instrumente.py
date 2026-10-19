"""Stellt die mitgelieferten Saiteninstrumente auf die Saiten aus Stufe 23 um
(S133, Entscheid Patrik S-G1 a: alle acht umstellen, die alten als "old"
behalten; docs/audio/Saiten_Konzept.md §5l).

    python asset/loop/make_saiten_instrumente.py

Je Instrument in `UMSTELLUNG`:

- `<name>_old.lvinst` ist die Fassung davor, mit "(old)" im Titel. Sie wird nur
  beim ersten Lauf aus der heutigen Datei angelegt und danach nie wieder
  geschrieben.
- `<name>.lvinst` entsteht immer aus `<name>_old.lvinst`: dazu die Tuning
  (`strings`), und jede Schicht *Plucked string* bekommt das Verfahren und die
  Regler aus der Tabelle; die anderen Schichten bleiben. `level` wird mit dem
  Faktor `pegel` angeglichen, damit die Spur im Loop so laut bleibt wie vorher
  (gemessen an den Hoerproben, +-1 dB).

Dazu sechs neue (S-G3 a): Jazz Guitar, Finger Bass, Ukulele, Drop D Guitar,
Pizzicato Violin, Piezo Guitar — gebaut aus `make_examples.schicht`, Werte in
`NEU`.

Seit S136 (Stufe 24f, S-W16 a bis S-W21 a) schreibt es die Instrumente mit Tuning in
Fassung 11 mit den Vorgaben fuer Strum und *Playing* (`SPREAD_MS`, `MUTE`) und gibt zum
Schluss jedem Muster der mitgelieferten Loops mit einem Saiteninstrument dessen Strum
(`strum_in_loops`).

**Reihenfolge:** zuerst `make_saiten_loops.py` und `make_spielweisen_loops.py` (die
Beispiel-Loops, aus denen die Genres gezaehlt werden), dann dieses Skript.

Zwei Laeufe ergeben dieselben Dateien. **Wer `make_examples.py` oder
`make_genres.py` neu laufen laesst, laesst danach dieses Skript laufen** —
jene schreiben die acht als Classic.

Die Werte je Instrument sind von Claude gesetzt, nicht gemessen (die Korpusse
dagegen sind belegt, §6/§6b); der Kontrabass hat noch keinen Korpus und
spielt ohne Body mit der Mensur der Vorlage Bass 4.
"""

import copy
import json

from instrument_katalog import HIER, ergaenze, genres_aus_loops
from make_examples import als_text, instrument, schicht

INSTRUMENTE = HIER / "instruments"

GITARRE = {"open": [40, 45, 50, 55, 59, 64], "capo": 0, "stringChoice": "perChord", "scale": 648}
NYLON = dict(GITARRE, scale=650)
BASS4 = {"open": [28, 33, 38, 43], "capo": 0, "stringChoice": "perChord", "scale": 864}


def abnehmer(art, mm):
    return {"type": art, "position": mm}


# Name -> (Tuning oder None, was jede Saiten-Schicht bekommt, Faktor fuer `level`)
# und, wo noetig, was eine andere Schicht bekommt (nach ihrem Namen): `ANDERE`
UMSTELLUNG = {
    "guitar": (GITARRE, dict(stringModel="extendedKS", body="parlor", bodyAmount=0.6), 1.0),
    "nylon_guitar": (NYLON, dict(stringModel="extendedKS", body="classical", bodyAmount=0.6, pluck=0.35,
                                 pluckPosition=0.18, highDamping=0.5, stiffness=0.05), 0.851),
    "clean_guitar": (GITARRE, dict(stringModel="waveguide", body="none", ringTime=2.5,
                                   pickupNeck=abnehmer("singleCoil", 160), pickupBridge=abnehmer("singleCoil", 41),
                                   pickupSelect="both", cable=500), 0.569),
    # an der Grenze `level` 1,0 waere es rund 3,7 dB leiser als vorher (Ring time, Pluck, Both und
    # Extended KS aendern daran nichts — S133 gemessen); S-G2 a: die Schicht Thud von 0,4 auf 0,6
    "muted_guitar": (GITARRE, dict(stringModel="waveguide", body="none", ringTime=0.25, highDamping=0.8,
                                   pickupNeck=abnehmer("none", 160), pickupBridge=abnehmer("singleCoil", 41),
                                   pickupSelect="bridge", cable=500), 1.25),
    "overdrive_guitar": (GITARRE, dict(stringModel="waveguide", body="none", ringTime=4.0,
                                       pickupNeck=abnehmer("none", 160), pickupBridge=abnehmer("humbucker", 41),
                                       pickupSelect="bridge", cable=500), 1.135),
    "upright_bass": (BASS4, dict(stringModel="extendedKS", body="none", pluck=0.3, pluckPosition=0.25, ringTime=2.0,
                                 highDamping=0.6), 1.122),
    # Pegel 1,25 statt 1,349: so bleibt A4 unter der Spitze 0,75, die jedes mitgelieferte Instrument
    # einhaelt (test_LoopInstrument); die Spur ist dafuer rund 0,5 dB leiser als vorher
    "slap_bass": (BASS4, dict(stringModel="waveguide", body="none", pluck=1.0, pluckPosition=0.35, ringTime=1.2,
                              pickupNeck=abnehmer("none", 160), pickupBridge=abnehmer("singleCoil", 40),
                              pickupSelect="bridge", cable=500), 1.25),
    # ein Tasteninstrument: eine Saite je Taste, keine Tuning
    "clavinet": (None, dict(stringModel="waveguide", body="none", pluck=0.9, pluckPosition=0.1, ringTime=0.8,
                            pickupNeck=abnehmer("singleCoil", 120), pickupBridge=abnehmer("singleCoil", 30),
                            pickupSelect="both", cable=500), 1.175),
}


ANDERE = {
    "muted_guitar": {"Thud": {"level": 0.6}},  # S-G2 a (Patrik S133)
}

# --- neue Saiteninstrumente (S-G3 a, Patrik S133) ------------------------------
# Name -> (Titel, Tuning, Saite, Huelle der Schicht, level). Der Pegel ist so gestellt, dass die Spur
# auf einem Muster so laut ist wie die eines vergleichbaren mitgelieferten Instruments (gemessen S133):
# Jazz und Piezo wie die Nylon, Drop D wie die Overdrive, Ukulele wie die Guitar. Finger Bass (wie der
# Upright, mit dessen Sinus-Schicht) und Pizzicato Violin (wie die Guitar) erreichen ihren Bezug an der
# Grenze 1,0 nicht: −2,6 dB und −4,6 dB — so belassen (S-G4 a, Patrik S133), nachgestellt nach Gehoer
# an den Beispiel-Loops (§5l).
UKULELE = {"open": [67, 60, 64, 69], "capo": 0, "stringChoice": "perChord", "scale": 381}
VIOLINE = {"open": [55, 62, 69, 76], "capo": 0, "stringChoice": "perChord", "scale": 327}
DROP_D = dict(GITARRE, open=[38, 45, 50, 55, 59, 64])

NEU = {
    "jazz_guitar": ("Jazz Guitar", GITARRE,
                    dict(stringModel="waveguide", body="none", pluck=0.45, pluckPosition=0.2, ringTime=2.5,
                         highDamping=0.55, pickupNeck=abnehmer("humbucker", 160), pickupBridge=abnehmer("none", 41),
                         pickupSelect="neck", cable=700),
                    dict(attack=2, release=150), 0.51),
    "finger_bass": ("Finger Bass", BASS4,
                    dict(stringModel="waveguide", body="none", pluck=0.4, pluckPosition=0.2, ringTime=2.0,
                         highDamping=0.5, pickupNeck=abnehmer("singleCoil", 150), pickupBridge=abnehmer("singleCoil", 40),
                         pickupSelect="both", cable=500),
                    dict(attack=2, release=80), 1.0),
    "ukulele": ("Ukulele", UKULELE,
                dict(stringModel="extendedKS", body="none", pluck=0.4, pluckPosition=0.25, ringTime=1.2,
                     highDamping=0.4, stiffness=0.05),
                dict(attack=2, release=100), 0.878),
    "drop_d_guitar": ("Drop D Guitar", DROP_D,
                      dict(stringModel="waveguide", body="none", ringTime=4.0, pickupNeck=abnehmer("none", 160),
                           pickupBridge=abnehmer("humbucker", 41), pickupSelect="bridge", cable=500, drive=0.6),
                      dict(attack=2, release=120), 0.795),
    "pizzicato_violin": ("Pizzicato Violin", VIOLINE,
                         dict(stringModel="extendedKS", body="none", pluck=0.5, pluckPosition=0.3, ringTime=0.8,
                              highDamping=0.6),
                         dict(attack=1, release=80), 1.0),
    "piezo_guitar": ("Piezo Guitar", GITARRE,
                     dict(stringModel="waveguide", body="none", pluck=0.5, ringTime=3.0,
                          pickupNeck=abnehmer("none", 160), pickupBridge=abnehmer("piezo", 0), pickupSelect="bridge",
                          cable=500),
                     dict(attack=2, release=120), 0.505),
}


# --- Spielweisen, Stufe 24f (S136; docs/audio/Saiten_Konzept.md S-W16 a bis S-W20 a, §6c) -----------
# Spread = 8 ms x Saitenabstand am Anschlagort / 11,17 mm (Freire 2018 an der Yamaha CG182S), je
# Bauart; welche Bauart je Instrument gilt, hat Claude nach Koerper und Tonabnehmern zugeordnet.
SPREAD_MS = {
    "nylon_guitar": 8.0,                                    # Klassik
    "guitar": 7.5, "piezo_guitar": 7.5,                     # Steelstring (Martin)
    "clean_guitar": 7.0, "muted_guitar": 7.0,               # Strat, 2-Punkt-Tremolo (S-W20 a)
    "overdrive_guitar": 7.0, "drop_d_guitar": 7.0,          # Les Paul
    "jazz_guitar": 7.0,                                     # Archtop
    "finger_bass": 12.5, "slap_bass": 12.5,                 # E-Bass
    "upright_bass": 17.5,                                   # Kontrabass (Blog, S-W19 a)
    "ukulele": 9.5,                                         # Konzert
    "pizzicato_violin": 7.0,                                # Violine (Forum, S-W19 a)
}
# Mute: die Baesse nach IDMT-SMT-Bass (S-W17 a; der Upright uebertragen), sonst die Vorgabe
MUTE = {"finger_bass": 0.37, "slap_bass": 0.37, "upright_bass": 0.37}
# die Vorgaben des Programms (include/loop/Instrument.hpp, Spielweise.hpp) — S-W11 a: immer schreiben;
# seit S136 (Stufe 25c, Fassung 12) dazu Slide time und Legato level (S-L10 a, S-L11 a)
PLAYING = {"upLevel": 1.0, "mute": 0.6, "tapLevel": 0.75, "touch": 0.3, "slideTime": 20.0, "legatoLevel": 0.38}
STRUM = {"spread": 8.0, "hand": "halfBeat"}
# ohne Buende (S-L9 a): ein Slide gleitet stetig
FRETLESS = ("upright_bass", "pizzicato_violin")
# die Beispiele der Spielweisen (S-W22 a, `make_spielweisen_loops.py`) setzen ihr Strum selbst
EIGENES_STRUM = ("strum_demo", "palm_mute_demo", "harmonics_demo", "tapping_demo")


def spielweisen(name, daten):
    """Fassung 12: `strum` und `fretless` unter `strings`, `playing` an jeder Saite Extended KS /
    Waveguide."""
    daten["version"] = max(12, int(daten.get("version", 1)))
    if "strings" in daten:
        daten["strings"] = dict(daten["strings"], strum=dict(STRUM, spread=SPREAD_MS[name]), fretless=name in FRETLESS)
    for schicht in daten["layers"]:
        if schicht.get("waveform") == "string" and schicht.get("stringModel") in ("extendedKS", "waveguide"):
            schicht["playing"] = dict(PLAYING, mute=MUTE.get(name, PLAYING["mute"]))
    return daten


def strum_in_loops():
    """S-W21 a: jedes Muster eines mitgelieferten Loops mit Saiteninstrument bekommt dessen Vorgaben
    (String choice und Strum) wie ein neues Muster im Programm (`uebernimmVorgaben`); der Loop dann in
    Fassung 9. Mitgeliefert ist, was `asset/Asset.cmake` nennt: `*_demo`, `genres/`, `vergleich/`."""
    vorgaben = {}
    for datei in INSTRUMENTE.glob("*.lvinst"):
        saiten = lies(datei).get("strings")
        if saiten and "strum" in saiten:
            vorgaben[datei.stem] = (saiten.get("stringChoice", "perChord"), saiten["strum"])
    dateien = (sorted(HIER.glob("*_demo.lvloop")) + sorted((HIER / "genres").glob("*.lvloop"))
               + sorted((HIER / "vergleich").glob("*.lvloop")))
    zahl = 0
    for datei in dateien:
        if datei.stem in EIGENES_STRUM:
            continue
        loop = lies(datei)
        geaendert = False
        for muster in loop.get("patterns", []):
            vorgabe = vorgaben.get(muster.get("instrument")) if muster.get("kind") == "instrument" else None
            if vorgabe is not None:
                muster["stringChoice"], muster["strum"] = vorgabe[0], copy.deepcopy(vorgabe[1])
                geaendert = True
        if geaendert:
            loop["version"] = max(9, int(loop.get("version", 2)))
            schreibe(datei, loop)
            zahl += 1
    print(zahl, "Loops mit Strum der Instrumente")


def baue_neu(name, titel, tuning, saite, huelle, level):
    schichten = [schicht("String", "string", **huelle)]
    schichten[0].update(copy.deepcopy(saite))
    if name == "finger_bass":
        # wie Upright und Slap Bass: eine Sinus-Schicht unter der Saite — die des Upright
        upright = lies(INSTRUMENTE / "upright_bass_old.lvinst")
        schichten.append(copy.deepcopy([s for s in upright["layers"] if s.get("waveform") == "sine"][0]))
    daten = instrument(titel, level, *schichten)
    daten["version"] = 10
    daten["strings"] = tuning
    return daten


def lies(pfad):
    return json.loads(pfad.read_text(encoding="utf-8"))


def schreibe(pfad, daten):
    pfad.write_text(als_text(daten), encoding="utf-8", newline="\n")


def main():
    genres = genres_aus_loops()
    for name, (tuning, saite, pegel) in UMSTELLUNG.items():
        alt_pfad = INSTRUMENTE / f"{name}_old.lvinst"
        if not alt_pfad.exists():
            alt = lies(INSTRUMENTE / f"{name}.lvinst")
            alt["title"] = alt["title"] + " (old)"
            schreibe(alt_pfad, ergaenze(f"{name}_old", alt, genres))
        alt = lies(alt_pfad)
        neu = copy.deepcopy(alt)
        neu["title"] = alt["title"].removesuffix(" (old)")
        neu["version"] = max(10, int(alt.get("version", 1)))
        neu["level"] = round(min(1.0, alt.get("level", 1.0) * pegel), 3)
        if tuning is not None:
            neu["strings"] = tuning
        for schicht in neu["layers"]:
            if schicht.get("waveform") == "string":
                schicht.update(copy.deepcopy(saite))
            elif schicht.get("name") in ANDERE.get(name, {}):
                schicht.update(copy.deepcopy(ANDERE[name][schicht["name"]]))
        if tuning is not None:
            spielweisen(name, neu)  # das Clavinet hat keine Tuning: es bleibt in Fassung 10
        schreibe(INSTRUMENTE / f"{name}.lvinst", ergaenze(name, neu, genres))
        print(f"{name}: level {alt.get('level', 1.0)} -> {neu['level']}")
    for name, (titel, tuning, saite, huelle, level) in NEU.items():
        daten = spielweisen(name, baue_neu(name, titel, tuning, saite, huelle, level))
        schreibe(INSTRUMENTE / f"{name}.lvinst", ergaenze(name, daten, genres))
        print(f"{name}: neu, level {level}")
    strum_in_loops()


if __name__ == "__main__":
    main()
