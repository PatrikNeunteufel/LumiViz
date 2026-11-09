# Recherche IC1: Tonrad — der Klangerzeuger ohne Leslie

> **Auftrag:** Klasse 11: Tonradgenerator der B-3/C-3/A-100 — Räder, Abnehmer, Übersprechen, Zugriegel, Tastenklick,
> Perkussion, Scanner-Vibrato/Chorus, Vorverstärker; Zahlen, Rechenverfahren, was je Note und was nur auf der Summe geht.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug] = vom
> Abrufwerkzeug zitiert; [nur Zusammenfassung]; [eigene Rechnung].
> **Reichweite:** im Bericht selbst (16 Suchen, rund 49 Abrufe; MDPI Werner/Abel 2016, SOS-Serie, xen.wiki nicht
> erreicht).
> **Anmerkung der Sitzung:** Der Agent hat entgegen dem Auftrag eine Textausgabe des DAFx-16-PDFs im Scratchpad
> abgelegt (`scratchpad\werner.txt`, nicht im Projekt). Die Hersteller-Handbücher (Hammond Service Manual, b3world-
> Abschriften) sind rechtlich ungeklärt — Zahlen daraus als Fakten, kein Wortlaut nach LumiViz.
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md`.
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.

---

# Bericht IC – Klasse 11 „Tonrad": Klangerzeuger der Hammond-Orgel ohne Leslie

**Auftrag:** Vorab-Recherche zum Tonrad-Klangerzeuger der B-3/C-3/A-100 ohne Leslie: Tonräder, Abnehmer, Übersprechen, Zugriegel, Tastenklick, Perkussion, Scanner-Vibrato/Chorus, Vorverstärker; mit Zahlenwerten, Rechenverfahren und der Frage, was je Note und was nur auf der Summe geht.
**Datum:** 2026-10-26

**Reichweite**
- **Umfang:** 16 Websuchen und etwa 49 WebFetch-Abrufe, davon 9 auf verschiedene Abschnitte desselben OCR-Textes. Dazu kommt die lokale Textauswertung der zwei PDFs, die das Werkzeug zwischengespeichert hat (pdftotext).
- **Orte:** dafx.de, Crossref-API, Google Patents, archive.org, GitHub (setBfree, ZynAddSubFX), gareus.org, b3world.com, electricdruid.net, soundonsound.com, bonedo.de, pubs.aip.org (JASA), Wikipedia (nur als Hinweis).
- **Sprachen:** Englisch, Deutsch.
- **Nicht erreichbar:**
  - mdpi.com (403): Volltext Werner/Abel 2016 nicht gelesen.
  - en.xen.wiki „Just Hammond" (403).
  - SOS „Synthesizing Tonewheel Organs" (410 unter der alten URL; die Teile 1 und 2 unter neuer URL nicht abgerufen).
  - tapatalk ampworkshop (403), Mailingliste linuxaudio 2009 (404), QMRO-Seite (leer).
  - Native-Instruments-Forum: nur der Titel kam an.
  - Wikipedia-Artikel nur bis Zeichen 100 000 gelesen.
  - Patent US10199024B1 nur bis Zeichen 100 000 gelesen, die Ansprüche (claims) nicht.
- **Vom Werkzeug zwischengespeichert:**
  - `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\webfetch-1793044081467-ee78sg.pdf` (DAFx-11, Pekonen u. a., 4 S.)
  - `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\webfetch-1793044126551-dawey5.pdf` (DAFx-16, Werner u. a., 8 S.)
  - `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\bnel8w28t.txt` (pdftotext-Ausgabe zu DAFx-11, vom Werkzeug abgelegt)
- **Abweichung vom Auftrag (bitte beachten):** Ich habe eine Datei geschrieben, entgegen „schreibe nichts in Dateien": die Textausgabe des DAFx-16-PDFs, damit ich sie lesen konnte. Sie liegt unter `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\scratchpad\werner.txt`, nur im Scratchpad, nicht im Repo. Ich habe sie nicht gelöscht.

**Kennzeichnung**
- [wörtlich, selbst gelesen, S. x]: in der pdftotext-Ausgabe selbst gelesen.
- [wörtlich, Werkzeug]: Wortlaut, den WebFetch als Zitat geliefert hat.
- [nur Zusammenfassung]: kein Beleg.
- [eigene Rechnung]: von mir gerechnet.

---

## Quellen

**Q1 Pekonen, Pihlajamäki, Välimäki (2011)**
- Titel: „Computationally Efficient Hammond Organ Synthesis", Proc. 14th Int. Conference on Digital Audio Effects (DAFx-11), Paris, 19.–23.09.2011, S. DAFX-19 bis DAFX-22.
- Fundort: dafx.de/paper-archive/2011/Papers/49_e.pdf, frei abrufbar.
- Rechtsvermerk: weder im PDF (alle 4 Seiten gelesen) noch auf der Detailseite. Die Detailseite nennt nur die ISSN 2413-6689 (online). Abgerufen 2026-10-26.

**Q2 Werner, Dunkel, Germain (2016)**
- Titel: „A Computational Model of the Hammond Organ Vibrato/Chorus using Wave Digital Filters", Proc. 19th DAFx (DAFx-16), Brno, 5.–9.09.2016, S. DAFX-271 bis 278.
- Fundort: dafx.de/paper-archive/2016/dafxpapers/38-DAFx-16_paper_54-PN.pdf, frei.
- Rechtsvermerk: im Text aller 8 Seiten nicht gefunden. Abgerufen 2026-10-26.
- Die Bauteilwerte stammen laut Ref. [1] aus dem „Service manual … Tech. Rep. HOOO-OOO495, Nov. 1987".

**Q3 Werner, Abel (2016)**
- Titel: „Modal Processor Effects Inspired by Hammond Tonewheel Organs", Applied Sciences 6(7), 185, DOI 10.3390/app6070185.
- Lizenz laut Crossref: `https://creativecommons.org/licenses/by/4.0/`. Volltext bei MDPI nicht erreichbar (403).

**Q4 Patent US10199024B1, Abel und Werner**
- Titel: „Modal processor effects inspired by hammond tonewheel organs", angemeldet 31.05.2017, Priorität 01.06.2016, veröffentlicht 05.02.2019, Anmelder „Individual".
- Inhalt deckt sich mit Q3. Gelesen über Google Patents, abgerufen 2026-10-26.
- Laut Suchergebnis ist das Patent „active" [nur Zusammenfassung].

**Q5 Hanert, J. M., US-Patent 2,382,413**
- Titel: „Electrical musical apparatus", angemeldet 10.05.1943, erteilt 14.08.1945, Inhaber Hammond Instrument Company. Abgelaufen.

**Q6 „Hammond Organ Service Manual A, A100, … B3, C3 …"**
- Creator laut archive.org: „Hammond Organ Co (original version)". Restaurierte Fassung 2016 von Joe Luca, hochgeladen von joeluca am 16.05.2016.
- Fundort: archive.org/details/HSM4952016, OCR-Text Hsm495-2016_djvu.txt.
- Rechte: archive.org führt kein Lizenzfeld. Vorwort: „The manual is intended to be distributed FREE of COST." [wörtlich, Werkzeug].
- **Herstellerunterlage.** Die Rechtslage des Hammond-Originals (H000-000495) habe ich nicht geprüft. „Hammond" ist ein Markenname.

**Q7 b3world.com, „Hammond Organ Technical Information"**
- Abschriften der Absätze 5-1 bis 5-13 eines Hammond-Handbuchs, ohne Rechtsvermerk („Keyboard Exchange International").
- Rechtlich zweifelhaft (abgeschriebene Herstellerunterlage).

**Q8 Electric Druid, „Technical aspects of the Hammond Organ"**
- Gepostet am 15.06.2008; Antworten in den Kommentaren von Tom Wiltshire.
- Fußzeile „All Rights Reserved." Code und Schaltpläne der Seite stehen unter CC BY-NC-SA 4.0, der Artikel nicht.

**Q9 Moro, McPherson, Sandler (2017)**
- Titel: „Dynamic temporal behaviour of the keyboard action on the Hammond organ and its perceptual significance", J. Acoust. Soc. Am. 142(5), 2808–2822, DOI 10.1121/1.5003796.
- Lizenz laut Crossref CC BY 4.0. Volltext bei pubs.aip.org über das Werkzeug gelesen.

**Q10 setBfree (GitHub pantherb/setBfree)**
- README, AUTHORS, `doc/setBfree.1`, `doc/jboverdrive.1`, Kopfkommentar von `src/tonegen.c`; Code gelesen, aber nicht übernommen.
- Lizenz laut GitHub „GPL-2.0" (COPYING). Kopf von tonegen.c: „either version 2, or (at your option) any later version." [wörtlich, Werkzeug].
- Copyright: Fredrik Kilander 2003–2004, Robin Gareus 2008–2018, Will Panther 2012; Ken Restivo 2010 laut Manpage.

**Q11 Beatrix (Fredrik Kilander)**
- Seite gareus.org/wiki/beatrix. Ursprüngliche Lizenz: „YOU MAY NOT BUNDLE, RENAME, SELL, REDISTRIBUTE, REPACKAGE…" [wörtlich, Werkzeug].
- Später unter GPL in setBfree aufgegangen. Die Seite widerspricht sich selbst („not even FLOSS").

**Q12 ZynAddSubFX**
- GitHub: „available under the GPL-2.0-or-later license" [wörtlich, Werkzeug]. Kein Tonrad-Modell erwähnt.

**Q13 Gordon Reid, „Synthesizing Hammond Organ Effects", Sound On Sound, Januar 2004**
- Rechtsvermerk: „All contents copyright © SOS Publications Group and/or its licensors, 1985-2026. All rights reserved."

**Q14 Michael Geisel, „Hammond Workshop 4: Vibrato und Percussion", bonedo.de, 09.04.2022**
- Rechtsvermerk: „Copyright © 2026 Bonedo".

**Q15 Wikipedia „Hammond organ"**
- Nur als Hinweis. Lizenz-Fußzeile nicht gelesen.

---

## 1. Tonradgenerator

**Zahl der Räder und Zähne**
- „Only 91 frequencies are required for the organ." [wörtlich, Werkzeug, Q7 Abs. 5-x]
- „There are 12 wheels with 2 teeth" und „7 tone wheels have 192 teeth" [wörtlich, Werkzeug, Q7]. Dazwischen je 12 Räder mit 4, 8, 16, 32, 64 und 128 Zähnen [nur Zusammenfassung, Q7/Q8].
  - Summe: 7 × 12 + 7 = 91 [eigene Rechnung].
- 48 rotierende Baugruppen und „24 driving gears, 2 each of 12 sizes" [wörtlich, Werkzeug, Q7].
- Radpaare auf einem Rad: 2/32, 4/64, 8/128, 16/192 [nur Zusammenfassung, Q6 Abs. 5-3 bis 5-11].

**Antrieb und Getriebe**
- Synchronmotor „a synchronous speed of 1,200 rpm" bei 60 Hz, 1500 U/min bei 50 Hz [wörtlich bzw. Zusammenfassung, Q7].
- Formel F = 20 U/s × Zähne × Übersetzung; Beispiel A = 20 × 16 × 88/64 = 440 [nur Zusammenfassung, Q8 sowie Wijnen 2022 auf researchcatalogue.net].
- Übersetzungen (Zähne treibend/getrieben) nach Q8 [wörtlich, Werkzeug, als Tabelle]: C 85/104, C♯ 71/82, D 67/73, D♯ 105/108, E 103/100, F 84/77, F♯ 74/64, G 98/80, G♯ 96/74, A 88/64, A♯ 67/46, B 108/70.
- Tiefstes Rad: C ≈ 32,69 Hz [eigene Rechnung]; „C1 (approximately 32.7 Hz)" bis ≈ 5919,9 Hz [nur Zusammenfassung, Q4].

**Abweichung von der gleichstufigen Stimmung (A = 440 Hz)** [eigene Rechnung aus den Übersetzungen von Q8]

| Ton | C | C♯ | D | D♯ | E | F | F♯ | G | G♯ | A | A♯ | B |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Cent | −0,58 | −0,68 | +0,20 | −0,09 | −0,15 | −0,68 | +0,03 | +0,02 | −0,71 | 0 | −0,29 | −0,59 |

- Q8 schreibt dazu: G♯ „They are 0.69 cents flat from the correct pitch." [wörtlich, Werkzeug]. Q4: „deviations of up to 0:69 cents" [wörtlich, Werkzeug, Druckfehler im Original].
- Oberste 7 Räder (192 Zähne, C8 bis F♯8): +1,27 / +1,98 / +1,98 / +1,25 / +1,96 / +1,67 / +1,36 Cent [eigene Rechnung].
  - Q8: „the C# is the farthest off pitch, about 1.93 cents sharp." [wörtlich, Werkzeug]. Ich rechne 1,98; C♯ und D liegen gleichauf.
- Zusätzlich weichen die Zugriegel-Obertöne vom reinen Teilton ab, weil sie gleichstufig sind: 2⅔′ und 1⅓′ −1,955 Cent, 1⅗′ +13,686 Cent [nur Zusammenfassung, Q4 Tabelle 1]. Die Zahlen sind mathematisch nachvollziehbar [eigene Rechnung].

**Wellenform**
- „The lowest 12 tonewheels produce roughly square-wave signals" [wörtlich, Werkzeug, Q4]. Q4 setzt dafür Grundton, 3. und 5. Harmonische mit 4/π, 4/(3π), 4/(5π) an [nur Zusammenfassung].
- Q6: „twelve two-tooth complex tone wheels, which supply a fundamental tone" [wörtlich, Werkzeug].
- Kupferringe an tiefen Spulen: „for the purpose of reducing harmonics" [wörtlich, Werkzeug, Q7 Abs. 5-7].
- Pekonen: Unwucht als kleine Amplitudenmodulation, „simulates the small distance fluctuations between the disk and the pickup" [wörtlich, selbst gelesen, S. DAFX-20].
- Q4 modelliert die Verzerrung des Abnehmers (pickup) als gedächtnislose Kennlinie mit a = 0,3 [nur Zusammenfassung].

**Filter**
- „Both capacitors and re-actors are used with frequencies numbered 49 to 91 inclusive." [wörtlich, Werkzeug, Q7 Abs. 5-10]
- Räder 44–48 haben nur Drosseln (reactors). „Below frequency 44, neither capacitors nor reactors are used" [wörtlich, Werkzeug].
- Kondensatoren 0,255 µF für die Räder 49–54 und 0,105 µF für 55–91 [wörtlich bzw. Zusammenfassung, Q7].
- „Each transformer is matched to its condenser" [wörtlich, Werkzeug, Q6].

**Pegel**
- Q6 Tabelle V „Generator Output Voltages" in V, gemessen über die Klemmen G am Vorverstärker, Schweller voll, Messleitung am 5. Busbar [Werkzeug].
- Werte z. B. Rad 1: 9,8; Räder 9–20: etwa 14–16; Rad 43: 9; Rad 44: 2,5; Räder 80–91: 0,2–0,7 [wörtlich, Werkzeug, Tabelle].
- Daraus: Sprung von etwa −11 dB zwischen Rad 43 und 44 (Beginn der Drosseln) und etwa −35 dB von der Mitte unten bis Rad 91 [eigene Rechnung].
- Die Werte enthalten den Frequenzgang von Vorverstärker und Mischung. Toleranz: nachjustieren nur, wenn sie „deviate more than 30 percent" [wörtlich, Werkzeug].

## 2. Übersprechen (leakage/crosstalk)

**Mechanismus**
- „There is a certain amount of magnetic leakage between tones that are in the same bin." [wörtlich, Werkzeug, Q8]
- Hörbar ist das Übersprechen „four octaves above or below" [wörtlich, Werkzeug, Q8]. Das passt zu den Radpaaren 2/32 usw. [eigene Einschätzung].
- Weitere Wege laut setBfree-Manpage: „Crosstalk between connection on the terminal strip", dazu Übersprechen über Transformatoren und Verdrahtung [wörtlich, Werkzeug, Q10].

**Pegel**
- Kein Messwert gefunden.
- setBfree-Vorgaben: compartment-, terminalstrip- und wiring-crosstalk je 0,01, also −40 dB [eigene Rechnung]; transformer 0 [wörtlich, Werkzeug, Q10].
- Q4 simuliert als Vorführwerte „−∞, −24, −18, −12, and 16 dB" [wörtlich, Werkzeug, Druckfehler im Original]. Dort aber zwischen Nachbarrädern im Halbtonabstand, nicht im Abstand von vier Oktaven.

## 3. Zugriegel, Fußlagen, Rückfaltung, Mischung

**Mechanik und Mischung**
- Neun Kontakte je Taste, „All key contacts are alive at all times." [wörtlich, Werkzeug, Q6]
- Neun Busbars. Jeder Zugriegel kontaktiert einen davon, „connected to taps on the mixing transformer" [wörtlich, Werkzeug, Q6 Abs. 5-33].
- Stufen 0 („drawbar fully pushed in") bis 8 [wörtlich, Werkzeug, Q6 Abs. 4-3].
- Ein dB-Verhältnis je Stufe nennt Q6 nicht.
- Matching-Transformator: passt „the low impedance of the generator and key circuits" an den hochohmigen Verstärkereingang an [wörtlich, Werkzeug, Q6].

**dB je Stufe**
- Q4: „converted to an amplitude in 3 dB increments" [wörtlich, Werkzeug].
- Die Tabelle 2 kam beim Abruf verstümmelt an. Plausibel: 8 = 0 dB, je Stufe −3 dB, 0 = aus [eigene Einschätzung].
- setBfree stellt das als Taper-Modell je Taste, Busbar und Rad ein („These units are in dB") [wörtlich, Werkzeug].

**Fußlagen**
- 16′, 5⅓′, 8′, 4′, 2⅔′, 2′, 1⅗′, 1⅓′, 1′ entsprechen den Halbtonabständen −12, +7, 0, +12, +19, +24, +28, +31, +36 [nur Zusammenfassung, Q4 Tabelle 1].

**Rückfaltung (foldback), Radbereiche je Zugriegel für 61 Tasten**
- Nach Q8 [Tabelle wörtlich, Werkzeug; Spaltenzuordnung von mir, die Summe 61 ist je Zeile erfüllt]:

| Zugriegel | Radbereich | Rückfaltung |
|---|---|---|
| 16′ | 13–24, dann 13–61 | unten (tiefste Oktave) |
| 5⅓′ | 20–80 | – |
| 8′ | 13–73 | – |
| 4′ | 25–85 | – |
| 2⅔′ | 32–91 | + 80 |
| 2′ | 37–91 | + 80–85 |
| 1⅗′ | 41–91 | + 80–89 |
| 1⅓′ | 44–91 | + 80–91 + 80 |
| 1′ | 49–91 | + 80–91 + 80–85 |

- setBfree unterscheidet Generatortypen „C-based generator, foldback" und „… no foldback" [wörtlich, Werkzeug].

## 4. Tastenklick (key click)

- Ursachen: „nine key contacts not shutting simultaneously" plus Kontaktprellen (bounce) [wörtlich, Werkzeug, Q8].
- Messungen nach Q9 [wörtlich, Werkzeug]:
  - Kontakte schließen „within the space of about 1.5 mm from the earliest to the latest" (Abschn. IV B 2), Tastenweg 11 mm.
  - Frühe Prellungen sind „usually less than 5 ms long"; späte folgen „3 ms or more after the end of the early bounces" (IV B 3).
  - Angeschlagene Tasten (struck, 0,5–0,9 m/s) prellen insgesamt „in the 10–60 ms range" (IV C).
  - Einschwingen einer Note: „35 ms for the pressed one and 20 ms for the struck one" (IV B 1).
  - Der Versatz zwischen den Kontakten zeigt „inverse proportionality with the onset velocity".
- Modelle:
  - Pekonen: „adding the sixth harmonic", Abklingen 70 ms [wörtlich, selbst gelesen, S. DAFX-19/20].
  - setBfree: zufällige Dämpfung beim Schließen der Bus-Oszillator-Verbindung, Rauschstoß von 12,5 bis 62,5 % von 128 Samples [nur Zusammenfassung der Manpage-Werte, Q10].
  - Moro: neun virtuelle Kontakte, gesteuert über die fortlaufende Tastenposition [nur Zusammenfassung].
- Pekonen zum Hüllverlauf: Anschlag „a constant decay time of 50 ms" auf einen Haltepegel nahe dem Maximum [wörtlich, selbst gelesen, S. DAFX-20].

## 5. Perkussion

**Schaltung laut Q6**
- Nur Obermanual, nur mit gedrückter B-Preset-Taste [Werkzeug].
- „The 8th harmonic signal is not available on the upper manual as long as percussion tablet is on." [wörtlich, Werkzeug]
  - Bei eingeschalteter Perkussion ist also der 1′ stumm. Bonedo: „der 1′ Zugriegel nicht verwendbar und somit stumm" [wörtlich, Werkzeug, Q14].
- Pegelabsenkung: R1 in Reihe zur Sekundärseite von T2, „to reduce upper manual organ signal so that lower manual will musically balance" [wörtlich, Werkzeug, Q6].
- Soft: schaltet „R46 across the percussion output transformer" [wörtlich, Werkzeug].
- Fast: R57 parallel zu R58; R59 wird abgetrennt, „which increases output signal about 50%" [wörtlich, Werkzeug].
- Single Trigger: Laden und Entladen von C31. Die Perkussion kommt wieder, erst wenn alle Obermanual-Tasten losgelassen sind [nur Zusammenfassung, Q6 Abs. 5-52].
- Reid: „Hammond percussion is polyphonic, but of the single-triggering variety" [wörtlich, Werkzeug, Q13].

**Zeiten**
- Herstellerwerte nicht gefunden.
- Bonedo: „‘Fast’ (ca. 250 ms) und ‘Slow’ (ca. 900 ms)" [wörtlich, Werkzeug, Q14]; was genau gemessen ist, sagt der Artikel nicht.
- setBfree-Vorgaben: fast 1,0, slow 4,0 (Ziel −60 dB laut Kommentar), soft 0,5012 ≈ −6 dB [Werkzeug bzw. eigene Rechnung].

## 6. Vibrato/Chorus-Scanner

**Antrieb, Platten, Leitung**
- „driven at 412 revolutions per minute", also 6,87 Hz [wörtlich, Werkzeug, Q6; eigene Rechnung].
- 16 Sätze feststehender Platten [Werkzeug, Q6].
- Q2 nennt „≈ 6 Hz" (nach Hanert) [wörtlich, selbst gelesen, S. DAFX-273]. Q8 nennt 7 Hz, setBfree 7,25 Hz [Werkzeug].
- Leitung: frühe Ausführungen „25 section vibrato line", die gekoppelte Ausführung „requires only 18 sections" [wörtlich, Werkzeug, Q6].
- Q2 Tabelle 1 [selbst gelesen, S. DAFX-272]: L = 500 mH, C = 0,004 µF (C18 = 0,001 µF), Rc 22 kΩ, Rt 15 kΩ, Spannungsteiler Rk± an den Abgriffen 1–6.
- Q2 widerspricht sich: im Text „19 LC ladder stages", in Abbildung und Tabelle L1…L18.

**Tiefe**
- V3 scannt die ganze Leitung „in about 1/14 second", das bewirkt „about 1-1/2 percent decrease in frequency" [wörtlich, Werkzeug, Q6 Abs. 5-22] ≈ 26 Cent [eigene Rechnung].
- V2: „about half", V1: „about one-third" der Leitung [wörtlich, Werkzeug].
- Q2 Tabelle 2, Abgriffe [selbst gelesen, S. DAFX-272]:
  - V1/C1: v1–v9
  - V2/C2: v1, v2, v3, v5, v7, v9, v11, v12, v13
  - V3/C3: v1, v2, v4, v7, v10, v13, v16, v18, v19
- Q4: „a depth of 25 cents being typical" [wörtlich, Werkzeug].
- Hanert 1945 (älterer Entwurf): 32 T-Glieder, ≈ 6/s, 2,8 / 2,1 / 0,7 % [nur Zusammenfassung, Q5].

**Verzögerung und Frequenzgang**
- „≈ 0.85 ms" Gesamtverzögerung, Durchlassgrenze „≈ 7075 Hz" [wörtlich, selbst gelesen, S. DAFX-276].
- Welligkeit „many around 6 dB" [selbst gelesen, S. DAFX-277]; Teilerverluste −2,9 … −0,56 dB an den Abgriffen 1–6 (Tabelle 3, S. DAFX-277).

**Chorus**
- Vibrato-Signal „mixed with a portion of signal without vibrato"; „For normal vibrato, this resistor is short-circuited." [wörtlich, Werkzeug, Q6 Abs. 5-23]

**Modellierung**
- Q2: Wellendigitalfilter (wave digital filter, WDF) für die LC-Kette (LC ladder). Der Scanner ist ein linearer Überblender zwischen zwei Abgriffen; die Last durch den Scanner wird vernachlässigt [selbst gelesen, S. DAFX-273].
- Das verzerrte bilineare Verfahren (warped BLT) trifft 7075 Hz. Alternativ „4× oversampling is an effective though expensive way" [wörtlich, selbst gelesen, S. DAFX-276].
- Reid: Ein normaler Chorus scheitert, weil er nur „a single instance of the pitch-modulated signal" beimischt [wörtlich, Werkzeug, Q13].

## 7. Vorverstärker und Schweller

- Röhren des AO-28 laut Händler-Ersatzsatz: 2× 6AU6, 2× 6C4, 12AX7, 12AU7A, 12BH7, 6X4 [nur Zusammenfassung, Händlerseite thetubestore.com; kein Beleg].
- Reid: „A Hammond exhibits mild compression when you add notes to a chord" [wörtlich, Werkzeug, Q13].
- setBfree-Übersteuerung: Parameter bias, feedback, sagtobias, globfeed; das nachgebildete Schaltbild ist nicht dokumentiert [nur Zusammenfassung, Q10].
- Schweller (expression pedal):
  - Q6 sagt nur, er wirke „on the two manuals and pedals equally" [Werkzeug].
  - Ältere Modelle: Rheostat mit „32 contacts" [Werkzeug].
  - Ein Drehkondensator, der Lautstärke und Klangbalance zugleich verändert, ist nur aus Foren belegt [nur Zusammenfassung].
  - Kennlinie und Frequenzgang: nicht gefunden.

## 8. Rechenverfahren und Kosten

- **Pekonen (Q1):** additive Synthese je Taste; „it requires only ten sinusoidal oscillators" (neun Teiltöne plus Tremolo) [wörtlich, selbst gelesen, S. DAFX-21].
  - Das Tremolo mit „five Hertz" aus dem Patent von 1934 (S. DAFX-20) ist **nicht** der Scanner der B-3.
  - Q1 rechnet ohne Rückfaltung, ohne Übersprechen und ohne Perkussion; die Klangfarbe ist von Hand gewählt.
- **Werner u. a. (Q2):** WDF der Kette mit 36 Reaktanzen, 19 Abgriffe, Scanner als Überblendung. Eine Operationszahl nennt die Arbeit nicht.
- **Werner/Abel (Q3/Q4):** „Hammondizer" als Modalprozessor mit „1177 exponentially spaced modes" [wörtlich, Werkzeug]. **Patentiert (US10199024B1)**: vor einer Übernahme der Idee prüfen.
- **setBfree:** echte Generatorsimulation mit 91 Rädern, Übersetzungen für 60 und 50 Hz, Taper je Taste, Bus und Rad, Übersprechen, Klick, Scanner, Übersteuerung. Lizenz GPL-2.0-or-later, also keinen Code übernehmen.
- **Beatrix:** Vorläufer, ursprünglich restriktiv lizenziert.
- **ZynAddSubFX:** GPL-2.0-or-later, kein Tonrad-Modell.

---

## Je Note oder Summe

**Je Note vorab rechenbar**
- Wellenform je Rad, Rad-Amplitudenmodulation, Stimmung (Cent-Tabelle), Rückfaltung, Zugriegel-Mischung einer festen Registrierung (feste Registrierung = ein Tonspeicher-Satz je Registrierung).
- Übersprechen im Rad-Fach, weil es mit der Taste mitgeschaltet wird [eigene Einschätzung].
- Tastenklick als Varianten je Note und Anschlag [eigene Einschätzung].

**Nur auf der Summe oder global**
- **Phasenkohärenz:** Ein Rad speist mehrere Tasten mit derselben Phase; zwei Noten, die dasselbe Rad nutzen, addieren sich kohärent. Vorab gerechnete Noten mit eigener Startphase zerstören das. Abhilfe: Tonspeicher **je Rad** (91 periodische Wellen) statt je Note, Phase aus der globalen Zeit [eigene Einschätzung].
- **Leichte Kompression bei Akkorden** (Last am Transformator) [Q13 qualitativ; Modell: eigene Einschätzung].
- **Perkussion:** eine einzige Hüllkurve (C31) für das ganze Obermanual, Auslösung nur nach Loslassen aller Tasten. Je Note lassen sich nur die Varianten „mit" und „ohne" vorab rechnen; ein Akkord, der nicht gleichzeitig angeschlagen wird, wird damit falsch [eigene Einschätzung].
- **Globale Schalter:** 1′ stumm und Absenkung über R1 betreffen die ganze Registrierung.
- **Scanner:** linear, aber zeitvariant mit globaler Phase bei 6,87 Hz. Rechnerisch ginge er je Note nur mit derselben absoluten Scannerphase; das passt nicht zu frei startenden, vorab gerechneten Noten. Also auf der Summe je Manual [eigene Einschätzung, Begründung: Überlagerung (Superposition) bei linearen zeitvarianten Systemen].
- **Vorverstärker, Übersteuerung, Schweller:** auf der Summe.

**Kosten** [eigene Einschätzung]
- 91 Tabellen-Oszillatoren und neun Multiply-Add-Operationen je klingender Taste sind vernachlässigbar.
- Ein WDF mit 18 Gliedern (eventuell 4× überabgetastet) je Manual ist in Echtzeit klein.

## Nicht gefunden

- Gemessene Übersprech-Pegel in dB.
- Herstellerwerte für die Perkussionszeiten und für die dB-Werte Soft/Normal.
- Ein Zugriegel-dB-Verhältnis aus Hammond-Unterlagen.
- Schweller-Kennlinie und Frequenzgang; Schaltbild und Kennwerte des AO-28 aus einer Primärquelle.
- Die „96 Räder": Q7 nennt 96 Klemmen, davon 91 Frequenzen und 3 Masse; ergibt 94, der Rest bleibt unerklärt.
- Ein Rechtsvermerk von Hammond-Suzuki zu den Handbüchern (nicht geprüft).

Kein Treffer ist kein Beweis.

## Eigene Einschätzung (kein Beleg)

- Für LumiViz ist die saubere Architektur ein **Generator mit Tonspeicher je Rad**, globaler Phase und einer Mischmatrix aus Tasten, Busbars und Rädern, dazu Summenstufen für Perkussion, Scanner und Vorverstärker. „Je Note ein vorab gerechneter Ton" verfehlt die Kopplung.
- Die Werte für den Scanner (6,87 Hz, V3 ≈ 26 Cent, 0,85 ms) sind durch Q2 und Q6 gut gestützt.
- Die Zeiten für Perkussion und Klick sollten an Aufnahmen angepasst werden.
- Q1 ist als Vorbild zu grob: Tremolo statt Scanner, keine Rückfaltung.
