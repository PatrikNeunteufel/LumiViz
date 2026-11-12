"""Das mitgelieferte Beispiel des Kerns `freeReed` (S192, Stufe 34j-9; docs/audio/Instrumente_IC_Konzept.md 34j-9, IK2-8 a:
„je Klasse ein Beispiel <name>_demo“).

    python asset/loop/make_zungen_loops.py [--wurzel <kopie von asset/loop>]
    python asset/loop/instrument_pegel.py --loops [--wurzel <kopie>]
    python asset/loop/instrument_katalog.py

Schreibt `asset/loop/modelled_instruments/free_reeds_demo.lvloop`: acht Takte, 104 bpm, zweimal Am – Dm – E – Am. Die
ersten vier Takte das Akkordeon (Musette: zwei Choere gegeneinander verstimmt) mit der Melodie und auf der Bass-Seite
(unter F3: Grundton und Quinte auf 1 und 3, der Akkord in der Lage des Basses auf 2 und 4), dazu das Harmonium mit
liegenden Akkorden; die letzten vier die Melodica mit der Melodie und die Mundharmonika mit Akkorden auf den Achteln
dazwischen, Bass und Harmonium laufen weiter. **Crescendo ueber die Expression am Clip** (ZV3 a: an einer Zunge ist die
Expression der Wind): das Harmonium schwillt ueber sechs Takte von 0,3 auf 1 an, der Bass ueber den ersten Takt von 0,5.
Keine Spuren am Muster (der Balgwechsel als Spur kommt mit 31g). Je Muster ein Kanal, Pan nach Lage. Selbst gesetzt,
kein fremdes Werk.

Die Lautstaerken der Muster sind fuer den Pegel der Instrumente auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt =
neu); die Lautstaerke des Loops kommt aus der gemessenen Tafel `loop_lautstaerke.LAUT` (LV1 a). Kein mitgelieferter Loop
wird auf die neuen Instrumente umgestellt (IK2-8 a). Kein Zufall: zwei Laeufe ergeben dieselbe Datei.
"""

import argparse
from pathlib import Path

from loop_lautstaerke import laut
from make_examples import TAKT, als_text, je_takt, loop, muster

HIER = Path(__file__).resolve().parent

# Am – Dm – E – Am, die rechte Hand ueber F3 (die Bass-Seite des Akkordeons liegt darunter, `bassBelow` 53)
AKKORDE = [(57, 60, 64), (57, 62, 65), (56, 59, 64), (57, 60, 64)]
# (Grundton, Quinte, Akkord) auf der Bass-Seite
BASS = [(45, 40, (48, 52)), (38, 45, (45, 50)), (40, 35, (44, 47)), (45, 40, (48, 52))]
# (Schritt, Laenge, Hoehe, Anschlag) je Takt; Viertel = 4 Schritte
MELODIE = [
    [(0, 4, 76, 0.8), (4, 2, 72, 0.6), (6, 2, 74, 0.65), (8, 4, 76, 0.75), (12, 4, 81, 0.8)],
    [(0, 6, 77, 0.8), (6, 2, 76, 0.6), (8, 4, 74, 0.7), (12, 4, 69, 0.6)],
    [(0, 4, 71, 0.75), (4, 2, 68, 0.6), (6, 2, 71, 0.65), (8, 4, 76, 0.8), (12, 4, 74, 0.65)],
    [(0, 6, 72, 0.8), (6, 2, 71, 0.6), (8, 8, 69, 0.75)],
]


def melodie():
    return [(t * TAKT + s, l, h, a) for t, takt in enumerate(MELODIE) for s, l, h, a in takt]


def bass():
    noten = []
    for t, (grund, quinte, akk) in enumerate(BASS):
        noten += [(t * TAKT, 3, grund, 0.8), (t * TAKT + 8, 3, quinte, 0.7)]
        noten += [(t * TAKT + s, 2, h, 0.55) for s in (4, 12) for h in akk]
    return noten


def free_reeds_demo():
    akkordeon = muster("Accordion", "accordion", melodie(), laut=0.7, pan=-0.1)
    akkordeon_bass = muster("Accordion bass", "accordion", bass(), laut=0.6, pan=-0.25)
    harmonium = muster("Harmonium", "harmonium", je_takt(AKKORDE, lambda t, a: [(0, 16, h, 0.6) for h in a]), laut=0.5, pan=0.3)
    melodica = muster("Melodica", "melodica", melodie(), laut=0.65, pan=0.15)
    harmonika = muster("Harmonica", "harmonica",
                       je_takt(AKKORDE, lambda t, a: [(s, 2, h + 12, 0.55) for s in (2, 6, 10, 14) for h in a]), laut=0.5, pan=-0.3)
    kanaele = [("Accordion", [("Accordion", 0, 1)]), ("Accordion bass", [("Accordion bass", 0, 2)]),
               ("Harmonium", [("Harmonium", 0, 2)]), ("Melodica", [("Melodica", 4, 1)]), ("Harmonica", [("Harmonica", 4, 1)])]
    daten = loop("Free reeds demo", "Folk", 104, laut("modelled_instruments/free_reeds_demo", 0.3),  # LV1 a
                 [akkordeon, akkordeon_bass, harmonium, melodica, harmonika], kanaele)
    # das Crescendo am Clip (in Takten ab dem Beginn des Clips; seit Fassung 12 gelesen)
    kurven = {"Harmonium": [[0.0, 0.3], [6.0, 1.0]], "Accordion bass": [[0.0, 0.5], [1.0, 1.0]]}
    for k in daten["channels"]:
        for c in k["clips"]:
            c["expression"] = kurven.get(c["pattern"], [])
    return daten


def main():
    teile = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    teile.add_argument("--wurzel", default=str(HIER), help="Ordner asset/loop (oder eine Kopie)")
    ziel = Path(teile.parse_args().wurzel) / "modelled_instruments"
    ziel.mkdir(parents=True, exist_ok=True)
    (ziel / "free_reeds_demo.lvloop").write_text(als_text(free_reeds_demo()), encoding="utf-8", newline="\n")
    print("1 Loop der Zungen")


if __name__ == "__main__":
    main()
