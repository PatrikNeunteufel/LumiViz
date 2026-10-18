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
from make_examples import als_text

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
        schreibe(INSTRUMENTE / f"{name}.lvinst", ergaenze(name, neu, genres))
        print(f"{name}: level {alt.get('level', 1.0)} -> {neu['level']}")


if __name__ == "__main__":
    main()
