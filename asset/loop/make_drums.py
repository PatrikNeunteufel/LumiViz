"""Erzeugt die sechs gerechneten Drum-Instrumente neben dem Kit (S117, Stufe 11f;
Entscheid Patrik K1 a, S115).

    python asset/loop/make_drums.py

Schreibt nach `asset/loop/instruments/drum_*.lvinst` (Fassung 6: Rauschen, Pitch
sweep, Art des Filters; **Kick und Clap seit S118, Stufe 16e, in Fassung 7** mit
der Reihe `path` — Signal path). Kein Zufall: zwei Laeufe ergeben dieselben
Dateien. Massgebend sind die Dateien im Repo, nicht ein neuer Lauf.

Vorbild sind die sechs Klaenge des Kits (`kit/make_kit.py`); gebaut sind sie nur
aus dem, was eine Schicht des Instrument-Creators kann. Die Einstellungen sind
nach Rechnung gesetzt und gegen das Kit gemessen, nicht nach Gehoer.

**Alle sechs sind fuer die Note C2 gebaut** — die Note, die eine Drum-Spur einem
Instrument der Art Drums vorbelegt. Wohin der Schlag gehoert, steht als
Transpose in der Schicht: der Filter einer Schicht folgt der Tonhoehe (Cutoff ist
ein Vielfaches des Tons), und bei C2 selbst reichte er nur bis 4,2 kHz.

    Kick     Sinus bei F#1 (46 Hz), faellt aus 22 Halbtoenen, Drive HINTER der Envelope;
             ein Klick aus Rauschen
    Tom      Sinus bei A2 (110 Hz), faellt aus 10 Halbtoenen; ein Klick aus Rauschen
    Snare    Sinus bei 185 Hz, ein zweiter bei 165 Hz mit seinem zweiten Teilton
             (330 Hz), Rauschen ueber 1,8 kHz
    Clap     vier Schichten Rauschen durch Hochpass (520 Hz) und Bandpass (1,2 kHz),
             je 10 ms spaeter, die letzte klingt aus
    Hi-Hats  Rauschen ueber 5,8 kHz, kurz und lang

Anders als das Kit, weil eine Schicht es nicht kann: die 330 Hz der Snare kommen
als Teilton eines Sinus bei 165 Hz, der mitklingt (Transpose reicht bis +24, also
bis 262 Hz); die Hi-Hats sind Rauschen allein, ohne die sechs Rechtecke; der Klick
von Kick und Tom dauert 10 ms (kuerzer stellt der Regler nicht). Die Werte sind aus
Reihen gewaehlt, die das Programm gerechnet und
`.claude/handover/hoerproben_11f/vergleich.py` gegen das Kit gemessen hat (S117) —
Zahlen, kein Ohr.

Seit Stufe 16e (S118) nutzen Kick und Clap, was die Reihe einer Schicht kann: beim
Kick steht Drive hinter der Envelope (er laesst mit dem Ton nach, wie das `tanh`
des Kits), der Clap hat zwei Filter hintereinander (Hochpass, dann Bandpass, wie
das Kit). Wieder aus Reihen gewaehlt (Kick 42 Varianten, Clap 173) und gegen das
Kit gemessen: Kick 0,9 / 1,6 dB in den Baendern (vorher 4,9 / 15,2), Clap
1,1 / 3,5 dB (vorher 4,7 / 12,5).

Leiser als das Kit: eine Schicht bei vollem Level hat eine Spitze um 0,55, ein
Klang des Kits 0,9.
"""

from pathlib import Path

from make_examples import als_text, schicht

HIER = Path(__file__).resolve().parent


def schlag(name, waveform, decay, release, **werte):
    """Eine Schicht ohne Anstieg und ohne Halten; dazu die Felder seit Fassung 5 und 6."""
    neu = {"pitchStart": 0, "pitchTime": 100, "filterType": "lowpass", "filterResonance": 0}
    extra = {k: werte.pop(k) for k in list(werte) if k in neu}
    s = schicht(name, waveform, attack=0, decay=decay, sustain=0, release=release, **werte)
    s.update(neu)
    s.update(extra)
    return s


def instrument(titel, level, *schichten, fassung=6):
    return {"format": "lumiviz-instrument", "version": fassung, "title": titel, "level": level, "layers": list(schichten)}


# --- die Reihe einer Schicht (Fassung 7, Signal path): Filter und Drive beliebig oft, Envelope einmal

HUELLE = {"type": "envelope"}


def filt(art, cutoff, resonance=0.0):
    return {"type": "filter", "filterType": art, "cutoff": cutoff, "envelope": 0, "resonance": resonance}


def drive(amount):
    return {"type": "drive", "amount": amount}


def schlag_mit_reihe(name, waveform, decay, release, reihe, **werte):
    """Wie `schlag`, in Fassung 7: statt der flachen Felder fuer Filter und Drive die Liste `path`."""
    s = schlag(name, waveform, decay, release, **werte)
    for feld in ("drive", "filterCutoff", "filterEnvelope", "filterType", "filterResonance"):
        del s[feld]
    s["path"] = list(reihe)
    return s


def clap_schichten():
    # Hochpass, dann Bandpass — wie das Kit (600 Hz, dann 1,2 kHz mit Guete 1,2); von C4 aus sind das
    # die Vielfachen 2,0 und 4,6. Drive hinter der Envelope hebt den Pegel: durch den Bandpass kommt
    # wenig Rauschen. Der Ausklang ist mit 190 ms etwas kuerzer als im Kit (210): Drive haelt ihn laenger.
    reihe = [filt("highpass", 2.0), filt("bandpass", 4.6, 0.19), HUELLE, drive(0.4)]
    return [schlag_mit_reihe(name, "noise", decay, 200, reihe, transpose=24, startOffset=start)
            for name, start, decay in (("Hit 1", 0, 10), ("Hit 2", 10, 10), ("Hit 3", 20, 10), ("Tail", 30, 190))]


def schicht_ohne_anschlag(name, waveform, attack, decay, release, **werte):
    """Wie `schlag`, aber mit Anstieg: ein Klang, der anschwillt (der Besen, der kreist)."""
    s = schlag(name, waveform, decay, release, **werte)
    s["attack"] = attack
    return s


# Release ist bei den langen Schlaegen gross: eine Drum-Spur spielt die Note 250 ms (Length),
# danach soll der Schlag ausklingen wie sein Decay, nicht abbrechen.
INSTRUMENTE = {
    # Drive hinter der Envelope: er verzerrt den lauten Anschlag und laesst mit dem Ton nach (das Kit:
    # tanh(1,5 * Ton), also Drive 0,19). Sweep time 105 ms: das Dreifache der 35 ms des Kits.
    "drum_kick": instrument(
        "Kick", 1.0,
        schlag_mit_reihe("Body", "sine", 480, 1000, [HUELLE, drive(0.19)], transpose=-6, pitchStart=22, pitchTime=105),
        schlag_mit_reihe("Click", "noise", 10, 30, [HUELLE], level=0.25),
        fassung=7),
    "drum_tom": instrument(
        "Tom", 1.0,
        schlag("Body", "sine", 420, 700, transpose=9, pitchStart=10, pitchTime=120),
        schlag("Click", "noise", 10, 30, level=0.15)),
    # Level 0.6: drei Schichten zusammen waeren sonst fast doppelt so laut wie die anderen fuenf
    "drum_snare": instrument(
        "Snare", 0.6,
        schlag("Drum", "sine", 180, 200, transpose=18, level=0.6),
        schlag("Ring", "sine", 110, 120, transpose=16, level=0.2, harmonics=[1.0]),
        schlag("Wires", "noise", 250, 250, transpose=18, level=0.9, filterType="highpass", filterCutoff=9.7)),
    "drum_clap": instrument("Clap", 1.0, *clap_schichten(), fassung=7),
    "drum_hat_closed": instrument(
        "Hi-Hat Closed", 1.0,
        schlag("Noise", "noise", 70, 60, transpose=24, filterType="highpass", filterCutoff=22)),
    "drum_hat_open": instrument(
        "Hi-Hat Open", 1.0,
        schlag("Noise", "noise", 450, 1000, transpose=24, filterType="highpass", filterCutoff=22)),
    # --- S123: sechs weitere, fuer die Kits (Entscheid 3 a); nach Rechnung, nicht gegen eine Vorlage gemessen
    # Die 808-Kick: tiefer und laenger als Kick, die Tonhoehe faellt schneller, mehr Drive
    "drum_kick_808": instrument(
        "808 Kick", 1.0,
        schlag_mit_reihe("Body", "sine", 900, 1300, [HUELLE, drive(0.3)], transpose=-5, pitchStart=12, pitchTime=60),
        schlag_mit_reihe("Click", "noise", 10, 30, [HUELLE], level=0.15),
        fassung=7),
    "drum_tom_low": instrument(
        "Low Tom", 1.0,
        schlag("Body", "sine", 520, 800, transpose=3, pitchStart=8, pitchTime=150),
        schlag("Click", "noise", 10, 30, level=0.12)),
    "drum_tom_high": instrument(
        "High Tom", 1.0,
        schlag("Body", "sine", 340, 600, transpose=14, pitchStart=8, pitchTime=100),
        schlag("Click", "noise", 10, 30, level=0.12)),
    # zwei kurze Stoesse Rauschen, der zweite 25 ms spaeter: das Korn rutscht nach
    "drum_shaker": instrument(
        "Shaker", 1.0,
        schlag("Shake", "noise", 90, 80, transpose=24, filterType="highpass", filterCutoff=12),
        schlag("Grain", "noise", 60, 60, transpose=24, startOffset=25, level=0.6, filterType="highpass", filterCutoff=12)),
    # Ride: langes, hohes Rauschen und zwei Rechtecke im Abstand einer Quarte darueber als Glocke
    "drum_ride": instrument(
        "Ride", 0.8,
        schlag("Wash", "noise", 1400, 1500, transpose=24, level=0.5, filterType="highpass", filterCutoff=16),
        schlag("Bell", "square", 1200, 1200, transpose=24, level=0.25, filterType="highpass", filterCutoff=8),
        schlag("Bell 2", "square", 900, 900, transpose=19, level=0.15, filterType="highpass", filterCutoff=10)),
    "drum_crash": instrument(
        "Crash", 0.8,
        schlag("Wash", "noise", 1800, 1800, transpose=24, filterType="highpass", filterCutoff=8),
        schlag("Sizzle", "noise", 900, 900, transpose=24, level=0.5, filterType="highpass", filterCutoff=20)),
    # --- S124, Stufe 21d: drei Besen fuer das Kit Jazz Brushes (Entscheid K6c a); nach Rechnung, nicht gehoert.
    # Ein Besen ist Rauschen ohne harten Anschlag: viele Draehte, die das Fell streifen. Der Filter folgt der
    # Tonhoehe — bei Transpose 24 (C4, 262 Hz) liegt Cutoff 11 bei 2,9 kHz, 6 bei 1,6 kHz.
    # Tap: ein kurzer Schlag auf die Snare — Rauschen ueber 1,6 kHz, Bandpass um 2,9 kHz darauf, ein Hauch
    # Fell darunter. Nur der Bandpass war 7,5 dB leiser als die anderen Snares (gemessen S124).
    "drum_brush_tap": instrument(
        "Brush Tap", 1.0,
        schlag("Wires", "noise", 110, 140, transpose=24, filterType="highpass", filterCutoff=6),
        schlag("Bristle", "noise", 80, 100, transpose=24, level=0.7, filterType="bandpass", filterCutoff=11,
               filterResonance=0.1),
        schlag("Head", "sine", 70, 80, transpose=18, level=0.2)),
    # Slap: der Akzent — lauter, heller, mit mehr Fell und einem kurzen Klick
    "drum_brush_slap": instrument(
        "Brush Slap", 1.0,
        schlag("Wires", "noise", 220, 250, transpose=24, filterType="highpass", filterCutoff=8),
        schlag("Head", "sine", 120, 140, transpose=18, level=0.35),
        schlag("Click", "noise", 15, 20, transpose=24, level=0.3, filterType="highpass", filterCutoff=20)),
    # Sweep: der Besen kreist auf dem Fell — Rauschen schwillt an und ab, ohne Anschlag; in einer
    # Drum-Spur mit langer Length (das Kit setzt 500 ms) klingt es einen halben Takt
    "drum_brush_sweep": instrument(
        "Brush Sweep", 1.0,
        schicht_ohne_anschlag("Swish", "noise", 180, 420, 500, transpose=24, filterType="bandpass", filterCutoff=6,
                              filterResonance=0.05),
        schicht_ohne_anschlag("Air", "noise", 260, 380, 450, transpose=24, level=0.4, filterType="highpass",
                              filterCutoff=14)),
}


def main():
    from instrument_katalog import ergaenze, genres_aus_loops
    genres = genres_aus_loops()
    for name, daten in INSTRUMENTE.items():
        (HIER / "instruments" / (name + ".lvinst")).write_text(als_text(ergaenze(name, daten, genres)),
                                                               encoding="utf-8", newline="\n")
    print(len(INSTRUMENTE), "Drum-Instrumente")


if __name__ == "__main__":
    main()
