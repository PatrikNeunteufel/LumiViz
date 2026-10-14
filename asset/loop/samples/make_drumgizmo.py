"""Akustische Drums aus DrumGizmo holen (S124, Stufe 21d; Entscheide Patrik K6b a, K6c a, D1 a).

    python asset/loop/samples/make_drumgizmo.py verzeichnis <ordner>

Die Kits DRSKit 2.1 und CrocellKit 1.1 von drumgizmo.org stehen unter CC BY 4.0. Sie sind
mehrere GB gross; gebraucht werden je Rolle nur wenige Anschlaege. Deshalb laedt dieses Skript
nie ein ganzes Paket (Entscheid D1 a), sondern liest ueber Teil-Abrufe (HTTP Range) erst das
Inhaltsverzeichnis jeder ZIP und spaeter nur die gewaehlten Dateien. Die Originale liegen NICHT
im Repo, sondern in `<ordner>` (in S124: `...\\Visuals_Project\\cmake\\DrumGizmo`).

Schritte:
  verzeichnis  liest das Inhaltsverzeichnis jeder ZIP nach `<ordner>/<kit>_inhalt.json`
               (Name, gepackte und ungepackte Groesse, Lage in der ZIP) und holt die
               XML-Dateien der Kits (wenige KB): sie nennen je Anschlag seine Staerke
               (`power`) und welcher Kanal der WAV welches Mikrofon ist
  holen        holt die gewaehlten Anschlaege (`WAHL`, Entscheid D1b: 37 Dateien, 296,5 MB; dazu D4 a:
               der Schaft-Crash statt der Spitze, 13,98 MB)
               nach `<ordner>/<kit>/<instrument>/samples/`; vorhandene mit richtiger Groesse
               bleiben, jede neue wird gegen die Pruefsumme der ZIP geprueft
  schreiben    mischt jeden Anschlag zu einem Stereo-Sample `<name>.wav` hier im Ordner
               (Entscheid D2 a: Stereo, 16 Bit, 44100 Hz; D6 a, ersetzt D3 a und D5 a: die ganze
               Laenge der Aufnahme, nur die Stille bis 2 ms vor dem Anschlag weg und am Ende 10 ms
               ausgeblendet; die Spitze auf 0,89 wie die uebrigen Samples) und schreibt den
               Abschnitt DrumGizmo in `CREDITS.md` (alles ab der Marke `MARKE`)

Die Mischung (von Claude gesetzt, S124; gehoert wird sie an den Hoerproben): die Nahmikrofone
der angeschlagenen Trommel 0 dB in der Mitte, die Overheads -6 dB links und rechts (ein mittlerer
in der Mitte), der Raum -12 dB links und rechts; die Nahmikrofone fremder Trommeln fallen weg,
sie tragen nur Uebersprechen. Ein zweites Nahmikrofon (Snare unten, Kick aussen) wird gedreht,
wenn es in den ersten 30 ms gegen das erste laeuft.

Gewaehlt wird je Eintrag in `WAHL` der Anschlag, dessen Staerke (`power` in der XML des
Instruments) der Lage in der Spanne am naechsten liegt: leise 25 %, mittel 60 %, voll 90 %
(von Claude gesetzt, S124).

Gebraucht: Python 3; fuer `schreiben` dazu numpy und scipy.
"""

import json
import xml.etree.ElementTree as ET
import struct
import sys
import urllib.request
import zlib
from pathlib import Path

HIER = Path(__file__).resolve().parent
RATE = 44100
SPITZE = 0.89
ANFANG_DB = -50.0         # davor ist Stille: gegen die Spitze gemessen
AUSBLENDEN_S = 0.010      # D6 a: nur das Ende der Datei
# Erst D3 a (bis -50 dB unter der Spitze), dann D5 a (bis -60 dB am Effektivwert): beides schnitt in den
# Ausklang und sparte zuletzt nur 3 MB. Die Aufnahmen von DrumGizmo klingen selbst in Stille aus und sind
# hoechstens 23,8 s lang (das Programm nimmt 30 s) — sie bleiben ganz (D6 a, S124).
MARKE = "<!-- DrumGizmo: ab hier schreibt make_drumgizmo.py -->"

# Je Kit: Mikrofon -> (Art, Seite); Art nah = Nahmikrofon, oh = Overhead, raum = Raum
MIKROFONE = {
    "DRSKit": {"AmbL": ("raum", "L"), "AmbR": ("raum", "R"), "OHL": ("oh", "L"), "OHR": ("oh", "R")},
    "CrocellKit": {"AmbLeft": ("raum", "L"), "AmbRight": ("raum", "R"), "OHLeft": ("oh", "L"),
                   "OHRight": ("oh", "R"), "OHCenter": ("oh", "M")},
}
PEGEL_DB = {"nah": 0.0, "oh": -6.0, "raum": -12.0}

# Je Instrument die eigenen Nahmikrofone (das erste ist das Haupt-Mikrofon); keins = nur Overheads und Raum
EIGENE = {
    "Kdrum_without_contact": ["Kdrum_front", "Kdrum_back"],
    "Snare": ["Snare_top", "Snare_bottom", "SnareTop", "SnareBottom"],
    "Snare_rim": ["Snare_top", "Snare_bottom"],
    "Hihat_closed": ["Hihat"], "Hihat_foot": ["Hihat"], "Hihat_open": ["Hihat"],
    "HihatClosed": ["Hihat"], "HihatOpen": ["Hihat"],
    "Tom1": ["Tom1"], "Tom2": ["Tom2"], "Tom3": ["Tom3"], "FTom1": ["FTom1"], "FTom2": ["FTom2"],
    "Ride_tip": ["Ride"], "RideR": ["Ride"],
    "KDrumL": ["KDrumInside", "KDrumOutside"], "KDrumR": ["KDrumInside", "KDrumOutside"],
}

# Was in CREDITS.md steht: Kit -> (Titel, Seite, Wer, Satz zur Herkunft)
HERKUNFT = {
    "DRSKit": ("DRSKit 2.1", "https://drumgizmo.org/wiki/doku.php?id=kits:drskit", "the DrumGizmo team and Jes Eiler of DRSDrums",
               "A collaboration between the DrumGizmo team and Jes Eiler of DRSDrums, who lent the kit."),
    "CrocellKit": ("CrocellKit 1.1", "https://drumgizmo.org/wiki/doku.php?id=kits:crocellkit", "the DrumGizmo team",
                   "The kit of the Danish metal band Crocell, sampled by the DrumGizmo team at JBOSound (Jacob Olsen) "
                   "while the band recorded its album Relics."),
}
WAS = {"leise": "soft", "mittel": "medium", "voll": "hard"}

KITS = {
    "DRSKit": "https://drumgizmo.org/kits/DRSKit/DRSKit2_1.zip",
    "CrocellKit": "https://drumgizmo.org/kits/CrocellKit/CrocellKit1_1.zip",
}


def hole_bereich(url, von, bis):
    """Die Bytes von `von` bis `bis` (einschliesslich) — ein Teil-Abruf."""
    anfrage = urllib.request.Request(url, headers={"Range": f"bytes={von}-{bis}", "User-Agent": "LumiViz-make_drumgizmo"})
    with urllib.request.urlopen(anfrage, timeout=60) as antwort:
        if antwort.status != 206:
            raise SystemExit(f"{url}: der Server gibt keinen Teil heraus (HTTP {antwort.status})")
        return antwort.read()


def groesse(url):
    anfrage = urllib.request.Request(url, method="HEAD", headers={"User-Agent": "LumiViz-make_drumgizmo"})
    with urllib.request.urlopen(anfrage, timeout=60) as antwort:
        return int(antwort.headers["Content-Length"])

STUFE = {"leise": 0.25, "mittel": 0.6, "voll": 0.9}  # Anteil der Spanne der Staerken

# Name im Loop (lib:<name>) -> (Kit, Instrument im Kit, Staerke)
WAHL = {
    # DRSKit: Rock'n'Roll, Hardrock, Jazz Brushes
    "drs_kick": ("DRSKit", "Kdrum_without_contact", "mittel"),
    "drs_kick_hard": ("DRSKit", "Kdrum_without_contact", "voll"),
    "drs_kick_soft": ("DRSKit", "Kdrum_without_contact", "leise"),
    "drs_snare": ("DRSKit", "Snare", "mittel"),
    "drs_snare_hard": ("DRSKit", "Snare", "voll"),
    "drs_rim": ("DRSKit", "Snare_rim", "mittel"),
    "drs_hat": ("DRSKit", "Hihat_closed", "mittel"),
    "drs_hat_hard": ("DRSKit", "Hihat_closed", "voll"),
    "drs_hat_foot": ("DRSKit", "Hihat_foot", "mittel"),
    "drs_hat_open": ("DRSKit", "Hihat_open", "mittel"),
    "drs_hat_open_hard": ("DRSKit", "Hihat_open", "voll"),
    "drs_tom1": ("DRSKit", "Tom1", "mittel"),
    "drs_tom2": ("DRSKit", "Tom2", "mittel"),
    "drs_tom3": ("DRSKit", "Tom3", "mittel"),
    "drs_tom1_hard": ("DRSKit", "Tom1", "voll"),
    "drs_tom2_hard": ("DRSKit", "Tom2", "voll"),
    "drs_tom3_hard": ("DRSKit", "Tom3", "voll"),
    "drs_ride": ("DRSKit", "Ride_tip", "mittel"),
    "drs_ride_soft": ("DRSKit", "Ride_tip", "leise"),
    "drs_crash_left": ("DRSKit", "Crash_left_tip", "mittel"),
    "drs_crash_left_hard": ("DRSKit", "Crash_left_tip", "voll"),
    # Entscheid D4 a (S124): der Schaft statt der Spitze — der Tip-Anschlag blieb 6,6 dB unter crash 1
    "drs_crash_right_shank_hard": ("DRSKit", "Crash_right_shank", "voll"),
    # CrocellKit: Heavy Metal
    "crocell_kick_left": ("CrocellKit", "KDrumL", "voll"),
    "crocell_kick_right": ("CrocellKit", "KDrumR", "voll"),
    "crocell_snare": ("CrocellKit", "Snare", "voll"),
    "crocell_hat": ("CrocellKit", "HihatClosed", "voll"),
    "crocell_hat_open": ("CrocellKit", "HihatOpen", "voll"),
    "crocell_tom1": ("CrocellKit", "Tom1", "voll"),
    "crocell_tom2": ("CrocellKit", "Tom2", "voll"),
    "crocell_tom3": ("CrocellKit", "FTom1", "voll"),
    "crocell_tom4": ("CrocellKit", "FTom2", "voll"),
    "crocell_ride": ("CrocellKit", "RideR", "voll"),
    "crocell_crash1": ("CrocellKit", "CrashL", "voll"),
    "crocell_crash2": ("CrocellKit", "CrashR", "voll"),
    "crocell_crash3": ("CrocellKit", "CrashRXtra", "voll"),
    "crocell_china": ("CrocellKit", "ChinaL", "voll"),
    "crocell_splash": ("CrocellKit", "SplashL", "voll"),
}


def lies_verzeichnis(url):
    """Das zentrale Verzeichnis der ZIP (auch ZIP64): je Datei Name, Groessen, Lage."""
    gesamt = groesse(url)
    schwanz_laenge = min(gesamt, 65536 + 22)
    schwanz = hole_bereich(url, gesamt - schwanz_laenge, gesamt - 1)
    ende = schwanz.rfind(b"PK\x05\x06")
    if ende < 0:
        raise SystemExit(f"{url}: kein Ende des Verzeichnisses gefunden")
    (_, _, _, _, anzahl, cd_groesse, cd_lage, _) = struct.unpack("<IHHHHIIH", schwanz[ende:ende + 22])
    if anzahl == 0xFFFF or cd_groesse == 0xFFFFFFFF or cd_lage == 0xFFFFFFFF:
        # ZIP64: der Zeiger steht 20 Byte vor dem Ende des Verzeichnisses
        zeiger = schwanz[ende - 20:ende]
        if zeiger[:4] != b"PK\x06\x07":
            raise SystemExit(f"{url}: ZIP64 ohne Zeiger")
        (z64_lage,) = struct.unpack("<Q", zeiger[8:16])
        z64 = hole_bereich(url, z64_lage, z64_lage + 55)
        if z64[:4] != b"PK\x06\x06":
            raise SystemExit(f"{url}: ZIP64-Ende nicht gefunden")
        anzahl, cd_groesse, cd_lage = struct.unpack("<QQQ", z64[32:56])
    verzeichnis = hole_bereich(url, cd_lage, cd_lage + cd_groesse - 1)
    dateien = []
    pos = 0
    for _ in range(anzahl):
        if verzeichnis[pos:pos + 4] != b"PK\x01\x02":
            raise SystemExit(f"{url}: Verzeichnis kaputt bei {pos}")
        (methode, crc, gepackt, ungepackt, nlen, elen, clen, lage) = struct.unpack(
            "<H4xIIIHHH8xI", verzeichnis[pos + 10:pos + 46])  # Zeit und Datum uebersprungen
        name = verzeichnis[pos + 46:pos + 46 + nlen].decode("utf-8", "replace")
        extra = verzeichnis[pos + 46 + nlen:pos + 46 + nlen + elen]
        # ZIP64-Feld: die grossen Werte in der Reihenfolge ungepackt, gepackt, Lage
        e = 0
        while e + 4 <= len(extra):
            kennung, laenge = struct.unpack("<HH", extra[e:e + 4])
            if kennung == 0x0001:
                werte = extra[e + 4:e + 4 + laenge]
                w = 0
                if ungepackt == 0xFFFFFFFF:
                    (ungepackt,) = struct.unpack("<Q", werte[w:w + 8]); w += 8
                if gepackt == 0xFFFFFFFF:
                    (gepackt,) = struct.unpack("<Q", werte[w:w + 8]); w += 8
                if lage == 0xFFFFFFFF:
                    (lage,) = struct.unpack("<Q", werte[w:w + 8]); w += 8
            e += 4 + laenge
        dateien.append({"name": name, "methode": methode, "crc": crc, "gepackt": gepackt,
                        "ungepackt": ungepackt, "lage": lage})
        pos += 46 + nlen + elen + clen
    return {"url": url, "groesse": gesamt, "verzeichnis_bytes": cd_groesse, "dateien": dateien}


def hole_datei(url, eintrag):
    """Eine Datei aus der ZIP holen — nur ihre Bytes, ueber den lokalen Kopf."""
    kopf = hole_bereich(url, eintrag["lage"], eintrag["lage"] + 29)
    if kopf[:4] != b"PK\x03\x04":
        raise SystemExit(f"{eintrag['name']}: kein lokaler Kopf")
    nlen, elen = struct.unpack("<HH", kopf[26:30])
    anfang = eintrag["lage"] + 30 + nlen + elen
    daten = hole_bereich(url, anfang, anfang + eintrag["gepackt"] - 1) if eintrag["gepackt"] else b""
    if eintrag["methode"] == 8:
        daten = zlib.decompress(daten, -15)
    elif eintrag["methode"] != 0:
        raise SystemExit(f"{eintrag['name']}: Packart {eintrag['methode']} unbekannt")
    if zlib.crc32(daten) != eintrag["crc"]:
        raise SystemExit(f"{eintrag['name']}: Pruefsumme falsch")
    return daten


def verzeichnis(ordner):
    ordner.mkdir(parents=True, exist_ok=True)
    for kit, url in KITS.items():
        inhalt = lies_verzeichnis(url)
        ziel = ordner / f"{kit}_inhalt.json"
        ziel.write_text(json.dumps(inhalt, indent=1), encoding="utf-8")
        print(f"{kit}: {len(inhalt['dateien'])} Dateien, Verzeichnis {inhalt['verzeichnis_bytes']} Byte -> {ziel}")
        xml = [e for e in inhalt["dateien"] if e["name"].lower().endswith(".xml")]
        geholt = 0
        for eintrag in xml:
            datei = ordner / eintrag["name"]
            if not datei.exists():
                datei.parent.mkdir(parents=True, exist_ok=True)
                datei.write_bytes(hole_datei(url, eintrag))
            geholt += eintrag["gepackt"]
        print(f"  {len(xml)} XML-Dateien, {geholt} Byte gepackt")


def waehle(ordner, kit, instrument, stufe):
    """Der Anschlag nach `STUFE`: (Datei im Ordner des Instruments, power, Kanaele je Mikrofon)."""
    wurzel = ET.parse(ordner / kit / instrument / f"{instrument}.xml").getroot()
    proben = []
    for probe in wurzel.iter("sample"):
        dateien = probe.findall("audiofile")
        kanaele = {a.get("channel"): int(a.get("filechannel")) for a in dateien}
        proben.append((float(probe.get("power")), dateien[0].get("file"), kanaele))
    proben.sort(key=lambda p: p[0])
    ziel = proben[0][0] + STUFE[stufe] * (proben[-1][0] - proben[0][0])
    power, datei, kanaele = min(proben, key=lambda p: abs(p[0] - ziel))
    return datei, power, kanaele


def holen(ordner):
    inhalt = {kit: {e["name"]: e for e in json.loads((ordner / f"{kit}_inhalt.json").read_text(encoding="utf-8"))["dateien"]}
              for kit in KITS}
    summe = 0
    for name, (kit, instrument, stufe) in WAHL.items():
        datei, power, _ = waehle(ordner, kit, instrument, stufe)
        eintrag = inhalt[kit][f"{kit}/{instrument}/{datei}"]
        ziel = ordner / eintrag["name"]
        if ziel.exists() and ziel.stat().st_size == eintrag["ungepackt"]:
            print(f"{name:22s} schon da: {eintrag['name']}")
            continue
        ziel.parent.mkdir(parents=True, exist_ok=True)
        ziel.write_bytes(hole_datei(KITS[kit], eintrag))
        summe += eintrag["gepackt"]
        print(f"{name:22s} {eintrag['name']}  power {power:.3f}  {eintrag['gepackt'] / 1e6:.2f} MB")
    print(f"geholt: {summe} Byte ({summe / 1e6:.1f} MB)")


def mische(kit, instrument, x, rate, kanaele, protokoll):
    """Die Mikrofone eines Anschlags zu Stereo (Spalten L, R)."""
    import numpy as np

    eigene = [m for m in EIGENE.get(instrument, []) if m in kanaele]
    links = np.zeros(len(x))
    rechts = np.zeros(len(x))
    haupt = None
    for mikrofon, kanal in kanaele.items():
        spur = x[:, kanal - 1].astype(np.float64)
        art, seite = MIKROFONE[kit].get(mikrofon, ("nah", "M"))
        if art == "nah":
            if mikrofon not in eigene:
                continue  # eine fremde Trommel: nur Uebersprechen
            if haupt is None and mikrofon == eigene[0]:
                haupt = spur
        g = 10 ** (PEGEL_DB[art] / 20)
        if seite in ("L", "M"):
            links += g * spur
        if seite in ("R", "M"):
            rechts += g * spur
    # ein zweites Nahmikrofon gegen das erste? Dann gedreht (in den ersten 30 ms nach dem Anschlag)
    if len(eigene) > 1 and haupt is not None:
        anschlag = int(np.argmax(np.abs(haupt) > 0.1 * np.abs(haupt).max()))
        stueck = slice(anschlag, anschlag + int(0.030 * rate))
        for mikrofon in eigene[1:]:
            spur = x[:, kanaele[mikrofon] - 1].astype(np.float64)
            if np.dot(haupt[stueck], spur[stueck]) < 0:
                links -= 2 * spur
                rechts -= 2 * spur
                protokoll.append(f"{mikrofon} gedreht")
    return np.stack([links, rechts], axis=1)


def kuerze(y, rate):
    """D6 a: ab 2 ms vor dem Anschlag bis zum Ende der Datei; die letzten 10 ms ausgeblendet."""
    import numpy as np

    spitzen = np.abs(y).max(axis=1)
    anfang = max(0, int(np.nonzero(spitzen > spitzen.max() * 10 ** (ANFANG_DB / 20))[0][0]) - int(0.002 * rate))
    z = y[anfang:].copy()
    aus = min(len(z), int(AUSBLENDEN_S * rate))
    if aus > 1:
        z[-aus:] *= (0.5 + 0.5 * np.cos(np.linspace(0.0, np.pi, aus)))[:, None]
    return z * (SPITZE / np.abs(z).max())


def schreiben(ordner):
    import warnings

    import numpy as np
    from scipy.io import wavfile
    from scipy.signal import resample_poly

    warnings.filterwarnings("ignore", message="Chunk")  # DrumGizmo-WAVs tragen eigene Abschnitte
    zeilen = {kit: [] for kit in KITS}
    gesamt = 0
    for name, (kit, instrument, stufe) in WAHL.items():
        datei, power, kanaele = waehle(ordner, kit, instrument, stufe)
        rate, x = wavfile.read(ordner / kit / instrument / datei)
        protokoll = []
        y = mische(kit, instrument, x, rate, kanaele, protokoll)
        if rate != RATE:
            from math import gcd
            t = gcd(RATE, rate)
            y = resample_poly(y, RATE // t, rate // t, axis=0)
        y = kuerze(y, RATE)
        ziel = HIER / f"{name}.wav"
        wavfile.write(ziel, RATE, np.round(y * 32767).astype(np.int16))
        gesamt += ziel.stat().st_size
        print(f"{name:22s} {len(y) / RATE:5.2f} s  {ziel.stat().st_size / 1e6:5.2f} MB  {', '.join(protokoll)}")
        zeilen[kit].append(f"| `{name}.wav` | {instrument.replace('_', ' ')}, {WAS[stufe]} hit | `{kit}/{instrument}/{datei}` |")
    abschnitt = [MARKE, "", "## Acoustic drums — DrumGizmo", ""]
    abschnitt += [
        "These sounds come from two drum kits made for **DrumGizmo** (<https://drumgizmo.org>), released under",
        "**Creative Commons Attribution 4.0 International** (<https://creativecommons.org/licenses/by/4.0/>).",
        "When the samples are used in a composition, any context in which other credits (e.g. to performers) are",
        "presented must include an attribution — for example: *Drums: DRSKit / CrocellKit by the DrumGizmo team, CC BY 4.0.*",
        "",
        "LumiViz changed them: of each hit, the close microphones of the drum that was hit (centre), the",
        "overheads at -6 dB (left and right) and the room microphones at -12 dB (left and right) are mixed to",
        "stereo; the close microphones of the other drums are left out. Silence before the hit is removed, the",
        "last 10 ms are faded out, the level is raised to a peak of 0.89, written as stereo, 16 bit, 44100 Hz.",
        "In LumiViz a sample `name.wav` is called `lib:name`.",
        "",
    ]
    for kit, (titel, seite, wer, satz) in HERKUNFT.items():
        abschnitt += [f"### {titel}", "", f"By {wer}. {satz} Source: <{seite}>, `{KITS[kit]}`.", "",
                      "| File | What | File in the kit |", "|---|---|---|"] + zeilen[kit] + [""]
    credits = HIER / "CREDITS.md"
    alt = credits.read_text(encoding="utf-8") if credits.exists() else ""
    vorne = alt.split(MARKE)[0].rstrip("\n") + "\n\n"
    credits.write_text(vorne + "\n".join(abschnitt), encoding="utf-8", newline="\n")
    print(f"{len(WAHL)} Samples, {gesamt / 1e6:.1f} MB, CREDITS.md")


def main():
    if len(sys.argv) != 3 or sys.argv[1] not in ("verzeichnis", "holen", "schreiben"):
        raise SystemExit(__doc__)
    ordner = Path(sys.argv[2])
    if sys.argv[1] == "verzeichnis":
        verzeichnis(ordner)
    elif sys.argv[1] == "holen":
        holen(ordner)
    elif sys.argv[1] == "schreiben":
        schreiben(ordner)


if __name__ == "__main__":
    main()
