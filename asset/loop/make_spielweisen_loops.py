"""Beispiel-Loops zu den Spielweisen aus Stufe 24 (S136, Entscheid Patrik S-W22 a;
docs/audio/Saiten_Konzept.md §3).

    python asset/loop/make_spielweisen_loops.py

Schreibt vier `asset/loop/*_demo.lvloop`, je einer mit klarem Schwerpunkt:

- `strum_demo` — Folk, Guitar: G D Em C, die Hand in Achteln (D D U U D U ergibt
  sich aus der abwechselnden Hand, weil die Ups auf den Achteln dazwischen liegen;
  ein Up trifft nur die oberen Saiten), im vierten Takt *Together* auf der Eins
  und ein langsam gebrochener Schlussakkord (Spread 40 ms je Saite).
- `palm_mute_demo` — Rock, Drop D Guitar und Finger Bass: Powerchords in
  gedaempften Achteln, nur Downstrokes (Muster der Hand `DDDDDDDD`), die Eins
  jedes Takts offen; der Schlussakkord mit *Let ring*.
- `harmonics_demo` — Ambient, Clean Guitar: natuerliche Flageoletts am 12., 7.
  und 5. Bund mit *Let ring*, darunter die Nylon Guitar mit einem Arpeggio und
  *Let ring*.
- `tapping_demo` — Rock, Overdrive Guitar: derselbe Lauf auf der hohen E-Saite,
  zwei Takte getippt, zwei gezupft — zum Vergleich.

Jedes Muster mit Saiteninstrument traegt das Strum seines Instruments
(`SPREAD_MS` aus `make_saiten_instrumente.py`), wo noetig mit eigenem Muster der
Hand. `make_saiten_instrumente.strum_in_loops` laesst diese Loops aus
(`EIGENES_STRUM`).

**Reihenfolge:** `make_saiten_loops.py`, dieses Skript, dann
`make_saiten_instrumente.py` (es zaehlt die Genres auch aus diesen Beispielen).
Zwei Laeufe ergeben dieselben Dateien. Alles nach Rechnung gesetzt, nicht nach
Gehoer.
"""

from make_examples import HIER, als_text
from make_genres import Bau
from make_saiten_instrumente import EIGENES_STRUM, SPREAD_MS

# die Bits von `Note::artikulationen` (include/loop/Spielweise.hpp)
PALM_MUTE, LET_RING, TAPPING, HARMONIC = 1, 2, 4, 8
KEINE_SAITE = -1


def saiten_muster(b, name, inst, noten, laut=0.8, pan=0.0, hand=None, anschlaege=()):
    """Ein Muster mit Saiteninstrument. `noten`: (Schritt, Laenge, Tonhoehe, Staerke[, Saite[,
    Artikulationen[, Bend]]]) — der Bend als `[[zeit, halbtoene], …]` (`.lvloop` Fassung 10, seit S138 fuer
    `make_tab_loops.py`); `hand`: das Muster der Hand als Text (`D`/`U` je Stelle) oder None (Alternate);
    `anschlaege`: (Schritt, Art, Spread oder None) — Art `hand`, `down`, `up`, `together`."""
    zeilen = []
    for n in noten:
        schritt, laenge, hoehe, staerke = n[:4]
        saite = n[4] if len(n) > 4 else KEINE_SAITE
        art = n[5] if len(n) > 5 else 0
        bend = n[6] if len(n) > 6 else None
        assert 0 <= schritt and laenge >= 1 and schritt + laenge <= b.schritte * 4, (name, schritt, laenge)
        assert 21 <= hoehe <= 108 and 0 < staerke <= 1, (name, hoehe, staerke)
        assert not bend or all(0 <= z <= 1 and 0 <= h <= 4 for z, h in bend), (name, bend)
        zeile = [schritt, laenge, hoehe, staerke]
        if saite >= 0 or art or bend:
            zeile.append(saite)
        if art or bend:
            zeile.append(art)
        if bend:
            zeile.append(bend)
        zeilen.append(zeile)
    strum = {"spread": SPREAD_MS[inst], "hand": "halfBeat"}
    if hand:
        strum["pattern"] = hand
    muster = {"name": name, "kind": "instrument", "bars": 4, "instrument": inst, "volume": laut, "pan": pan,
              "mute": False, "notes": sorted(zeilen), "stringChoice": "perChord", "strum": strum}
    if anschlaege:
        muster["strokes"] = [dict({"step": s, "stroke": art}, **({"spread": sp} if sp is not None else {}))
                             for s, art, sp in anschlaege]
    b.patterns.append(muster)
    return name


def fertig(b):
    loop = b.fertig()
    loop["version"] = 9  # Strum, Anschlaege und Artikulationen gibt es seit Fassung 9
    return loop


# --- strum_demo ---------------------------------------------------------------------------

def strum():
    b = Bau("Porch Swing", "Folk", 92, 0.4)
    # je Takt der Akkord fuer Down (alle Saiten) und Up (die oberen) — G D Em C in offenen Lagen
    akk = [((43, 47, 50, 55, 59, 67), (55, 59, 67)), ((50, 57, 62, 66), (57, 62, 66)),
           ((40, 47, 52, 55, 59, 64), (55, 59, 64)), ((48, 52, 55, 60, 64), (55, 60, 64))]
    noten = []
    for takt, (ab, auf) in enumerate(akk):
        t = 16 * takt
        if takt < 3:
            schlaege = ((0, 4, ab, 0.8), (4, 2, ab, 0.75), (6, 4, auf, 0.6), (10, 2, auf, 0.6), (12, 2, ab, 0.75),
                        (14, 2, auf, 0.6))
        else:
            # die Eins zugleich (Together), dann ein Down, ein Up und der langsam gebrochene Schluss
            schlaege = ((0, 4, ab, 0.85), (4, 2, ab, 0.7), (6, 6, auf, 0.6), (12, 4, ab, 0.8))
        noten += [(t + s, l, h, v) for s, l, a, v in schlaege for h in a]
    gitarre = saiten_muster(b, "Guitar", "guitar", noten, 0.85, 0.15,
                            anschlaege=((48, "together", None), (60, "hand", 40.0)))
    beat = b.drums("Brush", [("Kick", "kit:kick", 0.5, 0.0, "x.......x......."),
                             ("Snare", "kit:snare", 0.35, 0.0, "....x.......x..."),
                             ("Hi-Hat", "kit:hat_closed", 0.3, 0.3, "x.x.x.x.x.x.x.x.")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Guitar", (gitarre, 0, 2))
    return fertig(b)


# --- palm_mute_demo -----------------------------------------------------------------------

def palm_mute():
    b = Bau("Chug", "Rock", 132, 0.35)
    akk = [(38, 45, 50), (41, 48, 53), (43, 50, 55), (38, 45, 50)]
    grund = [38, 41, 43, 38]  # der Bass auf den Grundtoenen der tiefen Saite (D2 ist seine leere D-Saite)
    gitarre, bass = [], []
    for takt, (a, g) in enumerate(zip(akk, grund)):
        t = 16 * takt
        if takt < 3:
            gitarre += [(t, 2, h, 0.9) for h in a]                                         # die Eins offen
            gitarre += [(t + s, 2, h, 0.75, KEINE_SAITE, PALM_MUTE) for s in range(2, 16, 2) for h in a[:2]]
            bass += [(t, 2, g, 0.9)] + [(t + s, 2, g, 0.75, KEINE_SAITE, PALM_MUTE) for s in range(2, 16, 2)]
        else:
            gitarre += [(t, 16, h, 0.9, KEINE_SAITE, LET_RING) for h in a]                 # klingt aus
            bass += [(t, 16, g, 0.9)]
    riff = saiten_muster(b, "Riff", "drop_d_guitar", gitarre, 0.8, -0.2, hand="DDDDDDDD")
    tief = saiten_muster(b, "Bass", "finger_bass", bass, 0.9, 0.1, hand="DDDDDDDD")
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.9, 0.0, ["x.x...x.x.x...x.", "x.x...x.x.x...x.", "x.x...x.x.x...x.",
                                                            "x..............."]),
                            ("Snare", "kit:snare", 0.8, 0.0, ["....x.......x...", "....x.......x...", "....x.......x...",
                                                              "................"]),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, ["x.x.x.x.x.x.x.x.", "x.x.x.x.x.x.x.x.",
                                                                     "x.x.x.x.x.x.x.x.", "x..............."])])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Riff", (riff, 0, 2))
    b.kanal("Bass", (tief, 0, 2))
    return fertig(b)


# --- harmonics_demo -----------------------------------------------------------------------

def harmonics():
    b = Bau("Glass Bells", "Ambient", 76, 0.4)
    # klingende Toene natuerlicher Flageoletts (S-W7 a: die Piano-Roll zeigt den Klang): 12., 7., 5. Bund
    glocken = [(52, 59, 64, 67), (62, 69, 74, 76), (52, 59, 64, 67), (64, 71, 76, 79)]
    # je Note ein Schlag: nacheinander, nicht ueberlappend — sonst waere die Saite belegt und die
    # Saitenwahl nahm ein kuenstliches Flageolett; Let ring laesst jede klingen bis zur naechsten Note
    # auf ihrer Saite
    oben = [(16 * t + 4 * i, 4, h, 0.8, KEINE_SAITE, HARMONIC | LET_RING)
            for t, motiv in enumerate(glocken) for i, h in enumerate(motiv)]
    arpeggio = [(40, 47, 52, 55, 59, 55, 52, 47), (48, 52, 55, 60, 64, 60, 55, 52),
                (43, 47, 50, 55, 59, 55, 50, 47), (50, 57, 62, 66, 69, 66, 62, 57)]
    unten = [(16 * t + 2 * i, 2, h, 0.6 if i else 0.7, KEINE_SAITE, LET_RING)
             for t, folge in enumerate(arpeggio) for i, h in enumerate(folge)]
    glocke = saiten_muster(b, "Harmonics", "clean_guitar", oben, 0.8, 0.3)
    nylon = saiten_muster(b, "Arpeggio", "nylon_guitar", unten, 0.75, -0.25)
    beat = b.drums("Pulse", [("Kick", "kit:kick", 0.4, 0.0, "x.........x....."),
                             ("Hi-Hat", "kit:hat_closed", 0.2, 0.3, "..x...x...x...x.")])
    b.kanal("Pulse", (beat, 4, 1))
    b.kanal("Harmonics", (glocke, 0, 2))
    b.kanal("Arpeggio", (nylon, 0, 2))
    return fertig(b)


# --- tapping_demo -------------------------------------------------------------------------

HOHE_E = 5  # die hohe E-Saite (die Stelle in `strings.open`)


def tapping():
    b = Bau("Tap Dance", "Rock", 112, 0.35)
    # auf der hohen E-Saite je Schlag: oben getippt, dann zwei Lagen der linken Hand — Am, F, Am, F
    lauf = {"Am": (81, 69, 72, 69), "F": (77, 65, 69, 65)}
    folge = ["Am", "F", "Am", "F"]
    noten = []
    for takt, akkord in enumerate(folge):
        art = TAPPING if takt < 2 else 0  # zwei Takte getippt, zwei gezupft
        noten += [(16 * takt + 4 * schlag + i, 1, h, 0.85 if i == 0 else 0.7, HOHE_E, art)
                  for schlag in range(4) for i, h in enumerate(lauf[akkord])]
    gitarre = saiten_muster(b, "Tapping", "overdrive_guitar", noten, 0.75, 0.0)
    bass = saiten_muster(b, "Bass", "finger_bass",
                         [(16 * t, 16, g, 0.85) for t, g in enumerate((33, 29, 33, 29))], 0.85, 0.0)
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.8, 0.0, "x.......x.x....."),
                            ("Snare", "kit:snare", 0.7, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.4, 0.25, "x.x.x.x.x.x.x.x.")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Tapping", (gitarre, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return fertig(b)


BEISPIELE = {"strum_demo": strum, "palm_mute_demo": palm_mute, "harmonics_demo": harmonics, "tapping_demo": tapping}
assert tuple(BEISPIELE) == EIGENES_STRUM, "EIGENES_STRUM in make_saiten_instrumente.py nachziehen"


def main():
    for name, bau in BEISPIELE.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(BEISPIELE), "Beispiele der Spielweisen")


if __name__ == "__main__":
    main()
