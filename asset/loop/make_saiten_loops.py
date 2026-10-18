"""Loops zu den Saiteninstrumenten aus Stufe 23 (S133, Wunsch Patrik: Vergleiche
der alten mit den neuen Instrumenten, und Beispiele mit den neuen).

    python asset/loop/make_saiten_loops.py

Schreibt

- `asset/loop/vergleich/*.lvloop` — je ein mitgelieferter Loop zweimal: zuerst
  mit den Instrumenten „(old)“ (`<name>_old`), danach mit den umgestellten. Die
  anderen Spuren spielen beide Male gleich. Der Ordner zaehlt nicht fuer die
  Genres der Instrumente (`instrument_katalog.genres_aus_loops` liest nur
  `*_demo.lvloop` und `genres/`): die alten bleiben ohne Genre.
- `asset/loop/*_demo.lvloop` — vier Beispiele mit den sechs neuen Instrumenten
  (S-G3 a): Jazz Guitar, Finger Bass, Ukulele, Drop D Guitar, Pizzicato Violin,
  Piezo Guitar.

**Reihenfolge:** zuerst dieses Skript, dann `make_saiten_instrumente.py` (es
zaehlt die Genres aus den Beispielen). Zwei Laeufe ergeben dieselben Dateien.
Alles nach Rechnung gesetzt, nicht nach Gehoer.
"""

import copy
import json

from make_examples import HIER, als_text
from make_genres import Bau, lauf, schlag

UMGESTELLT = ("guitar", "nylon_guitar", "clean_guitar", "muted_guitar", "overdrive_guitar", "upright_bass",
              "slap_bass", "clavinet")

# --- Vergleiche: Quelle -> Name der Datei unter vergleich/
VERGLEICHE = {
    "genres/bossa.lvloop": "bossa_old_new",
    "genres/funk.lvloop": "funk_old_new",
    "genres/disco.lvloop": "disco_old_new",
    "rock_demo.lvloop": "rock_old_new",
    "instrument_demo.lvloop": "guitar_old_new",
}


def vergleich(quelle):
    """Den Loop zweimal hintereinander: erst mit den alten, dann mit den neuen Instrumenten."""
    loop = json.loads((HIER / quelle).read_text(encoding="utf-8"))
    takte = {p["name"]: p.get("bars", 4) for p in loop["patterns"]}
    laenge = max(c["start"] + c["repeat"] * takte[c["pattern"]] for k in loop["channels"] for c in k["clips"])
    alt = {}
    for p in list(loop["patterns"]):
        if p.get("kind") == "instrument" and p.get("instrument") in UMGESTELLT:
            kopie = copy.deepcopy(p)
            kopie["name"] = p["name"] + " (old)"
            kopie["instrument"] = p["instrument"] + "_old"
            loop["patterns"].append(kopie)
            alt[p["name"]] = kopie["name"]
    for kanal in loop["channels"]:
        vorn = [dict(c, pattern=alt.get(c["pattern"], c["pattern"])) for c in kanal["clips"]]
        hinten = [dict(c, start=c["start"] + laenge) for c in kanal["clips"]]
        kanal["clips"] = vorn + hinten
    tags = loop.setdefault("tags", {})
    tags["title"] = tags.get("title", "Loop") + " — old, then new"
    return loop


# --- Beispiele mit den neuen Instrumenten ---------------------------------------------

def jazz():
    # ii-V-I in C mit Wendung: Dm7 G7 Cmaj7 A7; die Gitarre auf 2 und 4, der Bass laeuft in Vierteln
    b = Bau("Blue Room", "Jazz", 132, 0.35, swing=0.3)
    akk = [(50, 53, 57, 60), (55, 59, 62, 65), (48, 52, 55, 59), (45, 49, 52, 55)]
    gang = [(38, 41, 45, 48), (43, 47, 50, 41), (36, 40, 43, 45), (33, 37, 40, 43)]
    beat = b.drums("Swing", [("Kick", "kit:kick", 0.5, 0.0, "x.......x......."),
                             ("Snare", "kit:snare", 0.4, 0.1, "....x.......x..."),
                             ("Hi-Hat", "kit:hat_closed", 0.45, 0.3, "x..ox..ox..ox..o")])
    gitarre = b.noten("Guitar", "jazz_guitar", b.takte(akk, lambda t, a: schlag((4, 12), 3, a, 0.75) + schlag((15,), 1, a[1:], 0.55)), 0.8, 0.25)
    bass = b.noten("Bass", "finger_bass", b.takte(gang, lambda t, g: lauf((0, 4, 8, 12), 4, g, 0.85, betont=(0,))), 0.9, -0.1)
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Guitar", (gitarre, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return b.fertig()


def insel():
    # C G Am F: die Ukulele schlaegt Achtel, die Piezo-Gitarre zupft die Akkorde, der Bass die Grundtoene
    b = Bau("Lagoon", "World", 104, 0.35)
    uke = [(60, 64, 67, 72), (62, 67, 71), (64, 69, 72), (65, 69, 72)]
    tief = [(48, 52, 55, 60), (43, 47, 50, 55), (45, 48, 52, 57), (41, 45, 48, 53)]
    grund = [36, 31, 33, 29]
    beat = b.drums("Shaker", [("Kick", "kit:kick", 0.6, 0.0, "x.......x......."),
                              ("Clap", "kit:clap", 0.4, 0.0, "....x.......x..."),
                              ("Shaker", "kit:hat_closed", 0.35, 0.3, "o.x.o.x.o.x.o.x.")])
    ukulele = b.noten("Ukulele", "ukulele", b.takte(uke, lambda t, a: schlag((0, 3, 6, 8, 10, 14), 2, a, 0.7)), 0.85, 0.3)
    gitarre = b.noten("Guitar", "piezo_guitar", b.takte(tief, lambda t, a: lauf((0, 2, 4, 6, 8, 10, 12, 14), 2, a, 0.6, betont=(0, 8))), 0.6, -0.3)
    bass = b.noten("Bass", "finger_bass", b.takte(grund, lambda t, g: [(0, 6, g + 12, 0.9), (8, 4, g + 12, 0.75), (12, 4, g + 19, 0.7)]), 0.9)
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Ukulele", (ukulele, 0, 2))
    b.kanal("Guitar", (gitarre, 4, 1))
    b.kanal("Bass", (bass, 0, 2))
    return b.fertig()


def tief_d():
    # Drop D: Powerchords auf D, F, G, Bb — die tiefe Saite leer; der Bass doppelt den Grundton
    b = Bau("Low D", "Rock", 140, 0.3)
    akk = [(38, 45, 50), (41, 48, 53), (43, 50, 55), (46, 53, 58)]
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.9, 0.0, "x.x.....x.x....."),
                            ("Snare", "kit:snare", 0.8, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.x.x.x.x.x.x.x.")])
    riff = b.noten("Riff", "drop_d_guitar", b.takte(akk, lambda t, a: schlag((0, 2, 4, 6, 8, 10), 2, a, 0.9) + schlag((12, 14), 2, a[:2], 0.7)), 0.8, -0.2)
    bass = b.noten("Bass", "finger_bass", b.takte(akk, lambda t, a: lauf((0, 2, 4, 6, 8, 10, 12, 14), 2, (a[0],), 0.85, betont=(0, 8))), 0.9, 0.1)
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Riff", (riff, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return b.fertig()


def pizzicato():
    # G-Dur, zwei Violinen pizzicato: oben eine Melodie in Achteln, unten die Begleitung; der Bass gezupft
    b = Bau("Pizzicato", "Classical", 112, 0.4)
    oben = [(67, 71, 74, 79, 74, 71, 72, 74), (72, 76, 79, 76, 74, 72, 71, 69), (71, 74, 79, 74, 71, 67, 69, 71),
            (69, 72, 74, 72, 71, 69, 67, 67)]
    unten = [(55, 59, 62), (60, 64, 67), (55, 59, 62), (62, 66, 69)]
    grund = [43, 48, 43, 50]
    melodie = b.noten("Violin I", "pizzicato_violin", b.takte(oben, lambda t, m: lauf((0, 2, 4, 6, 8, 10, 12, 14), 2, m, 0.75, betont=(0, 8))), 0.9, 0.3)
    begleitung = b.noten("Violin II", "pizzicato_violin", b.takte(unten, lambda t, a: schlag((0, 4, 8, 12), 2, a, 0.6)), 0.7, -0.3)
    bass = b.noten("Bass", "finger_bass", b.takte(grund, lambda t, g: [(0, 4, g, 0.8), (8, 4, g + 7, 0.7)]), 0.7)
    b.kanal("Violin I", (melodie, 0, 2))
    b.kanal("Violin II", (begleitung, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return b.fertig()


BEISPIELE = {"jazz_guitar_demo": jazz, "ukulele_demo": insel, "drop_d_demo": tief_d, "pizzicato_demo": pizzicato}


def main():
    ordner = HIER / "vergleich"
    ordner.mkdir(exist_ok=True)
    for quelle, name in VERGLEICHE.items():
        (ordner / (name + ".lvloop")).write_text(als_text(vergleich(quelle)), encoding="utf-8", newline="\n")
    for name, bau in BEISPIELE.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(VERGLEICHE), "Vergleiche,", len(BEISPIELE), "Beispiele")


if __name__ == "__main__":
    main()
