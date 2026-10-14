"""Die mitgelieferten Samples aus den Originalen schreiben (S123; Entscheide Patrik 1 a und 2 a).

    python asset/loop/samples/make_samples.py <ordner-mit-den-originalen>

Die Originale stammen aus der OLPC-Sammlung auf archive.org (CC BY 3.0, die Lizenz steht je
Eintrag in den Metadaten) und liegen NICHT im Repo; geladen werden sie einzeln aus den ZIPs der
Eintraege, Adresse je Datei unten in `QUELLEN` (`<eintrag>/<zip>/<pfad>`). Im Ordner heisst ein
Original `<eintrag>__<datei>.wav` — so, wie es die Sitzung S123 abgelegt hat.

Je Sample: Stille an den Raendern weg (Schwelle -50 dB, davor 2 ms stehen gelassen), am Ende
10 ms ausgeblendet, die Spitze auf 0,89 (wie die Klaenge des Kits), Mono, 16 Bit, 44100 Hz.
Dazu `CREDITS.md` (geht mit neben die Exe) und auf der Konsole die gemessene Tonhoehe der
Klaenge mit Ton — sie steht als `rootNote` in den Instrumenten (`make_sample_instruments.py`).

Gebraucht: Python 3 mit numpy und scipy.
"""

import sys
from pathlib import Path

import numpy as np
from scipy.io import wavfile

HIER = Path(__file__).resolve().parent
RATE = 44100
SPITZE = 0.89

# Name im Loop (lib:<name>) -> (Eintrag auf archive.org, ZIP, Datei im ZIP, Urheber laut Eintrag, Was, Ton?)
QUELLEN = {
    # --- Stimmen
    "voice_female_ha": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/femVoice_ha-c2.wav", "Chen Sokolovsky", "female voice, sung \"ha\"", True),
    "voice_female_hum": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/femVoice_hm-c2.wav", "Chen Sokolovsky", "female voice, hummed", True),
    "voice_female_uh": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/femVoice_uh-c2.wav", "Chen Sokolovsky", "female voice, sung \"uh\"", True),
    "voice_female_aah": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/singing-womanMA-C-aah.wav", "OpenPathMusic", "female voice, sung \"aah\"", True),
    "voice_female_oo": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/singing-womanMA-C-oo.wav", "OpenPathMusic", "female voice, sung \"oo\"", True),
    "voice_female_ah": ("Boulanger44CsoundBookSelected", "Boulanger44CsoundBookSelected.zip", "Boulanger44CsoundBookSelected/voicefemale_ah1.wav", "Richard Boulanger", "female voice, short \"ah\"", True),
    "voice_female_gliss": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/female-glissdown.wav", "OpenPathMusic", "female voice, gliding down", False),
    "voice_male_aah": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/voice-man-aah.wav", "OpenPathMusic", "male voice, sung \"aah\"", True),
    "voice_male_oo": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/voice-man-oo.wav", "OpenPathMusic", "male voice, sung \"oo\"", True),
    "voice_male_la": ("Boulanger44CsoundBookSelected", "Boulanger44CsoundBookSelected.zip", "Boulanger44CsoundBookSelected/voicemale_la.wav", "Richard Boulanger", "male voice, sung \"la\"", True),
    "voice_male_hey": ("OpenPathMusic44V5", "OpenPathMusic44V5.zip", "OpenPathMusic44V5/voice-man-shout-hey.wav", "OpenPathMusic", "male voice, shouted \"hey\"", False),
    # --- Beatbox
    "beatbox_kick": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/vocalDrum_bd1.wav", "Chen Sokolovsky", "beatbox kick", False),
    "beatbox_snare": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/vocalDrum_sn.wav", "Chen Sokolovsky", "beatbox snare", False),
    "beatbox_hat_closed": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/vocalDrum_chh.wav", "Chen Sokolovsky", "beatbox closed hi-hat", False),
    "beatbox_hat_open": ("ChenSokolovsky", "ChenSokolovsky44.zip", "ChenSokolovsky44/vocalDrum_ohh.wav", "Chen Sokolovsky", "beatbox open hi-hat", False),
    # --- Drum-Maschinen und Schlagwerk
    "kick_909": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr909kick.wav", "TamTam Drum Kits", "909-style kick", False),
    "kick_909_long": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr909bass.wav", "TamTam Drum Kits", "909-style bass drum, long", False),
    "snare_909": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr909sn.wav", "TamTam Drum Kits", "909-style snare", False),
    "snare_808": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr808sn.wav", "TamTam Drum Kits", "808-style snare", False),
    "hat_808_closed": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr808closed.wav", "TamTam Drum Kits", "808-style closed hi-hat", False),
    "clap_707": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr707clap.wav", "TamTam Drum Kits", "707-style clap", False),
    "hat_707_open": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/drum4tr707open.wav", "TamTam Drum Kits", "707-style open hi-hat", False),
    "tambourine": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/tambourine_high.wav", "TamTam Drum Kits", "tambourine", False),
    "djembe_low": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/djembe_low.wav", "TamTam Drum Kits", "djembe, low stroke", False),
    "djembe_slap": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/djembe_slap.wav", "TamTam Drum Kits", "djembe, slap", False),
    "conga_high": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/Conga aigu ouvert.wav", "TamTam Drum Kits", "high conga, open", False),
    "darbuka_doom": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/darbuka_doom.wav", "TamTam Drum Kits", "darbuka, doum", False),
    "darbuka_tak": ("TamTamDrumKits", "TamTamDrumKits44.zip", "tamtamDrumKits44/darbuka_tak.wav", "TamTam Drum Kits", "darbuka, tak", False),
    "kick_deep": ("MattLange", "MattLange44.zip", "MattLange44/kick1.wav", "Matt Lange", "deep electronic kick", False),
    "snap": ("MattLange", "MattLange44.zip", "MattLange44/snap1.wav", "Matt Lange", "finger snap", False),
    "rim": ("MattLange", "MattLange44.zip", "MattLange44/rim mono 1.wav", "Matt Lange", "rim shot", False),
    # --- Instrumente mit Ton
    "harp": ("LisaLim", "LisaLim44.zip", "LisaLim44/harpA69.wav", "Lisa Lim", "harp, one note", True),
    "koto": ("LisaLim", "LisaLim44.zip", "LisaLim44/koto-guzhengD61.wav", "Lisa Lim", "koto / guzheng, one note", True),
    "shakuhachi": ("LisaLim", "LisaLim44.zip", "LisaLim44/shakuhachiA69withHarmonics.wav", "Lisa Lim", "shakuhachi, one note with harmonics", True),
    "dulcimer": ("LisaLim", "LisaLim44.zip", "LisaLim44/hammeredDulcimerA57.wav", "Lisa Lim", "hammered dulcimer, one note", True),
    "thumb_piano": ("Berklee44Forsyth", "Berklee44Forsyth.zip", "Berklee44Forsyth/thumbpiano_do.wav", "Berklee (Forsyth)", "thumb piano, one note", True),
    "sax_alto": ("OpenPathMusic44V2", "OpenPathMusic44V2.zip", "OpenPathMusic44V2/saxophone-alto-C-octave1-vib.wav", "OpenPathMusic", "alto saxophone, one note with vibrato", True),
    "flute_alto": ("OpenPathMusic44V2", "OpenPathMusic44V2.zip", "OpenPathMusic44V2/flute-alto-C-vib.wav", "OpenPathMusic", "alto flute, one note with vibrato", True),
}

NOTEN = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]


def lies(datei):
    rate, x = wavfile.read(datei)
    if rate != RATE:
        raise SystemExit(f"{datei.name}: {rate} Hz, erwartet {RATE}")
    x = x.astype(np.float64)
    if x.ndim == 2:
        x = x.mean(axis=1)
    return x / 32768.0


def schneide(x):
    schwelle = 10 ** (-50 / 20) * np.abs(x).max()
    laut = np.nonzero(np.abs(x) > schwelle)[0]
    anfang = max(0, laut[0] - int(0.002 * RATE))
    ende = min(len(x), laut[-1] + 1)
    y = x[anfang:ende].copy()
    aus = min(len(y), int(0.010 * RATE))
    y[-aus:] *= np.linspace(1.0, 0.0, aus)
    return y * (SPITZE / np.abs(y).max())


def grundton(x):
    """Grundton (MIDI, mit Bruchteil) aus der Mitte des Klangs: Autokorrelation mit YIN-Normierung."""
    mitte = len(x) // 2
    stueck = x[max(0, mitte - 4096):mitte + 4096]
    stueck = stueck - stueck.mean()
    n = len(stueck)
    kmin, kmax = int(RATE / 1500), int(RATE / 55)
    d = np.array([np.sum((stueck[:n - k] - stueck[k:]) ** 2) for k in range(kmax + 1)])
    cmnd = np.ones_like(d)
    cmnd[1:] = d[1:] * np.arange(1, len(d)) / np.maximum(np.cumsum(d[1:]), 1e-12)
    k = kmin + int(np.argmin(cmnd[kmin:]))
    for kk in range(kmin, kmax):  # das erste Tal unter 0,15 — sonst greift man leicht eine Oktave zu tief
        if cmnd[kk] < 0.15:
            k = kk
            while k + 1 < kmax and cmnd[k + 1] < cmnd[k]:
                k += 1
            break
    if 1 <= k < len(cmnd) - 1:  # parabolisch verfeinern
        a, b, c = cmnd[k - 1], cmnd[k], cmnd[k + 1]
        k = k + 0.5 * (a - c) / (a - 2 * b + c) if (a - 2 * b + c) != 0 else k
    return 69 + 12 * np.log2(RATE / k / 440.0)


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    roh = Path(sys.argv[1])
    zeilen = []
    for name, (eintrag, zip_, pfad, urheber, was, ton) in QUELLEN.items():
        original = roh / f"{eintrag}__{Path(pfad).stem.replace(' ', '_')}.wav"
        if not original.is_file():
            # die Sitzung S123 hat zwei Namen beim Ablegen gekuerzt
            ersatz = {"conga_high": "conga_high_open", "rim": "rim_mono_1"}.get(name)
            original = roh / f"{eintrag}__{ersatz}.wav" if ersatz else original
        y = schneide(lies(original))
        wavfile.write(HIER / f"{name}.wav", RATE, np.round(y * 32767).astype(np.int16))
        info = f"{len(y) / RATE:5.2f} s"
        if ton:
            g = grundton(y)
            info += f", Ton {NOTEN[int(round(g)) % 12]}{int(round(g)) // 12 - 1} (MIDI {g:.2f})"
        print(f"{name:20s} {info}")
        zeilen.append(f"| `{name}.wav` | {was} | {urheber} | [{eintrag}](https://archive.org/details/{eintrag}), "
                      f"`{zip_}` → `{pfad}` |")
    # was ab der Marke steht, schreibt make_drumgizmo.py — es bleibt stehen
    marke = "<!-- DrumGizmo: ab hier schreibt make_drumgizmo.py -->"
    alt = (HIER / "CREDITS.md").read_text(encoding="utf-8") if (HIER / "CREDITS.md").exists() else ""
    hinten = ("\n" + marke + alt.split(marke, 1)[1]) if marke in alt else ""
    (HIER / "CREDITS.md").write_text(
        "# Samples — sources and credits\n\n"
        "## OLPC Sound Samples\n\n"
        "These sounds come from the **OLPC Sound Samples** collection (One Laptop per Child music library,\n"
        "compiled by Richard Boulanger and many contributors at Berklee), published on archive.org under\n"
        "**Creative Commons Attribution 3.0** (<https://creativecommons.org/licenses/by/3.0/>).\n\n"
        "LumiViz changed them: silence at the ends removed, a 10 ms fade at the end, the level raised to a\n"
        "peak of 0.89, written as mono, 16 bit, 44100 Hz. In LumiViz a sample `name.wav` is called `lib:name`.\n\n"
        "| File | What | By | Source |\n|---|---|---|---|\n" + "\n".join(zeilen) + "\n" + hinten,
        encoding="utf-8", newline="\n")
    print(len(QUELLEN), "Samples, CREDITS.md")


if __name__ == "__main__":
    main()
