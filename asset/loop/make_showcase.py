"""Erzeugt Beispiel-Loops fuer das, was in S123 dazukam: Instrumente aus Formen, aus
mitgelieferten Samples (`lib:`), die neuen Drums (Wunsch Patrik S123).

    python asset/loop/make_showcase.py

Schreibt nach `asset/loop/genres/*.lvloop` (zehn Loops; Brush Swing seit S124, Stufe 21d) — dort prueft der Test jede Datei
von selbst mit. Danach `python asset/loop/instrument_katalog.py`: die Genres der Instrumente
werden aus den Loops gezaehlt. Kein Zufall: zwei Laeufe ergeben dieselben Dateien.

Alles nach Rechnung gesetzt, nicht nach Gehoer — ob es so klingt, muss ein Mensch hoeren.
Drum-Spuren nennen ein mitgeliefertes Sample (`lib:<name>`), einen Klang des Kits oder ein
Instrument der Art Drums (Fassung 4: `instrument`, Note C2, Laenge 250 ms; `sound` bleibt
daneben stehen fuer ein aelteres Programm).
"""

from make_examples import als_text
from make_genres import HIER, Bau, lauf, schlag

# --- Akkorde als MIDI-Tupel
def dur(grund):
    return (grund, grund + 4, grund + 7)


def moll(grund):
    return (grund, grund + 3, grund + 7)


LEER = "................"


def drums(b, name, spuren):
    """Wie `Bau.drums`, dazu Spuren mit Instrument: (Name, Klang, Laut, Pan, Takte[, Instrument])."""
    b.drums(name, [s[:5] for s in spuren])
    muster = b.patterns[-1]
    for spur, eintrag in zip(muster["tracks"], spuren):
        if len(eintrag) > 5:
            spur.update({"instrument": eintrag[5], "note": 36, "length": 250})
    return name


def effekte(loop, je_kanal):
    """Fassung 5 und Effekte am Clip: je Kanal eine Reihe (`path`) fuer alle seine Clips."""
    loop["version"] = 5
    for kanal in loop["channels"]:
        reihe = je_kanal.get(kanal["name"])
        if reihe:
            for clip in kanal["clips"]:
                clip["path"] = reihe
    return loop


def hall(mix=0.25, groesse=0.6, sek=2.2):
    return {"type": "reverb", "on": True, "size": groesse, "decay": sek, "damping": 0.4, "mix": mix}


def echo(zeit="1/8d", mix=0.3, feedback=0.4):
    return {"type": "echo", "on": True, "time": zeit, "feedback": feedback, "damping": 0.4, "mix": mix}


# --- Stimmen ---------------------------------------------------------------

def vocal_house():
    b = Bau("Vocal House", "House", 124, 0.24)
    akk = [moll(57), dur(53), dur(48), dur(55)]  # Am F C G
    beat = drums(b, "Beat", [("Kick", "lib:kick_909", 1.0, 0.0, "x...x...x...x..."),
                             ("Clap", "lib:clap_707", 0.7, 0.0, "....x.......x..."),
                             ("Hi-Hat", "lib:hat_808_closed", 0.45, 0.25, "..x...x...x...x."),
                             ("Open Hat", "lib:hat_707_open", 0.3, -0.25, ["................"] * 3 + ["..............x."]),
                             ("Tambourine", "lib:tambourine", 0.3, 0.4, "x-o-x-o-x-o-x-o-")])
    bass = b.noten("Bass", "shape_moon_bass", b.takte(akk, lambda t, a: schlag((0, 3, 6, 10), 2, a[0] - 24, 0.9)), 0.85)
    chops = b.noten("Chops", "vocal_chop", b.takte(akk, lambda t, a: schlag((2, 7, 10), 1, a[2] + 12, 0.8)
                                                   + schlag((14,), 1, a[1] + 12, 0.7)), 0.6, 0.15)
    chor = b.noten("Choir", "female_choir", b.takte(akk, lambda t, a: [(0, 16, h, 0.6) for h in a]), 0.45)
    lead = b.noten("Lead", "vocal_lead", [(0, 6, 76, 0.8), (8, 4, 74, 0.7), (12, 4, 72, 0.7), (16, 12, 72, 0.8),
                                           (32, 6, 72, 0.8), (40, 4, 71, 0.7), (44, 4, 69, 0.7), (48, 14, 67, 0.8)], 0.5, -0.1)
    b.kanal("Drums", (beat, 0, 6))
    b.kanal("Bass", (bass, 4, 5))
    b.kanal("Chops", (chops, 4, 5))
    b.kanal("Choir", (chor, 0, 2), (chor, 16, 2))
    b.kanal("Lead", (lead, 12, 3))
    return effekte(b.fertig(), {"Chops": [echo("1/8d", 0.3)], "Choir": [hall(0.35)], "Lead": [hall(0.25)]})


def angel_steps():
    b = Bau("Angel Steps", "Chillout", 90, 0.31)
    akk = [dur(53), moll(57), dur(48), dur(55)]  # F Am C G
    beat = drums(b, "Beat", [("Kick", "lib:kick_deep", 0.9, 0.0, "x.........x....."),
                             ("Rim", "lib:rim", 0.5, 0.1, "....x.......x..."),
                             ("Snap", "lib:snap", 0.4, -0.2, ".......x.......x"),
                             ("Shaker", "kit:hat_closed", 0.35, 0.3, "x-o-x-o-x-o-x-o-", "drum_shaker")])
    harfe = b.noten("Harp", "angel_harp", b.takte(akk, lambda t, a: lauf(range(0, 16, 2), 2, (a[0], a[1], a[2], a[0] + 12,
                                                                                            a[2], a[1], a[0] + 12, a[1] + 12), 0.7)), 0.55)
    summen = b.noten("Hum", "hum_pad", b.takte(akk, lambda t, a: [(0, 16, a[0] - 12, 0.6), (0, 16, a[1], 0.5)]), 0.5)
    bass = b.noten("Bass", "round_bass", b.takte(akk, lambda t, a: [(0, 6, a[0] - 24, 0.8), (10, 4, a[0] - 17, 0.6)]), 0.8)
    b.kanal("Drums", (beat, 4, 4))
    b.kanal("Harp", (harfe, 0, 5))
    b.kanal("Hum", (summen, 0, 5))
    b.kanal("Bass", (bass, 4, 4))
    return effekte(b.fertig(), {"Harp": [echo("1/4", 0.25, 0.35), hall(0.3)], "Hum": [hall(0.3)]})


def beatbox_groove():
    b = Bau("Beatbox Groove", "Hip-Hop", 92, 0.26, swing=0.12)
    akk = [moll(50), moll(50), dur(46), dur(45)]  # Dm Dm Bb A
    beat = drums(b, "Beat", [("Kick", "lib:beatbox_kick", 1.0, 0.0, "x.....x...x....."),
                             ("Sub", "kit:kick", 0.5, 0.0, "x.........x.....", "drum_beatbox_kick"),
                             ("Snare", "lib:beatbox_snare", 0.9, 0.0, "....x.......x..."),
                             ("Hat", "lib:beatbox_hat_closed", 0.5, 0.2, "x.x.x.x.x.x.xx.x"),
                             ("Open Hat", "lib:beatbox_hat_open", 0.4, -0.2, ["................"] * 3 + ["..............x."]),
                             ("Hey", "lib:voice_male_hey", 0.5, 0.0, ["................"] * 3 + ["............x..."])])
    bass = b.noten("Bass", "shape_moon_bass", b.takte(akk, lambda t, a: [(0, 4, a[0] - 12, 0.9), (6, 2, a[0] - 12, 0.7),
                                                                         (10, 4, a[0], 0.8)]), 0.85)
    keys = b.noten("Keys", "shape_smiley_keys", b.takte(akk, lambda t, a: schlag((0, 6), 4, a, 0.6)), 0.5, 0.2)
    hook = b.noten("Hook", "vocal_chop", [(4, 2, 69, 0.8), (6, 2, 67, 0.7), (8, 4, 65, 0.8), (36, 2, 69, 0.8),
                                          (38, 2, 70, 0.8), (40, 4, 69, 0.8)], 0.55, -0.15)
    b.kanal("Drums", (beat, 0, 5))
    b.kanal("Bass", (bass, 4, 4))
    b.kanal("Keys", (keys, 4, 4))
    b.kanal("Hook", (hook, 8, 3))
    return effekte(b.fertig(), {"Keys": [hall(0.2)], "Hook": [echo("1/8", 0.3)]})


# --- Welt ------------------------------------------------------------------

def desert_caravan():
    b = Bau("Desert Caravan", "World", 100, 0.26)
    # D-Hijaz: D Eb F# G A Bb C
    leiter = (62, 63, 66, 67, 69, 70, 72, 74)
    beat = drums(b, "Beat", [("Doum", "lib:darbuka_doom", 1.0, 0.0, "x.....x...x....."),
                             ("Tak", "lib:darbuka_tak", 0.7, 0.15, "...x....x..x.x.."),
                             ("Djembe", "lib:djembe_slap", 0.5, -0.25, "..x...x.......x."),
                             ("Tambourine", "lib:tambourine", 0.4, 0.35, "x.o.x.o.x.o.x.oo")])
    drone = b.noten("Drone", "dulcimer", [(s, 8, h, 0.7) for s in range(0, 64, 8) for h in (38, 50)], 0.5)
    folge = (0, 1, 2, 1, 3, 4, 3, 2, 4, 5, 6, 5, 4, 3, 2, 1)
    melodie = b.noten("Flute", "shakuhachi", [(s * 4, 4, leiter[folge[s]], 0.8) for s in range(16)], 0.55, 0.1)
    koto = b.noten("Koto", "koto", [(s, 2, leiter[(s // 2 * 3) % 8] - 12, 0.7) for s in range(0, 62, 3)], 0.5, -0.2)
    b.kanal("Drums", (beat, 0, 5))
    b.kanal("Drone", (drone, 0, 5))
    b.kanal("Koto", (koto, 4, 4))
    b.kanal("Flute", (melodie, 8, 3))
    return effekte(b.fertig(), {"Flute": [hall(0.3, 0.7)], "Koto": [echo("1/8d", 0.2)]})


def zen_garden():
    b = Bau("Zen Garden", "Ambient", 70, 0.62)
    penta = (62, 64, 67, 69, 71, 74, 76)  # D-Dur-Pentatonik
    beat = drums(b, "Beat", [("Rim", "lib:rim", 0.35, 0.3, "x.......x......."),
                             ("Tom", "kit:tom", 0.4, -0.3, ["................", "............x...", "................",
                                                           "........x...x..."], "drum_tom_low")])
    koto = b.noten("Koto", "koto", [(0, 4, 62, 0.8), (6, 2, 69, 0.6), (8, 6, 67, 0.7), (16, 4, 64, 0.7), (22, 2, 71, 0.6),
                                    (24, 8, 69, 0.7), (32, 4, 74, 0.8), (38, 2, 71, 0.6), (40, 6, 69, 0.7), (48, 4, 67, 0.7),
                                    (52, 4, 64, 0.6), (56, 8, 62, 0.7)], 0.55, -0.15)
    wind = b.noten("Wind", "wind_voices", [(0, 32, 50, 0.6), (0, 32, 57, 0.5), (32, 32, 55, 0.6), (32, 32, 62, 0.5)], 0.45)
    tropfen = b.noten("Drops", "thumb_piano", [(s, 2, penta[(s * 5) % 7] + 12, 0.6) for s in (3, 11, 19, 27, 35, 43, 51, 59)],
                      0.45, 0.3)
    flute = b.noten("Shakuhachi", "shakuhachi", [(8, 12, 74, 0.7), (24, 6, 71, 0.6), (40, 16, 69, 0.7)], 0.5, 0.1)
    b.kanal("Wind", (wind, 0, 6))
    b.kanal("Koto", (koto, 4, 4))
    b.kanal("Drops", (tropfen, 0, 6))
    b.kanal("Shakuhachi", (flute, 12, 3))
    b.kanal("Drums", (beat, 8, 3))
    return effekte(b.fertig(), {"Koto": [hall(0.35, 0.8, 3.0)], "Drops": [echo("1/4", 0.35, 0.5), hall(0.3)],
                                "Shakuhachi": [hall(0.4, 0.8, 3.5)], "Wind": [hall(0.3, 0.8, 3.0)]})


def flute_tropics():
    b = Bau("Flute Tropics", "Afrobeat", 108, 0.25)
    akk = [dur(48), moll(57), dur(53), dur(55)]  # C Am F G
    beat = drums(b, "Beat", [("Kick", "lib:kick_909", 0.9, 0.0, "x.....x...x....."),
                             ("Djembe", "lib:djembe_low", 0.7, -0.2, "...x......x..x.."),
                             ("Slap", "lib:djembe_slap", 0.6, 0.2, "....x..x....x..."),
                             ("Conga", "lib:conga_high", 0.5, 0.35, "..x...x.x...x.x."),
                             ("Shaker", "kit:hat_closed", 0.4, -0.35, "xoxoxoxoxoxoxoxo", "drum_shaker")])
    bass = b.noten("Bass", "bass_808", b.takte(akk, lambda t, a: [(0, 3, a[0] - 24, 0.9), (6, 2, a[0] - 24, 0.7),
                                                                   (10, 3, a[2] - 24, 0.8)]), 0.8)
    daumen = b.noten("Thumb", "thumb_piano", b.takte(akk, lambda t, a: lauf((0, 3, 6, 8, 11, 14), 2, (a[0] + 12, a[2] + 12,
                                                                                                    a[1] + 12, a[2] + 12), 0.7)), 0.5, 0.2)
    flute = b.noten("Flute", "alto_flute", [(0, 4, 76, 0.8), (4, 2, 74, 0.7), (6, 2, 72, 0.7), (8, 8, 69, 0.8),
                                            (16, 4, 72, 0.8), (20, 4, 74, 0.7), (24, 8, 76, 0.8), (32, 4, 77, 0.8),
                                            (36, 4, 76, 0.7), (40, 8, 72, 0.8), (48, 6, 74, 0.8), (54, 2, 72, 0.7), (56, 8, 71, 0.8)],
                    0.55, -0.1)
    stahl = b.noten("Steel", "steel_drum", b.takte(akk, lambda t, a: schlag((2, 10), 2, a, 0.6)), 0.4, 0.3)
    b.kanal("Drums", (beat, 0, 6))
    b.kanal("Bass", (bass, 4, 5))
    b.kanal("Thumb", (daumen, 0, 6))
    b.kanal("Steel", (stahl, 8, 4))
    b.kanal("Flute", (flute, 12, 3))
    return effekte(b.fertig(), {"Flute": [hall(0.25)], "Thumb": [echo("1/8d", 0.2)]})


def sax_after_dark():
    b = Bau("Sax After Dark", "Jazz", 84, 0.32, swing=0.2)
    akk = [(50, 53, 57, 60), (55, 59, 62, 65), (48, 52, 55, 59), (48, 52, 55, 59)]  # Dm7 G7 Cmaj7 Cmaj7
    beat = drums(b, "Beat", [("Kick", "lib:kick_deep", 0.7, 0.0, "x.........x....."),
                             ("Rim", "lib:rim", 0.5, 0.1, "....x.......x..."),
                             ("Ride", "kit:hat_open", 0.45, 0.3, "x..x.xx..x.xx..x", "drum_ride")])
    bass = b.noten("Bass", "upright_bass", b.takte(akk, lambda t, a: lauf((0, 4, 8, 12), 4, (a[0] - 12, a[1] - 12, a[2] - 12,
                                                                                            a[1] - 12), 0.8)), 0.85)
    harfe = b.noten("Harp", "harp", b.takte(akk, lambda t, a: schlag((0,), 6, a, 0.55) + schlag((10,), 4, a[1:], 0.45)), 0.45, 0.2)
    sax = b.noten("Sax", "alto_sax", [(0, 6, 72, 0.8), (6, 2, 69, 0.7), (8, 4, 67, 0.7), (12, 4, 65, 0.7), (16, 10, 67, 0.8),
                                      (28, 4, 71, 0.7), (32, 6, 72, 0.8), (38, 2, 74, 0.7), (40, 8, 76, 0.8), (48, 12, 72, 0.8)],
                  0.55, -0.15)
    b.kanal("Drums", (beat, 0, 5))
    b.kanal("Bass", (bass, 0, 5))
    b.kanal("Harp", (harfe, 4, 4))
    b.kanal("Sax", (sax, 8, 3))
    return effekte(b.fertig(), {"Sax": [hall(0.3, 0.6, 2.0)], "Harp": [hall(0.25)]})


def brush_swing():
    """S124, Stufe 21d (K6c a): das Kit Jazz Brushes. Swing in Triolen: zwoelf Schritte je Takt.
    Ride „ding, ding-a", Fuss-Hi-Hat auf 2 und 4, der Besen kreist auf 1 und 3, Taps und im
    vierten Takt ein Slap als Akzent, die Kick gefedert."""
    b = Bau("Brush Swing", "Jazz", 132, 0.4, schlaege=4, je_schlag=3)
    akk = [(50, 53, 57, 60), (55, 59, 62, 65), (48, 52, 55, 59), (57, 61, 64, 67)]  # Dm7 G7 Cmaj7 A7
    t = "...o.-...o.-"
    beat = drums(b, "Brushes", [
        ("Kick", "lib:drs_kick_soft", 0.5, 0.0, "-..-..-..-.."),
        ("Brush", "kit:snare", 0.7, 0.0, [t, t, "...o.-..-o.-", t], "drum_brush_tap"),
        ("Accent", "kit:snare", 0.8, 0.0, ["............", "............", "............", ".........x.."], "drum_brush_slap"),
        ("Sweep", "kit:hat_open", 0.6, 0.0, "x.....o.....", "drum_brush_sweep"),
        ("Hat", "lib:drs_hat_foot", 0.7, 0.0, "...x.....x.."),
        ("Ride", "lib:drs_ride_soft", 0.6, 0.0, "x..o.-x..o.-")])
    muster = b.patterns[-1]
    muster["kit"] = "jazz_brushes"  # Entscheid K4 a: das Kit, nach dem die Spuren gesetzt sind
    for spur, rolle in zip(muster["tracks"], ("kick", "snare", "snare 2", "perc 1", "hat", "ride")):
        spur["role"] = rolle
    muster["tracks"][3]["length"] = 500  # der Besen kreist einen halben Takt, wie im Kit
    bass = b.noten("Bass", "upright_bass", b.takte(akk, lambda t, a: lauf((0, 3, 6, 9), 3, (a[0] - 12, a[1] - 12, a[2] - 12,
                                                                                          a[3] - 24), 0.8)), 0.85)
    b.kanal("Drums", (beat, 0, 4))
    b.kanal("Bass", (bass, 4, 3))
    return b.fertig()


# --- Formen ----------------------------------------------------------------

def shape_synthwave():
    b = Bau("Shape Synthwave", "Synthwave", 110, 0.24)
    akk = [moll(57), dur(53), dur(48), dur(55)]  # Am F C G
    beat = drums(b, "Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x...", "drum_kick_808"),
                             ("Snare", "kit:snare", 0.8, 0.0, "....x.......x...", "drum_snare_fat"),
                             ("Hat", "kit:hat_closed", 0.4, 0.25, "x-x-x-x-x-x-x-x-", "drum_hat_shake"),
                             ("Crash", "kit:hat_open", 0.4, -0.2, ["x...............", LEER, LEER, LEER], "drum_crash")])
    bass = b.noten("Bass", "shape_gear_bass", b.takte(akk, lambda t, a: schlag(range(0, 16, 2), 2, a[0] - 24, 0.85)), 0.8)
    pad = b.noten("Pad", "shape_heart_pad", b.takte(akk, lambda t, a: [(0, 16, h, 0.6) for h in a]), 0.45)
    arp = b.noten("Arp", "shape_bolt_pluck", b.takte(akk, lambda t, a: lauf(range(16), 1, (a[0] + 12, a[1] + 12, a[2] + 12,
                                                                                        a[1] + 12), 0.7, (0, 8))), 0.45, 0.25)
    lead = b.noten("Lead", "shape_star_lead", [(0, 8, 76, 0.85), (8, 4, 77, 0.8), (12, 4, 76, 0.8), (16, 8, 72, 0.85),
                                               (24, 8, 74, 0.8), (32, 12, 76, 0.85), (44, 4, 79, 0.8), (48, 16, 74, 0.85)], 0.5, -0.1)
    b.kanal("Drums", (beat, 4, 5))
    b.kanal("Bass", (bass, 4, 5))
    b.kanal("Pad", (pad, 0, 6))
    b.kanal("Arp", (arp, 8, 4))
    b.kanal("Lead", (lead, 12, 3))
    return effekte(b.fertig(), {"Arp": [echo("1/8d", 0.3)], "Lead": [hall(0.25), echo("1/4", 0.15)], "Pad": [hall(0.25)]})


def geometry():
    b = Bau("Geometry", "Electronica", 122, 0.24)
    akk = [moll(52), dur(48), dur(55), dur(50)]  # Em C G D
    beat = drums(b, "Beat", [("Kick", "kit:kick", 1.0, 0.0, "x...x...x...x...", "drum_kick_punch"),
                             ("Clap", "kit:clap", 0.7, 0.0, "....x.......x...", "drum_clap_stack"),
                             ("Ride", "kit:hat_open", 0.35, 0.3, "..x...x...x...x.", "drum_ride"),
                             ("Toms", "kit:tom", 0.6, -0.25, [LEER, LEER, LEER, "........x.x.x.x."], "drum_tom_high")])
    tief = b.noten("Low toms", "drum_tom_low", [(60, 2, 36, 0.8), (62, 2, 36, 0.9)], 0.6, 0.25)
    glocke = b.noten("Bell", "shape_atom_bell", b.takte(akk, lambda t, a: lauf((0, 6, 10), 4, (a[2] + 12, a[1] + 12, a[0] + 12), 0.7)),
                     0.45, 0.2)
    keys = b.noten("Keys", "shape_smiley_keys", b.takte(akk, lambda t, a: schlag((2, 6, 10, 14), 2, a, 0.6)), 0.45, -0.2)
    chor = b.noten("Rings", "shape_rings_choir", b.takte(akk, lambda t, a: [(0, 16, a[0], 0.6), (0, 16, a[2], 0.5)]), 0.45)
    drone = b.noten("Drone", "shape_infinity_drone", [(0, 64, 40, 0.6)], 0.4)
    katze = b.noten("Cat", "shape_cat", [(28, 4, 72, 0.7), (60, 4, 74, 0.7)], 0.4, 0.3)
    bass = b.noten("Bass", "shape_gear_bass", b.takte(akk, lambda t, a: schlag((0, 3, 8, 11), 2, a[0] - 24, 0.85)), 0.8)
    b.kanal("Drone", (drone, 0, 7))
    b.kanal("Rings", (chor, 0, 7))
    b.kanal("Bell", (glocke, 4, 6))
    b.kanal("Drums", (beat, 8, 5))
    b.kanal("Low toms", (tief, 8, 5))
    b.kanal("Bass", (bass, 8, 5))
    b.kanal("Keys", (keys, 12, 4))
    b.kanal("Cat", (katze, 16, 3))
    return effekte(b.fertig(), {"Bell": [echo("1/8d", 0.3), hall(0.3)], "Rings": [hall(0.3)], "Cat": [hall(0.3)],
                                "Drone": [hall(0.25)]})


LOOPS = {
    "vocal_house": vocal_house,
    "angel_steps": angel_steps,
    "beatbox_groove": beatbox_groove,
    "desert_caravan": desert_caravan,
    "zen_garden": zen_garden,
    "flute_tropics": flute_tropics,
    "sax_after_dark": sax_after_dark,
    "shape_synthwave": shape_synthwave,
    "geometry": geometry,
    "brush_swing": brush_swing,
}


def main():
    for name, bau in LOOPS.items():
        loop = bau()
        (HIER / "genres" / (name + ".lvloop")).write_text(als_text(loop), encoding="utf-8", newline="\n")
        takte = max(c["start"] + 4 * c["repeat"] for k in loop["channels"] for c in k["clips"])
        print(f"{name}.lvloop: {len(loop['patterns'])} Muster, {takte} Takte")
    # die Genres der Instrumente werden aus den Loops gezaehlt
    from instrument_katalog import main as katalog
    katalog()


if __name__ == "__main__":
    main()
