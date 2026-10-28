"""Die mitgelieferten Loops des Fluegels (S157, Stufe 31d-8 „Spielbar“, Entscheid Patrik SP1 a;
docs/audio/Spielweisen_Konzept.md §4.1, Teilplan 31d-8, Schritt 8.4).

    python asset/loop/make_fluegel_loops.py
    python asset/loop/instrument_pegel.py --loops

Schreibt `asset/loop/grand_piano_demo.lvloop`: acht Takte, 72 bpm, linke Hand gebrochene
Akkorde (Grundton, Quinte, Terz eine Oktave hoeher, Oktave als Achtel; C, Am, F, G, C, Am, G,
im letzten Takt C gehalten), rechte Hand eine Melodie aus Vierteln und Halben. Keine Spuren,
kein Pedal (Mitschwingen kommt mit 31e). Selbst gesetzt, kein fremdes Werk.

Ein Fluegel spielt in **einem** Muster mit beiden Haenden, in einem Kanal, Pan 0 (HD1 a, S157):
ein Daempfersystem, eine Pedal-Spur; den Raum gibt der Boden. Die Lautstaerke ist fuer den
Pegel des Instruments auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt = neu, DL1 a).

Danach `instrument_pegel.py --loops` (zieht den Loop auf den Pegel der Instrumente nach, wie
jeden anderen) und `instrument_katalog.py` (das Genre des Loops wird das Genre von
`grand_piano`). 31d-9 und 31h-3 bauen dieses Skript fuer die Chopin-Loops aus (CH1 a, CH2 a).
Kein Zufall: zwei Laeufe ergeben dieselbe Datei.

Schreibt dazu `asset/loop/nocturne_demo.lvloop` (31d-9, S158): die Nocturne Op. 9 Nr. 2 ganz, aus den Daten in
`nocturne_op9_2.py` — ein Muster *Piano* mit beiden Haenden (HD1 a), Stimmen *Right hand* / *Left hand* nach System
(N5 a), Pedal-Spur Sustain nach *Ped.* und ✱ (N3 a), Staerke nach der Tafel mit Gabeln (N2 a, DY1 a), festes Tempo
♪ = 132 (N4 a), Auftakt vorn (N7 a), Kadenz und Schlussfermate nach N8 a; Fassung 14, immer alle Felder. Was die
Beschreibung des Loops sagt (Tafel, Tempo-Stellen, Verzierungen, Abweichungen), steht in README.md.
`instrument_pegel.py --loops` laesst Fassung 14 unberuehrt — der Pegel steht hier (`NOCTURNE_GAIN_DB`).
"""

import math
from pathlib import Path

import nocturne_op9_2
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
        # beide Haende in einem Muster (HD1 a); die Stimmen je Note (HD2 a) kommen mit 31d-9 in die Datei
        [muster("Piano", "grand_piano", linke_hand() + rechte_hand(), laut=0.8, pan=0.0, takte=takte)],
        [("Piano", [("Piano", 0, 1)])],
    )


# --- Nocturne Op. 9 Nr. 2 (31d-9, CH1 a; Daten in nocturne_op9_2.py) ---------------------------------------------

NOCTURNE_TAKT = 48  # 12/8: zwoelf Achtel zu vier Schritten (Zweiunddreissigstel)
NOCTURNE_AUFTAKT = 44  # Schritt des Auftakts in Takt 0 — er liegt im Loop bei 0 (N7 a)
# N2 a: die Tafel in dB der Anschlagstufe (Stufe 0 = 5 m/s Hammer, HK2 a). Seit S160 (DY2 a) auf 21 dB gespreizt:
# gemessene Hammergeschwindigkeiten gespielter Toene reichen von 0,37 bis 4,07 m/s (Goebl, Bresin & Galembo 2004,
# ISMA Nara, §2.2 „Stimuli“; OFAI TR-2004-02, S. 2), die Tafel setzt ppp 0,45 m/s ... ff 5 m/s; das Modell spreizt am Ausgang 0,9-1,4 dB je dB Stufe (S160,
# `spreizung_messen.py`), die alte Tafel (ppp -30 ... ff 0, S157) gab 26-41 dB. Ein Akzent 4 dB ueber der Umgebung.
NOCTURNE_STUFEN = {"ppp": -21, "pp": -18, "p": -14, "mp": -11, "mf": -8, "f": -4, "ff": 0}
NOCTURNE_AKZENT_DB = 4
NOCTURNE_GABEL_DB = 4  # eine Gabel ohne Buchstaben an ihrem Ende fuehrt so weit hin (DY1 a)
NOCTURNE_GABELN = {"<": 1, "cresc.": 1, ">": -1, "dim.": -1}
NOCTURNE_VORGABE = "p"  # vor dem ersten Zeichen (die Ausgabe: espress. dolce, ohne Buchstaben)
# Fader des Kanals (9.5, S158, NP1 a): die Spitze der gerechneten Nocturne (die Kadenz im ff) bei −1 dBFS; mit +10 dB
# lag sie bei 2,40 (+7,6 dB) und die leisen Stellen so laut wie grand_piano_demo. S160: mit +1,4 dB lag sie bei 0,866
# (−1,25 dBFS), darum +1,65 dB (LUMIVIZ_LOOP_WAV misst sie: „groesster Betrag“).
NOCTURNE_GAIN_DB = 1.65
RECHTE_HAND, LINKE_HAND = 1, 2  # Stimmen nach System (N5 a)
TONNAMEN = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def tonhoehe(name):
    """„Eb5“, „C#6“, „Bbb5“ → MIDI (C4 = 60)."""
    ton, rest, vorzeichen = name[0], name[1:], 0
    while rest and rest[0] in "b#":
        vorzeichen += -1 if rest[0] == "b" else 1
        rest = rest[1:]
    return 12 * (int(rest) + 1) + TONNAMEN[ton] + vorzeichen


class Nocturne:
    """Rechnet die Partiturtakte in Schritte des Loops um und fuehrt Dynamik und Pedal nach N2 a, N3 a, DY1 a."""

    def __init__(self, takte):
        self.takte = takte
        self.anfang, ort = {}, 0
        for t in sorted(takte):
            self.anfang[t] = ort
            ort += takte[t].get("takte", 1) * NOCTURNE_TAKT
        self.ende = ort - NOCTURNE_AUFTAKT
        self.marken = [(self.stelle(t, s), z) for t in sorted(takte) for s, z in takte[t]["dyn"]]

    def stelle(self, takt, schritt):
        return self.anfang[takt] + schritt - NOCTURNE_AUFTAKT

    def taktende(self, stelle):
        for t in sorted(self.takte):
            ende = self.stelle(t, 0) + self.takte[t].get("takte", 1) * NOCTURNE_TAKT
            if ende > stelle:
                return ende
        return self.ende

    def grundstufe(self, stelle):
        """Stufe aus den Buchstaben bis `stelle`; sfp = f auf dem Schlag samt Vorschlaegen davor, danach p."""
        db = NOCTURNE_STUFEN[NOCTURNE_VORGABE]
        for ort, zeichen in self.marken:
            if ort > stelle:
                break
            if zeichen in NOCTURNE_STUFEN:
                db = NOCTURNE_STUFEN[zeichen]
            elif zeichen == "sfp":
                db = NOCTURNE_STUFEN["f"] if stelle - ort < 2 else NOCTURNE_STUFEN["p"]
        return db

    def stufe(self, stelle):
        """Mit Gabeln (DY1 a): eine Gabel reicht bis zum naechsten Zeichen, hoechstens bis zum Ende ihres Takts;
        steht dort ein Buchstabe, fuehrt sie gleichmaessig zu seiner Stufe, sonst um NOCTURNE_GABEL_DB hin."""
        db = self.grundstufe(stelle)
        for i, (ort, zeichen) in enumerate(self.marken):
            if zeichen not in NOCTURNE_GABELN or ort > stelle:
                continue
            naechste = self.marken[i + 1] if i + 1 < len(self.marken) else None
            ende = min(naechste[0], self.taktende(ort)) if naechste else self.taktende(ort)
            if not ort <= stelle < ende:
                continue
            start = self.grundstufe(ort)
            ziel = start + NOCTURNE_GABELN[zeichen] * NOCTURNE_GABEL_DB
            if naechste and naechste[0] == ende and naechste[1] in NOCTURNE_STUFEN:
                ziel = NOCTURNE_STUFEN[naechste[1]]
            db = start + (ziel - start) * (stelle - ort) / (ende - ort)
        return db

    def noten(self):
        noten = []
        for t in sorted(self.takte):
            for hand, stimme in (("rh", RECHTE_HAND), ("lh", LINKE_HAND)):
                for eintrag in self.takte[t][hand]:
                    anfang, dauer, toene = eintrag[:3]
                    ort = self.stelle(t, anfang)
                    db = min(0.0, self.stufe(ort) + (NOCTURNE_AKZENT_DB if len(eintrag) > 3 else 0))
                    noten += [note_mit_versatz(ort, dauer, tonhoehe(ton), db, stimme) for ton in toene]
        noten.sort(key=lambda n: (n[0], n[8], n[2]))
        return noten

    def pedal(self):
        punkte = []
        for t in sorted(self.takte):
            ped = self.takte[t]["ped"]
            if ped == "schlag":  # nieder an jedem Schlag, hoch nach dem zweiten Achtel (N3 a)
                ped = [(s, 1) for s in (0, 12, 24, 36)] + [(s + 8, 0) for s in (0, 12, 24, 36)]
            punkte += [(self.stelle(t, s), w) for s, w in ped]
        return [[float(ort), float(w)] for ort, w in sorted(p for p in punkte if p[0] >= 0)]


def note_mit_versatz(ort, dauer, hoehe, db, stimme):
    """Eine Zeile der Fassung 14: Bruchteile als Schritt + Versatz des Anfangs, Laenge + Versatz des Endes (N1 a);
    die Staerke als Faktor der Anschlagstufe (N2 a)."""
    schritt = math.floor(ort + 1e-9)
    laenge = max(1, math.ceil(dauer - 1e-9))
    return [schritt, laenge, hoehe, round(10 ** (db / 20), 4), -1, 0, [], stimme,
            round(ort - schritt, 6), round(dauer - laenge, 6)]


def nocturne_demo():
    stueck = Nocturne(nocturne_op9_2.TAKTE)
    noten = stueck.noten()
    takte = math.ceil(max(n[0] + n[1] for n in noten) / NOCTURNE_TAKT)
    muster_ = {
        "name": "Piano", "kind": "instrument", "bars": takte, "instrument": "grand_piano", "volume": 0.8, "pan": 0.0,
        "mute": False, "notes": noten, "voices": ["Right hand", "Left hand"],
        "lanes": {"sustain": stueck.pedal(), "sostenuto": [], "expression": [], "vibrato": [], "pitch": [],
                  "switches": {}},
    }
    return {
        "format": "lumiviz-loop", "version": 14, "bpm": 132, "beatsPerBar": 12, "stepsPerBeat": 4, "swing": 0.0,
        "volume": 0.35,
        "tags": {"title": "Nocturne Op. 9 No. 2", "artist": "Frédéric Chopin", "genre": "Classical"},
        "patterns": [muster_],
        "channels": [{"name": "Piano", "enabled": True, "clips": [{"pattern": "Piano", "start": 0, "repeat": 1}],
                      "gain": NOCTURNE_GAIN_DB}],
    }


LOOPS = {"grand_piano_demo": grand_piano_demo, "nocturne_demo": nocturne_demo}


def main():
    for name, bau in LOOPS.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(LOOPS), "Loop(s) des Fluegels")


if __name__ == "__main__":
    main()
