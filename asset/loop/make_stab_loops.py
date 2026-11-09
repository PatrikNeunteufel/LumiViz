"""Das mitgelieferte Beispiel des Kerns `bar` (S178, Stufe 34g; docs/audio/Instrumente_IC_Konzept.md 34g-6, IK2-8 a:
„je Klasse ein Beispiel <name>_demo“).

    python asset/loop/make_stab_loops.py
    python asset/loop/instrument_pegel.py --loops
    python asset/loop/instrument_katalog.py

Schreibt `asset/loop/modelled_instruments/bars_demo.lvloop` (seit S182 dort, IL3 a): acht Takte, 96 bpm, zweimal C – Am – F – G. Die ersten vier Takte die
Stabspiele mit Schlaegel (Marimba gebrochen, Vibraphon in Akkorden, Xylophon und Glockenspiel mit der Melodie, die
Roehrenglocken auf der Eins von Takt 1 und 3), die letzten vier die mit Tastatur oder Zunge (Celesta mit der Melodie,
Kalimba gebrochen, die beiden elektrischen Klaviere in Akkorden und im Bass). Keine Spuren (Motor, Bogen und Pedale
kommen mit 31g in den Editor). Je Instrument ein Kanal, Pan nach Lage. Selbst gesetzt, kein fremdes Werk.

Die Lautstaerken sind fuer den Pegel der Instrumente auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt = neu, wie
bei `grand_piano`). Kein mitgelieferter Loop wird auf die neuen Instrumente umgestellt (IK2-8 a). Kein Zufall: zwei
Laeufe ergeben dieselbe Datei.
"""

from pathlib import Path

from make_examples import TAKT, als_text, je_takt, loop, muster

HIER = Path(__file__).resolve().parent

# C – Am – F – G
AKKORDE = [(60, 64, 67), (57, 60, 64), (53, 57, 60), (55, 59, 62)]
# (Schritt, Laenge, Hoehe, Anschlag) je Takt; Viertel = 4 Schritte
MELODIE = [
    [(0, 4, 76, 0.8), (4, 4, 79, 0.7), (8, 6, 84, 0.85), (14, 2, 83, 0.6)],
    [(0, 4, 81, 0.8), (4, 4, 76, 0.65), (8, 8, 72, 0.7)],
    [(0, 4, 77, 0.8), (4, 4, 81, 0.7), (8, 4, 84, 0.8), (12, 4, 81, 0.65)],
    [(0, 6, 79, 0.8), (6, 2, 77, 0.6), (8, 8, 74, 0.7)],
]


def melodie(versatz=0):
    return [(t * TAKT + s, l, h + versatz, a) for t, takt in enumerate(MELODIE) for s, l, h, a in takt]


def mallets():
    marimba = je_takt(AKKORDE, lambda t, a: [(s, 2, (a[0] - 12, a[1] - 12, a[2] - 12, a[1], a[0], a[2] - 12, a[1] - 12, a[0])[i], 0.8 if i % 2 == 0 else 0.6)
                                             for i, s in enumerate(range(0, 16, 2))])
    vibes = je_takt(AKKORDE, lambda t, a: [(0, 8, h, 0.6) for h in a] + [(8, 8, h, 0.5) for h in (a[1], a[2], a[0] + 12)])
    xylophon = melodie()
    glocke = [(n[0], n[1], n[2] + 12, n[3] * 0.8) for n in melodie() if n[2] + 12 <= 96]
    roehren = [(0, 16, 60, 0.7), (2 * TAKT, 16, 57, 0.6)]
    return [
        muster("Marimba", "marimba", marimba, laut=0.8, pan=-0.3),
        muster("Vibraphone", "vibraphone", vibes, laut=0.7, pan=0.2),
        muster("Xylophone", "xylophone", xylophon, laut=0.6, pan=0.35),
        muster("Glockenspiel", "glockenspiel", glocke, laut=0.45, pan=0.5),
        muster("Tubular bells", "tubular_bells", roehren, laut=0.55, pan=-0.5),
    ]


def tasten():
    celesta = melodie()
    kalimba = je_takt(AKKORDE, lambda t, a: [(s, 2, (a[0], a[2], a[1], a[2])[i] + 12, 0.7) for i, s in enumerate((0, 4, 8, 12))])
    tine = je_takt(AKKORDE, lambda t, a: [(0, 6, h - 12, 0.65) for h in a] + [(8, 6, h - 12, 0.55) for h in a])
    reed = je_takt(AKKORDE, lambda t, a: [(0, 4, a[0] - 12, 0.8), (6, 2, a[0] - 5, 0.6), (8, 4, a[0] - 12, 0.7), (12, 4, a[0], 0.6)])
    return [
        muster("Celesta", "celesta", celesta, laut=0.6, pan=0.3),
        muster("Kalimba", "kalimba", kalimba, laut=0.6, pan=-0.35),
        muster("Tine piano", "tine_electric_piano", tine, laut=0.6, pan=-0.1),
        muster("Reed piano", "reed_electric_piano", reed, laut=0.7, pan=0.1),
    ]


def bars_demo():
    erste, zweite = mallets(), tasten()
    kanaele = [(m["name"], [(m["name"], 0, 1)]) for m in erste] + [(m["name"], [(m["name"], 4, 1)]) for m in zweite]
    return loop("Bars demo", "Ambient", 96, 0.3, erste + zweite, kanaele)


def main():
    (HIER / "modelled_instruments" / "bars_demo.lvloop").write_text(als_text(bars_demo()), encoding="utf-8", newline="\n")
    print("1 Loop der Stabspiele")


if __name__ == "__main__":
    main()
