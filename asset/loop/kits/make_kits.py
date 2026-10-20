"""Die mitgelieferten Kits schreiben (S123, Stufe 21a; Entscheide Patrik K3 a, K5 a, K6 a, K6b a).

    python asset/loop/kits/make_kits.py

Schreibt neben dieses Skript je Kit eine `.lvkit` (Fassung 1, `loop/DrumKit.hpp`). Der
Dateiname ist der Name im Loop: `808.lvkit` ist das Kit `808`. Kein Zufall: zwei Laeufe
ergeben dieselben Dateien.

Je Rolle ein Klang: `kit:` (das gerechnete Kit), `lib:` (die Samples unter `../samples`)
oder ein Instrument der Art Drums (`drum_*`, auf C2 gebaut; Note 36, Length 250).

**Volume** ist gemessen, nicht gehoert — seit Stufe 27d (S139, Entscheide Patrik LL4 a, LL9 a,
LL10 a, LL11 a; docs/audio/Loop_Konzept.md §4n) nach EBU R128: die **Kick** jedes Kits kommt
mit der Drum-Probe (`loop/Pegel.hpp`, Short-term-Maximum, wie die Instrumente) auf
`DRUM_ANKER_LUFS`; jede andere Rolle kommt als **Schlag** (Momentary-Maximum) auf die Kick
ihres Kits plus `ABSTAND_DB` ihrer Art — so, wie die Kits vor 27d klangen. Volume ist
hoechstens 1; was darueber muesste, wird gekappt und am Ende genannt (LL11 a).

`PEGEL` ist, was der Fall „Loop Kits: Pegel" in `test_LoopKits.cpp` je Klang bei Volume 1
gemessen hat. Wer einen Klang tauscht oder aendert:

    LUMIVIZ_KIT_PEGEL_DATEI=<datei.tsv> LumiViz.UnitTests --test-case="Loop Kits: Pegel*"
    python asset/loop/kits/make_kits.py --messung <datei.tsv>

`--messung` traegt die Werte hier ein (beim ersten Lauf auch den Anker: der Median der
Kicks, wie sie vor 27d standen) und schreibt die Kits. Ohne Argument schreibt das Skript die
Kits aus der Tabelle.

Seit Stufe 21d (S124) dazu Rock'n'Roll, Hardrock, Heavy Metal und Jazz Brushes: akustische
Aufnahmen aus DrumGizmo (`lib:drs_*`, `lib:crocell_*`, `../samples/make_drumgizmo.py`, Stereo) und
drei gerechnete Besen (`drum_brush_*`, `../make_drums.py`). Ein Stereo-Sample traegt seine Lage im
Raum schon (die Overheads): Hats und Becken stehen dort auf Pan 0, nur die Toms werden verteilt.
"""
import argparse
import json
import math
import pathlib
import re
import statistics

ORDNER = pathlib.Path(__file__).parent

# Der Anker (LL10 a): so laut misst die Kick jedes Kits auf der Drum-Probe (LUFS S max) — der Median
# der Kicks vor Stufe 27d; `--messung` setzt ihn beim ersten Lauf und laesst ihn danach stehen.
DRUM_ANKER_LUFS = -21.89

# Der Abstand je Art zur Kick des Kits, als Schlag (LU, M max; LL9 a): der Median ueber die Kits,
# wie sie vor Stufe 27d klangen (gemessen S139, Release-clang).
ABSTAND_DB = {
    "kick": 0.0, "snare": 1.0, "clap": -1.2, "rim": -2.9, "hat": -2.4, "open hat": 4.0,
    "ride": -1.6, "crash": 6.4, "tom": 3.6, "perc": 0.5, "fx": 2.6,
}

# --- PEGEL: geschrieben von --messung; Klang: (Schlag M max, Probe S max), bei Volume 1 ---
PEGEL = {
    "drum_beatbox_kick": (-20.4, -21.37),
    "drum_brush_slap": (-14.89, -15.86),
    "drum_brush_sweep": (-16.71, -17.61),
    "drum_brush_tap": (-17.35, -18.32),
    "drum_clap": (-18.37, -19.34),
    "drum_clap_stack": (-20.93, -21.9),
    "drum_crash": (-7.18, -7.93),
    "drum_hat_closed": (-21.3, -22.27),
    "drum_hat_open": (-13.13, -14.07),
    "drum_hat_shake": (-23.61, -24.58),
    "drum_kick": (-13.49, -14.43),
    "drum_kick_808": (-10.89, -11.64),
    "drum_kick_punch": (-19.0, -19.97),
    "drum_ride": (-16.61, -17.37),
    "drum_shaker": (-18.55, -19.52),
    "drum_snare_fat": (-18.07, -19.04),
    "drum_tom_high": (-15.57, -16.53),
    "drum_tom_low": (-14.74, -15.69),
    "kit:clap": (-14.66, -15.63),
    "kit:hat_closed": (-18.72, -19.69),
    "kit:hat_open": (-10.74, -11.69),
    "kit:kick": (-8.69, -9.64),
    "kit:snare": (-13.6, -14.56),
    "kit:tom": (-10.23, -11.2),
    "lib:beatbox_hat_closed": (-12.17, -13.14),
    "lib:beatbox_hat_open": (-4.52, -5.49),
    "lib:beatbox_snare": (-14.71, -15.67),
    "lib:clap_707": (-17.52, -18.49),
    "lib:clave": (-15.96, -16.93),
    "lib:conga_high": (-20.47, -21.44),
    "lib:cowbell": (-17.34, -18.31),
    "lib:crocell_china": (-12.14, -13.0),
    "lib:crocell_crash1": (-10.39, -11.08),
    "lib:crocell_crash2": (-8.5, -9.07),
    "lib:crocell_crash3": (-7.73, -8.23),
    "lib:crocell_hat": (-15.52, -16.48),
    "lib:crocell_hat_open": (-10.31, -11.09),
    "lib:crocell_kick_left": (-18.27, -19.24),
    "lib:crocell_kick_right": (-18.94, -19.91),
    "lib:crocell_ride": (-20.82, -21.68),
    "lib:crocell_snare": (-18.2, -19.16),
    "lib:crocell_splash": (-10.46, -11.24),
    "lib:crocell_tom1": (-12.57, -13.47),
    "lib:crocell_tom2": (-14.3, -15.19),
    "lib:crocell_tom3": (-17.06, -17.97),
    "lib:crocell_tom4": (-15.86, -16.74),
    "lib:darbuka_doom": (-12.25, -13.16),
    "lib:darbuka_tak": (-18.02, -18.98),
    "lib:djembe_low": (-13.26, -14.2),
    "lib:djembe_slap": (-13.84, -14.8),
    "lib:drs_crash_left": (-20.02, -20.58),
    "lib:drs_crash_left_hard": (-14.31, -14.79),
    "lib:drs_crash_right_shank_hard": (-8.46, -9.1),
    "lib:drs_hat": (-14.43, -15.39),
    "lib:drs_hat_foot": (-16.07, -17.04),
    "lib:drs_hat_hard": (-14.17, -15.14),
    "lib:drs_hat_open": (-9.42, -9.93),
    "lib:drs_hat_open_hard": (-10.69, -11.36),
    "lib:drs_kick": (-19.03, -19.99),
    "lib:drs_kick_hard": (-18.79, -19.75),
    "lib:drs_kick_soft": (-17.52, -18.47),
    "lib:drs_ride": (-12.2, -12.87),
    "lib:drs_ride_soft": (-12.42, -13.14),
    "lib:drs_rim": (-21.77, -22.74),
    "lib:drs_snare": (-18.47, -19.44),
    "lib:drs_snare_hard": (-19.73, -20.7),
    "lib:drs_tom1": (-14.94, -15.85),
    "lib:drs_tom1_hard": (-16.04, -16.96),
    "lib:drs_tom2": (-16.64, -17.45),
    "lib:drs_tom2_hard": (-15.22, -16.05),
    "lib:drs_tom3": (-17.28, -18.15),
    "lib:drs_tom3_hard": (-17.16, -18.07),
    "lib:hat_707_open": (-10.87, -11.84),
    "lib:hat_808_closed": (-20.13, -21.1),
    "lib:kick_909": (-14.02, -14.99),
    "lib:kick_deep": (-12.58, -13.44),
    "lib:rim": (-19.7, -20.67),
    "lib:snap": (-25.36, -26.33),
    "lib:snare_808": (-15.15, -16.12),
    "lib:snare_909": (-11.38, -12.35),
    "lib:tambourine": (-16.75, -17.72),
    "lib:voice_female_gliss": (-5.42, -6.35),
    "lib:voice_male_hey": (-11.3, -12.27),
    "shape_atom_bell": (-15.56, -16.16),
    "shape_cat": (-14.24, -15.09),
}
# --- Ende PEGEL ---

GEKAPPT = []  # (Kit, Rolle, fehlt in dB) — am Ende genannt

PAN = {"hat": 0.2, "open hat": 0.2, "ride": 0.25, "crash": -0.15}
PAN_NUMMERIERT = {"tom": (-0.25, 0.25, 0.0, 0.1), "perc": (0.3, -0.3, 0.15, -0.15)}


def art_von(rolle):
    teile = rolle.split(" ")
    return " ".join(teile[:-1]) if teile[-1].isdigit() else rolle


def ist_stereo(klang):
    """Die akustischen Aufnahmen aus DrumGizmo: Stereo mit dem Bild der Overheads."""
    return klang.startswith(("lib:drs_", "lib:crocell_"))


def eintrag(rolle, klang, volume):
    art = art_von(rolle)
    r = {"role": rolle}
    if klang.startswith(("kit:", "lib:")):
        r["sound"] = klang
    else:  # ein Instrument der Art Drums
        r["sound"] = ""
        r["instrument"] = klang
        r["note"] = 36
        r["length"] = 250
    r["volume"] = round(volume, 3)
    if art in PAN_NUMMERIERT:
        nummer = int(rolle.split(" ")[-1])
        r["pan"] = PAN_NUMMERIERT[art][(nummer - 1) % 4]
    else:
        r["pan"] = 0.0 if ist_stereo(klang) else PAN.get(art, 0.0)
    return r


def volumes(name, rollen):
    """Je Rolle das Volume (s. oben): die Kick auf den Anker, die anderen auf die Kick plus Abstand."""
    if DRUM_ANKER_LUFS is None or not PEGEL:
        return {rolle: 0.8 for rolle, _ in rollen}  # noch nicht gemessen
    kick = dict(rollen)["kick"]
    kick_volume = min(1.0, 10 ** ((DRUM_ANKER_LUFS - PEGEL[kick][1]) / 20))
    if kick_volume >= 1.0 and PEGEL[kick][1] < DRUM_ANKER_LUFS:
        GEKAPPT.append((name, "kick", DRUM_ANKER_LUFS - PEGEL[kick][1]))
    kick_schlag = PEGEL[kick][0] + 20 * math.log10(kick_volume)
    ergebnis = {}
    for rolle, klang in rollen:
        if rolle == "kick":
            ergebnis[rolle] = kick_volume
            continue
        ziel = kick_schlag + ABSTAND_DB[art_von(rolle)]
        volume = 10 ** ((ziel - PEGEL[klang][0]) / 20)
        if volume > 1.0:
            GEKAPPT.append((name, rolle, 20 * math.log10(volume)))
        ergebnis[rolle] = min(1.0, volume)
    return ergebnis


def kit(name, titel, beschreibung, rollen):
    vol = volumes(name, rollen)
    return {"format": "lumiviz-kit", "version": 1, "title": titel, "description": beschreibung,
            "roles": [eintrag(rolle, klang, vol[rolle]) for rolle, klang in rollen]}


def baue_kits():
    return {
        "classic": kit("classic", "Classic", "The six computed sounds of the original kit.", [
            ("kick", "kit:kick"), ("snare", "kit:snare"), ("clap", "kit:clap"), ("hat", "kit:hat_closed"),
            ("open hat", "kit:hat_open"), ("tom 1", "kit:tom")]),
        "808": kit("808", "808", "A long 808-style kick, 808 snare and hat, computed toms and crash.", [
            ("kick", "drum_kick_808"), ("snare", "lib:snare_808"), ("clap", "drum_clap"), ("hat", "lib:hat_808_closed"),
            ("open hat", "drum_hat_open"), ("tom 1", "drum_tom_low"), ("tom 2", "drum_tom_high"), ("crash", "drum_crash")]),
        "909_house": kit("909_house", "909 House", "909-style kick and snare, a 707 clap and open hat, ride and tambourine.", [
            ("kick", "lib:kick_909"), ("snare", "lib:snare_909"), ("clap", "lib:clap_707"), ("hat", "drum_hat_closed"),
            ("open hat", "lib:hat_707_open"), ("ride", "drum_ride"), ("crash", "drum_crash"), ("perc 1", "lib:tambourine")]),
        "electro": kit("electro", "Electro", "Layered drums: punch kick, fat snare, clap stack and hat shake.", [
            ("kick", "drum_kick_punch"), ("snare", "drum_snare_fat"), ("clap", "drum_clap_stack"), ("hat", "drum_hat_shake"),
            ("open hat", "drum_hat_open"), ("tom 1", "drum_tom_low"), ("tom 2", "drum_tom_high"), ("crash", "drum_crash")]),
        # S128 (Entscheid Patrik C2 a): Hand drums mit Clave als perc 4, Reggaeton mit Cowbell als perc 3
        "hand_drums": kit("hand_drums", "Hand drums", "Djembe, conga, darbuka, claves, tambourine and shaker; kick and snare are the low and the bright stroke.", [
            ("kick", "lib:djembe_low"), ("snare", "lib:djembe_slap"), ("hat", "drum_shaker"), ("open hat", "lib:tambourine"),
            ("perc 1", "lib:conga_high"), ("perc 2", "lib:darbuka_doom"), ("perc 3", "lib:darbuka_tak"), ("perc 4", "lib:clave")]),
        "beatbox": kit("beatbox", "Beatbox", "A recorded human beatbox with a sine sub under the kick, and a shouted hey.", [
            ("kick", "drum_beatbox_kick"), ("snare", "lib:beatbox_snare"), ("hat", "lib:beatbox_hat_closed"),
            ("open hat", "lib:beatbox_hat_open"), ("fx", "lib:voice_male_hey")]),
        "lo_fi": kit("lo_fi", "Lo-fi", "A deep kick, 808 snare, rim shot and finger snap, with tambourine.", [
            ("kick", "lib:kick_deep"), ("snare", "lib:snare_808"), ("rim", "lib:rim"), ("clap", "lib:snap"),
            ("hat", "lib:hat_808_closed"), ("open hat", "drum_hat_open"), ("perc 1", "lib:tambourine")]),
        "reggae": kit("reggae", "Reggae", "For the one drop: kick and rim shot together on the three.", [
            ("kick", "drum_kick"), ("snare", "kit:snare"), ("rim", "lib:rim"), ("hat", "lib:hat_808_closed"),
            ("open hat", "drum_hat_open"), ("perc 1", "lib:tambourine"), ("tom 1", "drum_tom_high")]),
        "reggaeton": kit("reggaeton", "Reggaeton", "For the dembow: 808 kick and snare, a 707 clap, conga, rim and cowbell.", [
            ("kick", "drum_kick_808"), ("snare", "lib:snare_808"), ("clap", "lib:clap_707"), ("hat", "drum_hat_closed"),
            ("open hat", "lib:hat_707_open"), ("perc 1", "lib:conga_high"), ("perc 2", "lib:rim"), ("perc 3", "lib:cowbell")]),
        "strange": kit("strange", "Strange", "Beatbox, snaps, a gliding voice, a cat and a bell made of shapes.", [
            ("kick", "drum_beatbox_kick"), ("snare", "lib:snap"), ("hat", "lib:beatbox_hat_closed"), ("open hat", "drum_crash"),
            ("perc 1", "shape_cat"), ("perc 2", "lib:voice_female_gliss"), ("fx", "lib:voice_male_hey"),
            ("tom 1", "shape_atom_bell")]),
        # --- S124, Stufe 21d (K6b a, K6c a)
        "rock_n_roll": kit("rock_n_roll", "Rock'n'Roll", "An acoustic kit (DRSKit, DrumGizmo), played medium: cross-stick, three toms, ride and crash.", [
            ("kick", "lib:drs_kick"), ("snare", "lib:drs_snare"), ("rim", "lib:drs_rim"), ("hat", "lib:drs_hat"),
            ("open hat", "lib:drs_hat_open"), ("ride", "lib:drs_ride"), ("crash", "lib:drs_crash_left"),
            ("tom 1", "lib:drs_tom1"), ("tom 2", "lib:drs_tom2"), ("tom 3", "lib:drs_tom3")]),
        "hardrock": kit("hardrock", "Hardrock", "The same acoustic kit (DRSKit, DrumGizmo), hit hard, with two crashes.", [
            ("kick", "lib:drs_kick_hard"), ("snare", "lib:drs_snare_hard"), ("hat", "lib:drs_hat_hard"),
            ("open hat", "lib:drs_hat_open_hard"), ("ride", "lib:drs_ride"), ("crash", "lib:drs_crash_left_hard"),
            ("crash 2", "lib:drs_crash_right_shank_hard"), ("tom 1", "lib:drs_tom1_hard"), ("tom 2", "lib:drs_tom2_hard"),
            ("tom 3", "lib:drs_tom3_hard")]),
        "heavy_metal": kit("heavy_metal", "Heavy Metal", "A metal band's kit (CrocellKit, DrumGizmo): double pedal, four toms, three crashes, china and splash.", [
            ("kick", "lib:crocell_kick_left"), ("kick 2", "lib:crocell_kick_right"), ("snare", "lib:crocell_snare"),
            ("hat", "lib:crocell_hat"), ("open hat", "lib:crocell_hat_open"), ("ride", "lib:crocell_ride"),
            ("crash", "lib:crocell_crash1"), ("crash 2", "lib:crocell_crash2"), ("crash 3", "lib:crocell_crash3"),
            ("tom 1", "lib:crocell_tom1"), ("tom 2", "lib:crocell_tom2"), ("tom 3", "lib:crocell_tom3"),
            ("tom 4", "lib:crocell_tom4"), ("perc 1", "lib:crocell_splash"), ("fx", "lib:crocell_china")]),
        "jazz_brushes": kit("jazz_brushes", "Jazz Brushes", "Computed brushes on the snare (tap, slap, sweep) with a soft acoustic kick, hi-hat foot and ride (DRSKit).", [
            ("kick", "lib:drs_kick_soft"), ("snare", "drum_brush_tap"), ("snare 2", "drum_brush_slap"),
            ("hat", "lib:drs_hat_foot"), ("ride", "lib:drs_ride_soft"), ("perc 1", "drum_brush_sweep")]),
    }


# Wo ein Instrument nicht auf C2 gebaut ist, steht die Note hier (MIDI)
NOTEN = {("strange", "perc 1"): 64, ("strange", "tom 1"): 48}
# Wo ein Instrument laenger gehalten wird als 250 ms (Length, ms): der Besen kreist einen halben Takt
LAENGEN = {("jazz_brushes", "perc 1"): 500}


def lies_messung(tsv):
    """Die TSV aus test_LoopKits: je Klang (Schlag M max, Probe S max) und je Kit die Kick mit Volume."""
    pegel, kicks = {}, []
    for zeile in pathlib.Path(tsv).read_text(encoding="utf-8").splitlines():
        if zeile.startswith("#") or not zeile.strip():
            continue
        kit_name, rolle, klang, volume, schlag, probe = zeile.split("\t")
        pegel[klang] = (round(float(schlag), 2), round(float(probe), 2))
        if rolle == "kick":
            kicks.append(float(probe) + 20 * math.log10(float(volume)))
    return pegel, kicks


def schreibe_tabellen(pegel, anker):
    datei = pathlib.Path(__file__)
    text = datei.read_text(encoding="utf-8")
    zeilen = "".join(f"    {json.dumps(k)}: {v!r},\n" for k, v in sorted(pegel.items()))
    text, n = re.subn(r"(# --- PEGEL: [^\n]*\nPEGEL = \{\n).*?(\}\n# --- Ende PEGEL ---)",
                      lambda m: m.group(1) + zeilen + m.group(2), text, flags=re.S)
    text, n2 = re.subn(r"\nDRUM_ANKER_LUFS = [^\n]*\n", f"\nDRUM_ANKER_LUFS = {anker!r}\n", text, count=1)
    if n != 1 or n2 != 1:
        raise SystemExit("Marken nicht gefunden")
    datei.write_text(text, encoding="utf-8", newline="\n")


def main():
    global DRUM_ANKER_LUFS
    teile = argparse.ArgumentParser(description="Die mitgelieferten Kits schreiben")
    teile.add_argument("--messung", help="TSV aus test_LoopKits (LUMIVIZ_KIT_PEGEL_DATEI)")
    teile.add_argument("--ordner", default=str(ORDNER), help="wohin die .lvkit (fuer eine Kopie)")
    args = teile.parse_args()
    if args.messung:
        pegel, kicks = lies_messung(args.messung)
        if DRUM_ANKER_LUFS is None:
            DRUM_ANKER_LUFS = round(statistics.median(kicks), 2)
        PEGEL.clear()
        PEGEL.update(pegel)
        schreibe_tabellen(PEGEL, DRUM_ANKER_LUFS)
    kits = baue_kits()
    ordner = pathlib.Path(args.ordner)
    for name, daten in kits.items():
        for r in daten["roles"]:
            if (name, r["role"]) in NOTEN:
                r["note"] = NOTEN[(name, r["role"])]
                r["length"] = 400
            if (name, r["role"]) in LAENGEN:
                r["length"] = LAENGEN[(name, r["role"])]
        (ordner / f"{name}.lvkit").write_text(json.dumps(daten, indent=4) + "\n", encoding="utf-8", newline="\n")
    fehlt = sorted({k for d in kits.values() for k in (r.get("instrument") or r["sound"] for r in d["roles"])} - set(PEGEL))
    print(len(kits), "Kits; Anker", DRUM_ANKER_LUFS, "LUFS;", "ohne Messung:", ", ".join(fehlt) if fehlt else "keiner")
    for name, rolle, fehlt_db in GEKAPPT:
        print(f"  gekappt: {name}, {rolle} — fehlen {fehlt_db:.1f} dB")
    print(len(GEKAPPT), "Rollen gekappt")


if __name__ == "__main__":
    main()
