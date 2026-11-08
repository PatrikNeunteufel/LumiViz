"""Das mitgelieferte Beispiel des Kerns `membrane` (S179, Stufe 34h; docs/audio/Instrumente_IC_Konzept.md 34h-6, IK2-8 a:
„je Klasse ein Beispiel <name>_demo“; TM2 a, TM3 a).

    python asset/loop/make_membran_loops.py
    python asset/loop/instrument_pegel.py --loops
    python asset/loop/instrument_katalog.py

Schreibt `asset/loop/membrane_demo.lvloop`: acht Takte, 100 bpm. Ein Drum-Muster mit dem Kit `modelled_drums` (TM3 a):
Bassdrum, Snare, Cross-Stick (*rim*), Randschlag (*snare 2*), Low tom, Conga, Bongo — dazu ein Wirbel der Snare als
eigene Spur (`coreTracks` roll 1 ueber 750 ms; die Spielweise einer Spur, TM2 a). Die Pauke spielt C3 und G3 als
Instrument (ihr Umfang C3–G3), die Tabla als Instrument mit wechselnder Staerke (TM1 a: die Stufe ist die Lautstaerke in
dB). Die zweite Haelfte nimmt die Haende dazu. Selbst gesetzt, kein fremdes Werk.

Die Lautstaerken sind fuer den Pegel der Instrumente auf dem Bezug gesetzt (`instrument_pegel.PEGEL`: alt = neu) und fuer
die Volumes des Kits (`kits/make_kits.py`). Kein mitgelieferter Loop wird auf die neuen Instrumente umgestellt (IK2-8 a).
Kein Zufall: zwei Laeufe ergeben dieselbe Datei.
"""

from pathlib import Path

from make_examples import TAKT, als_text, loop, muster

HIER = Path(__file__).resolve().parent
KIT = HIER / "kits" / "modelled_drums.lvkit"


def kit_rollen():
    """Rolle -> Eintrag des Kits (Instrument, Note, Laenge, Spielweise, Volume, Pan), aus der geschriebenen Datei."""
    import json
    daten = json.loads(KIT.read_text(encoding="utf-8"))
    return {r["role"]: r for r in daten["roles"]}


def spur(name, rolle, kit, **anders):
    r = dict(kit[rolle]) if rolle in kit else {}
    r.update(anders)
    eintrag = {"name": name, "sound": r.get("sound", ""), "volume": r.get("volume", 1.0), "pan": r.get("pan", 0.0), "mute": False,
               "instrument": r["instrument"], "note": r.get("note", 36), "length": r.get("length", 250),
               "coreTracks": r.get("coreTracks", {}), "role": rolle}
    return eintrag


def zeile(schlaege, takte):
    """schlaege: {Schritt im Takt: Staerke} oder {(Takt, Schritt): Staerke}."""
    z = [0] * (TAKT * takte)
    for takt in range(takte):
        for s, w in schlaege.items():
            if isinstance(s, tuple):
                if s[0] == takt:
                    z[takt * TAKT + s[1]] = w
            else:
                z[takt * TAKT + s] = w
    return z


def trommeln(kit, mit_haenden):
    takte = 4
    spuren = [
        (spur("Bass drum", "kick", kit), {0: 1.0, 7: 0.6, 8: 0.9, (3, 14): 0.7}),
        (spur("Snare", "snare", kit), {4: 1.0, 12: 1.0}),
        (spur("Cross-stick", "rim", kit), {2: 0.6, 10: 0.6, 14: 0.5}),
        (spur("Rimshot", "snare 2", kit), {(1, 12): 1.0, (3, 12): 1.0}),
        (spur("Snare roll", "snare 3", kit, instrument="snare_drum", note=38, length=750, coreTracks={"roll": 1},
              sound="", volume=kit["snare"]["volume"] * 0.7, pan=0.0), {(3, 4): 0.8}),
        (spur("Low tom", "tom 1", kit), {(1, 14): 0.8, (3, 15): 0.9}),
    ]
    if mit_haenden:
        spuren += [
            (spur("Conga", "perc 1", kit), {3: 0.7, 6: 0.9, 11: 0.7, 14: 0.9}),
            (spur("Bongo", "perc 2", kit), {1: 0.6, 5: 0.5, 9: 0.6, 13: 0.5, 15: 0.7}),
        ]
    tracks = [s for s, _ in spuren]
    rows = [zeile(z, takte) for _, z in spuren]
    name = "Hands" if mit_haenden else "Kit"
    return {"name": name, "kind": "drumkit", "bars": takte, "tracks": tracks, "rows": rows, "kit": "modelled_drums"}


def pauke():
    # C3 und G3 auf der Eins und der Drei, am Ende ein kleiner Wirbel aus Achteln
    noten = []
    for takt in range(4):
        noten += [(takt * TAKT, 8, 48, 0.9), (takt * TAKT + 8, 8, 55, 0.7)]
    noten[-1] = (3 * TAKT + 8, 2, 55, 0.6)
    noten += [(3 * TAKT + 10, 2, 55, 0.7), (3 * TAKT + 12, 2, 55, 0.8), (3 * TAKT + 14, 2, 48, 1.0)]
    return muster("Timpani", "timpani", noten, laut=0.7, pan=-0.2)


def tabla():
    # ein Theka-Gefuehl in Sechzehnteln, die Staerke wechselt (TM1 a)
    staerken = (1.0, 0.35, 0.55, 0.35, 0.8, 0.35, 0.55, 0.45)
    noten = [(takt * TAKT + 2 * i, 1, 62, staerken[i]) for takt in range(4) for i in range(8)]
    return muster("Tabla", "tabla", noten, laut=0.6, pan=0.35)


def membrane_demo():
    kit = kit_rollen()
    erste, zweite = trommeln(kit, False), trommeln(kit, True)
    patterns = [erste, zweite, pauke(), tabla()]
    kanaele = [("Kit", [("Kit", 0, 1), ("Hands", 4, 1)]), ("Timpani", [("Timpani", 0, 2)]), ("Tabla", [("Tabla", 4, 1)])]
    return loop("Membrane demo", "World", 100, 0.3, patterns, kanaele)


def main():
    (HIER / "membrane_demo.lvloop").write_text(als_text(membrane_demo()), encoding="utf-8", newline="\n")
    print("1 Loop der Trommeln")


if __name__ == "__main__":
    main()
