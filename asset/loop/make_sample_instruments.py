"""Erzeugt die mitgelieferten Instrumente aus den mitgelieferten Samples (S123; Entscheide
Patrik 1 a und 2 a — die Samples liegen unter `asset/loop/samples`, im Loop `lib:<name>`).

    python asset/loop/make_sample_instruments.py

Schreibt nach `asset/loop/instruments/*.lvinst`, Fassung 8 (Pan und Signal path). Kein
Zufall: zwei Laeufe ergeben dieselben Dateien.

- **Grundton und Fine tune** sind gemessen (`samples/make_samples.py` gibt sie aus; bei Harfe
  und Hackbrett nach dem Spektrum, die Messung im Zeitbereich griff dort daneben). Fine tune
  ist, wie weit die Aufnahme ueber dem Grundton liegt.
- **Die Schleife** (Sustain loop) gehaltener Klaenge liegt im ruhigen Mittelteil; ihre Raender
  sind auf aufsteigende Nulldurchgaenge gelegt, die dem gewuenschten Wert am naechsten sind
  — gegen ein Knacken an der Naht. Gehoert hat sie niemand.
- **Die Drums aus Samples** sind fuer die Note C2 gebaut wie die gerechneten (Grundton 36):
  auf C2 klingt die Aufnahme, wie sie ist.

Alles nach Rechnung und Messung eingestellt, nicht nach Gehoer.
"""

from pathlib import Path

import numpy as np
from scipy.io import wavfile

from make_examples import als_text, schicht

HIER = Path(__file__).resolve().parent
SAMPLES = HIER / "samples"
HUELLE = {"type": "envelope"}


def filt(art, cutoff, envelope=0.0, resonance=0.0):
    return {"type": "filter", "filterType": art, "cutoff": cutoff, "envelope": envelope, "resonance": resonance}


def drive(amount):
    return {"type": "drive", "amount": amount}


def lage(name, waveform, reihe=None, pan=0.0, pitch=None, **werte):
    s = schicht(name, waveform, **werte)
    for feld in ("drive", "filterCutoff", "filterEnvelope"):
        del s[feld]
    s.update({"pan": pan, "pitchStart": pitch[0] if pitch else 0, "pitchTime": pitch[1] if pitch else 100,
              "path": list(reihe or [HUELLE])})
    return s


def nulldurchgang(daten, ms):
    """Der aufsteigende Nulldurchgang, der `ms` am naechsten liegt, in Millisekunden (auf 0,01 ms)."""
    stellen = np.nonzero((daten[:-1] < 0) & (daten[1:] >= 0))[0]
    ziel = ms * 44.1
    i = stellen[np.argmin(np.abs(stellen - ziel))]
    # zwischen den beiden Werten linear: wo die Linie die Null schneidet
    genau = i + daten[i] / (daten[i] - daten[i + 1])
    return round(float(genau) / 44.1, 2)


def probe(name, sample, grundton, fein=0, schleife=None, start=0, **werte):
    """Eine Sample-Schicht; `schleife` = (von_ms, bis_ms) als Wunsch, auf Nulldurchgaenge gelegt."""
    s = lage(name, "sample", **werte)
    von = bis = 0
    if schleife:
        _, roh = wavfile.read(SAMPLES / (sample + ".wav"))
        daten = roh.astype(np.float64) / 32768.0
        von, bis = nulldurchgang(daten, schleife[0]), nulldurchgang(daten, schleife[1])
    s.update({"sample": "lib:" + sample, "rootNote": grundton, "fineTune": fein, "sampleStart": start,
              "sustainLoop": bool(schleife), "loopFrom": von, "loopTo": bis})
    return s


def instrument(titel, level, *schichten):
    return {"format": "lumiviz-instrument", "version": 8, "title": titel, "level": level, "layers": list(schichten)}


INSTRUMENTE = {
    # --- Stimmen
    "female_choir": instrument(
        "Female Choir", 0.8,
        probe("Aah", "voice_female_aah", 60, -14, (700, 2400), attack=220, decay=400, sustain=0.9, release=650, pan=-0.2),
        probe("Oo", "voice_female_oo", 60, 5, (900, 3600), attack=380, decay=400, sustain=0.9, release=750, level=0.75,
              pan=0.2)),
    # Ein Maennerchor fehlt mit Absicht: voice_male_aah und voice_male_oo gleiten in 1,5 s um rund
    # fuenfzehn Halbtoene nach unten, voice_male_la schwankt um +-0,7 — gemessen (S123), fuer einen
    # gehaltenen Ton taugt keine. Sie bleiben Samples fuer Effekte.
    # Die Stimmen schleifen in den Ton (Hum 300 ms, Ha 250 ms Atem): Sample start dahinter.
    "hum_pad": instrument(
        "Hum Pad", 0.85,
        probe("Hum", "voice_female_hum", 72, -38, (1500, 5000), start=300, attack=500, decay=600, sustain=0.95,
              release=1100),
        probe("Uh", "voice_female_uh", 72, -13, (800, 2800), transpose=-12, attack=700, decay=600, sustain=0.9,
              release=1100, level=0.45),
        lage("Sub", "sine", transpose=-12, attack=600, decay=600, sustain=1.0, release=1000, level=0.2)),
    "vocal_lead": instrument(
        "Vocal Lead", 0.85,
        probe("Ha", "voice_female_ha", 72, -33, (1500, 6000), start=260, attack=40, decay=300, sustain=0.9, release=350),
        lage("Edge", "saw", [filt("lowpass", 3.0), HUELLE], attack=40, decay=300, sustain=0.8, release=300, level=0.12)),
    "vocal_chop": instrument(
        "Vocal Chop", 0.9,
        probe("Ah", "voice_female_ah", 66, -30, attack=2, decay=260, sustain=0, release=120),
        probe("Aah", "voice_female_aah", 60, -14, transpose=12, attack=2, decay=180, sustain=0, release=100, level=0.3)),
    # --- Gezupft und geschlagen
    "harp": instrument(
        "Harp", 0.85,
        probe("Harp", "harp", 69, -5, attack=1, decay=2000, sustain=0, release=900),
        lage("Body", "sine", attack=1, decay=600, sustain=0, release=400, level=0.15)),
    "koto": instrument(
        "Koto", 0.8,
        probe("Koto", "koto", 62, 14, attack=1, decay=3000, sustain=0, release=800)),
    "dulcimer": instrument(
        "Dulcimer", 0.9,
        probe("Dulcimer", "dulcimer", 57, -5, attack=1, decay=2200, sustain=0, release=800)),
    "thumb_piano": instrument(
        "Thumb Piano", 0.8,
        probe("Tine", "thumb_piano", 57, 37, attack=1, decay=2600, sustain=0, release=700),
        lage("Tick", "sine", transpose=24, attack=1, decay=40, sustain=0, release=30, level=0.15)),
    # --- Blaeser
    "shakuhachi": instrument(
        "Shakuhachi", 0.85,
        # die ersten 150 ms blaest es kurz in die Oktave: Sample start dahinter
        probe("Bamboo", "shakuhachi", 69, 32, (300, 1000), start=200, attack=70, decay=300, sustain=0.9, release=220),
        lage("Breath", "noise", [filt("bandpass", 1.2, resonance=0.3), HUELLE], transpose=12, attack=40, decay=160,
             sustain=0.2, release=150, level=0.1)),
    "alto_sax": instrument(
        "Alto Sax", 0.85,
        probe("Sax", "sax_alto", 72, 5, (700, 2000), attack=30, decay=300, sustain=0.9, release=180)),
    "alto_flute": instrument(
        "Alto Flute", 0.85,
        probe("Flute", "flute_alto", 60, 10, (900, 2600), attack=60, decay=300, sustain=0.9, release=220)),
    # --- Zwei Aufnahmen, die zusammen passen
    "angel_harp": instrument(
        "Angel Harp", 0.75,
        probe("Harp", "harp", 69, -5, attack=1, decay=2000, sustain=0, release=900),
        probe("Choir", "voice_female_oo", 60, 5, (900, 3600), transpose=12, attack=700, decay=600, sustain=0.8,
              release=1200, level=0.4)),
    "wind_voices": instrument(
        "Wind Voices", 0.8,
        probe("Flute", "flute_alto", 60, 10, (900, 2600), attack=400, decay=500, sustain=0.9, release=900, pan=-0.25),
        probe("Hum", "voice_female_hum", 72, -38, (1500, 5000), attack=600, decay=500, sustain=0.9, release=1100,
              level=0.6, pan=0.25)),
    # --- Drums aus Samples, geschichtet (fuer C2 gebaut)
    "drum_kick_punch": instrument(
        "Punch Kick", 0.6,
        probe("Click", "kick_909", 36, attack=0, decay=400, sustain=0, release=300),
        lage("Sub", "sine", transpose=-7, pitch=(12, 60), attack=0, decay=450, sustain=0, release=400, level=0.6,
             reihe=[HUELLE, drive(0.15)])),
    "drum_snare_fat": instrument(
        "Fat Snare", 0.65,
        probe("Snare", "snare_909", 36, attack=0, decay=300, sustain=0, release=200),
        probe("Clap", "clap_707", 36, startOffset=8, attack=0, decay=200, sustain=0, release=150, level=0.6),
        lage("Tail", "noise", [filt("highpass", 6.0), HUELLE], transpose=24, attack=0, decay=280, sustain=0, release=250,
             level=0.25)),
    "drum_clap_stack": instrument(
        "Clap Stack", 0.85,
        probe("Clap", "clap_707", 36, attack=0, decay=200, sustain=0, release=150, pan=-0.15),
        probe("Clap 2", "clap_707", 36, 15, startOffset=12, attack=0, decay=200, sustain=0, release=150, pan=0.15,
              level=0.8),
        probe("Snap", "snap", 36, startOffset=4, attack=0, decay=150, sustain=0, release=100, level=0.5)),
    "drum_hat_shake": instrument(
        "Hat Shake", 0.85,
        probe("Hat", "hat_808_closed", 36, attack=0, decay=100, sustain=0, release=80),
        probe("Tambourine", "tambourine", 36, attack=0, decay=260, sustain=0, release=150, level=0.35, pan=0.3)),
    "drum_beatbox_kick": instrument(
        "Beatbox Kick", 0.6,
        probe("Mouth", "beatbox_kick", 36, attack=0, decay=500, sustain=0, release=300),
        lage("Sub", "sine", transpose=-10, pitch=(10, 70), attack=0, decay=320, sustain=0, release=300, level=0.45)),
}


def main():
    from instrument_katalog import ergaenze, genres_aus_loops
    genres = genres_aus_loops()
    for name, daten in INSTRUMENTE.items():
        (HIER / "instruments" / (name + ".lvinst")).write_text(als_text(ergaenze(name, daten, genres)),
                                                               encoding="utf-8", newline="\n")
    print(len(INSTRUMENTE), "Instrumente aus Samples")


if __name__ == "__main__":
    main()
