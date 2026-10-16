"""Die mitgelieferten Kits schreiben (S123, Stufe 21a; Entscheide Patrik K3 a, K5 a, K6 a, K6b a).

    python asset/loop/kits/make_kits.py

Schreibt neben dieses Skript je Kit eine `.lvkit` (Fassung 1, `loop/DrumKit.hpp`). Der
Dateiname ist der Name im Loop: `808.lvkit` ist das Kit `808`. Kein Zufall: zwei Laeufe
ergeben dieselben Dateien.

Je Rolle ein Klang: `kit:` (das gerechnete Kit), `lib:` (die Samples unter `../samples`)
oder ein Instrument der Art Drums (`drum_*`, auf C2 gebaut; Note 36, Length 250).

**Volume** ist gemessen, nicht gehoert: `ZIEL_DB` je Rolle ist die Lautheit, auf die jeder
Klang gebracht wird; `PEGEL_DB` ist, was der Fall „Loop Kits: Pegel" in `test_LoopKits.cpp`
fuer den Klang bei Volume 1 gemessen hat (Effektivwert der ersten 300 ms, dB). Volume =
10^((ZIEL - PEGEL) / 20), hoechstens 1. Wer einen Klang tauscht, misst neu
(`LUMIVIZ_KIT_PEGEL=1`) und traegt den Wert hier ein.

Seit Stufe 21d (S124) dazu Rock'n'Roll, Hardrock, Heavy Metal und Jazz Brushes: akustische
Aufnahmen aus DrumGizmo (`lib:drs_*`, `lib:crocell_*`, `../samples/make_drumgizmo.py`, Stereo) und
drei gerechnete Besen (`drum_brush_*`, `../make_drums.py`). Ein Stereo-Sample traegt seine Lage im
Raum schon (die Overheads): Hats und Becken stehen dort auf Pan 0, nur die Toms werden verteilt.
"""
import json
import math
import pathlib

ORDNER = pathlib.Path(__file__).parent

# Lautheit je Rolle (dB, Effektivwert der ersten 300 ms) — knapp unter dem leisesten Klang der
# Rolle, damit fast alle sie mit Volume <= 1 erreichen; die Kick vorn, Hats und Ride leiser.
# Nicht erreicht: der Snap als Snare im Kit Strange (-26,9 dB bei Volume 1).
ZIEL_DB = {
    "kick": -20.0, "snare": -22.0, "clap": -24.0, "rim": -22.0, "hat": -27.0, "open hat": -22.0,
    "ride": -25.0, "crash": -20.0, "tom": -18.0, "perc": -23.0, "fx": -20.0,
}

# gemessen bei Volume 1 (test_LoopKits.cpp, „Loop Kits: Pegel", LUMIVIZ_KIT_PEGEL=1; S123, Testing-MSVC)
PEGEL_DB = {
    "drum_beatbox_kick": -20.5, "drum_clap": -21.9, "drum_clap_stack": -24.3, "drum_crash": -12.7,
    "drum_hat_closed": -26.4, "drum_hat_open": -18.3, "drum_hat_shake": -28.7, "drum_kick": -12.3,
    "drum_kick_808": -8.9, "drum_kick_punch": -18.2, "drum_ride": -22.1, "drum_shaker": -23.6,
    "drum_snare_fat": -20.1, "drum_tom_high": -16.3, "drum_tom_low": -14.6,
    "kit:clap": -18.2, "kit:hat_closed": -18.6, "kit:hat_open": -15.9, "kit:kick": -7.3, "kit:snare": -17.5,
    "kit:tom": -10.7,
    "lib:beatbox_hat_closed": -17.0, "lib:beatbox_hat_open": -9.7, "lib:beatbox_snare": -17.1, "lib:clap_707": -17.2,
    "lib:clave": -19.5, "lib:cowbell": -19.2,  # S128, Release-clang
    "lib:conga_high": -21.7,"lib:darbuka_doom": -12.8, "lib:darbuka_tak": -17.6, "lib:djembe_low": -12.3,
    "lib:djembe_slap": -14.6, "lib:hat_707_open": -15.9, "lib:hat_808_closed": -19.7, "lib:kick_909": -13.1,
    "lib:kick_deep": -11.3, "lib:rim": -17.5, "lib:snap": -26.9, "lib:snare_808": -16.9, "lib:snare_909": -12.9,
    "lib:tambourine": -21.2, "lib:voice_female_gliss": -7.6, "lib:voice_male_hey": -13.5,
    "shape_atom_bell": -15.6, "shape_cat": -16.0,
    # S124, Stufe 21d: DrumGizmo (Stereo) und die Besen
    "drum_brush_slap": -19.7, "drum_brush_sweep": -22.0, "drum_brush_tap": -22.2,
    "lib:crocell_china": -17.1, "lib:crocell_crash1": -15.7, "lib:crocell_crash2": -14.1, "lib:crocell_crash3": -14.0,
    "lib:crocell_hat": -19.9, "lib:crocell_hat_open": -15.1, "lib:crocell_kick_left": -15.9, "lib:crocell_kick_right": -16.5,
    "lib:crocell_ride": -25.9, "lib:crocell_snare": -19.3, "lib:crocell_splash": -15.6, "lib:crocell_tom1": -13.1,
    "lib:crocell_tom2": -14.7, "lib:crocell_tom3": -17.5, "lib:crocell_tom4": -15.8,
    "lib:drs_crash_left": -24.3, "lib:drs_crash_left_hard": -19.1, "lib:drs_crash_right_shank_hard": -13.9, "lib:drs_hat": -19.2,
    "lib:drs_hat_foot": -20.2, "lib:drs_hat_hard": -18.9, "lib:drs_hat_open": -14.4, "lib:drs_hat_open_hard": -15.8,
    "lib:drs_kick": -19.2, "lib:drs_kick_hard": -19.0, "lib:drs_kick_soft": -17.5, "lib:drs_ride": -15.5,
    "lib:drs_ride_soft": -15.8, "lib:drs_rim": -23.8, "lib:drs_snare": -19.6, "lib:drs_snare_hard": -21.0,
    "lib:drs_tom1": -15.7, "lib:drs_tom1_hard": -16.8, "lib:drs_tom2": -17.0, "lib:drs_tom2_hard": -15.7,
    "lib:drs_tom3": -17.7, "lib:drs_tom3_hard": -17.6,
}

PAN = {"hat": 0.2, "open hat": 0.2, "ride": 0.25, "crash": -0.15}
PAN_NUMMERIERT = {"tom": (-0.25, 0.25, 0.0, 0.1), "perc": (0.3, -0.3, 0.15, -0.15)}


def art_von(rolle):
    teile = rolle.split(" ")
    return " ".join(teile[:-1]) if teile[-1].isdigit() else rolle


def ist_stereo(klang):
    """Die akustischen Aufnahmen aus DrumGizmo: Stereo mit dem Bild der Overheads."""
    return klang.startswith(("lib:drs_", "lib:crocell_"))


def eintrag(rolle, klang):
    art = art_von(rolle)
    r = {"role": rolle}
    if klang.startswith(("kit:", "lib:")):
        r["sound"] = klang
    else:  # ein Instrument der Art Drums
        r["sound"] = ""
        r["instrument"] = klang
        r["note"] = 36
        r["length"] = 250
    pegel = PEGEL_DB.get(klang)
    volume = 0.8 if pegel is None else min(1.0, 10 ** ((ZIEL_DB[art] - pegel) / 20))
    r["volume"] = round(volume, 3)
    if art in PAN_NUMMERIERT:
        nummer = int(rolle.split(" ")[-1])
        r["pan"] = PAN_NUMMERIERT[art][(nummer - 1) % 4]
    else:
        r["pan"] = 0.0 if ist_stereo(klang) else PAN.get(art, 0.0)
    return r


def kit(titel, beschreibung, rollen):
    return {"format": "lumiviz-kit", "version": 1, "title": titel, "description": beschreibung,
            "roles": [eintrag(rolle, klang) for rolle, klang in rollen]}


KITS = {
    "classic": kit("Classic", "The six computed sounds of the original kit.", [
        ("kick", "kit:kick"), ("snare", "kit:snare"), ("clap", "kit:clap"), ("hat", "kit:hat_closed"),
        ("open hat", "kit:hat_open"), ("tom 1", "kit:tom")]),
    "808": kit("808", "A long 808-style kick, 808 snare and hat, computed toms and crash.", [
        ("kick", "drum_kick_808"), ("snare", "lib:snare_808"), ("clap", "drum_clap"), ("hat", "lib:hat_808_closed"),
        ("open hat", "drum_hat_open"), ("tom 1", "drum_tom_low"), ("tom 2", "drum_tom_high"), ("crash", "drum_crash")]),
    "909_house": kit("909 House", "909-style kick and snare, a 707 clap and open hat, ride and tambourine.", [
        ("kick", "lib:kick_909"), ("snare", "lib:snare_909"), ("clap", "lib:clap_707"), ("hat", "drum_hat_closed"),
        ("open hat", "lib:hat_707_open"), ("ride", "drum_ride"), ("crash", "drum_crash"), ("perc 1", "lib:tambourine")]),
    "electro": kit("Electro", "Layered drums: punch kick, fat snare, clap stack and hat shake.", [
        ("kick", "drum_kick_punch"), ("snare", "drum_snare_fat"), ("clap", "drum_clap_stack"), ("hat", "drum_hat_shake"),
        ("open hat", "drum_hat_open"), ("tom 1", "drum_tom_low"), ("tom 2", "drum_tom_high"), ("crash", "drum_crash")]),
    # S128 (Entscheid Patrik C2 a): Hand drums mit Clave als perc 4, Reggaeton mit Cowbell als perc 3
    "hand_drums": kit("Hand drums", "Djembe, conga, darbuka, claves, tambourine and shaker; kick and snare are the low and the bright stroke.", [
        ("kick", "lib:djembe_low"), ("snare", "lib:djembe_slap"), ("hat", "drum_shaker"), ("open hat", "lib:tambourine"),
        ("perc 1", "lib:conga_high"), ("perc 2", "lib:darbuka_doom"), ("perc 3", "lib:darbuka_tak"), ("perc 4", "lib:clave")]),
    "beatbox": kit("Beatbox", "A recorded human beatbox with a sine sub under the kick, and a shouted hey.", [
        ("kick", "drum_beatbox_kick"), ("snare", "lib:beatbox_snare"), ("hat", "lib:beatbox_hat_closed"),
        ("open hat", "lib:beatbox_hat_open"), ("fx", "lib:voice_male_hey")]),
    "lo_fi": kit("Lo-fi", "A deep kick, 808 snare, rim shot and finger snap, with tambourine.", [
        ("kick", "lib:kick_deep"), ("snare", "lib:snare_808"), ("rim", "lib:rim"), ("clap", "lib:snap"),
        ("hat", "lib:hat_808_closed"), ("open hat", "drum_hat_open"), ("perc 1", "lib:tambourine")]),
    "reggae": kit("Reggae", "For the one drop: kick and rim shot together on the three.", [
        ("kick", "drum_kick"), ("snare", "kit:snare"), ("rim", "lib:rim"), ("hat", "lib:hat_808_closed"),
        ("open hat", "drum_hat_open"), ("perc 1", "lib:tambourine"), ("tom 1", "drum_tom_high")]),
    "reggaeton": kit("Reggaeton", "For the dembow: 808 kick and snare, a 707 clap, conga, rim and cowbell.", [
        ("kick", "drum_kick_808"), ("snare", "lib:snare_808"), ("clap", "lib:clap_707"), ("hat", "drum_hat_closed"),
        ("open hat", "lib:hat_707_open"), ("perc 1", "lib:conga_high"), ("perc 2", "lib:rim"), ("perc 3", "lib:cowbell")]),
    "strange": kit("Strange", "Beatbox, snaps, a gliding voice, a cat and a bell made of shapes.", [
        ("kick", "drum_beatbox_kick"), ("snare", "lib:snap"), ("hat", "lib:beatbox_hat_closed"), ("open hat", "drum_crash"),
        ("perc 1", "shape_cat"), ("perc 2", "lib:voice_female_gliss"), ("fx", "lib:voice_male_hey"),
        ("tom 1", "shape_atom_bell")]),
    # --- S124, Stufe 21d (K6b a, K6c a)
    "rock_n_roll": kit("Rock'n'Roll", "An acoustic kit (DRSKit, DrumGizmo), played medium: cross-stick, three toms, ride and crash.", [
        ("kick", "lib:drs_kick"), ("snare", "lib:drs_snare"), ("rim", "lib:drs_rim"), ("hat", "lib:drs_hat"),
        ("open hat", "lib:drs_hat_open"), ("ride", "lib:drs_ride"), ("crash", "lib:drs_crash_left"),
        ("tom 1", "lib:drs_tom1"), ("tom 2", "lib:drs_tom2"), ("tom 3", "lib:drs_tom3")]),
    "hardrock": kit("Hardrock", "The same acoustic kit (DRSKit, DrumGizmo), hit hard, with two crashes.", [
        ("kick", "lib:drs_kick_hard"), ("snare", "lib:drs_snare_hard"), ("hat", "lib:drs_hat_hard"),
        ("open hat", "lib:drs_hat_open_hard"), ("ride", "lib:drs_ride"), ("crash", "lib:drs_crash_left_hard"),
        ("crash 2", "lib:drs_crash_right_shank_hard"), ("tom 1", "lib:drs_tom1_hard"), ("tom 2", "lib:drs_tom2_hard"),
        ("tom 3", "lib:drs_tom3_hard")]),
    "heavy_metal": kit("Heavy Metal", "A metal band's kit (CrocellKit, DrumGizmo): double pedal, four toms, three crashes, china and splash.", [
        ("kick", "lib:crocell_kick_left"), ("kick 2", "lib:crocell_kick_right"), ("snare", "lib:crocell_snare"),
        ("hat", "lib:crocell_hat"), ("open hat", "lib:crocell_hat_open"), ("ride", "lib:crocell_ride"),
        ("crash", "lib:crocell_crash1"), ("crash 2", "lib:crocell_crash2"), ("crash 3", "lib:crocell_crash3"),
        ("tom 1", "lib:crocell_tom1"), ("tom 2", "lib:crocell_tom2"), ("tom 3", "lib:crocell_tom3"),
        ("tom 4", "lib:crocell_tom4"), ("perc 1", "lib:crocell_splash"), ("fx", "lib:crocell_china")]),
    "jazz_brushes": kit("Jazz Brushes", "Computed brushes on the snare (tap, slap, sweep) with a soft acoustic kick, hi-hat foot and ride (DRSKit).", [
        ("kick", "lib:drs_kick_soft"), ("snare", "drum_brush_tap"), ("snare 2", "drum_brush_slap"),
        ("hat", "lib:drs_hat_foot"), ("ride", "lib:drs_ride_soft"), ("perc 1", "drum_brush_sweep")]),
}

# Wo ein Instrument nicht auf C2 gebaut ist, steht die Note hier (MIDI)
NOTEN = {("strange", "perc 1"): 64, ("strange", "tom 1"): 48}
# Wo ein Instrument laenger gehalten wird als 250 ms (Length, ms): der Besen kreist einen halben Takt
LAENGEN = {("jazz_brushes", "perc 1"): 500}


def main():
    for name, daten in KITS.items():
        for r in daten["roles"]:
            if (name, r["role"]) in NOTEN:
                r["note"] = NOTEN[(name, r["role"])]
                r["length"] = 400
            if (name, r["role"]) in LAENGEN:
                r["length"] = LAENGEN[(name, r["role"])]
        (ORDNER / f"{name}.lvkit").write_text(json.dumps(daten, indent=4) + "\n", encoding="utf-8", newline="\n")
    fehlt = sorted({k for d in KITS.values() for k in (r.get("instrument") or r["sound"] for r in d["roles"])} - set(PEGEL_DB))
    print(len(KITS), "Kits;", "ohne Messung:", ", ".join(fehlt) if fehlt else "keiner")


if __name__ == "__main__":
    main()
