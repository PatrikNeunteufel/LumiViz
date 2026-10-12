"""Erzeugt die sechs gerechneten Drum-Instrumente neben dem Kit (S117, Stufe 11f;
Entscheid Patrik K1 a, S115).

    python asset/loop/make_drums.py

Schreibt nach `asset/loop/instruments/drum_*.lvinst` (Fassung 6: Rauschen, Pitch
sweep, Art des Filters). Kein Zufall: zwei Laeufe ergeben dieselben Dateien.
Massgebend sind die Dateien im Repo, nicht ein neuer Lauf.

Vorbild sind die sechs Klaenge des Kits (`kit/make_kit.py`); gebaut sind sie nur
aus dem, was eine Schicht des Instrument-Creators kann. Die Einstellungen sind
nach Rechnung gesetzt und gegen das Kit gemessen, nicht nach Gehoer.

**Alle sechs sind fuer die Note C2 gebaut** — die Note, die eine Drum-Spur einem
Instrument der Art Drums vorbelegt. Wohin der Schlag gehoert, steht als
Transpose in der Schicht: der Filter einer Schicht folgt der Tonhoehe (Cutoff ist
ein Vielfaches des Tons), und bei C2 selbst reichte er nur bis 4,2 kHz.

    Kick     Sinus bei F#1 (46 Hz), faellt aus 22 Halbtoenen; ein Klick aus Rauschen
    Tom      Sinus bei A2 (110 Hz), faellt aus 10 Halbtoenen; ein Klick aus Rauschen
    Snare    Sinus bei 185 Hz, ein zweiter bei 165 Hz mit seinem zweiten Teilton
             (330 Hz), Rauschen ueber 1,8 kHz
    Clap     vier Schichten Rauschen um 1,7 kHz, je 10 ms spaeter, die letzte klingt aus
    Hi-Hats  Rauschen ueber 5,8 kHz, kurz und lang

Anders als das Kit, weil eine Schicht es nicht kann: die 330 Hz der Snare kommen
als Teilton eines Sinus bei 165 Hz, der mitklingt (Transpose reicht bis +24, also
bis 262 Hz); der Clap hat einen Filter statt zwei; die Hi-Hats sind Rauschen
allein, ohne die sechs Rechtecke; der Klick von Kick und Tom dauert 10 ms (kuerzer
stellt der Regler nicht). Die Werte sind aus Reihen gewaehlt, die das Programm
gerechnet und `.claude/handover/hoerproben_11f/vergleich.py` gegen das Kit gemessen
hat (S117) — Zahlen, kein Ohr.

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


def instrument(titel, level, *schichten):
    return {"format": "lumiviz-instrument", "version": 6, "title": titel, "level": level, "layers": list(schichten)}


def clap_schichten():
    # Drive hebt hier nur den Pegel: durch den Bandpass kommt wenig Rauschen
    return [schlag(name, "noise", decay, 200, transpose=24, startOffset=start, drive=0.4,
                   filterType="bandpass", filterCutoff=6.5, filterResonance=0.35)
            for name, start, decay in (("Hit 1", 0, 10), ("Hit 2", 10, 10), ("Hit 3", 20, 10), ("Tail", 30, 210))]


# Release ist bei den langen Schlaegen gross: eine Drum-Spur spielt die Note 250 ms (Length),
# danach soll der Schlag ausklingen wie sein Decay, nicht abbrechen.
INSTRUMENTE = {
    "drum_kick": instrument(
        "Kick", 1.0,
        schlag("Body", "sine", 480, 1000, transpose=-6, pitchStart=22, pitchTime=80, drive=0.19),
        schlag("Click", "noise", 10, 30, level=0.25)),
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
    "drum_clap": instrument("Clap", 1.0, *clap_schichten()),
    "drum_hat_closed": instrument(
        "Hi-Hat Closed", 1.0,
        schlag("Noise", "noise", 70, 60, transpose=24, filterType="highpass", filterCutoff=22)),
    "drum_hat_open": instrument(
        "Hi-Hat Open", 1.0,
        schlag("Noise", "noise", 450, 1000, transpose=24, filterType="highpass", filterCutoff=22)),
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
