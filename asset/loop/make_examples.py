"""Erzeugt die sechzehn Instrumente aus Schichten und die fuenf Beispiel-Loops
dazu (S111, Wunsch Patrik: weitere Instrumente, auch ganz ungewoehnliche, und
fuenf Beispiele).

    python asset/loop/make_examples.py

Schreibt nach `asset/loop/instruments/*.lvinst` (Fassung 2, mit `layers`) und
`asset/loop/*_demo.lvloop`. Kein Zufall: zwei Laeufe ergeben dieselben Dateien.
Massgebend sind die Dateien im Repo, nicht ein neuer Lauf — wer ein Instrument
im Instrument-Creator nachstellt und speichert, aendert die Datei, nicht dieses
Skript.

Die Einstellungen sind nach Rechnung gesetzt, nicht nach Gehoer.

**Seit S133 (Stufe 23g):** die Overdrive Guitar schreibt dieses Skript als Classic —
danach `make_saiten_instrumente.py` laufen lassen, das sie auf die Saiten umstellt.
"""

import json
from pathlib import Path

HIER = Path(__file__).resolve().parent


# --- Instrumente -------------------------------------------------------------

def schicht(name, waveform, **werte):
    s = {
        "name": name, "on": True, "level": 1.0, "transpose": 0, "startOffset": 0, "drive": 0,
        "waveform": waveform, "harmonics": [], "voices": 1, "detune": 0,
        "attack": 5, "decay": 200, "sustain": 0.7, "release": 150,
        "filterCutoff": 0, "filterEnvelope": 0,
        "vibratoRate": 0, "vibratoDepth": 0, "vibratoDelay": 0,
        "damping": 0.996,
    }
    for schluessel, wert in werte.items():
        if schluessel not in s:
            raise KeyError(schluessel)
        s[schluessel] = wert
    return s


def instrument(titel, level, *schichten):
    # Art, Genres und Beschreibung (Fassung 3, S114) kommen beim Schreiben dazu:
    # instrument_katalog.ergaenze
    return {"format": "lumiviz-instrument", "version": 2, "title": titel, "level": level, "layers": list(schichten)}


INSTRUMENTE = {
    # --- neun gewoehnliche
    "bell": instrument(
        "Bell", 0.7,
        schicht("Strike", "sine", harmonics=[0, 0, 0.25], attack=1, decay=1800, sustain=0, release=900),
        schicht("Partial", "sine", transpose=19, voices=2, detune=14, attack=1, decay=900, sustain=0, release=500, level=0.45),
        schicht("Ping", "sine", transpose=24, attack=1, decay=350, sustain=0, release=200, level=0.3)),
    "tine_piano": instrument(
        "Tine Piano", 0.75,
        schicht("Body", "sine", harmonics=[0.35, 0.1, 0.05], attack=2, decay=1400, sustain=0.15, release=280, drive=0.25),
        schicht("Tine", "sine", transpose=24, attack=1, decay=180, sustain=0, release=120, level=0.3)),
    "brass": instrument(
        "Brass", 0.7,
        schicht("Section", "saw", voices=2, detune=7, attack=45, decay=250, sustain=0.8, release=140,
                filterCutoff=2.5, filterEnvelope=7, vibratoRate=5, vibratoDepth=8, vibratoDelay=350),
        schicht("Edge", "square", startOffset=20, attack=60, decay=200, sustain=0.6, release=120,
                filterCutoff=3, filterEnvelope=4, level=0.3)),
    "flute": instrument(
        "Flute", 0.8,
        schicht("Tone", "triangle", attack=70, decay=200, sustain=0.85, release=160, filterCutoff=5,
                vibratoRate=5.2, vibratoDepth=12, vibratoDelay=400),
        schicht("Air", "sine", transpose=12, harmonics=[0.3], attack=90, decay=200, sustain=0.6, release=160, level=0.2),
        schicht("Chiff", "string", transpose=12, damping=0.92, attack=1, release=40, level=0.25)),
    "super_saw": instrument(
        "Super Saw", 0.5,
        schicht("Wide", "saw", voices=3, detune=28, attack=8, decay=300, sustain=0.8, release=260, filterCutoff=9),
        schicht("Tight", "saw", voices=3, detune=11, attack=8, decay=300, sustain=0.8, release=260, filterCutoff=9, level=0.8),
        schicht("Top", "saw", transpose=12, voices=3, detune=18, attack=8, decay=300, sustain=0.8, release=260,
                filterCutoff=6, level=0.4)),
    "fifth_pad": instrument(
        "Fifth Pad", 0.55,
        schicht("Root", "saw", voices=3, detune=12, attack=350, decay=600, sustain=0.85, release=900, filterCutoff=4),
        schicht("Fifth", "saw", transpose=7, startOffset=120, voices=3, detune=10, attack=500, decay=600, sustain=0.85,
                release=900, filterCutoff=3, level=0.45),
        schicht("Low", "sine", transpose=-12, attack=300, decay=600, sustain=1.0, release=700, level=0.35)),
    "overdrive_guitar": instrument(
        "Overdrive Guitar", 0.5,
        schicht("String", "string", damping=0.997, attack=2, release=120, drive=0.75),
        schicht("Fifth", "string", transpose=7, damping=0.996, attack=2, release=120, drive=0.75, level=0.5),
        schicht("Octave", "string", transpose=12, startOffset=10, damping=0.996, attack=2, release=120, drive=0.6, level=0.35)),
    "kalimba": instrument(
        "Kalimba", 0.8,
        schicht("Tine", "sine", harmonics=[0, 0, 0.2, 0, 0.1], attack=1, decay=450, sustain=0, release=250),
        schicht("Knock", "string", transpose=12, damping=0.93, attack=1, release=30, level=0.4),
        schicht("Ring", "sine", transpose=24, voices=2, detune=20, attack=1, decay=150, sustain=0, release=100, level=0.2)),
    "marimba": instrument(
        "Marimba", 0.85,
        schicht("Bar", "sine", harmonics=[0, 0, 0.3], attack=1, decay=380, sustain=0, release=200),
        schicht("Mallet", "triangle", transpose=24, attack=1, decay=40, sustain=0, release=30, level=0.25)),
    # --- sieben ungewoehnliche
    "cascade": instrument(
        "Cascade", 0.6,
        schicht("One", "sine", harmonics=[0.3, 0.1], attack=2, decay=500, sustain=0.25, release=300),
        schicht("Two", "sine", transpose=12, startOffset=90, harmonics=[0.3, 0.1], attack=2, decay=500, sustain=0.25,
                release=300, level=0.6),
        schicht("Three", "sine", transpose=19, startOffset=180, harmonics=[0.3, 0.1], attack=2, decay=500, sustain=0.25,
                release=300, level=0.5),
        schicht("Four", "sine", transpose=24, startOffset=270, harmonics=[0.3, 0.1], attack=2, decay=500, sustain=0.25,
                release=300, level=0.45)),
    "siren": instrument(
        "Siren", 0.5,
        schicht("Wail", "square", attack=30, decay=0, sustain=1.0, release=200, filterCutoff=6,
                vibratoRate=1.6, vibratoDepth=100, vibratoDelay=0),
        schicht("Shadow", "saw", startOffset=150, voices=2, detune=30, attack=30, decay=0, sustain=1.0, release=200,
                filterCutoff=4, vibratoRate=1.6, vibratoDepth=100, vibratoDelay=0, level=0.4)),
    "alien_choir": instrument(
        "Alien Choir", 0.6,
        schicht("Vowel", "sine", harmonics=[0, 0.6, 0, 0.4, 0, 0.2], voices=3, detune=35, attack=250, decay=300,
                sustain=0.9, release=600, vibratoRate=6.5, vibratoDepth=35, vibratoDelay=200),
        schicht("Whisper", "triangle", transpose=12, startOffset=200, voices=3, detune=60, attack=400, decay=300,
                sustain=0.9, release=600, vibratoRate=3, vibratoDepth=50, vibratoDelay=0, level=0.4),
        schicht("Throat", "sine", transpose=19, attack=250, decay=300, sustain=0.9, release=600,
                vibratoRate=9, vibratoDepth=40, vibratoDelay=0, level=0.25)),
    "broken_radio": instrument(
        "Broken Radio", 0.45,
        schicht("Carrier", "square", drive=1.0, attack=2, decay=150, sustain=0.7, release=80, filterCutoff=5,
                vibratoRate=20, vibratoDepth=100, vibratoDelay=0),
        schicht("Static", "string", transpose=24, damping=0.999, attack=2, release=80, drive=1.0, level=0.3),
        schicht("Hum", "sine", drive=0.5, attack=2, decay=150, sustain=0.7, release=80, level=0.5)),
    "ghost": instrument(
        "Ghost", 0.7,
        schicht("Wail", "sine", harmonics=[0.15], voices=3, detune=55, attack=900, decay=500, sustain=0.9, release=1500,
                vibratoRate=0.7, vibratoDepth=60, vibratoDelay=0),
        schicht("Breath", "triangle", transpose=12, startOffset=400, voices=2, detune=40, attack=1200, decay=500,
                sustain=0.9, release=1800, vibratoRate=0.4, vibratoDepth=45, vibratoDelay=0, level=0.4)),
    "growl_bass": instrument(
        "Growl Bass", 0.55,
        schicht("Growl", "saw", drive=0.9, attack=3, decay=220, sustain=0.6, release=90, filterCutoff=2, filterEnvelope=6,
                vibratoRate=17, vibratoDepth=25, vibratoDelay=0),
        schicht("Grit", "square", transpose=12, drive=0.7, attack=3, decay=220, sustain=0.6, release=90, filterCutoff=3,
                level=0.3),
        schicht("Sub", "sine", attack=3, decay=220, sustain=0.8, release=90, level=0.6)),
    "reverse_swell": instrument(
        "Reverse Swell", 0.7,
        schicht("Swell", "saw", voices=3, detune=15, attack=800, decay=0, sustain=1.0, release=25, filterCutoff=2,
                filterEnvelope=8),
        schicht("Shine", "saw", transpose=12, voices=2, detune=20, attack=1000, decay=0, sustain=1.0, release=25,
                filterCutoff=2, filterEnvelope=10, level=0.4)),
}


# --- Loops -------------------------------------------------------------------

TAKT = 16  # Schritte je Takt: 4 Schlaege, 4 Schritte je Schlag


def drums(name, spuren, takte=4):
    """spuren: Liste (Name, Klang, Lautstaerke, Panorama, {Schritt im Takt: Lautstaerke}) — je Takt gleich."""
    tracks, rows = [], []
    for titel, klang, laut, pan, schlaege in spuren:
        tracks.append({"name": titel, "sound": klang, "volume": laut, "pan": pan, "mute": False})
        zeile = [0] * (TAKT * takte)
        for takt in range(takte):
            for schritt, wert in schlaege.items():
                if isinstance(schritt, tuple):  # (Takt, Schritt): nur in diesem Takt
                    if schritt[0] == takt:
                        zeile[takt * TAKT + schritt[1]] = wert
                else:
                    zeile[takt * TAKT + schritt] = wert
        rows.append(zeile)
    return {"name": name, "kind": "drumkit", "bars": takte, "tracks": tracks, "rows": rows}


def muster(name, inst, noten, laut=0.8, pan=0.0, takte=4):
    for schritt, laenge, hoehe, staerke in noten:
        assert 0 <= schritt and schritt + laenge <= TAKT * takte, (name, schritt, laenge)
        assert 24 <= hoehe <= 96 and 0 < staerke <= 1, (name, hoehe, staerke)
    return {"name": name, "kind": "instrument", "bars": takte, "instrument": inst, "volume": laut, "pan": pan,
            "mute": False, "notes": [list(n) for n in sorted(noten)]}


def je_takt(akkorde, tu):
    """tu(takt, akkord) -> Noten mit Schritten im Takt; je Takt ein Akkord."""
    noten = []
    for takt, akkord in enumerate(akkorde):
        for schritt, laenge, hoehe, staerke in tu(takt, akkord):
            noten.append((takt * TAKT + schritt, laenge, hoehe, staerke))
    return noten


def loop(titel, genre, bpm, laut, patterns, kanaele, swing=0.0):
    return {
        "format": "lumiviz-loop", "version": 2, "bpm": bpm, "beatsPerBar": 4, "stepsPerBeat": 4, "swing": swing,
        "volume": laut, "tags": {"title": titel, "artist": "LumiViz", "genre": genre},
        "patterns": patterns,
        "channels": [{"name": name, "enabled": True,
                      "clips": [{"pattern": m, "start": start, "repeat": mal} for m, start, mal in clips]}
                     for name, clips in kanaele],
    }


def bells_demo():
    # C – Am – F – G
    akk = [(60, 64, 67), (57, 60, 64), (53, 57, 60), (55, 59, 62)]
    pad = je_takt(akk, lambda t, a: [(0, 16, a[0] - 12, 0.7), (0, 16, a[1] - 12, 0.6)])
    kalimba = je_takt(akk, lambda t, a: [(s, 2, (a[0], a[1], a[2], a[1] + 12, a[2], a[1], a[0] + 12, a[2])[i] + 12, 0.75 if i % 2 else 0.9)
                                         for i, s in enumerate(range(0, 16, 2))])
    marimba = je_takt(akk, lambda t, a: [(0, 2, a[0], 0.9), (3, 1, a[2], 0.6), (6, 2, a[0], 0.8), (8, 1, a[1], 0.6),
                                         (11, 1, a[2], 0.7), (14, 2, a[0] + 12, 0.7)])
    bell = [(0, 8, 79, 0.9), (8, 8, 76, 0.7), (16, 12, 72, 0.9), (28, 4, 76, 0.6),
            (32, 8, 77, 0.9), (40, 8, 72, 0.7), (48, 6, 74, 0.9), (54, 10, 79, 0.8)]
    bass = je_takt(akk, lambda t, a: [(0, 6, a[0] - 24, 0.9), (8, 4, a[0] - 24, 0.7), (14, 2, a[0] - 12, 0.6)])
    beat = drums("Shaker", [("Hi-Hat", "kit:hat_closed", 0.4, 0.25, {s: (0.8 if s % 4 == 0 else 0.45) for s in range(0, 16, 2)}),
                            ("Tom", "kit:tom", 0.5, -0.2, {12: 0.8, (3, 14): 0.6}),
                            ("Kick", "kit:kick", 0.7, 0.0, {0: 1.0, 10: 0.6})])
    return loop("Bell garden", "Ambient", 92, 0.3,
                [beat, muster("Pad", "fifth_pad", pad, 0.6, -0.1), muster("Kalimba", "kalimba", kalimba, 0.8, 0.3),
                 muster("Marimba", "marimba", marimba, 0.8, -0.3), muster("Bells", "bell", bell, 0.8, 0.1),
                 muster("Sub", "sub_bass", bass, 0.8)],
                [("Drums", [("Shaker", 8, 2)]), ("Pad", [("Pad", 0, 4)]), ("Kalimba", [("Kalimba", 0, 4)]),
                 ("Marimba", [("Marimba", 4, 3)]), ("Bells", [("Bells", 8, 2)]), ("Bass", [("Sub", 4, 3)])])


def synthwave_demo():
    # Am – F – C – G
    akk = [(57, 60, 64), (53, 57, 60), (48, 52, 55), (55, 59, 62)]
    chords = je_takt(akk, lambda t, a: [(0, 14, h, 0.7) for h in a])
    bass = je_takt(akk, lambda t, a: [(s, 2, a[0] - 24 + (12 if s == 14 else 0), 0.9 if s % 4 == 0 else 0.7) for s in range(0, 16, 2)])
    tine = je_takt(akk, lambda t, a: [(s, 2,(a[0], a[2], a[1] + 12, a[2], a[0] + 12, a[2], a[1] + 12, a[2])[i] + 12, 0.7)
                                      for i, s in enumerate(range(0, 16, 2))])
    brass = [(0, 6, 69, 0.9), (6, 2, 72, 0.7), (8, 8, 76, 0.9),
             (16, 6, 77, 0.9), (22, 2, 76, 0.7), (24, 8, 72, 0.8),
             (32, 4, 67, 0.8), (36, 4, 72, 0.8), (40, 8, 76, 0.9),
             (48, 6, 74, 0.9), (54, 2, 71, 0.7), (56, 8, 67, 0.8)]
    beat = drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, {0: 1.0, 8: 1.0, (3, 14): 0.7}),
                          ("Snare", "kit:snare", 0.8, 0.0, {4: 1.0, 12: 1.0}),
                          ("Hi-Hat", "kit:hat_closed", 0.45, 0.2, {s: (0.8 if s % 4 == 0 else 0.5) for s in range(0, 16, 2)}),
                          ("Open Hat", "kit:hat_open", 0.35, 0.2, {(3, 14): 0.7})])
    return loop("Night drive", "Synthwave", 108, 0.28,
                [beat, muster("Chords", "super_saw", chords, 0.7, -0.15), muster("Bass", "bass", bass, 0.9),
                 muster("Tines", "tine_piano", tine, 0.7, 0.3), muster("Brass", "brass", brass, 0.8, 0.1)],
                [("Drums", [("Beat", 0, 4)]), ("Bass", [("Bass", 0, 4)]), ("Chords", [("Chords", 4, 3)]),
                 ("Tines", [("Tines", 0, 1), ("Tines", 8, 2)]), ("Lead", [("Brass", 8, 2)])])


def rock_demo():
    # E – G – A – E, Grundtoene; die Gitarre legt Quinte und Oktave selbst dazu
    grund = [52, 55, 57, 52]
    gitarre = je_takt(grund, lambda t, g: [(s, 2 if s not in (6, 14) else 1, g, 0.9 if s % 4 == 0 else 0.7) for s in (0, 2, 4, 6, 8, 10, 12, 14)]
                                          + [(7, 1, g + 3, 0.7), (15, 1, g + 5, 0.7)])
    bass = je_takt(grund, lambda t, g: [(0, 3, g - 12, 0.9), (4, 2, g - 12, 0.7), (6, 2, g - 12, 0.8), (8, 3, g - 12, 0.9),
                                         (12, 2, g - 12, 0.7), (14, 2, g - 5, 0.8)])
    orgel = je_takt(grund, lambda t, g: [(0, 16, g + 12, 0.6), (0, 16, g + 19, 0.5), (0, 16, g + 15 if g != 57 else g + 16, 0.5)])
    solo = [(0, 2, 76, 0.9), (2, 2, 79, 0.8), (4, 4, 81, 0.9), (8, 2, 79, 0.8), (10, 2, 76, 0.8), (12, 4, 74, 0.9),
            (16, 2, 74, 0.8), (18, 2, 76, 0.8), (20, 2, 79, 0.9), (22, 2, 81, 0.8), (24, 8, 83, 0.9),
            (32, 3, 81, 0.9), (35, 3, 79, 0.8), (38, 2, 76, 0.8), (40, 4, 81, 0.9), (44, 4, 84, 0.9),
            (48, 2, 83, 0.9), (50, 2, 81, 0.8), (52, 2, 79, 0.8), (54, 2, 76, 0.8), (56, 8, 76, 0.9)]
    beat = drums("Beat", [("Kick", "kit:kick", 1.0, 0.0, {0: 1.0, 6: 0.8, 8: 1.0, (3, 15): 0.7}),
                          ("Snare", "kit:snare", 0.9, 0.0, {4: 1.0, 12: 1.0, (3, 13): 0.6, (3, 14): 0.8}),
                          ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, {s: (0.9 if s % 4 == 0 else 0.6) for s in range(0, 16, 2)}),
                          ("Open Hat", "kit:hat_open", 0.4, 0.25, {(1, 14): 0.7}),
                          ("Tom", "kit:tom", 0.6, -0.3, {(3, 10): 0.8, (3, 11): 0.7})])
    return loop("Garage", "Rock", 132, 0.26,
                [beat, muster("Riff", "overdrive_guitar", gitarre, 0.8, -0.3), muster("Bass", "growl_bass", bass, 0.8),
                 muster("Organ", "organ", orgel, 0.5, 0.3), muster("Solo", "lead", solo, 0.6, 0.15)],
                [("Drums", [("Beat", 0, 4)]), ("Guitar", [("Riff", 0, 4)]), ("Bass", [("Bass", 4, 3)]),
                 ("Organ", [("Organ", 8, 2)]), ("Lead", [("Solo", 8, 2)])])


def strange_demo():
    # d-Moll, wenig Harmonie: D – D – Bb – A
    grund = [50, 50, 46, 45]
    radio = je_takt(grund, lambda t, g: [(s, 1, g + (0, 0, 12, 0, 7, 0, 12, 3)[i % 8], 0.9 if i % 4 == 0 else 0.6)
                                         for i, s in enumerate((0, 1, 3, 4, 6, 8, 10, 11, 12, 14))])
    sirene = [(0, 28, 62, 0.7), (32, 28, 65, 0.7)]
    chor = je_takt(grund, lambda t, g: [(0, 15, g + 12, 0.7), (0, 15, g + 19, 0.6), (0, 15, g + (15 if t < 2 else 16), 0.6)])
    kaskade = je_takt(grund, lambda t, g: [(0, 8, g + 24, 0.9), (8, 6, g + 27 if t < 2 else g + 28, 0.7)])
    bass = je_takt(grund, lambda t, g: [(0, 4, g - 12, 0.9), (6, 2, g - 12, 0.7), (10, 3, g - 12, 0.8), (14, 2, g, 0.6)])
    beat = drums("Machine", [("Kick", "kit:kick", 1.0, 0.0, {0: 1.0, 7: 0.7, 10: 0.9}),
                             ("Clap", "kit:clap", 0.6, 0.1, {4: 0.9, 12: 0.9, (3, 15): 0.6}),
                             ("Tom", "kit:tom", 0.6, -0.4, {3: 0.6, 14: 0.7}),
                             ("Hi-Hat", "kit:hat_closed", 0.4, 0.4, {2: 0.6, 6: 0.6, 9: 0.5, 13: 0.6})])
    return loop("Strange machines", "Experimental", 100, 0.26,
                [beat, muster("Radio", "broken_radio", radio, 0.6, -0.35), muster("Siren", "siren", sirene, 0.45, 0.4),
                 muster("Choir", "alien_choir", chor, 0.7, 0.0), muster("Cascade", "cascade", kaskade, 0.7, 0.3),
                 muster("Growl", "growl_bass", bass, 0.8)],
                [("Drums", [("Machine", 4, 3)]), ("Radio", [("Radio", 0, 4)]), ("Choir", [("Choir", 4, 3)]),
                 ("Cascade", [("Cascade", 8, 2)]), ("Siren", [("Siren", 12, 1)]), ("Bass", [("Growl", 4, 3)])],
                swing=0.2)


def ambient_demo():
    # Dm – Bb – F – C, ohne Schlagzeug
    akk = [(62, 65, 69), (58, 62, 65), (53, 57, 60), (48, 52, 55)]
    pad = je_takt(akk, lambda t, a: [(0, 16, a[0] - 12, 0.7), (0, 16, a[2] - 12, 0.5)])
    # der Schwell endet auf dem Taktstrich: er fuehrt in den naechsten Akkord
    schwell = je_takt(akk, lambda t, a: [(4, 12, akk[(t + 1) % 4][1], 0.8), (4, 12, akk[(t + 1) % 4][0], 0.6)])
    geist = [(0, 30, 74, 0.7), (32, 30, 72, 0.7)]
    floete = [(0, 6, 77, 0.8), (6, 2, 76, 0.6), (8, 8, 74, 0.8),
              (16, 8, 74, 0.8), (24, 4, 77, 0.7), (28, 4, 82, 0.8),
              (32, 6, 81, 0.8), (38, 2, 77, 0.6), (40, 8, 72, 0.8),
              (48, 4, 76, 0.7), (52, 4, 79, 0.8), (56, 8, 76, 0.8)]
    glocke = je_takt(akk, lambda t, a: [(0, 16, a[2] + 12, 0.8)] if t % 2 == 0 else [(8, 8, a[0] + 12, 0.6)])
    kaskade = je_takt(akk, lambda t, a: [(8, 8, a[0], 0.7)] if t % 2 == 1 else [])
    return loop("Slow tide", "Ambient", 72, 0.5,
                [muster("Pad", "fifth_pad", pad, 0.7, 0.0), muster("Swell", "reverse_swell", schwell, 0.6, -0.3),
                 muster("Ghost", "ghost", geist, 0.6, 0.3), muster("Flute", "flute", floete, 0.8, 0.15),
                 muster("Bells", "bell", glocke, 0.7, -0.2), muster("Cascade", "cascade", kaskade, 0.6, 0.35)],
                [("Pad", [("Pad", 0, 4)]), ("Swell", [("Swell", 4, 3)]), ("Ghost", [("Ghost", 8, 2)]),
                 ("Flute", [("Flute", 4, 2)]), ("Bells", [("Bells", 0, 1), ("Bells", 12, 1)]),
                 ("Cascade", [("Cascade", 8, 2)])])


LOOPS = {
    "bells_demo": bells_demo,
    "synthwave_demo": synthwave_demo,
    "rock_demo": rock_demo,
    "strange_demo": strange_demo,
    "ambient_demo": ambient_demo,
}


def als_text(daten):
    """JSON mit Einzug; Listen aus Zahlen (Schritte, Noten, Obertoene) in einer Zeile."""
    def schreibe(wert, einzug):
        innen = " " * (einzug + 4)
        if isinstance(wert, dict):
            if not wert:
                return "{}"
            return "{\n" + ",\n".join(innen + json.dumps(k) + ": " + schreibe(v, einzug + 4) for k, v in wert.items()) + "\n" + " " * einzug + "}"
        if isinstance(wert, list):
            if all(not isinstance(v, (dict, list)) for v in wert):
                return json.dumps(wert)
            return "[\n" + ",\n".join(innen + schreibe(v, einzug + 4) for v in wert) + "\n" + " " * einzug + "]"
        return json.dumps(wert, ensure_ascii=False)
    return schreibe(daten, 0) + "\n"


def main():
    # erst die Loops: die Genres eines Instruments werden aus ihnen gezaehlt
    for name, bau in LOOPS.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    from instrument_katalog import ergaenze, genres_aus_loops
    genres = genres_aus_loops()
    for name, daten in INSTRUMENTE.items():
        (HIER / "instruments" / (name + ".lvinst")).write_text(als_text(ergaenze(name, daten, genres)),
                                                               encoding="utf-8", newline="\n")
    print(len(INSTRUMENTE), "Instrumente,", len(LOOPS), "Loops")


if __name__ == "__main__":
    main()
