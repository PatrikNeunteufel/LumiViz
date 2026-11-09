# Recherche IC1: Schlagwerk — freie Aufnahmen und Messdaten (Klassen 5 und 6)

> **Auftrag:** Klassen 5 (Membran) und 6 (Becken und Gong): freie Aufnahmen und Messdaten mit an der Quelle geprüfter
> Lizenz; die Lizenz der DrumGizmo-Kits, die LumiViz schon nutzt, an der Quelle nachlesen.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [S] = vom eigenen Skript gelesener Rohtext/Feldinhalt; [W] = vom Abrufwerkzeug geliefert; [Z] = nur
> Zusammenfassung; [A] = eigene Auszählung aus ZIP-Inhaltsverzeichnis oder Dateibaum.
> **Reichweite:** im Bericht selbst (rund 45 Aufrufe; „Tabla Solo dataset“ gesperrt).
> **Anmerkung der Sitzung zur Methode:** Der Agent hat keine Archive geladen, aber per HTTP-Range die
> Inhaltsverzeichnisse von ZIP-Archiven, einzelne kleine Textdateien darin und bei zwei Proben die ersten 4 KB einer
> WAV-Datei im Speicher gelesen (Skript `scratchpad\s\zipdir.py`). Das ist ein Teilabruf, kein Download in Dateien;
> LumiViz hat für DrumGizmo denselben Weg mit Freigabe genutzt (Entscheid D1 a). Im Gespräch an Patrik gemeldet.
> **Von der Sitzung selbst nachgelesen:** siehe `klassen/membran/aufnahmen.md`, `klassen/becken-gong/aufnahmen.md`.
> **Eingeflossen in:** die `aufnahmen.md`, `messplan.md`, `offene-fragen.md` der Klassen 5 und 6.

---

# Bericht: Freie Aufnahmen und Messdaten für Klasse 5 „Membran“ und Klasse 6 „Becken und Gong“

**Auftrag:** Freie Aufnahmen und Messdaten mit an der Quelle geprüfter Lizenz für Membran- und Becken-/Gong-Instrumente suchen, gegen die ein gerechnetes Modell gemessen werden kann.
**Datum:** 2026-10-26

**Reichweite:**
- Etwa 45 Werkzeugaufrufe:
  - 1 Websuche.
  - 17 Suchanfragen an die Zenodo-API.
  - 4 Suchen auf Freesound.
  - Etwa 30 gezielte Abrufe.
- Gelesen habe ich die Rohtexte und das DokuWiki-Markup der Seiten auf drumgizmo.org, die Verzeichnislisten unter drumgizmo.org/kits/, die Seiten von theremin.music.uiowa.edu, GitHub (API, LICENSE- und README-Dateien), philharmonia.co.uk, die Zenodo-API, magenta.tensorflow.org und freesound.org.
- **Methode bei ZIP-Archiven (bitte prüfen, ob das so in Ordnung ist):** Bei den DrumGizmo-Kits, beim Philharmonia-Percussion-ZIP und beim Tom-Tom-Datensatz habe ich keine Archive geladen. Ich habe per HTTP-Range nur das Inhaltsverzeichnis des ZIP und einzelne kleine Textdateien (README, XML, LICENSE) gelesen, dazu bei zwei Proben die ersten 4 KB der WAV-Datei für den Kopf. Das lief im Speicher.
  - Dafür liegt ein Hilfsskript im Scratchpad: `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\scratchpad\s\zipdir.py`
  - Im Projekt habe ich nichts geschrieben.
- **Nicht erreicht oder nicht geprüft:**
  - „Tabla Solo dataset“ auf Zenodo ist nur eingeschränkt zugänglich („restricted“).
  - Die Freesound-Packs habe ich nur über die erste Seite gesehen; die Gesamtzahl der Sounds je Pack habe ich nicht gelesen.
  - Die Kopplung an CompMusic-Seiten außerhalb von Zenodo habe ich nicht verfolgt.

**Kennzeichnung der Angaben:**
- **[S]** = wörtlich, selbst gelesen (Rohtext per curl oder API).
- **[W]** = wörtlich, über das Abrufwerkzeug.
- **[Z]** = nur Zusammenfassung, also kein Beleg.
- **[A]** = eigene Auszählung aus dem ZIP-Inhaltsverzeichnis oder dem Dateibaum.

---

## 1. DrumGizmo (drumgizmo.org)

**Lizenz** – Wortlaut aus dem Wiki-Rohtext `doku.php?id=kits:<kit>&do=export_raw`, abgerufen 2026-10-26 [S]:

- **DRSKit:** „The DRSKit drumkit source files are released under the … Creative Commons Attribution 4.0 International License“.
  - Zusatz: „When the samples are used in a composition, any context in which other credits (e.g. to performers) are presented must include an attribution.“
  - README.md im ZIP DRSKit2_1.zip [S]: „This kit is distributed under the Creative Commons Attribution 4.0 International license (https://creativecommons.org/licenses/by/4.0/).“
- **CrocellKit:** „The CrocellKit drumkit is released under the … Creative Commons Attribution 4.0 International License“, mit demselben Zusatz zur Namensnennung.
  - README.md im ZIP [S]: „This kit is distributed under the Creative Commons Attribution 4.0 International license“.
- **MuldjordKit:** „… released under the … Creative Commons Attribution 4.0 International License“.
  - Zusatz: „Attribution requirement as per January 2021: … ‚Drum samples provided by DrumGizmo.org'“.
- **The Aasimonster:** „… Creative Commons Attribution 4.0 International License“, mit dem Zusatz wie bei DRSKit.
- **ShittyKit:** „… released under the … Attribution 4.0 International license.“ Das Kit hat das alte Format mit festen Velocity-Gruppen.
- **Sommerhack 2016-Kit:** CC BY 4.0. Es enthält keine Trommeln und Becken, ist also nicht einschlägig.
- **Fußzeile des Wikis** [S]: „content on this wiki is licensed under … GNU Free Documentation License 1.3“. Das gilt für den Wiki-Text, nicht für die Kits.

**Damit ist die Angabe in den CREDITS von LumiViz (CC BY 4.0) für DRSKit und CrocellKit an der Quelle bestätigt.** Neu ist die Auflage, die Namensnennung in den Credits einer Komposition zu führen.

**Größen** laut Verzeichnisliste drumgizmo.org/kits/ [S]:

| Kit (Archiv) | Größe in Byte |
|---|---|
| DRSKit2_1.zip | 2 803 397 710 |
| CrocellKit1_1.zip | 5 646 502 341 |
| MuldjordKit3.zip | 2 536 576 789 |
| aasimonster2_1.zip | 2 416 291 882 |
| ShittyKit1_2.zip | 369 655 148 |
| snare-position.zip | 307 333 518 |

**Format** laut WAV-Kopf [S]:
- DRSKit: 13 Kanäle, 44 100 Hz, 32 bit.
- CrocellKit: 15 Kanäle, 48 000 Hz, 32 bit.
- Je Anschlag gibt es eine mehrkanalige WAV-Datei, in der alle Mikrofone gleichzeitig liegen.

**Mikrofone** (Wiki [S]):
- **DRSKit, 13 Mikrofone:**
  - Nah: Snare oben und unten, 3 Toms (je SM57), Kick hinten (D112) und vorn (SM57), Ride und Hi-Hat (Beyerdynamic MCE 86 II).
  - 2 Overheads (Røde NT5).
  - 2 Raummikrofone (t.bone RB500).
- **CrocellKit, 15 Kanäle:** Amb L/R, OH L/R/Center, Hi-Hat, Ride, Snare oben und unten, 4 Toms, Kick innen und außen.
- **MuldjordKit, 16 Kanäle:** unter anderem 2 Raummikrofone, 2 Overheads, Nahmikrofone und eine Trigger-Spur.
- **Aasimonster:** 14 Mikrofone, dazu Trigger.

**Anschläge je Instrument** [A, aus dem Inhaltsverzeichnis des ZIP; je WAV ein Anschlag]:

- **DRSKit 2.1:**
  - Kick: mit Kontakt 28, ohne Kontakt 29.
  - Snare 29, Snare_rim 17, Snare_rest 13, Snare_whisker (Besen) 18, Snare_circle_whisker 4.
  - Toms: Tom1 27, Tom2 30, Tom3 30, dazu je 11–14 Besenschläge.
  - Hi-Hat: closed 24, closed_shank 16, semi_open 18, open 18, open_tip 16, foot 6, mit Besen 12 bzw. 19.
  - Ride: tip 15, shank 10, tip_bell 11, shank_bell 10, tip_chain 11, tip_bell_chain 10, whisker 11, rest 3.
  - Crash L/R: tip, shank und whisker mit je 10–13.
  - Die Snare-XML [S] nennt 29 Proben mit `power` von 0.037478 bis 37.8792. Die Stärken sind also fein und monoton gestaffelt.
  - Laut Reaper-Notendatei [S] gibt es außerdem `Kick_With_Contact` / `Kick_Without_Contact`, `Snare_Rest` (abgestoppt), `Ride_Rest` und `Hihat_Foot`.
- **CrocellKit 1.1:**
  - Snare 98, SnareRimShot 13, SnareRim 18, SnareRest 11.
  - Toms: Tom1 45, Tom2 39, FTom1 29, FTom2 34.
  - Kick: KDrumL 51, KDrumR 49.
  - Hi-Hat: Closed 41, ClosedNoPedal 42, SemiOpen 36, Open 47, Pedal 10, PedalHit 5.
  - Ride: RideR 37, RideRBell 18.
  - Crash: CrashL 25, CrashR 21, CrashLStopped 12, CrashRStopped 9 (Choke), CrashRXtra 8.
  - China L/R 16/25, Splash L/R 11/8.
  - Snare-`power` von 0.00986 bis 3.29 [S].
- **MuldjordKit 3:** Snare 56, SnareRest 13, Toms 11/13/15/20, Kick 25/26, Hi-Hat closed 29 und open 30, Ride L/R je 10 (Kuppe 3 bzw. 8), Crash 9/12, China 12.
- **Aasimonster 2.1:**
  - snare_on_center 93, snare_off_center 23, snare_rim_shot 33.
  - Toms 22–24.
  - Kick 30/24.
  - Hi-Hat closed1/2/3 je 20–21, open 23.
  - Ride 19, ride_bell 15 und 16.
  - Crash 19 und 19, crash_stop je 3.
  - China 8", 10" und 18" mit 19–21.
  - Zilbel 3.
- **ShittyKit:** 139 Anschläge insgesamt, nicht je Instrument aufgeteilt.
- **snare-position.zip:** im Wiki nicht verlinkt, aber im Verzeichnis vorhanden.
  - 123 Snare-Anschläge.
  - Kommentar in der XML [S]: „Positions 2 - 6, where 2 is center (position 0) and 6 is (near)rim (position 1)“, mit `position` 0 … 1 und `power` je Probe.
  - Das wäre die einzige gefundene Reihe mit **Schlagstelle von der Mitte zum Rand mal Stärke**.
  - **Eine Lizenz dafür habe ich nirgends gefunden.**

---

## 2. University of Iowa MIS (theremin.music.uiowa.edu)

**Lizenz** – MIS.html [S], 2026-10-26: „Since 1997, these recordings have been freely available on this website and may be downloaded and used for any projects, without restrictions.“ Ein förmlicher Lizenzname wird nicht genannt.

**Aufnahme** [S]:
- Bis 2011 mit einem Neumann KM 84 bei 16 bit / 44,1 kHz, in den Stufen pp, mf und ff.
- Ab 2011 mit drei Earthworks QTC-40 als Decca Tree, 24 bit / 96 kHz, „5 feet in front of the performer“.
- Mit Ausnahme des Klaviers alles „recorded in an anechoic chamber“, mit Absorption „down to 60 Hz“.

**Schlaginstrumente** (Kategorien [S]): Marimba, Xylophone, Vibraphone, Bells, Crotales, Cymbals, Gongs/Tamtams, Hand Percussion, Tambourines.
- **Keine Pauken und keine Trommeln.**
- Hand Percussion [S, Dateien]: Woodblocks, Granitblöcke, Claves, Guiro, Kastagnetten, Triangeln.

**Becken** (Dateinamen [S]):
- Crash 13", 17", 18" und 20", China 16", 19" und 20", Ride 21", Splash.
  - Jeweils `stick.bell` / `stick.normal` / `stick.shoulder` in pp, mf und ff.
  - Dazu `chokecrash`, `stick.roll`, `mallet`, `mallet.roll` und `bow`.
- Orchesterbecken 15–18" (bandaturka, french, germanic, viennese, medheavy).
- Hi-Hat: `hihat.bell`, `.normal`, `.shoulder`, `.footclose`, `.footsplash`.
- Das sind Kuppe, Fläche und Rand mal drei Stärken, schalltot aufgenommen.

**Gongs** [S]:
- `20windgong`, `22tamtam`, `28tamtam`, `40tamtam`, jeweils pp, mf, ff und bow.
- Thai-Gongs chromatisch C4–C5 in pp, mf und ff.
- Ab 2012 in Stereo (ff, bow).

**Format:** AIFF.

---

## 3. VSCO 2 Community Edition (GitHub sgossner/VSCO-2-CE)

**Lizenz:**
- GitHub-API `license.spdx_id` = „CC0-1.0“.
- Die LICENSE-Datei beginnt mit „CC0 1.0 Universal“ [S], 2026-10-26.

**Percussion** [A, Dateibaum]:
- **Pauke:** `Percussion/Timpani`, 28 Dateien.
  - 5 Kessel (Timpani1–5), Stärken v1/v3/v4, je 2 Round-Robins.
  - Dazu 10 Wirbel (v3/v5).
- **Große Trommel:** `VSCO 1 Percussion/drums/bass`, 39 Dateien.
  - ppp bis fff, dazu `bdrum_muted_*` (ppp–fff).
- **Snare:** drum1 40, drum2 36, drum3_marching 17, OldSnare 15.
  - Stärken pp–ff, dazu `snare_rim` und `click`.
- **Tenortrommel:** 41 Dateien, pp–fff.
- **Becken:** `varMetal/Cymbals`.
  - susp 29 (`hardmall`, `softmall` und `metal`, mf–fff, dazu scrape).
  - clash 8 (`crash_hit_*_loose/tight`).
- **Gong:** `varMetal/Gong`, 30 Dateien.
  - `gong_hit_pppp` … `fff` (17 Anschläge in Stärken), Wirbel, scrape und smack.
- **Tamtam:** nicht als eigener Ordner vorhanden.
- Die Pauken liegen zusätzlich auf Freesound (Packs 21042/21043 von sgossner, „VSCO 2 CE - Percussion - Timpani“ [S]).

---

## 4. VCSL (GitHub sgossner/VCSL)

**Lizenz:**
- GitHub-API „CC0-1.0“.
- README [S]: „This collection is under a Creative Commons 0 license. Essentially it's Public Domain“.
- Zur Aufnahme sagt die README [S] nur als Empfehlung für Beiträge „‚mid-close' position, typically 0.5-3 meters“ und „Most of the sample set is recorded with spaced pair or NOS“. Konkrete Angaben zu Mikrofonen und Räumen je Instrument habe ich nicht gefunden.

**Membranophone** [A, Dateinamen]:
- **Timpani 2:** 204 Anschläge, Dateien wie `Timpani1A_hit_v2/v3/v5_rr1/rr2_main`.
  - Das sind mehrere Kessel und Töne, 3 Stärken und 2 Round-Robins.
  - **Glissando nicht gefunden.**
- **Timpani 1:** 40 Dateien.
  - 5 Kessel, v2–v4, 2 Round-Robins, dazu Wirbel.
- **Snare Drum, Modern 1:** 29 Dateien.
  - `HitSN` (mit Teppich) v3–v9 und `HitNS` (ohne Teppich) v2–v6, je 2 Round-Robins.
  - Dazu Wirbel, `stick` (vermutlich Stock auf Stock oder Cross-Stick, ungeklärt) und `taps`.
- **Snare Drum, Rope Tension:** 32 Dateien.
  - sn, ns und tsn in vl1–vl4, `sidestick` (Cross-Stick) und `stick`.
- **Snare Drum, Modern 2 und 3:** 24 bzw. 19 Dateien.
- **Tom 1 und Tom 2:** je 22 Dateien.
  - `HitM` und `HitS` v2–v4 je 2 Round-Robins.
  - Dazu `RollM`/`RollS`, `rimS` und `rimFLS` (Randschläge).
- **Bass Drum 2:** 30 Dateien.
  - `hit_pp1` … `hit_ff`, mehrere Wirbelarten, `cresc` und `rub`.
- **Bongos:** 28 Dateien.
  - H und L, jeweils `Hit1` und `HitMuted` (gedämpft) v1–v3 mit 2 Round-Robins, dazu Wirbel.
- **Conga:** 34 Dateien, Instrumente Conga, Quinto und Tumba.
  - `HitN` v1–v4 und `HitFM` v1–v4, je 2 Round-Robins.
  - Die Bedeutung von N und FM ist in den Dateien nicht erklärt. Vermutlich steht N für open und FM für muted. **Slap ist nicht erkennbar.**
- **Weitere Membranophone:** Frame Drum (Hit, HitMuted, Hand), Darbuka 20, Ocean Drum, Legacy Snares 108, Legacy Toms 41.

**Idiophone der Klasse 6** [A]:
- **Suspended Cymbal 1:** 25 Dateien.
  - `hit_pp1` … `fff1`, `hit_bell_pp`/`mf`/`fff`, `hit_stick_pp`/`mp`/`f`.
  - Bogen, cresc-Wirbel und scrape.
- **Suspended Cymbal 2:** 23 Dateien.
- **Hi-Hat Cymbal:** 15 Dateien.
  - `HitC` v1–v4, `HitO`, `HitOC`, `HitLoose` und `Close` (Fuß).
- **Clash Cymbals 1 und 2.**
- **Gong 1:** 7 Dateien, p bis fff und scrape.
- **Gong 2:** 6 Dateien, **im MP3-Format**.
- **Kein Tamtam.**

---

## 5. Philharmonia Orchestra Sound Samples

**Lizenz** – philharmonia.co.uk/resources/sound-samples/ [S], 2026-10-26: „You are free to use these samples as you wish, including releasing them as part of a commercial work. The only restriction is that they must not be sold or made available ‘as is’ (i.e. as samples or as a sampler instrument).“

**Inhalt von Percussion.zip** [A]:
- 7,15 MB, **nur MP3**.
- bass drum 18, snare drum 12, tom-toms 2, tenor drum 2, suspended cymbal 4, clash cymbals 7, chinese cymbal 2, sizzle cymbal 1, tam-tam 4 (scraped, damped, undamped, „rimshot“), thai gong 3, djembe 3, surdo 4, djundjun 4.
- **Keine Pauken im Percussion-Paket.** Ob sie in all-samples.zip stecken, habe ich nicht geprüft.

---

## 6. Tabla, Mridangam und Beijing-Oper (Zenodo-API, `/api/records/<id>`, 2026-10-26)

**Zur Kennzeichnung:** Das Lizenzfeld der Zenodo-API steht hier im Wortlaut [S], ist aber nur eine Kennung.

- **Tabla strokes dataset** (Zenodo 4327350, Subodh Deolekar, 2020):
  - `"license": {"id": "cc-by-4.0"}`.
  - Beschreibung: „Tabla set is tuned to C# scale.“
  - [A] 650 WAV-Dateien, **13 Bols je 50**: Da, Dh, Di, Ge, Ka, N, Na, T, Ta, Ti, Tin, Tr, Tu.
  - Etwa 18–88 KB je Datei, also kurze Einzelschläge.
  - Mikrofon, Raum und Stärken sind nicht angegeben.
- **Mridangam Stroke Dataset** (Zenodo 4068196):
  - `"cc-by-nc-4.0"`.
  - 6977 Schläge, 10 Bols, 6 Grundtöne.
  - „semi-anechoic studio conditions … SM-58 … 44.1 kHz … 16 bit“ [S].
- **Tabla Solo dataset** (Zenodo 1267024):
  - Lizenzfeld leer, `access_right` „restricted“ [S].
- **Beijing Opera Percussion Instrument Dataset** (Zenodo 1285212, 2014):
  - `"cc-by-4.0"`.
  - Einzelschläge von Bangu, Naobo (Becken), Daluo (großer Gong) und Xiaoluo (kleiner Gong).
  - „AKG C414 … 44.1 kHz … 16 bit“ [S].
  - 33 MB.
- **Freesound:**
  - **„Tabla bols“** von mmiron (Pack 8162): alle 12 sichtbaren Sounds tragen „License: Creative Commons 0“. Die Sound-Seite verlinkt creativecommons.org/publicdomain/zero/1.0/ [S].
  - **„Tabla Strokes“** von ajaysm (Pack 10737, Beschreibung „A sound pack of Tabla strokes“, Bezug zu CompMusic):
    - Alle 12 sichtbaren Sounds tragen „License: Attribution“.
    - Die Sound-Seite verlinkt creativecommons.org/licenses/by/4.0/ [S].
    - 12 Bols: te, tit, tin, tak, tun, ne, re, kat, na, dhin, dhun, ga [W].
    - Die Gesamtzahl der Sounds im Pack habe ich nicht gelesen.

---

## 7. Datensätze zur Schlagzeug-Transkription

| Datensatz | Lizenz an der Quelle [S], 2026-10-26 | Anmerkung |
|---|---|---|
| IDMT-SMT-Drums (Zenodo 7544164) | `"cc-by-nc-nd-4.0"` | Nur Kick, Snare und Hi-Hat in Loops; Teile stammen aus Sample-Bibliotheken und Synthesizern; 287 MB. |
| ENST-Drums (Zenodo 21506051, v2 2026) | `"cc-by-nc-nd-4.0"`; Beschreibung: „The use and exploitation of the database should be limited to research purposes. No commercial use is possible.“ | 3 Schlagzeuger, 8 Kanäle, Stöcke, Rods, Besen und Schlägel; 9,6 GB. |
| MDB Drums (GitHub CarlSouthall/MDBDrums) | README: „published under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License“ | 23 Stücke aus MedleyDB, 21 Unterklassen der Spielweise. |
| E-GMD und GMD (Magenta) | E-GMD: „made available by Google LLC under a Creative Commons Attribution 4.0 International (CC BY 4.0) License“; GMD: „Creative Commons Attribution 4.0 International (CC BY 4.0) License“ | Audio stammt von Roland TD-17 bzw. TD-11, ist also **synthetisch** und für die Messung physikalischer Instrumente unbrauchbar. |
| StemGMD (Zenodo 7882857) | `"cc-by-4.0"` | Gerendert aus Logic-Pro-X-Bibliotheken („single hits … ten different velocities“); die Herkunftsrechte sind fraglich. |
| Drum and Percussion Kits (Zenodo 3994999) | `"cc-by-4.0"` | Material aus dem „sample swap project“, Herkunft unklar. |
| Freesound One-Shot Percussive Sounds (Zenodo 4687854) | `"other-at"`; Lizenzen je Sound in licenses.txt | Auf 16 kHz heruntergerechnet und normalisiert, nicht brauchbar. |
| OrchideaSOL (Zenodo 3740399) | Metadaten CC BY 4.0, Audio unter der „Ircam Forum License“ [S] | **Keine Schlaginstrumente**: 14 Instrumente, Blech, Holz, Streicher und Akkordeon [S]. |

---

## 8. Messnahe Datensätze (Fund dieser Suche)

- **Drum Tom-Tom Timbre Dataset** (Zenodo 22783358, C. E. Leal de Castro, 2026-09-16):
  - `"cc-by-4.0"`.
  - LICENSE-DATA.txt im ZIP [S]: „Creative Commons Attribution 4.0 International (CC BY 4.0)“. Code steht unter MIT.
  - **144 Einzelschläge** an 12"×10"-Toms von Odery: 9 Bauweisen (3 Hölzer, Wandstärke, Verstärkungsring, Fell, Spannreifen) mal 16 Stimmungen.
  - Die Stimmungen sind je Spannschraube mit dem Tama Tension Watch eingestellt: Schlagfell und Resonanzfell je 60, 70, 80 und 85.
  - Grundtöne laut Methodenblatt „about 112–246 Hz“ [S].
  - **Pendel-Anschlagapparat** mit einem Paukenschlägel mit Filzkopf. Die Kraft ist nicht gemessen („No force, velocity or contact sensor“ [S]).
  - Nahmikrofon Behringer ECM8000 in etwa 5 cm Abstand, in einem isolierten Raum der USP.
  - Format 24 bit / 44,1 kHz; 90 MB.
- **Cymbal percussion: multimodal motion capture, audio and video** (Zenodo 21710617, Universität Oslo, fourMs 2011):
  - `"cc-by-4.0"`; README: „**License:** CC BY 4.0“ [S].
  - 72 WAV-Dateien (44,1 kHz mono) mit Flams, Wirbeln und Staccato, Hängebecken mit Mitte, `midt` und `kant` (Rand), soft/medium/strong, gedämpft.
  - Dazu synchrone Bewegungsdaten von Qualisys und Xsens.
- **Dataset for drums source separation using microphone arrays** (Zenodo 17706651, Kitamura, 2025):
  - `"cc-by-4.0"`.
  - Bass Drum, Snare, Hi-Hat und Crash mit je einem Nahmikrofon und drei Mikrofon-Arrays.
  - Es sind Spielsequenzen, keine Einzelschläge; 2,8 GB.
- **Messdaten von Modenfrequenzen oder Dämpfungen als Datensatz:** nicht gefunden (siehe unten).

---

## Tabelle je Instrument

| Instrument | Bester Datensatz | Lizenz | Eignung |
|---|---|---|---|
| Pauke | VCSL Timpani 2 (204 Anschläge, 3 Stärken, 2 Round-Robins) und Timpani 1; VSCO 2 CE Timpani (5 Kessel, 3 Stärken, Wirbel) | CC0 | Gut für Einzelschläge; wenige Stärken, Raumanteil unbekannt; **kein Glissando** |
| Toms | Tom-Tom Timbre Dataset (kontrollierter Anschlag, 16 Stimmungen); DrumGizmo CrocellKit (29–45 Anschläge) und DRSKit (27–30) | CC BY 4.0 | Sehr gut: Tom-Tom-Datensatz für Stimmung und Bauweise, DrumGizmo für viele Stärken mit Nahmikrofon |
| Bassdrum (Kit) | DrumGizmo CrocellKit (51 und 49), DRSKit (28 und 29, mit und ohne Kontakt) | CC BY 4.0 | Gut, Mikrofon innen und außen |
| Große Trommel (Orchester) | VSCO 1 bass (ppp–fff, gedämpft), VCSL Bass Drum 2 | CC0 | Mittel: wenige Stufen |
| Snare | DrumGizmo CrocellKit (98 Anschläge, Rimshot 13, Rim 18); Aasimonster (Mitte 93, außermittig 23, Rimshot 33); DRSKit (29 mit feiner `power`-Staffel, Besen) | CC BY 4.0 | Sehr gut; **Schlagstelle Mitte bis Rand nur in snare-position.zip, Lizenz ungeklärt** |
| Conga | VCSL Conga (Conga, Quinto, Tumba; N und FM, v1–v4) | CC0 | Mittel: Slap fehlt, Bedeutung der Kürzel nicht dokumentiert |
| Bongo | VCSL Bongos (offen und gedämpft, v1–v3) | CC0 | Mittel |
| Tabla | Zenodo Tabla strokes (13 Bols je 50) | CC BY 4.0 | Gut für Bols; Stärken und Mikrofon unbekannt |
| Crash | Iowa MIS (4 Größen; Kuppe, Fläche, Rand, Choke, Bogen, Schlägel; pp/mf/ff; schalltot) | „without restrictions“ | **Sehr gut**: trocken, systematisch |
| Ride | Iowa MIS 21" ride (Kuppe, Fläche, Rand; pp/mf/ff); DrumGizmo DRSKit (tip, shank, bell, chain; 10–15 je Spielweise) | Iowa frei bzw. CC BY 4.0 | Sehr gut |
| Hi-Hat | DrumGizmo CrocellKit (closed 41, semi 36, open 47, pedal 10, pedal hit 5), DRSKit (closed, semi, open, foot); Iowa (bell, normal, shoulder, footclose, footsplash) | CC BY 4.0 bzw. frei | Sehr gut |
| Gong | Iowa (Windgong 20", Thai-Gongs C4–C5, pp/mf/ff); VSCO 1 Gong (pppp–fff); Beijing Opera Daluo und Xiaoluo | frei / CC0 / CC BY 4.0 | Gut |
| Tamtam | Iowa 22", 28" und 40" (pp/mf/ff, Bogen; schalltot) | „without restrictions“ | **Sehr gut** |

---

## Tabelle aller Datensätze

Alle Lizenzangaben sind am 2026-10-26 an der jeweiligen Quelle gelesen.

| Datensatz | Fundort | Lizenz (Wortlaut / Ort) | Inhalt | Größe | Eignung | Übernahme nach LumiViz |
|---|---|---|---|---|---|---|
| DRSKit 2.1 | drumgizmo.org/kits/DRSKit | „Creative Commons Attribution 4.0 International License“ (Wiki) und dieselbe Lizenz in README.md im ZIP | Komplettes Kit; 13 Kanäle Nah, OH und Raum; 44,1 kHz | 2,80 GB | Hoch | Namensnennung, auch in Kompositions-Credits |
| CrocellKit 1.1 | drumgizmo.org/kits/CrocellKit | wie oben (Wiki und README) | Metal-Kit; 15 Kanäle; 48 kHz; Snare 98 Anschläge | 5,65 GB | Hoch | Namensnennung |
| MuldjordKit 3 | drumgizmo.org | CC BY 4.0, Zusatz „Drum samples provided by DrumGizmo.org“ | Tama Superstar; 16 Kanäle | 2,54 GB | Mittel bis hoch | Namensnennung |
| Aasimonster 2.1 | drumgizmo.org | CC BY 4.0 (Wiki) | Snare Mitte und außermittig, Rimshot | 2,42 GB | Hoch für die Snare | Namensnennung |
| ShittyKit 1.2 | drumgizmo.org | „Attribution 4.0 International“ | 5 Instrumente, altes Format | 0,37 GB | Gering | Namensnennung |
| snare-position | drumgizmo.org/kits/snare-position.zip | **keine gefunden** | 123 Snare-Anschläge, 5 Positionen Mitte bis Rand | 0,31 GB | Sehr hoch (Schlagstelle) | **nicht**, bis die Lizenz geklärt ist |
| Iowa MIS | theremin.music.uiowa.edu | „may be downloaded and used for any projects, without restrictions“ | Becken, Hi-Hat, Tamtam, Gong; schalltot; pp/mf/ff | einzelne AIFF-Dateien | Sehr hoch (Klasse 6) | frei (kein förmlicher Lizenzname) |
| VSCO 2 CE | github.com/sgossner/VSCO-2-CE | „CC0-1.0“ (API), LICENSE „CC0 1.0 Universal“ | Pauken, große Trommel, Snare, Tenortrommel, Hängebecken, Gong | Percussion etwa 0,7 GB [A] | Mittel | frei |
| VCSL | github.com/sgossner/VCSL | „CC0-1.0“; README „Creative Commons 0 license“ | Pauken, Snare (4 Arten), Toms, große Trommel, Conga, Bongo, Becken, Hi-Hat, Gong | Pauken allein etwa 0,37 GB [A] | Mittel bis hoch | frei |
| Philharmonia | philharmonia.co.uk | „must not be sold or made available ‘as is’“ | MP3, wenige Schläge | 7 MB | Gering | **nicht** als Rohdaten |
| Tabla strokes | Zenodo 4327350 | `cc-by-4.0` | 13 Bols je 50 | 42 MB | Gut | Namensnennung |
| Freesound „tabla bols“ (mmiron) | freesound Pack 8162 | „License: Creative Commons 0“ | Tabla-Bols | ? | Unbekannt | frei, je Sound zu prüfen |
| Freesound „Tabla Strokes“ (ajaysm) | freesound Pack 10737 | „License: Attribution“, verlinkt CC BY 4.0 | 12 Bols | ? | Unbekannt | Namensnennung |
| Mridangam Stroke | Zenodo 4068196 | `cc-by-nc-4.0` | 6977 Schläge | 130 MB | Gut, aber nicht Klasse 5 | nicht (NC) |
| Tabla Solo | Zenodo 1267024 | leer, „restricted“ | — | — | — | nicht |
| Beijing Opera Percussion | Zenodo 1285212 | `cc-by-4.0` | Gong groß und klein, Becken, Trommel | 33 MB | Mittel | Namensnennung |
| Tom-Tom Timbre | Zenodo 22783358 | `cc-by-4.0`; LICENSE-DATA „CC BY 4.0“ | 144 kontrollierte Tom-Schläge, 16 Stimmungen | 90 MB | **Sehr hoch** | Namensnennung |
| Cymbal mocap Oslo | Zenodo 21710617 | `cc-by-4.0`; README „CC BY 4.0“ | 72 Becken-Takes mit Bewegungsdaten | 0,8 GB (WAV 38 MB) | Mittel | Namensnennung |
| Drum-Array (Kitamura) | Zenodo 17706651 | `cc-by-4.0` | BD, SD, HH, CC; Arrays und Nahmikrofone | 2,8 GB | Gering (Sequenzen) | Namensnennung |
| IDMT-SMT-Drums | Zenodo 7544164 | `cc-by-nc-nd-4.0` | Loops | 287 MB | Gering | nicht |
| ENST-Drums | Zenodo 21506051 | `cc-by-nc-nd-4.0` | 3 Kits, 8 Kanäle | 9,6 GB | Mittel | nicht |
| MDB Drums | GitHub CarlSouthall/MDBDrums | CC BY-NC-SA 4.0 (README) | 23 Stücke | ? | Gering | nicht |
| E-GMD / GMD | magenta.tensorflow.org | CC BY 4.0 | E-Drum, synthetisch | 444 h | keine | (Namensnennung) – irrelevant |
| StemGMD | Zenodo 7882857 | `cc-by-4.0` | aus Logic Pro X gerendert | 30 GB | keine | fraglich, nicht |

---

## Nicht gefunden

- **Offene Messdatensätze mit Modenfrequenzen und Dämpfungen** von Trommelfellen, Pauken, Becken oder Tamtams. Gesucht habe ich in der Zenodo-API (timpani, cymbal, snare drum, drumhead, gong, drum modal frequencies, cymbal vibration) und mit einer Websuche.
  - Die Websuche [Z] nennt Arbeiten der Hellenic Mediterranean University mit Impulshammer und Beschleunigungsaufnehmer, aber ohne veröffentlichte Daten.
  - Ein Datensatz mit 11 114 FDTD-**simulierten** Membranklängen (Acoustics 2023) ist ebenfalls keine Messung.
- **Pauken-Glissando** in VCSL, VSCO 2 CE, Iowa und Philharmonia.
- **Conga-Slap** als eindeutig benannte Spielweise.
- **Cross-Stick** eindeutig: nur `RopeSnare_sidestick` in VCSL. Ob `Snare_rim` bei DrumGizmo Cross-Stick oder Randklick bedeutet, ist ungeklärt.
- **Pauken und Trommeln bei Iowa MIS:** gibt es nicht.
- **Lizenz für snare-position.zip.**
- **Mikrofone und Raum je Instrument bei VCSL** und Angaben zur Aufnahme beim Tabla-strokes-Datensatz.
- **Percussion bei OrchideaSOL/Studio Online:** keine. TinySOL nicht einzeln geprüft; laut OrchideaSOL-Beschreibung gibt es dort keine Schlaginstrumente.

Kein Treffer ist kein Beweis.

## Eigene Einschätzung (kein Beleg)

1. **Klasse 6** ist mit Iowa MIS sehr gut abgedeckt: trocken, systematisch und mit drei Stärken. Für Hi-Hat-Zustände und Ride-Spielweisen kommen die DrumGizmo-Kits dazu, die LumiViz schon nutzt, mit Nahmikrofonen und 10–47 Stärken. Iowa trägt keinen förmlichen Lizenznamen. Der Wortlaut „without restrictions“ sollte in `quellen.md` mit Datum festgehalten werden.
2. **Klasse 5:** Für Toms ist der Tom-Tom-Timbre-Datensatz die beste Messreferenz, weil Anschlag und Stimmung kontrolliert sind. Für Snare und Kick mit vielen Stärken taugen CrocellKit und DRSKit, weil die Nahmikrofone trocken genug sind und `power` je Probe in der XML steht. Die Pauken kommen am besten aus VCSL und VSCO (CC0), haben aber nur 3 Stärken.
3. **Lizenzklärung lohnt sich bei snare-position.zip** (Schlagstelle Mitte bis Rand), etwa durch eine Anfrage an das DrumGizmo-Team. Der Ordner liegt neben den CC-BY-Kits, die Lizenz ist aber nicht belegt.
4. **Nicht für LumiViz verwendbar:** Alles mit NC oder ND (IDMT, ENST, MDB, Mridangam) sowie Philharmonia als Rohdaten. Synthetische Sätze (E-GMD, StemGMD) taugen nicht als Messreferenz.
5. **Modendaten:** Hier fehlt ein offener Datensatz. Modenfrequenzen müssten wir vermutlich selbst aus den trockenen Einzelschlägen (Iowa, Tom-Tom-Datensatz, DrumGizmo-Nahkanäle) schätzen oder aus der Literatur zitieren.

Quellen der Websuche: [mta.hmu.gr](https://mta.hmu.gr/en/research/in-brief-research/research-demos/vibroacoustic-analysis-percussion/), [doaj.org Acoustics 2023](https://doaj.org/article/36f13745a95442da9b9d15f34562f91a)
