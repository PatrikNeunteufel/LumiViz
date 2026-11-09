# Recherche IC1: Körper und Abstrahlung — welche Klasse welchen Körper braucht, offene Daten, Übertragbarkeit

> **Auftrag:** Welche Instrumente brauchen welchen Körper; offene Daten je Körper (Mores und CNSM nachprüfen, neu:
> Cello, Viola, Kontrabass, Harfe, Cembalo, Trommeln, Marimba-Resonatoren, Geometrien); Übertragbarkeit der Platte
> des Flügels auf Gitarre, Violine, Membran; Abstrahlung kleiner Körper und Raum.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut/Feldinhalt, den WebFetch zitierend zurückgab; [nur Zusammenfassung] = sonst (auch
> Suchtreffer mit Anführungszeichen); [kein Beleg — Fachwissen] = aus dem Gedächtnis des Agenten.
> **Reichweite:** im Bericht selbst (26 Suchen, rund 33 Abrufe, nur Englisch; HAL, UPF, Acta Acustica, IOA, OpenAIR
> nicht erreichbar).
> **Von der Sitzung selbst nachgelesen:** siehe `../koerper.md` (je Angabe vermerkt); was dort nicht als
> „nachgelesen“ steht, gilt als „nur Bericht“.
> **Eingeflossen in:** `../koerper.md`.
> **Anmerkung der Sitzung:** Das PDF, das WebFetch selbsttätig zwischengespeichert hat (Le Carrou), liegt im Cache der
> Claude-Sitzung, nicht in diesem Repo.

---

# Körper und Abstrahlung von Instrumenten: Recherchebericht (IC, Abruf 2026-10-26)

## Reichweite

- **Umfang:** 26 Websuchen (WebSearch, Modus „standard“, englische Begriffe) und etwa 33 WebFetch-Abrufe, zusammen rund 59. Das liegt über den vorgesehenen 25–40. Grund: Viele Abrufe sind gescheitert und wurden wiederholt.
- **Suchbegriffe (Auswahl):** cello bridge admittance dataset zenodo; violin CT scan DICOM Stradivari; open source FEM guitar top bracing github; violin FEM python; Christian … air loading timpani; Bilbao Webb timpani GPGPU; Christensen Vistisen; Gough violin shell FEM; Woodhouse violin review; TU Berlin directivity depositonce; OpenAIR license; harp soundboard admittance Le Carrou; harpsichord soundboard FEM; marimba resonator Doutaut Chaigne; Elejabarrieta guitar coupled modes; snare/tom air coupling Rossing; NESS code license; Ducceschi modal plate code; double bass admittance Askenfelt; violin monopole dipole f-holes; steelpan FEM / tabla membrane cavity; USDA Wood Handbook; guitar BEM vs Rayleigh baffle; koto/oud/kalimba body.
- **Datenbanken/Orte:** Zenodo (Web und API), arXiv (abs und HTML), DepositOnce (TU Berlin), Apollo (Cambridge), Brandeis ScholarWorks; dazu Suchmaschinentreffer.
- **Nicht erreichbar:**
  - Zenodo-Dateien `Method.pdf` und `Comments.pdf` (HTTP 404 mit „Retry-After“, viermal versucht, über die Web- und die API-URL).
  - HAL und UPF-Repositorium: Bot-Sperre („Anubis … Access Denied“).
  - acta-acustica.edpsciences.org: HTTP 403.
  - ioa.org.uk-Proceedings (Gough): HTTP 404.
  - OpenAIR (york.ac.uk): Zertifikat abgelaufen.
  - Le-Carrou-PDF (lam.jussieu.fr): Binärdaten, kein Text lesbar.
- **Hinweis zu „nichts speichern“:** WebFetch hat beim Le-Carrou-PDF die Binärdatei **selbsttätig** abgelegt unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\webfetch-1793015735459-2ffotl.pdf`. Ich habe das nicht veranlasst. Patrik möchte sie gegebenenfalls löschen.
- **Nicht gesucht:**
  - Google Scholar direkt, JASA/AIP-Volltexte (Bezahlschranke), GitHub-Suche direkt.
  - Japanische, arabische und indische Quellen (Koto, Oud, Tabla).
  - Deutsche Fachliteratur außer einem Treffer.
  - Zenodo-Volltextsuche nach „cello“ oder „double bass“ (nur über die Suchmaschine).
- **Zur Kennzeichnung:** Was eine Websuche lieferte, ist eine Zusammenfassung der Suchmaschine. Es ist immer mit **[nur Zusammenfassung]** markiert, auch wenn darin Anführungszeichen stehen. **[wörtlich]** steht nur dort, wo WebFetch Feldinhalt oder Wortlaut zitierend zurückgab.

---

## 1. Welches Instrument braucht welchen Körper?

**Vorbemerkung:** In dieser Recherche belegt sind nur einzelne Punkte (unten mit Quelle). Die Tabelle stützt sich sonst auf Fachwissen aus der Standardliteratur (Fletcher/Rossing, *The Physics of Musical Instruments*; Woodhouse 2014). Sie ist **in dieser Sitzung nicht nachgeprüft** und gilt als **[kein Beleg — Fachwissen]**.

| Instrument | „Körper“ in der Literatur | Hörbar wichtige Effekte | Belege aus dieser Recherche |
|---|---|---|---|
| Akustikgitarre (Nylon/Stahl) | Decke mit Beleistung (bracing), Luftraum mit Schallloch, Boden, Zargen | Helmholtz-Luftmode A0 gekoppelt mit der ersten Deckenmode T(1,1); dazu die Boden-Mode (Dreifachkopplung Decke–Luft–Boden); Stahlsaiten-Gitarre mit X-Beleistung und höherer Steifigkeit | Christensen–Vistisen als Zwei-Freiheitsgrad-Modell (2 DOF) der ersten zwei gekoppelten Moden [nur Zusammenfassung]. Mores 2020: asymmetrische Beleistung, mehrere Resonanzen „between A0 and A1“ [wörtlich] |
| Harfe | Resonanzboden, an dem die Saiten befestigt sind, mit Schallkasten und Löchern im Rücken | A0 und T1. Die Löcher im Kasten sind akustisch wichtig, obwohl sie zum Saitenaufziehen gedacht sind. Dazu sympathisch mitschwingende Saiten | Le Carrou/Gautier/Foltête [nur Zusammenfassung]; Le Carrou/Gautier/Badeau, ESPRIT, sympathische Moden [nur Zusammenfassung] |
| Cembalo | Resonanzboden mit Rippen und Steg, Kasten mit Luftraum (offen nach unten oder mit Rose) | Dem Flügel am nächsten. Randbedingungen und Saitenzug verändern die Bodenmoden | Le Conte et al., J. Cultural Heritage 2012, FEM eines Couchet-Cembalos (nur über eine Scribd-Kopie gesichtet, Zugang ungeklärt) [nur Zusammenfassung] |
| Koto | Langer, gewölbter Kasten aus Paulownia mit Schalllöchern im Boden | Kastenmoden und Luftraum; bewegliche Stege (ji) | Keine Koto-Arbeit gefunden |
| Laute/Oud | Dünne flache Decke mit Querbalken, gewölbter Muschelboden, Rosette | A0, Decke–Balken-Moden | Oud: Modalanalyse mit Hammer plus FEM bis 400 Hz, Variation von Schallloch, Deckenstärke und Beleistung (World Scientific; Autoren nicht gesichtet) [nur Zusammenfassung] |
| Violine | Gewölbte Decke (Fichte) mit Bassbalken, gewölbter Boden (Ahorn), Stimmstock, f-Löcher, Luftraum | A0 bei etwa 280 Hz, Centre-Bout-Rotation (CBR) bei 350–450 Hz, B1−/B1+ als Hauptstrahler; Steg-Hügel (bridge hill) um 2–3 kHz; Wolfton beim Cello | Gough (IOA), Signaturmoden „A0 air resonance at ~ 280Hz“ [nur Zusammenfassung]; Bissinger: f-Löcher tragen mehr als 50 % der Abstrahlung bei B1 [nur Zusammenfassung, über strad3d.org] |
| Viola, Cello, Kontrabass | Gleicher Aufbau wie Violine, skaliert, aber nicht ähnlich skaliert: Luftraum zu klein im Verhältnis | Lage von A0 und B1 relativ zum Tonumfang; Wolfton (Kopplung Saite–starke Körpermode) besonders beim Cello; beim Bass Ankopplung über den Stachel an den Boden | Kontrabass: Helmholtz-Resonanz „near 60 Hz“, Ankopplung an nachgiebigen Bühnenboden (Guettler/Buen/Askenfelt 2008) [nur Zusammenfassung]; Askenfelt 1982 „Eigenmodes and Tone Quality of the Double Bass“ [nur Zusammenfassung] |
| Marimba, Xylophon, Vibraphon | Stab (Biegebalken) und darunter ein Rohrresonator, unten geschlossen | Verstärkung des Grundtons, längere Anschwingzeit, verkürztes Ausklingen, Abstand Stab–Rohr; beim Vibraphon Tremolo durch Drehklappen | Doutaut/Matignon/Chaigne 1998: Stab als Reihe pulsierender Kugeln, offenes Rohrende als pulsierende Kugel [nur Zusammenfassung]; Soares/Debut/Antunes 2022, konzentrierte Parameter (lumped), Dämpfungsverhältnis Stab/Rohr [nur Zusammenfassung] |
| Kalimba | Holzkasten mit Luftraum und Löchern (Helmholtz) | Kastenmode und Luftmode; „Wah“ durch Abdecken der Löcher | Ludwigsen 2008, „Modes of the Kalimba Resonator Box“, JASA, DOI 10.1121/1.4782900 (nur Abstract) [nur Zusammenfassung]; taiwanische Masterarbeit 2018 mit FEM und Kopplung an den Hohlraum [nur Zusammenfassung] |
| Glockenspiel | Meist kein Resonator; Stäbe auf Filz, Kasten schwach | Körper vermutlich vernachlässigbar | — |
| Celesta | Stäbe mit Holzresonatoren (Kästen) je Ton | Ähnlich wie Marimba-Resonator | Nicht gesucht |
| Steeldrum | Gehämmerte Schale mit Tonfeldern, Mantel (skirt) | Lokalisierung der Moden im Tonfeld, Kopplung zwischen Tonfeldern, Mantelmoden | Bryde/Mahadevan, arXiv 2212.14465: „the skirt of the pan is not included in our model“ [nur Zusammenfassung]; Hansen/Rossing: Mantelmode bei etwa 830 Hz [nur Zusammenfassung] |
| Pauke | Membran über starrem Kessel, Luft innen und außen | Luftlast verschiebt die Moden in eine fast harmonische Reihe; (0,1)-Mode stark gedämpft durch Abstrahlung | Christian et al. 1984 [nur Zusammenfassung, Zitat aus einer Literaturliste]; Mills 1980: Massenbelastung als Hauptursache der Frequenzverschiebung [nur Zusammenfassung] |
| Toms, Bassdrum, Snare | Zwei Felle, Kessel, eingeschlossene Luft | Kopplung der Felle **über die Luft** stärker als über den Kessel; gleich- und gegenphasige Modenpaare | Zhao 1990 (Masterarbeit, Betreuer Rossing): „heads interact mainly through the enclosed air“ [nur Zusammenfassung]; Worland/Boe, Vienna Talk 2015, Speckle-Interferometrie (ESPI) und FEM [nur Zusammenfassung] |
| Conga, Bongo, Tabla | Ein Fell über offenem (Conga, Bongo) oder geschlossenem (Tabla, Bayan) Körper | Tabla: belastete Membran (syahi) macht harmonische Obertöne; Luftraum | Keine Tabla-FEM-Arbeit gefunden |

---

## 2. Offene Daten je Körper

### 2.1 Mores-Archiv (Zenodo 4604576): nachgeprüft

- **Lizenz:** API-Feld `license.id`: „cc-by-4.0“ [wörtlich, zenodo.org/api/records/4604576, Abruf 2026-10-26]. Weitere Felder: `access_right` „open“, `publication_date` „2021-03-15“, Ersteller „Robert Mores“, Hamburg University of Applied Sciences [wörtlich]. Die Web-Ansicht des Datensatzes zeigte kein Lizenzfeld, nur „Open“ [wörtlich]. Die Lizenz ist daher nur über die API belegt.
- **Dateien:** 342 Dateien [wörtlich, API]. Darunter:
  - `qualified_selected_impulses.mat` (32 729 680 Byte), `Method.pdf` (465 263), `Comments.pdf` (112 585)
  - `List_of_guitars_physical_measures.pdf`, `List_of_guitars_description.pdf`
  - Datenblätter der Messgeräte: `Instrumentation_KISTLER_8778A500.pdf`, `…_DYTRAN_5800SL.pdf`, `…_HEIM_DIC6.pdf`, `…_ROGA_MI17.pdf`
  - Matlab-Skripte `batch_extract_bridge_mobility_archive.m` und `create_fig_archive.m`
  - je Gitarre `bridge_mobility_gNN.jpg` sowie Fotos der Beleistung
- **Kanäle:** Laut Beschreibung eine Zweikanal-Messung am Steg und drei Nahfeldkanäle [nur Zusammenfassung des Beschreibungsfelds].
- **Vermutete Geräte (nicht belegt):** Die Dateinamen legen nahe, dass ein Kistler-Mikrofon und Dytran-Hammer oder -Sensoren verwendet wurden. Das ist ein Schluss aus Namen, nicht belegt.
- **Abtastrate, Fensterlänge, Aufbau der .mat-Datei: NICHT geprüft.** `Method.pdf` und `Comments.pdf` waren nicht abrufbar (siehe Reichweite). Hier muss nachgearbeitet werden.
- **Neu gefunden, Ergänzung desselben Autors:** Zenodo 4010596, „Tuning asymmetrically braced guitars - analytical models and MATLAB code“, Robert Mores, 2020-09-01. Lizenz `license.id` „cc-by-4.0“ [wörtlich, zenodo.org/api/records/4010596, Abruf 2026-10-26]. Dateien: zwei .m-Skripte und `Tuning_for_asymmetric_guitar_bracing.pdf` [wörtlich]. Das ist ein offenes analytisches Gitarrenmodell mit Code.

### 2.2 CNSM-Violinen (Zenodo 18696785, arXiv 2605.18305): nachgeprüft

- **Paper:** „Violin "Playing-In": Disentangling Physical Change from Player Adaptation via Physical Measurements“. Hugo Pauget Ballesteros, Philippe Lalitte, Vincent Lostanlen, Claudia Fritz [wörtlich, arxiv.org/abs/2605.18305].
  - Lizenz des Papers: arXiv „nonexclusive-distrib/1.0“ [wörtlich], also **nicht** CC.
  - Frei lesbar.
- **Zahl der Violinen:** **drei** (eine Testvioline, zwei Kontrollviolinen) [nur Zusammenfassung, Abschnitt 3.1 im HTML].
- **Messaufbau der Admittanz** (Abschnitt 2.4.1 „Bridge mobility“, arxiv.org/html/2605.18305):
  - Anregung: „an instrumented impact hammer (PCB 086E80)“ [wörtlich]
  - Messung: „Laser Doppler Vibrometer (Polytec OFV-505/OFV-3001)“ [wörtlich]
  - Anregung „on the G-string side“, Messung „at the E-string side“ [wörtlich]
  - Lagerung: „placed horizontally on a block of soft foam“ [wörtlich], Saiten gedämpft [nur Zusammenfassung]
  - Wiederholung: „5 consecutive impacts“ je Übertragungsfunktion (FRF) [wörtlich], dreimal je Phase [wörtlich]
  - Frequenzbereich und Abtastrate der Admittanzmessung stehen dort **nicht** [nur Zusammenfassung]
- **Datensatz:**
  - Titel „CNSM Dataset“, Ersteller „Pauget Ballesteros, Hugo“, `license.id` „cc-by-4.0“, `publication_date` „2026-02-19“ [wörtlich, zenodo.org/api/records/18696785, Abruf 2026-10-26].
  - Eine einzige Datei `dataset.zip`, 654 025 212 Byte [wörtlich]. Kein Beschreibungsfeld.
  - **Inhalt des ZIP weiterhin ungeprüft** (nicht heruntergeladen).
  - Das Paper selbst nennt für die Daten keine Lizenz [nur Zusammenfassung].

### 2.3 Neu gesucht

**Violine, Geometrie (CT):**
- Brandeis, „1716 Stradivari violin "Messiah" (Ashmolean Museum, Oxford)“. Ersteller „Digital Amati (Research Group)“ und Harry Mairson. DOI 10.48617/data.1109 [wörtlich].
- **Nicht frei:** Zugang „by permission only“, Nutzung „for personal use only, and not to be sent to anyone else“, CT-Ordner „Restricted“ [wörtlich, Datensatzseite, Abruf 2026-10-26]. Kein Lizenzfeld.
- Strad3D und die Smithsonian-CT-Studie: Ansichten im Netz, kein Rohdaten-Download gefunden [nur Zusammenfassung].

**Cello, Viola:**
- Kein offener Datensatz gefunden.
- Maestre/Scavone/Smith 2013 (SMAC), „Digital modeling of bridge driving-point admittance from measurements on violin-family instruments“, liegt im UPF-Repositorium [nur Zusammenfassung]. Bot-Sperre; ob Messdaten dabei sind, ist unbekannt.
- Zhang/Woodhouse, „Motion of the cello bridge“ (IOA): Admittanz an fünf Stegpunkten [nur Zusammenfassung].
- Deutscher Bericht (UniBw München) „cello 2009 3 steg“: B&K-Impedanzkopf, bis 1 kHz [nur Zusammenfassung]. Keine Daten.

**Kontrabass:** Nur Literatur (Askenfelt 1982; Guettler/Buen/Askenfelt 2008), keine Daten.

**Violine, weitere:** „Bridge admittance measurements of 10 preference-rated violins“, HAL hal-00810717 (Saitis et al.) [nur Zusammenfassung]. HAL gesperrt, Daten und Lizenz ungeprüft.

**Harfe:**
- Canadian Acoustics, „Vibrational characteristics of harp soundboards“ (jcaa.caa-aca.ca, Artikel 2108): Admittanz entlang der Saitenlinie, 0,2-g-Beschleunigungsaufnehmer, Hammer, Salvi Aurora [nur Zusammenfassung]. Frei lesbar, keine Daten.
- Forum Acusticum 2023 (dael.euracoustics.org …/000712.pdf): Konduktanzkarten [nur Zusammenfassung].
- Kein Datensatz gefunden.

**Cembalo:** Nur Le Conte et al. 2012 (siehe Tabelle). Keine Daten.

**Pauke, Trommelkessel:**
- Keine offenen Messdaten gefunden.
- Pauken-Aufnahmen und Richtwirkung gibt es im TU-Berlin-Datensatz, siehe 4.

**Marimba-Resonatoren:** Nur Literatur, keine Daten.

**Gitarre, Baupläne und offene FEM:**
- Kein frei lizenziertes FEM-Modell und kein frei lizenzierter Bauplan gefunden.
- Politecnico Milano, Abschlussarbeiten Vassena 2021 (parametrisches Netz nach Torres, COMSOL) und Di Bella 2024 (parametrisches CAD-Modell): frei lesbare PDFs, Modelldateien nicht veröffentlicht [nur Zusammenfassung].
- Longo et al., JASA 2025, parametrisches Archtop-FEM-Modell: hinter Bezahlschranke [nur Zusammenfassung].

**Holzwerte:**
- USDA Forest Products Laboratory, *Wood Handbook*: als Werk einer US-Bundesbehörde vermutlich gemeinfrei. **Nicht am Ort geprüft.** Die Ausgabe FPL-GTR-282 wurde nicht abgerufen.
- Sitka-Fichte nach FPL 1999, zitiert in einer Springer-Tabelle: ER/EL 0,078, νLR 0,372 [nur Zusammenfassung].

---

## 3. Lässt sich die Platte des Flügels übertragen?

### Gitarre

- **Christensen, O.; Vistisen, B. B. (1980):** „Simple model for low-frequency guitar function“, JASA 68. Seitenangabe uneinheitlich: 756–766 oder 758–766 [nur Zusammenfassung]. Nicht frei.
  - Modell mit zwei Freiheitsgraden (2 DOF) aus Mobilitätsmessung an der Decke.
  - Erfolgreich für 100–800 Hz bei fünf Gitarren [nur Zusammenfassung, Sekundärquelle].
- **Elejabarrieta, Ezcurra, Santamaría (2001):** „Vibrational behaviour of the guitar soundboard analysed by the finite element method“, Acta Acustica united with Acustica 87(1), 128–136.
- **Dieselben (2002):** „Coupled modes of the resonance box of the guitar“, JASA 111(5), 2283–2292, DOI 10.1121/1.1470163 [nur Zusammenfassung, Metadaten der Repositorien]. Nicht frei. Inhalt nicht gesichtet: Gesamtkasten mit Luft (FEM).
- **Richardson/Walker (IOA):** Modell der Gitarre in einer unendlichen Schallwand. Übereinstimmung „only superficial“. Kopplung Decke–Luft und Abstrahlung durchs Schallloch sind nötig [nur Zusammenfassung].
- **Richardson/Brooke (IOA):** Randelementmethode (BEM) über alle Oberflächen [nur Zusammenfassung].
- **Davis 1990 (U Washington):** orthotrope Platte mit Leisten, Rayleigh-Ritz, Kolbenmodell mit Hohlraum und Schallloch [nur Zusammenfassung].

**Folgerung (eigene Einschätzung, kein Beleg):** Die Platte mit Leisten des Flügels lässt sich direkt auf die Gitarrendecke übertragen. Zwei Dinge fehlen:
1. die Luft (A0) und der Boden,
2. eine Abstrahlung ohne unendliche Schallwand.

### Violine

- **Gough (IOA, Titel laut Suche):** „A generic shell model for instruments of the violin family“ und „Finite element analysis of violin shell modes“.
  - Der Ansatz zielt auf Verständnis, nicht auf ein bestimmtes Instrument [nur Zusammenfassung].
  - Die PDFs waren nicht abrufbar (404). Software, Netz und Rechenzeit sind daher **nicht belegt**.
- **Woodhouse, Jim:** „The acoustics of the violin: a review.“ Reports on Progress in Physics 77, 115901 (2014), DOI 10.1088/0034-4885/77/11/115901. Kopie im Cambridge-Repositorium (Handle 1810/245817, Datei `OA-1506_Violin.pdf`), Rechtefeld „all-rights-reserved“ [wörtlich, Abruf 2026-10-26]. Frei lesbar, nicht frei nutzbar.
- **Folgerung (eigene Einschätzung):** Eine gewölbte Schale lässt sich nicht als ebene Platte rechnen. Man braucht Schalenelemente, den Stimmstock als Feder oder Zwangsbedingung und die f-Löcher, die die Decke schwächen.

### Membran mit Luftlast (Pauke)

- Christian, Davis, Tubis, Anderson, Mills, Rossing (1984), „Effects of air loading on timpani membrane vibrations“, JASA 76(5), 1336–1345 [nur Zusammenfassung, Literaturliste]. Nicht frei.
- Davis (Purdue, Dissertation, Betreuer Tubis): Green-Funktion und Randintegrale, starrer Kessel [nur Zusammenfassung].
- Bilbao/Webb, JAES 61(10), 2013, 737–748; DAFx 2012 „Timpani Drum Synthesis in 3D on GPGPUs“: Finite Differenzen im Zeitbereich (FDTD) mit 3D-Luft, Kessel mit Normalgeschwindigkeit null, hohe Rechenkosten [nur Zusammenfassung]. AES nur für Mitglieder.
  - Die Arbeit von C. J. Webb liegt auf ness.music.ed.ac.uk, vermutlich frei. Die Lizenz wurde nicht geprüft.
- Chaigne/Joly/Rhaouti 2000: Methode der fiktiven Gebiete (fictitious domain), innere und äußere Luft gekoppelt [nur Zusammenfassung].

### Rechenkosten und offene Implementierungen

- Keine belastbaren Zahlen gefunden.
- **Keine** offene, lizenzierte Implementierung eines Instrumentkörpers gefunden: keinen NESS-Code und keinen Ducceschi-Modalcode.
- Allgemeine offene Werkzeuge, nur aus Suchtreffern und ohne geprüfte Lizenz:
  - FeenoX mit Gmsh (Beispiel für Modalanalyse)
  - SfePy
  - Akantu (LGPL laut EPFL-Seite) [nur Zusammenfassung]
- Ausnahme: der offene Matlab-Code von Mores (Zenodo 4010596, cc-by-4.0, analytisch, kein FEM).

---

## 4. Abstrahlung kleiner Körper und Raum

### Monopol und Dipol, Schalllöcher

- Unterhalb etwa 500 Hz ist der Monopolanteil wichtig. Bei A0 strahlt die Violine durch reine Volumenänderung (PMC4353046; UNSW, McLennan) [nur Zusammenfassung].
- Weinreichs „sound hole sum rule“ (Summenregel der Schalllöcher): Bei inkompressibler Luft verschwindet das Monopolmoment eines Körpers mit Löchern, er strahlt bei tiefsten Frequenzen nicht kugelförmig [nur Zusammenfassung, BYU-Abstract].
- Bissinger 2007 (Nahfeld-Holografie): Die f-Löcher tragen mehr als 50 % der Abstrahlung bei B1 und strahlen gerichteter [nur Zusammenfassung über strad3d.org].
- Politzer, David: „Sound Hole Sound“, arXiv 1505.04787 (2015), Lizenz arXiv nonexclusive-distrib/1.0 [wörtlich]. Frei lesbar. Behandelt Helmholtz' Ergebnis „steady flow through an elliptical hole“ [wörtlich].

### Wann gilt Rayleigh in Schallwand?

- Die Gitarre in unendlicher Schallwand passt nur oberflächlich zu Messungen (Richardson/Walker) [nur Zusammenfassung].
- **Eigene Einschätzung, kein Beleg:**
  - Rayleigh setzt eine ebene Fläche in einer starren unendlichen Wand voraus.
  - Für einen kleinen freien Körper (ka < 1) fehlt die Rückseite. Front und Rückseite heben sich als Dipol zum Teil auf, und die Abstrahlung durch Loch und Decke ist ein Monopol.
  - Für die Gitarre also eher: Monopol- plus Dipol-Ersatzquellen oder BEM. Beim Flügel ist die Schallwand-Näherung wegen der Größe und des Rahmens eher vertretbar.

### TU-Berlin-Richtcharakteristiken: nachgeprüft

- **Paper:** „A Database with Directivities of Musical Instruments“, David Ackermann, Fabian Brinkmann, Stefan Weinzierl. arXiv 2307.02110, eingereicht 5. Juli 2023 [wörtlich]. Die Paper-Lizenz ist dort CC BY-SA 4.0 (Link „view license“) [nur Zusammenfassung].
  - Im Text: „The data is available under the CC BY-SA 4.0 licence.“ [wörtlich, arXiv-HTML v1]
  - Daten sollten „after the review … with a DOI on the DepositOnce repository“ erscheinen. Vorläufig auf tubcloud.tu-berlin.de/s/8joeeK3fFingLgp [wörtlich].
- **Instrumente laut Tabelle 1 (wörtlich), 40 Zeilen:**
  - Streicher: Violin, Viola, Cello, Double bass (je m/h)
  - Acoustic guitar (m), Double action harp (m)
  - Holz: Oboe, Classic oboe, Romantic oboe, English horn, Clarinet (m/h), Bass clarinet, Bassoon, Classic bassoon, Baroque bassoon, Contrabassoon, Dulcian, Alto/Tenor saxophone, Flute, Transverse flute, Keyed flute, Basset horn
  - Blech: Trumpet, Natural trumpet, French horn, Natural horn, Tenor trombone (m/h), Alto trombone, Bass trombone, Tuba
  - Pauken: **Timpani (m), Pedal timpani (m)**
  - Soprano (Sängerin)
  - Die Zahl „41“ entsteht vermutlich durch Zeilen, die zusammengefasst sind [nur Zusammenfassung].
  - **Für das Projekt relevant:** Streicher, Gitarre, Harfe, Pauke. **Nicht** enthalten: Cembalo, Stabspiele, Trommeln außer Pauke.
  - Terzbänder: „M=30 centre frequencies from 25 Hz to 20 kHz“ [wörtlich].
- **Endgültiger Ort (gefunden):** DepositOnce, „A Database of Anechoic Microphone Array Measurements of Musical Instruments“.
  - Autoren laut Seite: Weinzierl, Vorländer, Ackermann, Behler, Brinkmann, von Coler, Detzner, Krämer, Lindau, Pollow, Schulz, Shabtai.
  - DOI 10.14279/depositonce-19858, `dc.date.issued` „2017-04-10“, Version 3 vom 2024-02-12 „aktualisierte Dateien“.
  - Dateien u. a. `1a_DirectivitiesSingleTones_SOFA.zip`, `1b_Directivities3rdOctave_SOFA.zip`, `…OpenDAFF.zip`, `…GLL.zip`, `0_Documentation.pdf` [wörtlich, Abruf 2026-10-26].
  - Begleitartikel DOI 10.17743/jaes.2022.0128 [wörtlich].
- **Lizenz am Ort: WIDERSPRUCH.**
  - Die DepositOnce-Seite zeigt `dc.rights.uri` „http://rightsstatements.org/vocab/InC/1.0/“ und den Rechtehinweis „In Copyright“ [wörtlich, Abruf 2026-10-26].
  - Im License-Bündel liegt eine `license.txt` (2,62 KB), die ich nicht gelesen habe.
  - Das arXiv-Paper sagt CC BY-SA 4.0.
  - **Bis zur Klärung (license.txt lesen, `0_Documentation.pdf` lesen oder bei der TU Berlin fragen) gilt die Lizenz als ungeklärt.**

### Raum

- Übliche Lösung in der Literatur (eigene Einschätzung, in dieser Sitzung nicht belegt): Faltung des trockenen Signals mit einer Raumimpulsantwort (RIR). Bei Richtwirkung: Richtcharakteristik als Quellbeschreibung in einer Raumsimulation (deshalb liefert die TU Berlin OpenDAFF und GLL).
- **OpenAIR (University of York):** Lizenz je Impulsantwort, vom Beitragenden gewählt. Jede CC-Lizenz oder „reserve all rights“ ist möglich, die Angabe steht unten auf der jeweiligen Seite [nur Zusammenfassung, Suchtreffer].
  - Die Seite selbst war nicht abrufbar (Zertifikat abgelaufen). **Keine wörtliche Lizenz am Ort.**

---

## Nicht gefunden

(Kein Treffer ist kein Beweis. Die Suche war englisch und auf eine Suchmaschine beschränkt.)

- **Abtastrate, Fensterlänge, Kanalbelegung und .mat-Aufbau bei Mores:** `Method.pdf` und `Comments.pdf` waren nicht abrufbar.
- **Inhalt von CNSM `dataset.zip`:** Frequenzbereich und Abtastrate der Admittanz.
- **Offene gemessene Daten:** keine Mobilität oder Admittanz für Cello, Viola, Kontrabass, Harfe, Cembalo, Pauke oder Trommelkessel, keine Marimba-Resonatordaten.
- **Geometrie:** keine frei lizenzierten CT-Daten einer Violine (Messiah nur auf Anfrage), keine offenen Gitarren-Baupläne, kein offenes FEM-Modell von Violine oder Gitarre.
- **Code:** keine offene, lizenzierte Implementierung eines gekoppelten Körpers (FEM mit Luft) für irgendein Instrument.
- **Literatur je Instrument:** keine Koto- und keine Tabla-Arbeit. Celesta und Glockenspiel nicht gesucht.
- **Nicht gelesen:** Rechenzeiten und Modenzahlen aus Gough, Elejabarrieta und Bilbao/Webb (Volltexte nicht erreichbar).
- **Ungeklärt:** wörtliche Lizenzen bei OpenAIR und auf der DepositOnce-Seite (dort steht „In Copyright“).

## Eigene Einschätzung (kein Beleg)

1. **Nah am Flügel:** Cembalo und Harfe passen am besten zum Verfahren des Flügels (ebener Boden mit Rippen). Die Harfe braucht zusätzlich Kasten und Löcher.
2. **Gitarre:** Die Gitarre braucht mindestens eine dritte Komponente: Decke, Luft (A0) und Boden, wie im Christensen–Vistisen-Modell mit drei Oszillatoren. Für diese Klasse liegen die besten offenen Messdaten vor (Mores, 65 Gitarren, cc-by-4.0). Das spricht dafür, dort mit gemessenen Moden zu beginnen und das gerechnete Modell daran zu prüfen.
3. **Violinfamilie:** Rechnen ist deutlich teurer (Schale, Stimmstock, f-Löcher). Ein Modell aus gemessenen Daten (Maestre-artige Modenanpassung) ist vermutlich der realistischere Weg. Offen verfügbar sind dafür nur die drei CNSM-Violinen.
4. **Pauke, Trommeln:** Die Luftlast ist kein kleiner Zusatz, sie bestimmt die Tonhöhen. Eine Membran im Vakuum mit Rayleigh reicht nicht. Man braucht mindestens eine Modalkorrektur über eine Luftlast oder eine Hohlraum-Feder. Für Trommeln mit zwei Fellen ist die Kopplung über die Luft das Kernstück.
5. **Stabspiele:** Hier ist der „Körper“ ein eindimensionales Rohr. Statt einer Liste von Plattenmoden passt besser ein Stab gekoppelt an eine Rohrmode mit Abstrahlung als Kugelquelle (Doutaut/Chaigne).
6. **Rayleigh in Schallwand:** Für kleine freie Körper eher ungeeignet. Ersatzquellen (Monopol plus Dipol, gewichtet) oder gemessene Richtcharakteristiken wären fachlich treuer. Der TU-Berlin-Datensatz deckt Streicher, Gitarre, Harfe und Pauke ab. Er ist erst nutzbar, wenn die Lizenz geklärt ist.
