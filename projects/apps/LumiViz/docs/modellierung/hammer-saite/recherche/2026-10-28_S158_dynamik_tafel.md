# Recherche: gemessene Abstände der Dynamikstufen im Klavierspiel (S158, für DY2 a)

> Agentenbericht, unverändert abgelegt. Auftrag: Quelle für eine neue Dynamik-Tafel der Chopin-Loops.
> Werkzeuge: WebSearch, WebFetch. Datum: 2026-10-28.
> (Zusätzlich `curl` für Seiten und PDFs, `pdftotext` zum Lesen der geladenen PDFs; kein Browser.)

Zum Vergleich die heutige Tafel (dB der Anschlagstärke): ppp −30, pp −24, p −18, mp −15, mf −12,
f −6, ff 0; Akzent +6. Also p→f = 12 dB, pp→ff = 24 dB.

## Kurzfassung

**Messungen in dB je einzelnem Dynamikzeichen beim Klavier habe ich nicht gefunden.** Die
Chopin-Studien (Kosta u. a.) messen zwar je Zeichen, aber in Sone, die je Aufnahme normiert sind;
im Text stehen nur Rangfolgen und Streuungen, keine Mittelwerte je Zeichen. Wirklich in dB oder m/s
belegt sind nur der **Gesamtumfang** und die Geschwindigkeiten des Hammers bei **„piano“ und „forte“**
(KTH, Askenfelt und Jansson).

| Quelle | Größe | Abstand | Fundstelle | Stand |
|---|---|---|---|---|
| Askenfelt & Jansson (1990), KTH *Five lectures on the acoustics of the piano* | Pegel, pp bis ff | **pp→ff = 33 dB** | Kapitel „From touch to string vibration“, Seite *keybott*, Bildunterschrift Fig. 5 | belegt (Wortlaut) |
| Askenfelt & Jansson (1991), JASA 90, S. 2385, zitiert bei Goebl (2001) | Endgeschwindigkeit des Hammers | piano ≈ 1 m/s, forte ≈ 5 m/s → **p→f ≈ 14 dB** in der Geschwindigkeit (selbst gerechnet: 20·log10 5) | Goebl 2001, JASA 110, S. 564 | Zitat aus Sekundärquelle belegt; Askenfelt & Jansson 1991 selbst **nicht nachgelesen** |
| Goebl (2001), JASA 110, 563–572, Bösendorfer SE, 22 Pianisten, Chopin op. 38 und op. 10/3 | übliche Spanne der Hammergeschwindigkeit | 0,21 bis 4,26 m/s → **≈ 26 dB** (selbst gerechnet) | Fußnote 6 | belegt (Wortlaut) |
| Goebl (2001) | Hervorheben einer Stimme (Ballade, Versionen mit Betonung) | 1,01 → 1,28 m/s (**≈ +2,1 dB**), 0,84 → 1,12 m/s (**≈ +2,5 dB**) (selbst gerechnet) | Abschnitt Ergebnisse, Fig. 8, S. 567 | belegt (Wortlaut) |
| Palmer & Brown (1991), JASA 90, 60–66 | Spitzenamplitude gegen Hammergeschwindigkeit | **linear proportional** → Verdopplung der Geschwindigkeit ≈ +6 dB (abgeleitet, nicht im Text) | Abstract | Abstract belegt, Volltext nicht nachgelesen |
| Kosta, Bandtlow, Chew (2018), JNMR 47 (5) | Lautheit in Sone, je Aufnahme auf [0, 1] normiert, 44 Mazurkas | **kein Abstand in dB**; nur Rangfolge (Kendall τ) und SD des log-Verhältnisses je Übergang 0,20–0,46 | S. 7, S. 10–11 (Tab. 2), S. 14 (Abb. 3) | belegt (Autorenfassung) |
| MuseScore (Quelltext `dynamic.cpp`) | MIDI-Velocity je Zeichen | ppp 16, pp 33, p 49, mp 64, mf 80, f 96, ff 112, fff 126 | `DYN_LIST`, Zeilen 59–69 | **Konvention, keine Messung** |
| DLS-Kurve, nach einem Beitrag im MIDI.org-Forum | Velocity → dB | dB = 20·log10(127²/v²) = 40·log10(127/v) | Forenbeitrag Clemens Ladisch, 16.10.2022 | **Konvention**, Spezifikation selbst **nicht nachgelesen** |

**Was das für die Tafel heißt (meine Auswertung, keine Quelle):**
- Gemessen ist pp→ff ≈ 33 dB als Pegelspanne eines Flügels und p→f ≈ 14 dB in der Hammergeschwindigkeit.
  Die heutige Tafel (pp→ff 24 dB, p→f 12 dB) ist damit **nicht größer** als die gemessenen Spannen.
  Wenn sie trotzdem zu extrem klingt, liegt das wahrscheinlich nicht an den Abständen allein — zu prüfen
  ist (a) ob die dB auf die Hammergeschwindigkeit wirken und der Pegel im Modell steiler steigt als
  1 dB je dB (Filz wird hart, mehr Obertöne), und (b) ob ein Chopin-Nocturne den Umfang ausschöpft.
  Kosta u. a. zeigen, dass Pianisten Zeichen **relativ zum Umfeld** spielen und keine festen Stufen
  einhalten — das spricht eher für kleinere Abstände in einem lyrischen Stück.
- MuseScore mit DLS-Kurve ergäbe (selbst gerechnet, auf ff = 0 bezogen): ppp −33,8, pp −21,2, p −14,4,
  mp −9,7, mf −5,8, f −2,7, ff 0 dB; also p→f ≈ 11,7 dB — fast wie die heutige Tafel, aber mit kleinen
  Schritten oben (mf→f→ff je ≈ 3 dB). Das ist eine Konvention.
- Für den Akzent ist das einzige Maß, das ich gefunden habe, das gewollte Hervorheben einer Stimme
  (+2 bis +2,5 dB in der Hammergeschwindigkeit, Goebl 2001). Das ist kein Akzentzeichen, aber ein
  Hinweis, dass +6 dB eher viel ist.

---

## 1 Kosta, Bandtlow, Chew — Chopin-Mazurkas (Priorität 1)

### 1.1 Kosta, Bandtlow, Chew (2018): „Dynamics and Relativity: Practical Implications of Dynamic Markings in the Score“

- Ort: Journal of New Music Research 47 (5), 438–461, DOI 10.1080/09298215.2018.1486430 (Angaben aus
  WebSearch-Treffern, beim Verlag nicht geprüft).
- Gelesen: **akzeptierte Autorenfassung** `jnmr_kosta_acc.pdf` von der Webseite des Mitautors
  (https://webspace.maths.qmul.ac.uk/o.bandtlow/publ/jnmr_kosta_acc.pdf), per curl geladen, mit `pdftotext`
  gelesen. Seitenzahlen = gedruckte Seitenzahlen der Autorenfassung (nicht die der Zeitschrift).

**Was gemessen ist (Einheit):** Lautheit in Sone aus dem Audio (Pampalk, MA-Toolbox, `ma_sone`),
geglättet, **je Aufnahme auf [0, 1] normiert**. S. 7:

> „The sequence is then normalised to [0, 1] by dividing the values by the maximum loudness value of the particular recording.“

und gleich danach: „…focusing on relative rather than absolute changes.“ (S. 7). Die Umrechnung Phon → Sone
steht dort als Formel (3), S. 7: S = 2^((L−40)/10) für L ≥ 40 phon — eine Verdopplung der Sone sind
also +10 phon. Das wäre der Weg, ein Sone-Verhältnis in Pegel umzurechnen.

→ **Keine absoluten Pegel** (weder dB SPL noch absolute Sone) je Zeichen.

**Je Zeichen:** Je Aufnahme werden Mittel E(pp), E(p), E(mf), E(f), E(ff) gebildet (S. 10–11), aber nur
als **Rangfolge** gegen die erwartete Folge pp < p < mp < mf < f < ff ausgewertet (Kendall τ, Tabelle 2,
S. 11). Zahlenwerte der Mittel stehen im Text **nicht**. mp kommt nicht vor (S. 10):

> „It so happens that mp does not occur in the scores we consider in this study.“

Schluss der Autoren (S. 11):

> „As general conclusion, we cannot assign fixed thresholds for the different dynamic markings.“

**Übergänge zwischen Zeichen:** Abb. 2 (S. 13) zeigt das log-Verhältnis der Lautheit zweier
aufeinanderfolgender Zeichen; die Mittel sind nur als Grafik da, aus der Textebene nicht ablesbar
(**nicht nachgelesen**). Abb. 3 (S. 14) gibt die **mittlere Standardabweichung** dieses log-Verhältnisses
je Paar; die Basis des Logarithmus nennt der Text nicht. S. 14:

> „the five smallest SD values can be found for the sequences (mf, pp), (f, ff), (ff, p), (ff, pp), and (pp, ff) with corresponding values 0.2003, 0.2146, 0.2349, 0.2497, and 0.2535.“

Die Werte in Abb. 3 (aus der Textebene des PDF) liegen zwischen 0,2003 und 0,4646. Folgerung der
Autoren (S. 14): beim Wechsel von laut nach leise sind die Übergänge gleichmäßiger.

**Bewertung:** Belegt sind nur (a) die Rangfolge wird meist eingehalten, aber nicht immer (Tab. 2, Tab. 3);
(b) es gibt keine festen Schwellen; (c) die Streuung je Übergang ist groß. Bei natürlichem Logarithmus
wäre eine SD von 0,2–0,46 ein Faktor von etwa 1,2–1,6 in Sone, also grob ±3 bis ±7 phon — **die Basis ist
nicht belegt**. Einen Abstand p→f in dB gibt der Text **nicht** her.

### 1.2 Kosta, Ramírez, Bandtlow, Chew (2016): „Mapping between dynamic markings and performed loudness: A machine learning approach“

- Ort: Journal of Mathematics and Music (gelesen als eingereichte Fassung vom 15.05.2016,
  https://webspace.maths.qmul.ac.uk/o.bandtlow/publ/JMM.pdf, über pdftotext).
- Dieselbe Messung (Sone, je Aufnahme normiert). Abschnitt 2.1:

> „The final values are normalized by dividing each value with the largest one per recording“

- Bestand, Abschnitt 2.1.2: „pp occurs 63 times, p 234, mf 21, f 170, and ff 43 times“.
- Wert je Zeichen = Mittel aus dem Schlag des Zeichens und den zwei folgenden.
- Keine Tafel der Mittel je Zeichen im Text. Zu Op. 24 Nr. 3 (Abschnitt 3, Abb. 2):

> „in three out of the eight recordings, the p immediately following the mf is actually louder than the mf.“

### 1.3 Kosta, Bandtlow, Chew (2018): „MazurkaBL: Score-aligned loudness, beat, and expressive markings data for 2000 Chopin Mazurka recordings“ (TENOR 2018)

- Gelesen als https://webspace.maths.qmul.ac.uk/o.bandtlow/publ/12_Kosta_tenor18.pdf (pdftotext).
- Beschreibt den Datensatz: Lautheit in Sone je Schlag, Lage der Zeichen. **Keine Mittel je Zeichen**
  im Text. Der Datensatz liegt auf GitHub (`katkost/MazurkaBL`, laut GitHub-API vorhanden, keine Lizenz
  angegeben; Inhalt **nicht abgerufen**). Mit ihm ließen sich Verhältnisse je Zeichen innerhalb einer
  Aufnahme selbst ausrechnen und über Formel (3) in phon umrechnen.

## 2 Hammergeschwindigkeit, Velocity und Pegel (Priorität 2)

### 2.1 Goebl (2001): „Melody lead in piano performance: Expressive device or artifact?“

- JASA 110 (1), 563–572 (Kopfzeilen des PDF). Gelesen:
  https://iwk.mdw.ac.at/goebl/papers/Goebl_JASA2001_melodyLead.pdf (pdftotext). Bösendorfer SE290,
  22 Pianisten, Chopin Ballade op. 38 und Etude op. 10/3.
- S. 564, mit Verweis auf Askenfelt und Jansson 1991, S. 2385 (Weg der Taste bis zum Boden):

> „ranges from 25 ms (forte or 5-m/s final hammer velocity, FHV) to 160 ms (piano or 1-m/s FHV“

  → piano ≈ 1 m/s, forte ≈ 5 m/s; daraus selbst gerechnet 20·log10(5) ≈ **14 dB** Geschwindigkeit.
  Das ist eine Angabe als Beispielwerte, keine Verteilung.
- Fußnote 6 (Spanne beim Spiel):

> „The performing pianists typically produce IHV values between 30 (4.26 m/s) and about 600 (0.21 m/s)“

  → selbst gerechnet ≈ 26 dB in der Hammergeschwindigkeit.
- Hervorheben einer Stimme (Ballade, Takte 1–9, S. 567):

> „the emphasized voice was played louder than in the normal version (mean FHV 1.28 m/s vs 1.01 m/s)“

  und für die dritte Stimme: „played loudest at about FHV 1.12 m/s on average, with the melody somewhat
  attenuated (0.84 m/s)“. Selbst gerechnet: +2,1 dB bzw. +2,5 dB.

### 2.2 Palmer & Brown (1991): „Investigations in the amplitude of sounded piano tones“

- JASA 90, 60–66 (Angaben aus dem Literaturverzeichnis bei Goebl 2001 und Europe PMC, PMID 1880301).
  PubMed selbst war hinter einer Browser-Prüfung; Abstract über die Europe-PMC-Schnittstelle gelesen:

> „Maximum amplitudes over the duration of the sounded tones were linearly proportional to piano hammer velocities for a range of frequencies and hammer velocities.“

  → Bei echter Proportionalität bringt die Verdopplung der Geschwindigkeit +6,0 dB Spitzenpegel
  (abgeleitet, nicht im Abstract). Ob „linearly proportional“ eine Gerade durch null oder mit Versatz
  meint, steht nur im Volltext (**nicht nachgelesen**).

### 2.3 Goebl & Bresin (2003): „Measurement and reproduction accuracy of computer-controlled grand pianos“

- JASA 114 (4), Oktober 2003 (Kopfzeilen S. 2275–2279 im PDF gesehen; ganzer Seitenbereich nicht geprüft). Gelesen:
  https://iwk.mdw.ac.at/goebl/papers/Goebl-Bresin_JASA2003_reproAccuracy.pdf (pdftotext).
- Pegel als Spitzen-Effektivwert, kalibriert mit 94 dB bei 1 kHz (Fußnote 3). Die Beziehung
  MIDI-Velocity → Spitzenpegel ist nur als Grafik da (Fig. 7); im Text:

> „The higher the pitch, the louder the radiated sound at the same MIDI velocity.“

  Eine Steigung in dB je Velocity steht im Text **nicht**. Zur Disklavier-Wiedergabe: „soft tones were
  played back too loudly, and very loud tones too softly“ (S. 2279).

### 2.4 Repp (1993): „Some empirical observations on sound level properties of recorded piano tones“

- JASA 93, 1136–1144 (Europe PMC, PMID 8445121). Nur das Abstract gelesen (dort abgeschnitten).
  Es misst Spitzenpegel in dB bei verschiedenen Hammergeschwindigkeiten (Disklavier, Bösendorfer 290SE),
  nennt aber im Abstract keine Steigung je Velocity. Volltext **nicht nachgelesen**.

### 2.5 Repp (1999): „A microcosm of musical expression: II. Quantitative analysis of pianists' dynamics in the initial measures of Chopin's Etude in E major“

- JASA 105, 1972–1988 (Europe PMC, PMID 10089616). 115 Aufnahmen, Takte 1–5 von op. 10/3. Nur Abstract
  gelesen; es nennt keine dB-Werte. Der Volltext dürfte die Spanne der Melodie in dB enthalten
  (**nicht nachgelesen**, kein freies PDF gefunden).

### 2.6 Weitere, nur als Abstract gelesen

- Repp (1996), „The dynamics of expressive piano performance: Schumann's ‚Träumerei‘ revisited“,
  JASA 100, 641–650 (Europe PMC): MIDI-Velocity, keine dB im Abstract.
- Nakamura (1987), „The communication of dynamics between musicians and listeners through musical
  performance“, Perception & Psychophysics 41, 525–533 (Abstract auf link.springer.com):

> „the marks did not correspond to a fixed-intensity level. This suggested a context effect common to playing and listening.“

  Welche Instrumente und welche Pegel gemessen sind, steht im Abstract nicht (**nicht nachgelesen**).
- Jones & Friberg (2023), „Probing the underlying principles of dynamics in piano performances using a
  modelling approach“, Frontiers in Psychology, DOI 10.3389/fpsyg.2023.1269715 (Volltext gelesen):
  Disklavier mit gemessener Kurve Velocity → Pegel, aber ohne Dynamikzeichen (die Zeichen wurden aus
  den Melodien entfernt) und je Lied normiert; keine Zahlen je Zeichen.

## 3 Gesamtumfang eines Flügels (Priorität 3)

### 3.1 Askenfelt & Jansson (1990): „From touch to string vibration“

- In: A. Askenfelt (Hrsg.), *Five lectures on the acoustics of the piano*, Royal Swedish Academy of Music,
  1990 (Seminar vom 27.05.1988). Webfassung: https://www.speech.kth.se/music/5_lectures/askenflt/
  (Seiten per curl gelesen).
- Seite *keybott.html*, Bildunterschrift Fig. 5:

> „The dynamic span ranges from pp to ff, corresponding to 33 dB.“

  Gemessen an C4 (Abbildung zum Zeitverhalten von Tastenboden und Hammeranschlag), Spieler: ein
  ungeübter und ein Berufspianist. Ob Spitzenpegel oder anderer Pegel gemeint ist, steht nicht dabei.
- Seite *motions.html*: „In the forte example, the maximum hammer velocity is about 5 m/s (18 km/h), not
  very far from the highest velocity observed during the experiments.“
- Seite *stricont.html*: die Dauer des Kontakts schwankt „within a comfortably accessible dynamic range
  (p - ff)“ um etwa ±20 % gegenüber mf.
- **Fletcher & Rossing** (*The Physics of Musical Instruments*) und **Meyer** (*Acoustics and the
  Performance of Music*): **nicht erreicht**, keine Zahl mit Fundstelle gefunden.
- Nicht verwendet, weil ohne Quelle: Forenangaben „70 dB“ (mmdigest.com) und eine Folie der Univ. Illinois
  mit Lehrbuch-Richtwerten (pp ≈ 50, ff ≈ 90 dB) — beides keine Messung am Klavier.

## 4 Konventionen der Programme (Priorität 4 — keine Messungen)

- **MuseScore** (Quelltext, `src/engraving/dom/dynamic.cpp` im Zweig `main`, Tafel `DYN_LIST`,
  per curl von raw.githubusercontent.com gelesen): ppp 16, pp 33, p 49, mp 64, mf 80, f 96, ff 112,
  fff 126; fp 96 mit Änderung −47, sf/sfz/rfz 112 mit −18.
- **DLS / GM2** (nur aus dem MIDI.org-Forum, Beitrag Clemens Ladisch, 16.10.2022): DLS
  „dB = 20 × log₁₀ (127² / velocity²)“; zu GM2: „The velocity effect on volume is not defined“. Die
  Spezifikationen selbst **nicht nachgelesen**.
- Daraus selbst gerechnet (MuseScore-Werte durch DLS-Kurve, ff = 0 dB): ppp −33,8, pp −21,2, p −14,4,
  mp −9,7, mf −5,8, f −2,7, fff +2,0 dB.
- Sibelius, Finale: **nicht gesucht** (Zeit), keine Angaben.

## Reichweite der Suche

- WebSearch (Begriffe): „Kosta Bandtlow Chew "Dynamics and relativity" dynamic markings loudness“;
  „piano dynamic markings measured loudness sones pp p mf f ff recordings Chopin mazurkas“;
  „"Dynamics and relativity" Kosta Journal of New Music Research 2018 pdf sones markings“;
  „Repp 1999 "microcosm of musical expression" Chopin Etude …“; „Repp "A microcosm …" haskins pdf“;
  „Repp "Quantitative analysis of pianists' dynamics …" pdf“; „Nakamura 1987 "communication of dynamics …"“;
  „Goebl Bresin 2003 "Measurement and reproduction accuracy …"“; „Goebl 2001 "Melody lead …"“;
  „Palmer Brown 1991 "Investigations in the amplitude …"“; „average MIDI velocity per dynamic marking …
  Magaloff Chopin statistics“; „piano dynamic range pp ff dB … Fletcher Rossing Meyer“;
  „Hyon Kim Xavier Serra score-informed MIDI velocity estimation …“; „DLS General MIDI velocity …
  "40 log"“; „"grand piano" dynamic range pp to ff dB … doubling of hammer velocity“.
- Gelesen (Volltext): Kosta u. a. 2018 (JNMR, Autorenfassung), Kosta u. a. 2016 (JMM, eingereichte Fassung),
  Kosta u. a. 2018 (TENOR), Goebl 2001, Goebl & Bresin 2003, Goebl/Bresin/Galembo 2004 (ISMA, OFAI-Bericht
  oefai-tr-2004-02, nur Spanne 0,37–4,07 m/s der Proben, keine Dynamikzeichen), Jones & Friberg 2023,
  KTH *Five lectures* (Kapitel Askenfelt & Jansson, alle Unterseiten; Kapitel Hall, Seite *compare*),
  MuseScore `dynamic.cpp`, MIDI.org-Forumsfaden.
- Gelesen (nur Abstract, Europe PMC bzw. Springer): Palmer & Brown 1991, Repp 1993, Repp 1996, Repp 1998/1999
  (I–III), Nakamura 1987, Goebl/Bresin/Galembo 2005, Goebl/Bresin/Fujinaga 2014.
- **Nicht erreichbar:** QMRO (qmro.qmul.ac.uk; Handles 41323 und 30712, keine Antwort — damit auch
  Kostas Dissertation nicht); PubMed (Browser-Prüfung); labs.sonicfield.org (Name nicht auflösbar);
  haskinslabs.org (Liste ohne PDF-Verweise); Repp-PDFs unter continuum-hypothesis.com (Teil II/III: 404).
- **Nicht gesucht:** Fletcher & Rossing und Meyer im Volltext, Sibelius/Finale, KTH-Regelsystem
  (Director Musices) mit dB je Stufe, Datensätze mit Velocity je Zeichen (ASAP, ATEPP, Magaloff).

## Offene Punkte (was als Volltext fehlt)

1. **Kosta (2017), Dissertation** „Computational Modelling and Quantitative Analysis of Dynamics in
   Performed Music“ (QMUL) — enthält vermutlich Verteilungen je Zeichen; QMRO war nicht erreichbar.
2. **MazurkaBL-Daten** (GitHub `katkost/MazurkaBL`) — erlaubt, Verhältnisse je Zeichen selbst zu rechnen
   (Sone je Aufnahme, über S = 2^((L−40)/10) in phon). Das wäre die einzige Messung **je Zeichen** an
   Chopin; aber Lautheit, nicht Hammergeschwindigkeit, und Mazurkas, nicht Nocturnes.
3. **Repp (1999 II)** — dB-Spanne der Melodie in 115 Aufnahmen von op. 10/3; kein freies PDF gefunden.
4. **Askenfelt & Jansson (1991), JASA 90, 2383–2393** — Verteilung der Hammergeschwindigkeit je Stufe
   (pp–ff) am Original prüfen; hier nur über Goebl 2001 zitiert.
5. **Palmer & Brown (1991) Volltext** — ob die Proportionalität durch null geht (dann 6 dB je Verdopplung).
6. **Fletcher & Rossing**, Abschnitt Klavier — Gesamtumfang mit Seitenzahl.
