"""Das mitgelieferte Beispiel der Klasse 1 ohne Fluegel (S181, Stufe 34i; docs/audio/Instrumente_IC_Konzept.md 34i-6,
IK2-8 a: „je Klasse ein Beispiel <name>_demo“, KL2 a: `keys_demo`).

    python asset/loop/make_klasse1_loops.py
    python asset/loop/instrument_pegel.py --loops
    python asset/loop/instrument_katalog.py

Schreibt `asset/loop/keys_demo.lvloop`: acht Takte, 104 bpm, zweimal F – Dm – Bb – C mit leichtem Swing. Die ersten
vier Takte die Klaviere mit Hammer (das Upright in Akkorden und Bass, das Honky-tonk mit der Melodie eine Oktave hoeher),
die letzten vier die elektrischen (der CP-70 in Akkorden, das Clavinet modal mit einem Riff in Sechzehnteln). Keine
Spuren (Pedale, Daempferschieber und Tonabnehmer kommen mit 31g in den Editor). Je Instrument ein Kanal, Pan nach Lage.
Selbst gesetzt, kein fremdes Werk. Der Umfang: CP-70 E1–E7 (28–100), Clavinet F1–E6 (29–88).

Die Lautstaerken sind fuer den Pegel der Instrumente auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt = neu, wie bei
`grand_piano`). Kein mitgelieferter Loop wird auf die neuen Instrumente umgestellt (IK2-8 a). Kein Zufall: zwei Laeufe
ergeben dieselbe Datei.
"""

from pathlib import Path

from make_examples import TAKT, als_text, je_takt, loop, muster

HIER = Path(__file__).resolve().parent

# F – Dm – Bb – C
AKKORDE = [(53, 57, 60), (50, 53, 57), (46, 50, 53), (48, 52, 55)]
# (Schritt, Laenge, Hoehe, Anschlag) je Takt; Viertel = 4 Schritte
MELODIE = [
    [(0, 3, 72, 0.8), (3, 1, 74, 0.6), (4, 4, 77, 0.75), (8, 2, 76, 0.65), (10, 6, 72, 0.7)],
    [(0, 4, 69, 0.75), (4, 2, 72, 0.6), (6, 2, 74, 0.6), (8, 8, 77, 0.8)],
    [(0, 3, 74, 0.75), (3, 1, 72, 0.6), (4, 4, 70, 0.7), (8, 4, 74, 0.7), (12, 4, 77, 0.65)],
    [(0, 6, 76, 0.8), (6, 2, 74, 0.6), (8, 8, 72, 0.7)],
]


def melodie(versatz=0):
    return [(t * TAKT + s, l, h + versatz, a) for t, takt in enumerate(MELODIE) for s, l, h, a in takt]


def hammer():
    upright = je_takt(AKKORDE, lambda t, a: [(0, 2, a[0] - 12, 0.75), (4, 2, a[0] - 5, 0.55), (8, 2, a[0] - 12, 0.7), (12, 2, a[0] - 5, 0.55)]
                      + [(s, 2, h + 12, 0.5) for s in (2, 6, 10, 14) for h in a])
    honky = [(s, l, h + 12, a) for s, l, h, a in melodie() if h + 12 <= 96]
    return [
        muster("Upright piano", "upright_piano", upright, laut=0.7, pan=-0.2),
        muster("Honky-tonk piano", "honky_tonk_piano", honky, laut=0.6, pan=0.25),
    ]


def elektrisch():
    cp70 = je_takt(AKKORDE, lambda t, a: [(0, 6, h + 12, 0.6) for h in a] + [(8, 4, h + 12, 0.5) for h in a] + [(0, 8, a[0] - 12, 0.7)])
    riff = je_takt(AKKORDE, lambda t, a: [(s, 1, (a[0], a[0] + 12, a[1] + 12, a[0] + 12)[i % 4], 0.8 if i % 4 == 0 else 0.55)
                                          for i, s in enumerate((0, 2, 3, 6, 8, 10, 11, 14))])
    return [
        muster("Electric grand", "electric_grand_piano", cp70, laut=0.6, pan=0.15),
        muster("Clavinet modal", "clavinet_modal", riff, laut=0.6, pan=-0.3),
    ]


def keys_demo():
    erste, zweite = hammer(), elektrisch()
    kanaele = [(m["name"], [(m["name"], 0, 1)]) for m in erste] + [(m["name"], [(m["name"], 4, 1)]) for m in zweite]
    return loop("Keys demo", "Jazz", 104, 0.3, erste + zweite, kanaele, swing=0.15)


def main():
    (HIER / "keys_demo.lvloop").write_text(als_text(keys_demo()), encoding="utf-8", newline="\n")
    print("1 Loop der Klasse 1")


if __name__ == "__main__":
    main()
