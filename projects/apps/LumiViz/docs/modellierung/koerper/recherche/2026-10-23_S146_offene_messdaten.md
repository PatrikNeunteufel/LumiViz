# Recherche S146: frei lizenzierte Messdaten zu Instrumentenkörpern

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** frei lizenzierte (CC0, CC BY, gemeinfrei, MIT o. ä.) gemessene
> Impulsantworten, Admittanzen oder modale Datensätze von Klavier-/Flügelböden, ersatzweise anderer Körper; die
> Lizenz je Kandidat wörtlich. Nichts heruntergeladen.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = Feldinhalt, den das Werkzeug als Zitat zurückgab (am verlässlichsten aus den JSON-APIs),
> [nur Zusammenfassung] = sonst.
> **Von Claude selbst nachgelesen:** AudioLabs Erlangen „we measured impulse responses of the soundboard of a
> Yamaha C3 grand piano“, ohne Download und Lizenz (S146, vor dem Auftrag); Salamander-Lizenz „Attribution 3.0“
> (S141). Alles andere: nur Bericht.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund vor 31c“ und „Befund Recherche Boden und Körper“.

---

**Bericht: frei lizenzierte Messdaten zu Instrumentenkörpern (Stand 23.10.2026, nur gelesen, nichts heruntergeladen)**

Vorbemerkung zur Belegkraft: Alle Abrufe liefen über WebFetch und WebSearch. Beide Werkzeuge fassen zusammen. Als **[wörtlich]** markiere ich nur Feldinhalte, die das Werkzeug als Zitat zurückgab, am verlässlichsten aus den JSON-APIs von Zenodo und archive.org. Alles andere ist **[nur Zusammenfassung]**.

## Ergebnis vorweg
Einen frei lizenzierten, gemessenen Datensatz eines **Klavier- oder Flügel-Resonanzbodens** habe ich nicht gefunden. Brauchbar und frei sind zwei Ersatzkandidaten: Gitarre (Mores, CC BY 4.0) und Violine (CNSM, CC BY 4.0).

## Kandidaten

**1. Mores: Archiv klassischer, Flamenco- und romantischer Gitarren, Brückenmobilität (HAW Hamburg)**
- URL: https://zenodo.org/records/4604576 (DOI 10.5281/zenodo.4604576), Spiegel: https://reposit.haw-hamburg.de/handle/20.500.12738/12269
- Lizenz **[wörtlich, Zenodo-API]**: `"license": {"id": "cc-by-4.0"}`. Auf der HAW-Seite **[wörtlich]**: "This item is licensed under a Creative Commons License", das verlinkte Abzeichen nennt CC BY 4.0.
- Inhalt **[wörtlich]**: "The two-channel vibro-acoustical measurement at the bridge captures the two main degrees of freedom", dazu "three more channels sparsely capture near-field radiation." Zur Aufbereitung: "Documentation of wall distances allow adequate post-preparation of short reflection-free impulse responses."
- Inhalt **[nur Zusammenfassung]**:
  - Rohdaten `qualified_selected_impulses.mat` mit 32,7 MB, insgesamt rund 1,1 GB (überwiegend Fotos).
  - 63 Plots der Brückenmobilität, MATLAB-Skript `batch_extract_bridge_mobility_archive.m`, PDFs zur Methode.
  - Datenblätter: Impulshammer DYTRAN 5800SL, Beschleunigungsaufnehmer von Kistler.
  - Über 60 Gitarren, gemessen vor Ort; Boden- und Wandreflexionen sind eine bekannte Grenze.
- Abtastrate: nicht gefunden, sie steht vermutlich in `Method.pdf`.
- **Bester Ersatzkandidat** (Hammer, Brücke, zwei Richtungen, viele Instrumente).

**2. CNSM-Datensatz, Violine („Playing-In“, Pauget Ballesteros)**
- URL: https://zenodo.org/records/18696785; Code: https://github.com/hugo-paugesteros/CNSM-dataset
- Lizenz **[wörtlich, Zenodo-API]**: `"license": {"id": "cc-by-4.0"}`. Die Seite zeigt **[wörtlich]** "Creative Commons Attribution 4.0 International".
- Inhalt **[nur Zusammenfassung]**: Laut dem Paper arXiv 2605.18305 enthält er Brückenadmittanz, Motion-Capture und Audio. Auf Zenodo liegt `dataset.zip` mit 654,0 MB; der Eintrag hat keine Beschreibung.
- Messaufbau, Format und die Zahl der Violinen konnte ich nicht lesen. Das README auf GitHub ließ sich nicht abrufen, und im Repo ist keine LICENSE-Datei zu sehen.

**3. Salamander Grand Piano V3 (Yamaha C5)**
- URL: https://archive.org/details/SalamanderGrandPianoV3
- Lizenz **[wörtlich, Metadaten]**: licenseurl "http://creativecommons.org/licenses/by/3.0/". Das Feld Usage lautet **[wörtlich]** "Attribution 3.0", der Lizenzabschnitt **[wörtlich]** "CC-by".
- Inhalt **[nur Zusammenfassung]**:
  - 48 kHz / 24 bit, 16 Anschlagstufen, zwei AKG C414 etwa 12 cm über den Saiten.
  - Hammergeräusch-Releases chromatisch in einer Stufe; Saitenresonanz-Releases in kleinen Terzen in drei Stufen; Pedalgeräusche.
  - Dateien: `…_48khz24bit.tar.bz2` mit 1,3 GB, `…_44.1khz16bit.tar.bz2` mit 466,1 MB, Ogg mit 74,4 MB.
- Eignung für den Boden: Aufnahmen mit gedämpften Saiten oder eine Boden-Impulsantwort werden nicht erwähnt. Allenfalls die Hammergeräusch-Releases taugen für eine grobe Abschätzung. Das Mikrofon sitzt dicht an den Saiten, es ist keine Boden-Impulsantwort.

**4. CCRMA REALSIMPLE: Körperantwort einer Gypsy-Gitarre**
- URL: https://ccrma.stanford.edu/realsimple/phys_mod_overview/Body_Resonator.html
- Lizenz **[nur Zusammenfassung]**: Laut Seite am 20.02.2008 unter Creative Commons Attribution 2.5 veröffentlicht (Lee, Smith). Einen Lizenztext gibt es dort nicht.
- Inhalt: Die gemessene Admittanz am Antriebspunkt wird per inverser FFT zur Impulsantwort. Online stehen nur PNG-Plots, **keine Datendatei**.

**5. TU Berlin: Richtcharakteristiken von 41 Instrumenten (Ackermann, Brinkmann, Weinzierl)**
- URL: https://arxiv.org/abs/2307.02110
- Lizenz **[wörtlich]**: "The data is available under the CC BY-SA 4.0 licence."
- Inhalt **[nur Zusammenfassung]**: Gemessen im reflexionsarmen Raum mit einem 32-Kanal-Kugelarray; Einzeltöne, Richtcharakteristiken und FIRs je Terzband; Formate SOFA, OpenDAFF und GLL.
- Das sind **Abstrahldaten, keine Körperadmittanz**. Die Instrumentenliste (ob Flügel oder Harfe dabei sind) und den endgültigen Datenort (DepositOnce) konnte ich nicht prüfen.

## Geprüft, aber nicht frei oder ohne Daten
- **AudioLabs Erlangen, Yamaha C3** (https://audiolabs-erlangen.de/resources/MIR/2023-piano-synth-lbd):
  - **[wörtlich]**: "we measured impulse responses of the soundboard of a Yamaha C3 grand piano."
  - Anregung über einen Kontaktlautsprecher an der Unterseite, Chirp von 10 s, Stereo-Kondensatormikrofone, vier Anregungspunkte, Saiten gespannt.
  - **Kein Download, keine Lizenzangabe** auf der Seite, nur Abbildungen. Allenfalls auf Anfrage bei den Autoren.
- **BiVib** (Zenodo 1213211): Lizenz **[wörtlich]** `"cc-by-nc-sa-4.0"`, also **nicht kommerziell**. Die Vibration wurde an der Tastatur gemessen, nicht am Boden.
- **Balázs Bank** (home.mit.bme.hu/~bank): Thesen und Papers, aber keine Boden-Impulsantwort als Datei gefunden.
- **Välimäki/Aalto:** Das Cembalo-Paper von 2004 nutzt gemessene Brücken-Impulsantworten, ein Datenrelease ist nicht gefunden.
- **OpenAIR (York):** reine Raumakustik, keine Instrumentenkörper.
- **Freesound:** keine Körper-Impulsantwort von Gitarre oder Klavier mit bestätigtem CC0 gefunden (nur über die Suchmaschine, nicht direkt in Freesound gefiltert).
- **Nur Papers, ohne Daten:**
  - Piano-Boden: Ege/Boutillon/Chaigne, arXiv 1212.2323, 1305.3057, 1210.5688; Giordano 1998.
  - Harfe: JCAA; Cello: UniBw München; Snare und Bendir.
  - Violine: Saitis et al. (10 Violinen, HAL), Malvermi et al. (7 Violinen).
  - NBody (1998): richtungsabhängige Impulsantworten von Gitarre, Mandoline und Violine; Ort und Lizenz unbekannt.

## Nicht durchsucht oder offen
- Figshare, KTH, IRCAM, LAM Paris und die Pianoteq-Community habe ich nicht gezielt abgefragt; nur die allgemeine Websuche lief.
- Ergänzungsmaterial zu JASA und JSV habe ich nicht einzeln geöffnet.
- Reichweite: etwa 15 Websuchen und 15 Seitenabrufe. Kein Treffer ist kein Beweis, gerade für Figshare und das Ergänzungsmaterial von Papers.

## Empfehlung zum Weiterprüfen
1. Bei Mores die Dateien `Method.pdf` und `Comments.pdf` lesen, um Abtastrate, Fensterlänge und Kanalbelegung zu klären. Das ginge ohne den Download der .mat-Datei.
2. Beim CNSM-Datensatz das Paper arXiv 2605.18305 auf den Messaufbau der Admittanz lesen.
3. Für einen Flügelboden bleibt realistisch nur eine Anfrage bei AudioLabs Erlangen (Berendes, Schwär, Schäfer, Müller) nach einer Freigabe der C3-Impulsantworten.
