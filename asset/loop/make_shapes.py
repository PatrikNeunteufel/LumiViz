"""Erzeugt die mitgelieferten Instrumente aus Formen (S123; Klang aus einer Form,
`projects/apps/LumiViz/docs/audio/Shape_Konzept.md`).

    python asset/loop/make_shapes.py

Schreibt nach `asset/loop/instruments/shape_*.lvinst`, **Fassung 9** (die
Shape-Schicht). Jede Shape-Schicht nennt eine mitgelieferte Form aus
`asset/contours` beim Dateinamen; `shapeContour` 0 heisst „All, one path",
sonst die Kennung einer Kontur. Kein Zufall: zwei Laeufe ergeben dieselben
Dateien. Massgebend sind die Dateien im Repo, nicht ein neuer Lauf.

Wie alle mitgelieferten Instrumente nach Rechnung eingestellt, nicht nach
Gehoer: die Zahl der Harmonics setzt die Helligkeit (wenige: weich wie ein
Sinus, viele: die Ecken der Form hoerbar), Harmonics envelope wirkt wie die
Huelle eines Filters — positiv oeffnet der Anschlag, negativ schliesst er.
"""

from pathlib import Path

from make_examples import als_text, schicht

HIER = Path(__file__).resolve().parent

HUELLE = {"type": "envelope"}


def filt(art, cutoff, envelope=0.0, resonance=0.0):
    return {"type": "filter", "filterType": art, "cutoff": cutoff, "envelope": envelope, "resonance": resonance}


def drive(amount):
    return {"type": "drive", "amount": amount}


def lage(name, waveform, reihe=None, pan=0.0, **werte):
    """Eine Schicht in Fassung 7 und spaeter: statt der flachen Felder fuer Filter und Drive die Reihe `path`."""
    s = schicht(name, waveform, **werte)
    for feld in ("drive", "filterCutoff", "filterEnvelope"):
        del s[feld]
    s.update({"pan": pan, "pitchStart": 0, "pitchTime": 100, "path": list(reihe or [HUELLE])})
    return s


def form(name, datei, kontur=0, harmonics=24.0, huelle=0.0, winkel=0.0, start=0.0, xy=False, rueckwaerts=False,
         reihe=None, pitch=None, **werte):
    """Eine Shape-Schicht (Fassung 9)."""
    s = lage(name, "shape", reihe, **werte)
    if pitch:
        s["pitchStart"], s["pitchTime"] = pitch
    s.update({
        "shape": datei, "shapeContour": kontur, "shapeHarmonics": harmonics, "shapeHarmonicsEnvelope": huelle,
        "shapeAngle": winkel, "shapeStart": start, "shapeStereoXY": xy, "shapeReverse": rueckwaerts,
    })
    return s


def instrument(titel, level, *schichten):
    return {"format": "lumiviz-instrument", "version": 9, "title": titel, "level": level, "layers": list(schichten)}


INSTRUMENTE = {
    # --- Flaechen
    "shape_heart_pad": instrument(
        "Heart Pad", 0.55,
        form("Heart", "heart", harmonics=10, huelle=10, voices=3, detune=12, attack=450, decay=800, sustain=0.85,
             release=1100, vibratoRate=4.5, vibratoDepth=5, vibratoDelay=600),
        lage("Sub", "sine", transpose=-12, attack=400, decay=600, sustain=1.0, release=900, level=0.3)),
    "shape_rings_choir": instrument(
        "Rings Choir", 0.5,
        # Stereo XY: links x, rechts y — die fuenf Ringe als ein Pfad zeichnen sich im XY Scope
        form("Rings", "rings", harmonics=20, xy=True, voices=2, detune=9, attack=600, decay=900, sustain=0.8,
             release=1300, vibratoRate=5.5, vibratoDepth=14, vibratoDelay=300),
        form("Air", "rings", transpose=12, harmonics=6, voices=2, detune=16, attack=900, decay=900, sustain=0.7,
             release=1300, level=0.25)),
    "shape_infinity_drone": instrument(
        "Infinity Drone", 0.5,
        form("Eight", "infinity", harmonics=14, huelle=18, xy=True, attack=1200, decay=2500, sustain=0.9,
             release=2500, vibratoRate=0.4, vibratoDepth=10),
        form("Fifth", "infinity", transpose=7, harmonics=8, winkel=90, attack=1600, decay=2500, sustain=0.8,
             release=2500, level=0.4)),
    # --- Bass
    "shape_gear_bass": instrument(
        "Gear Bass", 0.6,
        # die Zaehne hoert man erst mit vielen Teiltoenen: der Anschlag oeffnet bis 52, dann zurueck auf 6
        form("Gear", "gear", harmonics=6, huelle=46, attack=3, decay=260, sustain=0.5, release=160,
             reihe=[HUELLE, drive(0.25)]),
        lage("Sub", "sine", attack=3, decay=400, sustain=0.9, release=160, level=0.45)),
    "shape_moon_bass": instrument(
        "Moon Bass", 0.65,
        form("Moon", "moon", harmonics=4, huelle=12, attack=4, decay=500, sustain=0.75, release=200),
        lage("Sub", "sine", transpose=-12, attack=4, decay=500, sustain=0.9, release=200, level=0.35)),
    # --- Lead und Pluck
    "shape_star_lead": instrument(
        "Star Lead", 0.5,
        form("Star", "star", harmonics=40, voices=2, detune=10, attack=12, decay=300, sustain=0.85, release=220,
             vibratoRate=5.5, vibratoDepth=12, vibratoDelay=250, reihe=[HUELLE, drive(0.2)]),
        form("Octave", "star", transpose=12, harmonics=16, attack=12, decay=300, sustain=0.7, release=220, level=0.3)),
    "shape_bolt_pluck": instrument(
        "Bolt Pluck", 0.9,
        # negative Huelle: der Anschlag hat alle 64, dann schliesst es wie ein Filter
        form("Bolt", "bolt", harmonics=64, huelle=-58, attack=1, decay=320, sustain=0, release=200),
        lage("Body", "sine", attack=1, decay=380, sustain=0, release=200, level=0.35)),
    "shape_flower_flute": instrument(
        "Flower Flute", 0.7,
        form("Flower", "flower", harmonics=8, attack=80, decay=250, sustain=0.85, release=180,
             vibratoRate=5.2, vibratoDepth=11, vibratoDelay=380),
        lage("Breath", "noise", transpose=24, attack=30, decay=120, sustain=0.15, release=120, level=0.12,
             reihe=[filt("bandpass", 1.5, resonance=0.3), HUELLE])),
    # --- mehrere Konturen als Schichten
    "shape_smiley_keys": instrument(
        "Smiley Keys", 0.5,
        # je Kontur eine Schicht: Gesicht auf der Note, die Augen eine Oktave und eine Duodezime hoeher, der Mund als Quinte
        form("Face", "smiley", kontur=1, harmonics=12, attack=2, decay=1100, sustain=0.2, release=400),
        form("Left eye", "smiley", kontur=2, transpose=12, harmonics=8, attack=2, decay=600, sustain=0, release=300,
             level=0.35, pan=-0.4),
        form("Right eye", "smiley", kontur=3, transpose=19, harmonics=8, attack=2, decay=500, sustain=0, release=300,
             level=0.25, pan=0.4),
        form("Mouth", "smiley", kontur=4, transpose=7, harmonics=16, attack=2, decay=800, sustain=0.1, release=350,
             level=0.3)),
    "shape_atom_bell": instrument(
        "Atom Bell", 0.55,
        form("Core", "atom", kontur=1, harmonics=4, attack=1, decay=2200, sustain=0, release=1200),
        form("Orbit 1", "atom", kontur=2, transpose=12, harmonics=10, attack=1, decay=1400, sustain=0, release=900,
             level=0.4, pan=-0.5),
        form("Orbit 2", "atom", kontur=3, transpose=19, harmonics=10, attack=1, decay=900, sustain=0, release=600,
             level=0.3, pan=0.5, detune=8, voices=2),
        form("Orbit 3", "atom", kontur=4, transpose=24, harmonics=10, attack=1, decay=500, sustain=0, release=400,
             level=0.2)),
    # --- Ungewoehnlich
    "shape_cat": instrument(
        "Cat", 0.75,
        # ein Miau: die Tonhoehe kommt von unten, das Vibrato setzt spaet ein, die Ohren machen es hell;
        # der Anschlag hat 32 Harmonics, danach schliesst es auf 12 wie ein Mund
        form("Meow", "cat", harmonics=12, huelle=20, pitch=(-5, 110), attack=40, decay=500, sustain=0.6, release=260,
             vibratoRate=6.5, vibratoDepth=25, vibratoDelay=220)),
}

def main():
    from instrument_katalog import ergaenze, genres_aus_loops
    genres = genres_aus_loops()
    for name, daten in INSTRUMENTE.items():
        (HIER / "instruments" / (name + ".lvinst")).write_text(als_text(ergaenze(name, daten, genres)),
                                                               encoding="utf-8", newline="\n")
    print(len(INSTRUMENTE), "Instrumente aus Formen")


if __name__ == "__main__":
    main()
