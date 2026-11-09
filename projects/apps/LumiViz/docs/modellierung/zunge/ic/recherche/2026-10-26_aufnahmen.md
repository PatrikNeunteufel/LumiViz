# Recherche IC1: Durchschlagende Zunge — freie Aufnahmen und Messdaten

> **Auftrag:** Klasse 10 (Akkordeon, Mundharmonika, Harmonium, Melodica): freie Aufnahmen und Messdaten mit an der
> Quelle geprüfter Lizenz.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [S] = per curl selbst gelesener Rohtext/JSON; [W] = vom Abrufwerkzeug zitiert; [Z] = nur Zusammenfassung.
> **Reichweite:** im Bericht selbst (rund 57 Aufrufe; Freesound je Pack nur ein Sound geprüft).
> **Anmerkung der Sitzung:** Die Metadaten-CSVs von TinySOL und OrchideaSOL hat der Agent im Speicher gelesen, nicht
> gespeichert; Audio wurde nicht angefasst. Die Lieferungen der Klasse 10 schreibt die nächste Sitzung
> (Session-Abschluss nach den Berichten, Wunsch Patrik).
> **Von der Sitzung selbst nachgelesen (IC Session 2, 2026-10-26):** TinySOL (Zenodo-API `cc-by-4.0`, Größe; Akkordeon
> aus `TinySOL_metadata.csv` im Speicher ausgezählt: 689 Zeilen, MIDI 28–109, 82 Tonhöhen, pp/mf/ff je 82, p 8, 35
> nachgestimmt); GitHub-API `CC0-1.0` für VCSL und evanyerburgh/euterpea-harmonium; Freesound 330410 (CC0), 23039 und
> 325377 (CC BY 4.0) über den Lizenzlink der Seite; Commons „Accordion registers.ogg“ (API: „Public domain“, Größe,
> Beschreibung). Eingeflossen in `aufnahmen.md`.

---

# Klasse 10 „Durchschlagende Zunge“: freie Aufnahmen und Messdaten

**Auftrag:** Freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz für Akkordeon, Mundharmonika, Harmonium und Melodica, gegen die ein gerechnetes Modell gemessen werden kann.
**Datum:** 2026-10-26. Alle Abrufe am selben Tag.

**Reichweite:**
- **Zahl der Aufrufe:** rund 57 Werkzeugaufrufe mit etwa 80 Einzelabrufen. Davon 3 Websuchen, 29 WebFetch-Aufrufe und rund 25 curl-Läufe gegen die GitHub-, Zenodo- und Commons-API.
- **Orte:** GitHub (sgossner/VCSL, sgossner/VSCO-2-CE, Repo-Suche), Zenodo-API (TinySOL, OrchideaSOL, Good-Sounds, Suchen), Ircam-Forum-Lizenz, Iowa MIS, Philharmonia, Freesound (Such- und Pack-Seiten, 13 einzelne Sound-Seiten), Wikimedia-Commons-API (Suche und extmetadata), Magenta/NSynth.
- **Metadaten-CSV:** Die Metadaten-CSVs von TinySOL (317 KB) und OrchideaSOL (1,9 MB) wurden per curl in Python gelesen und ausgezählt, nicht gespeichert. Audio, ZIP und TAR wurden nicht angefasst.
- **Nicht erreichbar:** nichts. Zenodo sperrte nicht; ein erster Abruf mit Browser-User-Agent lieferte kein JSON, ohne User-Agent ging es.
- **Nicht geprüft:** der Inhalt der PDFs (AiP 2021 „Accordion acoustics“, Forum Acusticum 2023), die Freesound-API (braucht einen Schlüssel) und Freesound-Packs über die erste Ergebnisseite hinaus.

**Kennzeichnung:**
- **[S]** = wörtlich, selbst gelesen (Rohtext oder JSON per curl).
- **[W]** = wörtlich, vom Werkzeug zitiert (WebFetch).
- **[Z]** = nur Zusammenfassung.

**Lizenz je Sound:** Bei Freesound steht die Lizenz an jedem einzelnen Sound. Ich habe je Pack **einen** Sound geprüft. Für die anderen Sounds im selben Pack ist die Lizenz nicht belegt.

---

## 1. Bester Datensatz je Instrument

| Instrument | Bester Datensatz | Lizenz (Wortlaut) | Eignung |
|---|---|---|---|
| **Akkordeon** | **TinySOL** (Zenodo 3685367), Accordion ordinario: 689 Einzeltöne, E1–C#8, je pp/mf/ff | Feld `license`: `{'id': 'cc-by-4.0'}` [S]; Beschreibung: „TinySOL is offered free of charge under the terms of the Creative Commons Attribution 4.0 International (CC BY 4.0) license“ [S] | **Hoch** für Spektrum, Einschwingen und Dynamik über den ganzen Umfang (mono, 44,1 kHz/16 bit, Ircam-Studio). Register sind nicht ausgewiesen, Musette ist nicht belegt. |
| **Mundharmonika** | **VCSL**: Hohner Special 20 in C und in F (diatonisch) sowie Super 64 (chromatisch) | LICENSE-Datei: „CC0 1.0 Universal“ [S]; GitHub-Feld `spdx_id: CC0-1.0` [S] | **Mittel.** Haltetöne in 2–4 Spielarten (Soft/Normal/Accented/Vib, HandVib, Stac) sowie Release-Samples, aber nur C-, E- und G-Töne (beim F-Instrument F-, A- und C-Töne). Allem Anschein nach nur Blastöne, keine Ziehtöne und **keine Bendings**. |
| Mundharmonika, Bendings | Freesound n_audioman, „Harmonica_Bend_Mute 01.wav“ (Special 20 in C) | „Attribution 4.0“ [W] | **Gering.** Nur ein einzelner Bend von 2,4 s, Stereo, Raum unbekannt. |
| **Harmonium** | **donyaquick**, „Harmonium Samples - All Keys and Drones“ (Freesound 330410). Dieselbe Aufnahme liegt geschnitten im GitHub-Repo evanyerburgh/euterpea-harmonium. | Freesound: „Creative Commons 0“ [W]; Repo: LICENSE „CC0 1.0 Universal“, `spdx_id CC0-1.0` [S] | **Mittel.** Alle Tasten einzeln (42 Tasten, laut README „C1–F4“), dazu Drones; 96 kHz, Stereo, Euterpea Studio (Yale), „Some equalization applied“ [W]. Eine Stärke, keine Register. |
| **Melodica** | **fese**, „Hohner Melodica“ (Freesound Pack 1425) | „Attribution 4.0“ [W] (am Sound melodica-g4) | **Gering bis mittel.** „four samples/octave“ [W], mono, 24 bit, rund 9 s je Ton, eine Stärke. Die Alternative urlande (CC0) hat nur rund 1 s lange Töne. |

---

## 2. Alle Datensätze

Spalte „LumiViz“: frei / Namensnennung / ShareAlike / nicht.

| Datensatz (Fundort) | Lizenz (Wortlaut, Ort, Datum) | Inhalt | Größe, Format | Eignung | LumiViz |
|---|---|---|---|---|---|
| **TinySOL**, zenodo.org/records/3685367 | Zenodo-API `license: {'id': 'cc-by-4.0'}` [S]; Beschreibung „Conditions of Use“ (Zitat siehe Tabelle 1) [S]; dazu „TinySOL is the only subset of SOL which is released under a Creative Commons License“ [S]; 2026-10-26 | Siehe Abschnitt 3.1 | Ganzer Datensatz `TinySOL.tar.gz` 1.026.917.185 B, 14 Instrumente [S]; WAV mono, 44,1 kHz/16 bit, 2–10 s [S] | **Hoch** für Akkordeon | **Namensnennung** |
| **OrchideaSOL**, zenodo.org/records/3740399 | Zenodo-Feld `cc-by-4.0` gilt laut Beschreibung **nur für die Metadaten**: „The audio samples in OrchideaSOL are offered free of charge under the Ircam Forum License“ [S]; Ircam-Forum-Lizenz Art. 2: „Le droit d'utilisation porte sur une utilisation non commerciale.“ [W] (Fassung „Novembre 2025“, v4.1.8) | Akkordeon 872 Clips [S]: ordinario 689 (dieselben wie TinySOL), combination_of_registers 107 (mf), sforzato 73 (fp), breath 2, key_click 1 [S] | Audio nicht bei Zenodo, sondern „after registering to the Ircam forum“ [S] | Hoch für Registerkombinationen und Balggeräusch (breath), aber Lizenz | **nicht** (nicht kommerziell) |
| **VCSL**, github.com/sgossner/VCSL | Siehe Tabelle 1; README: „you can do whatever you want with these sounds (even make commercial software), no royalties, no credit“ [S] | Siehe Abschnitt 3.2. **Kein** Akkordeon, Harmonium oder Melodica (Free Aerophones: nur die drei Mundharmonikas und Siren [S]) | WAV, Sustains je rund 0,5–1,0 MB [S]; Rate, Kanäle, Mikrofon und Raum je Datei nicht belegt | Mittel für Mundharmonika | **frei** |
| **VSCO 2 CE**, github.com/sgossner/VSCO-2-CE | `spdx_id CC0-1.0`, LICENSE „CC0 1.0 Universal“ [S] | **Keine** Durchschlagzunge. Treffer „harmon“ war nur „Trumpet/harmonM-sus“ (Dämpfer) [S]; Keys/Organ ist eine Pfeifenorgel von Ivy Audio [S] | – | entfällt | – |
| **Univ. of Iowa MIS**, theremin.music.uiowa.edu/MIS.html | „freely available on this website“, „without restrictions“ [W] | **Kein** Akkordeon und keine Mundharmonika [Z]; reflexionsarmer Raum, KM 84 bzw. Earthworks QTC-40 [W] | – | entfällt | – |
| **Philharmonia**, philharmonia.co.uk/resources/sound-samples | „You are free to use these samples as you wish, including releasing them as part of a commercial work.“; „they must not be sold or made available 'as is'“ [W] | Keine Durchschlagzunge erwähnt [Z] | – | entfällt | – |
| **Good-Sounds**, zenodo.org/records/4588740 | Feld `cc-by-4.0` [S], Beschreibung dagegen: „This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License“ [S]. **Widerspruch bestätigt.** | 12 Instrumente: flute, cello, clarinet, trumpet, violin, sax_alto, sax_tenor, sax_baritone, sax_soprano, oboe, piccolo, bass [S]. **Kein Akkordeon.** | ZIP 13,9 GB [S] | entfällt | – |
| **NSynth**, magenta.tensorflow.org/datasets/nsynth | „made available by Google Inc. under a Creative Commons Attribution 4.0 International (CC BY 4.0) license“ [W] | Familien reed und organ ohne Klarnamen; ob Akkordeon oder Mundharmonika enthalten sind, ist nicht feststellbar [Z]; 4 s, 16 kHz, 5 Anschlagstärken [W] | – | Gering (16 kHz, anonym) | Namensnennung |
| Freesound juskiddink „Accordion Notes.wav“ (sounds/120715) | „Attribution 4.0“ [W] | „Five accordion notes ranging from highest to lowest“; Paolo Soprani 80 Bass; „Rode NT4 into ZoomH4“ [W] | 24,5 s, 44,1 kHz/24 bit, Stereo, 6,2 MB [W] | Gering (5 Töne in einer Datei) | Namensnennung |
| Freesound LeQuest „Giulietti … 'Violin Register' set“ (Pack 23701; Sound 419274) | „Attribution NonCommercial 3.0“ [W] | 10 Töne zu je 10 s, Register „violin“ (Musette-artig? nicht belegt) [Z] | 44,1 kHz/16 bit, Stereo [W] | Mittel (einzelnes Register), aber Lizenz | **nicht** |
| Freesound tim.kahn „Old Toriana Accordion“ (Pack 11810; Sound 187053) | „Attribution NonCommercial 4.0“ [W] | Akkordknöpfe; Balg Zug und Druck: „first pulling the accordion apart and then pushing it back together“ [W]; AT4021 XY [W] | 48 kHz/24 bit, Stereo [W] | Gering, Lizenz | **nicht** |
| Freesound Miles_Thompson „Cellini Accordion“ (Pack 31477) | Lizenz je Sound **nicht geprüft** | „Sampled accordion keys“ [W]; m4a und mp3 (verlustbehaftet) | – | Gering | offen |
| Freesound CBeeching „Chromatic Harmonica“ (Pack 7336, Sound 116955) und „Mixed Standard Harmonica (all keys)“ (Pack 7335, Sound 116924) | je „Creative Commons 0“ [W] | „recorded in mono with my AKG C214 on my stairs“ bzw. „on my stair case for that oakey timbre“ [W] | 44,1 kHz/32 bit, mono [W] | Gering (halliges Treppenhaus) | frei (nur die zwei geprüften Sounds) |
| Freesound Sadiquecat „Harmonicas“ (Pack 43058, Sound 792630) | „Creative Commons 0“ [W] | Harley Benton in C, Blas- und Ziehtöne gemischt, vor allem Akkorde; NT5; „inside a blanket castle“; Pegel nachträglich angehoben [W] | 192 kHz/24 bit, mono [W] | Gering bis mittel (trocken, aber Akkorde und ein Anfänger) | frei (nur der geprüfte Sound) |
| Freesound n_audioman „Harmonica_Bend_Mute 01.wav“ (325377) | „Attribution 4.0“ [W] | Bend auf Special 20 in C [W] | 2,4 s, 48 kHz/16 bit, Stereo [W] | Gering | Namensnennung |
| Freesound d4vidsc „Diatonic Harmonica note D“ (521853) | „Attribution NonCommercial 4.0“ [W] | ein Ton D, C-Instrument (vermutlich Ziehton) [Z] | 4 s, mono [W] | Gering | **nicht** |
| Freesound donyaquick „Harmonium Samples - All Keys and Drones“ (330410) | „Creative Commons 0“ [W] | Siehe Tabelle 1 | 7:37 min, 96 kHz/16 bit, Stereo, 167,4 MB [W] | Mittel | frei |
| GitHub evanyerburgh/euterpea-harmonium | LICENSE „CC0 1.0 Universal“ [S] | Dieselbe Euterpea-Aufnahme, in Einzel-WAVs geschnitten (Harmonium_C1…, Drones C#2, D#2, G#2) [S]; README „42 keys spanning C1–F4“ [S] | 76,5 MB Repo [S] | Mittel (bequemer als die Gesamtdatei) | frei |
| Freesound fese „Hohner Melodica“ (Pack 1425, Sound 23039) | „Attribution 4.0“ [W] | 4 Töne je Oktave [W] | 9,2 s, 44,1 kHz/24 bit, mono [W] | Gering bis mittel | Namensnennung |
| Freesound urlande „Melodica“ (Pack 9578, Sound 155090) | „Creative Commons 0“ [W] | Einzeltöne sol0…si2 [W] | 0,99 s, 16 bit, Stereo [W] | Gering (zu kurz für Haltetöne) | frei (nur der geprüfte Sound) |
| GitHub NeoSoundFonts/SimonePiervergili-LeeOskar, -HohnerSilverstar, -UnknownHarmonica | LICENSE „CC0 1.0 Universal“ [S] | je 10 WAVs, C, E und G über 3–4 Oktaven (060, 064, 067 … 096) [S], also wie VCSL nur Dreiklangstöne | je 4–6 MB [S] | Gering bis mittel; Aufnahmedaten fehlen | frei |
| GitHub michael02022/basic-harmonica | LICENSE CC0 1.0 [S] | 7 FLAC-Töne und Releases, Tonhöhen nicht benannt [S] | 3,2 MB [S] | Gering | frei |
| GitHub filipemlins/Harmonica-Dataset | **keine Lizenz** (`license: None`) [S] | Diatonisch in Bb, C und G, Phrasen mit Annotation in Hz und MIDI [S] | 62,6 MB [S] | Für Tonhöhenverlauf und Bends interessant, aber ohne Lizenz | **nicht** |
| Commons „Accordion registers.ogg“ | LicenseShortName „Public domain“, Artist „Necz0r“ [S] | „the same octave cycled through five different accordion keyboard tones“ [S] | 26,3 s, Ogg (verlustbehaftet) [S] | Mittel als Registervergleich (eine Oktave, 5 Register) | frei (PD-Vorlage auf der Dateiseite noch prüfen) |
| Commons „C1 cis1 harmonium.ogg“ | „CC BY-SA 3.0“, Joachim Mohr [S] | c′ und cis′ (Schwebung) [S] | 12,5 s, Ogg | Gering | ShareAlike |
| Commons „Harmonium (C Major).wav“ | „CC BY-SA 4.0“, ShortStat [S] | C-Dur-Tonleiter [S] | 4,7 s, WAV | Gering | ShareAlike |
| Commons „Electra-Melodica-Hohner-1967 …flac“ | „CC BY-SA 4.0“, Reuter und Schefczik [S] | Electra-Melodica, elektrisch verstärkt [S] | 8,3 s, FLAC | Gering (Sonderbauart) | ShareAlike |
| Commons „Harmonica-pumping.ogg“ | „CC BY-SA 3.0“, David Monniaux [S] | „Harmonica demo (Hohner Marine Band)“ [S] | 8,6 s, Ogg | Gering | ShareAlike |
| Commons „Harmonica 02.ogg“ und „Harmonica 05.ogg“ | „Public domain“ (PDSounds) [S] | Ursprung „MP3 Mono 22kHz“ [S] | 17 s bzw. 6 s | Gering (verlustbehaftet, 22 kHz) | frei |
| Commons „Accordion chords-01.ogg“ | „CC BY-SA 3.0“ [S] | Dur- und Mollakkorde [S] | 11,8 s, Ogg | Gering | ShareAlike |
| Commons „Zunge.abwaerts.ogg“ und „Zunge.aufwaerts.ogg“ | „CC BY-SA 3.0“ [S] | „tuning an organ reed pipe“ [S]. Aufschlagende Zunge, **nicht Klasse 10** | – | entfällt | – |

---

## 3. Inhalt der wichtigsten Datensätze

### 3.1 TinySOL, Akkordeon (ausgezählt aus TinySOL_metadata.csv) [S]

- **Umfang:** 689 Zeilen „Keyboards/Accordion/ordinario“, MIDI 28–109 (E1–C#8), 82 Tonhöhen.
- **Stärken:** pp, mf und ff je über alle 82 Tonhöhen. Zählung: mf 302, pp 194, ff 185, p 8.
- **Mehrfachaufnahmen:** Kennungen alt1 bis alt5 (alt1 209, alt2 136, alt3 52, alt4 35, alt5 3). Ihre Bedeutung beim Akkordeon nennt die Beschreibung nicht.
- **Nachbearbeitung:** 35 Dateien digital nachgestimmt („Needed digital retuning“ TRUE).
- **Nicht dokumentiert:** Register, Balgrichtung, Mikrofon, Raum.

### 3.2 VCSL, Mundharmonikas (aus dem Dateibaum) [S]

**Hohner Special 20 in C:**
- **Haltetöne:** Soft, Normal, Vib.
- **Release-Samples:** ebenfalls Soft, Normal, Vib.
- **Tonhöhen:** C3–C6 sowie E3–E5 und G3/G4–G5.

**Hohner Special 20 in F:**
- **Haltetöne:** Accented, HandVib, Normal, Stac (mit zwei Round Robins, „rr1/rr2“), Vib.
- **Tonhöhen:** F, A und C, von F3 bis F6.

**Hohner Super 64 (chromatisch):**
- **Haltetöne:** Accented, Normal, Vib.
- **Tonhöhen:** C, E und G, von C2 bis G5.

**Fehlt in allen dreien:** Bend-, Overblow- und Ziehton-Ordner gibt es nicht.

**Hinweis zur Verwendung:** Die Datei „Suggested parameters.txt“ empfiehlt für die Releases „Add a 100ms attack“ [S].

---

## 4. Messdaten (Balg- und Atemdruck)

Keine Datensätze mit Lizenz gefunden. Das betrifft die Zenodo-API (Datensätze zu „accordion audio“, „harmonica audio“, „free reed“, „bellows pressure accordion“), die GitHub-Repo-Suche und zwei Websuchen.

Was die Suche dazu nennt [Z]:
- Forum Acusticum 2023 (dael.euracoustics.org/confs/fa2023/data/articles/000853.pdf): Harmoniumzunge mit Laservibrometer und Druckaufnehmer, laut Zusammenfassung „0.2 to 1.5 kPa“. Das ist eine Veröffentlichung, kein Datensatz; das PDF habe ich nicht gelesen.
- DAGA 2021 (Pomberger u. a.) zu Mundharmonika-Stimmplatten.
- AiP 2021 „Accordion acoustics“.
- Bahnson, Antaki, Beery 1998 (JASA 103(4)).

Ein Balggeräusch gibt es nur in **OrchideaSOL** (Technik „breath“, 2 Clips), lizenzrechtlich nicht übernehmbar.

---

## Nicht gefunden

Kein Treffer ist kein Beweis.
- **Akkordeon:** kein freier Datensatz mit **ausgewiesenen Einzelregistern** (8′ allein, Musette 3-chörig) über den Umfang. Nur Commons „Accordion registers.ogg“ (eine Oktave, PD) und LeQuest „Violin Register“ (NC).
- **Mundharmonika:** kein freier Datensatz mit systematischen **Zieh- und Blastönen je Kanal** samt **Bendings** und Overblows. Das einzige Material mit Phrasen und Annotation ist filipemlins, ohne Lizenz.
- **Harmonium und Melodica:** keine Aufnahme in mehreren Stärken.
- **Messdaten:** keine frei lizenzierten Balg- oder Atemdruckmessungen und keine Spielerstudie mit Druckkurven auf Zenodo.
- **VSCO 2 CE, Iowa MIS, Philharmonia, Good-Sounds:** keine Durchschlagzunge.

---

## Eigene Einschätzung (kein Beleg)

- **Akkordeon:** TinySOL ist mit Abstand die beste übernehmbare Quelle (CC BY 4.0): drei Stärken über E1–C#8 aus einem Studio. Der tiefe Umfang bis E1 lässt Bass- oder Konverterseite oder ein tiefes Register vermuten. Die alt1–alt5-Aufnahmen bei mf könnten Register sein, das ist nicht belegt. Vor einer Messung lohnt es, ein paar alt-Varianten anzuhören.
- **Mundharmonika:** VCSL hat bei Special 20 in C nur die Töne C, E und G. Das passt genau zu den Blastönen einer C-Harmonika, die Ziehtöne (D, F, A, H) fehlen; beim F-Instrument und den NeoSoundFonts-Repos ist es genauso. Für das Modell heißt das: Blaszunge gut belegbar, Ziehzunge und Bending (das Zusammenspiel beider Zungen eines Kanals) **nicht**. Für Bendings ist eine eigene Aufnahme realistischer als ein weiterer Fund. Die Oktavangaben C2–G5 beim Super 64 wirken gegenüber dem üblichen Umfang um eine Oktave tief benannt; das ist ungeprüft.
- **Harmonium:** Die Euterpea-Aufnahme (CC0, 96 kHz) taugt für Spektrum und Einschwingen je Taste. „C1–F4“ ist vermutlich eine Benennungskonvention des Sampler-Repos und nicht die klingende Lage.
- **Melodica:** dünn belegt; fese (CC BY 4.0) ist das Brauchbarste.
- **Lizenzfallen:**
  - OrchideaSOL: Das Zenodo-Feld `cc-by-4.0` gilt nur für die Metadaten; Audio unter Ircam-Forum-Lizenz, nicht kommerziell.
  - Good-Sounds: Feld und Beschreibung widersprechen sich, und Akkordeon ist ohnehin nicht dabei.
  - Freesound: Die Lizenz gilt je Sound, nicht je Pack; vor einer Übernahme jede Datei einzeln prüfen.
  - Commons „Public domain“ (Accordion registers, PDSounds): die Vorlage auf der Dateiseite gegenlesen.
- **Messdaten:** Für Balg- und Atemdruck bleibt nur eine eigene Messung oder eine Anfrage an die Autoren der Forum-Acusticum-2023- und DAGA-2021-Arbeiten.
