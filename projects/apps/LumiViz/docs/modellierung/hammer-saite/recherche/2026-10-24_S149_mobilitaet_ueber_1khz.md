# Recherche S149: Mobilität am Steg eines Flügels oberhalb 1 kHz (Prüfstein 31c-6)

> **Datum:** 2026-10-24 (Session 149) · **Auftrag:** Frage 1 und 2 aus PL31 a — gemessene Werte der Mobilität
> (Admittanz) am Steg eines Flügels oberhalb 1 kHz bis 5 kHz, mit Einheit und Bezugsgröße; Bauzustand je Messung
> und Einfluss von Saitenlast, Wölbung und Einbau; was MAESSTRO Abb. 5 und 6 zeigen; charakteristische Mobilität nach
> Boutillon & Ege.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen, [abgelesen] = aus einer Abbildung abgelesen, [nur Zusammenfassung] = Abstract,
> Suchergebnis oder Sekundärquelle.
> **Von Claude selbst nachgelesen:** Plath 2019 gedruckt S. 89 (Steg „decreases overall mobility by 10 dB and 10 dB to
> 20 dB above 1 kHz“; Rast „From 500 Hz to 5000 Hz the mean mobility stays constant“; Saiten und Rahmen „Up to 350 Hz“);
> Plath Fig. 6.10 c/d am gerenderten Bild: Achse „Driving point mobility [dB]“ ohne Bezugsgröße, 1–5 kHz um 10–18 dB;
> MAESSTRO S. 6–8 (Vereinfachungen, Tab. 1–2 mit η = 2 % je Werkstoff, Abb. 5, Gl. 12 geometrisches Mittel über
> 25–5000 Hz, „about 1500 kg/s, which corresponds to a mean mobility of −63.5 dB“). Alles andere: nur Bericht.
> **Quellen-PDFs:** `../quellen/` (nicht versioniert); die Hilfsbilder des Agenten im Scratchpad gehen mit der Sitzung
> verloren.
> **Eingeflossen in:** noch nicht — Vorlage der Fragen 1, 2 und 5 zu 31c-6 (PL31 a).

---

## Kurzfazit

1. **Gemessen am Flügelsteg oberhalb 1 kHz gibt es genau zwei frei lesbare Quellen.**
   - Plath 2019, Fig. 6.10: Treiberpunkt am Steg, 20 Hz bis 5 kHz, sechs Bauzustände. Die dB-Achse hat keinen genannten Bezug.
   - Conklin 1996, Fig. 6b: Wiedergabe bei Ege & Boutillon. E2-Steg, bis 3,2 kHz, Bezug ebenfalls unbekannt.

2. **Die einzige absolut skalierte Messkurve bis über 5 kHz ist Giordano 1998.** Es ist ein Klavier (upright), Steg bei C4, die Kurve in kg/s. Sie ist bei Boutillon & Ege 2013 Fig. 7 sauber wiedergegeben. Abgelesen: |Z| ≈ 2·10³ kg/s von 150 Hz bis 3 kHz, das ist etwa −66 dB re 1 s/kg. Danach fällt |Z| auf etwa 0,8–1·10³ bei 5 kHz.

3. **Plaths dB-Bezug lässt sich erschließen, er steht aber nirgends.** Aus Fig. 6.9 (s/kg) und Fig. 6.10 (dB) folgt schlüssig ein Bezug von etwa 1·10⁻⁴ s/kg. Das ist mein Schluss, nicht belegt. Damit liegt der eingeleimte und besaitete Flügel bei 1–5 kHz bei etwa 3–8·10⁻⁴ s/kg (−70 bis −62 dB re 1 s/kg). Die Kurve ist flach, ein Anstieg über 1 kHz ist nicht zu sehen.

4. **Bauzustand: oberhalb etwa 350 Hz bis 1 kHz zählen Saiten, Rahmen und Saitenlast im Mittel kaum.** Plath: Wirkung der Besaitung nur bis 350 Hz. Conklin: über 1 kHz „less modified“.
   - Entscheidend sind dagegen **Steg** (−10 bis −20 dB über 1 kHz) und **Rippen** (etwa −10 dB).
   - Das Einleimen in den Rast verändert das Mittel über 500 Hz nicht.
   - Euer Zustand „Rand eingespannt, ohne Saitenlast, ohne Wölbung“ ist damit über 1 kHz gut mit Plath PROD5/PROD6 und mit Conklin vergleichbar.

5. **MAESSTRO Abb. 6 ist nicht frequenzaufgelöst.** Es ist je Stegpunkt (Note) ein geometrisches Mittel über 25–5000 Hz in dB re 1 s/kg. Ein Terzband-Vergleich geht damit nicht. Ein Vergleich „ein Wert je Saite“ geht sehr wohl, und das ist für euch derselbe Flügel.

6. **Es bleibt eine Unsicherheit von etwa ±3–5 dB.** Gründe: Plaths dB-Bezug, Conklins unbekannter Bezug, Giordano ist ein Klavier, und alle Kurven sind abgelesen.

---

## Je Quelle

### 1. Plath, N. (2019): *From Workshop to Concert Hall …* Dissertation Uni Hamburg (ediss 6308)
Zugang: lokal, `Plath_2019_WorkshopToConcertHall_Diss.pdf`.

**Messung, Abschnitt 6.3 (gedruckt S. 86–91)**
- [wörtlich, S. 86] „Measurements are taken on the soundboard of D1 in six stages of production“. Angeregt wird an 15 Positionen, das sind die Saitenendpunkte auf Bass- und Diskantsteg (POS1–POS15).
- [wörtlich, S. 87] Messgerät: Impulshammer Kistler 9722A500 (0,1 kg), Aufnehmer PCB 352C23 „approx. 2 mm to 3 mm“ vom Anschlag. Es ist also praktisch ein Treiberpunkt.
- [wörtlich, S. 87] Zum Miniaturhammer: „the mobility functions obtained do not differ below 5 kHz“.
- [wörtlich, S. 87] Zu Fig. 6.10: „Each function is the mean of five independent measurements.“

**Bauzustände, Tab. 4.1 (S. 55) [wörtlich]**

| PROD | Zustand |
|---|---|
| 1 | blanke Platte |
| 2 | Rippen |
| 3 | Steg |
| 4 | Rippen ausgeklinkt |
| 5 | in den Rast geleimt |
| 6 | Eisenrahmen und Besaitung |

- [wörtlich, Tab. 6.2, S. 88] Masse des Bodens D1 bei PROD5–7: 13,70 kg. Rippen 4,17 kg, Steg 3,15 kg. Der Hersteller ist in meiner Suche nicht genannt; es sind „two concert grand pianos“.

**Fig. 6.9 (S. 89): Mittel 50–1000 Hz in s/kg** [abgelesen, ±0,0002]

| Stufe | Mittel |
|---|---|
| PROD1 | ≈ 0,005 |
| PROD2 | ≈ 0,0016 |
| PROD3–7 | ≈ 0,0007 |
| Wogram/Giordano gestrichelt | ≈ 0,001 |

Werte über 1 kHz enthält Fig. 6.9 nicht.

**Fig. 6.10 (S. 90): „Modulus of mobility“ 20 Hz–5 kHz, Achse „Driving point mobility [dB]“ ohne Bezugsgröße** [wörtlich: Achse und Bildunterschrift]
- Die Teilbilder:
  - a) Rippen, POS5
  - b) Steg, POS11
  - c) Einleimen in den Rast, POS10
  - d) Besaitung, POS5
- Gestrichelt = vorher, durchgezogen = nachher.
- Abgelesen 1–5 kHz (±3 dB):

| Teilbild | vorher | nachher |
|---|---|---|
| a) | ≈ 35–40 | ≈ 25–32 |
| b) | 20 dB bei 1 kHz, auf ≈ 35 bei 3–5 kHz steigend | ≈ 10–15 |
| c) | beide ≈ 12–18 dB, leicht steigend | beide ≈ 12–18 dB, leicht steigend |
| d) | beide ≈ 10–15 dB, Senke ≈ 8 dB bei 4 kHz | beide ≈ 10–15 dB, Senke ≈ 8 dB bei 4 kHz |

- **Bezug erschlossen (mein Schluss, nicht belegt):** Mit 1·10⁻⁴ s/kg passen beide Abbildungen zusammen.
  - PROD1: 0,005 s/kg entspricht 34 dB, Fig. a gestrichelt zeigt etwa 35–40.
  - PROD3–7: 0,0007 s/kg entspricht 17 dB, Fig. c/d zeigen bei 0,5–1 kHz etwa 13–17.
  - Ein Bezug von 1 s/kg oder 10⁻³ s/kg ergäbe negative Werte.
- Unter dieser Annahme gilt für PROD5/6 bei 1–5 kHz: ≈ 3–8·10⁻⁴ s/kg, also |Z| ≈ 1,3–3·10³ kg/s.

**Text dazu (S. 89/91)** [wörtlich]
- „From 500 Hz to 5000 Hz the mean mobility stays constant“ (nach dem Einleimen)
- Steg: „decreases overall mobility by 10 dB and 10 dB to 20 dB above 1 kHz“
- Rippen: „by 10 dB in the low and mid frequency range“
- Besaitung: „Up to 350 Hz“ Resonanzen etwa +20 Hz, Amplituden etwa −10 dB
- Gegen Conklin: Einfluss der Besaitung „only observable up to 350 Hz“
- „above 1 kHz the presented results can not confirm a sudden increase in mobility“

**Fig. 12.14 (S. 174)** [wörtlich] PROD7 gegen PROD8, Taste F4, nur bis 1200 Hz, dB-Achse −15…0, „all strings are damped“. Für oberhalb 1 kHz kaum brauchbar.

**Literaturüberblick S. 38–39** [wörtlich] Plath über Conklin:
- „No influence of down-bearing on mobilities is observable above 1 kHz.“
- Conklins Daten bestätigen keinen Anstieg bei hohen Frequenzen; „up to 3.2 kHz“.

**Zenodo 3271370** [nur Zusammenfassung, WebFetch]
- „Influence of Playing on the Tonal Characteristics of a Concert Piano – Dataset“, zur ISMA 2019.
- Inhalt: Tastenkraft (Kistler), Beschleunigung am Steg (PCB) beim Spielen, Kunstkopf; PROD07/PROD08, Tasten 1–88, 50 kHz Abtastrate.
- Es sind **keine Mobilitätsmessungen**. Ich habe nichts geladen.

### 2. Ege & Boutillon (2010/2012): *Synthetic description of the piano soundboard mechanical mobility*, ISMA 2010, arXiv 1210.5688
Zugang: frei, lokal.
- [wörtlich, S. 2] „Only four measurements of the admittance (or impedance) at the bridge“ sind veröffentlicht: Wogram, Nakamura, Conklin, Giordano. Alle sind „for upright pianos with muted strings“, außer Conklin (Konzertflügel).
- **Wogram (Fig. 1, F♯4, upright, gespielt)** [wörtlich, S. 2]
  - Zwischen 100 und 1000 Hz liegt |Z| bei „roughly 10³ kg s⁻¹“.
  - Darüber fällt sie um etwa 5 dB je Oktave auf „approximately 160 kg s⁻¹ at 10⁴ Hz“.
  - Das gilt als Messartefakt („purely springy impedance“, Weinreich).
- **Nakamura (Fig. 2, upright, bis 5 kHz)** [wörtlich]
  - Geschwindigkeit je Kraft, ohne absolute Skala.
  - „Above 1 kHz, the mobility becomes suddenly much larger“.
  - Resonanzen des Erregers liegen im Bereich.
- **Giordano (Fig. 3, upright, C4-Steg und Punkt zwischen Rippen, 100 Hz–10 kHz, |Z| in kg/s)**
  - [wörtlich] Am Steg bricht die Impedanz erst „above approximately 2.5 kHz“ ein, zwischen den Rippen schon bei ≈ 700 Hz.
  - [wörtlich] Unter 700 Hz: am Steg 1–2·10³ kg/s, zwischen den Rippen 0,6–0,7·10³ kg/s.
- **Conklin (Fig. 6, Konzertflügel, E2, Taste 20, normal zum Boden, 0–200 Hz und 0–3,2 kHz)**
  - [wörtlich] Peaks mit Rahmen und Saiten „about 15 dB less“, „Above 1 kHz the mobility is less modified“.
  - [wörtlich] Keine Messungen über 3,2 kHz veröffentlicht.
  - [abgelesen aus der Wiedergabe] Skala „LGMAG DB“ von −40 bis −90, Bezug nicht angegeben. Bei 1–3,2 kHz liegen beide Kurven (zerlegt und spielfertig) bei etwa −72 bis −75 dB (±2 dB), mittlerer Abstand höchstens 2–3 dB. Unter 1 kHz liegen sie bei etwa −60 bis −70.
  - Hinweis (Vermutung): Falls der Bezug 1 m/(N s) wäre, entspräche das 2·10⁻⁴ s/kg. Das passt zu MAESSTRO am Basssteg (−70 bis −73 dB). Unbelegt.
- **Synthese (Aufrechtklavier)** [wörtlich, S. 5–6]
  - Eingaben: n∞ = 1/19,5 modes/Hz, η = 2 %, M = 9 kg.
  - Mittel 100–1000 Hz ≈ 1,3·10⁻³ s/kg, entspricht etwa 800 kg/s.
  - Im Wellenleiter fällt die Impedanz auf „230 kg s⁻¹“ bei 2500 Hz („about 3.5 times less“), mit h ≈ 8 mm, Ly ≈ 55 cm, p ≈ 12,8 cm, n ≈ 4·10⁻³ modes/Hz.
  - Steg und Platte unterscheiden sich um „a factor of 2 to 3“, durch die Steifigkeit des Stegs.

### 3. Ege & Boutillon (2011): *Global and local synthetic descriptions of the piano soundboard*, Forum Acusticum, arXiv 1210.5109
Zugang: frei, lokal.
- [wörtlich, S. 4] Mit dem Steg ist das System „more or less homogeneous“ bis ≈ 4,2 kHz. Dort ist die halbe Biegewellenlänge im Steg gleich dem Rippenabstand.
- [wörtlich, S. 4–5] Am Steg passt die Synthese (≈ 800 kg/s) zu Wogram und Giordano (≈ 10³ kg/s). Es gibt jedoch eine „discrepency“ zu Giordano, „particularly visible around 1-3kHz“ in Fig. 8: gemessen liegt höher als synthetisiert.
- [abgelesen, Fig. 8] Giordano am Steg 1–3 kHz etwa 1–3·10³ kg/s, Abfall erst über 3–4 kHz.

### 4. Boutillon & Ege (2013): *Vibroacoustics of the piano soundboard: reduced models, mobility synthesis, and acoustical radiation regime*, arXiv 1305.3057 (JSV)
Zugang: frei, lokal.
- Eigene Messungen gibt es nur zu Modendichte und Dämpfung an einem Aufrechtklavier (Modalwerte bis 3 kHz), keine Mobilitätsmessung.
  - [wörtlich, S. 17] „modal density of roughly 0.02 Hz⁻¹“
  - [wörtlich, S. 20] Verlustfaktor 2,3 % · Zufallszahl
- [wörtlich, S. 20] Einzige verlässliche Vergleichsmessung: „The only reliable and well-documented point-mobility measurements“ sind Giordanos. Dessen Boden weicht nur „by only a few centimetres“ ab.
- **Gl. (26)** [wörtlich, S. 20]: Gν = n(f)/(4 Mν) mit Mν = M ⟨ξν²⟩/ξ²ν,Q. Das ist das geometrische Mittel von Re(Y) je Mode; ξ ist die Modenform am Punkt Q, M die Gesamtmasse.
- **Gl. (27)** [wörtlich, S. 21]: Y_c,plate = n_plate/(4 M_plate) und Y_c,beam = n_beam/(4 M_beam)·(1 − j).
  - Wellenleiter: G = ε_{Q,m}·n_guide/(2 M_g).
  - Gl. (28): Kopplung dreier Wellenleiter.
  - Am Steg addieren sich die charakteristischen Impedanzen von Balken (Steg) und Platte.
- **Fig. 6** [abgelesen, ±20 %] Charakteristische Impedanz am Steg (gestrichelt) ≈ 1,6–1,8·10³ kg/s von 100 Hz bis 3 kHz, das sind etwa 6·10⁻⁴ s/kg (≈ −64 dB re 1 s/kg).
- **Fig. 7 links (Giordano, C4-Steg, gemessen)** [abgelesen, Faktor ≈ 1,3]

| Bereich | \|Z\| in kg/s |
|---|---|
| 150 Hz – 3 kHz | Schwankung 0,5–5·10³, Mitte ≈ 2·10³ (≈ 5·10⁻⁴ s/kg, −66 dB) |
| 3–5 kHz | Abfall auf ≈ 0,8–1·10³ (≈ −59 dB) |
| 6–8 kHz | 150–500 |

  - Die berechnete Kurve Zc steigt von ≈ 1,6 auf ≈ 2·10³ bei 3 kHz.
  - [wörtlich, S. 22] Erklärung: der Steg „begins to "see" the ribs“ ab ≈ 3 kHz.
- **Fig. 7 rechts (Punkt X zwischen den Rippen)** [abgelesen] ≈ 800 kg/s bei 100 Hz, ≈ 500 bei 1 kHz, Senke bei 1,7 kHz, dann ≈ 300–400 bei 2–5 kHz.

### 5. Boutillon, Ege & Paulello (2012): *Comparison of pianos* (arXiv 1210.3948, SMAC)
Zugang: frei, lokal.
- [wörtlich, Tab. 1] **Steinway model D: Rippenabstand p = 12,7 cm, fg = 1355 Hz** (Grenze zwischen Platten- und Wellenleiterregime).
- [wörtlich, Gl. 1/2] Platte: Y_c,p = n_p/(r M_p) mit r = 4, oder r = 2 bei Anregung am Rand. Balken: Y_c,b = n_p/(r M_p)(1 − j).
- [abgelesen] Fig. 5 (Atlas-Klavier, berechnet) Zc ≈ 1,3–2·10³ kg/s bei 100 Hz–1 kHz. Fig. 3, gemessene Modendichte: ≈ 0,04–0,05 /Hz unter 1 kHz, über 1,2 kHz ≈ 0,02.

### 6. Elie, Cotté & Boutillon (2022): MAESSTRO, Acta Acustica 6, 30 (CC BY 4.0)
Zugang: frei, lokal.

**Modell** [wörtlich, S. 6–7, Tab. 1/2]
- Vereinfachter Steinway D: Platte durchgehend 8 mm, Steghöhe konstant, Rippen nicht verjüngt, 23 006 Elemente.
- Material: Sitka-Fichte ρ = 380, Ex = 11 GPa, Ey = 0,65 GPa; Buche für den Steg; η = 2 %.
- Stege: Bass 60 mm × 32 mm, Diskant 20 mm × 32 mm. Rippen 17–27 mm hoch, 21–25 mm breit.

**Abb. 5: Modendichte (Hz⁻¹) gegen Modenfrequenz, 0–5 kHz** [wörtlich: Bildunterschrift; Gl. 6: Kehrwert des Modenabstands, gleitend über 6 Moden]
- Werte für RP [abgelesen, ±0,02]:

| Frequenz | Modendichte (/Hz) |
|---|---|
| 0 Hz | ≈ 0,05 |
| 1 kHz | ≈ 0,10 |
| 1,5 kHz | ≈ 0,13 |
| 2 kHz | ≈ 0,15 |
| 3 kHz | ≈ 0,20 |
| 4 kHz | ≈ 0,30 |
| 5 kHz | ≈ 0,31 |

- Mein Integral daraus: ≈ 900 Moden bis 5 kHz (±10 %). Das ist vergleichbar mit euren 820.
- [wörtlich, S. 7] Zum Vergleich mit der Messung: „qualitatively similar with that of an upright piano measured in [5], but with larger values“.
- Gemessene Klavierwerte über 1,1 kHz sinken (Ege/Boutillon), MAESSTRO steigt.

**Abb. 6: „Mean mobility of the soundboard at the string-bridge coupling points“** [wörtlich]
- **x-Achse = Notenindex, nicht Frequenz.**
- [wörtlich, Gl. 12, S. 8] Je Saitenendpunkt das **geometrische** Mittel von |Y_A(f)| über die Frequenzpunkte 25–5000 Hz, berechnet aus Gl. (10).
- Mein Schluss: Bei gleich weit gesetzten Frequenzpunkten liegen etwa 80 % davon über 1 kHz. Der Wert ist dann vor allem ein Mittel über 1–5 kHz. Die Punktverteilung nennt das Papier nicht.
- **Einheit** [wörtlich, S. 8]: dB re 1 s/kg. Belegt durch „about 1500 kg/s, which corresponds to a mean mobility of −63.5 dB“; 20·log₁₀(1/1500) = −63,5.
- Werte RP [abgelesen, ±1 dB; gezeichnet sind nur Index 1–84]:

| Notenindex | mittlere Mobilität (dB re 1 s/kg) |
|---|---|
| 1–17 (Basssteg) | −68 bis −73 |
| 18 (Sprung) | −59 |
| 19–40 | −62 bis −66, schwankend |
| 40–60 | −63 bis −65 |
| 60–77 | −60 bis −63 |
| 78–84 | fällt auf ≈ −73 |

  - [wörtlich] Abstand Bass zu Diskant ≈ 10 dB bei 8 mm Plattendicke.
- **Vergleich mit Messung** [wörtlich, S. 8]: nur ein Zahlenwert, Giordano C4 (Klavier, Index 40) ≈ 1500 kg/s = −63,5 dB. Dazu: „the mean mobility of the reference piano is between −65 and −63 dB.“ Sonst gibt es keinen Messvergleich.
- **Gl. (11)** [wörtlich]: Y_C = 1/(8√(ρhD)) (unendliche Platte).

### 7. Corradi, Miccoli, Squicciarini, Fazioli (2017): *Modal analysis of a grand piano soundboard at successive manufacturing stages*, Applied Acoustics 125 (Preprint)
Zugang: Preprint lokal, Verlagsfassung kostenpflichtig.
- Fazioli F278.
- [wörtlich, S. 6] Stufe 3 = in den Rast geleimt, Stufe 4 = fertig mit Saiten und Saitenlast.
- [wörtlich, S. 26] Fig. 10: „STAGE 3: experimental vs. numerical FRF“, bis 4 kHz, „dB re 1 m/s/N“.
- **Achtung, das ist eine Transfer-Mobilität, keine Treiberpunkt-Mobilität** [wörtlich, S. 26]: angeregt nahe dem Steg (Kreuzung A♯4), gemessen „three ribs apart“.
- [wörtlich, S. 27] Die Übereinstimmung FE–Messung oberhalb 100 Hz ist „in general satisfactory“, mit verschobenen Resonanzen.
- [abgelesen, ±3 dB] Gemessen 1–4 kHz etwa −50 bis −62 dB re 1 s/kg, Mittel ≈ −56 dB.
- [nur Zusammenfassung, Suchergebnis] Behauptung: Über 200 Hz seien die Stegmobilitäten der frei hängenden und der eingebauten Platte „very similar“. Im Preprint habe ich das nicht gefunden.

### 8. Giordano (1998), JASA 103, 2128
- Volltext nicht frei: nicht auf der Purdue-Seite (geprüft), kein anderer freier Fundort.
- Die Kurven sind in Quelle 2, 3 und 4 wiedergegeben; siehe dort.

### 9. Conklin (1996) Part II, JASA 100, 695
- Volltext nicht frei gefunden.
- [nur Zusammenfassung] KTH-Vorlesungsseite „howdoes.html“: erste Mode 48 auf ≈ 60 Hz mit Saiten. Fig. 21 zeigt die Mobilität, Zahlen stehen nicht im Text.

### 10. Wogram (1980), *Das Musikinstrument* 24
- Original nicht frei.
- [nur Zusammenfassung, WebFetch der KTH-Vorlesung „inputimp.html“, Aufrechtklavier, gestimmt, MP 7]
  - „around 1000 kg/s at 100 Hz and 10 kg/s at 10 000 Hz“, mit etwa 6 dB je Oktave über 1 kHz.
  - Bezug: 0 dB = 1000 kg/s.
  - Das widerspricht Ege/Boutillons Ablesung (≈ 160 kg/s bei 10⁴ Hz). Beides gilt als Artefakt.
- [nur Zusammenfassung] „stringtension.html“: Saitenspannung verschiebt nur die unteren Resonanzen, etwa +50 Hz.

---

## Zu Frage 2: Bauzustand und Einfluss, je Frequenzbereich

| Änderung | Wirkung | Quelle |
|---|---|---|
| Rippen | ≈ −10 dB, unter 1 kHz und auch darüber | Plath, Fig. 6.10a |
| Steg | −10 dB im Mittel, −10 bis −20 dB über 1 kHz | Plath [wörtlich] |
| Steg statt freier Platte | Faktor 2–3 | Ege [wörtlich] |
| Einleimen in den Rast | starke Resonanzen bis 300 Hz, Mittel über 500 Hz unverändert | Plath [wörtlich] |
| Rahmen und Saiten | Wirkung nur bis 350 Hz (Resonanzen etwa +20 Hz, Peaks etwa −10 dB) | Plath [wörtlich] |
| Rahmen und Saiten | Peaks −15 dB, über 1 kHz „less modified“ | Conklin [wörtlich in Ege] |
| Rahmen und Saiten | mittlerer Abstand über 1 kHz höchstens 2–3 dB | Conklin Fig. 6b [abgelesen] |
| Saitenlast (downbearing) | „No influence … above 1 kHz“ | Plath über Conklin [wörtlich] |

**Wölbung:** Eine eigene dB-Angabe zur Mobilität habe ich nicht gefunden.

**Folgerung:** Ab etwa 0,5–1 kHz ist der Einfluss von Saiten, Rahmen und Last auf das Mittel klein, wenige dB. Für euren Zustand sind die Rippen und die Stegmodellierung maßgeblich.

## Zu Frage 4: Zahlen zur charakteristischen Mobilität

- Formel: Gl. 26/27 bei Boutillon & Ege 2013, Gl. 1/2 bei Boutillon, Ege & Paulello 2012, Gl. 11 bei MAESSTRO.
- Für einen Flügel geben Boutillon und Ege **keine eigene Zahl**.
- Die Zahlen gelten für Aufrechtklaviere:
  - Zc am Steg ≈ 1,6–2·10³ kg/s (Fig. 6/7, abgelesen)
  - Synthese ≈ 800 kg/s (Mittel 100–1000 Hz)
  - Wellenleiter 230 kg/s bei 2,5 kHz
- Ein grober Überschlag mit MAESSTROs n (≈ 0,1/Hz bei 1 kHz) und Plaths M = 13,7 kg ergibt n/(4M) ≈ 1,8·10⁻³ s/kg. Das ist mein Überschlag für einen Punkt der Platte, nicht des Stegs; am Steg liegt der Wert nach Gl. 26 niedriger.

## Nicht gefunden
- Volltexte von Giordano 1998, Conklin 1996, Wogram 1980 und Nakamura 1983.
- Plaths dB-Bezug, der nirgends genannt ist.
- Eine gemessene Treiberpunkt-Mobilität am **Flügel** in absoluter Einheit über 3,2 kHz. Plath reicht bis 5 kHz, aber mit erschlossenem Bezug.
- Eine eigene Messung am Steg bei Boutillon/Ege.
- Neuere Messungen 2020–2026 (ein Forum-Acusticum-2026-Papier aus Budapest ist offenbar nur ein Modell, sein Text war für WebFetch nicht lesbar).
- Chabassier, Tan/Lim, Berthaut, Kartofelev, Moore/Zietlow: keine Stegmobilität mit Zahlen. Die lokalen Chabassier-PDFs ergeben keinen Treffer zu mobility/admittance.

## Reichweite der Suche
- **Lokal gelesen:**
  - MAESSTRO S. 6–9
  - 1210.5688 vollständig (7 S.)
  - 1305.3057 S. 17–22
  - 1210.5109 S. 4–5
  - 1210.3948 S. 2–3
  - Plath S. 38–39, 55, 84–91, 172–175 sowie Tab. 6.2
  - Corradi S. 6, 9, 25–27
- **Lokal durchsucht ohne Mobilitätsdaten:** 1212.3068, Chaigne/Cotte/Viggiano 2013, Chabassier 2013 und RT-0425; etwa 15 Suchaufrufe mit quelle_suchen und pdftotext.
- **Web:** 6 Suchen, 9 Abrufe (Zenodo, Purdue, KTH Wogram ×3, KTH Conklin, arXiv 1101.4511, Southampton 403, FA2026).
- **Abgelesene Werte:** aus Abbildungen, die ich mit pdftoppm mit 220–250 dpi gerendert habe.

## Dateien
Im Repo habe ich nichts geändert. Diese Dateien sind entstanden und können gelöscht werden:

- Hilfsbilder, die ich gerendert habe, im Scratchpad:
  - `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\6e750082-935a-481c-9efc-7ba02ae01ec8\scratchpad\be13-22.png`
  - `…\scratchpad\be13_fig7.png`
  - `…\scratchpad\eb12-3.png`
  - `…\scratchpad\eb12_fig3.png`
  - `…\scratchpad\eb12_fig6.png`
  - `…\scratchpad\eb12_fig6b.png`
  - `…\scratchpad\mae-09.png`
  - `…\scratchpad\mae_fig6.png`
  - `…\scratchpad\plath-124.png`
  - `…\scratchpad\plath_ab.png`
  - `…\scratchpad\plath_cd.png`
- **Gegen die Vorgabe „nichts speichern“:** WebFetch hat beim Abruf des FA2026-PDFs automatisch eine Kopie abgelegt. Ich habe sie nicht geöffnet:
  `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\6e750082-935a-481c-9efc-7ba02ae01ec8\tool-results\webfetch-1792833869745-zh1gwm.pdf`
