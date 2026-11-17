> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_06_einzeltoene_gezupfte_saiten` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 21:16–21:19, claude-sonnet-5-5, 36 Runden, success, 0.53 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202:** Der Bericht stützt sich nach eigener Angabe **ganz** auf Abrufwerkzeuge (`[Werkzeug]`,
> `[nur Zusammenfassung]`); der Lauf (Sonnet, knappes Budget) hat keine Seite selbst gelesen.
> - **RWC Instruments Database, Zenodo 17170844 — selbst über die Zenodo-API gelesen (2026-11-17):** Titel, Lizenz
>   **CC BY-NC 4.0**, Zugang „open“, `RWC-I.zip` 14,2 GB, md5 fb5789335fe68abdc09929618e9f0403, „covers 50 musical
>   instruments … three variations for each instrument“ — stimmt. **Die Instrumentenliste nennt die Seite nicht**; die
>   AIST-Seite `staff.aist.go.jp/m.goto/RWC-MDB/rwc-mdb-i.html` war auch jetzt nicht erreichbar (Zeitüberschreitung),
>   die Vorschau des Archivs bei Zenodo zeigt keine Dateiliste. Ob Harfe, Cembalo, Gitarre darin sind, bleibt offen.
> - **Alles Übrige nicht gegengelesen** (Freesound, Philharmonia, IRCAM, IDMT, NSynth, VCSL, Sonstige).
> - **Nacharbeit:** Auftrag `2026-11-17_26_einzeltoene_nacharbeit` der Werkbank.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_06_einzeltoene_gezupfte_saiten.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_06_einzeltoene_gezupfte_saiten/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.

# Bericht: Einzeltöne gezupfter Saiteninstrumente (Stufe 34k)

**Auftrag:** `auftraege/2026-11-16_06_einzeltoene_gezupfte_saiten.md` · **Datum:** 2026-11-16 · **Modell:** claude-sonnet-5-5

**Vorweg:** Der Lauf war knapp bemessen. **Alle Angaben unten stammen aus Abrufwerkzeugen (`WebFetch`, `WebSearch`), keine aus selbst gelesenem Wortlaut einer Quelle im PDF oder an der Seite selbst.** Die Marke `[Werkzeug]` heißt: das Werkzeug hat die Seite gelesen und die Angabe zitiert oder umgeformt, ich habe die Seite nicht selbst gesehen. `[nur Zusammenfassung]` heißt: nur aus einer Suchzusammenfassung. Nichts ist `[wörtlich]` im Sinn von CLAUDE.md. Die Lizenzen sind deshalb vor jeder Übernahme an der Quellseite zu prüfen.

**Reichweite:** 13 `WebSearch`, 14 `WebFetch`, keine Downloads, kein `curl`, keine PDFs gelesen. Nicht erreicht: `staff.aist.go.jp` (RWC-Wurzelseite und Instrumentenliste, beide Abrufe `ECONNREFUSED`), Freesound-Seiten mit Lizenzangabe (Login-Seite), IRCAM Forum (Anmeldung), Philharmonia-Originalseite im Detail (Dateiformat, Dynamik nicht lesbar), MUMS (keine Primärquelle gefunden). Der Auftrag war klar.

**Gelesen als Vorbedingung:** `2026-11-12_S195_ziele_aufnahmen.md` (Ziele und Lücken; Laute und Oud ohne Aufnahme im Archiv) und `Index_samples.md` (Archiv: Iowa-Gitarre, VCSL Harfen/Cembali, Mores, VSCO 2 CE ohne Harfe). Darauf wird aufgebaut; VCSL, Iowa, Mores werden nicht wiederholt.

## Kurzergebnis

**Es wurde keine Quelle gefunden, die für eines der fünf Instrumente „jede Taste, bei mehreren Dynamikstufen, trocken, mit bekanntem Abstand, offene Lizenz“ erfüllt.** Die beste Aussicht haben, in dieser Reihenfolge: (1) **RWC Musical Instrument Sound** (Harfe, Gitarre, Cembalo enthalten laut Suchzusammenfassung; Halbtonschritte über den ganzen Umfang, drei Stärken; **CC BY-NC 4.0**, 14,2 GB) — die Instrumentenliste ist **nicht** gelesen, das ist der erste Punkt zum Nachprüfen; (2) **Freesound GtrClass (quartertone)**, Gitarre je Saite und Bund, fünf Stärken, **Lizenz unbekannt**; (3) **Philharmonia**, Gitarre, CC BY-SA 3.0, aber MP3. Laute und Oud: **nichts Offenes gefunden.**

## Fundstellen

### 1. RWC Music Database — Musical Instrument Sound (Neuveröffentlichung 2026)
| Feld | Angabe | Marke |
|---|---|---|
| Quelle | Zenodo „RWC Instruments Database“, https://zenodo.org/records/17170844 | [Werkzeug] |
| Umfang | „covers 50 musical instruments“, rund drei Varianten je Instrument, rund 150 Aufführungen; „generally played individual sounds at half-tone intervals over the entire range of tones“; „three levels of dynamics (forte, mezzo, piano)“ je Spielart | [Werkzeug] |
| Welche Instrumente | Die Zenodo-Seite nennt **keine** Instrumente. Aus einer Suchzusammenfassung der AIST-Seite (https://staff.aist.go.jp/m.goto/RWC-MDB/rwc-mdb-i.html): Harfe (Nr. 19, drei Varianten), Cembalo (Nr. 03), klassische Nylongitarre, aber **keine Laute gefunden** | [nur Zusammenfassung] |
| Format | Original-DVD: rund 3.544 WAV, rund 29,1 GB; Abtastrate, Bit, Mono/Stereo auf der Zenodo-Seite **nicht** angegeben | [Werkzeug] |
| Aufnahme | Mikrofon, Abstand, Raum: **nicht gefunden** | — |
| Größe | RWC-I.zip 14,2 GB (md5 fb5789335fe68abdc09929618e9f0403) | [Werkzeug] |
| Lizenz | „Creative Commons Attribution Non Commercial 4.0 International“ (CC BY-NC 4.0); Zusatz: bei Nutzung die Urheber und Veröffentlichungen nennen | [Werkzeug] |
| Konto | Als „Open“ geführt; ob Anmeldung nötig, nicht geprüft | [Werkzeug] |

Reicht für Forschung wohl (Frage der Lizenz-Reichweite: NC). **Zu prüfen:** welche Harfen (Pedalharfe?), welche Gitarren, welche Cembali; ob wirklich jede Taste; Register je Cembalo.

### 2. Freesound — quartertone, „GtrClass“ (Yamaha Eterna, klassische Gitarre)
| Feld | Angabe | Marke |
|---|---|---|
| URL | https://freesound.org/people/quartertone/packs/11573/ (Oberwellen: Pack 11598) | [Werkzeug] |
| Umfang | Einzelschüsse, benannt „GtrClass-[Bund]f,[Saite]s([MIDI])~v[1–5]“ → je Saite und Bund, **fünf Stärken**; „N#“ = Bass-E-Saite abgestimmt tiefer; Gesamtzahl nicht angegeben (12 in der Vorschau) | [Werkzeug] |
| Aufnahme | „recorded with a CAD E100 microphone“; Abstand, Raum nicht angegeben | [Werkzeug] |
| Lizenz | **auf der abgerufenen Seite nicht angezeigt** | [Werkzeug] |
| Format | nicht angegeben | — |

Eine Gitarre — keine Streuung zwischen Instrumenten. Obergrenze „bis 19. Bund“ nicht geprüft.

### 3. Philharmonia Orchestra — Sound samples
| Feld | Angabe | Marke |
|---|---|---|
| URL | https://philharmonia.co.uk/resources/sound-samples/ ; Spiegel https://github.com/skratchdot/philharmonia-samples/ | [Werkzeug] |
| Umfang | Gitarre (auch Mandoline, Banjo) vorhanden; **Harfe nicht gelistet**, keine Laute/Oud/Cembalo; Dynamikstufen und Längen nicht beschrieben | [Werkzeug] |
| Format | je Instrument ein ZIP; Suchzusammenfassung: MP3, 64–96 kbit/s — **für Teiltonanalyse ungeeignet** | [nur Zusammenfassung] |
| Lizenz | „You are free to use these samples as you wish … The only restriction is they must not be sold or made available ‚as is‘“; Spiegel: „licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License“ | [Werkzeug] |

### 4. IRCAM SOL / OrchideaSOL / TinySOL
| Feld | Angabe | Marke |
|---|---|---|
| OrchideaSOL | https://zenodo.org/records/3686252 — 13265 WAV, 44,1 kHz mono 16 bit; 14 Instrumente (Tuba, Horn, Posaune, Trompete, Akkordeon, Kontrabass, Violine, Viola, Cello, Fagott, Klarinette, Flöte, Oboe, Altsaxophon); **weder Harfe noch Gitarre noch Laute** | [Werkzeug] |
| Lizenz | Audio „under the Ircam Forum License“, nur nach Anmeldung im IRCAM-Forum; Metadaten CC BY 4.0 | [Werkzeug] |
| TinySOL | https://zenodo.org/records/3685331 — 2913 WAV, 1,0 GB, gleiche 14 Instrumente, CC BY 4.0 | [Werkzeug] |
| Anmerkung | Eine Suchzusammenfassung der OrchideaSOL-Arbeit (arXiv 2007.00763) sagt, Harfe und Gitarre seien für das ursprüngliche SOL aufgenommen worden; das widerspricht der Liste der 14 nicht, bedeutet aber nur, dass sie **nicht im offenen Satz** liegen | [nur Zusammenfassung] |

→ Für diese Lücke **ungeeignet**.

### 5. IDMT-SMT-Guitar (Fraunhofer IDMT)
| Feld | Angabe | Marke |
|---|---|---|
| URL | https://zenodo.org/records/7544110 | [Werkzeug] |
| Umfang | Teil 2: 400 Einzelnoten (mono/poly) auf zwei Gitarren, je Ton eine Datei, 16 bit; Teil 1: rund 4.700 Noten, 24 bit, drei Gitarren; sieben Gitarren insgesamt, **fast sicher E-Gitarren** (Suchzusammenfassung: 5 elektrisch, 1 akustisch) | [Werkzeug] / [nur Zusammenfassung] |
| Format | WAV, 44,1 kHz, 1 Kanal; 1,3 GB | [Werkzeug] |
| Aufnahme | direkt am Tonabnehmer, in einem Fall Kondensatormikrofon; kein Raum | [Werkzeug] |
| Lizenz | „Creative Commons Attribution Non Commercial No Derivatives 4.0 International“ | [Werkzeug] |

Nicht klassisch, nicht je Bund geordnet → nur Randnutzen.

### 6. NSynth (Magenta)
https://magenta.tensorflow.org/datasets/nsynth — 305.979 Noten, Familie „guitar“ und „string“, MIDI 21–108, fünf Anschlagstärken (25–127), **16 kHz mono**, CC BY 4.0 `[Werkzeug]`. Quellen sind teils Klangbibliotheken, teils synthetisch; keine Harfe als Familie. 16 kHz verbietet jede Teiltonprüfung über 8 kHz → **ungeeignet**.

### 7. Versilian (VCSL, Gesamtliste)
https://versilian-studios.com/vcsl/ `[Werkzeug]`: CC0 („This collection is under a Creative Commons 0 (zero) license“), 5 GB. Plucked-Einträge: Concert Harp, Folk Harp, Strumstick, Dan Tranh, Cembali (English, Flemish, French, Italian, Unknown), Psalterium. **Keine Gitarre, keine Laute.** Nichts Neues gegenüber dem Archiv; keine weiteren Zupfinstrumente zu holen. SFZ-Liste (sfzinstruments.github.io/misc und /orchestra): keine Gitarre/Laute/Oud, Garritan-Harfen kommerziell `[Werkzeug]`.

### 8. Sonstige Hinweise
- **Sonatina Symphonic Orchestra** (Harfe „Sonatina Harp“ daraus abgeleitet): Lizenz „CC Sampling Plus 1.0“ (veraltet) — `[nur Zusammenfassung]`, Umfang der Harfe nicht geprüft.
- **bitKlavier Grand** (Princeton, CC BY 4.0): Klavierrahmen, keine Harfe — nicht relevant.
- **Freesound VSCO 2 CE Harfe** (KSHarp_*_mf.wav, CC0): gleiche Quelle wie VCSL, nichts Neues.
- **Freesound „Nylon Plucks“ (bangcorrupt 833157):** Cordoba, nur nichtkommerziell, Einzelsound, kein Satz `[nur Zusammenfassung]`.
- **Freesound Carlos_Vaquero 153963/153971:** Einzeltöne einer klassischen Gitarre, Lizenz unbekannt `[nur Zusammenfassung]`.
- **GuitarSet** (Zenodo 1422265), **GAPS** (klassische Gitarre, 14 h, Stücke, keine Einzeltöne), **Good-sounds** (Streicher/Blas, CC BY-NC): keine Einzeltöne gezupfter Saiten `[nur Zusammenfassung]`.
- **MUMS:** DVD-Satz (früher 3 CDs); nach einer Magazinzusammenfassung „jede Note des Umfangs“; Instrumentenliste, Abtastrate und Bezugsquelle nicht gefunden; nicht frei `[nur Zusammenfassung]`.
- Kommerziell, nicht offen (nur der Vollständigkeit halber): Realsamples German Harpsichord (je Register, Lautenzug), VSL Synchron Harpsichord (2×8′, 4′, Lautenzug), Orchestral Tools Cetra, Sonokinetic Oud.
- **ORD-CC32** (Kairoer Kongress 1932, arXiv 2506.14503): historische Aufnahmen, keine Einzeltöne `[Werkzeug]`.

## Ladeliste (nach Nutzen für LumiViz)

Nichts geladen. Alle Bezüge sind **Vorschläge**, bis die Lizenz geprüft ist.

| # | Name (Vorschlag) | URL | Größe | Lizenz (siehe oben) | wozu |
|---|---|---|---|---|---|
| 1 | `RWC_2003_Instrumente.zip` | https://zenodo.org/records/17170844 | 14,2 GB | CC BY-NC 4.0 | Harfe, Gitarre, Cembalo (zu prüfen), jede Halbtonstufe, f/mf/p |
| 2 | `quartertone_GtrClass.zip` (Freesound-Pack, Einzelabruf) | https://freesound.org/people/quartertone/packs/11573/ | unbekannt | **unbekannt**, Seite prüfen | Gitarre je Saite/Bund, 5 Stärken |
| 3 | `Philharmonia_Gitarre.zip` | https://philharmonia.co.uk/resources/sound-samples/ | unbekannt | CC BY-SA 3.0 (Spiegel) | zweite Gitarre; MP3 — nur grobe Lautheit |
| 4 | IDMT-SMT-Guitar | https://zenodo.org/records/7544110 | 1,3 GB | CC BY-NC-ND 4.0 | nur Randnutzen (E-Gitarren) |

OrchideaSOL: Anmeldung im IRCAM-Forum nötig (nur vermerkt, kein Konto angelegt) — für diese Instrumente ohne Nutzen.

## Reicht das für …?

- **Harfe:** Aus offenen Quellen **nicht belegt**. Bleiben VCSL (jede 3.–4. Taste, nur f/mf) plus **RWC**, falls dort eine Pedalharfe in Halbtonstufen steht (nicht geprüft; Mikrofonabstand unbekannt, Raum unbekannt). G3–A3 und die oberste Oktave: **unbekannt**, bis die RWC-Dateinamen vorliegen. pp und ff sind in RWC nicht vorgesehen (nur f, mf, p).
- **Gitarre:** **Wahrscheinlich ja für eine Gitarre** (GtrClass: je Saite und Bund, fünf Stärken), falls die Lizenz passt, dazu RWC und Philharmonia als weitere Instrumente (Streuung zwischen Instrumenten). Mikrofonabstand 1 m: **nirgends gefunden**; Bund 19: nicht geprüft.
- **Cembalo:** Weiter **nein** für Register einzeln, Instrument nach Bauer/Jahr und Mechanik-Geräusch. Einzige offene Zusatzquelle RWC (Register unbekannt). Kommerzielle Bibliotheken bieten Register und Lautenzug, sind aber nicht offen.
- **Laute:** **Nein** — nichts Offenes gefunden.
- **Oud:** **Nein** — nichts Offenes gefunden (nur kommerzielle Pakete).

## Nicht gefunden (Suchwege)

Suchbegriffe (je eine `WebSearch`, englisch): concert harp single notes chromatic CC0/CC BY; classical guitar dataset isolated notes every string fret multiple guitars Zenodo; OrchideaSOL instruments; Philharmonia harp guitar; RWC musical instrument sound; oud single notes dataset CC0; harpsichord stops 8′/4′/lute CC0; Freesound classical guitar chromatic CC0; Sonatina/MUMS; lute/theorbo/oud sample library free; IRCAM SOL harp guitar; IDMT-SMT-Guitar; MUMS. Seiten durchgesehen: siehe Fundstellen (jeweils nur die Seite selbst, keine Unterseiten, keine Dateilisten).

**Nicht durchgesehen:** Freesound-Suche nach Lizenz CC0 (Login), Dateilisten innerhalb RWC, VCSL-GitHub, Hochschul-Repositorien (nur durch die Zenodo-Suche gestreift), Pianobook, Karoryfer, Sonatina-Seite selbst, Lautenbau-Messdaten (außerhalb Auftrag), Einzeltöne Oud aus Hochschulprojekten, Harfenaufnahmen mit Mikrofonabstand aus der Akustikliteratur (Messreihen statt Samples).

## Was der Auftrag verlangt hat und nicht geliefert ist

- Lizenz **im Wortlaut mit URL der Lizenzangabe**: nur für RWC, TinySOL/OrchideaSOL, Philharmonia, IDMT, NSynth, VCSL zitiert, und auch dort nur über Werkzeug-Antworten.
- Mikrofonabstand, Mikrofon, Raum: für RWC und Philharmonia nicht gefunden; für GtrClass nur das Mikrofon.
- Laute, Oud, Cembalo-Register, Mechanikgeräusche: nichts.

## Was die LumiViz-Sitzung selbst nachprüfen muss

1. **RWC:** Instrumentenliste (Harfe: Pedalharfe? Cembali: welche Register? Gitarre: Nylon? Laute?), Abtastrate, Bit, Dateinamen je Ton; ob NC für LumiViz tragbar ist. Dazu die Quellseite öffnen: https://staff.aist.go.jp/m.goto/RWC-MDB/rwc-mdb-i.html (war hier nicht erreichbar).
2. **Freesound GtrClass:** Lizenzfeld an der Pack- und Einzelseite (Login-Seite verdeckte es), Stückzahl.
3. **Philharmonia:** Format der Dateien im ZIP (MP3-Angabe stammt aus einer Suchzusammenfassung).
4. Die Aussage, OrchideaSOL enthalte keine Harfe/Gitarre, stützt sich auf die Zenodo-Seite (14 Instrumente) — die Suchzusammenfassung zum Aufsatz sagt es anders; das Metadaten-CSV prüfen, falls das IRCAM-Forum ohnehin besucht wird.
