# Bericht — Vorab-Recherche und Bau der Rechenkerne (IC Sessions 1–4)

> **Stand:** 2026-10-30, Ende IC Session 4 (Cloud), Zweig `claude/youthful-ritchie-0iorxi`, PR
> [#1](https://github.com/PatrikNeunteufel/InstrumentsCalculation-/pull/1). **Zweck:** die Zusammenfassung vor dem Merge
> nach master, als Grundlage für die lokale Planung des weiteren Vorgehens. **Belegkraft:** jede Zahl hier steht mit
> Quelle in der genannten Datei; Einschätzungen sind als solche gekennzeichnet. Einzelheiten: `STAND.md` (knapp),
> `.claude/journal/2026-10-29.md` und `2026-10-30.md` (Verlauf, Irrwege), je Klasse `klassen/<Klasse>/offen.md` und
> `modell.md`.

---

## 1. Kurzantwort: Ist alles gemacht?

**Ja, für alles, was ohne Aufnahmen und ohne LumiViz geht:**

- **Vorab-Recherche** aller Klassen 0–11 (IC Sessions 1–3): je Klasse die sieben Lieferungen aus `AUFGABE.md`
  (Literatur, Modellentwurf, Aufnahmen, Messplan, Quellen, offene Fragen, README).
- **Fragen:** alle entschieden (T1–T9, E1, E2, Z-1, F-B1–F-B6; `FRAGEN_KONSOLIDIERT.md`). **Es gibt keine offene Frage
  an Patrik.**
- **Grundlagen gebaut** (IC Session 4, nach `AUFTRAG_BAU.md`) für **alle Klassen 1–11**:
  - Rechenkern je Klasse, Instrumente mit Vorgaben aus Quellen (je Wert Herkunft und Belegkraft).
  - Tests gegen Analytik und Literatur; Rechenzeit; Abdrücke für gcc und clang, die bitgleich sind.
  - Hörproben-Werkzeug, Messkette B1–B8, Setz-Werkzeug nach den Lehren L8–L18.
- **Tests:** 114 Testfälle, 1363 Prüfungen, grün unter gcc 13.3 und clang 18.1.

**Nein, und zwar mit Absicht, bei allem, was lokal ist** (`AUFTRAG_BAU.md` §0, §10):

- **Vergleiche gegen Aufnahmen:** Setzen, Toleranzen und Hörurteile fehlen. Kein Instrument ist **gemessen**.
- **Hörproben:** rund 450 WAVs, keine davon gehört (L5).
- **Integration in LumiViz:** wird nach IK1 entschieden. Für diese Entscheidung ist der Bericht da.

**Nicht gebaut, weil Daten fehlen:**

| Klasse | Instrument | Grund |
|---|---|---|
| 7 | Tuba, Horn | keine Bohrung |
| 7 | Didgeridoo | Bohrung da, aber kein Bordun mit der Posaunenlippe |
| 8 | Oboe | Bohrung da, mit dem Klarinettenblatt nur 7/36 |
| 8 | Fagott | keine Bohrung |
| 9 | Querflöte, Altflöte | Strahl und Mund fehlen |
| 2 | Koto | zurückgestellt bis zu einer freien Aufnahme |

---

## 2. Was gebaut ist

Code unter `rechnung/` (C++20, CMake, kein Qt; Externals doctest MIT, pocketfft BSD-3). Bau und Test:
`cmake -S rechnung -B rechnung/out && cmake --build rechnung/out && ctest --test-dir rechnung/out`.

| Klasse | Kern | spielbar (alle ungemessen) | Prüfsteine | Rechenzeit* |
|---|---|---|---|---|
| 1 Hammer-Saite | Flügel aus LumiViz portiert (Prüfstein), Saitenkern | Upright, Honky-tonk, CP-70, Clavinet; Hackbrett, Cimbalom (Deluxe) mit Mitschwingen je Clip | Flügel gegen Bezug < 5·10⁻⁷ dB (Saite), ≤ 0,004 dB (Chor) | ≤ 4,6 %; Hackbrett 4,6–5,7 %, Cimbalom 7,7–9,1 % |
| 2 Gezupfte Saite | Waveguide, zwei Polarisationen, 2×2-Admittanz am Steg | Cembalo, Harfe (mit Pedalen), Gitarre (Deluxe), Laute, Oud; Mitschwingen je Clip | zweistufiges Abklingen, Zupfpunkt nach Välimäki | 0,3–0,5 %; Harfe-Clip mit 47 Saiten 26 % |
| 3 Bogen-Saite | Waveguide mit Reibstelle, Korpus am Steg | Cello 48/48, Violine 42/42, Viola 40/42, Kontrabass 42/42, Ensemble | Wolfton entsteht von selbst; Einschwingen 25–55 ms (Guettler) | 1,9 % |
| 4 Platte und Stab | FE-Balken, Resonator, Tonabnehmer | Marimba, Xylophon, Vibraphon (auch gestrichen), Glockenspiel, Röhrenglocken, Celesta, Kalimba, Rhodes, Wurlitzer, Steeldrum (Deluxe) | Rucz (Xylophon F4 T60 0,71 s gegen 0,7 s); Rhodes 440 Hz auf 0,5 % | 0,2–3,6 % |
| 5 Membran | Fellmoden mit Luftlast (Rayleigh) und Kessel | Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo | Pauke gegen Packer auf 3 %; Tabla harmonisch | 3,1–3,4 % |
| 6 Becken und Gong | von Kármán modal, energiestabil (SAV) | Tamtam, Gong, Crash, China, Splash, Ride, Hi-Hat — als **flache Platte** | Energie auf 10⁻¹³ erhalten; Aufblühen bei ff | **vorab**, 1–7 × Echtzeit |
| 7 Lippe und Rohr | Rohrschleife mit Zug, Schmettern, Ventilen, Dämpfern | Posaune (jede Note ±10 Cent), Alphorn, Trompete 29/30; Wah-Wah am Harmon | Richtung des Schmetterns (Myers) | 2,4 % |
| 8 Rohrblatt und Rohr | Rohr modal mit Tonlochgitter | Klarinette 32/33, Sopransax 29/33, Tenorsax 30/33, Altsax 21/33 | Chalumeau 11/11, Schwelle (Almeida) | ≈ 5 % |
| 9 Luftstrahl und Rohr | Strahl mit Laufzeit und tanh | Labialpfeife (alle), Blockflöte 20/25, Shakuhachi 6/25 | Fletcher & Douglas | 0,6 % |
| 10 Durchschlagende Zunge | Zunge mit träger Strömung (Puranik & Scavone) | Akkordeon 42/42, Harmonium, Melodica, Mundharmonika (gerade Töne) | Musette 2,00 Hz (Hergert) | 2,4 % |
| 11 Tonrad | Orgel als Ganzes über den Clip, 91 Räder, Rotary, Scanner | `organ`, `house_organ` | Räder auf 0,02 Cent, Scanner V3 ±1,17 % | 2,5–2,9 % |

\* Die Rechenzeit ist der Anteil an der Echtzeit, gemessen relativ zum portierten Flügel auf derselben Maschine. Die
Grenze liegt bei etwa 5 % im tiefsten Ton; darüber gilt „Deluxe“.

**Die Zahlen „x/y“:** so viele Prüfpunkte treffen die Note auf ±10 Cent, meist jede dritte Note bei pp/mf/ff. Das sind
Tonhöhen am Modell, keine Klangurteile.

**Dazu, für alle Klassen gemeinsam:**

- **Messkette B1–B8:**
  - Teiltonverfolgung, Tonhöhe, Vibrato, Geräusch, Einschwingen, Modalfit, Gleiten.
  - Geprüft an synthetischen Signalen.
  - Regeln in `gemeinsam/messkette.md`: Erkennung nur am Bezug (§3a), Glätte des Ziels (§3b, L16), Toleranz als
    Auflösung des Bezugs (§4a).
- **Setz-Werkzeug** `rechnung/setzen/`:
  - Levenberg–Marquardt je Note mit eigenem λ (L8).
  - Wirkungslose Parameter fest (L9), verrauschte gemeldet (L11).
  - Wellige Größen zuerst eindimensional (L12), aber nicht, wenn ein zweiter Parameter dieselbe Wahrnehmung trägt (L16).
  - Protokoll mit Wiederaufnahme (L7), Hörschwelle getrennt (L13), Schnitt (L14), Ergebnis als „Vorschlag“ (L18).
  - Geprüft an synthetischen Aufgaben; der Lauf an Aufnahmen ist lokal.
- **Hörproben:** `ic_hoerproben <Ordner> [Klasse …]`, rund 450 WAVs mit `liste.txt` und einer Frage je Datei.
- **Weitere Werkzeuge:** `ic_stimme_<klasse>` erzeugt die `Stimmung.inc` (Klassen 3, 4, 7, 8, 9, 10). `ic_messe` misst
  lokal an Aufnahmen und liest die Wurzel aus `IC_SAMPLES`.

---

## 3. Erkenntnisse

### 3.1 Über alle Klassen

1. **Spielbar vor Feinabstimmung trägt (L6):**
   - Jede Klasse hat ein Instrument mit Werten aus Quellen, das in der Tonhöhe stimmt und vorab rechenbar ist.
   - Alle Klassen bis auf Klasse 6 liegen unter etwa 5 % der Echtzeit.
   - Das ist der Zustand, in dem `grand_piano` in LumiViz ausgeliefert wurde.
2. **Die Physik bringt Effekte von selbst hervor, ohne Kunstgriff** (Belege für die Modelle):
   - Wolfton am Cello (kleinste Bogenkraft an F3 0,226 N gegen 0,057 N an C3);
   - zweistufiges Abklingen der Zupfsaite (Rate früh/spät 4,6 mit Kopplung gegen 1,4 ohne);
   - Aufblühen des Crash-Beckens bei ff (Anteil über 1 kHz 0,31 → 0,35 in 150 ms);
   - Musette-Schwebung 2,00 Hz;
   - Pauke auf 3 % an Packer.
3. **Das Erregen ist der schwierige Teil, nicht der Resonator:**
   - Die Rohre spielen dort schlecht, wo der Erreger nicht passt:
     - Didgeridoo: Posaunenlippe am weiten Eingang;
     - Oboe: Klarinettenblatt statt Doppelrohrblatt;
     - Shakuhachi: der Ansatz des Spielers fehlt;
     - Saxophone: mehrstabil, Altsax überbläst unten.
   - Mehrstabilität und Registersprünge sind ein Befund des Modells (L4: Struktur an Schwellen), kein Programmfehler.
     Beim Setzen sind das die heiklen Stellen.
4. **Die Lehren des Flügels gelten für alle Klassen** (L1–L18, übernommen nach F-L1–F-L5 a, T9 a, F-B6 a):
   - Toleranz aus dem Bezug, Kurven statt Werte je Taste, Erkennung nur am Bezug, je Note eigener Löser.
   - Glätte des Ziels vor dem Plan, Hörschwelle als Urteil, genau ein zweiter Anlauf.
   - Ausstieg mit Vorschlag statt Zwang.
   - Jeder `messplan.md` enthält jetzt „Feinabstimmung und Ausstieg (L15–L18)“, mit dem heiklen Mechanismus je Klasse
     (Einschätzung): Chöre und Polarisation (1, 2), Vibrato und Wolfton (3), Tremolo (4), Fell gegen Kessel (5),
     Modenpaare (6), Register (7–9), Musette (10), Leslie (11).
5. **Quellenlage:**
   - Saiten, Stäbe und Membranen sind gut belegt.
   - Bei den Bläsern fehlen veröffentlichte Bohrungen in Zahlen. Aus HAL/arXiv kam nichts Brauchbares. Was gebaut
     ist, stammt aus Aufsätzen, die du lokal geladen hast (Soares 2025, Petersen 2020, Ukshini 2025, Mamou-Mani). Die
     Tuba-Quelle von Myers ist nicht mehr abrufbar; Hendrie 2007 enthält kein ganzes Horn.
6. **Determinismus:**
   - Feste Rechenregeln (`-ffp-contract=off`, kein fast-math).
   - Abdrücke je Instrument für gcc und clang bitgleich, auch beim chaotischen Becken.
   - Damit lässt sich jede spätere Änderung am Klang nachweisen.

### 3.2 Je Klasse — die wichtigsten Befunde (vollständig in `offen.md`)

| Klasse | Befund |
|---|---|
| 1 | Die Prüfsumme `hammer_bezug_clang.txt` lässt sich nicht nachrechnen (FNV-1a, `std::hash`, Polynom 31 geprüft); die Pegel stimmen. Clavinet: −3 Halbtöne beim Loslassen auf 0,2 %. Kennlinie des Clavinet-Tonabnehmers: Tab. 2 bei Gabrielli und Remaggi ohne Einheit, nicht gebaut. |
| 2 | Cembalo-Tastenstoß +4,4 dB statt +10 dB je Stufe (Penttinen). Harfe mit Mitschwingen ist teuer (26 % je Clip). |
| 3 | Viola C3 schwingt bei pp nicht und springt bei mf in die Oktave. Violine A4: H1 ≈ 30 dB unter H2 (auffällig). Dämpfer −1,9 dB gegen −5…−13 dB (Elie). |
| 4 | Gestrichenes Vibraphon tief mit Resonator +80…+110 Cent (rastet auf der oberen Paar-Mode ein; ohne Resonator −5 Cent); κ/κ_c offen. |
| 5 | Pauke (1,1) 188 Hz gegen gemessen 172 Hz (mit σ 0,31: 173 Hz; Packer widersprüchlich 0,262/0,31). Kontaktzeit Tom 9–11 ms gegen 5–8 ms. |
| 6 | Becken als flache Platte; oberhalb 1,2–3 kHz fehlt die Bandbreite. Kontakt Crash ff 563 N gegen 100–300 N. Hi-Hat-Schalenkontakt divergierte, nicht gebaut. |
| 7 | Schmettern ×1,54 gegen ×2,7 (Myers). Alphorn-Bohrung ≈ 0,4 m zu kurz (Spitze 2 +186 Cent). Bass bis 50 dB leiser als die Höhe. |
| 8 | Klarinette D4 mf springt auf die Duodezime. Schwelle 2,5–6 kPa gegen Almeida 2–4 kPa. |
| 9 | Shakuhachi 19/25 falsch (Ansatz fehlt). Blockflöte D6–E6 ohne Fenster (+700 … +1636 Cent). |
| 10 | Bend nur −3,7 Cent bei verlangten −80. Basszungen sprechen kaum an. Tonhöhe über dem Druck ≈ −1 Cent gegen −16 ct/kPa. |
| 11 | Rotary-Zeitkonstanten ohne Quelle (Herrera ohne Werte). Vorverstärker und Schweller ohne Kennlinie. |

---

## 4. Was sich schon in LumiViz integrieren lässt

**Grundsatz:** Das Muster ist `grand_piano`. Ein Instrument wird mit dem besten gemessenen Stand mitgeliefert, und
Werte ohne Messung werden innerhalb belegter Grenzen nach Gehör gesetzt (SP1 a). Feinabstimmung kommt danach, mit
denselben Werkzeugen. Hier ist **nichts gemessen**: „integrierbar“ heißt, das Instrument spielt mit Quellenwerten
richtig in der Tonhöhe und in der Rechenzeit. Wie es klingt, entscheidet das erste Hören.

### 4.1 Einteilung (Einschätzung, nach Tonhöhe, Rechenzeit und Rechenweg)

**A — spielbar wie `grand_piano`.** Eine Note rein, ein Ton raus, unter 5 % der Echtzeit, Tonhöhe auf ±10 Cent:

| Klasse | Instrumente | ersetzt in LumiViz (heute) |
|---|---|---|
| 1 | Upright (Stufe 32), Honky-tonk, CP-70, Clavinet | `piano` aus Sinus-Schichten, `clavinet` als Sample |
| 4 | Marimba, Xylophon, Vibraphon, Glockenspiel, Röhrenglocken, Celesta, Kalimba, Rhodes, Wurlitzer | `marimba`, `vibraphone`, `bell` aus Sinus/Dreieck |
| 5 | Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo | Kits im Drum-Editor (`.lvkit`, Rollen) |
| 10 | Akkordeon, Harmonium, Melodica, Mundharmonika | — (neu) |
| 2 | Cembalo, Harfe, Gitarre, Laute, Oud (je Note; Mitschwingen siehe B) | — |

**B — spielbar, braucht aber einen neuen Rechenweg in LumiViz.** Kette, Clip oder Vorabrechnung (T2 a: ein Ton mit der
Spur im Namen, Freeze):

| Klasse | Instrumente | was LumiViz dafür braucht |
|---|---|---|
| 3 | Cello, Violine, Viola, Kontrabass, Ensemble | Ketten (Legato ist ein Ton), Expression, Vibrato |
| 7 | Posaune, Trompete, Alphorn (Dämpfer, Wah-Wah) | Ketten; Wah als Spur |
| 8 | Klarinette, Sopransaxophon, Tenorsaxophon | Ketten; ersetzt `sax` |
| 9 | Labialpfeife, Blockflöte | Ketten; Register |
| 11 | `organ`, `house_organ` | ganzer Clip mit Rotary, Register, Schweller; Rotary-Zustand über Clips |
| 6 | Tamtam, Gong, Crash, China, Splash, Ride, Hi-Hat | Vorabrechnung je Stärke und Schlagstelle (1–7 × Echtzeit), Modellversion im Tonspeicher (E1 a) |
| 1 / 2 | Hackbrett, Cimbalom, Harfe mit Mitschwingen | Clip (Deluxe, 7–26 %) |

**C — vorerst nicht mitliefern.** Spielt nur teilweise; ein Ausliefern würde falsche Töne bringen:

- Altsaxophon 21/33 (tiefe Lage überbläst).
- Shakuhachi 6/25; Blockflöte D6–E6.
- Viola C3; gestrichenes Vibraphon in der tiefen Lage.
- Becken nur als flache Platte. Wenn sie mitgehen, als „vorläufig“ kennzeichnen; die Form wird lokal gesetzt.

**D — nicht spielbar:** Tuba, Horn, Didgeridoo, Oboe, Fagott, Querflöte, Altflöte, Koto. Sie sind als Gerüst im Code,
mit `spielbar = false`.

**Empfohlene Reihenfolge** (Einschätzung): Klasse 1 (Upright als Stufe 32, nach dem Flügel), dann 4 und 5. Diese
Klassen brauchen keinen neuen Rechenweg und ersetzen die schwächsten heutigen Klänge. Danach der Rechenweg für Ketten
(3, 7–9) und Clips (11, 2).

### 4.2 Was vor der Integration zu klären ist (Entscheid nach IK1 — lokal, mit LumiViz)

1. **Schnitt und Repo:**
   - Kommt `rechnung/` als Bibliothek nach LumiViz, oder bleibt es ein eigenes Repo?
   - Die Kerne sind reines C++20 ohne Qt. Das Vorbild hält Qt nur in der JSON-Datei (`hammerSaiteAlsJson`); hier gibt
     es keine `.lvinst`-Anbindung.
2. **Gemeinsame Schnittstelle:** Alle Kerne haben die Form `rechne<X>Ton(spiel, anschlag, mono, frames, messung*)`.
   Unterschiede, für die ein Adapter nötig ist:
   - **Stärke:**
     - `stufe` in dB bei Hammer und Stab, wie beim Flügel (HA3 a);
     - `staerke` 0…1 bei Zupf, Trommel und Becken;
     - `expression` 0…1 plus `haltenS` bei den gehaltenen Klassen.
   - **Note:** Einzelnote; Kette (`kette`) bei 3 und 7–9; ganzer Clip bei 2, 11 und beim Mitschwingen.
   - **Spuren:** Motor, Balg, Pedale der Harfe, Rotary und Register haben im Flügel kein Gegenstück.
   - **Vorbereitung:** `bereite(s)` muss vorher laufen (Trommel, Becken, Blech-Filter).
3. **Was LumiViz dazugibt:**
   - Die Ausgabe ist **mono** und hat keinen Körper aus Bauplänen.
   - Stereo, Körper als Faltung und Pegel (−13,4 LUFS, `instrument_pegel.py`) kommen aus LumiViz, wie beim Flügel.
4. **Erzeugte Stimmungen:**
   - `Stimmung.inc` (Klassen 3, 4, 7, 8, 9, 10) gehört zum Instrument.
   - Nach jeder Modelländerung mit `ic_stimme_<klasse>` neu erzeugen, sonst stimmen die Noten nicht mehr.
   - Fehlt die Datei, stimmt `baueVorgabe…()` selbst; das ist teuer.
5. **Lizenzen** (`UEBERNAHME_LUMIVIZ.md`, ist nachgeführt):
   - Für Werte aus CC-BY-Quellen braucht es einen Eintrag in `CREDITS.md`, z. B.:
     - Trompete: Soares 2025;
     - Altsaxophon: Petersen 2020 und Ukshini 2025;
     - Clavinet: Gabrielli 2013;
     - Upright nach dem Setzen an K18.
   - Aus NC/ND/SA-Quellen wird nichts ausgeliefert (TU-Note, jRhodes3d, SASS-E, VAO). Diese dienen nur zum Messen.
   - Iowa: vor jeder Auslieferung eines daran gesetzten Instruments eine schriftliche Bestätigung einholen.
6. **Rechenzeit:**
   - Die Prozente gelten relativ zum Flügel auf der Cloud-Maschine; lokal neu messen (L7: Wärmegrenze, höchstens 11
     Threads).
   - Echtzeit-Prüfungen kippen, wenn ein zweiter Testlauf gleichzeitig läuft.

---

## 5. Was lokal noch herunterzuladen ist

**Regeln:**

- Nur lokal laden, nie in der Cloud.
- Vor jedem Laden Dateien, Größen und Zielordner zur Freigabe vorlegen (T7).
- **Ziel lokal:** `…\Visuals_Project\cmake\samples\<Klasse n - Name>\<Datensatz>\`. Im Code ist kein Pfad eingetragen,
  die Wurzel kommt aus `IC_SAMPLES`.
- **Im Repo** je Datensatz unter `klassen/<Klasse>/aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` nur drei Dateien:
  `HERKUNFT.md`, `SHA256SUMS` und `LADELISTE.tsv`. Dazu eine Zeile in `UEBERNAHME_LUMIVIZ.md` §3.
- Archive über 100 MB außerhalb laden, auspacken, dann löschen.

**Schon lokal geladen** (nicht nochmals laden):

| Klasse | Datensätze |
|---|---|
| 1 | K18 Upright (CC BY 4.0), VCSL Uprights (CC0) |
| 2 | VCSL Harfen/Cembali, Iowa Gitarre, Mores (Zenodo 4604577) |
| 3 | Iowa Strings stereo, TinySOL Strings, TU-Note Violin (ND, nur messen), VSCO 2 CE Strings, CNSM (18696786), Lampis (13374477) |

### 5.1 Noch zu laden — entschieden als Bezug

Für die Klassen 4–11 ist noch nichts geladen.

| Klasse | Datensatz | Instrumente | Quelle | Lizenz | Zweck |
|---|---|---|---|---|---|
| 1 | FreePats „Old Piano FB“ | Honky-tonk | freepats.zenvoid.org | CC0 | Bezug Verstimmung (M3) |
| 4 | Iowa MIS (pp/mf/ff, alle Schlägel) | Marimba, Xylophon, Vibraphon, Glockenspiel | theremin.music.uiowa.edu/MIS.html | „without restrictions“ | **Bezug** |
| 4 | VCSL | dieselben, Röhrenglocken, Kalimba | github.com/sgossner/VCSL | CC0 | Gegenprüfen |
| 4 | jRhodes3d | Rhodes | github.com/sfzinstruments/jlearman.jRhodes3d | CC BY-NC | **nur messen** (F-7 c) |
| 4 | SASS-E | Steeldrum | Zenodo 7803316 (5,4 GB) | CC BY-NC 4.0 | **nur messen** |
| 5 | Drum Tom-Tom Timbre | Tom | Zenodo 22783358 (90 MB) | CC BY 4.0 | Bezug Moden, Stimmung |
| 5 | DrumGizmo DRSKit 2.1, CrocellKit 1.1 (nur gebrauchte Instrumente) | Snare, Tom, Bassdrum | DrumGizmo-Wiki | CC BY 4.0 | Bezug Stärke, Schlag |
| 5 | VCSL / VSCO 2 CE | Pauke, Hand-Trommeln | github sgossner | CC0 | Bezug |
| 5 | Tabla strokes | Tabla | Zenodo 4327350 (43 MB) | CC BY 4.0 | Bezug |
| 6 | Iowa MIS | Becken, Tamtam, Gong, Hi-Hat | MIS.html | „without restrictions“ | **Bezug** (Form F-5 a) |
| 6 | DrumGizmo (nur Becken) | Hi-Hat, Ride, Choke | Wiki | CC BY 4.0 | Bezug |
| 6 | VSCO 2 CE `varMetal` | Gong (17 Stärken) | github | CC0 | Stärken |
| 6 | Beijing Opera Percussion | Operngongs | Zenodo 1285212 (33 MB) | CC BY 4.0 | Gleiten (M5) |
| 6 | Oslo Cymbal | Hängebecken | Zenodo 21710617 (780 MB) | CC BY 4.0 | Schlagstelle |
| 7 | Iowa MIS | Trompete, Posaune, Horn, Tuba | MIS.html | „without restrictions“ | **Bezug** |
| 7 | TinySOL (Blech) | dieselben | Zenodo 3685367 | CC BY 4.0 | Gegenprüfen |
| 7 | VSCO 2 CE | Dämpfer (Harmon, Straight), Horn mute, Vibrato | github | CC0 | Dämpfer (M5) |
| 8 | Iowa MIS | Klarinette, Altsax, Oboe, Fagott | MIS.html | „without restrictions“ | **Bezug** |
| 8 | TinySOL | dieselben | Zenodo 3685367 | CC BY 4.0 | Gegenprüfen |
| 9 | Iowa MIS | Querflöte, Altflöte | MIS.html | „without restrictions“ | **Bezug** |
| 9 | VCSL, VSCO 2 CE | Blockflöten; Flöte mit Vibrato | github | CC0 | Bezug |
| 9 | VAO Cuntz / Silbermann | Labialpfeifen | Zenodo 22151203 / 22151507 (je 6,1–6,4 GB) | CC BY-SA 4.0 (nur Bericht) | **nur setzen, nie ausliefern** |
| 9 | Lars Palo GrandOrgue | Orgelregister | familjenpalo.se/vpo/download | CC BY-SA (nur Bericht) | wie VAO |
| 10 | TinySOL **ganz** | Akkordeon (dient auch 3, 7, 8) | Zenodo 3685367, `TinySOL.tar.gz` (1,03 GB) | CC BY 4.0 | **Bezug** |
| 10 | Euterpea-Harmonium | Harmonium | GitHub evanyerburgh/euterpea-harmonium | CC0 | Bezug |
| 10 | VCSL Mundharmonikas | Mundharmonika | github sgossner/VCSL | CC0 | Bezug (Blastöne) |
| 10 | fese Pack 1425 | Melodica | Freesound | CC BY 4.0 (je Datei prüfen) | Bezug |
| 11 | beckstrom_hammond_organ_drones | Tonrad-Generator | archive.org (≈ 210 MB) | CC BY 4.0 | Setzen M1, M3 |
| 11 | Freesound 136653, 136652 | Leslie | Freesound | CC0 | Rotordynamik |
| 11 | Zenodo 3562442 | Leslie 145 | Zenodo (968 MB) | CC BY 4.0 | Leslie-Filter (M7/M8) |

### 5.2 Zurückgestellt oder mit Vorbehalt

| Datensatz | Status |
|---|---|
| AG-PT-set (Zenodo 10159492, 6,7 GB) | zurückgestellt bis zum Stahlsaitenkorpus (K2 F-1 a) |
| Lampis Saitenantworten (Zenodo 17749111, ≥ 14 GB) | zurückgestellt wegen Platz (K3 F-9-1 a) |
| TU Berlin Anechoic DB (DOI 10.14279/depositonce-19858) | drei widersprüchliche Lizenzangaben; nach G F-D1 b nur messen, nichts nach LumiViz |
| `snare-position.zip` (DrumGizmo) | keine Lizenz gefunden; erst nach Anfrage beim DrumGizmo-Team |
| Good-Sounds (Zenodo 4588740, 13,9 GB) | Lizenz widersprüchlich; nur nach Prüfung am Ort |
| Ohne Ladeentscheid, nur aufgelistet | Posaune 31 Spieler (Zenodo 18679877); MIPCAT (7042528); Blockflöte Druck/PIV (13919050); VCSL Didgeridoo; Pàmies-Vilà, Seven Violin, URMP |

### 5.3 Ausgeschlossen

- **Nicht kommerziell oder ohne Bearbeitung:** OrchideaSOL, IDMT-SMT, ENST, MDB, MTG Vibrato, Pianobook,
  „PianoSamples“, „Zur Ehre des Alphorns“, Freesound NC-Pakete.
- **Ohne Lizenz:** Harmonica-Dataset, HamOrg.
- **Urheberrechtlich, nur nach Anfrage:** UNSW-Impedanzen, Rau/Smith/Abel.
- **Ungeeignet:** NSynth, E-GMD.

Vollständig je Klasse: `klassen/<Klasse>/aufnahmen.md`; Entscheide in `FRAGEN_KONSOLIDIERT.md` T7.

---

## 6. Was noch weitere Abklärungen braucht

### 6.1 Daten oder Quellen (Suche; teils lokal laden, teils anfragen)

| Klasse | was fehlt | Kandidat |
|---|---|---|
| 7 | Bohrung Horn, Tuba | Pulsreflektometrie Kemp/Campbell (Edinburgh); Myers 2019 nicht mehr abrufbar |
| 7 | Lippenwerte für das Didgeridoo | Fletcher 1983, Tarnopolsky |
| 7 | Alphorn: R1, Mundstück, Becher; echte Ventilzüge der Trompete | Marcinnò (unvollständig) |
| 8 | Doppelrohrblatt (Dynamik, Masse, Resonanz) für die Oboe | Almeida et al. (HAL-00083699, HAL-01161426) |
| 8 | Fagott-Bohrung | Sharp et al. (keine digitalen Daten) |
| 9 | Querflöte/Altflöte: Strahldicke, Mundloch | Fabre et al. ISMA 2019 (nur Bohrung) |
| 1 | Clavinet: Einheit der Kennlinie (Tab. 2), Klangschalter (Tab. 3) | Schaltplan [36] bei Gabrielli oder eine Messung |
| 1 | Upright-Mensur | Fletcher 1964, sonst aus B an K18 zurückrechnen |
| 2 | Harfe: Saiten über den Umfang, Körper; Laute/Oud: Saiten, Decke, Doppelchöre | Le Carrou (nur Abstract); Abdallah 2026 (Bot-Seite) |
| 3 | Kontrabass-Korpus; Viola-Admittanz; Stoffwerte thermische Reibung | Askenfelt 1982 (KTH); Woodhouse 2003 |
| 4 | Unterschnittprofile, Kalimba-Kasten, Motor des Vibraphons | — (Aufnahme mit bekanntem Motor) |
| 5 | Fellspannungen (Bassdrum, Snare-Resonanzfell, Tabla, Conga, Bongo), Kesseltiefen; Flächendichte der Pauke | Händlerangaben; Packer (0,262 oder 0,31) |
| 6 | Beckendicken, Material B20 | nur Brezas für 8″ |
| 10 | Bend-Aufnahme, Basszungen, Harmonium-Filter | Adachi 2026, DAFx23 |
| 11 | Rotary-Zeitkonstanten, Vorverstärker/Schweller | — |
| alle | **Hörschwellen** nachlesen (`gemeinsam/hoerschwellen.md`, heute nur Bericht) | Järveläinen & Tolonen, JAES 49(11) 2001; Järveläinens Dissertation (aaltodoc.aalto.fi, in der Cloud 403); Ewert 2002 |

### 6.2 Lokal (Aufnahmen, Hören, Setzen) — die eigentliche nächste Phase

1. **Hören** (L5):
   - rund 450 Hörproben, mit Urteil und Datum;
   - das Urteil entscheidet, welche Vorgaben brauchbar sind, bevor gesetzt wird.
2. **Laden** nach §5.1, je Klasse vor dem Setzen.
3. **Setzen** nach dem Plan A–D im `messplan.md` jeder Klasse:
   - Toleranz aus dem Bezug, Schnitt vorab, Glätte des Ziels, je Note eigener Löser;
   - ein zweiter Anlauf, Ausstieg mit Vorschlag.
   - **Zuerst die Größen, die nur am Bezug setzbar sind:**
     - Korpus der Gitarre (Mores) und der Violine/des Cellos (CNSM) per Modalfit;
     - Form der Becken (Iowa);
     - κ/κ_c des Vibraphons;
     - Fellspannungen.
4. **Klasse 1:** Das Verfahren der Prüfsumme in LumiViz `test_LoopHammerTon.cpp` nachlesen. Damit `hammer_bezug_*.txt`
   nachrechenbar wird.

### 6.3 Größere Rechnungen, zurückgestellt (Deluxe, F-B5)

- **Fünf Deluxe-Rechnungen:**
  - Halbventil (FDTD);
  - Bogensaite mit Steifigkeit, Torsion, zwei Polarisationen und Bogenbreite;
  - Abstrahlung je Band am Streicher;
  - Scanner als LC-Kette;
  - Torsion und 2. Mode der Zunge.
- **Ohne Deluxe-Vermerk:** thermische Reibung, Steifigkeit der Zupfsaite, Form der Becken (lokal, an Iowa).

---

## 7. Hinweise zum Merge

- **Konflikte:** Der Zweig lässt sich ohne Konflikt nach master mergen (am 2026-10-30 mit `git merge-tree` geprüft;
  master trägt `dld/` und L16–L18). Master wurde nie in den Zweig gemergt; master-Dateien wurden nur gelesen.
- **`dld/` auf master** ist deine Ablage:
  - Was eine CC-Lizenz hat und gebaut wurde, liegt jetzt unter `klassen/<Klasse>/quellen/` mit `SHA256SUMS`: Soares,
    Petiot, Petersen, Ukshini, Gabrielli.
  - Ohne CC-Lizenz (z. B. Hendrie, Mamou-Mani) sind nur die Zahlen mit Fundstelle in `quellen.md` übernommen.
  - `dld/` kann danach weg. Die Dateien ohne CC-Lizenz liegen dann nur noch lokal.
- **Größe:** In der Git-Historie liegen noch rund 2 GB alter Audiodateien (Commits `60bdc39`, `d965732`).
  Kleiner wird es nur durch Umschreiben der Historie (`git filter-repo`, Force-Push); das machst du selbst
  (`UEBERNAHME_LUMIVIZ.md` §3).
- **Nach dem Merge** lokal einmal bauen und testen, auch unter Windows (`/fp:contract-`). Danach die Abdrücke mit
  `rechnung/werte/` vergleichen; für einen neuen Compiler einen eigenen Abdruck anlegen.
