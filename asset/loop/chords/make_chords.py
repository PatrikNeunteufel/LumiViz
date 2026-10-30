"""Die mitgelieferten Akkorde schreiben (S137, Stufe 26b; Entscheide Patrik S-T3 a, S-T4 a, S-T6 a,
docs/audio/Saiten_Konzept.md §1).

    python -I asset/loop/chords/make_chords.py <Ordner mit guitar.json und ukulele.json>

Der Ordner ist das npm-Paket `@tombatossals/chords-db` 0.5.1 (`lib/guitar.json`, `lib/ukulele.json`,
`LICENSE`), geladen S137 nach `...\\cmake\\ChordsDb\\0.5.1`, seit S155 auf der Wechselplatte unter
`E:\\samples\\ChordsDb\\0.5.1` — ausserhalb des Repos, wie die Originale der
Samples. Schreibt neben dieses Skript je Sammlung eine Datei `<sammlung>.lvchords` (Format
`lumiviz-chord-collection` Fassung 1, `loop/Akkord.hpp`; seit S167, AK1 a): im Kopf die Stimmung
`open` der Griffbilder (bei `voicings` fehlt sie), unter `chords` je Akkord eine Zeile — `name` und
das Objekt einer `.lvchord` (Fassung 1). Eine Datei statt tausender: Windows prueft jede Datei beim
ersten Oeffnen, 6373 einzelne brauchten so 25,6 s, sechs Sammlungen 34 ms (gemessen S167).

- `guitar`, `ukulele`: jede Lage aus chords-db (MIT, D. Rubert; `CREDITS.md`). Die Buende stehen
  dort ab `baseFret` (1 = der Bund `baseFret`); umgerechnet wird auf Buende ab dem Sattel und gegen
  `midi` der Quelle geprueft — eine Abweichung bricht ab.
- `drop_d`, `bass`, `violin`: gerechnet nach der Regel aus S-T4 a (`gerechnet`), `origin`
  `computed`: die Akkordtoene ueber die Saiten, hoechstens vier Buende Spannweite, leere Saiten
  bevorzugt, der Grundton im Bass, hoechstens vier Finger (ein Barre zaehlt als einer), die
  klingenden Saiten ohne Luecke.
- `voicings`: je Grundton und Art Grundstellung, Umkehrungen und eine weite Lage (Lehrbuchwissen,
  `ABSTAENDE`).

Die Arten (`ABSTAENDE`) liest es aus `types.json` daneben — dieselbe Datei liest das Programm
(`akkordArten()`, S-T13 a); sie ist keine Ausgabe dieses Skripts.

Sammlung und `name` ergeben die Nennung im Programm (`guitar/c_major_1`). Kein Zufall: zwei Laeufe
ergeben dieselben Dateien, die Akkorde nach Namen geordnet. Vorher werden die Sammlungen und die
Ordner mit einzelnen `.lvchord` aus der Zeit vor S167 geloescht (alles hier ist erzeugt). Zum
Schluss: die Zahl je Sammlung — `test_LoopAkkord.cpp` traegt sie.
"""
import itertools
import json
import pathlib
import sys

ORDNER = pathlib.Path(__file__).parent

# die Namen der Grundtoene wie in `grundtonNamen()` (Akkord.cpp)
GRUNDTOENE = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]
STUFE = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}

# die Abstaende der Arten in Halbtoenen ueber dem Grundton — aus `types.json` daneben, die auch
# das Programm liest (S-T13 a; Lehrbuchwissen, die Namen wie chords-db)
ABSTAENDE = {t["name"]: t["steps"]
             for t in json.loads((ORDNER / "types.json").read_text(encoding="utf-8"))["types"]}
# gerechnete Griffbilder: diese Arten (S-T4 a; von Claude gewaehlt — die gaengigen bis zu vier Toenen)
GERECHNET_ARTEN = ["major", "minor", "7", "maj7", "m7", "sus2", "sus4", "dim", "aug", "m7b5", "6", "m6", "add9"]
# Stimmungen ohne Quelle (die mitgelieferten Instrumente, `make_saiten_instrumente.py`)
GERECHNET_STIMMUNGEN = {
    "drop_d": [38, 45, 50, 55, 59, 64],
    "bass": [28, 33, 38, 43],
    "violin": [55, 62, 69, 76],
}
LAGEN_MAX = 3      # je Akkord hoechstens so viele gerechnete Lagen
SPANNE = 3         # hoechstens vier Buende: der hoechste gegriffene hoechstens drei ueber dem tiefsten
LAGE_BIS = 12      # gesucht wird bis zur Lage am 12. Bund
# Voicings: diese Arten (die Dreiklaenge und Vierklaenge; von Claude gewaehlt)
VOICING_ARTEN = ["major", "minor", "dim", "aug", "sus2", "sus4", "6", "m6", "7", "maj7", "m7", "m7b5",
                 "dim7", "7sus4", "mmaj7", "add9", "9", "maj9", "m9"]


def grundton(name):
    ton = STUFE[name[0].upper()]
    if name[1:] == "#":
        ton += 1
    elif name[1:] == "b":
        ton -= 1
    return ton % 12


def schreibweise(ton):
    """`C#` -> `cs`, `Eb` -> `eb` (fuer Dateinamen)."""
    return GRUNDTOENE[ton].lower().replace("#", "s")


def art_name(art):
    """`7#9` -> `7s9`, `m/C#` -> `m_over_cs` — wie `eigenerName` es annimmt."""
    teile = art.split("/")
    name = teile[0].replace("#", "s")
    if len(teile) > 1:
        name = (name + "_" if name else "") + "over_" + schreibweise(grundton(teile[1]))
    return name.lower()


def symbol(schluessel, art):
    """Der Titel: `C`, `Cm`, `C7`, `C/E` — wie Akkordsymbole geschrieben werden."""
    if art == "major":
        return schluessel
    if art == "minor":
        return schluessel + "m"
    return schluessel + art


def akkord_json(titel, ton, art, beschreibung, herkunft=None, saiten=None, voicing=None):
    o = {"format": "lumiviz-chord", "version": 1, "title": titel, "root": GRUNDTOENE[ton], "type": art}
    if herkunft:
        o["origin"] = herkunft
    o["description"] = beschreibung
    if saiten is not None:
        o["strings"] = saiten
    else:
        o["voicing"] = voicing
    return o


# je Sammlung die Akkorde (Name, Objekt einer `.lvchord`); `schreibe_sammlungen` legt sie ab
SAMMLUNGEN = {}

BESCHREIBUNG = {
    "guitar": "Guitar, standard tuning: every position from chords-db 0.5.1 (MIT, D. Rubert).",
    "ukulele": "Ukulele, standard tuning: every position from chords-db 0.5.1 (MIT, D. Rubert).",
    "drop_d": "Guitar, drop D: computed by rule (no source for this tuning).",
    "bass": "Bass, four strings: computed by rule (no source for this tuning).",
    "violin": "Violin: computed by rule (no source for this tuning).",
    "voicings": "Voicings without strings: root position, inversions and an open position per chord type.",
}


def sammle(sammlung, name, daten):
    SAMMLUNGEN.setdefault(sammlung, []).append((name, daten))


def schreibe_sammlungen():
    """Je Sammlung eine `.lvchords`: der Kopf mehrzeilig, je Akkord eine Zeile (lesbare Diffs)."""
    for sammlung, akkorde in SAMMLUNGEN.items():
        akkorde = sorted(akkorde)
        if len({n for n, _ in akkorde}) != len(akkorde):
            sys.exit(f"{sammlung}: ein Name doppelt")
        stimmungen = {tuple(d["strings"]["open"]) if "strings" in d else None for _, d in akkorde}
        if len(stimmungen) != 1:
            sys.exit(f"{sammlung}: mehr als eine Stimmung")
        stimmung = stimmungen.pop()
        kopf = [f'    "format": "lumiviz-chord-collection"', f'    "version": 1',
                f'    "description": {json.dumps(BESCHREIBUNG[sammlung], ensure_ascii=False)}']
        if stimmung is not None:
            kopf.append(f'    "open": {json.dumps(list(stimmung))}')
        zeilen = ["        " + json.dumps({"name": n, **d}, ensure_ascii=False) for n, d in akkorde]
        text = "{\n" + ",\n".join(kopf) + ',\n    "chords": [\n' + ",\n".join(zeilen) + "\n    ]\n}\n"
        (ORDNER / f"{sammlung}.lvchords").write_text(text, encoding="utf-8", newline="\n")


def lage_text(nummer, buende):
    """`Position 1, open` (eine leere Saite klingt) oder `Position 2, from fret 3` (der tiefste gegriffene)."""
    if 0 in buende:
        return f"Position {nummer}, open"
    return f"Position {nummer}, from fret {min(b for b in buende if b > 0)}"


def name_gueltig(name):
    erlaubt = set("abcdefghijklmnopqrstuvwxyz0123456789_")
    return 0 < len(name) <= 64 and set(name) <= erlaubt and not name.startswith("_") and not name.endswith("_") \
        and "__" not in name


# --- aus chords-db -------------------------------------------------------------------------------

def aus_quelle(quelle, instrument):
    daten = json.loads((quelle / f"{instrument}.json").read_text(encoding="utf-8"))
    namen = {"E2": 40, "A2": 45, "D3": 50, "G3": 55, "B3": 59, "E4": 64, "G4": 67, "C4": 60, "A4": 69}
    leer = [namen[n] for n in daten["tunings"]["standard"]]
    zahl = 0
    for liste in daten["chords"].values():
        for akkord in liste:
            schluessel, art = akkord["key"], akkord["suffix"]
            ton = grundton(schluessel)
            for nummer, lage in enumerate(akkord["positions"], start=1):
                basis = lage["baseFret"]
                buende = [f if f <= 0 else basis + f - 1 for f in lage["frets"]]
                # gegen die Quelle: die klingenden Toene in der Reihenfolge der Saiten
                klingt = [leer[s] + b for s, b in enumerate(buende) if b >= 0]
                if klingt != lage["midi"]:
                    sys.exit(f"{instrument} {schluessel}{art} Lage {nummer}: {klingt} gegen midi {lage['midi']}")
                barre = sorted({basis + b - 1 for b in lage.get("barres", [])})
                name = f"{schreibweise(ton)}_{art_name(art)}_{nummer}"
                if not name_gueltig(name):
                    sys.exit(f"kein gueltiger Name: {name}")
                beschreibung = lage_text(nummer, buende)
                saiten = {"open": leer, "frets": buende, "fingers": lage["fingers"], "barres": barre}
                sammle(instrument, name, akkord_json(symbol(schluessel, art), ton, art, beschreibung, saiten=saiten))
                zahl += 1
    return zahl


# --- gerechnet (S-T4 a) ----------------------------------------------------------------------------

def finger_fuer(buende):
    """Die Finger: ein Barre (der tiefste gegriffene Bund auf mindestens zwei Saiten) mit dem
    Zeigefinger, die uebrigen gegriffenen nach Bund und Saite mit 2, 3, 4. Mehr als vier: None."""
    gegriffen = [(b, s) for s, b in enumerate(buende) if b > 0]
    finger = [0] * len(buende)
    if not gegriffen:
        return finger, []
    tief = min(b for b, _ in gegriffen)
    am_tiefsten = [s for b, s in gegriffen if b == tief]
    barre = [tief] if len(am_tiefsten) >= 2 else []
    naechster = 1
    if barre:
        for s in am_tiefsten:
            finger[s] = 1
        naechster = 2
        rest = sorted((b, s) for b, s in gegriffen if b != tief)
    else:
        rest = sorted(gegriffen)
    for _, s in rest:
        if naechster > 4:
            return None, []
        finger[s] = naechster
        naechster += 1
    return finger, barre


def gerechnet(leer, ton, art):
    """Die Lagen eines Akkords auf diesen Saiten nach der Regel; die besten je Lage, verschieden."""
    stufen = {(ton + a) % 12 for a in ABSTAENDE[art]}
    pflicht = set(stufen)
    if len(ABSTAENDE[art]) >= 4:
        pflicht.discard((ton + 7) % 12)  # bei vier Toenen darf die Quinte fehlen
    mindestens = 3 if len(leer) >= 4 else len(leer)
    beste = {}
    for lage in range(0, LAGE_BIS + 1):
        auswahl = []
        for s, l in enumerate(leer):
            moeglich = [-1]
            if l % 12 in stufen:
                moeglich.append(0)
            for b in range(max(1, lage), lage + SPANNE + 1):
                if (l + b) % 12 in stufen:
                    moeglich.append(b)
            auswahl.append(moeglich)
        for buende in itertools.product(*auswahl):
            spielt = [s for s, b in enumerate(buende) if b >= 0]
            if len(spielt) < mindestens or spielt != list(range(spielt[0], spielt[-1] + 1)):
                continue
            klingt = [leer[s] + buende[s] for s in spielt]
            if klingt[0] != min(klingt) or klingt[0] % 12 != ton:
                continue  # der Grundton im Bass
            if not pflicht <= {k % 12 for k in klingt}:
                continue
            gegriffen = [b for b in buende if b > 0]
            if gegriffen and max(gegriffen) - min(gegriffen) > SPANNE:
                continue
            finger, barre = finger_fuer(list(buende))
            if finger is None:
                continue
            tief = min(gegriffen, default=0)
            wertung = (-len(spielt), -sum(1 for b in buende if b == 0), tief, sum(gegriffen))
            schluessel = min(gegriffen, default=0) // 1
            vorher = beste.get(schluessel)
            if vorher is None or wertung < vorher[0]:
                beste[schluessel] = (wertung, list(buende), finger, barre)
    # verschiedene Lagen, die tiefsten zuerst
    lagen = [beste[k] for k in sorted(beste)]
    gesehen, ergebnis = set(), []
    for _, buende, finger, barre in lagen:
        if tuple(buende) in gesehen:
            continue
        gesehen.add(tuple(buende))
        ergebnis.append((buende, finger, barre))
        if len(ergebnis) == LAGEN_MAX:
            break
    return ergebnis


def gerechnete():
    zahlen = {}
    for ordner, leer in GERECHNET_STIMMUNGEN.items():
        zahl = 0
        for ton in range(12):
            for art in GERECHNET_ARTEN:
                for nummer, (buende, finger, barre) in enumerate(gerechnet(leer, ton, art), start=1):
                    name = f"{schreibweise(ton)}_{art_name(art)}_{nummer}"
                    beschreibung = lage_text(nummer, buende) + " — computed by rule (no source for this tuning)"
                    saiten = {"open": leer, "frets": buende, "fingers": finger, "barres": barre}
                    sammle(ordner, name,
                           akkord_json(symbol(GRUNDTOENE[ton], art), ton, art, beschreibung, "computed", saiten))
                    zahl += 1
        zahlen[ordner] = zahl
    return zahlen


# --- Voicings (S-T6 a) -----------------------------------------------------------------------------

def voicings_einer_art(art):
    """Grundstellung, die Umkehrungen, eine weite Lage: (Name, Beschreibung, Abstaende ab dem tiefsten)."""
    a = ABSTAENDE[art]
    ergebnis = [("root", "Root position", a)]
    toene = sorted(x % 12 for x in a) if max(a) < 12 else None
    if toene is not None:
        reihe = list(a)
        for n in range(1, len(a)):
            reihe = reihe[1:] + [reihe[0] + 12]
            ergebnis.append((f"inv{n}", ["", "First", "Second", "Third"][n] + " inversion",
                             [x - reihe[0] for x in reihe]))
    if len(a) == 3:
        weit = [a[0], a[2], a[1] + 12]  # Grundton, Quinte, Terz eine Oktave hoeher
        ergebnis.append(("open", "Open position", weit))
    elif len(a) == 4 and max(a) < 12:
        drop2 = sorted([a[0], a[1], a[3], a[2] - 12])  # Drop 2: die zweithoechste eine Oktave tiefer
        ergebnis.append(("drop2", "Drop 2", [x - drop2[0] for x in drop2]))
    return ergebnis


def voicings():
    zahl = 0
    for ton in range(12):
        for art in VOICING_ARTEN:
            for kennung, beschreibung, abstaende in voicings_einer_art(art):
                name = f"{schreibweise(ton)}_{art_name(art)}_{kennung}"
                sammle("voicings", name,
                       akkord_json(symbol(GRUNDTOENE[ton], art), ton, art, beschreibung, voicing=abstaende))
                zahl += 1
    return zahl


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    quelle = pathlib.Path(sys.argv[1])
    for sammlung in ["guitar", "ukulele", "voicings", *GERECHNET_STIMMUNGEN]:
        (ORDNER / f"{sammlung}.lvchords").unlink(missing_ok=True)
        alt = ORDNER / sammlung  # die einzelnen `.lvchord` vor S167
        if alt.is_dir():
            for datei in alt.glob("*.lvchord"):
                datei.unlink()
            alt.rmdir()
    zahlen = {"guitar": aus_quelle(quelle, "guitar"), "ukulele": aus_quelle(quelle, "ukulele")}
    zahlen.update(gerechnete())
    zahlen["voicings"] = voicings()
    schreibe_sammlungen()
    for ordner, zahl in zahlen.items():
        print(f"{ordner}: {zahl}")
    print("zusammen:", sum(zahlen.values()))


if __name__ == "__main__":
    main()
