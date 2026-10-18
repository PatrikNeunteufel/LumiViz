"""Erzeugt Beispiel-Loops fuer viele Genres und die Instrumente, die dafuer
fehlten (S111, Wunsch Patrik: Loops fuer Trance, Funk, Reggaeton, Reggae und
viele mehr; fehlende Instrumente dazu).

    python asset/loop/make_genres.py

Schreibt nach `asset/loop/instruments/*.lvinst` (siebzehn Instrumente; Cowbell
und Clave kommen seit S128 aus Samples, `make_sample_instruments.py`) und
`asset/loop/genres/*.lvloop` (neunzehn Loops). Kein Zufall: zwei Laeufe ergeben
dieselben Dateien. Massgebend sind die Dateien im Repo, nicht ein neuer Lauf.
**Seit S133 (Stufe 23g):** Clavinet, Slap Bass, Clean, Muted und Nylon Guitar und
Upright Bass schreibt dieses Skript als Classic — danach
`make_saiten_instrumente.py` laufen lassen, das sie auf die Saiten umstellt.

Alles ist nach Rechnung gesetzt, nicht nach Gehoer: Rhythmus, Tonart und
Besetzung folgen dem, was das Genre ueblicherweise ausmacht — ob es so klingt,
muss ein Mensch hoeren.
"""

from make_examples import HIER, als_text, instrument, schicht

# --- Instrumente -------------------------------------------------------------

INSTRUMENTE = {
    # Trance
    "trance_lead": instrument(
        "Trance Lead", 0.5,
        schicht("Main", "saw", voices=3, detune=22, attack=4, decay=250, sustain=0.75, release=320, filterCutoff=10),
        schicht("Octave", "saw", transpose=12, voices=3, detune=16, attack=4, decay=250, sustain=0.75, release=320,
                filterCutoff=8, level=0.5),
        schicht("Bite", "square", drive=0.3, attack=2, decay=120, sustain=0.3, release=200, filterCutoff=6, level=0.3)),
    "trance_pluck": instrument(
        "Trance Pluck", 0.75,
        schicht("Pluck", "saw", voices=2, detune=10, attack=1, decay=160, sustain=0, release=260, filterCutoff=1.5,
                filterEnvelope=10),
        schicht("Shine", "saw", transpose=12, voices=2, detune=14, attack=1, decay=110, sustain=0, release=220,
                filterCutoff=2, filterEnvelope=8, level=0.45),
        schicht("Body", "sine", attack=1, decay=200, sustain=0, release=200, level=0.4)),
    "rolling_bass": instrument(
        "Rolling Bass", 0.7,
        schicht("Roll", "saw", drive=0.3, attack=1, decay=90, sustain=0.15, release=40, filterCutoff=2, filterEnvelope=6),
        schicht("Sub", "sine", attack=1, decay=110, sustain=0.3, release=40, level=0.7)),
    "hoover": instrument(
        "Hoover", 0.45,
        schicht("Howl", "saw", drive=0.4, voices=3, detune=45, attack=40, decay=300, sustain=0.8, release=250,
                filterCutoff=7, vibratoRate=5.5, vibratoDepth=20, vibratoDelay=100),
        schicht("High", "saw", transpose=12, voices=3, detune=30, attack=40, decay=300, sustain=0.8, release=250,
                filterCutoff=5, level=0.5),
        schicht("Edge", "square", attack=40, decay=300, sustain=0.7, release=250, filterCutoff=4, level=0.3)),
    # Funk, Disco, Reggae
    "clavinet": instrument(
        "Clavinet", 0.6,
        schicht("String", "string", damping=0.99, attack=1, release=50, drive=0.35),
        schicht("Bark", "square", attack=1, decay=90, sustain=0, release=50, filterCutoff=3, filterEnvelope=8, drive=0.3,
                level=0.5)),
    "slap_bass": instrument(
        "Slap Bass", 0.75,
        schicht("String", "string", damping=0.994, attack=1, release=60, drive=0.2),
        schicht("Thump", "sine", harmonics=[0.3], attack=1, decay=160, sustain=0.2, release=60, level=0.8),
        schicht("Pop", "square", transpose=12, attack=1, decay=35, sustain=0, release=30, filterCutoff=4, level=0.25)),
    "clean_guitar": instrument(
        "Clean Guitar", 0.7,
        schicht("String", "string", damping=0.995, attack=2, release=90),
        schicht("Shimmer", "string", transpose=12, startOffset=10, damping=0.993, attack=2, release=90, level=0.3)),
    "muted_guitar": instrument(
        "Muted Guitar", 0.8,
        schicht("Mute", "string", damping=0.965, attack=1, release=30),
        schicht("Thud", "triangle", attack=1, decay=50, sustain=0, release=30, level=0.4)),
    "round_bass": instrument(
        "Round Bass", 0.85,
        schicht("Round", "sine", harmonics=[0.25, 0.08], attack=4, decay=400, sustain=0.5, release=90),
        schicht("Finger", "triangle", attack=2, decay=70, sustain=0, release=40, filterCutoff=3, level=0.3)),
    # Hip-Hop, Trap, Drum and Bass
    "bass_808": instrument(
        "808 Bass", 0.8,
        schicht("Boom", "sine", drive=0.45, attack=2, decay=900, sustain=0.3, release=180),
        schicht("Click", "triangle", transpose=12, attack=1, decay=30, sustain=0, release=20, level=0.3)),
    "reese_bass": instrument(
        "Reese Bass", 0.6,
        schicht("Reese", "saw", voices=3, detune=40, attack=10, decay=300, sustain=0.9, release=120, filterCutoff=2.5),
        schicht("Sub", "sine", attack=10, decay=300, sustain=0.9, release=120, level=0.6)),
    # Jazz, Bossa, Blues, Lo-Fi
    "upright_bass": instrument(
        "Upright Bass", 0.85,
        schicht("String", "string", damping=0.995, attack=3, release=80),
        schicht("Wood", "sine", harmonics=[0.2], attack=5, decay=350, sustain=0.3, release=80, level=0.7)),
    "nylon_guitar": instrument(
        "Nylon Guitar", 0.75,
        schicht("String", "string", damping=0.994, attack=3, release=110),
        schicht("Warmth", "triangle", attack=3, decay=300, sustain=0.1, release=110, filterCutoff=3, level=0.4)),
    "sax": instrument(
        "Sax", 0.65,
        schicht("Reed", "square", drive=0.25, attack=35, decay=200, sustain=0.85, release=110, filterCutoff=3,
                filterEnvelope=3, vibratoRate=5.5, vibratoDepth=14, vibratoDelay=250),
        schicht("Breath", "saw", attack=50, decay=200, sustain=0.7, release=110, filterCutoff=5, level=0.35,
                vibratoRate=5.5, vibratoDepth=14, vibratoDelay=250)),
    "vibraphone": instrument(
        "Vibraphone", 0.8,
        schicht("Bar", "sine", harmonics=[0, 0, 0.18], attack=1, decay=1400, sustain=0.05, release=600,
                vibratoRate=4.5, vibratoDepth=5, vibratoDelay=0),
        schicht("Strike", "sine", transpose=24, attack=1, decay=60, sustain=0, release=40, level=0.2)),
    "steel_drum": instrument(
        "Steel Drum", 0.7,
        schicht("Pan", "sine", harmonics=[0.5, 0, 0.3], attack=1, decay=420, sustain=0.05, release=260),
        schicht("Ring", "sine", transpose=12, voices=2, detune=18, attack=1, decay=260, sustain=0, release=200, level=0.4),
        schicht("Hit", "triangle", transpose=24, attack=1, decay=30, sustain=0, release=20, level=0.2)),
    # Schlagwerk als Instrument: die Tonhoehe der Note ist die Tonhoehe des Schlags
    # (Cowbell und Clave waren bis S128 hier gerechnet — Entscheid Patrik C2 a)
    "conga": instrument(
        "Conga", 0.85,
        schicht("Skin", "sine", harmonics=[0.15], attack=1, decay=150, sustain=0, release=60),
        schicht("Slap", "string", transpose=12, damping=0.94, attack=1, release=30, level=0.35)),
}

# --- Bausteine ---------------------------------------------------------------

_STAERKE = {"x": 1.0, "o": 0.7, "-": 0.4}


class Bau:
    """Ein Loop im Bau: `schritte` je Takt (4 Schlaege mal 4, oder 3 mal 4, oder 4 mal 3)."""

    def __init__(self, titel, genre, bpm, laut, schlaege=4, je_schlag=4, swing=0.0):
        self.kopf = (titel, genre, bpm, laut, schlaege, je_schlag, swing)
        self.schritte = schlaege * je_schlag
        self.patterns = []
        self.kanaele = []

    def drums(self, name, spuren):
        """spuren: (Name, Klang, Lautstaerke, Panorama, Takte) — Takte ist eine Zeichenkette fuer jeden der vier
        Takte oder eine Liste von vier; x laut, o mittel, - leise, sonst nichts."""
        tracks, rows = [], []
        for titel, klang, laut, pan, takte in spuren:
            if isinstance(takte, str):
                takte = [takte] * 4
            assert len(takte) == 4 and all(len(t) == self.schritte for t in takte), (name, titel)
            tracks.append({"name": titel, "sound": klang, "volume": laut, "pan": pan, "mute": False})
            rows.append([_STAERKE.get(z, 0) for t in takte for z in t])
        self.patterns.append({"name": name, "kind": "drumkit", "bars": 4, "tracks": tracks, "rows": rows})
        return name

    def noten(self, name, inst, noten, laut=0.8, pan=0.0):
        for schritt, laenge, hoehe, staerke in noten:
            assert 0 <= schritt and laenge >= 1 and schritt + laenge <= self.schritte * 4, (name, schritt, laenge)
            assert 24 <= hoehe <= 96 and 0 < staerke <= 1, (name, hoehe, staerke)
        self.patterns.append({"name": name, "kind": "instrument", "bars": 4, "instrument": inst, "volume": laut,
                              "pan": pan, "mute": False, "notes": [list(n) for n in sorted(set(noten))]})
        return name

    def takte(self, folge, tu):
        """tu(takt, eintrag) -> Noten mit Schritten im Takt; `folge` hat vier Eintraege."""
        assert len(folge) == 4
        return [(t * self.schritte + s, l, h, v) for t, e in enumerate(folge) for s, l, h, v in tu(t, e)]

    def kanal(self, name, *clips):
        """clips: (Muster, Start-Takt, Wiederholungen)"""
        self.kanaele.append((name, clips))

    def fertig(self):
        titel, genre, bpm, laut, schlaege, je_schlag, swing = self.kopf
        return {
            "format": "lumiviz-loop", "version": 2, "bpm": bpm, "beatsPerBar": schlaege, "stepsPerBeat": je_schlag,
            "swing": swing, "volume": laut, "tags": {"title": titel, "artist": "LumiViz", "genre": genre},
            "patterns": self.patterns,
            "channels": [{"name": name, "enabled": True,
                          "clips": [{"pattern": m, "start": start, "repeat": mal} for m, start, mal in clips]}
                         for name, clips in self.kanaele],
        }


def schlag(schritte, laenge, hoehen, staerke=0.8):
    """Dieselben Tonhoehen an mehreren Schritten."""
    if isinstance(hoehen, int):
        hoehen = (hoehen,)
    return [(s, laenge, h, staerke) for s in schritte for h in hoehen]


def lauf(schritte, laenge, hoehen, staerke=0.8, betont=()):
    """Je Schritt eine Tonhoehe aus der Folge (wiederholt sich)."""
    return [(s, laenge, hoehen[i % len(hoehen)], 1.0 if s in betont else staerke) for i, s in enumerate(schritte)]


# --- Trance ------------------------------------------------------------------

def trance_uplifting():
    b = Bau("Sunrise", "Trance", 138, 0.24)
    akk = [(57, 60, 64), (53, 57, 60), (48, 52, 55), (55, 59, 62)]  # Am F C G
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Clap", "kit:clap", 0.6, 0.0, "....x.......x..."),
                            ("Open Hat", "kit:hat_open", 0.4, 0.2, "..x...x...x...x."),
                            ("Hi-Hat", "kit:hat_closed", 0.35, -0.2, "-o--.o---o--.o--")])
    roll = b.drums("Roll", [("Snare", "kit:snare", 0.7, 0.0, ["-.......-.......", "-...-...-...-...", "-.-.-.o.o.o.o.o.", "ooooooooxxxxxxxx"]),
                            ("Open Hat", "kit:hat_open", 0.4, 0.2, ["................"] * 3 + ["............x..."])])
    bass = b.noten("Bass", "rolling_bass", b.takte(akk, lambda t, a: schlag((2, 6, 10, 14), 2, a[0] - 24, 0.9)), 0.9)
    pluck = b.noten("Pluck", "trance_pluck", b.takte(akk, lambda t, a: lauf(range(16), 1, (a[0] + 12, a[1] + 12, a[2] + 12, a[0] + 24, a[2] + 12, a[1] + 12, a[0] + 24, a[1] + 24), 0.7, (0, 8))), 0.7, 0.3)
    gate = b.noten("Gate", "super_saw", b.takte(akk, lambda t, a: schlag((0, 3, 6, 8, 11, 14), 2, a, 0.7)), 0.6, -0.2)
    pad = b.noten("Pad", "warm_pad", b.takte(akk, lambda t, a: [(0, 16, a[0], 0.7), (0, 16, a[2], 0.6)]), 0.6)
    lead = b.noten("Lead", "trance_lead", [(0, 6, 76, 0.9), (6, 2, 74, 0.7), (8, 8, 72, 0.9), (16, 6, 72, 0.9), (22, 2, 74, 0.7), (24, 8, 77, 0.9),
                                           (32, 4, 79, 0.9), (36, 4, 76, 0.8), (40, 8, 72, 0.9), (48, 6, 74, 0.9), (54, 2, 76, 0.7), (56, 8, 79, 0.9)], 0.7, 0.1)
    b.kanal("Drums", (beat, 0, 4), (roll, 20, 1), (beat, 24, 2))
    b.kanal("Bass", (bass, 0, 4), (bass, 24, 2))
    b.kanal("Pluck", (pluck, 4, 7))
    b.kanal("Chords", (gate, 8, 2), (gate, 24, 2))
    b.kanal("Pad", (pad, 16, 4))
    b.kanal("Lead", (lead, 16, 4))
    return b.fertig()


def trance_acid():
    b = Bau("Acid line", "Acid Trance", 140, 0.24)
    grund = [45, 45, 43, 41]  # A A G F
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Clap", "kit:clap", 0.6, 0.0, "....x.......x..."),
                            ("Open Hat", "kit:hat_open", 0.4, 0.2, "..x...x...x...x."),
                            ("Hi-Hat", "kit:hat_closed", 0.4, -0.2, ["x-o-x-o-x-o-x-o-"] * 3 + ["x-o-x-o-x-o-xoxo"])])
    roll = b.noten("Roll", "rolling_bass", b.takte(grund, lambda t, g: schlag((1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15), 1, g - 12, 0.8)), 0.9)
    linie = (0, 0, 12, 0, 3, 0, 12, 7, 0, 0, 10, 0, 12, 0, 7, 3)
    acid = b.noten("Acid", "acid_bass", b.takte(grund, lambda t, g: [(s, 1, g + linie[s], 1.0 if s in (2, 6, 12) else 0.7) for s in range(16)]), 0.8, 0.15)
    hoover = b.noten("Hoover", "hoover", b.takte(grund, lambda t, g: [(0, 6, g + 12, 0.8), (0, 6, g + 19, 0.7)] if t % 2 else [(10, 4, g + 24, 0.7)]), 0.6, -0.2)
    sirene = b.noten("Riser", "siren", [(32, 30, 69, 0.7)], 0.4, 0.3)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (roll, 0, 4))
    b.kanal("Acid", (acid, 4, 3))
    b.kanal("Hoover", (hoover, 8, 2))
    b.kanal("FX", (sirene, 12, 1))
    return b.fertig()


def trance_psy():
    b = Bau("Full on", "Psytrance", 145, 0.24)
    grund = [40, 40, 40, 41]  # E E E F — phrygisch
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Snare", "kit:snare", 0.4, 0.0, "....o.......o..."),
                            ("Open Hat", "kit:hat_open", 0.4, 0.2, "..x...x...x...x."), ("Hi-Hat", "kit:hat_closed", 0.4, -0.2, "-o-o-o-o-o-o-o-o")])
    bass = b.noten("Bass", "rolling_bass", b.takte(grund, lambda t, g: schlag((1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15), 1, g - 12, 0.9)), 1.0)
    leiter = (0, 1, 3, 5, 7, 8, 10, 12)
    folge = (0, 2, 1, 3, 2, 4, 3, 5, 7, 5, 4, 2, 3, 1, 2, 0)
    lead = b.noten("Lead", "acid_bass", b.takte(grund, lambda t, g: [(s, 1, 52 + leiter[folge[(s + 3 * t) % 16]], 0.9 if s % 4 == 0 else 0.65) for s in range(16)]), 0.7, 0.2)
    radio = b.noten("Radio", "broken_radio", b.takte(grund, lambda t, g: schlag((2, 10), 1, g + 12, 0.8) + schlag((7, 15), 1, g + 19, 0.6)), 0.5, -0.35)
    chor = b.noten("Choir", "alien_choir", [(0, 30, 64, 0.7), (0, 30, 71, 0.5), (32, 30, 65, 0.7), (32, 30, 72, 0.5)], 0.6)
    kaskade = b.noten("Cascade", "cascade", [(8, 8, 76, 0.8), (40, 8, 77, 0.8)], 0.6, 0.35)
    sirene = b.noten("Riser", "siren", [(32, 30, 76, 0.7)], 0.35, 0.3)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Lead", (lead, 4, 3))
    b.kanal("Radio", (radio, 8, 2))
    b.kanal("Choir", (chor, 8, 2))
    b.kanal("Cascade", (kaskade, 4, 3))
    b.kanal("FX", (sirene, 12, 1))
    return b.fertig()


# --- Funk, Disco -------------------------------------------------------------

def funk():
    b = Bau("Pocket", "Funk", 104, 0.26, swing=0.15)
    grund = [40, 40, 45, 45]  # Em7 Em7 A7 A7
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x.....x...x....."), ("Snare", "kit:snare", 0.9, 0.0, "....x..-.-..x..-"),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x-o-x-o-x-o-x-o-"),
                            ("Open Hat", "kit:hat_open", 0.4, 0.25, ["................"] * 3 + ["..............x."])])
    bass = b.noten("Bass", "slap_bass", b.takte(grund, lambda t, g: [(0, 2, g, 1.0), (3, 1, g, 0.7), (6, 1, g + 12, 0.9), (8, 2, g, 0.9), (11, 1, g + 3, 0.7),
                                                                     (12, 1, g + 5, 0.8), (14, 1, g + 7, 0.8), (15, 1, g + 10, 0.7)]), 0.9)
    clav = b.noten("Clav", "clavinet", b.takte(grund, lambda t, g: schlag((2, 3, 7, 10, 11, 15), 1, (g + 27, g + 31) if g == 40 else (g + 28, g + 34), 0.8)), 0.7, -0.3)
    gitarre = b.noten("Scratch", "muted_guitar", b.takte(grund, lambda t, g: lauf((0, 1, 4, 5, 6, 9, 12, 13, 14), 1, (g + 36, g + 36, g + 39, g + 36, g + 34), 0.7, (4, 12))), 0.7, 0.35)
    horns = b.noten("Horns", "brass", [(40, 2, 64, 0.9), (40, 2, 67, 0.8), (40, 2, 71, 0.8), (43, 2, 64, 0.8), (43, 2, 67, 0.7), (43, 2, 71, 0.7), (46, 2, 66, 0.9), (46, 2, 69, 0.8), (46, 2, 74, 0.8),
                                       (56, 1, 69, 0.9), (56, 1, 73, 0.8), (58, 1, 69, 0.8), (58, 1, 73, 0.7), (60, 4, 71, 1.0), (60, 4, 76, 0.9)], 0.7, 0.1)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (gitarre, 0, 4))
    b.kanal("Clav", (clav, 4, 3))
    b.kanal("Horns", (horns, 8, 2))
    return b.fertig()


def disco():
    b = Bau("Mirror ball", "Disco", 120, 0.24)
    akk = [(57, 60, 64), (50, 53, 57), (55, 59, 62), (48, 52, 55)]  # Am Dm G C
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Clap", "kit:clap", 0.7, 0.0, "....x.......x..."),
                            ("Open Hat", "kit:hat_open", 0.45, 0.2, "..x...x...x...x."), ("Hi-Hat", "kit:hat_closed", 0.4, -0.2, "x-.-x-.-x-.-x-.-"),
                            ("Tom", "kit:tom", 0.5, -0.3, ["................"] * 3 + ["..........x.xx.."])])
    bass = b.noten("Bass", "bass", b.takte(akk, lambda t, a: lauf(range(0, 16, 2), 2, (a[0] - 24, a[0] - 12), 0.85)), 0.9)
    strings = b.noten("Strings", "strings", b.takte(akk, lambda t, a: [(0, 16, a[2] + 12, 0.7), (0, 16, a[1] + 12, 0.6)]), 0.6, -0.25)
    gitarre = b.noten("Guitar", "clean_guitar", b.takte(akk, lambda t, a: schlag((2, 3, 6, 10, 11, 14), 1, (a[1] + 12, a[2] + 12), 0.7)), 0.6, 0.35)
    keys = b.noten("Keys", "tine_piano", b.takte(akk, lambda t, a: schlag((0, 6, 12), 3, a, 0.7)), 0.6, 0.1)
    melodie = b.noten("Melody", "brass", [(0, 3, 76, 0.9), (4, 2, 72, 0.8), (6, 6, 69, 0.9), (16, 3, 77, 0.9), (20, 2, 74, 0.8), (22, 6, 69, 0.9),
                                          (32, 3, 79, 0.9), (36, 2, 74, 0.8), (38, 6, 71, 0.9), (48, 4, 76, 0.9), (52, 4, 79, 0.9), (56, 8, 84, 0.9)], 0.6)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (gitarre, 4, 3))
    b.kanal("Keys", (keys, 0, 4))
    b.kanal("Strings", (strings, 8, 2))
    b.kanal("Melody", (melodie, 8, 2))
    return b.fertig()


# --- Reggae, Ska, Reggaeton --------------------------------------------------

def reggae():
    b = Bau("One drop", "Reggae", 76, 0.3, swing=0.3)
    akk = [(57, 60, 64), (57, 60, 64), (50, 53, 57), (50, 53, 57)]  # Am Am Dm Dm
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "........x......."), ("Rim", "kit:snare", 0.5, 0.1, "........x......."),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.o.x.o.x.o.x.o."),
                            ("Tom", "kit:tom", 0.5, -0.3, ["................"] * 3 + ["............o.x."])])
    bass = b.noten("Bass", "round_bass", b.takte(akk, lambda t, a: [(0, 3, a[0] - 12, 1.0), (4, 2, a[0] - 12, 0.8), (6, 2, a[1] - 12, 0.8), (8, 3, a[2] - 12, 0.9),
                                                                    (12, 2, a[1] - 12, 0.8), (14, 2, a[0] - 12, 0.8)]), 1.0)
    skank = b.noten("Skank", "clean_guitar", b.takte(akk, lambda t, a: schlag((4, 12), 2, (a[0] + 12, a[1] + 12, a[2] + 12), 0.8)), 0.7, 0.35)
    bubble = b.noten("Bubble", "organ", b.takte(akk, lambda t, a: schlag((2, 6, 10, 14), 1, (a[1], a[2]), 0.6)), 0.5, -0.3)
    horns = b.noten("Horns", "brass", [(0, 6, 69, 0.8), (6, 2, 72, 0.7), (8, 8, 76, 0.8), (16, 4, 74, 0.8), (20, 4, 72, 0.7), (24, 8, 69, 0.8),
                                       (32, 6, 74, 0.8), (38, 2, 77, 0.7), (40, 8, 81, 0.8), (48, 4, 77, 0.8), (52, 4, 76, 0.7), (56, 8, 74, 0.8)], 0.5, 0.1)
    sirene = b.noten("Dub siren", "siren", [(48, 14, 81, 0.6)], 0.35, -0.4)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (skank, 0, 4))
    b.kanal("Organ", (bubble, 4, 3))
    b.kanal("Horns", (horns, 8, 2))
    b.kanal("FX", (sirene, 12, 1))
    return b.fertig()


def ska():
    b = Bau("Rude step", "Ska", 152, 0.26)
    akk = [(60, 64, 67), (57, 60, 64), (53, 57, 60), (55, 59, 62)]  # C Am F G
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x.......x......."), ("Snare", "kit:snare", 0.9, 0.0, ["....x.......x..."] * 3 + ["....x.......x.xx"]),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.x.x.x.x.x.x.x.")])
    bass = b.noten("Walking", "upright_bass", b.takte(akk, lambda t, a: [(0, 4, a[0] - 24, 0.9), (4, 4, a[1] - 24, 0.8), (8, 4, a[2] - 24, 0.8), (12, 4, a[1] - 24 + 1, 0.8)]), 1.0)
    skank = b.noten("Skank", "clean_guitar", b.takte(akk, lambda t, a: schlag((2, 6, 10, 14), 1, (a[0] + 12, a[1] + 12, a[2] + 12), 0.8)), 0.7, 0.35)
    piano = b.noten("Piano", "piano", b.takte(akk, lambda t, a: schlag((2, 6, 10, 14), 1, a, 0.6)), 0.5, -0.3)
    horns = b.noten("Horns", "brass", [(0, 2, 72, 0.9), (2, 2, 76, 0.8), (4, 4, 79, 0.9), (10, 2, 76, 0.8), (12, 4, 72, 0.9), (16, 2, 69, 0.9), (18, 2, 72, 0.8), (20, 4, 76, 0.9), (28, 4, 72, 0.8),
                                       (32, 2, 77, 0.9), (34, 2, 76, 0.8), (36, 4, 72, 0.9), (42, 2, 69, 0.8), (44, 4, 72, 0.9), (48, 2, 74, 0.9), (50, 2, 79, 0.8), (52, 4, 83, 0.9), (58, 6, 79, 0.9)], 0.6, 0.1)
    sax = b.noten("Sax", "sax", [(0, 8, 64, 0.7), (8, 8, 67, 0.7), (16, 8, 64, 0.7), (24, 8, 60, 0.7), (32, 8, 65, 0.7), (40, 8, 69, 0.7), (48, 8, 67, 0.7), (56, 8, 71, 0.7)], 0.5, -0.1)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (skank, 0, 4))
    b.kanal("Piano", (piano, 4, 3))
    b.kanal("Horns", (horns, 8, 2))
    b.kanal("Sax", (sax, 8, 2))
    return b.fertig()


def reggaeton():
    b = Bau("Dembow", "Reggaeton", 94, 0.28)
    akk = [(57, 60, 64), (53, 57, 60), (48, 52, 55), (55, 59, 62)]  # Am F C G
    beat = b.drums("Dembow", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Snare", "kit:snare", 0.8, 0.0, "...x..x....x..x."),
                              ("Hi-Hat", "kit:hat_closed", 0.4, 0.25, "x.x.x.x.x.x.x.x."),
                              ("Clap", "kit:clap", 0.5, -0.1, ["................"] * 3 + ["..............x."])])
    bass = b.noten("Bass", "bass_808", b.takte(akk, lambda t, a: [(0, 3, a[0] - 24, 1.0), (6, 2, a[0] - 24, 0.8), (8, 4, a[0] - 24, 0.9), (14, 2, a[2] - 24, 0.8)]), 0.9)
    gitarre = b.noten("Guitar", "nylon_guitar", b.takte(akk, lambda t, a: lauf((0, 3, 6, 8, 11, 14), 2, (a[0], a[2], a[1] + 12, a[0] + 12, a[2], a[1] + 12), 0.75)), 0.7, 0.3)
    pan = b.noten("Steel", "steel_drum", [(0, 3, 76, 0.9), (3, 3, 72, 0.8), (6, 2, 69, 0.8), (8, 6, 72, 0.9), (16, 3, 77, 0.9), (19, 3, 72, 0.8), (22, 2, 69, 0.8), (24, 6, 72, 0.9),
                                          (32, 3, 79, 0.9), (35, 3, 76, 0.8), (38, 2, 72, 0.8), (40, 6, 76, 0.9), (48, 3, 74, 0.9), (51, 3, 71, 0.8), (54, 2, 67, 0.8), (56, 8, 74, 0.9)], 0.7, -0.2)
    pad = b.noten("Pad", "warm_pad", b.takte(akk, lambda t, a: [(0, 16, a[0], 0.6), (0, 16, a[2], 0.5)]), 0.5)
    perc = b.noten("Perc", "cowbell", b.takte(akk, lambda t, a: schlag((2, 10), 1, 79, 0.6) + schlag((7, 15), 1, 84, 0.5)), 0.4, 0.4)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (gitarre, 4, 3))
    b.kanal("Pad", (pad, 4, 3))
    b.kanal("Lead", (pan, 8, 2))
    b.kanal("Perc", (perc, 8, 2))
    return b.fertig()


# --- Hip-Hop, Trap, Drum and Bass, Techno ------------------------------------

def hiphop():
    b = Bau("Boom bap", "Hip-Hop", 90, 0.3, swing=0.35)
    akk = [(62, 65, 69, 72), (55, 59, 62, 65), (60, 64, 67, 71), (57, 61, 64, 67)]  # Dm7 G7 Cmaj7 A7
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, ["x......x..x.....", "x......x..x..x.."] * 2), ("Snare", "kit:snare", 0.9, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.2, "x.o.x.o.x.o.x.oo")])
    bass = b.noten("Bass", "upright_bass", b.takte(akk, lambda t, a: [(0, 4, a[0] - 24, 1.0), (7, 1, a[0] - 24, 0.7), (10, 3, a[2] - 24, 0.8), (14, 2, a[1] - 24, 0.7)]), 1.0)
    keys = b.noten("Keys", "tine_piano", b.takte(akk, lambda t, a: schlag((0,), 6, a, 0.7) + schlag((10,), 4, a[1:], 0.6)), 0.7, -0.2)
    vibes = b.noten("Vibes", "vibraphone", [(4, 4, 81, 0.8), (10, 6, 77, 0.7), (22, 2, 74, 0.7), (24, 8, 79, 0.8), (36, 4, 83, 0.8), (42, 6, 79, 0.7), (52, 4, 76, 0.8), (58, 6, 73, 0.7)], 0.7, 0.25)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Keys", (keys, 0, 4))
    b.kanal("Vibes", (vibes, 8, 2))
    return b.fertig()


def trap():
    b = Bau("Half time", "Trap", 140, 0.28)
    grund = [41, 41, 37, 39]  # F F Db Eb
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, ["x.....x...x.....", "x.....x......x.."] * 2), ("Clap", "kit:clap", 0.8, 0.0, "........x......."),
                            ("Snare", "kit:snare", 0.6, 0.0, ["........x......."] * 3 + ["........x....xxx"]),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, ["x.x.x.x.x.x.x.x.", "x.x.xxx.x.x.x-o-", "x.x.x.x.x.x.x.x.", "x.xxx.x.xoxoxoxo"]),
                            ("Open Hat", "kit:hat_open", 0.35, 0.25, ["..............x.", "................"] * 2)])
    bass = b.noten("808", "bass_808", b.takte(grund, lambda t, g: [(0, 6, g - 12, 1.0), (6, 4, g - 12, 0.9), (10, 6, g - 12 if t % 2 == 0 else g - 5, 0.9)]), 1.0)
    glocke = b.noten("Bells", "bell", b.takte(grund, lambda t, g: lauf((0, 3, 6, 8, 11, 14), 2, (g + 36, g + 39, g + 43, g + 36, g + 44, g + 43), 0.7)), 0.5, 0.25)
    kalimba = b.noten("Keys", "kalimba", b.takte(grund, lambda t, g: lauf((0, 2, 4, 6, 8, 10, 12, 14), 2, (g + 24, g + 31, g + 27, g + 31), 0.6)), 0.5, -0.3)
    geist = b.noten("Pad", "ghost", [(0, 30, 65, 0.7), (32, 30, 61, 0.7)], 0.6)
    b.kanal("Drums", (beat, 4, 3))
    b.kanal("808", (bass, 4, 3))
    b.kanal("Bells", (glocke, 0, 4))
    b.kanal("Keys", (kalimba, 8, 2))
    b.kanal("Pad", (geist, 0, 4))
    return b.fertig()


def dnb():
    b = Bau("Roller", "Drum and Bass", 174, 0.24)
    akk = [(57, 60, 64), (57, 60, 64), (53, 57, 60), (52, 55, 59)]  # Am Am F Em
    beat = b.drums("Break", [("Kick", "kit:kick", 1.0, 0.0, ["x.........x.....", "x.........x..x.."] * 2), ("Snare", "kit:snare", 0.9, 0.0, ["....x.......x...", "....x..-.-..x..."] * 2),
                             ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.x.x.x.x.x.x.x."),
                             ("Open Hat", "kit:hat_open", 0.35, 0.25, ["................"] * 3 + ["..............x."])])
    bass = b.noten("Reese", "reese_bass", b.takte(akk, lambda t, a: [(0, 6, a[0] - 24, 1.0), (6, 4, a[0] - 24, 0.8), (10, 3, a[0] - 21, 0.9), (13, 3, a[0] - 26, 0.9)]), 0.9)
    pad = b.noten("Pad", "warm_pad", b.takte(akk, lambda t, a: [(0, 16, a[1], 0.6), (0, 16, a[2], 0.6)]), 0.6, -0.2)
    keys = b.noten("Keys", "tine_piano", b.takte(akk, lambda t, a: schlag((3, 10), 3, (a[0] + 12, a[1] + 12, a[2] + 12), 0.7)), 0.6, 0.3)
    glocke = b.noten("Lead", "vibraphone", [(0, 8, 76, 0.8), (8, 8, 72, 0.7), (16, 12, 69, 0.8), (32, 8, 77, 0.8), (40, 8, 72, 0.7), (48, 8, 71, 0.8), (56, 8, 76, 0.8)], 0.6, 0.15)
    b.kanal("Drums", (beat, 4, 3))
    b.kanal("Bass", (bass, 4, 3))
    b.kanal("Pad", (pad, 0, 4))
    b.kanal("Keys", (keys, 0, 4))
    b.kanal("Lead", (glocke, 8, 2))
    return b.fertig()


def techno():
    b = Bau("Warehouse", "Techno", 130, 0.24)
    grund = [38, 38, 38, 36]  # D D D C
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Clap", "kit:clap", 0.5, 0.0, "....o.......o..."),
                            ("Open Hat", "kit:hat_open", 0.45, 0.2, "..x...x...x...x."), ("Hi-Hat", "kit:hat_closed", 0.4, -0.2, "-o-o-o-o-o-o-o-o"),
                            ("Tom", "kit:tom", 0.5, -0.35, "...o..o....o..o.")])
    bass = b.noten("Bass", "rolling_bass", b.takte(grund, lambda t, g: lauf((1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15), 1, (g - 12, g - 12, g), 0.9)), 1.0)
    stab = b.noten("Stab", "chord_stab", b.takte(grund, lambda t, g: schlag((2, 5, 10, 13), 1, (g + 24, g + 27, g + 31), 0.8)), 0.7, 0.25)
    radio = b.noten("Noise", "broken_radio", b.takte(grund, lambda t, g: schlag((6, 14), 1, g + 36, 0.6)), 0.4, -0.35)
    hoover = b.noten("Hoover", "hoover", [(48, 14, 62, 0.8), (48, 14, 69, 0.6)], 0.5)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Stab", (stab, 4, 3))
    b.kanal("Noise", (radio, 8, 2))
    b.kanal("Hoover", (hoover, 4, 1), (hoover, 12, 1))
    return b.fertig()


# --- Bossa, Blues, Walzer, Lo-Fi ---------------------------------------------

def bossa():
    b = Bau("Praia", "Bossa Nova", 132, 0.3)
    akk = [(57, 60, 64, 67), (50, 54, 57, 60), (55, 59, 62, 66), (52, 56, 59, 62)]  # Am7 D7 Gmaj7 E7
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.7, 0.0, "x.....x.x.....x."), ("Hi-Hat", "kit:hat_closed", 0.4, 0.25, "o.o.o.o.o.o.o.o.")])
    clave = b.noten("Clave", "clave", b.takte(akk, lambda t, a: schlag((0, 6, 12), 1, 84, 0.8) if t % 2 == 0 else schlag((4, 10), 1, 84, 0.8)), 0.6, -0.3)
    bass = b.noten("Bass", "upright_bass", b.takte(akk, lambda t, a: [(0, 6, a[0] - 24, 0.9), (6, 2, a[2] - 24, 0.7), (8, 6, a[2] - 24, 0.8), (14, 2, a[0] - 24, 0.7)]), 1.0)
    gitarre = b.noten("Guitar", "nylon_guitar", b.takte(akk, lambda t, a: schlag((0, 4, 10) if t % 2 == 0 else (2, 6, 12), 3, a[1:], 0.7) + [(0, 4, a[0] - 12, 0.7), (8, 4, a[0] - 12, 0.6)]), 0.8, 0.3)
    floete = b.noten("Flute", "flute", [(0, 6, 76, 0.8), (6, 2, 79, 0.7), (8, 8, 72, 0.8), (16, 6, 74, 0.8), (22, 2, 72, 0.7), (24, 8, 69, 0.8),
                                        (32, 6, 71, 0.8), (38, 2, 74, 0.7), (40, 8, 78, 0.8), (48, 4, 76, 0.8), (52, 4, 74, 0.7), (56, 8, 71, 0.8)], 0.7, -0.1)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Clave", (clave, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (gitarre, 0, 4))
    b.kanal("Flute", (floete, 8, 2))
    return b.fertig()


def blues():
    # Zwoelf Takte in A, im Dreier-Raster: drei Schritte je Schlag, der Shuffle liegt auf dem ersten und dritten
    b = Bau("Twelve bars", "Blues", 96, 0.3, schlaege=4, je_schlag=3)
    form = [[45, 45, 45, 45], [50, 50, 45, 45], [52, 50, 45, 52]]  # I I I I | IV IV I I | V IV I V
    beat = b.drums("Shuffle", [("Kick", "kit:kick", 0.9, 0.0, "x.....x....."), ("Snare", "kit:snare", 0.8, 0.0, ["...x.....x.."] * 3 + ["...x.....x.x"]),
                               ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.ox.ox.ox.o")])
    boogie = (0, 4, 7, 9)
    for i, grund in enumerate(form):
        bass = b.noten("Bass %d" % (i + 1), "upright_bass", b.takte(grund, lambda t, g: [(3 * s, 2, g - 12 + boogie[s], 0.9) for s in range(4)] + [(3 * s + 2, 1, g - 12 + boogie[s], 0.6) for s in range(4)]), 1.0)
        gitarre = b.noten("Guitar %d" % (i + 1), "clean_guitar", b.takte(grund, lambda t, g: [n for s in range(4) for n in ((3 * s, 2, g, 0.8), (3 * s, 2, g + (7 if s % 2 == 0 else 9), 0.8),
                                                                                                                (3 * s + 2, 1, g, 0.6), (3 * s + 2, 1, g + (7 if s % 2 == 0 else 9), 0.6))]), 0.7, 0.3)
        orgel = b.noten("Organ %d" % (i + 1), "organ", b.takte(grund, lambda t, g: [(0, 12, g + 16, 0.6), (0, 12, g + 22, 0.5)]), 0.4, -0.3)
        b.kanaele.append(("Bass" if i == 0 else None, bass, i))
        b.kanaele.append(("Guitar" if i == 0 else None, gitarre, i))
        b.kanaele.append(("Organ" if i == 0 else None, orgel, i))
    # a-Moll-Pentatonik passt ueber alle drei Stufen
    sax = b.noten("Sax", "sax", [(0, 5, 69, 0.9), (5, 1, 72, 0.7), (6, 6, 74, 0.9), (12, 3, 76, 0.9), (15, 2, 74, 0.8), (17, 1, 72, 0.7), (18, 6, 69, 0.9),
                                 (24, 2, 72, 0.8), (26, 1, 74, 0.7), (27, 3, 76, 0.9), (30, 6, 79, 0.9), (36, 3, 76, 0.9), (39, 3, 74, 0.8), (42, 6, 69, 0.9)], 0.7, -0.1)
    # die Kanaele aus den drei Abschnitten zusammensetzen
    reihen = {}
    for name, muster, abschnitt in b.kanaele:
        art = muster.split(" ")[0]
        reihen.setdefault(art, []).append((muster, 4 * abschnitt, 1))
    b.kanaele = []
    b.kanal("Drums", (beat, 0, 3))
    for art in ("Bass", "Guitar", "Organ"):
        b.kanal(art, *reihen[art])
    b.kanal("Sax", (sax, 4, 2))
    return b.fertig()


def waltz():
    # Drei Schlaege je Takt
    b = Bau("Three steps", "Jazz Waltz", 138, 0.3, schlaege=3, je_schlag=4)
    akk = [(60, 64, 67), (57, 60, 64), (50, 53, 57), (55, 59, 62)]  # C Am Dm G
    beat = b.drums("Brushes", [("Kick", "kit:kick", 0.7, 0.0, "x..........."), ("Hi-Hat", "kit:hat_closed", 0.45, 0.25, "....o...o.-."),
                               ("Snare", "kit:snare", 0.3, 0.0, ["............"] * 3 + ["........-.o."])])
    bass = b.noten("Bass", "upright_bass", b.takte(akk, lambda t, a: [(0, 4, a[0] - 24, 0.9), (8, 4, a[2] - 24, 0.6)] if t % 2 else [(0, 8, a[0] - 24, 0.9)]), 1.0)
    gitarre = b.noten("Guitar", "nylon_guitar", b.takte(akk, lambda t, a: schlag((4, 8), 3, a, 0.7)), 0.7, 0.3)
    vibes = b.noten("Vibes", "vibraphone", [(0, 8, 76, 0.8), (8, 4, 79, 0.7), (12, 12, 81, 0.8), (24, 4, 77, 0.8), (28, 4, 76, 0.7), (32, 4, 74, 0.7), (36, 12, 71, 0.8)], 0.7, -0.2)
    floete = b.noten("Flute", "flute", [(0, 12, 84, 0.7), (12, 8, 81, 0.7), (20, 4, 79, 0.6), (24, 12, 77, 0.7), (36, 8, 79, 0.7), (44, 4, 83, 0.6)], 0.6, 0.1)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Guitar", (gitarre, 0, 4))
    b.kanal("Vibes", (vibes, 4, 3))
    b.kanal("Flute", (floete, 8, 2))
    return b.fertig()


def lofi():
    b = Bau("Rainy window", "Lo-Fi", 78, 0.32, swing=0.3)
    akk = [(53, 57, 60, 64), (52, 55, 59, 62), (50, 53, 57, 60), (48, 52, 55, 59)]  # Fmaj7 Em7 Dm7 Cmaj7
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.9, 0.0, ["x.....x...x.....", "x.....x..x......"] * 2), ("Snare", "kit:snare", 0.6, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.4, 0.2, "o.-.o.-.o.-.o.--")])
    bass = b.noten("Bass", "round_bass", b.takte(akk, lambda t, a: [(0, 6, a[0] - 24, 0.9), (6, 2, a[0] - 24, 0.6), (10, 4, a[2] - 24, 0.7)]), 1.0)
    keys = b.noten("Keys", "tine_piano", b.takte(akk, lambda t, a: schlag((0,), 7, a, 0.7) + schlag((10,), 5, a[1:], 0.55)), 0.7, -0.2)
    vibes = b.noten("Vibes", "vibraphone", [(2, 4, 76, 0.7), (8, 6, 72, 0.7), (20, 2, 74, 0.6), (22, 8, 71, 0.7), (34, 4, 72, 0.7), (40, 6, 69, 0.7), (52, 2, 71, 0.6), (54, 8, 67, 0.7)], 0.7, 0.25)
    kalimba = b.noten("Kalimba", "kalimba", b.takte(akk, lambda t, a: [(14, 2, a[3] + 12, 0.6)] if t % 2 == 0 else [(12, 2, a[2] + 12, 0.6), (14, 2, a[3] + 12, 0.5)]), 0.5, 0.4)
    b.kanal("Drums", (beat, 4, 3))
    b.kanal("Bass", (bass, 4, 3))
    b.kanal("Keys", (keys, 0, 4))
    b.kanal("Vibes", (vibes, 8, 2))
    b.kanal("Kalimba", (kalimba, 0, 4))
    return b.fertig()


# --- Synth-Pop, Afrobeat, Metal ----------------------------------------------

def synthpop():
    b = Bau("Neon heart", "Synth-Pop", 118, 0.24)
    akk = [(60, 64, 67), (55, 59, 62), (57, 60, 64), (53, 57, 60)]  # C G Am F
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x..."), ("Snare", "kit:snare", 0.9, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.45, 0.2, "x.x.x.x.x.x.x.x."),
                            ("Tom", "kit:tom", 0.6, -0.3, ["................"] * 3 + ["..........xxx.x."])])
    bass = b.noten("Bass", "bass", b.takte(akk, lambda t, a: schlag(range(0, 16, 2), 2, a[0] - 24, 0.85)), 0.9)
    chords = b.noten("Chords", "super_saw", b.takte(akk, lambda t, a: schlag((0,), 14, a, 0.7)), 0.6, -0.15)
    arp = b.noten("Arp", "pluck", b.takte(akk, lambda t, a: lauf(range(16), 1, (a[0] + 12, a[1] + 12, a[2] + 12, a[1] + 12), 0.6, (0, 4, 8, 12))), 0.5, 0.35)
    lead = b.noten("Lead", "lead", [(0, 4, 79, 0.9), (4, 2, 76, 0.8), (6, 2, 79, 0.8), (8, 8, 84, 0.9), (16, 4, 83, 0.9), (20, 2, 79, 0.8), (22, 2, 74, 0.8), (24, 8, 79, 0.9),
                                    (32, 4, 76, 0.9), (36, 2, 72, 0.8), (38, 2, 76, 0.8), (40, 8, 81, 0.9), (48, 4, 77, 0.9), (52, 4, 76, 0.8), (56, 8, 72, 0.9)], 0.5, 0.1)
    hook = b.noten("Hook", "brass", b.takte(akk, lambda t, a: schlag((0, 3, 6), 2, (a[0] + 12, a[2] + 12), 0.8)), 0.5, -0.3)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 0, 4))
    b.kanal("Chords", (chords, 4, 3))
    b.kanal("Arp", (arp, 0, 4))
    b.kanal("Lead", (lead, 8, 2))
    b.kanal("Hook", (hook, 12, 1))
    return b.fertig()


def afrobeat():
    b = Bau("Lagos night", "Afrobeat", 108, 0.28)
    grund = [45, 45, 43, 45]  # Am Am G Am
    beat = b.drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, "x..x..x...x..x.."), ("Snare", "kit:snare", 0.6, 0.0, "....x..o....x..o"),
                            ("Hi-Hat", "kit:hat_closed", 0.45, 0.25, "x-x-x-x-x-x-x-x-"),
                            ("Open Hat", "kit:hat_open", 0.35, 0.25, "......x.......x.")])
    glocke = b.noten("Bell", "cowbell", b.takte(grund, lambda t, g: lauf((0, 2, 4, 5, 7, 9, 11, 12, 14), 1, (81, 81, 84), 0.7, (0, 5, 12))), 0.5, 0.35)
    conga = b.noten("Congas", "conga", b.takte(grund, lambda t, g: lauf((2, 3, 6, 8, 10, 11, 14, 15), 1, (60, 60, 65, 57, 60, 60, 65, 65), 0.75, (6, 14))), 0.7, -0.35)
    bass = b.noten("Bass", "round_bass", b.takte(grund, lambda t, g: [(0, 2, g - 12, 1.0), (3, 2, g - 12, 0.8), (6, 2, g - 5, 0.8), (8, 2, g - 2, 0.9), (11, 2, g - 5, 0.8), (14, 2, g - 9, 0.8)]), 1.0)
    gitarre = b.noten("Guitar", "clean_guitar", b.takte(grund, lambda t, g: lauf((0, 2, 3, 5, 6, 8, 10, 11, 13, 14), 1, (g + 24, g + 27, g + 24, g + 31, g + 29, g + 27), 0.7)), 0.6, 0.2)
    orgel = b.noten("Organ", "organ", b.takte(grund, lambda t, g: schlag((2, 10), 2, (g + 12, g + 15 if g == 45 else g + 16, g + 19), 0.6)), 0.4, -0.2)
    horns = b.noten("Horns", "brass", [(0, 3, 69, 0.9), (0, 3, 72, 0.8), (3, 3, 69, 0.8), (3, 3, 72, 0.7), (6, 6, 71, 0.9), (6, 6, 76, 0.8), (16, 3, 69, 0.9), (16, 3, 72, 0.8), (19, 3, 67, 0.8), (22, 6, 69, 0.9), (22, 6, 64, 0.8),
                                       (32, 3, 67, 0.9), (32, 3, 71, 0.8), (35, 3, 67, 0.8), (35, 3, 71, 0.7), (38, 6, 74, 0.9), (48, 4, 72, 0.9), (52, 4, 71, 0.8), (56, 8, 69, 0.9), (56, 8, 64, 0.8)], 0.6)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bell", (glocke, 0, 4))
    b.kanal("Congas", (conga, 0, 4))
    b.kanal("Bass", (bass, 4, 3))
    b.kanal("Guitar", (gitarre, 4, 3))
    b.kanal("Organ", (orgel, 8, 2))
    b.kanal("Horns", (horns, 8, 2))
    return b.fertig()


def metal():
    b = Bau("Iron road", "Metal", 160, 0.22)
    grund = [40, 40, 36, 38]  # E E C D
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.9, 0.0, ["xoxoxoxoxoxoxoxo"] * 3 + ["xoxoxoxox.x.xxxx"]), ("Snare", "kit:snare", 1.0, 0.0, ["....x.......x..."] * 3 + ["....x.......xxxx"]),
                            ("Open Hat", "kit:hat_open", 0.45, 0.25, "x...x...x...x..."), ("Tom", "kit:tom", 0.6, -0.3, ["................"] * 3 + ["..........x.x..."])])
    riff = b.noten("Riff", "overdrive_guitar", b.takte(grund, lambda t, g: [(s, 1, g + 12 + (3 if s in (6, 14) else 5 if s == 11 else 0), 1.0 if s in (0, 6, 11, 14) else 0.7) for s in range(16)]), 0.8, -0.3)
    bass = b.noten("Bass", "growl_bass", b.takte(grund, lambda t, g: schlag(range(0, 16, 2), 2, g, 0.9)), 0.8)
    harm = (0, 2, 3, 5, 7, 8, 11, 12)  # harmonisch Moll
    solo = b.noten("Solo", "lead", b.takte(grund, lambda t, g: [(s, 1, 76 + harm[(s * (3 if t % 2 else 1) + 2 * t) % 8], 0.85) for s in range(12)] + [(12, 4, 76 + harm[(7 - t) % 8], 0.9)]), 0.5, 0.25)
    chor = b.noten("Choir", "strings", b.takte(grund, lambda t, g: [(0, 16, g + 24, 0.6), (0, 16, g + 31, 0.5)]), 0.5)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Guitar", (riff, 0, 4))
    b.kanal("Bass", (bass, 4, 3))
    b.kanal("Strings", (chor, 8, 2))
    b.kanal("Lead", (solo, 8, 2))
    return b.fertig()


LOOPS = {
    "trance_uplifting": trance_uplifting, "trance_acid": trance_acid, "trance_psy": trance_psy,
    "funk": funk, "disco": disco, "reggae": reggae, "ska": ska, "reggaeton": reggaeton,
    "hiphop": hiphop, "trap": trap, "dnb": dnb, "techno": techno,
    "bossa": bossa, "blues": blues, "waltz": waltz, "lofi": lofi,
    "synthpop": synthpop, "afrobeat": afrobeat, "metal": metal,
}


def main():
    # erst die Loops: die Genres eines Instruments werden aus ihnen gezaehlt
    ordner = HIER / "genres"
    ordner.mkdir(exist_ok=True)
    for name, bau in LOOPS.items():
        (ordner / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    from instrument_katalog import ergaenze, genres_aus_loops
    genres = genres_aus_loops()
    for name, daten in INSTRUMENTE.items():
        (HIER / "instruments" / (name + ".lvinst")).write_text(als_text(ergaenze(name, daten, genres)),
                                                               encoding="utf-8", newline="\n")
    print(len(INSTRUMENTE), "Instrumente,", len(LOOPS), "Loops")


if __name__ == "__main__":
    main()
