"""Gitarren-Proben zu den Spielweisen aus Stufe 24 und 25 (S136, Wunsch Patrik:
Gitarrenstuecke zum Testen, im Stil bekannter Songs).

    python asset/loop/proben/make_gitarren_proben.py

**Stiluebungen, keine Nachbauten.** Jeder Loop folgt nur dem, was einen Stil
ausmacht — freie Akkordfolgen (I–bVII–IV, Zwoelftakt-Ausschnitt, Moll-Pentatonik),
uebliche Spielmuster (Boogie-Shuffle, Travis-Picking, Sechzehntel-Scratch,
Palm-mute-Chug) und eigene Melodien und Licks. Kein geschuetztes Riff, keine
Melodie und kein Solo eines Songs ist Note fuer Note nachgebaut; die Titel
beschreiben den Stil und nennen keinen Song.

Schreibt sechs `.lvloop` nach `asset/loop/proben/` — Testmaterial, **nicht
mitgeliefert**: der Ordner steht nicht in `asset/Asset.cmake` und soll dort auch
nicht hin (darum auch kein `*_demo` und nichts unter `genres/` oder `vergleich/`).

- `southern_rock_d` — Southern Rock in D: D | C G, Akkorde mit Let ring, ein
  Lick mit Slide, Hammer-on, Bend und Release, Pull-off.
- `folk_walzer_c` — Folk Waltz in C (3/4): Grundton, Hammer-on in den Akkord,
  Strum Down/Up, ein breiter gebrochener Akkord am Schluss.
- `blues_shuffle_e` — Shuffle Blues in E (12/8-Gefuehl): Boogie mit Palm mute
  nur in Downstrokes, Lead mit Ganzton-Bend, Viertelton-Curl, Bend und Release,
  Pull-off, Slide und Vibrato als Bend-Kurve.
- `funk_scratch_e9` — Funk Scratch in E9: Sechzehntel mit Akzenten und
  gedaempften Ghosts, Slide von D#9 nach E9; Slap Bass mit Pop, Pull-off,
  Hammer-on und Slide.
- `travis_picking_c` — Travis Picking in C: Wechselbass mit Palm mute,
  Finger mit Let ring, Hammer-on im Bass, Slide in der Melodie.
- `hard_rock_a` — Hard Rock Riff in A: Palm-mute-Chug, Powerchord-Slides,
  ein getippter Lauf mit Pull-offs und Hammer-ons, Bend mit Vibrato.

Notenzeile: `[schritt, laenge, hoehe, laut, saite, artikulationen, bend]`; Bits
Palm mute 1, Let ring 2, Tapping 4, Harmonic 8, Legato 16, Slide 32 (Legato und
Slide an der Zielnote; die Note davor liegt auf derselben Saite und endet dort,
wo die Zielnote beginnt). Bends steigen in rund 90 ms (Recherche S136: 80–100 ms).
Legato, Slide und Bend klingen erst ab Stufe 25c — bis dahin schlagen die Noten
normal an.

Zwei Laeufe ergeben dieselben Dateien. Am Ende liest das Skript jede Datei
wieder und prueft sie (`pruefe`). Alles nach Rechnung gesetzt, nicht nach Gehoer.
"""

import json
import sys
from pathlib import Path

PROBEN = Path(__file__).resolve().parent
LOOP = PROBEN.parent
sys.path.insert(0, str(LOOP))

from make_examples import als_text  # noqa: E402
from make_genres import Bau  # noqa: E402
from make_saiten_instrumente import SPREAD_MS  # noqa: E402

# die Bits von `Note::artikulationen` (include/loop/Spielweise.hpp)
PALM_MUTE, LET_RING, TAPPING, HARMONIC, LEGATO, SLIDE = 1, 2, 4, 8, 16, 32
AUTO = -1
ANSTIEG_MS = 90.0  # Anstieg eines Bends


# --- Bausteine -----------------------------------------------------------------------------

def schritt_ms(b):
    _, _, bpm, _, _, je_schlag, _ = b.kopf
    return 60000.0 / bpm / je_schlag


def _anteil(b, laenge, ms):
    return ms / (laenge * schritt_ms(b))


def _r(werte):
    return [[round(z, 3), h] for z, h in werte]


def bend_auf(b, laenge, ziel, ab=0.0):
    """Biegt ab `ab` (Anteil der Note) in ANSTIEG_MS auf `ziel` Halbtoene und haelt."""
    f = _anteil(b, laenge, ANSTIEG_MS)
    return _r([(ab, 0.0), (min(1.0, ab + f), ziel), (1.0, ziel)])


def bend_und_zurueck(b, laenge, ziel, zurueck=0.5):
    """Biegt auf, haelt, laesst ab `zurueck` wieder los (Release)."""
    f = _anteil(b, laenge, ANSTIEG_MS)
    return _r([(0.0, 0.0), (f, ziel), (zurueck, ziel), (min(1.0, zurueck + f), 0.0), (1.0, 0.0)])


def release(b, laenge, ziel, ab=0.4):
    """Vorgebogen angeschlagen (Pre-bend), ab `ab` losgelassen."""
    f = _anteil(b, laenge, ANSTIEG_MS)
    return _r([(0.0, ziel), (ab, ziel), (min(1.0, ab + f), 0.0), (1.0, 0.0)])


def vibrato(b, laenge, ziel=0.0, tiefe=0.5, ab=0.3, schwingungen=5):
    """Biegt (wenn `ziel`) auf und schwingt ab `ab` zwischen ziel und ziel − tiefe (ohne Ziel: 0 und + tiefe)."""
    punkte = [(0.0, 0.0)]
    if ziel:
        punkte.append((_anteil(b, laenge, ANSTIEG_MS), ziel))
    ruhe, ausschlag = (ziel, ziel - tiefe) if ziel else (0.0, tiefe)
    schritt = (1.0 - ab) / (2 * schwingungen)
    for i in range(2 * schwingungen + 1):
        punkte.append((ab + i * schritt, ruhe if i % 2 == 0 else ausschlag))
    return _r(punkte)


def saiten_muster(b, name, inst, noten, laut=0.8, pan=0.0, raster="halfBeat", hand=None, anschlaege=()):
    """Ein Muster mit Saiteninstrument. `noten`: (Schritt, Laenge, Hoehe, Staerke[, Saite[, Bits[, Bend]]]);
    `hand`: Muster der Hand (`D`/`U` je Stelle) oder None (Alternate); `anschlaege`: (Schritt, Art, Spread)."""
    zeilen = []
    for n in noten:
        schritt, laenge, hoehe, staerke = n[:4]
        saite = n[4] if len(n) > 4 else AUTO
        art = n[5] if len(n) > 5 else 0
        bend = n[6] if len(n) > 6 else None
        zeile = [schritt, laenge, hoehe, staerke]
        if saite >= 0 or art or bend:
            zeile.append(saite)
        if art or bend:
            zeile.append(art)
        if bend:
            zeile.append(bend)
        zeilen.append(zeile)
    strum = {"spread": SPREAD_MS[inst], "hand": raster}
    if hand:
        strum["pattern"] = hand
    muster = {"name": name, "kind": "instrument", "bars": 4, "instrument": inst, "volume": laut, "pan": pan,
              "mute": False, "notes": sorted(zeilen, key=lambda z: (z[0], z[2])), "stringChoice": "perChord",
              "strum": strum}
    if anschlaege:
        muster["strokes"] = [dict({"step": s, "stroke": art}, **({"spread": sp} if sp is not None else {}))
                             for s, art, sp in anschlaege]
    b.patterns.append(muster)
    return name


def fertig(b):
    loop = b.fertig()
    loop["version"] = 10  # Legato, Slide und Bend seit Fassung 10
    return loop


# --- Southern Rock in D --------------------------------------------------------------------

def southern_rock():
    b = Bau("Southern Rock in D", "Southern Rock", 98, 0.35)
    D, C, G = (50, 57, 62, 66), (48, 52, 55, 60, 64), (43, 47, 50, 55, 59, 67)

    def halbtakt(t0, akk):
        # Strum auf der Eins, der Grundton gezupft, ein Up auf den oberen drei — alles klingt aus
        return ([(t0, 4, h, 0.85, AUTO, LET_RING) for h in akk] + [(t0 + 4, 2, akk[0], 0.7, AUTO, LET_RING)]
                + [(t0 + 6, 2, h, 0.6, AUTO, LET_RING) for h in akk[-3:]])

    rhythmus = []
    for takt, haelften in enumerate(((D, D), (C, G), (D, D), (C, G))):
        for i, akk in enumerate(haelften):
            rhythmus += halbtakt(16 * takt + 8 * i, akk)
    # Lead: im zweiten Takt eine kurze Antwort, im vierten ein Lick in D-Dur-Pentatonik
    G_, H_ = 3, 4  # G- und H-Saite
    lead = [(24, 2, 66, 0.8, H_), (26, 2, 67, 0.75, H_, LEGATO), (28, 4, 71, 0.85, H_, SLIDE),
            (48, 2, 62, 0.8, G_), (50, 2, 64, 0.8, G_, SLIDE), (52, 2, 66, 0.75, G_, LEGATO),
            (54, 4, 69, 0.9, H_, 0, bend_auf(b, 4, 2.0)),
            (58, 2, 69, 0.8, H_, 0, release(b, 2, 2.0, 0.3)),
            (60, 2, 66, 0.75, H_, LEGATO), (62, 2, 62, 0.8, G_)]
    bass = []
    for takt, folge in enumerate(((38, 38), (36, 31), (38, 38), (36, 31))):
        for i, g in enumerate(folge):
            t = 16 * takt + 8 * i
            bass += [(t, 3, g, 0.9), (t + 3, 1, g, 0.6), (t + 4, 2, g + 7, 0.75), (t + 6, 2, g, 0.7)]
    akkorde = saiten_muster(b, "Rhythm", "clean_guitar", rhythmus, 0.8, -0.25)
    solo = saiten_muster(b, "Lead", "overdrive_guitar", lead, 0.65, 0.25)
    tief = saiten_muster(b, "Bass", "finger_bass", bass, 0.85, 0.0)
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.85, 0.0, "x.......x.x....."),
                            ("Snare", "kit:snare", 0.75, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.45, 0.25, "x.o.x.o.x.o.x.o.")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Rhythm", (akkorde, 0, 2))
    b.kanal("Lead", (solo, 0, 2))
    b.kanal("Bass", (tief, 0, 2))
    return fertig(b)


# --- Folk Waltz in C (3/4) -----------------------------------------------------------------

def folk_walzer():
    b = Bau("Folk Waltz in C", "Folk", 108, 0.38, schlaege=3, je_schlag=4)
    # je Takt: Grundton (Saite), die Saite fuer den Hammer-on (leer -> 2. Bund), die oberen Saiten fuer den Strum
    takte = [(48, 1, 2, (55, 60, 64)),   # C
             (45, 1, 2, (57, 60, 64)),   # Am
             (41, 0, 3, (57, 60, 65)),   # F
             (43, 0, 1, (55, 59, 67))]   # G
    offen = (40, 45, 50, 55, 59, 64)
    noten = []
    for takt, (grund, gs, hs, oben) in enumerate(takte):
        t = 12 * takt
        leer = offen[hs]
        noten += [(t, 2, grund, 0.85, gs, LET_RING),
                  (t + 2, 1, leer, 0.65, hs), (t + 3, 1, leer + 2, 0.7, hs, LEGATO)]
        if takt < 3:
            schlaege = ((4, 2, 0.75), (6, 2, 0.55), (8, 2, 0.75), (10, 2, 0.55))
        else:
            schlaege = ((4, 8, 0.8),)  # der Schluss: ein breit gebrochener Akkord
        noten += [(t + s, l, h, v, AUTO, LET_RING if l > 2 else 0) for s, l, v in schlaege for h in oben]
    gitarre = saiten_muster(b, "Guitar", "piezo_guitar", noten, 0.85, 0.1, anschlaege=((40, "down", 45.0),))
    bass = saiten_muster(b, "Bass", "finger_bass", [(12 * t, 4, g, 0.8) for t, g in enumerate((36, 33, 29, 31))]
                         + [(12 * t + 8, 4, g, 0.6) for t, g in enumerate((43, 40, 36, 38))], 0.75, 0.0)
    beat = b.drums("Brush", [("Kick", "kit:kick", 0.45, 0.0, "x..........."),
                             ("Hi-Hat", "kit:hat_closed", 0.3, 0.3, "....x...x...")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Guitar", (gitarre, 0, 2))
    b.kanal("Bass", (bass, 0, 2))
    return fertig(b)


# --- Shuffle Blues in E --------------------------------------------------------------------

def blues_shuffle():
    # 12/8-Gefuehl: je Schlag drei Schritte, gespielt wird auf dem ersten (lang) und dem dritten (kurz)
    b = Bau("Shuffle Blues in E", "Blues", 92, 0.35, schlaege=4, je_schlag=3)
    # Boogie: Grundton auf der tieferen Saite, Quinte und Sexte im Wechsel auf der hoeheren
    boogie = [(0, 40, (47, 49)), (1, 45, (52, 54)), (0, 40, (47, 49)), (1, 47, (54, 56))]  # E A E B
    rhythmus, bass = [], []
    lauf = {40: (28, 32, 35, 37), 45: (33, 37, 40, 42), 47: (35, 39, 42, 44)}
    for takt, (s, grund, (quinte, sexte)) in enumerate(boogie):
        for schlag in range(4):
            t = 12 * takt + 3 * schlag
            oben = quinte if schlag % 2 == 0 else sexte
            for d, l, v in ((0, 2, 0.85), (2, 1, 0.65)):
                rhythmus += [(t + d, l, grund, v, s, PALM_MUTE), (t + d, l, oben, v, s + 1, PALM_MUTE)]
                bass.append((t + d, l, lauf[grund][schlag], v))
    G_, H_, D_ = 3, 4, 2
    lead = [  # Takt 1: Ganzton-Bend, Pull-off, Curl auf der kleinen Terz
            (3, 3, 69, 0.9, G_, 0, bend_auf(b, 3, 2.0)),
            (6, 2, 74, 0.85, H_), (8, 1, 71, 0.75, H_, LEGATO),
            (9, 3, 67, 0.8, G_, 0, bend_auf(b, 3, 0.5, 0.2)),
            # Takt 3: hoch, Bend und Release, Pull-off, Slide in den Grundton mit Vibrato (Takt 4)
            (24, 2, 76, 0.85, 5), (26, 1, 74, 0.75, H_),
            (27, 6, 69, 0.9, G_, 0, bend_und_zurueck(b, 6, 2.0)),
            (33, 1, 67, 0.75, G_, LEGATO), (34, 1, 64, 0.8, D_), (35, 1, 62, 0.75, D_),
            (36, 9, 64, 0.85, D_, SLIDE, vibrato(b, 9, tiefe=0.5))]
    akkorde = saiten_muster(b, "Boogie", "clean_guitar", rhythmus, 0.75, -0.25, raster="beat", hand="DDDD")
    solo = saiten_muster(b, "Lead", "overdrive_guitar", lead, 0.65, 0.25)
    tief = saiten_muster(b, "Bass", "finger_bass", bass, 0.85, 0.0)
    beat = b.drums("Shuffle", [("Kick", "kit:kick", 0.8, 0.0, "x.....x....."),
                               ("Snare", "kit:snare", 0.7, 0.0, "...x.....x.."),
                               ("Hi-Hat", "kit:hat_closed", 0.45, 0.25, "x.ox.ox.ox.o")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Boogie", (akkorde, 0, 2))
    b.kanal("Lead", (solo, 0, 2))
    b.kanal("Bass", (tief, 0, 2))
    return fertig(b)


# --- Funk Scratch in E9 --------------------------------------------------------------------

def funk_scratch():
    b = Bau("Funk Scratch in E9", "Funk", 100, 0.3, swing=0.1)
    e9, dis9 = (52, 56, 62, 66), (51, 55, 61, 65)  # A-, D-, G-, H-Saite: 7. bzw. 6. Lage
    # x Akzent, m gedaempfter Scratch, . Pause; je Takt sechzehn Stellen
    folgen = ["x.mmx.mxm.x.mmx.", "x.mmx.mxm.x.m.xm", "x.mmx.mxm.x.mmx.", "sx.mx.mxm.x.xmx."]
    gitarre = []
    for takt, folge in enumerate(folgen):
        for i, z in enumerate(folge):
            t = 16 * takt + i
            if z == "x":
                vorher_s = i > 0 and folge[i - 1] == "s"
                gitarre += [(t, 1, h, 0.85, 1 + k, SLIDE if vorher_s else 0) for k, h in enumerate(e9)]
            elif z == "m":
                gitarre += [(t, 1, h, 0.4, 1 + k, PALM_MUTE) for k, h in enumerate(e9)]
            elif z == "s":
                gitarre += [(t, 1, h, 0.8, 1 + k) for k, h in enumerate(dis9)]
    bass = []
    for takt in range(4):
        t = 16 * takt
        bass += [(t, 2, 28, 0.95, 0), (t + 3, 1, 40, 0.85, 2), (t + 4, 1, 38, 0.75, 2, LEGATO),
                 (t + 6, 2, 28, 0.85, 0), (t + 10, 1, 31, 0.75, 0), (t + 11, 1, 32, 0.8, 0, LEGATO)]
        if takt < 3:
            bass += [(t + 12, 2, 35, 0.8, 1), (t + 14, 1, 40, 0.85, 2), (t + 15, 1, 38, 0.7, 2, LEGATO)]
        else:
            bass += [(t + 12, 2, 35, 0.8, 1), (t + 14, 2, 40, 0.9, 1, SLIDE)]  # Slide auf der A-Saite hoch
    akkorde = saiten_muster(b, "Scratch", "clean_guitar", gitarre, 0.7, 0.3, raster="step")
    slap = saiten_muster(b, "Slap", "slap_bass", bass, 0.85, 0.0)
    beat = b.drums("Groove", [("Kick", "kit:kick", 0.9, 0.0, "x..x..x...x..x.."),
                              ("Snare", "kit:snare", 0.8, 0.0, "....x.......x..o"),
                              ("Hi-Hat", "kit:hat_closed", 0.4, 0.25, "xoxoxoxoxoxoxoxo"),
                              ("Clap", "kit:clap", 0.4, -0.1, "............x...")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Scratch", (akkorde, 0, 2))
    b.kanal("Slap", (slap, 0, 2))
    return fertig(b)


# --- Travis Picking in C -------------------------------------------------------------------

def travis():
    b = Bau("Travis Picking in C", "Folk", 112, 0.4)
    # je Takt: der Daumen (Hoehe, Saite) auf den vier Schlaegen, die Finger auf H- und hoher E-Saite
    takte = [([(48, 1), (52, 2), (43, 0), (52, 2)], (60, 64)),   # C
             ([(45, 1), (52, 2), (40, 0), (52, 2)], (60, 64)),   # Am
             ([(50, 2), (57, 3), (45, 1), (57, 3)], (62, 65)),   # Dm
             ([(43, 0), (50, 2), (47, 1), (50, 2)], (59, 65))]   # G7
    noten = []
    for takt, (daumen, (h_hoch, e_hoch)) in enumerate(takte):
        t = 16 * takt
        for schlag, (h, s) in enumerate(daumen):
            if schlag == 1 and takt < 2:
                # Hammer-on im Bass: leere D-Saite, dann 2. Bund
                noten += [(t + 4, 1, 50, 0.7, 2, PALM_MUTE), (t + 5, 3, 52, 0.75, 2, PALM_MUTE | LEGATO)]
            else:
                noten.append((t + 4 * schlag, 4, h, 0.8, s, PALM_MUTE))
        noten += [(t, 6, e_hoch, 0.75, 5, LET_RING), (t + 2, 4, h_hoch, 0.6, 4, LET_RING),
                  (t + 6, 4, e_hoch, 0.6, 5, LET_RING), (t + 10, 4, h_hoch, 0.6, 4, LET_RING)]
        if takt < 3:
            noten.append((t + 14, 2, e_hoch, 0.6, 5, LET_RING))
        else:
            # die Melodie rutscht auf der H-Saite vom 1. in den 3. Bund
            noten += [(t + 14, 1, 60, 0.65, 4), (t + 15, 1, 62, 0.7, 4, SLIDE | LET_RING)]
    gitarre = saiten_muster(b, "Picking", "guitar", noten, 0.85, 0.0)
    beat = b.drums("Shaker", [("Hi-Hat", "kit:hat_closed", 0.25, 0.3, "x.o.x.o.x.o.x.o.")])
    b.kanal("Picking", (gitarre, 0, 2))
    b.kanal("Shaker", (beat, 4, 1))
    return fertig(b)


# --- Hard Rock Riff in A -------------------------------------------------------------------

def hard_rock():
    b = Bau("Hard Rock Riff in A", "Rock", 124, 0.33)

    def a5(t, l=2, v=0.75, art=PALM_MUTE):
        return [(t, l, 45, v, 1, art), (t, l, 52, v, 2, art)]  # leere A-Saite, D-Saite 2. Bund

    riff = []
    for takt in range(4):
        t = 16 * takt
        for s in (0, 2, 4):
            riff += a5(t + s)
        if takt in (0, 2):
            # C5 -> D5, gerutscht auf A- und D-Saite
            riff += [(t + 6, 2, 48, 0.9, 1), (t + 6, 2, 55, 0.9, 2),
                     (t + 8, 4, 50, 0.9, 1, SLIDE), (t + 8, 4, 57, 0.9, 2, SLIDE)]
        elif takt == 1:
            # G5 -> A5 auf E- und A-Saite
            riff += [(t + 6, 2, 43, 0.9, 0), (t + 6, 2, 50, 0.9, 1),
                     (t + 8, 4, 45, 0.9, 0, SLIDE), (t + 8, 4, 52, 0.9, 1, SLIDE)]
        else:
            riff += a5(t + 6) + a5(t + 8, 4, 0.9, LET_RING)
        if takt < 3:
            riff += a5(t + 12) + a5(t + 14)
        else:
            riff += a5(t + 12, 4, 0.9, LET_RING)
    E_ = 5
    lead = []
    # Takt 2: getippt auf der hohen E-Saite (17. Bund), Pull-offs auf den 12. und 8., Hammer-on zurueck
    for i, h in enumerate((81, 76, 72, 76, 81, 76, 72, 76)):
        lead.append((24 + i, 1, h, 0.85 if h == 81 else 0.7, E_, TAPPING if h == 81 else LEGATO))
    # Takt 4: Ganzton-Bend auf der H-Saite, gehalten mit Vibrato
    lead.append((56, 8, 74, 0.9, 4, 0, vibrato(b, 8, ziel=2.0, tiefe=0.5, ab=0.35, schwingungen=4)))
    bass = []
    grund = [(33, 36, 38), (33, 31, 33), (33, 36, 38), (33, 33, 33)]
    for takt, (a, c, d) in enumerate(grund):
        t = 16 * takt
        bass += [(t + s, 2, a, 0.85) for s in (0, 2, 4)] + [(t + 6, 2, c, 0.9), (t + 8, 4, d, 0.9)]
        bass += [(t + 12, 2, a, 0.85), (t + 14, 2, a, 0.8)]
    gitarre = saiten_muster(b, "Riff", "overdrive_guitar", riff, 0.75, -0.2, hand="DDDDDDDD")
    solo = saiten_muster(b, "Lead", "overdrive_guitar", lead, 0.65, 0.25)
    tief = saiten_muster(b, "Bass", "finger_bass", bass, 0.9, 0.0, hand="DDDDDDDD")
    beat = b.drums("Beat", [("Kick", "kit:kick", 0.9, 0.0, "x.x.....x.x....."),
                            ("Snare", "kit:snare", 0.85, 0.0, "....x.......x..."),
                            ("Hi-Hat", "kit:hat_closed", 0.5, 0.25, "x.x.x.x.x.x.x.x.")])
    b.kanal("Drums", (beat, 0, 2))
    b.kanal("Riff", (gitarre, 0, 2))
    b.kanal("Lead", (solo, 0, 2))
    b.kanal("Bass", (tief, 0, 2))
    return fertig(b)


PROBEN_LOOPS = {"southern_rock_d": southern_rock, "folk_walzer_c": folk_walzer, "blues_shuffle_e": blues_shuffle,
                "funk_scratch_e9": funk_scratch, "travis_picking_c": travis, "hard_rock_a": hard_rock}


# --- Pruefung ------------------------------------------------------------------------------

def _offen(inst):
    daten = json.loads((LOOP / "instruments" / (inst + ".lvinst")).read_text(encoding="utf-8"))
    return daten["strings"]["open"]


def pruefe(pfad):
    """Liest eine geschriebene Datei und prueft Grenzen, Saiten, Bends und die Vorgaenger verbundener Noten."""
    loop = json.loads(pfad.read_text(encoding="utf-8"))
    assert loop["version"] == 10, pfad
    schritte = loop["beatsPerBar"] * loop["stepsPerBeat"]
    namen = {m["name"] for m in loop["patterns"]}
    assert len(namen) == len(loop["patterns"]), pfad
    fehler = []
    for m in loop["patterns"]:
        ende = m["bars"] * schritte
        if m["kind"] == "drumkit":
            assert all(len(r) == ende for r in m["rows"]), (pfad, m["name"])
            continue
        offen = _offen(m["instrument"])
        noten = m["notes"]
        for n in noten:
            schritt, laenge, hoehe, laut = n[:4]
            saite = n[4] if len(n) > 4 else AUTO
            art = n[5] if len(n) > 5 else 0
            bend = n[6] if len(n) > 6 else []
            wo = (pfad.name, m["name"], n[:6])
            if not (0 <= schritt and laenge >= 1 and schritt + laenge <= ende):
                fehler.append(("ausserhalb des Musters", wo))
            if not (21 <= hoehe <= 108 and 0 < laut <= 1):
                fehler.append(("Hoehe/Staerke", wo))
            if not (AUTO <= saite < len(offen)) or art & ~63:
                fehler.append(("Saite/Bits", wo))
            if saite >= 0 and not (0 <= hoehe - offen[saite] <= 24):
                fehler.append(("nicht greifbar auf der Saite", wo))
            if bend:
                zeiten = [p[0] for p in bend]
                if (len(bend) > 16 or zeiten != sorted(zeiten)
                        or any(not (0 <= z <= 1 and 0 <= h <= 3 and (h * 4) == int(h * 4)) for z, h in bend)
                        or all(h == 0 for _, h in bend)):
                    fehler.append(("Bend", wo))
            if art & (LEGATO | SLIDE):
                if saite < 0:
                    fehler.append(("verbunden ohne Saite", wo))
                vorher = [v for v in noten if len(v) > 4 and v[4] == saite and v[0] + v[1] == schritt]
                if not vorher or any(v[2] == hoehe for v in vorher):
                    fehler.append(("keine Note davor auf derselben Saite", wo))
    return fehler


def main():
    alle = []
    for name, bau in PROBEN_LOOPS.items():
        pfad = PROBEN / (name + ".lvloop")
        pfad.write_text(als_text(bau()), encoding="utf-8", newline="\n")
        alle += pruefe(pfad)
    for f in alle:
        print("FEHLER", *f)
    print(len(PROBEN_LOOPS), "Gitarren-Proben,", len(alle), "Fehler")
    return 1 if alle else 0


if __name__ == "__main__":
    sys.exit(main())
