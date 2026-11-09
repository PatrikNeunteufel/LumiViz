# Recherche IC2: Gezupfte Saite (Deluxe) — freie Aufnahmen und Messdaten

> **Auftrag:** Klasse 2 (Akustikgitarre Stahl/Nylon, Cembalo, Harfe, Koto, Laute/Oud): freie Aufnahmen und Messdaten
> von Korpussen mit an der Quelle geprüfter Lizenz.
> **Datum:** 2026-10-26 (IC Session 2).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [S] = per curl selbst gelesener Rohtext/JSON; [W] = vom Abrufwerkzeug zitiert; [Z] = nur Zusammenfassung.
> **Reichweite:** im Bericht selbst (48 Werkzeugaufrufe, rund 230 HTTP-Abfragen; GitHub-Suche ab etwa dem 12. Aufruf am
> Rate-Limit gescheitert).
> **Von der Sitzung selbst nachgelesen (IC Session 2, 2026-10-26):** Iowa MIS.html („without restrictions“) und
> MISguitar.html (Raimundo 118, Anechoic Chamber, 5 feet, QTC40); Zenodo-API `cc-by-4.0` und Dateigrößen für 10159492,
> 3371780, 4604577, 10079910, 19823590; VCSL-Dateibaum (Zweig `master`, 4550 Einträge, WAV je Ordner gezählt);
> Freesound 381515 (CC0-Link). Eingeflossen in `aufnahmen.md`.

---

# Klasse 2 „Gezupfte Saite (Deluxe)“: freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz

**Auftrag:** Für Akustikgitarre (Stahl und Nylon), Cembalo, Harfe, Koto und Laute/Oud die besten frei lizenzierten Datensätze finden, dazu Messdaten von Korpussen (Stegadmittanz, Moden, Impulsantworten).
**Datum:** 2026-10-26, alle Abrufe an diesem Tag.

**Reichweite:**
- **Aufrufe:** 48 Werkzeugaufrufe mit zusammen etwa 230 HTTP-Abfragen. Kein Audio, keine ZIP-Datei, kein Range-Abruf, keine Datei geschrieben.
- **Gelesene CSV-Metadaten** (im Speicher, nicht gespeichert): TinySOL, OrchideaSOL, MusicNet, Medley-solos-DB.
- **Zenodo:** Etwa 35 Suchbegriffe, u. a. bridge admittance/mobility, harpsichord, harp, koto, oud, lute/laute, cembalo, guzheng, qanun, makam, Arab-Andalusian, guitar impulse response, soundboard, Ersteller „Mores“. Dazu 25 Einzeldatensätze gelesen.
- **GitHub:** Repo-Suche mit etwa 15 Begriffen. Ab etwa dem 12. Suchaufruf griff das Rate-Limit („guitar impulse response“ und „guitar body model“ lieferten nichts mehr), diese Suchen sind also nicht gelaufen. Orgs `freepats` und `sfzinstruments` vollständig gelistet; Bäume von VCSL und VSCO-2-CE ausgezählt.
- **Freesound:** Nur HTML, ohne API-Schlüssel. 9 Suchen, 24 Pack-Seiten. Den Lizenz-Link je Sound habe ich bei 5 Sounds auf der Sound-Seite selbst geprüft. Alle anderen Lizenzangaben stammen aus der Pack-Liste, die die Lizenz je Sound anzeigt.
- **Wikimedia Commons:** API mit `extmetadata`, 4 Suchen und 3 Dateien im Detail. Die 4 geratenen Kategorien gibt es nicht.
- **archive.org:** 5 Suchen, nichts Brauchbares.
- **Hugging Face:** 8 Suchen über die API-Tags.
- **Webseiten:** Iowa MIS (Startseite und Gitarrenseite), FreePats (Index, Nylon-, Stahl- und Harfenseite), NSynth-Seite.
- **Nicht geprüft:** Philharmonia (war vorgegeben), Sonatina Symphonic Orchestra, Soni Musicae (Blanchet-Cembalo), Inhalt der PDFs (Mores: Method.pdf u. a.).
- Kein Treffer ist kein Beweis.

**Kennzeichnung:**
- **[S]:** Rohtext oder JSON selbst per curl gelesen.
- **[W]:** vom Abrufwerkzeug zitiert. Kommt nicht vor, alles lief über curl.
- **[Z]:** nur Zusammenfassung oder Vorwissen.

---

## 1 Bester Datensatz je Instrument

| Instrument | Datensatz | Lizenz-Wortlaut (Ort) | Eignung |
|---|---|---|---|
| **Nylongitarre** | **Iowa MIS „Guitar“** (Raimundo 118) | Seitentext: „may be downloaded and used for any projects, without restrictions“ (theremin.music.uiowa.edu/MIS.html) [S]. Keine formale Lizenz. | **Sehr gut:** schalltoter Raum (Anechoic Chamber), 5 ft, Earthworks QTC40, pp/mf/ff je Saite, 24 bit/96 kHz. Chromatische Läufe je Datei, müssen zerschnitten werden. |
| **Stahlsaitengitarre** | **AG-PT-set** (Zenodo 10159492) | `license.id: cc-by-4.0` [S] | Brauchbar: 7 Stahlsaitengitarren, 6 Spieler, 32 592 Noten mit Onset-Markierung, Techniken „Pick Near Bridge“ und „Pick Over the Soundhole“. Keine pp/ff-Stufen beschrieben; Mikrofon im gelesenen Text nicht genannt; 6,7 GB. Ein trockener Einzelton-Satz für Stahl mit offener Lizenz fehlt. |
| **Cembalo** | **VCSL**, Ordner `Chordophones/Zithers/Harpsichord, *` (5 Instrumente) | GitHub `license.spdx_id: CC0-1.0`; README: „This collection is under a Creative Commons 0 license“ [S] | **Gut:** Sustain- und Release-Samples getrennt, Register (Lute/Normal bzw. High/Low), Mikrofon „Far“. |
| **Harfe** | **VCSL Concert Harp** (KSHarp) und **Folk Harp** (EWHarp) | CC0-1.0 wie oben [S] | **Gut:** f/mf bzw. v2/v3. Laut FreePats-Seite: kleiner Raum in Boston, 2× Rode NT1-A (spaced) und 2× AT Pro 37 (nah) [S]. |
| **Koto** | **Kein echter Koto-Satz gefunden.** Ersatz: VCSL **Dan Tranh** (vietnamesische Wölbbrettzither) | CC0-1.0 [S] | Nur Ersatz: 115 Dateien, Einzeltöne mf/ff, Vibrato, FX. |
| **Laute/Oud** | **Kein frei lizenzierter Einzelton-Satz gefunden.** Notbehelf: Commons „Oud open Strings.ogg“ | `LicenseShortName: CC BY-SA 3.0` (zusätzlich Kategorie GFDL) [S] | Schwach: eine Datei mit 61 kB, offene Saiten. Credit-Feld nennt eine Ursprungsdatei „.mid“, die Herkunft ist also unklar. |

---

## 2 Alle Datensätze

**Übernahme nach LumiViz:**
- **frei:** CC0 oder gleichwertig
- **Namensnennung:** CC BY
- **ShareAlike:** CC BY-SA oder GPL
- **nicht:** NC, ND, Ircam-Lizenz oder unklar

| Fundort | Lizenz (Ort, 2026-10-26) | Inhalt | Größe/Format | Eignung | Übernahme |
|---|---|---|---|---|---|
| Iowa MIS Guitar | Webseite, Zitat in Tabelle 1 [S] | Nylon (Raimundo 118), pp/mf/ff, Saiten E–A–D–G–B–E, 90 AIFF-Links | 16 bit/44 kHz und 24 bit/96 kHz, Zip | Nylon, trocken: beste Quelle | frei (formlose Freigabe; vor Übernahme schriftlich bestätigen lassen) |
| VCSL (github sgossner/VCSL) | `spdx_id: CC0-1.0` [S] | Concert Harp 45 Dateien/77 MB; Folk Harp 58/83 MB; Cembali English 110/94 MB, Flemish 108/126 MB, French 58, Italian 63, Unk 122; Dan Tranh 115/101 MB; Strumstick 57/90 MB; Psaltery 33 | WAV | Harfe, Cembalo, Koto-Ersatz | frei |
| VSCO 2 CE (github sgossner/VSCO-2-CE) | `spdx_id: CC0-1.0` [S] | `Strings/Harp`: 23 Dateien KSHarp, meist mf. Teilmenge von VCSL. Keine Gitarre. | WAV | Harfe (VCSL ist vollständiger) | frei |
| FreePats Spanish classical guitar (github freepats) | `spdx_id: CC0-1.0`; README „Creative Commons CC0 public domain dedication“ [S] | Nylon, 2008, AKG Perception 120. README: „recording conditions were far from ideal, so several filters have been applied“ | SFZ+FLAC 4,5 MiB | Nur grob, weil gefiltert | frei |
| FreePats FSS Steel-String (Webseite) | „GNU General Public License version 3 or later, with a special exception“ [S] | Teilmenge der FlameStudios-Gigabank, bearbeitet | SFZ/WAV 13 MiB | Stahl, bearbeitet | ShareAlike (GPL; Ausnahme nicht gelesen) |
| FreePats Concert Harp | „CC0 1.0“ [S] | Aufbereitete VCSL-Samples, 2 Velocity-Stufen | SFZ 4,8–9,1 MiB | Harfe (Originale besser bei VCSL) | frei |
| AG-PT-set, Zenodo 10159492 | `cc-by-4.0` [S] | 7 Stahlsaitengitarren, 12 Techniken, 15 h 55 min, 32 592 Noten markiert | 1 ZIP, 6,75 GB | Stahl, Anschlagort, Onsets | Namensnennung |
| GuitarSet, Zenodo 3371780 | `cc-by-4.0` [S] | Darbietungen (keine Einzeltöne); Mikrofon, Mono-Pickup, Hex-Pickup | ZIPs: mic 0,66 GB, hex 3,2/3,6 GB | Hex-Pickup gibt Einzelsaiten, aber nicht trocken als Korpus | Namensnennung |
| Five guitar dataset, Zenodo 4988354 | `cc-by-4.0` [S] | 5 Gitarren (davon 2 akustisch: Larrivée OM-406, Eastman), Songs, DI/Handy/Computer-Mikro | WAV, je etwa 8 MB | Kaum geeignet | Namensnennung |
| NSynth (magenta) | Seite: „Creative Commons Attribution 4.0 International (CC BY 4.0)“ [S] | Einzeltöne MIDI 21–108, Velocity 25/50/75/100/127, 4 s; Familie „guitar“, Quelle „acoustic“. Instrumente aus kommerziellen Bibliotheken, ohne Namen | 16 kHz | Nur Statistik | Namensnennung (Herkunft aus Fremdbibliotheken unklar) |
| TinySOL, Zenodo 3685367 | `cc-by-4.0` [S] | Ausgezählt: **keine Gitarre, keine Harfe** (14 Instrumente) | – | – | – |
| OrchideaSOL, Zenodo 3740399 | Feld `license.id: cc-by-4.0` [S]. **Widerspruch:** Die Beschreibung sagt „audio data … complies with the Ircam Forum License“; nur die Metadaten stehen unter CC BY 4.0. Audio nur nach Anmeldung im Ircam Forum. | Ausgezählt: **Guitar 353** (ordinario 210, davon mf 152/ff 44/pp 44); **Harp 507** | – | Gute Einzeltöne, rechtlich gesperrt | nicht |
| IDMT-SMT-Guitar, Zenodo 7544110 | `cc-by-nc-nd-4.0` [S] | E- und Akustikgitarren, Einzeltöne und Techniken | ZIP 1,33 GB, 44,1 kHz mono | – | nicht |
| Medley-solos-DB, Zenodo 3464194 | `cc-by-4.0` [S] | Ausgezählt: nur „distorted electric guitar“ (1854); keine Zupfinstrumente der Klasse | – | – | – |
| MusicNet, Zenodo 5120004 | `cc-by-4.0` [S] | Ausgezählt: 4 Aufnahmen „Violin and Harpsichord“; keine Gitarre | Stücke | Unbrauchbar | – |
| Good-sounds, Zenodo 4588740 | `cc-by-4.0` [S] | Instrumentliste: flute, cello, clarinet, trumpet, violin, sax ×4, oboe, piccolo, bass. **Keine Zupfinstrumente** | – | – | – |
| Freesound SpeedY Pack 469 „Nylon Guitar Single notes.“ | Sound 8403: Link `creativecommons.org/publicdomain/zero/1.0/` [S] | Leere Saiten: pluck, pick, Flageolett | WAV | Ergänzung Nylon | frei |
| Freesound Kyster Packs 7398/7397 (Nylon) | Sound 117852: `creativecommons.org/licenses/by/4.0/` [S] | Chromatische Einzeltöne, leere Saiten | WAV | Ergänzung | Namensnennung |
| Freesound pitx Pack 1098 „Spanish Guitar Notes“ | Pack-Liste: „Attribution NonCommercial“ [S] | Einzeltöne Eb3–F5 … | WAV | – | nicht |
| Freesound harri Pack 730 „acoustic_guitar“ | Pack-Liste: „Attribution“ [S] | Töne, MP3 | MP3 | Schwach | Namensnennung |
| Freesound pjcohen Pack 21464 „Harpsichord“ | Sound 381515: CC0-Link; Beschreibung „recorded with overhead mics“ [S] | Mindestens 12 Töne, D/E in Oktaven 1–5 (nur Seite 1 gezählt) | WAV | Cembalo-Ergänzung | frei |
| Freesound loofa 1491 „Clavecin“; mogigrumbles 29619 (CS-80-Synth) | „Attribution NonCommercial“ [S] | – | – | – | nicht |
| Freesound sgossner 21029 „VSCO 2 CE – Harp“ | Pack-Liste: „Creative Commons 0“ [S] | Wie VSCO | – | – | frei |
| Freesound tarane468 30959 „Arabic Oud“ | Sound 550882: CC0-Link [S]. **Widerspruch:** Beschreibung „I found this sound somewhere in the net as a free sound for Yamaha Motif“ | 10 Töne D1–C#4 | WAV | Rechtekette fehlt | **nicht** (CC0 vom Hochlader für fremdes Material) |
| Freesound xserra 11537/7862 („andalusian music“, „turkish music“) | Pack-Liste: „Attribution“ [S] | Musikausschnitte, keine Einzeltöne (nach Titel) | – | Kaum geeignet | Namensnennung |
| Freesound ingsey101 Sound 51404 „koto n strings“ | `creativecommons.org/licenses/by/3.0/` [S]. Beschreibung: „This is a Koto I think“ | 1 Datei, Herkunft unklar | WAV | Schwach | Namensnennung (fraglich) |
| Commons „Koto performance.ogg“ | `LicenseShortName: CC BY 3.0`, Artist Torsodog, 2006 [S] | 13-saitige Koto, „Sakura Sakura“ | OGG, 564 kB | Nur zum Hören | Namensnennung |
| Commons „Oud open Strings.ogg“ | `CC BY-SA 3.0` (zusätzlich GFDL) [S] | Leere Saiten, arabische Stimmung | OGG, 61 kB | Notbehelf | ShareAlike |
| Commons „Oud music by Andy R. Jordan …“ | `CC BY-SA 3.0` [S] | Musikstücke, WAV bis 19,7 MB | WAV | Kaum | ShareAlike |
| Arab-Andalusian corpus, Zenodo 1291776 | `cc-by-nc-4.0` [S] | 164 Konzerte, MP3, über 125 h | 8,9 GB | – | nicht |
| Turkish Makam Datensätze (Zenodo 1284501 u. a.) | `cc-by-4.0` bzw. „other-open“ [S] | Nach Titeln nur Metadaten und Partituren, kein Audio geprüft | – | – | – |
| ccmusic-database/Guzheng_Tech99 (Hugging Face) | Tag `license:cc-by-nc-nd-4.0` [S] | Guzheng-Spieltechniken | – | Koto-Verwandter | nicht |
| sfzinstruments: Clavecin, OvationGuitar | Kein `license` im Repo [S]; README „Converted from the original GigaSampler version by S. Christian Collins“ | – | – | – | nicht (unklar) |
| sfzinstruments: hungarian_zither, cithara-barbarica | `CC0-1.0` [S] | Zither (Strums), Leier mit Metallsaiten (mehrere Zupfarten) | SFZ | Nur Nebenquelle für Zupfmodelle | frei |
| yl92/blanchet-1720-soundfonts | `license: null` [S] | JS-Soundfonts aus Soni-Musicae-Samples | – | Lizenz der Soni-Musicae-Samples nicht geprüft | offen |
| Philharmonia | Vorgegeben: „must not be sold or made available ‘as is’“ (nicht erneut geprüft) | Gitarre, Mandoline, Banjo [Z] | – | – | nicht (ohne Rechtsprüfung) |

---

## 3 Inhalt der wichtigsten Datensätze (ausgezählt)

**Iowa MIS Guitar [S]**
- Kopfzeile der Seite: Model „Raimundo 118“, Performer Brian Penkrot, December 11, 2011, „Anechoic Chamber“, Distance „5 feet“, Microphone „Earthworks QTC40“, Interface Metric Halo.
- Formate: 16 bit/44 kHz und 24 bit/96 kHz, Stereo und Mono.
- 90 eindeutige AIFF-Links mit den Stärken pp, mf und ff je Saite.
- Dateischema `Guitar.<dyn>.sul<Saite>.<von><bis>.stereo.aif`: chromatische Läufe, also mehrere Töne je Datei.

**VCSL [S]**
- Baum vollständig (4550 Einträge, nicht abgeschnitten).
- Concert Harp: `KSHarp_<Ton>_{f1,mf1}.wav`, Tonumfang A2 … A6/G5 im Ausschnitt, 2 Stärken.
- Folk Harp: `EWHarp_Normal_<Ton>_v{2,3}_RR1.wav`.
- Harpsichord English: `ZuckermannKitHarpsi_{Lute,Normal}_{Sus,Rel}_<Ton>_rr1.wav`.
- Harpsichord Flemish: `HarpsiRH_{High,Low}[Rel]_Far_<Ton>_rr1.wav`.
- Velocity-Stufen bei Cembalo nicht erkennbar: nur ein Pegel, aber Register und Release getrennt.
- Dan Tranh: Ordner Vibrato (mf/ff) und FX.

**OrchideaSOL [S]**
- 13 265 Zeilen gesamt.
- Guitar 353: ordinario 210, sul_ponticello 44, pizzicato_bartok 41, ordinario_high_register 30, harmonic_fingering 27.
- Harp: 507 Zeilen.
- **Widerspruch in der Beschreibung:** Dort stehen „14 different instruments“ ohne Harfe und Gitarre, die CSV hat aber 16 Instrumente. Die Instrumentenliste stimmt wörtlich mit TinySOL überein.

**TinySOL [S]:** 2913 Zeilen, keine Gitarre, keine Harfe.

**AG-PT-set [S]:** 15 h 55 min, davon 10 h 4 min markiert, 32 592 Noten, 6 Spieler, 7 Stahlsaitengitarren.

---

## 4 Messdaten Korpus

| Fundort | Lizenz | Inhalt | Eignung |
|---|---|---|---|
| **Mores, Zenodo 4604577** (DOI-Konzept 4604576), „Archive for the acoustical documentation of classical Spanish guitars, flamenco guitars and romantic guitars … – bridge mobility“, 2021-03-15 | `license.id: cc-by-4.0` [S] | 342 Dateien, 1,10 GB: 330 JPG, 9 PDF, 2 `.m`, 1 `.mat`. **`qualified_selected_impulses.mat` (32,7 MB): „original, qualified raw data, organized as simple MATLAB matrix“.** 63 Plots `bridge_mobility_gNN.jpg` und Legende. Skripte `batch_extract_bridge_mobility_archive.m`, `create_fig_archive.m`. Messkette laut Datenblättern: Impulshammer Dytran 5800SL, Beschleunigungsaufnehmer Kistler 8778A500, Mikrofone Roga MI17, Frontend HEIM DIC6. Laut Text: 2 Kanäle am Steg („two main degrees of freedom“) und 3 Kanäle Nahfeld. | **Einzige gefundene Gitarren-Stegmobilität mit Rohdaten.** Nur klassische, Flamenco- und romantische Gitarren (Nylon). Messung vor Ort mit dokumentierten Wandabständen. |
| Mores, Zenodo 4010596 | `cc-by-4.0` [S] | MATLAB-Modelle zum Abstimmen asymmetrisch beleisteter Gitarren und PDF | Modellansatz für Resonanzen zwischen A0 und A1 |
| Mores, Zenodo 11383935 | `cc-by-4.0` [S] | Conde 1964: rückwärts erstellter Bauplan (PDF) und Fotos | Geometrie |
| Zenodo 10079910 „Bridge admittance measurements of a Moroccan rabāb“, 2023-11-07 | `cc-by-4.0` [S] | 6 Einzelmessungen und Mittelwert `Force_ACC_Bridge_*.via` (je 871 kB), Aufbaufotos, Maße. Gestrichene Laute mit Felldecke. | Verwandtes Instrument und Methodenvorlage; kein Ersatz für die Oud |
| Nemus-Project/harpsichord-interface-models (GitHub; Zenodo 16848274) | `spdx_id: GPL-3.0` [S] | Maße von Springer, Kiel und Dämpfer eines Cembalo-Interfaces | Zupfmechanik, **keine** Admittanz; ShareAlike |

---

## Nicht gefunden

- Echte Koto-Einzeltöne mit offener Lizenz, auch keine Koto-Forschungsdatensätze auf Zenodo, GitHub oder Hugging Face.
- Oud- oder Laute-Einzeltöne mit sauberer Lizenz.
- Trockene Einzeltöne einer Stahlsaitengitarre in mehreren Stärken unter offener Lizenz.
- Stegadmittanz oder Moden von **Stahlsaitengitarre, Harfe, Cembalo, Oud/Laute und Koto** als Datensatz.
- Freie Impulsantworten von Gitarrenkorpussen. Die GitHub-Suchen dazu sind am Rate-Limit gescheitert und nicht gelaufen.
- Nicht ausgewertet: Zenodo 19823590 „A Longitudinal Dataset for Guitar String Ageing“ (cc-by-4.0, ZIP 3,7 GB, 2026-05-01). Die Beschreibung ist leer, der Inhalt also unbekannt. Er könnte Einzelsaiten-Anschläge enthalten.

---

## Eigene Einschätzung (kein Beleg)

- **Nylon:** Iowa ist für die Gitarre das wertvollste Material, weil es schalltot und in drei Stärken aufgenommen ist. Zusammen mit dem Mores-Archiv ergeben sich Messung und Klang derselben Gattung (Nylon/klassisch), aber nicht desselben Instruments. Der Freibrief von Iowa ist keine Standardlizenz; vor einer Übernahme nach LumiViz sollte man sie schriftlich bestätigen lassen.
- **Stahl:** Für die Stahlsaitengitarre bleibt nur AG-PT-set (CC BY) als Klangreferenz. Die Korpusseite müsste man selbst messen oder aus der Literatur nehmen.
- **Cembalo und Harfe:** VCSL (CC0) deckt beide ohne rechtliche Hürde ab. Die Mikrofonierung ist raumbehaftet, also nicht trocken.
- **Koto und Oud:** Ohne eigene Aufnahmen oder Anfragen bei Forschungsgruppen ist kaum etwas zu machen. Der Basler Rabāb-Datensatz zeigt immerhin, dass solche Admittanzmessungen an Lauteninstrumenten offen veröffentlicht werden. Dort weiterzusuchen oder anzufragen, könnte für die Oud lohnen.
