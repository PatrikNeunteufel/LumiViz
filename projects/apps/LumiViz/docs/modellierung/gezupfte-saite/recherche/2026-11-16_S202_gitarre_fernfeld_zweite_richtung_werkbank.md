> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_03_gitarre_fernfeld_zweite_richtung` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 20:36–20:47, claude-opus-5-5, 101 Runden, success, 3.31 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Quellen unter `…\cmake\quellen\gezupfte-saite\`):
> - **Zitate in der Textebene gefunden:** Hess 2013 (Loar, PCB 086D80, 2 cm, 500 Hz, Mittel aus acht Schlägen), Meyer in
>   Jansson 1983 („scarcely differed … parallel to the top“, PDF 55–59), Bank & Karjalainen 2010 (Gibson, E-Saite;
>   y = senkrecht, z = parallel; f₃ = 246 Hz), Woodhouse u. a. 2012 (beide Stellen), Traube 2004 („between a third and a
>   tenth“), Scherrer 2013 („Skilled performers can compensate“), Elie u. a. 2012 („roughly constant …“, „between 1 and
>   2“).
> - **Hess 2013 S. 2 am Seitenbild:** Gl. (1)–(3), a = 10,9 cm, „up to about 500 Hz“, „2 cm << a“, „r = 2 m >> a“ —
>   stimmt.
> - **Bank & Karjalainen 2010 Abb. 4 am Seitenbild selbst abgelesen:** Yyy (senkrecht) 100 Hz −44…−45, Spitze 240 Hz
>   −21, 1–3 kHz −45…−50, 3–10 kHz −48…−55 dB; Yyz 100 Hz ≈ −62 (Bericht −65), Spitze −41, darüber −60…−70; Yzz
>   (parallel) 100 Hz −58…−60, Spitze bei ≈ 280 Hz ≈ −53, 3–10 kHz −65…−75 dB. Die Tabelle des Berichts stimmt in ±3 dB.
>   Gemessene (gestrichelt) und Fit (durchgezogen) liegen in der Abbildung übereinander; abgelesen ist der Fit.
> - **Nicht gegengelesen:** Hess Abb. 1a (79 / 70 / 86 dB/N), Scherrer Tab. 2.1, Elie Abb. 7b / 7c (Modendichte,
>   Verlustfaktor), Jansson Abb. 12, die TU-Berlin-Angaben (`[Werkzeug]`).
> - **Nacharbeit:** die Lücken 1 (Fernfeld je Terz an einer klassischen Gitarre) und 2 (Yyy, Yzy einer klassischen
>   Gitarre) sind offen — Auftrag `2026-11-17_23_gitarre_nacharbeit` der Werkbank.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_03_gitarre_fernfeld_zweite_richtung.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_03_gitarre_fernfeld_zweite_richtung/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Abgerufene PDFs** liegen seit S202 nur noch unter `…\cmake\quellen\` (im Archiv der Werkbank entfernt; der Bericht nennt die Namen der Werkbank):
> - `2010_2307.pdf` → `quellen\gezupfte-saite\Bank_Karjalainen_2010_PassiveAdmittanceMatrix_DAFx10.pdf` (lag schon dort) — gleiche Datei wie aus Auftrag 01; schon vorhanden, nicht kopiert
> - `Mores_2021_SymmetryAsymmetryGuitarBracings.pdf` → `quellen\gezupfte-saite\Mores_2021_SymmetryAsymmetryGuitarBracings_DAGA2021.pdf`
> - `2006_Kma38Ocr.pdf` → `quellen\gezupfte-saite\Jansson_1983_FunctionConstructionQualityGuitar_KMA38.pdf` — Sammelband, Royal Swedish Academy of Music No. 38 (Hg. E. V. Jansson); Name der Werkbank: 2006_Kma38Ocr.pdf
> - `webfetch-1794858162368-lf36ei.pdf` → `quellen\gezupfte-saite\Hess_2013_FrequencyResponseGuitarModifications_SavartJ.pdf` — Savart Journal, Artikel 19
> - `2012_ElieetalJasa2012.pdf` → `quellen\gezupfte-saite\Elie_Gautier_David_2012_MacroParametersClassicalGuitars_JASA.pdf` — Fassung der Autoren (datiert 30.09.2012)
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_gitarre_fernfeld_zweite_richtung_werkbank/`: `seite_von_zeile.py`.

# Bericht: Klassische Gitarre — Fernfeld gegen Nahfeld, zweite Stegrichtung, Zupfort, über 5 kHz

- **Auftrag:** `auftraege/2026-11-16_03_gitarre_fernfeld_zweite_richtung.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5
- **Reichweite:** 17 Websuchen, 16 Abrufe (`WebFetch`), davon 6 erfolgreich mit Volltext (4 PDFs, 2 Seiten);
  4 lokale PDFs aus `…\cmake\quellen\gezupfte-saite\` per `pdftotext` ausgewertet; 5 Seiten als Bild gegengeprüft.
  **Nicht erreicht:** Perry 2015 Diss. (ORCA Cardiff, HTTP 403), Perry & Richardson 2013 (IoA, HTTP 404), alle
  IoA-Proceedings-PDFs von Richardson (404), Woodhouse 2004 a/b (Acta Acustica, kein freier Volltext gefunden),
  Lambourg & Chaigne 1993 (SMAC 93, nicht im Netz gefunden), Christensen & Vistisen 1980 (JASA, Spiegel leer),
  Hill/Richardson/Richardson 2004 (nicht gefunden). Keine Downloads (Auftrag: nur Ladeliste).
- **Aufgebaut wird auf** (nicht wiederholt): Mores 2021 (Methode, Mikrofonorte 10 cm über der Decke), Woodhouse u. a.
  2012 (Saitentabelle), Wright 1996 Tab. 5.4, Scherrer 2013 Abb. A.11.

**Kurzbefund:** Lücke 1 ist **nicht geschlossen** — kein gefundener, lesbarer Beleg liefert Schalldruck/Kraft einer
klassischen Gitarre in 1 m *und* im Nahfeld je Terz. Die Quelle, die das fast sicher enthält (Perry 2015, Cardiff:
reflexionsarm, Hammer am Steg, zwei Messkugeln, 80–2000 Hz), war gesperrt → Ladeliste Nr. 1. Lücke 2 ist **teilweise**
belegt: eine gemessene 2×2-Stegadmittanz (Bank 2010) — aber an einer **Stahlsaiten**-Gitarre; für die klassische
Gitarre gibt es eine Aussage von Meyer (1983), dass senkrechte und parallele Anregung am Steg „kaum“ verschiedene
Schallkurven ergeben, ohne Zahlen. Lücke 3 und 4 teilweise belegt (Zupfort-Spannen, Auslenkung je Dynamik;
Modendichte und Verlustfaktor nur bis 2,5 kHz).

---

## 1. Fernfeld gegen Nahfeld, Richtcharakteristik

### 1.1 Hess 2013 — Umrechnung Nahfeld → Fernfeld (Kolbenstrahler-Näherung), gemessen Schalldruck/Kraft

D. P. Hess, „Frequency Response Evaluation of Acoustic Guitar Modifications“, *Savart Journal* (2013), Artikel 19,
<https://www.savartjournal.org/articles/19/article.pdf> — gelesen als `webfetch-1794858162368-lf36ei.pdf`; S. 2–3
als Bild gegengeprüft.

| Angabe | Wert | Marke |
|---|---|---|
| Instrument | Stahlsaiten-Gitarre Loar LH-200 (kleiner Korpus, Decke massiv Fichte) — **keine klassische** | [wörtlich] S. 2 |
| Anregung | Mini-Impulshammer PCB 086D80 auf die Bassseite des Stegs; Saiten bedämpft | [wörtlich] S. 2 |
| Nahfeld-Mikrofone | **2 cm über der Decke**: eines 1 cm vom Steg, eines über dem Schallloch | [wörtlich] S. 2 |
| Raum | gewöhnlicher Raum, zeitselektive Impulsantwort (kein reflexionsarmer Raum) | [wörtlich] S. 2–3 |
| Gl. (1) | p_N = p_p + √(S/A) · p_a (p_p über Decke, p_a über Schallloch, S Schalllochfläche, A wirksame Deckenfläche) | [wörtlich] S. 2, am Bild geprüft |
| Gl. (2) | p_F = a/(4r) · p_N (Vollraum, a Radius der wirksamen Deckenfläche, r Abstand) | [wörtlich] S. 2, am Bild geprüft |
| Gültigkeit Gl. (3) | 2πfa/c < 1; mit a = 10,9 cm „bis etwa 500 Hz“; Nahabstand 2 cm ≪ a, Fernabstand r = 2 m ≫ a | [wörtlich] S. 2 |
| Messbereich | 70–270 Hz, 0,5 Hz Auflösung, Mittel aus 8 Schlägen | [wörtlich] S. 3 |
| Normierung | SPL/Kraft in dB/N, Fernfeld r = 2,0 m (Tab. 1) | [wörtlich] S. 3, Abb. 1a, Tab. 1 |
| SPL/Kraft in 2 m, 1. Resonanz 116 Hz | ≈ 79 dB/N | [aus Abbildung abgelesen] Abb. 1a, ±1 dB |
| SPL/Kraft in 2 m, Senke ≈ 150 Hz | ≈ 70 dB/N | [aus Abbildung abgelesen] Abb. 1a |
| SPL/Kraft in 2 m, 2. Resonanz 219 Hz | ≈ 86 dB/N | [aus Abbildung abgelesen] Abb. 1a |
| Faktor a/(4r) für r = 1 m, a = 0,109 m | 0,0273 → **−31,3 dB** von p_N nach p_F | [eigene Rechnung] |

**Belang für LumiViz:** Das ist der einzige gefundene, gegengeprüfte **Rechenweg** Nahfeld → Fernfeld an einer Gitarre.
Er gilt aber nur unterhalb ≈ 500 Hz und nur für einen Nahabstand **≪ a**; Mores misst in 10 cm, also etwa **in der
Größe von a** — die Voraussetzung ist dort verletzt [eigene Einschätzung]. Als Terzband-Übertragungsmaß 10 cm → 1 m
taugt er daher nicht ohne eigene Begründung.

### 1.2 Jansson / Meyer 1983 — Schallkurven im reflexionsarmen Raum, Anregung am Steg

*Function, Construction and Quality of the Guitar*, hg. E. V. Jansson, Royal Swedish Academy of Music Nr. 38 (1983),
<https://www.speech.kth.se/music/publications/kma/papers/kma38-ocr.pdf> — gelesen als
`webfetch-1794858106130-r9j95p.pdf` (OCR-Textebene, Buchstaben gesperrt).

| Angabe | Wert | Marke |
|---|---|---|
| Jansson: Schallkurve einer Gitarre im reflexionsarmen Raum, Kraft **senkrecht zur Decke** am Steg; die untersten fünf Spitzen fallen mit der Admittanz zusammen | Abb. 12 (Abstand nicht genannt) | [wörtlich] PDF-S. 45–46 |
| Meyer: Erreger Philips PR 9271 von oben auf den Steg, **sechs Mikrofone** rund um das Instrument im reflexionsarmen Raum, gleichgerichtet und gemittelt („extensive equalization of the directivity“) | Abstand nicht genannt | [wörtlich] PDF-S. 57, 74 |
| Meyer zur Richtung der Anregung: Ergebnisse mit senkrechter Anregung „scarcely differed from those obtained with an excitation parallel to the top“ | — | [wörtlich] PDF-S. 57 |
| Meyer: Hörversuchsaufnahmen mit Kunstkopf in **2 m** Abstand, 30° aus der Blickachse, mittelgroßes Studio | — | [wörtlich] PDF-S. 46–47 |

Keine absoluten Werte (Pa/N), kein Abstand der sechs Mikrofone, keine Richtwirkung je Band. Die Zahl des
Meyer-Satzes gilt nur für die richtungsgemittelte Schallkurve.

### 1.3 Perry & Richardson 2013 / Perry 2015 (Cardiff) — die eigentlich passende Quelle, nicht gelesen

| Angabe | Wert | Marke |
|---|---|---|
| Titel | I. A. Perry, B. E. Richardson, „Radiation efficiency and sound radiation fields of classical guitars“, *Proc. IoA* 35 Pt 1 (2013); Diss. I. A. Perry, Cardiff 2014/2015 | [nur Zusammenfassung] |
| Instrumente | drei klassische Gitarren (zwei Torres-Stil BR1, BR2; eine Fabrikgitarre MAL) | [nur Zusammenfassung] |
| Anregung | automatischer Impulshammer PCB 086E80 am Steg rechts der hohen E-Saite; reflexionsarmer Raum | [nur Zusammenfassung] |
| Messfeld | zwei konzentrische Messkugeln, 36 Azimut × 9 Elevation, Kugelfunktionen bis Ordnung 9 | [nur Zusammenfassung] |
| Bereich, Befund | 80–2000 Hz; Monopol dominiert unter ≈ 300 Hz, darüber Dipole; größte Abstrahlgrade 200–600 Hz | [nur Zusammenfassung] |

Daraus ließe sich — wenn die Diss. Kurven p/F auf beiden Kugeln zeigt — genau das gesuchte Übertragungsmaß je
Band bis 2 kHz ableiten. Messradien und Zahlen **nicht gelesen**.

### 1.4 Richtwirkung je Terzband — Datenbank TU Berlin (gespielt, nicht Pa/N)

D. Ackermann, F. Brinkmann, S. Weinzierl, „A Database with Directivities of Musical Instruments“, arXiv:2307.02110
(2023), <https://ar5iv.labs.arxiv.org/html/2307.02110>; Grundlage Shabtai u. a., JASA 141 (2017) 1246–1256,
doi:10.1121/1.4976071.

| Angabe | Wert | Marke |
|---|---|---|
| Gitarre in der Datenbank | „acoustic guitar (m)“ — Bauart (klassisch oder Stahlsaiten) **nicht genannt** | [Werkzeug] |
| Mikrofone | 32, Pentakis-Dodekaeder, Durchmesser 2,1 m (Radius ≈ 1,05 m) | [Werkzeug] |
| Raum | reflexionsarmer Raum TU Berlin, ≈ 1070 m³ | [Werkzeug] |
| Anregung | von Musikern gespielte Einzeltöne; Terz-Richtwirkung aus ff-Aufnahmen | [Werkzeug] |
| Terzbänder | 25 Hz – 20 kHz (Array-Untergrenze 63 Hz) | [Werkzeug] |
| Daten, Lizenz | <https://tubcloud.tu-berlin.de/s/8joeeK3fFingLgp>, Rohaufnahmen doi:10.14279/depositonce-5861.2; CC BY-SA 4.0 | [Werkzeug] |

Liefert **Richtwirkung je Terz in ≈ 1 m** (vorne/seitlich/hinten), aber keine Nahfeldwerte und keine Normierung auf
die Kraft. Bauart der Gitarre muss in den Metadaten geprüft werden.

### 1.5 Tabelle für Lücke 1 (was an Zahlen vorliegt)

| Quelle | Gitarre | Abstand / Winkel | Raum | Anregung | Normierung | Band | Zahlen |
|---|---|---|---|---|---|---|---|
| Hess 2013 | Stahlsaiten | 2 cm (nah), 2 m (fern, gerechnet) | Raum, zeitselektiv | Hammer Steg | dB/N | 70–270 Hz | 79/70/86 dB/N bei 116/150/219 Hz; Faktor a/(4r) |
| Jansson 1983 | klassisch (?) | nicht genannt | reflexionsarm | Kraft ⊥ Steg | relativ | bis ≈ 1–2 kHz | nur Kurve Abb. 12 |
| Meyer 1983 | klassisch | 6 Mikrofone, gemittelt | reflexionsarm | Erreger ⊥ Steg | relativ | bis ≈ 5 kHz (Kurven) | keine Zahlen gelesen |
| Perry 2013/2015 | 3 klassische | 2 Kugeln, 36×9 Richtungen | reflexionsarm | Hammer Steg | vermutl. Pa/N | 80–2000 Hz | **nicht gelesen** |
| Ackermann 2023 | „acoustic“ | ≈ 1,05 m, 32 Richtungen | reflexionsarm | gespielt, ff | keine Kraft | Terzen 25 Hz–20 kHz | Daten frei (nicht geladen) |

---

## 2. Zweite Richtung am Steg, Kreuzadmittanz

### 2.1 Bank 2010 — gemessene 2×2-Stegadmittanz (Stahlsaiten-Gitarre)

B. Bank, M. Karjalainen, „Passive admittance matrix modeling for guitar synthesis“, *Proc. DAFx-10*, Graz 2010,
<https://home.mit.bme.hu/~bank/publist/dafx10adm.pdf> — gelesen als `webfetch-1794857980670-b3j00w.pdf`; S. 6 als
Bild gegengeprüft.

| Angabe | Wert | Marke |
|---|---|---|
| Instrument, Ort | „acoustic guitar bridge (Gibson, from 1960's) near the lowest (E) string“ — **Stahlsaiten, nicht klassisch** | [wörtlich] S. 5, Abschn. 5 |
| Methode | Drahtbruch (wire-break) in beiden Richtungen, Miniatur-Beschleunigungsaufnehmer in beiden Richtungen → volle 2×2-Matrix der Impulsantworten | [wörtlich] S. 5 |
| **Achsen bei Bank** | y = **senkrecht** zur Decke, z = **parallel** zur Decke (umgekehrt zur LumiViz-Bezeichnung) | [wörtlich] S. 5 |
| Symmetrie | Yyz ≠ Yzy gemessen, für den Fit symmetrisch angenommen | [wörtlich] S. 5 |
| Darstellung | Abb. 4 (a) Yyy, (b) Yyz, (c) Yzz in dB (Bezug nicht genannt), 50 Hz – 20 kHz log. | [wörtlich] S. 6 |

Abgelesen aus Abb. 4 (Genauigkeit ±3 dB, dB-Bezug unbekannt, aber gemeinsam; Hauptspitze ≈ 230–250 Hz, Text nennt
„partial No. 3 with f3 = 246 Hz is around the main peak“) — in **LumiViz-Bezeichnung** (z = senkrecht, y = parallel):

| Band | Yzz (⊥, Bank „yy“) | Yzy (Kreuz, Bank „yz“) | Yyy (∥, Bank „zz“) | Yyy − Yzz | Yzy − Yzz | Marke |
|---|---|---|---|---|---|---|
| ≈ 100 Hz | ≈ −45 dB | ≈ −65 dB | ≈ −60 dB | ≈ −15 dB | ≈ −20 dB | [aus Abbildung abgelesen] |
| Hauptspitze ≈ 240 Hz | ≈ −21 dB | ≈ −41 dB | ≈ −53 dB (Spitze eher ≈ 280–300 Hz) | ≈ −30 dB | ≈ −20 dB | [aus Abbildung abgelesen] |
| 1–3 kHz | ≈ −45…−50 dB | ≈ −60…−70 dB | ≈ −60…−65 dB | ≈ −12…−15 dB | ≈ −15…−20 dB | [aus Abbildung abgelesen] |
| 3–10 kHz | ≈ −50…−55 dB | ≈ −60…−70 dB | ≈ −65…−70 dB | ≈ −12…−15 dB | ≈ −12…−15 dB | [aus Abbildung abgelesen] |

Lesart [eigene Einschätzung]: Die Parallelrichtung liegt an der Hauptspitze rund 30 dB unter der Senkrechten, im
oberen Bereich nur noch 10–15 dB — die Senkrechte verliert oberhalb der tiefen Korpusmoden schneller an Beweglichkeit.

### 2.2 Woodhouse u. a. 2012 (lokal) — wie Woodhouse die zweite Richtung modelliert

`Woodhouse_etal_2012_PerceptualThresholdsGuitarModels_ActaAcustica98.pdf`:

| Angabe | Marke |
|---|---|
| Jede Körpermode trägt vier Größen: Frequenz, Dämpfung, Amplitude am Koppelpunkt **und einen Winkel der Bewegungsrichtung** am Koppelpunkt, „so that both polarisations of string motion can be included“ (PDF-S. 3, Abschn. 2) | [wörtlich] |
| Kopplung über 2×2-Admittanzmatrizen am Steg: Y_coup⁻¹ = Y_body⁻¹ + Y_str⁻¹, Gl. (5) (PDF-S. 4) | [wörtlich], Formel nicht am Bild gegengeprüft |
| Drahtbruch „can be used to apply force in different controlled directions to give information on the modal angles“ (PDF-S. 6) | [wörtlich] |
| Werte der Modalwinkel: **nicht angegeben** | — |

Damit ist der Modellweg belegt (je Mode ein Winkel → Yyy, Yzy aus denselben Moden), die Winkel selbst stehen in
Woodhouse 2004 (nicht gelesen) oder sind unveröffentlicht.

### 2.3 Meyer 1983 — klassische Gitarre, Richtung der Anregung

Siehe 1.2: senkrechte und parallele Anregung am Steg ergaben „scarcely“ verschiedene, richtungsgemittelte
Schallkurven [wörtlich, KMA 38 PDF-S. 57]. Das betrifft die Abstrahlung je Kraft, nicht die Admittanz Yyy selbst;
keine Zahlen.

### 2.4 Weitere Fundstellen ohne gelesenen Volltext

| Quelle | Inhalt laut Fundstelle | Marke |
|---|---|---|
| Lambourg & Chaigne, „Measurements and modeling of the admittance matrix at the bridge in guitars“, SMAC 93, S. 448–453 | Titel nennt Admittanzmatrix am Gitarrensteg | [wörtlich] Literaturverzeichnis Bank 2010 [4] |
| Woodhouse, „Plucked guitar transients…“, Acta Acust. 90 (2004) 945–965; „On the synthesis of guitar plucks“, ebd. 928–944 | Drahtbruch-Technik; Bank zitiert [5] als Quelle der Methode | [wörtlich] Bank 2010; Inhalt [nur Zusammenfassung] |
| Rau & Smith, ISMA 2019 (Resonatorgitarre) | Hammer senkrecht und parallel zur Decke, quer zur Saite | [nur Zusammenfassung] |
| Rau, Maestre, Smith, Scavone, ISMA 2017 Paper 71 | E-Gitarre, Steg vertikal und horizontal | [nur Zusammenfassung] |

---

## 3. Zupfort, Zupfwinkel, Auslenkung je Dynamik

Lokal: `Scherrer_2013_AngleOfRelease_Diss_McGill.pdf`, `Traube_2004_TimbreClassicalGuitar_Diss_McGill.pdf`.

| Angabe | Wert | Quelle, Stelle | Marke |
|---|---|---|---|
| Übliche Zupfstelle | „between a third and a tenth of the string length (i.e. 3 to 20 cm)“ | Traube 2004, Abschn. 3.1, PDF-S. 45 | [wörtlich] |
| Sor (zitiert): rechte Hand ≈ 1/10 der Saitenlänge; „mellow“ bei 1/8 | — | Traube 2004, Abschn. 3.3, gedr. S. 29 (PDF-S. 49) | [wörtlich] |
| Tiefe Saiten werden weiter vom Steg gezupft als hohe | — | Traube 2004, gedr. S. 29 | [wörtlich] |
| „Normal“ ≈ 1/10 der offenen Saite; flautando = Saitenmitte; sul tasto 12.–14. Bund; ponticello so nah am Steg wie möglich | Taxonomie nach Biberian | Scherrer 2013, Abb. 3.7, gedr. S. 57 (PDF-S. 90) | [wörtlich] |
| Abstand Zeigefinger–Ringfinger am Steg | 2–3 cm (Zeigefinger am weitesten vom Steg) | Scherrer 2013, gedr. S. 58 (PDF-S. 91), nach Traube 2004 | [wörtlich] |
| Ausgleich bei gegriffenen Tönen | „Skilled performers can compensate … by moving the plucking finger“ (relativer Zupfort R) | Scherrer 2013, gedr. S. 58 | [wörtlich] |
| Gitarristen gleichen den Wechsel meist **nicht** aus; Hand bleibt in der Unterarmachse | — | Traube 2004, PDF-S. 156 | [wörtlich] |
| Auslenkung je Dynamik, Zupfort 16 cm | leise h = 0,8 mm; stark h = 4 mm (Steigung h/x0 = 5·10⁻³ bzw. 25·10⁻³) | Scherrer 2013, Tab. 2.1, gedr. S. 25 (PDF-S. 58), nach Pavlidou 1997 Abb. 6.21 S. 136 | [wörtlich]; Spalten durch h/x0 = 0,8/160 geprüft [eigene Rechnung] |
| Rückstellkraft je 1 cm Auslenkung entlang der Saite (L = 65 cm, T = 70 N) | Kurve F_T(x0/L), steigt zum Steg stark an | Scherrer 2013, Abb. 3.8, gedr. S. 58 | [wörtlich] Bildunterschrift; Werte nicht abgelesen |
| Zupfwinkel | Scherrer misst den Ablösewinkel (AOR) — schon belegt (Abb. A.11) | — | — |

**Nicht gefunden:** an Spielern gemessener Zupfort **je gegriffenem Bund oder Lage**; Kraft/Auslenkung pp…ff über
zwei Stufen hinaus.

---

## 4. Über 5 kHz

### Elie, Gautier, David 2012 — Modendichte und Verlustfaktor klassischer Gitarren

B. Elie, F. Gautier, B. David, „Macro parameters describing the mechanical behavior of classical guitars“, JASA (2012;
Band und Seiten nicht geprüft), Vorabfassung datiert 30.09.2012, Kopfzeile „The guitar in 4 features“, <https://members.loria.fr/BElie/papers/Elieetal_jasa2012.pdf> —
gelesen als `webfetch-1794858212120-wkbmia.pdf`; S. 8–9 als Bild geprüft. (Seitenzahlen der Vorabfassung; der
Journal-Druck kann abweichen.)

| Angabe | Wert | Marke |
|---|---|---|
| Messung | Steg an der E2-Saite, Hammer PCB 086E80 und Beschleunigungsaufnehmer am selben Punkt, Gitarre am Kopf hängend | [wörtlich] S. 8, Abschn. V.A |
| Modendichte ab ≈ 800 Hz | „roughly constant in the mid and the high-frequency ranges“ | [wörtlich] S. 8 |
| Modendichte einer Gitarre | ≈ 0,018–0,019 Hz⁻¹ (≈ 1 Mode je 53 Hz) zwischen ≈ 800 und 2200 Hz | [aus Abbildung abgelesen] Abb. 7b |
| Modaler Verlustfaktor | „set between 1 and 2 %“; Abb. 7c: ≈ 2 % bis 1 kHz, gegen 2,5 kHz ≈ 1,5–2 % | [wörtlich] S. 8; [aus Abbildung abgelesen] Abb. 7c |
| f30 (Beginn MOF > 30 %) | 465–910 Hz bei 12 Gitarren | [wörtlich] Tab. IV, S. 10 |
| Ersatzplatte | 0,3 m × 0,3 m, frei gelagert; p = 7,2·10⁻³ m², q = 0,6 m in n(ω) = pβ + q√(β/ω) | [wörtlich] S. 9, Formel am Bild geprüft |
| Messbereich | Abbildungen bis ≈ 2,5 kHz | [aus Abbildung abgelesen] |

Für **5–20 kHz** liefert die Arbeit nur die Aussage, dass die Dichte asymptotisch konstant ist (Plattenverhalten).
Bank 2010 (2.1) zeigt die gemessene Stegadmittanz einer Stahlsaiten-Gitarre bis 20 kHz: ohne Abfall der Mittelwerte
über 3–10 kHz, Yzz um −50…−55 dB [aus Abbildung abgelesen]. Abstrahlung über 5 kHz: nur die TU-Berlin-Terzdaten
(1.4), gespielt, nicht gelesen.

**Nicht gefunden:** Verlustfaktor oder Abklingzeit je Band über 5 kHz an einer klassischen Gitarre. Ein Kandidat ist
Elie u. a., „Analysis of bridge admittance of plucked string instruments in the high frequency range“
(HAL hal-00835970) [nur Zusammenfassung, nicht gelesen].

---

## Reicht das für Hörorte in 1 m und eine belegte zweite Stegrichtung?

**Lücke 1 — nein.** Belegt sind: (a) ein Rechenweg Nahfeld → Fernfeld (Hess 2013: p_F = a/(4r)·p_N, −31 dB für
r = 1 m, a = 0,109 m) — aber nur unter ≈ 500 Hz und nur für einen Nahabstand ≪ a, also **nicht** für Mores' 10 cm;
(b) eine Stahlsaiten-Kurve Schalldruck/Kraft in 2 m bis 270 Hz; (c) frei verfügbare Terz-Richtwirkungen in ≈ 1 m
(TU Berlin, gespielt, Gitarrenbauart offen). Ein Übertragungsmaß „10 cm → 1 m je Terz“ bliebe heute **Annahme**.
Die passende Quelle ist Perry 2015 (Ladeliste Nr. 1); sie ist nachzuholen, bevor eine Zahl festgelegt wird.

**Lücke 2 — teilweise.** Belegt ist eine gemessene volle 2×2-Matrix mit Kreuzterm (Bank 2010), daraus abgelesen
Yyy − Yzz ≈ −30 dB an der Hauptspitze und ≈ −12…−15 dB über 1 kHz, Yzy − Yzz ≈ −15…−20 dB — **an einer Stahlsaiten-Gitarre**,
Ablesegenauigkeit ±3 dB. Dazu der Modellweg von Woodhouse (je Mode ein Winkel) und Meyers Aussage, dass die
Anregungsrichtung die gemittelte Abstrahlung einer klassischen Gitarre kaum ändert. Für die klassische Gitarre selbst
bleibt das Verhältnis eine **Übertragung von einer anderen Bauart**, mit Vermerk. Woodhouse 2004 und Lambourg &
Chaigne 1993 würden das schließen (Ladeliste).

**Lücke 3 — Spannen belegt**, nicht je Bund: Zupfort 3–20 cm, „normal“ ≈ 1/10 L, Finger 2–3 cm gestaffelt, Auslenkung
0,8 mm (leise) bis 4 mm (stark) bei 16 cm. Zupfort je Lage bleibt Annahme; Traube berichtet, dass Spieler meist
nicht ausgleichen.

**Lücke 4 — bis 2,5 kHz belegt** (Modendichte ≈ 0,019 Hz⁻¹, η ≈ 1–2 %); darüber nur die qualitative Aussage
„konstant“ und eine Stahlsaiten-Admittanz bis 20 kHz. Verlustfaktor und Abstrahlung über 5 kHz bleiben Annahme.

---

## Ladeliste (nichts geladen)

| Nr. | Dateiname (Vorschlag) | URL | Größe | Lizenz / Vermerk | Wozu |
|---|---|---|---|---|---|
| 1 | `Perry_2015_SoundRadiationGuitars_PhD_Cardiff.pdf` | <https://orca.cardiff.ac.uk/id/eprint/70916/1/2015PerryIAphd.pdf> | unbekannt (403) | ORCA-Repositorium, Lizenz nicht gelesen | Lücke 1: p/F auf zwei Kugeln, Richtwirkung, 80–2000 Hz |
| 2 | `Perry_Richardson_2013_RadiationEfficiencyGuitars_ProcIoA35.pdf` | <https://www.ioa.org.uk/system/files/proceedings/ia_perry_be_richardson_radiation_efficiency_and_sound_radiation_fields_in_classical_guitars.pdf> | unbekannt (404 für das Werkzeug) | IoA-Proceedings | Lücke 1, Kurzfassung von Nr. 1 |
| 3 | `Ackermann_Brinkmann_Weinzierl_2023_DirectivityDatabase/` (nur Gitarre) | <https://tubcloud.tu-berlin.de/s/8joeeK3fFingLgp>, doi:10.14279/depositonce-5861.2 | unbekannt | CC BY-SA 4.0 [Werkzeug] | Lücke 1: Terz-Richtwirkung in ≈ 1 m; Bauart prüfen |
| 4 | `Bank_Karjalainen_2010_PassiveAdmittanceMatrixGuitar_DAFx10.pdf` | <https://home.mit.bme.hu/~bank/publist/dafx10adm.pdf> | 240 KB | Autorenseite | Lücke 2: 2×2-Matrix (liegt unter `abrufe/`) |
| 5 | `Hess_2013_FrequencyResponseGuitarModifications_Savart19.pdf` | <https://www.savartjournal.org/articles/19/article.pdf> | 358 KB | © Savart Journal | Lücke 1: Nahfeld→Fernfeld-Formel (liegt unter `abrufe/`) |
| 6 | `Elie_Gautier_David_2012_MacroParametersGuitars_JASA132_preprint.pdf` | <https://members.loria.fr/BElie/papers/Elieetal_jasa2012.pdf> | 328 KB | Autorenseite, Vorabfassung | Lücke 4 (liegt unter `abrufe/`) |
| 7 | `Jansson_1983_FunctionConstructionQualityGuitar_KMA38.pdf` | <https://www.speech.kth.se/music/publications/kma/papers/kma38-ocr.pdf> | 3 MB | KTH, frei | Meyer/Jansson, Abb. 12 und Meyer Abb. 4 als Kurven |
| 8 | `Woodhouse_2004_PluckedGuitarTransients_ActaAcust90.pdf` + `Woodhouse_2004_SynthesisGuitarPlucks_ActaAcust90.pdf` | Acta Acust. united Acust. 90 (2004) 945–965 und 928–944 | — | Verlag, kein freier Volltext gefunden | Lücke 2: Modalwinkel, gemessene Kreuzterme klassischer Gitarren |
| 9 | `Lambourg_Chaigne_1993_AdmittanceMatrixBridgeGuitars_SMAC93.pdf` | SMAC 93, S. 448–453 (KTH-Veröffentlichung) | — | nicht im Netz gefunden | Lücke 2 |
| 10 | `Elie_etal_2013_BridgeAdmittanceHighFrequency_HAL.pdf` | <https://hal.archives-ouvertes.fr/hal-00835970> | unbekannt | HAL | Lücke 4 |
| 11 | `Rau_Smith_2019_ResonatorGuitar_ISMA2019.pdf` | <https://ccrma.stanford.edu/~mrau/papers/ISMA2019_Rau_Smith.pdf> | unbekannt | Autorenseite | Lücke 2, Vergleich einer zweiten Bauart |

---

## Nicht gefunden (mit Suchwegen)

- **Schalldruck/Kraft klassische Gitarre in 1 m plus Nahfeld an derselben Gitarre:** Websuche nach Hill/Richardson,
  Perry, Christensen & Vistisen, Meyer 1983, „Pa/N“ + „1 m“ + anechoic, Nahfeld/Fernfeld-Vergleich; Abrufe ORCA (403),
  IoA (404, mehrfach), scispace-Spiegel (leer). Hill/Richardson/Richardson 2004 nicht einmal bibliografisch bestätigt.
- **Richtcharakteristik einer klassischen Gitarre je Band als Polardiagramm:** Suche „classical guitar directivity
  anechoic polar“; nur die TU-Berlin-Datenbank (Bauart offen) und Bader (Hamburg, 128 Mikrofone, Rückprojektion auf
  die Decke; Bader 2012 JASA zu Vihuelas und einer klassischen Gitarre) [nur Zusammenfassung].
- **Lai & Burgess, Boullosa, Torres & Boullosa, Elejabarrieta, Maestre (Gitarre):** nicht gezielt gesucht, das
  Budget ging an die Spuren mit freiem Volltext. Maestre u. a. haben Radiativität mit Kugelmessungen nur an Violinen
  gezeigt [nur Zusammenfassung].
- **Yyy, Yzy einer klassischen Gitarre:** Suche nach Woodhouse 2004, Lambourg & Chaigne, „parallel to the soundboard“,
  „in-plane bridge admittance“; nur Bank 2010 (Stahlsaiten) mit Zahlen. Lokal: Woodhouse 2012 und Woodhouse &
  Lynch-Aird 2019 durchsucht (Begriffe horizontal, parallel, in-plane, polaris*, 2×2) — kein Zahlenwert.
- **Zupfort je Bund an Spielern gemessen:** lokal Traube 2004, Scherrer 2013 (Begriffe „from the bridge“, „plucking
  position“); im Netz nicht gesucht. Pavlidou 1997 (Diss. Cardiff) wäre die Primärquelle für Kraft/Auslenkung.
- **Über 5 kHz an klassischer Gitarre:** Suche „modal density damping loss factor above 5 kHz“; nur Elie 2012 bis 2,5 kHz.

## Was nicht geliefert ist und was die LumiViz-Sitzung prüfen muss

- **Nicht geliefert:** ein Übertragungsmaß 10 cm → 1 m je Terz (Lücke 1); Yyy/Yzy einer **klassischen** Gitarre;
  Zupfort je Lage; Dämpfung/Abstrahlung über 5 kHz.
- **Prüfen:** (1) Die Ablesungen aus Bank Abb. 4 (±3 dB, dB-Bezug unbekannt, Achsenbezeichnung y/z vertauscht gegen
  LumiViz). (2) Ob Hess' Kolbenformel für 10 cm überhaupt anwendbar ist (ka < 1, Nahabstand ≪ a — bei Mores nicht
  erfüllt). (3) Bauart der „acoustic guitar“ in der TU-Berlin-Datenbank. (4) Die Seitenangaben „gedruckt“ bei Scherrer
  und Traube sind aus der Textebene erschlossen, die PDF-Seiten aus den Formfeeds der `pdftotext`-Ausgabe.
- **Hilfsdateien in diesem Ordner:** `*.txt` (pdftotext-Auszüge), `seiten/` (geprüfte Seitenbilder),
  `seite_von_zeile.py` (Zeile → PDF-Seite).
