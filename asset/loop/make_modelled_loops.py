"""Die Loops der Instrumente mit Kern (S182; Entscheide Patrik IL1 a, IL2 a, IL3 a).

    python asset/loop/make_modelled_loops.py [--wurzel <kopie von asset/loop>]
    python asset/loop/instrument_pegel.py --loops [--wurzel <kopie>]
    python asset/loop/instrument_katalog.py

Schreibt unter `asset/loop/modelled_instruments/`:

- **je Instrument ein Loop der Nuancen** (IL1 a), `<instrument>.lvloop`, fuer jedes Instrument, das einen Kern
  rechnet (`grand_piano`, die Stabspiele aus 34g, die Trommeln aus 34h, die Tasten aus 34i). Kein Instrument dort heisst
  wie ein Beispiel (`*_demo`). Gleicher Aufbau, damit sich
  die Loops vergleichen lassen; 90 bpm, Sechzehntel:
  - Muster *Notes* (vier Takte, nur Instrumente mit mehr als einer Tonhoehe): Takt 1 **Lage** (acht Achtel vom unteren
    zum oberen Ende des Umfangs, auf C-Dur), Takt 2 **Staerke** (die Mitte in acht Stufen, 0,12 … 1), Takt 3 **Laenge**
    (Sechzehntel, Achtel, Viertel, Halbe), Takt 4 **Pedal und Expression** (ein Arpeggio in Sechzehnteln mit Sustain,
    dann ein Akkord, der auf der Spur Expression an- und abschwillt).
  - Muster *Strength* (ein Takt, nur Trommeln mit einer Tonhoehe): die Staerke in acht Stufen als Drum-Spur.
  - Muster *Techniques*: die **Spielweisen des Kerns** als Drum-Spuren mit festem `coreTracks` (IL2 a; TM2 a — Werte
    ueber der Zeit am Muster kommen mit 31g). Jede Spur ein Schlag auf einen halben Takt, die erste *Normal* ohne Spur
    zum Vergleich. Welche Spielweise ein Instrument zeigt, steht unten in `SPIELWEISEN`; jede ist durch den Fall
    `*Modelled loops*` in `test_LoopModelled.cpp` belegt: sie aendert den Ton.
  Die Loops der Nuancen tragen kein Genre (sie zaehlen im Katalog nicht).
- **das Ensemble** `modelled_ensemble.lvloop` (IL1 a): acht Takte, 100 bpm, Am – F – C – G zweimal; alle Instrumente oben
  zusammen, die Trommeln ueber das Kit `modelled_drums` (Rollen wie `make_membran_loops.py`), Genre *Fusion*.

Die Beispiele je Klasse (`grand_piano_demo`, `bars_demo`, `membrane_demo`, `keys_demo`) und das ganze Stueck
`full/nocturne_demo` schreiben ihre eigenen Skripte in denselben Ordner. Der Umfang jedes Instruments kommt aus seiner
Datei (`coreParams` `noteMin`/`noteMax`), nicht aus einer Liste hier. Die Lautstaerken sind fuer den Pegel der
Instrumente auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt = neu). Selbst gesetzt, kein fremdes Werk. Kein Zufall:
zwei Laeufe ergeben dieselben Dateien.
"""

import argparse
import json
from pathlib import Path

from make_examples import TAKT, als_text, loop
from make_membran_loops import kit_rollen, spur, zeile

HIER = Path(__file__).resolve().parent

# Instrumente mit Kern, in der Reihenfolge der Stufen
FLUEGEL = ["grand_piano", "upright_piano", "honky_tonk_piano", "electric_grand_piano"]
STAEBE = ["marimba", "vibraphone", "xylophone", "glockenspiel", "tubular_bells", "celesta", "kalimba",
          "tine_electric_piano", "reed_electric_piano"]
TROMMELN = ["bass_drum", "snare_drum", "low_tom", "conga", "bongo", "tabla", "timpani"]
TASTEN = ["clavinet_modal"]
ALLE = FLUEGEL + STAEBE + TROMMELN + TASTEN

# die Rolle im Kit `modelled_drums` je Trommel (die Lautstaerke kommt von dort)
ROLLE = {"bass_drum": "kick", "snare_drum": "snare", "low_tom": "tom 1", "conga": "perc 1", "bongo": "perc 2",
         "tabla": "perc 3"}

# Spielweisen je Instrument: (Name der Spur, coreTracks, Laenge in ms). Die Werte der Spuren nach den Kernen:
# hammerString soft/practice (getreten), struckString damperSlider 0…1, pickupSelection 0…3 (Beide, Steg, Mitte,
# Gegenphase), keyPressure Cent; bar motor U/s, bowed 0/1, malletOffset x/L, damperPressure 0…1; membrane stroke 0…5
# (Normal, Mitte, Rand, Randschlag, Reifen, Gedaempft), roll 0/1, snaresOff 0/1, pitch Cent.
#
# Nur Spielweisen, die am Instrument wirken (der Fall `*Modelled loops*` misst es). Nicht dabei, weil die Werte aus IC
# sie heute abschalten (S182 gemessen, −∞ dB Unterschied): `soft` an allen drei Hammer-Klavieren (`softFactor` 1),
# `bowed` am Glockenspiel (`bow` aus), `malletOffset` an der Kalimba (gezupft), `keyPressure` am Clavinet
# (`keyPressureLimit` 0 bis zu einer Messung, IC K1 F-4-1 a). Setzt IC einen dieser Werte, gehoert die Spur hier dazu.
NORMAL = ("Normal", {}, 1000)
HAMMER = [NORMAL, ("Practice pedal", {"practice": 1}, 1000)]
# damperPressure ist der Andruck des Daempfers: 1 = voll (wie ohne Spur), 0 = abgehoben
DAEMPFER = [("Normal, short", {}, 300), ("Half damper", {"damperPressure": 0.5}, 300),
            ("Damper off", {"damperPressure": 0}, 300)]
SCHLAEGEL = [("Mallet off centre", {"malletOffset": 0.15}, 1000), ("Mallet near the end", {"malletOffset": 0.3}, 1000)]
STOCK = [NORMAL, ("Centre", {"stroke": 1}, 1000), ("Edge", {"stroke": 2}, 1000), ("Rimshot", {"stroke": 3}, 1000),
         ("Cross-stick", {"stroke": 4}, 1000), ("Muted", {"stroke": 5}, 1000), ("Roll", {"roll": 1}, 1500),
         ("Pitch down", {"pitch": -300}, 1000), ("Pitch up", {"pitch": 300}, 1000)]
HAND = [NORMAL, ("Centre", {"stroke": 1}, 1000), ("Edge", {"stroke": 2}, 1000), ("Muted", {"stroke": 5}, 1000),
        ("Pitch down", {"pitch": -300}, 1000), ("Pitch up", {"pitch": 300}, 1000)]
SPIELWEISEN = {
    # nur, was das Instrument hat: das Upright und das Honky-tonk den Moderator; der Fluegel nur una corda (heute ohne
    # Wirkung, s. oben) und der CP-70 keins von beiden — beide ohne Muster *Techniques* (Sustain zeigt Takt 4)
    "upright_piano": HAMMER,
    "honky_tonk_piano": HAMMER,
    "marimba": [NORMAL] + SCHLAEGEL,
    "vibraphone": [NORMAL, ("Motor slow", {"motor": 2.5}, 2000), ("Motor fast", {"motor": 6}, 2000),
                   ("Bowed", {"bowed": 1}, 2000)] + DAEMPFER[1:] + SCHLAEGEL[:1],
    "xylophone": [NORMAL] + SCHLAEGEL,
    "glockenspiel": [NORMAL] + SCHLAEGEL[:1],
    "tubular_bells": DAEMPFER + SCHLAEGEL[:1],
    "celesta": DAEMPFER,
    "tine_electric_piano": DAEMPFER + SCHLAEGEL[:1],
    "reed_electric_piano": DAEMPFER + SCHLAEGEL[:1],
    "bass_drum": [s for s in STOCK if s[0] not in ("Rimshot", "Cross-stick")],
    "snare_drum": STOCK + [("Snares off", {"snaresOff": 1}, 1000)],
    "low_tom": STOCK,
    "conga": HAND,
    "bongo": HAND,
    "tabla": HAND,
    "timpani": [NORMAL, ("Centre", {"stroke": 1}, 1500), ("Edge", {"stroke": 2}, 1500), ("Muted", {"stroke": 5}, 1500),
                ("Roll", {"roll": 1}, 2000), ("Pedal down", {"pitch": -200}, 1500), ("Pedal up", {"pitch": 200}, 1500)],
    "clavinet_modal": [("Normal", {}, 400), ("Damper slider half", {"damperSlider": 0.5}, 400),
                       ("Damper slider full", {"damperSlider": 1}, 400), ("Bridge pickup", {"pickupSelection": 1}, 400),
                       ("Middle pickup", {"pickupSelection": 2}, 400), ("Out of phase", {"pickupSelection": 3}, 400)],
}
# ohne Daempfer kein Sustain: die Trommeln
MIT_PEDAL = set(FLUEGEL + STAEBE + TASTEN)

DUR = (0, 2, 4, 5, 7, 9, 11)
LAUT = 0.7          # Lautstaerke der Muster (der Pegel steht am Instrument)
LOOP_LAUT = 0.3
BPM = 90
HALB = TAKT // 2    # ein Schlag der Spielweisen je halben Takt


def datei(name):
    return json.loads((HIER / "instruments" / (name + ".lvinst")).read_text(encoding="utf-8"))


def umfang(name):
    """(noteMin, noteMax) aus der Tafel der Datei — der Umfang ist ein Wert des Kerns (KL4 a)."""
    for schicht in datei(name).get("layers", []):
        werte = schicht.get("coreParams")
        if werte:
            def wert(k):
                v = werte[k]
                return int(v["value"] if isinstance(v, dict) else v)
            return wert("noteMin"), wert("noteMax")
    raise SystemExit(f"{name}: keine Kern-Schicht")


def titel(name):
    return datei(name)["title"]


def auf_dur(h, lo, hi):
    """Die naechste Note in C-Dur innerhalb [lo, hi] (bei Gleichstand die tiefere)."""
    kandidaten = [n for n in range(lo, hi + 1) if n % 12 in DUR]
    return min(kandidaten, key=lambda n: (abs(n - h), n))


def akkord(grund, lo, hi):
    """Dreiklang auf C-Dur ueber `grund`; reicht er ueber den Umfang, eine Oktave tiefer."""
    while grund + 7 > hi and grund - 12 >= lo:
        grund -= 12
    return sorted({auf_dur(grund + d, lo, hi) for d in (0, 4, 7)})


def muster_frei(name, inst, noten, laut, pan, takte, spuren=None):
    """Wie `make_examples.muster`, aber mit dem ganzen Notenbereich A0–C8 (21–108)."""
    for schritt, laenge, hoehe, staerke in noten:
        assert 0 <= schritt and schritt + laenge <= TAKT * takte, (name, schritt, laenge)
        assert 21 <= hoehe <= 108 and 0 < staerke <= 1, (name, hoehe, staerke)
    m = {"name": name, "kind": "instrument", "bars": takte, "instrument": inst, "volume": laut, "pan": pan,
         "mute": False, "notes": [list(n) for n in sorted(noten)]}
    if spuren:
        m["lanes"] = {k: [list(p) for p in spuren.get(k, [])] for k in ("sustain", "sostenuto", "expression", "vibrato", "pitch")}
        m["lanes"]["switches"] = {}
    return m


def instrument_spur(name, inst, note, laenge_ms, kern, laut, pan=0.0, rolle=None):
    """Eine Drum-Spur, die ein Instrument mit festen Spuren des Kerns spielt (TM2 a)."""
    s = {"name": name, "sound": "", "volume": laut, "pan": pan, "mute": False, "instrument": inst, "note": note,
         "length": laenge_ms, "coreTracks": dict(sorted(kern.items()))}
    if rolle:
        s["role"] = rolle
    return s


def noten_muster(name):
    lo, hi = umfang(name)
    mitte = auf_dur((lo + hi) // 2, lo, hi)
    noten = []
    # Takt 1: Lage
    for i in range(8):
        noten.append((2 * i, 2, auf_dur(lo + round(i * (hi - lo) / 7), lo, hi), 0.7))
    # Takt 2: Staerke
    for i in range(8):
        noten.append((TAKT + 2 * i, 2, mitte, round((i + 1) / 8, 2)))
    # Takt 3: Laenge
    for schritt, laenge in ((0, 1), (2, 2), (4, 4), (8, 8)):
        noten.append((2 * TAKT + schritt, laenge, mitte, 0.7))
    # Takt 4: Arpeggio mit Sustain, dann ein Akkord mit Expression
    drei = akkord(mitte, lo, hi)
    arpeggio = (drei + [drei[0] + 12 if drei[0] + 12 <= hi else drei[-1]])[:4]
    for i, h in enumerate(arpeggio):
        noten.append((3 * TAKT + 2 * i, 1, h, 0.7))
    for h in drei:
        noten.append((3 * TAKT + HALB, HALB, h, 0.8))
    spuren = {"expression": [(0, 1.0), (3 * TAKT + HALB - 0.5, 1.0), (3 * TAKT + HALB, 0.2), (3 * TAKT + 12, 1.0),
                             (3 * TAKT + 15, 0.4)]}
    if name in MIT_PEDAL:
        spuren["sustain"] = [(3 * TAKT + 0.25, 1.0), (3 * TAKT + HALB - 0.25, 0.0)]
    return muster_frei("Notes", name, noten, LAUT, 0.0, 4, spuren)


def note_der_spielweisen(name):
    lo, hi = umfang(name)
    return lo if lo == hi else auf_dur((lo + hi) // 2, lo, hi)


def laut_der_spur(name, kit):
    rolle = ROLLE.get(name)
    return kit[rolle]["volume"] if rolle else LAUT


def staerke_muster(name, kit):
    note = note_der_spielweisen(name)
    spuren = [instrument_spur("Strength", name, note, 1000, {}, laut_der_spur(name, kit), rolle=ROLLE.get(name))]
    rows = [zeile({2 * i: round((i + 1) / 8, 2) for i in range(8)}, 1)]
    return {"name": "Strength", "kind": "drumkit", "bars": 1, "tracks": spuren, "rows": rows}


def spielweisen_muster(name, kit):
    weisen = SPIELWEISEN[name]
    note = note_der_spielweisen(name)
    takte = (len(weisen) * HALB + TAKT - 1) // TAKT
    spuren, rows = [], []
    for i, (titel_, kern, laenge) in enumerate(weisen):
        spuren.append(instrument_spur(titel_, name, note, laenge, kern, laut_der_spur(name, kit)))
        z = [0] * (TAKT * takte)
        z[i * HALB] = 0.8
        rows.append(z)
    return {"name": "Techniques", "kind": "drumkit", "bars": takte, "tracks": spuren, "rows": rows}


def nuancen(name, kit):
    lo, hi = umfang(name)
    if lo < hi:
        erstes = noten_muster(name)
    else:
        erstes = staerke_muster(name, kit)
    patterns = [erstes]
    kanaele = [(erstes["name"], [(erstes["name"], 0, 1)])]
    if name in SPIELWEISEN:
        patterns.append(spielweisen_muster(name, kit))
        kanaele.append(("Techniques", [("Techniques", erstes["bars"], 1)]))
    return loop(titel(name) + " nuances", "", BPM, LOOP_LAUT, patterns, kanaele)


# --- das Ensemble -------------------------------------------------------------

# Am – F – C – G
AKKORDE = [(57, 60, 64), (53, 57, 60), (48, 52, 55), (55, 59, 62)]
GRUND_PAUKE = [52, 53, 48, 55]  # E3, F3, C3, G3 — die Pauke reicht C3–G3
# (Schritt, Laenge, Hoehe, Anschlag) je Takt
MELODIE = [
    [(0, 4, 76, 0.8), (4, 2, 72, 0.6), (6, 2, 74, 0.65), (8, 6, 76, 0.8), (14, 2, 79, 0.6)],
    [(0, 4, 77, 0.8), (4, 4, 76, 0.65), (8, 4, 72, 0.7), (12, 4, 69, 0.6)],
    [(0, 4, 72, 0.75), (4, 2, 74, 0.6), (6, 2, 76, 0.65), (8, 8, 79, 0.85)],
    [(0, 4, 79, 0.8), (4, 4, 77, 0.65), (8, 4, 74, 0.7), (12, 4, 71, 0.6)],
]


def je_akkord(tu):
    noten = []
    for takt, a in enumerate(AKKORDE):
        for s, l, h, w in tu(takt, a):
            noten.append((takt * TAKT + s, l, h, w))
    return noten


def melodie(versatz=0, staerke=1.0):
    return [(t * TAKT + s, l, h + versatz, round(w * staerke, 2)) for t, takt in enumerate(MELODIE) for s, l, h, w in takt]


def ensemble_kit(kit, voll):
    spuren = [
        (spur("Bass drum", "kick", kit), {0: 1.0, 7: 0.5, 8: 0.85, (3, 14): 0.7}),
        (spur("Snare", "snare", kit), {4: 1.0, 12: 1.0, (3, 15): 0.6}),
        (spur("Cross-stick", "rim", kit), {2: 0.5, 10: 0.5}),
        (spur("Low tom", "tom 1", kit), {(3, 10): 0.8, (3, 12): 0.9}),
    ]
    if voll:
        spuren += [
            (spur("Conga", "perc 1", kit), {3: 0.7, 6: 0.85, 11: 0.7, 14: 0.85}),
            (spur("Bongo", "perc 2", kit), {1: 0.5, 5: 0.45, 9: 0.5, 13: 0.45}),
            (spur("Tabla", "perc 3", kit), {s: (0.7 if s % 4 == 0 else 0.4) for s in range(0, 16, 2)}),
        ]
    takte = 4
    return {"name": "Drums full" if voll else "Drums", "kind": "drumkit", "bars": takte,
            "tracks": [s for s, _ in spuren], "rows": [zeile(z, takte) for _, z in spuren], "kit": "modelled_drums"}


def ensemble(kit):
    p = []

    def m(name, inst, noten, laut, pan, spuren=None):
        p.append(muster_frei(name, inst, noten, laut, pan, 4, spuren))

    pedal_je_takt = {"sustain": [pt for t in range(4) for pt in ((t * TAKT + 0.25, 1.0), (t * TAKT + 15.5, 0.0))]}
    m("Timpani", "timpani", [(t * TAKT, 4, GRUND_PAUKE[t], 0.85) for t in range(4)] + [(3 * TAKT + 12, 2, 55, 0.6), (3 * TAKT + 14, 2, 55, 0.75)],
      0.55, -0.35)
    m("Marimba", "marimba", je_akkord(lambda t, a: [(s, 2, (a[0] - 12, a[0], a[2] - 12, a[0])[i % 4], 0.8 if i % 4 == 0 else 0.55)
                                                    for i, s in enumerate(range(0, 16, 2))]), 0.5, -0.15)
    m("Upright", "upright_piano", je_akkord(lambda t, a: [(s, 2, h, 0.55) for s in (2, 6, 10, 14) for h in a]), 0.4, -0.25)
    m("Vibraphone", "vibraphone", je_akkord(lambda t, a: [(0, 16, h + 12, 0.5) for h in a]), 0.4, 0.3, pedal_je_takt)
    m("Xylophone", "xylophone", melodie(), 0.45, 0.2)
    m("Kalimba", "kalimba", je_akkord(lambda t, a: [(s, 1, (a[0] + 12, a[1] + 12, a[2] + 12, a[1] + 24)[i % 4], 0.5)
                                                    for i, s in enumerate(range(0, 16, 2))]), 0.4, 0.4)
    m("Reed EP", "reed_electric_piano", je_akkord(lambda t, a: [(0, 6, a[0] - 12, 0.65), (8, 4, a[0] - 12, 0.55), (12, 4, a[2] - 12, 0.5)]), 0.45, -0.1)
    m("Tubular bells", "tubular_bells", [(0, 16, 57, 0.8), (2 * TAKT, 16, 60, 0.7)], 0.35, 0.1)
    m("Clavinet", "clavinet_modal", je_akkord(lambda t, a: [(s, 1, (a[0], a[0] + 12, a[1], a[0] + 12)[i % 4], 0.75 if i % 4 == 0 else 0.5)
                                                            for i, s in enumerate((0, 2, 3, 6, 8, 10, 11, 14))]), 0.4, -0.4)
    # zweite Haelfte
    m("Grand piano", "grand_piano", melodie(0, 0.9) + je_akkord(lambda t, a: [(0, 16, a[0] - 24, 0.6)]), 0.45, 0.0, pedal_je_takt)
    m("Glockenspiel", "glockenspiel", melodie(12, 0.8), 0.3, 0.35)
    m("Celesta", "celesta", je_akkord(lambda t, a: [(0, 8, a[2] + 12, 0.55), (8, 8, a[1] + 12, 0.5)]), 0.4, -0.3)
    m("Tine EP", "tine_electric_piano", je_akkord(lambda t, a: [(s, 2, h, 0.55) for s in (0, 6, 12) for h in a]), 0.45, 0.25,
      pedal_je_takt)
    m("Electric grand", "electric_grand_piano", je_akkord(lambda t, a: [(4, 1, h, 0.6) for h in a] + [(12, 1, h, 0.6) for h in a]),
      0.4, -0.2)
    m("Honky-tonk", "honky_tonk_piano", [(3 * TAKT + s, 1, h, 0.6) for s, h in ((8, 79), (9, 77), (10, 76), (11, 74), (12, 72),
                                                                             (13, 71), (14, 67), (15, 62))], 0.4, 0.3)
    p.append(ensemble_kit(kit, False))
    p.append(ensemble_kit(kit, True))
    erste = ["Timpani", "Marimba", "Upright", "Vibraphone", "Xylophone", "Kalimba", "Reed EP", "Tubular bells", "Clavinet"]
    zweite = ["Grand piano", "Glockenspiel", "Celesta", "Tine EP", "Electric grand", "Honky-tonk"]
    durchgehend = {"Timpani", "Marimba", "Vibraphone", "Clavinet"}
    kanaele = [("Drums", [("Drums", 0, 1), ("Drums full", 4, 1)])]
    for name in erste:
        kanaele.append((name, [(name, 0, 2 if name in durchgehend else 1)]))
    for name in zweite:
        kanaele.append((name, [(name, 4, 1)]))
    return loop("Modelled ensemble", "Fusion", 100, 0.25, p, kanaele)


def main():
    teile = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    teile.add_argument("--wurzel", default=str(HIER), help="Ordner asset/loop (oder eine Kopie)")
    wurzel = Path(teile.parse_args().wurzel)
    ziel = wurzel / "modelled_instruments"
    ziel.mkdir(parents=True, exist_ok=True)
    kit = kit_rollen()
    for name in ALLE:
        (ziel / (name + ".lvloop")).write_text(als_text(nuancen(name, kit)), encoding="utf-8", newline="\n")
    (ziel / "modelled_ensemble.lvloop").write_text(als_text(ensemble(kit)), encoding="utf-8", newline="\n")
    print(len(ALLE), "Loops der Nuancen und das Ensemble")


if __name__ == "__main__":
    main()
