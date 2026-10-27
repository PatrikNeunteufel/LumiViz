"""Die mitgelieferten Loops des Fluegels (S157, Stufe 31d-8 „Spielbar“, Entscheid Patrik SP1 a;
docs/audio/Spielweisen_Konzept.md §4.1, Teilplan 31d-8, Schritt 8.4).

    python asset/loop/make_fluegel_loops.py
    python asset/loop/instrument_pegel.py --loops

Schreibt `asset/loop/grand_piano_demo.lvloop`: acht Takte, 72 bpm, linke Hand gebrochene
Akkorde (Grundton, Quinte, Terz eine Oktave hoeher, Oktave als Achtel; C, Am, F, G, C, Am, G,
im letzten Takt C gehalten), rechte Hand eine Melodie aus Vierteln und Halben. Keine Spuren,
kein Pedal (Mitschwingen kommt mit 31e). Selbst gesetzt, kein fremdes Werk.

Danach `instrument_pegel.py --loops` (zieht den Loop auf den Pegel der Instrumente nach, wie
jeden anderen) und `instrument_katalog.py` (das Genre des Loops wird das Genre von
`grand_piano`). Stufe 31h-3 baut dieses Skript fuer die Chopin-Loops aus (BL1 a, BL2 a).
Kein Zufall: zwei Laeufe ergeben dieselbe Datei.
"""

from pathlib import Path

from make_examples import TAKT, als_text, loop, muster

HIER = Path(__file__).resolve().parent

ACHTEL = TAKT // 8

# je Takt der Grundton der linken Hand (MIDI); der letzte Takt haelt C
AKKORDE = [36, 33, 41, 43, 36, 33, 43]


def linke_hand():
    noten = []
    for takt, grund in enumerate(AKKORDE):
        # Grundton, Quinte, Terz eine Oktave hoeher, Oktave — zweimal je Takt; Dur ausser auf A (Moll)
        terz = 15 if grund == 33 else 16
        folge = [grund, grund + 7, grund + terz, grund + 12] * 2
        for i, hoehe in enumerate(folge):
            noten.append((takt * TAKT + i * ACHTEL, ACHTEL, hoehe, 0.55 if i == 0 else 0.45))
    schluss = len(AKKORDE) * TAKT
    noten += [(schluss, TAKT, h, 0.5) for h in (36, 43, 52)]
    return noten


# (Schritt im Takt, Laenge, Hoehe, Anschlag) je Takt; Viertel = 4 Schritte
MELODIE = [
    [(0, 4, 76, 0.7), (4, 4, 74, 0.6), (8, 8, 72, 0.65)],
    [(0, 4, 72, 0.6), (4, 4, 76, 0.7), (8, 8, 81, 0.8)],
    [(0, 4, 79, 0.7), (4, 4, 77, 0.6), (8, 4, 76, 0.6), (12, 4, 74, 0.55)],
    [(0, 8, 74, 0.65), (8, 4, 71, 0.55), (12, 4, 67, 0.5)],
    [(0, 4, 72, 0.65), (4, 4, 76, 0.7), (8, 8, 79, 0.75)],
    [(0, 4, 81, 0.8), (4, 4, 79, 0.7), (8, 8, 76, 0.65)],
    [(0, 4, 74, 0.6), (4, 4, 71, 0.55), (8, 4, 74, 0.6), (12, 4, 67, 0.5)],
    [(0, 16, 72, 0.6)],
]


def rechte_hand():
    return [(takt * TAKT + s, l, h, a) for takt, noten in enumerate(MELODIE) for s, l, h, a in noten]


def grand_piano_demo():
    takte = len(MELODIE)
    return loop(
        "Grand piano demo", "Classical", 72, 0.35,
        [muster("Left hand", "grand_piano", linke_hand(), laut=0.8, pan=-0.2, takte=takte),
         muster("Right hand", "grand_piano", rechte_hand(), laut=0.8, pan=0.2, takte=takte)],
        [("Left", [("Left hand", 0, 1)]), ("Right", [("Right hand", 0, 1)])],
    )


LOOPS = {"grand_piano_demo": grand_piano_demo}


def main():
    for name, bau in LOOPS.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(LOOPS), "Loop(s) des Fluegels")


if __name__ == "__main__":
    main()
