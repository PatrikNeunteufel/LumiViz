> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: `decay` (T60 der ersten Stufe, Teilton 1). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche `decay`: T60 des Grundtons in der ersten Abklingstufe, Konzertflügel

**Auftrag:** Gesucht waren belegte Wertebereiche für `decay` je Taste (MIDI 21–108). `decay` ist die T60 von Teilton 1 in der schnellen ersten Abklingstufe („prompt sound“), gemessen am Flügel.

**Umrechnung überall:** T60 = 60 / r, mit r = Abklingrate in dB/s (Pegel 20·log10 der Amplitude). Ebenso T60 = 6,91·τ (τ = Zeitkonstante der Amplitude) und T60 = 6,91/σ. Bei T20 (Abfall um 20 dB) gilt T60-Äquivalent = 3·T20, aber nur, wenn der Abfall linear in dB verläuft. Am Klavier trifft das gerade nicht zu (Doppelabklingen). Der Faktor 3 ist deshalb eine Näherung.

## Reichweite der Suche

- **Websuchen (8):** „piano decay rate dB/s across keyboard prompt sound aftersound grand“, „Modelling the decay of piano sounds“ (Cheng), „Martin 1947 Decay rates of piano tones“, „piano fundamental decay time T60 … Steinway“, „Lehtonen piano decay times … Aalto“, „Bensa … simulation of piano string vibration“, „Bensa thèse 2003“, „initial decay rate … Martin 1947 … Fletcher Rossing“, „piano T60 per key … arXiv“, „Akustika 2024 Petrof Mistral“, „Chaigne Askenfelt 1994 numerical simulations pdf“, „Hundley Benioff Martin 1978 … abstract“, „decay times in the bass in the treble“.
- **Gelesen (Volltext):**
  - KTH „Five lectures“: Inhalt; bei Weinreich `weinreic.html`, `motion.html`, `strings.html`, `mistuned.html`; Wogram `decay.html` mit Bild Abb. 11; Conklin Inhaltsseite.
  - Cheng/Dixon/Mauch 2015 (PDF).
  - Nakamura 1989 (Scan, J-STAGE).
  - Jirásek/Peloušek 2024 (Akustika).
  - Burred, „The Acoustics of the Piano“ (Studienarbeit, PDF).
  - Abstract Sabine 1960 (JASA-Seite).
  - ASA Lay Language Paper Wogram (146th).
- **Gesperrt oder nicht erreichbar:**
  - Bezahlschranke: Martin 1947 (JASA 19, 535), Hundley/Benioff/Martin 1978 (JASA 64, 1303), Weinreich 1977, Chaigne & Askenfelt 1994 I/II, Fletcher & Rossing.
  - ResearchGate (403).
  - aaltodoc (403), dspace.vut.cz und HAL (Bot-Sperre „Anubis“).
  - IOA-PDFs Wogram und Baxandall/Brown/Gough (404).
  - BYU Anderson & Strong 2005 (403).
  - NTNU-Kopie von Chaigne & Askenfelt I und par.nsf.gov (Verbindung abgewiesen).
  - Die Abstract-PDF von Martin 1947 auf AIP leitet auf einen signierten Link um; nicht abgerufen.
- **Nicht gesucht (Auftrag):** die lokalen PDFs unter `quellen/` (Bank, Chabassier, Rigaud, Tan …).
- **Abrufe:** rund 36.
- **Lücken:**
  - Kein Treffer für eine Tabelle „T60 des Grundtons, erste Stufe, je Taste, Konzertflügel“.
  - Kein Treffer ist kein Beweis. Die wahrscheinlichsten Fundorte sind Hundley et al. 1978 und Martin 1947 (beide hinter Bezahlschranke) sowie die lokalen Chabassier-/Bank-Arbeiten.

## Quellen

### Q1: Cheng, Dixon, Mauch, „Modelling the decay of piano sounds“, ICASSP 2015 (Queen Mary University of London)

- **Zugang:** frei, Autorenseite https://webspace.eecs.qmul.ac.uk/s.e.dixon/pub/2015/ChengDixonMauch-ICASSP2015-Decay.pdf. Lizenz nicht angegeben (Autorenkopie, © IEEE vermutlich; nicht geprüft).
- **Volltext gesehen.** Text mit pdftotext gelesen, S. 4 als Bild gerendert.
- **Lokal abgelegt:** `quellen/Cheng_Dixon_Mauch_2015_ModellingDecayPianoSounds_ICASSP.pdf`, 423 748 Byte.
- **Instrument:**
  - „We use the piano sounds from the RWC Musical Instrument Sound Database [12]. The notes are played at three dynamics: loud (forte, f), moderate (mezzo, m) and soft (piano, p). Each clip consists of 88 isolated notes covering the whole compass of the piano.“ (PDF S. 3)
  - Hersteller und Modell nennt die Arbeit nicht. Dass es ein Flügel ist, steht nicht ausdrücklich da. Abschnitt 2 spricht allgemein von „a grand piano“.
- **Messfenster und Definition:**
  - „We use a 2s sound clip beginning at 0.2s“ (PDF S. 2).
  - Gemessen wird am Pegel „SdB = 20 log10(S)“ (S. 2). Die Rate a ist die Steigung von y(t) = at + b, in dB/s.
  - Bei Doppelabklingen: „the slope of the line which covers the larger part of the time duration is recorded as the decay rate“ (S. 3).
  - **Folge:** Gemessen wird nicht sauber die erste Stufe, sondern die überwiegende Steigung im Fenster 0,2–2,2 s. Bei einer frühen Knickstelle ist das eher der Nachklang.
- **Aussagen im Text:**
  - „high frequency partials decay faster. The spread of observed decay rates is large, and increases with frequency.“ (S. 4)
  - „some frequencies in the low range, around 80 Hz (MIDI index 41) and 150 Hz (MIDI index 50), exhibit particularly fast decay rates.“ (S. 4)
  - „dynamics have no significant effect on the decay rate.“ (Abstract, S. 1)
- **Zahlen aus Abb. 9 „the 1st partial“ abgelesen (Unsicherheit ±5 dB/s):**
  - Die Kurve beginnt erst bei MIDI ≈ 26. Unter 26 liegt für Teilton 1 nichts vor.
  - Die Achse heißt „Log frequency (MIDI note index)“. Für Teilton 1 entspricht das der Taste.

| MIDI | Rate (dB/s), abgelesen | T60 = 60/r |
|---|---|---|
| 26–40 | 0 … −5 (Einzelwerte bis −12 bei 31) | ≥ 12 s, bis „unauflösbar lang“ |
| 41 | ≈ −32 (Einbruch) | ≈ 1,9 s |
| 42–60 | −3 … −20, typisch ≈ −10; Einbrüche bis ≈ −30 bei 49 | 3–20 s, typisch ≈ 6 s; Minimum ≈ 2 s |
| 61–72 | −10 … −25; Einbruch ≈ −35 bei 70 | 2,4–6 s; Minimum ≈ 1,7 s |
| 72–92 | 0 … −15, typisch ≈ −8 | ≥ 4 s, typisch ≈ 7,5 s |
| 93–100 | −15 … −65 | 0,9–4 s |
| 101–108 | −25 … −85 | 0,7–2,4 s |

- **Anmerkungen zu den Ablesungen:**
  - Werte nahe 0 dB/s bedeuten nur, dass im 2-s-Fenster kein messbarer Abfall war. Als Obergrenze von T60 taugen sie nicht.
  - Abb. 8 (alle Teiltöne, forte) zeigt Raten bis ≈ −120 dB/s bei MIDI ≈ 115. Das sind höhere Teiltöne.

### Q2: Weinreich, „The coupled motion of piano strings“, in: Five lectures on the Acoustics of the Piano (KTH)

- **Zugang:** frei, https://www.speech.kth.se/music/5_lectures/weinreic/motion.html. Lizenz nicht angegeben (frei lesbar).
- **Volltext gesehen.**
- **Zitate:**
  - Zur Erststufe: „drops (in this case) at a rate of about 8 dB/sec“.
  - Zum Nachklang: „at less than one-quarter that rate“.
  - Versuchsaufbau: „only one of the three strings belonging to this note (Eb4) was allowed to sound.“
  - Bildunterschrift Abb. 1: „Typical decay of a piano tone as illustrated by the sound pressure level versus time (Eb3 = 311 Hz).“
- **Widerspruch in der Quelle:** Der Text nennt Eb4, die Bildunterschrift „Eb3 = 311 Hz“. 311 Hz ist Eb4 (MIDI 63).
- **Messgröße:** Gemessen ist der Gesamt-Schalldruckpegel, nicht der Teilton 1 allein. Es klang eine einzelne Saite eines Chors. Das Instrument nennt die Seite nicht mit Hersteller oder Modell.
- **Umrechnung:**
  - Prompt sound 8 dB/s → T60 ≈ 7,5 s.
  - Aftersound < 2 dB/s → T60 > 30 s.
- **Weitere Seite** `strings.html`: Klemmt man eine Saite des Chors mit einem Filzkeil ab, steigt der Pegel um „close to 20 dB“. Das zeigt die Größe der Kopplungseffekte. Eine Rate enthält die Seite nicht.

### Q3: Wogram, „The strings and the soundboard“, Five lectures (KTH), Abschnitt decay

- **Zugang:** frei, https://www.speech.kth.se/music/5_lectures/wogram/decay.html und Bild `images/93_11.jpg`.
- **Volltext gesehen.**
- **Instrument:** ein Klavier, kein Flügel: „The decay times for the notes on a small upright piano are plotted in Fig. 11.“
- **Messgröße:**
  - „measure the decay time at the beginning of the note, from the maximum level to -20 dB (T20)“.
  - „The early decay, the so-called prompt sound, is of major importance in judging the tone quality.“
  - Gemessen ist der Gesamtpegel, nicht der Grundton.
- **Zahlen im Text:**
  - F#4 und G4: „the corresponding T20-values as 3.5 and 0.7 s, respectively“.
  - Akkorde: „C3 - C4 - G4 (3.8, 1.7, 0.7 s) and D3 - D4 - A4 (3.4, 3.3, 2.3 s)“.
  - Streuung: „the decay times vary drastically among the notes.“
- **Aus Abb. 11 abgelesen (T20 in s, ±0,3 s; Achse ohne Einheit, laut Bildunterschrift T20 in s):**
  - A0–C2: 2,5 … 7,2 s (Spitze ≈ 7,2 bei etwa D1–E1).
  - C2–C4: 2 … 5 s.
  - C4–C5: 0,7 … 3,5 s.
  - C5–C6: 0,8 … 2 s.
  - C6–C8: 0,4 … 1 s.
- **Als T60-Näherung (×3, nur unter Linearitätsannahme):**
  - Bass 7,5–22 s.
  - Mitte 2–15 s.
  - C4–C5 2–10 s.
  - Diskant oberhalb C6 1,2–3 s.

### Q4: Sabine, „Decay Characteristics of Piano Tones“, J. Acoust. Soc. Am. 32 (11_Supplement), 1493 (1960)

- **Zugang:** nur Abstract (Tagungsabstract), https://pubs.aip.org/asa/jasa/article/32/11_Supplement/1493/742559/Decay-Characteristics-of-Piano-Tones. Bezahlschranke für das PDF; das Abstract ist frei.
- **Instrument:** ein „high-quality spinet piano“ (laut Abstract).
- **Zitate:**
  - „Decay rates of individual components varied from 1 to 100 db/sec“.
  - Zum Grundton: „the fundamental of a given string decayed at a considerably higher rate than the harmonics“ (dritte bis fünfte Oktave).
- **Umrechnung:** 1–100 dB/s → T60 60 s … 0,6 s. Die Spanne gilt für alle Teiltöne, nicht nur Teilton 1, und für ein Spinett.

### Q5: Martin, „Decay Rates of Piano Tones“, JASA 19 (1947) 535–541, DOI 10.1121/1.1916547

- **Zugang:** nur Metadaten und Bruchstücke über eine Suchmaschine. Kein Beleg.
- Laut diesen Bruchstücken: Klavier, Stutzflügel („baby grand“) und elektrisches Spinett; der Resonanzboden prägt besonders die frühe Abklingstufe.
- Zahlen nicht gesehen.

### Q6: Hundley, Benioff, Martin, „Factors contributing to the multiple rate of piano tone decay“, JASA 64 (1978) 1303–1309

- **Zugang:** Bezahlschranke. Die Angabe stammt aus dem Literaturverzeichnis von Q1 [2], dort im Volltext gesehen.
- Zahlen nicht gesehen.

### Q7: Jirásek, Peloušek, „Comparison of grand pianos Ant. Petrof and Mistral“, Akustika 47 (Okt. 2024) 37–53

- **Zugang:** frei, Lizenz CC BY-NC-ND 4.0 (laut Artikelseite https://www.journalakustika.com/index.php/akustika/article/view/164).
- **Volltext gesehen.**
- **Lokal abgelegt:** `quellen/Jirasek_Pelousek_2024_PetrofMistral_Akustika.pdf`, 7 066 242 Byte, von https://www.journalakustika.com/index.php/akustika/article/download/164/101.
- **Instrumente:** zwei Petrof-Flügel. Gemessen alle 88 Tasten, im reflexionsarmen Raum, mit Anschlagautomat („play-bench“).
- **Abklingen:**
  - Nur als 3D-Bilder der Hüllkurven (Abb. 12/13, PDF S. 11 = gedruckt S. 47). Daraus sind keine Raten ablesbar.
  - Textzitat: „the decay phase is also shortened for these tones starting at low dynamics, in the range of approx. 2 to 4 seconds“.
  - Gemessen ist die Hüllkurve des Gesamtpegels, nicht der Grundton.
- **Bewertung:** Für `decay` nicht verwertbar. Belegt nur, dass das Abklingen je Taste mit Sprüngen zwischen Nachbartasten streut.

### Q8: Nakamura, „Fundamental theory and computer simulation of the decay characteristics of piano sound“, J. Acoust. Soc. Jpn. (E) 10, 5 (1989) 289–297

- **Zugang:** frei über J-STAGE, Scan.
- **Volltext gesehen** (S. 1 und 8 als Bild).
- Reine Theorie und Simulation, keine Messwerte. Der Text schließt mit „The next step is to apply constants obtained by actual measurements“ (S. 296).
- Für Grenzen nicht verwertbar.

### Q9: Burred, „The Acoustics of the Piano“ (Studienarbeit, jjburred.com)

- **Zugang:** frei.
- **Volltext gesehen** (Textsuche).
- Nur qualitative Aussagen, ohne Zahlen. Etwa: „Double decay is especially noticeable in the middle register“; „In the highest notes, the decay is made up of just one continuous curve“ (gedruckt S. 29).
- Sekundärquelle, kein Beleg für Zahlen.

### Nicht als Beleg geführt

- Forenwerte (Piano World), Sound on Sound („over 40 seconds in the bass“) und GitHub-Pull-Requests mit dB/s-Tabellen: nur über Suchzusammenfassungen gesehen, kein Volltext. Zudem Gesamtnachklang, keine erste Stufe.
- Vermutung aus dem Gedächtnis, nicht geprüft: Hundley et al. 1978 enthalten eine Kurve der Raten von „prompt“ und „aftersound“ über die Tastatur eines Konzertflügels.

## Tabelle „Bereich je Register“ (T60 der ersten Stufe, Teilton 1, in s)

| Register / MIDI | min | max | typisch | Quelle | Zitat-Kurzform | Prüfstand |
|---|---|---|---|---|---|---|
| A0–C#1 / 21–25 | — | — | — | keine | Cheng Abb. 9 beginnt erst bei MIDI ≈ 26 | Lücke |
| D1–E2 / 26–40 | ≈ 5 (Einzelwert) | > 20 (Rate ≈ 0, im 2-s-Fenster nicht auflösbar) | ≥ 12 | Q1 Abb. 9 | „2s sound clip beginning at 0.2s“ | RWC-Klavier (Modell unbekannt), Teilton 1, f/m/p; aus Abb. abgelesen ±5 dB/s |
| F2–C4 / 41–60 | ≈ 2 (Einbrüche bei 41, 49) | ≈ 20 | ≈ 6 | Q1 Abb. 9 und Text | „around 80 Hz (MIDI index 41) … particularly fast decay“ | wie oben |
| C#4–C5 / 61–72 | ≈ 1,7 | ≈ 6 | ≈ 4 | Q1 Abb. 9 | — | wie oben |
| Eb4 / 63 (Einzelpunkt) | — | — | ≈ 7,5 (8 dB/s) | Q2 | „about 8 dB/sec“ | eine Saite des Chors, Gesamtpegel, Instrument ungenannt; Volltext |
| C#5–G#6 / 73–92 | ≈ 4 | > 20 (Rate ≈ 0) | ≈ 7,5 | Q1 Abb. 9 | — | wie Cheng |
| A6–E7 / 93–100 | ≈ 0,9 | ≈ 4 | ≈ 2 | Q1 Abb. 9 | „spread … increases with frequency“ | wie Cheng |
| F7–C8 / 101–108 | ≈ 0,7 | ≈ 2,4 | ≈ 1,2 | Q1 Abb. 9 | — | wie Cheng |
| ganze Tastatur (Gegenprobe Klavier) | ≈ 1,2 (3·T20 = 0,4) | ≈ 22 (3·T20 = 7,2) | — | Q3 Abb. 11 | „small upright piano“, T20 | Gesamtpegel, Klavier, ×3 nur Näherung |
| ganze Tastatur (Gegenprobe Spinett, alle Teiltöne) | 0,6 (100 dB/s) | 60 (1 dB/s) | — | Q4 | „1 to 100 db/sec“ | nur Abstract |

## Offene Punkte

1. **Kein Konzertflügel mit Modell und Messung von Teilton 1 in der ersten Stufe gefunden.**
   - Q1 ist die beste frei zugängliche Kurve über die ganze Tastatur. Das Instrument ist aber unbekannt.
   - Die „Rate“ ist die überwiegende Steigung im Fenster 0,2–2,2 s, keine reine erste Stufe.
   - Für die erste Stufe sind die Werte deshalb eher eine **Obergrenze von T60**: Die erste Stufe ist schneller oder gleich schnell.
2. **MIDI 21–25 ohne Beleg.**
3. **Bezahlschranken:** Hundley/Benioff/Martin 1978 und Martin 1947 wären die Primärquellen für getrennte Raten von „prompt sound“ und „aftersound“ über die Tastatur. Beschaffung über Bibliothek nötig.
4. **Lokale PDFs:** Chabassier RT-0425 / 2013 (Steinway D) und Bank 2000/2006 könnten gemessene Abklingzeiten einzelner Töne enthalten. Das prüft der andere Agent.
5. **Ablesegenauigkeit Q1:** ±5 dB/s. Bei kleinen Raten (< 5 dB/s) ist T60 deshalb praktisch unbestimmt, die Obergrenze bleibt offen.
6. **Einbrüche zwischen Nachbartasten:** Q1 Abb. 9, Q3 Abb. 11 und Q7 belegen Faktor 3–5 zwischen Nachbartasten (Q3: F#4 3,5 s gegen G4 0,7 s). Eine Grenze je Taste sollte also breit sein und **nicht glatt interpoliert** werden.

## Zwischenkopien und Dateien

- **WebFetch-Zwischenkopien:**
  - `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\webfetch-1793045314768-ofn8e9.pdf` (Cheng 2015)
  - `…\tool-results\webfetch-1793045387059-z13qd7.jpg` (Wogram Abb. 11)
  - `…\tool-results\webfetch-1793045472131-n73l6t.pdf` (Bank & Sujbert, DAFx 2004, P_089; nicht ausgewertet)
  - `…\tool-results\webfetch-1793045393116-fnl355.pdf` (Nakamura 1989)
  - `…\tool-results\webfetch-1793045684714-za8rgs.pdf` (Burred)
- **Gerenderte Bilder (Scratchpad):** `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\bild\` (`cheng-4.png`, `p1.png`, `nak-*.png`, `petrof-10/11.png`).
- **Heruntergeladen nach `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`** (per Freigabe; der Ordner ist laut `.gitignore:471` ausgeschlossen):

| Datei | Quelle (URL) | Größe | Zugang / Lizenz |
|---|---|---|---|
| `Cheng_Dixon_Mauch_2015_ModellingDecayPianoSounds_ICASSP.pdf` | https://webspace.eecs.qmul.ac.uk/s.e.dixon/pub/2015/ChengDixonMauch-ICASSP2015-Decay.pdf | 423 748 Byte | Autorenseite, frei |
| `Jirasek_Pelousek_2024_PetrofMistral_Akustika.pdf` | https://www.journalakustika.com/index.php/akustika/article/download/164/101 | 7 066 242 Byte | CC BY-NC-ND 4.0 |
