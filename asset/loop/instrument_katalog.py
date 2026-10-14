"""Art, Genres und Beschreibung der mitgelieferten Instrumente (S114, Stufe 14;
Entscheid Patrik F3 b: drei Felder in der `.lvinst`, Fassung 3).

    python asset/loop/instrument_katalog.py

Schreibt jede `asset/loop/instruments/*.lvinst` neu: `category`, `genres` und
`description` dazu, `version` auf 3. Eine Datei der Fassung 1 (flach, vierzehn
aus S110) wird dabei zu einer mit einer Schicht — dieselben Werte, `level`
wandert in die Schicht (so liest das Programm Fassung 1 auch). Kein Zufall:
zwei Laeufe ergeben dieselben Dateien.

- **Art** und **Beschreibung** stehen unten in `KATALOG`. Die Art ist nach dem
  Titel und dem Aufbau zugeordnet, nicht nach Klang; die Beschreibung sagt,
  woraus das Instrument gebaut ist, nicht wie es klingt — abgehoert hat es
  niemand.
- **Genres** werden gezaehlt, nicht getippt: ein Instrument traegt das Genre
  jedes mitgelieferten Loops (`*_demo.lvloop`, `genres/*.lvloop`), in dem es
  steht. Steht es in keinem, traegt es keins.

`make_examples.py` und `make_genres.py` rufen `ergaenze` vor dem Schreiben,
damit ein neuer Lauf die Felder behaelt. Wer ein Instrument dazulegt, traegt
es hier ein — `main` bricht ab, wenn eine Datei im Katalog fehlt.
"""

import json
from pathlib import Path

HIER = Path(__file__).resolve().parent

ARTEN = ("keys", "bass", "lead", "pads", "plucked", "wind", "percussion", "drums", "unusual")

# Name der Datei -> (Art, Beschreibung)
KATALOG = {
    # --- Keys
    "piano": ("keys", "A sine with five overtones that fades over a long decay."),
    "house_piano": ("keys", "A bright sine with seven overtones and no sustain, for short chords."),
    "tine_piano": ("keys", "A soft, lightly driven sine body with a short tine two octaves above."),
    "organ": ("keys", "A sine with five overtones that holds at full level while the key is down."),
    "house_organ": ("keys", "A sine with four overtones and a quick attack, for stabbed chords."),
    "clavinet": ("keys", "A driven plucked string with a short square bark on top."),
    "chord_stab": ("keys", "Three detuned saws through a filter that snaps shut."),
    # --- Bass
    "bass": ("bass", "A filtered saw with a short filter sweep."),
    "acid_bass": ("bass", "A saw through a low filter with a strong sweep."),
    "sub_bass": ("bass", "An almost pure sine that holds."),
    "growl_bass": ("bass", "A heavily driven saw and square with vibrato over a sine sub."),
    "rolling_bass": ("bass", "A short, driven saw with a sine sub, made for fast repeated notes."),
    "slap_bass": ("bass", "A plucked string with a thump below and a short pop an octave above."),
    "bass_808": ("bass", "A driven sine with a long decay and a short click."),
    "reese_bass": ("bass", "Three widely detuned saws over a sine sub."),
    "round_bass": ("bass", "A soft sine with two overtones and a short finger noise."),
    "upright_bass": ("bass", "A plucked string with a woody sine body."),
    # --- Lead
    "lead": ("lead", "A bright square with vibrato."),
    "trance_lead": ("lead", "Detuned saws in two octaves with a driven square for bite."),
    "super_saw": ("lead", "Nine saws in three layers, wide, tight and an octave up."),
    "hoover": ("lead", "Widely detuned, driven saws in two octaves with vibrato and a square edge."),
    # --- Pads and strings
    "strings": ("pads", "Three detuned saws with a slow attack and light vibrato."),
    "violin": ("pads", "A single bright saw with a soft attack and strong vibrato."),
    "warm_pad": ("pads", "Three detuned saws through a low filter, very slow in and out."),
    "fifth_pad": ("pads", "Detuned saws on the note and a fifth above, over a sine an octave below."),
    # --- Plucked
    "guitar": ("plucked", "A single plucked string."),
    "clean_guitar": ("plucked", "A plucked string with a quieter one an octave above."),
    "muted_guitar": ("plucked", "A heavily damped plucked string with a short thud."),
    "nylon_guitar": ("plucked", "A plucked string with a warm triangle underneath."),
    "overdrive_guitar": ("plucked", "Three driven plucked strings: the note, a fifth and an octave."),
    "pluck": ("plucked", "A square through a filter that snaps shut, without sustain."),
    "trance_pluck": ("plucked", "Detuned saws in two octaves through a snapping filter, over a short sine."),
    # --- Wind
    "brass": ("wind", "Two detuned saws with a filter that opens on the attack, and a square edge."),
    "flute": ("wind", "A triangle with vibrato, a breathy sine an octave above and a short chiff."),
    "sax": ("wind", "A lightly driven square with vibrato and a breathy saw."),
    # --- Percussion
    "bell": ("percussion", "A long sine strike with detuned partials a twelfth and two octaves above."),
    "kalimba": ("percussion", "A short sine tine with a knock and a detuned ring two octaves above."),
    "marimba": ("percussion", "A short sine bar with a mallet click two octaves above."),
    "steel_drum": ("percussion", "A sine with strong overtones, a detuned ring an octave above and a short hit."),
    "vibraphone": ("percussion", "A long sine bar with slow vibrato and a short strike."),
    "clave": ("percussion", "A very short sine with a tick an octave above; the note sets the pitch of the hit."),
    "conga": ("percussion", "A short sine skin with a damped string slap; the note sets the pitch of the hit."),
    "cowbell": ("percussion", "Two short squares a fifth apart; the note sets the pitch of the hit."),
    # --- Drums (S117, Stufe 11f: `make_drums.py`; alle fuer die Note C2 gebaut)
    "drum_kick": ("drums", "A driven sine that drops almost two octaves, with a short noise click. Built for the note C2."),
    "drum_tom": ("drums", "A sine that drops ten semitones, with a short noise click. Built for the note C2."),
    "drum_snare": ("drums", "Two short sines, one with a strong overtone, under high-passed noise. Built for the note C2."),
    "drum_clap": ("drums", "Four layers of driven, band-passed noise, three short hits and a tail. Built for the note C2."),
    "drum_hat_closed": ("drums", "A very short burst of high-passed noise. Built for the note C2."),
    "drum_hat_open": ("drums", "High-passed noise that rings for half a second. Built for the note C2."),
    # --- Drums, S123 (gerechnet in `make_drums.py`, aus Samples in `make_sample_instruments.py`)
    "drum_kick_808": ("drums", "A long, driven sine that drops an octave quickly, with a short click. Built for the note C2."),
    "drum_tom_low": ("drums", "A low sine that drops eight semitones, with a short click. Built for the note C2."),
    "drum_tom_high": ("drums", "A higher sine that drops eight semitones, with a short click. Built for the note C2."),
    "drum_shaker": ("drums", "Two short bursts of high-passed noise, the second 25 ms later. Built for the note C2."),
    "drum_ride": ("drums", "Long high-passed noise with two high-passed squares as a bell. Built for the note C2."),
    "drum_crash": ("drums", "Two layers of high-passed noise that ring for almost two seconds. Built for the note C2."),
    "drum_kick_punch": ("drums", "The 909-style kick sample over a falling sine sub. Built for the note C2."),
    "drum_snare_fat": ("drums", "The 909-style snare sample with a 707-style clap and a noise tail. Built for the note C2."),
    "drum_clap_stack": ("drums", "Two 707-style claps 12 ms apart, left and right, with a finger snap. Built for the note C2."),
    "drum_hat_shake": ("drums", "The 808-style closed hat with a quiet tambourine on the right. Built for the note C2."),
    "drum_beatbox_kick": ("drums", "A beatbox kick over a falling sine sub. Built for the note C2."),
    # --- Unusual
    "alien_choir": ("unusual", "Slow, detuned sines and triangles with deep vibrato in three registers."),
    "cascade": ("unusual", "Four sines that enter one after the other, each higher than the last."),
    "siren": ("unusual", "A square and detuned saws with the deepest vibrato there is."),
    "broken_radio": ("unusual", "A fully driven square with wild vibrato, a driven string and a hum."),
    "ghost": ("unusual", "Widely detuned sines and triangles that take a second to arrive and longer to leave."),
    "reverse_swell": ("unusual", "Detuned saws that swell for a second and then stop at once."),
    # --- Aus mitgelieferten Samples (S123: `make_sample_instruments.py`; die Samples unter samples/, CC BY 3.0)
    "female_choir": ("pads", "Two recorded female voices, aah and oo, held in a loop, left and right."),
    "hum_pad": ("pads", "A recorded female hum with an uh an octave below, slow in and out, over a soft sine."),
    "vocal_lead": ("lead", "A recorded female voice singing ha, held in a loop, with a quiet filtered saw."),
    "vocal_chop": ("lead", "A short recorded female ah with an aah an octave above, for chopped vocal stabs."),
    "harp": ("plucked", "A recorded harp note with a short sine body."),
    "koto": ("plucked", "A recorded koto note that rings for three seconds."),
    "dulcimer": ("plucked", "A recorded hammered dulcimer note with its ringing strings."),
    "thumb_piano": ("percussion", "A recorded thumb piano note with a short tick two octaves above."),
    "shakuhachi": ("wind", "A recorded shakuhachi note, held in a loop, with a breath of band-passed noise."),
    "alto_sax": ("wind", "A recorded alto saxophone note with its own vibrato, held in a loop."),
    "alto_flute": ("wind", "A recorded alto flute note with its own vibrato, held in a loop."),
    "angel_harp": ("plucked", "The recorded harp with a female oo choir that swells in an octave above."),
    "wind_voices": ("pads", "The recorded alto flute and a female hum, held in loops, slow, left and right."),
    # --- Aus Formen (S123: `make_shapes.py`, Fassung 9; die Formen aus asset/contours)
    "shape_heart_pad": ("pads", "Three detuned voices of the heart shape with a slow swell, over a sine an octave below."),
    "shape_rings_choir": ("pads", "The five rings as one path, stereo XY, with deep vibrato and a soft octave above."),
    "shape_infinity_drone": ("unusual", "The figure eight in stereo XY, very slow in and out, with a fifth turned by 90 degrees."),
    "shape_gear_bass": ("bass", "The gear shape, driven; the attack opens its teeth, then it settles to a round tone over a sine."),
    "shape_moon_bass": ("bass", "The crescent moon with few harmonics, warm, over a sine an octave below."),
    "shape_star_lead": ("lead", "The star with forty harmonics, driven, with vibrato and a soft octave above."),
    "shape_bolt_pluck": ("plucked", "The bolt with all its corners on the attack, closing quickly like a filter."),
    "shape_flower_flute": ("wind", "The flower with few harmonics and vibrato, with a breath of band-passed noise."),
    "shape_smiley_keys": ("keys", "One layer per contour of the smiley: face on the note, eyes and mouth above it."),
    "shape_atom_bell": ("percussion", "The core and the three orbits of the atom as four layers, a bell with partials an octave, a twelfth and two octaves up."),
    "shape_cat": ("unusual", "The cat shape sliding up into the note, bright at first, with late vibrato: a meow."),
}

_KOPF = ("format", "version", "title", "category", "genres", "description", "level", "layers")


def genres_aus_loops():
    """Name eines Instruments -> sortierte Liste der Genres der Loops, in denen es steht."""
    je_instrument = {}
    dateien = sorted(HIER.glob("*_demo.lvloop")) + sorted((HIER / "genres").glob("*.lvloop"))
    for datei in dateien:
        daten = json.loads(datei.read_text(encoding="utf-8"))
        genre = daten.get("tags", {}).get("genre", "").strip()
        if not genre:
            continue
        for muster in daten.get("patterns", []):
            name = muster.get("instrument")
            if name:
                je_instrument.setdefault(name, set()).add(genre)
    return {name: sorted(genres) for name, genres in je_instrument.items()}


def ergaenze(name, daten, genres=None):
    """`daten` (eine `.lvinst` als dict, Fassung 1, 2 oder 3) als Fassung 3 mit Art, Genres, Beschreibung."""
    if name not in KATALOG:
        raise KeyError(f"{name}: fehlt in KATALOG (asset/loop/instrument_katalog.py)")
    art, beschreibung = KATALOG[name]
    if art not in ARTEN:
        raise ValueError(f"{name}: unbekannte Art {art!r}")
    if genres is None:
        genres = genres_aus_loops()
    if "layers" in daten:
        level, schichten = daten.get("level", 1.0), daten["layers"]
    else:
        # Fassung 1 ist flach: eine Schicht, `level` ist ihre Lautstaerke
        level, schichten = 1.0, [{k: v for k, v in daten.items() if k not in _KOPF or k == "level"}]
    return {
        # mindestens 3; eine Datei mit Feldern einer spaeteren Fassung (Drums: 6) behaelt ihre Zahl
        "format": "lumiviz-instrument", "version": max(3, int(daten.get("version", 1))), "title": daten["title"],
        "category": art, "genres": genres.get(name, []), "description": beschreibung,
        "level": level, "layers": schichten,
    }


def main():
    from make_examples import als_text

    genres = genres_aus_loops()
    dateien = sorted((HIER / "instruments").glob("*.lvinst"))
    fehlen = [d.stem for d in dateien if d.stem not in KATALOG]
    if fehlen:
        raise SystemExit("Nicht im KATALOG: " + ", ".join(fehlen))
    verwaist = sorted(set(KATALOG) - {d.stem for d in dateien})
    if verwaist:
        raise SystemExit("Im KATALOG, aber ohne Datei: " + ", ".join(verwaist))
    for datei in dateien:
        alt = json.loads(datei.read_text(encoding="utf-8"))
        datei.write_text(als_text(ergaenze(datei.stem, alt, genres)), encoding="utf-8", newline="\n")
    ohne = [d.stem for d in dateien if not genres.get(d.stem)]
    print(len(dateien), "Instrumente;", "ohne Genre:", ", ".join(ohne) if ohne else "keins")


if __name__ == "__main__":
    main()
