# Gitarre: Modalfit am Archiv Mores 2021 (S199, 34k-5, KQ3 a)

> **Stand:** 2026-11-15 (Session 199, Fable 5.2) — Weg gebaut und an zwölf Gitarren geprüft; Vorbild ist g34
> (KQ23 a); Körper `asset/loop/instrument_bodies/classical_guitar.*` gebaut (KQ24 a, KQ25 a; §9).
> **Quellen:** Mores 2021 (Zenodo 4604577, CC BY 4.0; Rohdaten `…\cmake\samples\Klasse 2 - Gezupfte
> Saite\Mores_2021_Zenodo4604577\qualified_selected_impulses.mat`, Wechselplatte; Method, Comments, List unter
> `…\cmake\quellen\gezupfte-saite\`); Ducceschi, Russo & Webb 2026 (DAFx26, §3.2–3.5).
> **Proben:** `proben/2026-11-15_S199_mores_uebersicht.py`, `…_mores_fit.py`, `…_mores_abstrahlung.py`.

## 1 Die Daten

`qualified_selected_impulses(Gitarre 1–65, 144000, 6)`: je Gitarre drei Schläge von 1 s bei 48 kHz (Schlagort bass,
center, treble — an der Stegeinlage zwischen den Saiten 6/5, 4/3, 2/1), Kanäle Hammer, Aufnehmer treble, Aufnehmer
bass (an den Enden der Einlage, Method Abb. 3), Mikrofone upper east, treble, bass (10 cm über der Decke). Einheiten
nach `batch_extract_bridge_mobility_archive.m`: Hammer 92,90 mV/lbf, Aufnehmer 10,64 mV/g, Mikrofone 50 mV/Pa,
Vollausschlag ±10 V. Je Gitarre gibt das sechs Steg-Kurven (Schlag × Aufnehmer) und neun Schall-Kurven.

Übersicht über alle 65 (`…_mores_uebersicht.py`, Tabelle je Schlagort); an den zwölf Kandidaten aus §4: Kraftspitze
12–49 N, Zweitschlag unter −52 dB (außer g05: −24 / −27,5 dB, wie Method §3 sagt), der Hammer fällt bei 8–12 kHz um
10 dB, Rauschabstand der Aufnehmer 46–70 dB. Am Bild sehen die Kurven aus wie bei Mores beschrieben: A0 bei rund 100 Hz, T1 bei rund 200 Hz,
Einbruch bei 120–130 Hz, darüber dichter.

## 2 Der Weg bei Ducceschi 2026 und zwei Abweichungen

Ducceschi §3.4 (am Seitenbild gegengelesen, Gl. 28, 29): H1-Schätzer, Impulsantwort auf 3000 Abtastwerte (62,5 ms)
gekürzt, Spitzensuche in |H_acc| (20 Hz–10 kHz, 0,5 dB Prominenz), Dämpfung aus der −3-dB-Breite, reelle Residuen mit
Tikhonov, Umrechnung β = β_acc / (−ω_j²), überdämpfte Moden verworfen; nur das Paar der Diskantseite. Eine
Modentabelle veröffentlichen sie nicht.

Abweichungen hier, mit Grund:

1. **Volles Signal statt 62,5 ms.** Ein Fenster von 62,5 ms verbreitert jede Spitze auf rund 16 Hz; A0 (Q 20–45 bei
   100 Hz) ist 2–5 Hz breit. Hier: 1 s mit Exponentialfenster 3 1/s ab dem Schlag, die 3 1/s werden von der Dämpfung
   abgezogen; die Kraft nur 5 ms um den Schlag.
2. **Pole aus ESPRIT über alle sechs Steg-Kurven gemeinsam** (je Terzband, 60 Hz–5 kHz, komplex heruntergemischt)
   statt Spitzensuche und −3-dB-Breite je Kurve — Spitzen verschmelzen oberhalb von 400 Hz. Danach reelle Residuen
   je Kurve in der Mobilitätsform Y = Σ R iω / (ω_j² − ω² + 2iσ_jω) mit Massen- und Steifeterm, Gewicht 1/|Y|
   (Fehler in dB); Moden unter −25 dB gegen ihr Band und doppelte Pole fallen heraus.

## 3 Befund: freier Fit, starrer Steg, Steg mit Biegung

Fehler = Effektivwert von 20 lg |Fit / Messung| über alle sechs Steg-Kurven, Gitarre g36:

| Fit | 60–400 Hz | 400–1000 Hz | 1–2,5 kHz | 2,5–5 kHz |
|---|---|---|---|---|
| frei (Residuen je Kurve) | 1,1 dB | 1,5 dB | 2,4 dB | 2,8 dB |
| Steg als starrer Stab, φ(x) = a + b x | 1,4 dB | 1,7 dB | 4,6 dB | 10,2 dB |
| Steg mit Biegung, φ(x) = a + b x + c x² | 1,3 dB | 1,7 dB | 2,7 dB | 3,6 dB |

- **Der freie Fit ist kein Satz von Moden:** bei 24–51 % der Moden (zwölf Gitarren) haben die Residuen der beiden
  Antriebspaare (Schlag und Aufnehmer derselben Seite) kein gemeinsames positives Vorzeichen; R_bt² = R_bb · R_tt gilt
  über 1 kHz nur bei 21–61 % innerhalb 3 dB.
- **Eingeschränkt auf Normalmoden:** R(Schlag, Aufnehmer) = φ(x_Schlag) · φ(x_Aufnehmer), φ längs der Stegeinlage.
  Pole fest, a, b (, c) je Mode und die Restterme mit Gauss–Newton (`scipy.optimize.least_squares`, analytische
  Jacobi-Matrix; am starren Stab mit 60 und 400 Schritten gleich).
- **Der starre Stab trägt bis rund 1,2 kHz.** Darüber liegt die Antwort am mittleren Schlagort über dem Mittel der
  Enden (Plateau 1,5–2,8 kHz): der Steg biegt sich.
- **Mit Biegung** bleibt der eingeschränkte Fit 0,2–0,8 dB hinter dem freien. Jede Mode trägt Frequenz, Dämpfung und
  drei Formwerte; daraus folgt ihr Wert an jeder der sechs Saiten. Der Kipppunkt von KQ3 a („oberhalb von rund 1 kHz
  keine stabilen Moden“) greift nicht.

Annahmen (am Foto Method Abb. 3 geschätzt): Saitenabstand am Steg 11,6 mm (Schlagorte −23,2 / 0 / +23,2 mm), Aufnehmer
±36 mm (±3 mm).

## 4 Zwölf klassische Kandidaten

„classical Spanish“ nach `List_of_guitars_description.pdf`, ohne die historisch gestimmten (g46–g50, 415 Hz) und ohne
g07, g30 (Mores: schlechte Rohdaten), g33 (Decke eingesunken). Steg-Fit mit Biegung, Fehler in dB; Schall = dieselben
Moden an den drei Mikrofonen (je Mikrofon ein komplexes Gewicht je Mode für alle drei Schlagorte, §5), Spanne über
die drei Mikrofone.

| Gitarre | Bau, Decke / Boden | Mensur | Raum | Moden (über 1 kHz) | Steg 60–400 | 400–1000 | 1–2,5 k | 2,5–5 k | Schall 60–400 | Schall 400–1000 | Vermerk |
|---|---|---|---|---|---|---|---|---|---|---|---|
| **g34** | Manuel Contreras 1971, Zeder / Rio-Palisander | 662 mm | reflexionsarm | 78 (52) | 1,10 | 1,79 | 2,46 | 2,31 | 1,2–2,0 | 3,6–5,6 | 1670 g |
| g35 | Manuel López Bellido 1978, Zeder / Rio-Palisander | 652 mm | reflexionsarm | 76 (51) | 1,35 | 2,08 | 3,25 | 3,56 | 1,2–2,4 | 4,0–5,9 | 1605 g; Hammer −10 dB schon bei 7,7–8,3 kHz |
| g36 | José López Bellido 2001, Fichte / ind. Palisander | 663 mm | reflexionsarm | 75 (51) | 1,33 | 1,70 | 2,66 | 3,56 | 1,9–2,7 | 3,7–5,4 | 1535 g; Stufe bei 2,8 kHz |
| g08 | Manuel Bellido 2018 „Aurea“, – / Palisander | 654 mm | halbhallig | 76 (52) | 1,53 | 2,00 | 2,45 | 3,65 | 1,9–4,6 | 4,9–8,1 | neue Saiten; Stufe bei 2,8 kHz |
| g09 | Manuel Bellido 2018 „Aurea“, offener Kopf | 644 mm | halbhallig | 76 (53) | 1,32 | 2,46 | 3,25 | 2,22 | 3,6–4,2 | 6,0–10,6 | neue Saiten |
| g63 | Valerio Licari 2018, Fichte / ind. Palisander | 652 mm | halbhallig | 81 (55) | 1,59 | 3,18 | 2,22 | 5,22 | 2,8–4,0 | 3,9–5,6 | 1405 g |
| g05 | Gil de Avalle 2014 | 650 mm | halbhallig | 76 (52) | 1,33 | 1,92 | 1,94 | 2,97 | – | – | Zweitschlag −24 dB |
| g06 | Aaron Garcia 2017 | 650 mm | halbhallig | 78 (52) | 1,19 | 1,83 | 2,65 | 3,70 | – | – | zusätzliches Schallloch in der Zarge |
| g10 | Manuel Bellido 1999 | 655 mm | halbhallig | 84 (57) | 2,28 | 3,60 | 3,00 | 2,58 | – | – | zwei reparierte Risse |
| g27 | Do Santos | 650 mm | halbhallig | 74 (52) | 1,17 | 2,27 | 2,25 | 2,20 | 1,9–3,6 | 4,9–6,7 | 1580 g, Baujahr fehlt |
| g31 | Masaru Matano 1979, Zeder / Palisander | 657 mm | halbhallig | 76 (50) | 1,19 | 1,78 | 2,24 | 2,61 | 2,0–2,9 | 4,5–4,8 | 1620 g |
| g32 | Francisco Vega María, Zeder / Palisander | 650 mm | halbhallig | 80 (57) | 1,53 | 2,03 | 2,69 | 4,33 | – | – | 1635 g, Baujahr fehlt |

Die Güte der Rohdaten unterscheidet die Kandidaten kaum (Kraft, Zweitschlag, Rauschabstand in §1). Median der Güten
40–56. Tiefe Moden der drei aus dem reflexionsarmen Raum (Hz / Q / wirksame Masse an der Stegmitte):
g34 99,7 / 25 / 0,28 kg (A0), 198–201 / 16–24 / 0,08–0,12 kg (T1), 260 Hz kippt (Diskant gegen Bass);
g35 102 / 44 / 0,57 kg, 212 / 13 / 0,10 kg; g36 105 / 45 / 0,51 kg, 213 / 20 / 0,12 kg.

## 5 Gegenprobe am Schall

`…_mores_abstrahlung.py`: P/F am Mikrofon m bei Schlag h = Σ φ_j(x_h) G_jm (−ω²) / (ω_j² − ω² + 2iσ_jω) + Rest — die
Form von `antwortSpektrumExakt`, φ fest aus dem Stegfit, G komplex. Ein Satz G je Mikrofon muss alle drei Schlagorte
zugleich treffen. Ergebnis: bis 400 Hz 1,2–2,7 dB an den drei Gitarren aus dem reflexionsarmen Raum (halbhallig
1,9–4,6 dB), 400–1000 Hz 3,6–5,9 dB, darüber 5–13 dB bei allen acht gerechneten. Die Moden des Stegs erklären also den Schall im
Bereich der einzelnen Moden; darüber braucht die Abstrahlung einen anderen Weg (offen, eigene Vorlage — KQ3 a nennt
„Abstrahlgewichte aus dem statistischen Teil über Plattenwerte“, was nach der Probe S195 ohne Bauplan nichts
liefert). Die Mikrofone stehen 10 cm über der Decke: Nahfeld (Memory „Nahfeld vor Fernfeld“).

## 6 Gegenprobe der tiefen Moden gegen Wright 1996 Tab. 5.4

Wright (gedruckt S. 98, am Bild gelesen): Gitarre BR2, **Schallloch verschlossen**, Antrieb am Steg an der tiefen
E-Saite — T(1,1) 165 Hz, wirksame Masse 0,062 kg, Q 36; T(2,1) 204 Hz, 0,201 kg, Q 66. Bei offenem Schallloch teilt
sich T(1,1) in A0 und T1 (mit Anteil des Bodens); die Summe der Residuen φ² = 1/m bleibt. Verglichen wird deshalb
1 / Σ φ(x₆)² über die Moden 85–240 Hz an der Saite 6 (`proben/2026-11-15_S199_mores_gegen_wright.py`):

| Gitarre | wirksame Masse an Saite 6 | gegen 0,062 kg | Moden in 85–240 Hz (Hz / Q) |
|---|---|---|---|
| g34 | 0,053 kg | −15 % | 97,3 / 32 und 99,7 / 25; 198,3 / 16 und 201,4 / 24; 225,4 / 12 |
| g35 | 0,059 kg | −5 % | 102,4 / 44; 212,2 / 13; 225,3 / 27 |
| g36 | 0,062 kg | 0 % | 88,3 / 18; 105,1 / 45; 202,4 / 12; 213,0 / 20; 224,2 / 23 |
| g08 | 0,055 kg | −11 % | 108,9 / 41; 218,2 / 34; 221,0 / 58; 226,4 / 29 |
| g63 | 0,038 kg | −39 % | 104,0 / 44; 209,9 / 10 |

Die Größenordnung stimmt bei allen (das bestätigt auch die Umrechnung der Einheiten); g34 fällt nicht heraus. **Aber:**
bei g34 legt der Fit A0 und T1 je als Paar dicht beieinander (97,3 / 99,7 Hz und 198,3 / 201,4 Hz, Abstand kleiner
als die Breite). Ob das zwei Moden sind (Hals, Boden) oder ein geteilter Pol, ist nicht geprüft; g35 trägt dort je
eine Mode.

## 7 KQ23 a: g34, und die Paare sind geteilte Pole

**KQ23 a (Patrik):** Vorbild ist g34 (Manuel Contreras 1971, Zeder / Rio-Palisander, 662 mm, 1670 g, reflexionsarm
gemessen), nachdem die Paare geklärt sind. Prüfung (`proben/2026-11-15_S199_mores_g34_paare.py`): jedes Paar unter
300 Hz, dessen Abstand kleiner ist als die mittlere Breite, durch einen Pol ersetzt, Frequenz und Dämpfung
nachgestellt (Nelder–Mead auf den Fehler des freien Fits in 85–240 Hz). Gefunden drei Paare: 97,3 / 99,7 Hz,
198,3 / 201,4 Hz, 260,4 / 269,6 Hz.

| Fit an g34 | 85–240 Hz | 60–400 Hz |
|---|---|---|
| frei, mit Paaren | 0,99 dB | 1,03 dB |
| frei, ein Pol je Paar | 0,61 dB | 1,10 dB |
| Steg mit Biegung, mit Paaren | 1,06 dB | 1,10 dB |
| Steg mit Biegung, ein Pol je Paar | 0,82 dB | 1,23 dB |

Ein Pol je Paar trifft 85–240 Hz besser (die Pole sind nachgestellt): **A0 98,1 Hz, Q 24; T1 199,7 Hz, Q 12**; die
wirksame Masse an Saite 6 bleibt 0,054 kg (−13 % gegen Wright). Die Paare an A0 und T1 waren geteilte Pole aus dem
ESPRIT-Schritt; der Kipppunkt (über 2 dB schlechter) greift nicht. In 60–400 Hz wird der Fit um 0,07–0,13 dB
schlechter — das kommt vom dritten Paar (260 / 270 Hz), das mit zusammengelegt wurde; es ist nicht einzeln geprüft.

Folgen für den Weg: nach ESPRIT braucht es einen Schritt, der dichte Paare prüft und die Pole nachstellt — heute nur
unter 300 Hz probiert, darüber offen.

## 8 Abstrahlung in Terzbändern (Vorlage KQ24)

`proben/2026-11-15_S199_mores_abstrahlung_terz.py` an g34 mit dem Satz „ein Pol je Paar“ (75 Moden): Fehler des
Terzpegels (Leistungsmittel je Band) in dB, Effektivwert über Bänder und drei Schlagorte, in Klammern der größte.
„je Mode“ = nur die Moden mit ihrem komplexen Gewicht, wie eine Körperdatei sie trägt; „geteilt“ = bis zur Grenze das
Gewicht je Mode, darüber ein Betrag je Terzband mit gezogenem Vorzeichen (Samen 1), so gestellt, dass der Terzpegel
im Mittel über die drei Schlagorte stimmt.

| Mikrofon | Weg | 63–400 Hz | 500–1000 Hz | 1250–2500 Hz | 3150–5000 Hz |
|---|---|---|---|---|---|
| bass | je Mode | 4,1 (13,6) | 2,3 (5,3) | 2,7 (4,1) | 4,3 (6,4) |
| bass | geteilt bei 500 Hz | 3,8 (11,5) | 1,7 (3,3) | 2,2 (4,9) | 3,7 (5,8) |
| bass | geteilt bei 1 kHz | 4,1 (13,6) | 2,0 (6,0) | 1,4 (3,2) | 2,6 (4,2) |
| treble | je Mode | 2,4 (6,9) | 2,9 (5,8) | 5,9 (10,2) | 5,8 (8,0) |
| treble | geteilt bei 500 Hz | 2,5 (6,8) | 0,8 (1,8) | 2,1 (3,6) | 2,6 (4,2) |
| treble | geteilt bei 1 kHz | 2,4 (6,8) | 2,3 (4,5) | 1,9 (3,9) | 1,9 (3,0) |
| upper east | je Mode | 0,7 (1,5) | 3,2 (6,7) | 3,9 (7,9) | 4,1 (6,2) |
| upper east | geteilt bei 500 Hz | 0,7 (1,6) | 1,0 (2,0) | 2,3 (4,3) | 2,8 (3,8) |
| upper east | geteilt bei 1 kHz | 0,7 (1,5) | 3,2 (6,8) | 1,2 (3,3) | 2,4 (3,0) |

Die großen Werte unter 400 Hz an den Mikrofonen über dem Steg stammen aus den Bändern 63 und 79 Hz (unter der
tiefsten Saite, Pegel rund 30 dB unter der Spitze; dort trägt im Fit der Restterm). Die Mikrofone bass und treble
unterscheiden sich im Terzpegel um höchstens ±0,7 dB im Mittel je Bereich (Streuung 0,6–2,6 dB), upper east weicht um
−3,7 … +4,3 dB ab.

## 9 KQ24 a, KQ25 a: der Körper

**KQ24 a (Patrik):** Abstrahlung gemessen und geteilt, Hörorte `left` = Mikrofon bass, `right` = Mikrofon treble. **KQ25 a (Patrik):** `.lvinstbody` Fassung 4 mit `origin: measurement`, Beschreibung `.lvinstbodyfit`.

`asset/loop/instrument_bodies/make_gitarre.py` fasst den Weg zusammen (Kurven, Pole, Paare, Steg, Abstrahlung) und schreibt Beschreibung und Körper; die Prüfzahlen stehen in der Beschreibung unter `check`. Lauf vom 15.11.2026 (205 s): 75 Moden bis 5 kHz, 52 über 1 kHz; Fehler über die sechs Stegkurven frei 0,91 / 1,39 / 1,98 / 1,55 dB, mit Normalmoden 1,01 / 1,77 / 2,45 / 2,31 dB; Paare (Fehler im Band mit Paar → mit einem Pol): 97,3 / 99,7 Hz 0,95 → 0,93 dB (98,0 Hz, Q 23), 198,3 / 201,4 Hz 1,07 → 0,50 dB (200,1 Hz, Q 13), 260,4 / 269,6 Hz 0,67 → 0,75 dB (260,1 Hz, Q 29; innerhalb der Regel „höchstens 0,2 dB schlechter“); wirksame Masse an Saite 6 über 85–240 Hz 0,049 kg (−21 % gegen Wright). Abstrahlung, Fehler des Terzpegels ab dem Band 100 Hz: Grenze 500 Hz 1,84 dB, Grenze 1 kHz 1,62 dB — gewählt 1 kHz; je Bereich `left` 0,65 / 2,14 / 1,45 / 2,52 dB, `right` 1,01 / 2,10 / 1,87 / 2,07 dB (100–400 Hz / 500–1000 Hz / 1250–2500 Hz / 3150–5000 Hz).

Die tiefen Moden im Körper (Hz / Q / φ an den Saiten 6 … 1 in 1/√kg): 79,0 / 20 / 0,54 … 0,57; 98,0 / 23 / 1,53 … 1,44; 200,1 / 13 / 3,78 … 3,19; 225,4 / 12 / 1,94 – 4,60 – 1,64 (in der Mitte am größten); 260,1 / 29 / 0,61 … −3,37 (kippt).

## 10 Offen

- Das Paar 260 / 270 Hz an g34 einzeln prüfen; dichte Paare über 300 Hz; Pole nach ESPRIT allgemein nachstellen.
- Der Bereich über 5 kHz: erledigt mit KQ26 a (§11). Offen daraus: im Band 3–4 kHz klingt der Körper schneller ab als gemessen; die Dichte des statistischen Teils in 34k-6 gegen die Aufnahmen prüfen.
- Nahfeld 10 cm gegen 1 m bei den anderen Körpern — in 34k-6 gegen die Aufnahmen prüfen.
- Die zweite Richtung am Steg (parallel zur Decke) misst Mores nicht — Annahme mit Vermerk (KQ3 a).
- Gegenprobe der tiefen Moden gegen Wright 1996 Tab. 5.4 und die Streuung nach Woodhouse 2012.
- Die Stufe bei 2,7–2,9 kHz an g08 und g36 (Aufnehmer oder Steg?).

## 11 Über 5 kHz (KQ26 a)

Proben: `proben/2026-11-15_S199_mores_ueber_5khz.py` (Anregung, Störabstand, Pegel), `_mores_statistik.py` (die Fassungen A, B, C gegen die Messung), `_mores_fit_bis_10khz.py` (Fassung D, 646 s).

**Was die Messung trägt.** Je Terzband 5 / 6,3 / 8 / 10 / 12,7 / 16 kHz: Hammerkraft gegen 1 kHz −2,4 / −4 / −6,5 / −10,5 / −17 / −24 dB; Störabstand an den Mikrofonen (150 ms nach dem Schlag gegen die letzten 150 ms der Sekunde) 22–30 / 18–27 / 15–22 / 11–16 / 5–10 / 1–4 dB, an den Aufnehmern rund 8 dB weniger. Die Messung trägt bis rund 10 kHz; der Anstieg der Kurven bei 16–20 kHz ist Rauschen durch eine fallende Kraft. Der Terzpegel von p / F bleibt von 2 bis 10 kHz flach (−2 … +3 dB re 1 Pa/N). Das Abklingen an den Mikrofonen (Schroeder-Kurve je Terzband, −5 … −25 dB, Rauschen abgezogen; α in 1/s, Energie ~ exp(−α t)) bei 2,5 / 3,15 / 4 / 5 / 6,3 / 8 / 10 kHz: 177 / 164 / 203 / 211 / 211 / 231 / 247. Die Schwankung des Pegels über 6–10 kHz (Streuung von 20 lg |p / F| auf dem 1-Hz-Raster) ist 5,5 / 5,4 dB (`left` / `right`) — der Wert eines dichten Feldes überlappender Moden (5,6 dB).

**Vier Fassungen** (Terzpegel Modell − Messung bei 6,3 / 8 / 10 kHz, Modell im Mittel über die sechs Saiten, Messung im Mittel über die drei Schläge):

| Fassung | `left` dB | `right` dB | α 1/s | Schwankung dB | Moden |
|---|---|---|---|---|---|
| A heute: die Ausläufer der 75 Moden | −3,6 / −4,4 / −7,7 | −3,7 / −3,0 / −5,3 | kein Abklingen | 0,5 / 0,4 | 75 |
| B statistisch nach `schaetze` (Band 2,5–5 kHz, Gesetz an die Pole angepasst: α = 252) | +10,4 / +12,7 / +11,8 | +9,6 / +13,5 / +13,6 | 244 / 238 / 242 | 6,9 / 7,1 | 236 |
| C statistisch, Größen an der Messung | −0,4 / +1,2 / −0,8 | −1,4 / +1,1 / +0,3 | 211 / 218 / 240 | 5,1 / 5,0 | 236 |
| C mit dreifacher Dichte | −0,2 / +0,9 / −0,7 | −1,0 / +1,6 / −0,6 | 238 / 223 / 238 | 5,2 / 6,0 | 538 |
| D Modalfit bis 10 kHz | nicht gerechnet | nicht gerechnet | — | — | 107 |

- **A:** Über der letzten Mode strebt jeder Summand gegen φ · G; die Summe ist ein Sockel ohne Frequenzgang, im Zeitbereich ein Stoß bei t = 0. Beim Schlag `treble` am Hörort `left` liegt er 12–15 dB unter der Messung.
- **B:** Die Gegenprobe im eigenen Band — die gefitteten Moden 2,5–5 kHz durch gezogene mit den geschätzten Größen ersetzt — liegt 2–9,5 dB über der Messung. Ursachen: Formwert und Abstrahlgewicht sind gegenläufig (das Gewicht ist je Terzband an den Pegel gestellt; rms(φ) · rms(G) liegt 2,6 dB über dem mit 1/σ gewichteten rms(φ · G)), und das relativ angepasste Gesetz (α = 252 1/s) liegt unter der Dämpfung der Pole (Median je Terzband 370–670 1/s, harmonisches Mittel 402 1/s). Die Regel `schaetze` setzt gerechnete Moden voraus.
- **C:** α(f) = a f² + b f + c mit a, b, c ≥ 0 an das gemessene Abklingen 2,5–10 kHz (relativ, kleinste Quadrate): a = 0, b = 0,0105, c = 147. Dichte (0,0104 Moden je Hz, 26 Pole im Band) und Streuung am Steg (1,12 / 2,15 / 2,78 / 2,78 / 2,15 / 1,04 1/√kg an den Saiten 6 … 1) wie `schaetze`; der Betrag der Abstrahlung je Hörort in acht Schritten so gestellt, dass der Terzpegel 6,3 / 8 / 10 kHz im Mittel stimmt (0,0029 / 0,0023 √kg/m²). Bei 12,7 und 16 kHz liegt das Modell +2,0 / +3,7 dB (`left`) und −0,2 / +0,6 dB (`right`) gegen eine Messung, die dort kaum über dem Rauschen liegt — kein Beleg.
- **D:** 107 Pole, davon 33 über 5 kHz; Abstand / Breite / Überlappung in 2,5–5 / 5–7 / 7–10 kHz: 100 / 107 / 1,07 — 133 / 161 / 1,21 — 167 / 234 / 1,40. Die Zahl der Pole je Hz fällt, die Modendichte einer Platte nicht: es sind Ersatzpole. Fehler über die sechs Stegkurven frei 0,84 / 1,35 / 1,99 / 1,62 / 2,65 / 1,94 dB, mit Normalmoden 1,15 / 1,81 / 2,43 / 3,07 / 5,72 / 5,95 dB (60–400 / 400–1000 / 1000–2500 / 2500–5000 / 5000–7000 / 7000–10 000 Hz) — die Form längs der Stegeinlage trägt über 5 kHz nicht, und das Band darunter wird schlechter (3,07 statt 2,31 dB).

**Nebenbefunde.** (1) Schlagorte (−23 / 0 / +23 mm) und Aufnehmer (±36 mm) liegen an verschiedenen Stellen: die Messung bestimmt Produkte φ(Schlag) · φ(Aufnehmer); die Größe von φ an den Saiten kommt nur über die Form a + b x + c x². Über rund 2,5 kHz ist die Aufteilung zwischen Formwert und Abstrahlgewicht deshalb nicht belegt, nur ihr Produkt — die Übertragung zwischen Saiten liegt im Modell bei 3–5 kHz 8–14 dB über der zwischen den gemessenen Orten. Für den Klang zählt nur das Produkt (`Boden.cpp`); wer die Stegadmittanz der Gitarre braucht (Saitenlast), darf sie über 2,5 kHz nicht aus diesem Körper nehmen. (2) Im Band 3,15 / 4 kHz klingt der Körper schneller ab als gemessen (α 310 / 276 gegen 164 / 203 1/s) — die gefitteten Pole sind dort breiter als die am längsten klingenden Moden; gehört zu den dichten Paaren über 300 Hz.

**KQ26 a (Patrik):** Fassung C. `make_gitarre.py` Schritt 6, `FIT_FASSUNG` = `kFitFassung` = 2; die Körperdatei trägt den Teil in `statistical` (5–20 kHz, Samen 1) und das Gesetz in `damping`; die Prüfzahlen stehen in der Beschreibung unter `check.statistical`. Lauf vom 15.11.2026 (185 s): die 75 gefitteten Moden Feld für Feld wie vorher, 161 gezogene Moden (die erste bei 5081,8 Hz, die letzte bei 19 930,4 Hz), Terzpegel `left` −0,36 / +1,20 / −0,84 dB, `right` −1,36 / +1,09 / +0,27 dB. Das Skript baut `ziehe` nach (SplitMix64, Rayleigh, Box-Muller); der Wächter „31c-5d“ zieht mit `ziehe` aus der Körperdatei und vergleicht Zahl, die ersten drei und die letzte Frequenz mit der Beschreibung. Über 10 kHz ist das Gesetz fortgesetzt, nicht geprüft.

Nicht gerechnet: wie viel ein Gitarrenton über 5 kHz trägt (Überschlag: bei einer ideal gezupften Saite E4 rund 4 % der Kraftleistung, −14 dB, vor Saitendämpfung und Fingerbreite) — in 34k-6 an den Tönen.
