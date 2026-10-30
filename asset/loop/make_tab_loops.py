"""Beispiel-Loops zu Tabs und Akkorden aus Stufe 26 (S138, Stufe 26h; docs/audio/Saiten_Konzept.md
S-T1 a bis S-T14 a, S-L1 a bis S-L15 b).

    python asset/loop/make_tab_loops.py

Schreibt zwei `asset/loop/*_demo.lvloop`:

- `tabs_demo` — Blues in A, Guitar und Overdrive Guitar: die Begleitung spielt **Griffbilder aus den
  Vorlagen** (`chords/guitar/a_7_1`, `d_7_1`, `e_7_1` — gelesen, jede Note auf ihrer Saite, wie das
  Fenster *Chords* sie setzt), die Lead-Linie in der Pentatonik am 5. Bund mit **Hammer-on**,
  **Pull-off** (beides Legato), **Slide**, **Bend** (full) und **Bend and release**; Finger Bass.
- `voicings_demo` — ii-V-I in C auf dem Tine Piano aus **Voicing-Vorlagen** (`chords/voicings/…`) in
  enger Stimmfuehrung: Dm7 in Grundstellung, G7 in der zweiten Umkehrung, Cmaj7, Am7 — der tiefste
  Ton je Akkord nah am vorigen; Round Bass auf den Grundtoenen.

Die Bend-Kurven wie die Vorlagen des Editors (`bendVorlage`, Spielweise.cpp). Die Saiten der Lead-Linie
sind gesetzt (S-M2 a), damit die Tabs zeigen, wo gespielt wird. **Reihenfolge:** nach
`make_spielweisen_loops.py`, vor `make_saiten_instrumente.py` (es zaehlt die Genres auch aus diesen
Beispielen und traegt das Strum der Instrumente ein). Zwei Laeufe ergeben dieselben Dateien. Alles nach
Rechnung gesetzt, nicht nach Gehoer.
"""

import json
import sys

from make_examples import HIER, als_text
from make_genres import Bau
from make_spielweisen_loops import saiten_muster

LEGATO, SLIDE = 16, 32
E_, A_, D_, G_, H_, HE_ = range(6)  # die Saiten der Gitarre, von der tiefen E-Saite an
AKKORDE = HIER / "chords"


def bend(hoehe):
    """Die Vorlage *Bend*: steigt im ersten Drittel, dann gehalten (Halbtoene)."""
    return [[0.0, 0.0], [0.3, hoehe]]


def bend_zurueck(hoehe):
    """Die Vorlage *Bend and release*."""
    return [[0.0, 0.0], [0.3, hoehe], [0.6, hoehe], [0.9, 0.0]]


def vorlage(nennung):
    """Ein Akkord der Vorlagen bei seiner Nennung `<sammlung>/<name>` (seit S167 in `<sammlung>.lvchords`)."""
    sammlung, name = nennung.split("/")
    for akkord in json.loads((AKKORDE / (sammlung + ".lvchords")).read_text(encoding="utf-8"))["chords"]:
        if akkord["name"] == name:
            return akkord
    sys.exit(f"keine Vorlage: {nennung}")


def griffbild(nennung):
    """Die Toene eines Griffbilds der Vorlagen: (Tonhoehe, Saite) je gespielter Saite."""
    daten = vorlage(nennung)["strings"]
    return [(leer + bund, s) for s, (leer, bund) in enumerate(zip(daten["open"], daten["frets"])) if bund >= 0]


def voicing(nennung, tiefster):
    """Die Toene eines Voicings der Vorlagen mit dem tiefsten auf `tiefster`."""
    return [tiefster + a for a in vorlage(nennung)["voicing"]]


def fertig(b):
    loop = b.fertig()
    loop["version"] = 10  # Legato, Slide und Bend gibt es seit Fassung 10
    return loop


# --- tabs_demo -----------------------------------------------------------------------------

def tabs():
    b = Bau("Bent Blue", "Blues", 92, 0.4, swing=0.15)
    # die Begleitung: je Takt das Griffbild auf Zwei und Vier — A7, A7, D7, E7
    folge = ["guitar/a_7_1", "guitar/a_7_1", "guitar/d_7_1", "guitar/e_7_1"]
    begleitung = [(16 * t + s, 3, h, v, saite) for t, n in enumerate(folge) for s, v in ((4, 0.75), (12, 0.7))
                  for h, saite in griffbild(n)]
    # die Lead-Linie: A-Moll-Pentatonik am 5. Bund (S-T5 a: in den Tabs sieht man die Bünde)
    lead = [
        # Takt 1: Hammer-on C4 → D4 auf der G-Saite, dann G4 auf der H-Saite bis A4 gebogen
        (8, 2, 60, 0.8, G_), (10, 2, 62, 0.75, G_, LEGATO), (12, 4, 67, 0.85, H_, 0, bend(2.0)),
        # Takt 2: A4, C5, Pull-off nach A4, G4, Slide hinunter nach E4, D4
        (16, 2, 69, 0.85, HE_), (18, 2, 72, 0.8, HE_), (20, 2, 69, 0.75, HE_, LEGATO),
        (22, 2, 67, 0.8, H_), (24, 4, 64, 0.8, H_, SLIDE), (28, 4, 62, 0.8, G_),
        # Takt 3: D4 einen Ganzton hinauf und zurück
        (40, 6, 62, 0.85, G_, 0, bend_zurueck(2.0)), (46, 2, 60, 0.75, G_),
        # Takt 4: A3, C4, Slide hinauf nach D4, zum Schluss A3 mit einem halben Bend
        (48, 2, 57, 0.8, D_), (50, 2, 60, 0.8, G_), (52, 4, 62, 0.85, G_, SLIDE), (56, 8, 57, 0.85, D_, 0, bend(1.0)),
    ]
    gitarre = saiten_muster(b, "Rhythm", "guitar", begleitung, 0.75, -0.25)
    solo = saiten_muster(b, "Lead", "overdrive_guitar", lead, 0.7, 0.2)
    bass = saiten_muster(b, "Bass", "finger_bass",
                         [(16 * t + s, 4, g + o, 0.85) for t, g in enumerate((33, 33, 38, 40))
                          for s, o in ((0, 0), (4, 7), (8, 12), (12, 7))], 0.85, 0.0)
    beat = b.drums("Shuffle", [("Kick", "kit:kick", 0.7, 0.0, "x.......x......."),
                               ("Snare", "kit:snare", 0.55, 0.0, "....x.......x..."),
                               ("Hi-Hat", "kit:hat_closed", 0.35, 0.3, "x.xxx.xxx.xxx.xx")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Rhythm", (gitarre, 0, 2))
    b.kanal("Lead", (solo, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return fertig(b)


# --- voicings_demo -------------------------------------------------------------------------

def voicings():
    b = Bau("Late Changes", "Jazz", 84, 0.4, swing=0.2)
    # ii-V-I-vi: der tiefste Ton nah am vorigen (D3, D3, E3, C3)
    folge = [("voicings/d_m7_root", 50), ("voicings/g_7_inv2", 50), ("voicings/c_maj7_inv1", 52),
             ("voicings/a_m7_inv1", 48)]
    klavier = []
    for t, (nennung, tief) in enumerate(folge):
        toene = voicing(nennung, tief)
        klavier += [(16 * t, 6, h, 0.7) for h in toene] + [(16 * t + 10, 4, h, 0.6) for h in toene]
    piano = b.noten("Keys", "tine_piano", klavier, 0.75, -0.1)
    bass = b.noten("Bass", "round_bass", [(16 * t + s, 4, g, 0.8) for t, g in enumerate((38, 43, 36, 45))
                                          for s in (0, 8)], 0.85, 0.0)
    beat = b.drums("Brush", [("Kick", "kit:kick", 0.4, 0.0, "x.........x....."),
                             ("Snare", "kit:snare", 0.3, 0.0, "....x.......x..."),
                             ("Ride", "kit:hat_closed", 0.3, 0.3, "x..xx..xx..xx..x")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Keys", (piano, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return fertig(b)


BEISPIELE = {"tabs_demo": tabs, "voicings_demo": voicings}


def main():
    for name, bau in BEISPIELE.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(BEISPIELE), "Beispiele zu Tabs und Akkorden")


if __name__ == "__main__":
    main()
