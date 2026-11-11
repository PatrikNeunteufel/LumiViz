# Akkordeon: Maße und Betriebsgrößen der durchschlagenden Zunge je Ton — Recherche

| | |
|---|---|
| **Typ** | Agentenbericht (unverändert abgelegt) |
| **Datum** | 2026-11-11 |
| **Sitzung** | LumiViz S190, Stufe 34j-4 |
| **Auftrag** | Belegte Maße (Zunge, Stimmplatte, Kanzelle) und Betriebsgrößen (Balgdruck, Einschwingen, Amplitude, Tonhöhe über Druck) der Akkordeon-Stimmzunge je Ton über den Umfang suchen, um Vorgaben je Taste herzuleiten. Nur Recherche, keine Änderung am Code. |
| **Werkzeuge** | WebSearch, WebFetch, `curl -sI` (nur Kopfzeilen). Kein Browser. |

## Vorbemerkungen zur Belegkraft

**Kennzeichen in den Tabellen**

- **B** — *am Seitenbild selbst gelesen*: Die Seite des PDF lag mir als Bild vor, die Zahl steht dort an der genannten Stelle (Seite, Tabelle, Abbildung). Das ist die stärkste Stufe in diesem Bericht.
- **A** — *vom Abrufwerkzeug als wörtliche Stelle geliefert*: WebFetch hat die Stelle als Zitat aus der Seite ausgegeben; ich habe das Original nicht selbst gesehen. Schwächer als B — das Werkzeug kann glätten.
- **Z** — *nur Zusammenfassung des Such- oder Abrufwerkzeugs*. **Kein Beleg**, nur eine Spur.
- **F** hinter dem Kennzeichen — Forum oder Händlerseite, **geringe Belegkraft** unabhängig von B/A.

**Zu den Zitaten:** Meine Arbeitsregeln erlauben mir nur sehr wenig wörtliche Wiedergabe aus fremden Texten. Statt eines Zitats je Angabe steht deshalb die **genaue Fundstelle** (Seite, Tabelle, Abbildung, Beitragsnummer mit Datum); Zahlen sind unverändert übernommen. Auch die Rechtsvermerke sind sinngemäß wiedergegeben und an der genannten Stelle im Wortlaut nachzulesen.

**Zum Abruf von PDFs:** WebFetch kann den Text gepackter PDFs nicht lesen, legt die Datei aber **von sich aus** im Ergebnisordner der Sitzung ab (`…\.claude\projects\…\tool-results\webfetch-*.pdf`). Dort habe ich sie mit dem Lesewerkzeug seitenweise als Bild gelesen. Ins Repo oder in eine Ablage habe ich nichts gelegt; diese Dateien sind Sitzungsreste und keine Quelle im Sinn der Ladeliste.

**Aus Abbildungen abgelesene Werte** sind mit „≈“ und „Abb.“ gekennzeichnet; Ablesefehler rund ±5 % der Achse.

**Reichweite der Suche:** rund 45 Suchanfragen (englisch, deutsch, italienisch, französisch, spanisch, tschechisch, russisch) und rund 55 Abrufe. Gesucht nach Maßtabellen der Hersteller (Voci Armoniche, Harmonikas Louny/Titlbach, Binci, Cagnoni, Antonelli), Handbüchern (Richter), IfM Zwota, UPV/EHU Bilbao, Ricot 2005, Tonon, Cottingham und Mitarbeitern, Foren (accordionists.info, rec.music.makers.squeezebox), Patenten, Museumskatalogen. **Nicht durchsucht:** gedruckte Bücher, Bezahlschranken (JASA, Acta Acustica 2014), HAL (durch Bot-Schutz gesperrt), Seiten mit No-AI-Vermerk, Bilder in Forenbeiträgen (dort stehen einige Maße nur im Foto). **Kein Treffer ist kein Beweis** — eine Maßtabelle der Hersteller kann es geben, ohne dass sie öffentlich im Netz liegt.

---

## 1 Quellen

| Nr. | Quelle | URL | Zugang | Rechtsvermerk (sinngemäß, mit Fundort) |
|---|---|---|---|---|
| Q1 | Llanos-Vázquez, Elejalde-García, Macho-Stadler: *Acústica de la lengüeta libre del acordeón*. Tecniacústica 2014 (45º Congreso Español de Acústica), Murcia. 8 Seiten. | `https://www.sea-acustica.es/archivo/publicaciones/AMS-0_003_CI.pdf` (die Adresse unter `documentacion.sea-acustica.es/storage/publicaciones/` leitet dorthin um) | frei | keiner gefunden (S. 1–8 gelesen) |
| Q2 | Tonon, Tom: *Reed Cavity Design and Resonance*. Papers of the International Concertina Association (PICA) 2, 2005; berichtigte Netzfassung. | Seite `https://concertina.org/?p=529`, PDF `https://www.concertina.org/wordpress/wp-content/uploads/2020/07/reed_cavity_design_resonance.pdf` | frei | im PDF (S. 1–19) keiner gefunden; Fußzeile der Seite: Copyright 1952–2026 International Concertina Association (A) |
| Q3 | Cottingham, Lilly, Reed: *The motion of air-driven free reeds*. Sammelband 137. ASA / 2. Forum Acusticum, Berlin 1999. 4 Seiten. | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF` | frei (im Vorlauf nicht erreicht, jetzt erreicht) | keiner gefunden |
| Q4 | Cottingham, Reed, Busha: *Variation of frequency with blowing pressure for an air-driven free reed*. Berlin 1999. 4 Seiten. | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF` | frei | keiner gefunden |
| Q5 | Cottingham: *Theoretical and experimental investigation of the air-driven free reed*. ICA 1998, S. 761–762. | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF` | frei | keiner gefunden |
| Q6 | Elejalde-García, Macho-Stadler, Llanos-Vázquez: *Accordion acoustics: A study on pitch bending*. Acoustics in Practice AiP-2021-02. (**schon bekannt**; hier nur für die Ventile neu ausgewertet) | `https://www.euracoustics.org/documents/14/AiP_2021_02_Accordion_acoustics.pdf` | frei | S. 1, Kopf rechts: alle Rechte vorbehalten, Nutzung durch Dritte nur mit schriftlicher Zustimmung des Rechteinhabers (B) |
| Q7 | Pomberger, Frank, Trabesinger, Höldrich: *Material-Dependent Sound and Vibration of a Harmonica Reed Block*. DAGA 2021 Wien, S. 852–855. | `https://pub.dega-akustik.de/DAGA_2021/data/articles/000550.pdf` | frei | keiner gefunden |
| Q8 | Forum accordionists.info, Thema *Bass Reeds Modification* (Febr. 2018), Beiträge #1–#13. | `https://www.accordionists.info/threads/bass-reeds-modification.4745/` | frei | Fußzeile nur zum Forensystem (XenForo); kein Vermerk zu den Beiträgen gefunden |
| Q9 | Newsgroup rec.music.makers.squeezebox, Thema zur Fachliteratur über Zungen (8.–9. Dez. 2004; „Rashid“, Th. Kloba, J. Pascher); dieselben Angaben wiedergegeben auf accordionists.info, Thema *Reed technique design discussion by Rashid and Steve #1* (17.6.2023). | `https://groups.google.com/g/rec.music.makers.squeezebox/c/SNgE_eMMAKE` · `https://accordionists.info/goto/post?id=107837` | frei | keiner gefunden |
| Q10 | Forum accordionists.info, Themen *About reed quality* (März 2025), *Slow reeds* (Juli 2022), *Common reed sizes?* (Febr. 2025), *Voci Armoniche Reeds* (Dez. 2023). | `https://accordionists.info/goto/post?id=145825` · `…/threads/slow-reeds.9173` · `…/goto/post?id=143607` · `…/goto/post?id=117201` | frei | wie Q8 |
| Q11 | Voci Armoniche s.r.l.: *Accordion Reed Production*. Accordions Worldwide, 2003. | `https://www.accordions.com/articles/accordion_reed_production.aspx` | frei | keiner im Seiteninhalt gefunden |
| Q12 | Petosa, Joe sr.: *The Heart and Soul of an Accordion*. Petosa Accordions, 5.5.2021. | `https://petosa.com/blogs/accordion-culture/the-heart-and-soul-of-an-accordion-by-joe-petosa` | frei | Fußzeile: Copyright 2026 Petosa Accordions (A) |
| Q13 | Vavra, Brendan: *MIDI Accordion — Dynamic Volume*, 2022 (Bastelprojekt, Instrument **ohne Zungen**). | `https://bvavra.github.io/MIDI_Accordion/volume/` | frei | keiner auf der Seite gefunden |
| Q14 | McFall, Pete: *Dance Organ Accordion Air Flow Requirement*. Mechanical Music Digest, 8.9.2014. | `https://mmdigest.com/Archives/Digests/201409/2014.09.08.02.html` | frei | nicht geprüft |
| Q15 | Heveling, Carsten: *Stimmtechnik / Harmonisierung* (Bandoneon). | `https://bando-bando.de/verschiedenes/bandoneon_technik/stimmtechnik-harmonisierung.html` | frei | Copyright 1998 bis heute BANDO BANDO Wuppertal (A); kein KI-Vermerk auf der Seite gefunden |
| Q16 | Pykett, C. E.: *The physics of free reeds*, 2023. | `https://colinpykett.org.uk/physics-of-free-reed-wind-instruments.htm` | frei | Copyright C E Pykett 2023 (A) |
| Q17 | McGee / Hornett: *Lachenal Duet Concertina 3316 reeds*, 2014 (Konzertina). | `https://mcgee-flutes.com/Concertina/Lachenal%20Duet%20Concertina%203316%20reeds.htm` | frei; Maße nur als Bilddiagramme | keiner gefunden |

**Aus Q1 und Q7 als wichtigste weiterführende Literatur genannt, nicht im Netz:**

- G. Richter: *Akustische Probleme bei Akkordeons und Mundharmonikas*, Teil 1 (Einführung in die allgemeinen Grundlagen) und Teil 2 (Untersuchungen spezieller Phänomene), U. Schmülling 1985 — in Q7 S. 855 als [1], [2] geführt (B). Q7 S. 852 stützt sich für die Kanzelle auf Teil 1, Kap. 5.3.
- T. Benetoux: *The Ins and the Outs of the Accordion*, Le Tour 2002 — in Q1 als [1] (B), Beleg für die beschwerten Basszungen.
- Llanos-Vázquez u. a.: *Physical and psychoacoustic characterization of the different types of attacks on the accordion*, Acta Acustica united with Acustica 100 (2014) 375–384 — in Q1 als [9] (B).

---

## 2 Maße der Stimmzungen je Ton

**Befund vorweg: Eine Maßtabelle je Ton (Länge, Breite, Dicke, Schlitz) habe ich nicht gefunden** — weder von einem Hersteller noch in einer erreichbaren wissenschaftlichen Arbeit. Gefunden sind Einzelzungen und Faustregeln.

### 2.1 Einzelne vermessene Zungen (Akkordeon)

| Größe | Wert | Einheit | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|---|---|
| Länge L, Zunge 1 | 42 | mm | f₁ = 142,7 Hz (zwischen C♯3 und D3) | Q1 S. 6, Tabla 3 (dort 0.042 m) | B |
| Masse m, Zunge 1 | 0,68 | g | wie oben | Q1 S. 6, Tabla 3 (dort 0.68·10⁻³ kg) | B |
| Länge L, Zunge 2 | 37 | mm | f₁ = 226,4 Hz (rund A3 +50 Cent) | Q1 S. 6, Tabla 3 | B |
| Masse m, Zunge 2 | 0,29 | g | wie oben | Q1 S. 6, Tabla 3 | B |
| Instrument zu Q1 | Pigini Sirius (Konzertakkordeon, Freibass) | — | — | Q1 S. 2, Montaje experimental | B |

Gegenprüfung in derselben Quelle: Q1 Abb. 5 (S. 6) und Tabla 4 (S. 7) nennen die Zungen 1 und 2 mit denselben Rollen; eine zweite Stelle mit L oder m gibt es nicht. Ob m die ganze Zunge mit Fuß oder nur den schwingenden Teil meint, sagt Q1 nicht.

Zum Vergleich (schon bekannt, nicht neu): Adachi u. a. 2026, C4: L = 32,8 mm, B = 3,45 mm, 0,248 g, 267,6 Hz.

### 2.2 Faustregeln und Verhältnisse

| Größe | Wert | Ton / Geltung | Quelle, Stelle | Kz. |
|---|---|---|---|---|
| Länge zu Breite der Zunge | typisch 8 : 1 bis 10 : 1 | allgemein | Q9, „Rashid“, 8.12.2004 | A, F |
| Länge zu Breite, gemessen | 5,7 : 1 bis 8,7 : 1, Mittel 7,4 : 1 | Diskantzungen auf langer Platte, alte Chemnitzer Konzertina von Dix | Q9, Th. Kloba, 8.12.2004 | A, F |
| Länge zu Breite, gemessen | 6,2 : 1 bis 9,8 : 1, Mittel 8,6 : 1 | Instrument „Star“ von 2000, lange Platten | Q9, Th. Kloba, 8.12.2004 | A, F |
| Dicke an der Spitze | bis herab zu 0,002 Zoll (0,05 mm) | höchste Töne | Q9, „Rashid“ | A, F |
| Lage der Biegestelle | rund 1/8 der Zungenlänge vom Fuß entfernt; bei den höchsten Tönen bei 5/8 der Länge zur Spitze hin | tief → hoch | Q9, „Rashid“ | A, F |
| Zungenform | hohe Zungen eher trapezförmig; Dicke bei hohen Tönen von der Biegestelle bis zur Spitze etwa gleich; bei tiefen Tönen an der Spitze dicker als an der Biegestelle | qualitativ | Suchergebnis zu Q9 | Z, F |
| Zahl verschiedener Plattengrößen | bis 65 verschiedene Größen für 41 Diskanttöne (beste Instrumente); bei billigen Stimmstöcken 8 bis 9 gleich große Platten nebeneinander | Diskant | Q10, „Ventura“, 5.12.2023 und 29.3.2025 | A, F |
| Bandoneon zum Vergleich | 276 Zungen für rund 71 Töne, nur 15 Zungengrößen | 142-töniges Bandoneon | Q15 | A |
| Längste / kürzeste Zungen | tiefste 5 bis 10 cm, höchste unter 6 mm | Akkordeon allgemein | Philharmonie de Paris, `accordeon-mai-23.pdf` | Z |

### 2.3 Werkstoff, Fertigung

| Größe | Wert | Quelle, Stelle | Kz. |
|---|---|---|---|
| Zungenstahl | blau angelassener Federstahl, Härte HRC 48–52 | Q9, „Rashid“ | A, F |
| Zungenstahl | schwedischer Blaustahl; Niet und Zunge Stahl, Platte Aluminium | Q12 | A |
| Bandstahl | heute Band C100S, 120 mm breit; früher für „a mano“ schmales Band 8 mm | Q10, „SteveBox“ (gibt eine Herstellerauskunft wieder), 29.3.2025 | A, F |
| Fertigungstoleranz | Hundertstelmillimeter; über 400 verschiedene Töne/Profile im Programm; je Ton ein eigenes Profil | Q11 | A |
| Platte | Avional (Al mit 4,75 % Cu) für Handzungen; Messingplatten für hohe Piccolo-Zungen | Q12 | A |
| Platte (russisch) | Flugzeug-Aluminium, rund 150 Brinell | Q9, „Rashid“ | A, F |

### 2.4 Schlitz, Seitenspalt, Plattendicke

| Größe | Wert | Einheit | Quelle, Stelle | Kz. |
|---|---|---|---|---|
| Seitenspalt Zunge–Schlitz, beste Handzungen | bis herab zu 0,02 | mm | Q9, „Rashid“ | A, F |
| Wirkung des Spalts | von 0,03 auf 0,05 mm soll die Hälfte der Obertöne wegfallen | mm | Q9, „Rashid“ (Behauptung ohne Messung) | A, F |
| Toleranz des Spalts | bis 5/10 000 Zoll (0,013 mm) | — | Q12 | A |
| Schlitzwand | nach unten öffnend, Winkel 4–5° | Grad | Q9, „Rashid“; Rückfrage J. Pascher | A, F |
| Plattendicke | etwa gleich dem größten Ausschlag der jeweiligen Zunge; dicker/dünner kostet etwas Lautstärke | — | Q9, „Rashid“ | A, F |
| Spalt als Rechenwert | Kanalbreite 2·0,0001 m (also Spalt 0,1 mm) | m | Q6 S. 6 (Abschätzung der Reynoldszahl; **schon bekannt**) | B |

Zahlen zu **Schlitzlänge, Schlitzbreite und Plattendicke je Ton**: nicht gefunden.

### 2.5 Verwandte Zungen (kein Akkordeon — nur Größenordnung)

| Größe | Wert | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|---|
| Harmoniumzunge, Länge | rund 4 cm | A♯ (118 Hz) und C3 | Q3 S. 1; Q4 S. 1, Abb. 1 | B |
| Harmoniumzunge C3, Breite b | 4 mm | 131–137 Hz | Q4 S. 3 (Parameterliste); Q5 S. 762 (4,0 mm) | B |
| dieselbe, Federkonstante an der Spitze | 150 N/m | Eigenfrequenz ω = 860 1/s (136,9 Hz) | Q5 S. 761 | B |
| dieselbe, wirksame Masse | 2,1·10⁻⁴ kg | — | Q5 S. 761 | B |
| dieselbe, Dämpfungszahl D | 0,012 | — | Q5 S. 762 | B |
| dieselbe, wirksame Länge a (aus Fläche S = a·b) | 8,0 mm | — | Q5 S. 762 | B |
| Konzertina (Lachenal), Zungenstahl | meist rund 0,48 mm Ausgangsdicke, Bass 0,82 mm; Rahmen rund 2,08 mm | ganzer Satz | Q17 (Werte im Text; Tabellen nur als Bild) | A |

---

## 3 Aufbiegung / Löseabstand

| Größe | Wert | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|---|
| Ruhelage der Spitze über der Platte | etwa gleich der Zungendicke (Faustregel) | allgemein | Suchergebnis; als Herkunft nennt das Werkzeug eine Reparaturanleitung und Forenbeiträge (accordionists.info), **die Stelle selbst habe ich in keinem Abruf wiedergefunden** | Z, F |
| dieselbe Regel, Mundharmonika | Spalt etwa gleich der Dicke der Zungenspitze; lange Zungen brauchen mehr | Mundharmonika | Suchergebnis (dummies.com) | Z |
| Wirkung | zu groß: Zunge spricht spät an, klingt dann aber ohne Zögern; Aufbiegung ist eine häufige Ursache langsamer oder schwacher Töne | allgemein | Q10, *Slow reeds*, „debra“ 22.7.2022 (nur der zweite Halbsatz als Stelle geliefert, ohne Zahl) | A, F |
| Ruhelage als Rechen-/Messwert | 0,3 mm bei C4 | C4 | Adachi u. a. 2026 (**schon bekannt**) | — |
| Ruhelage Harmonium | verschiebt sich im Betrieb um 1–2 mm in Strömungsrichtung | 100–200 Hz | Q3 S. 1 | B |

**Zahlen je Tonhöhe: nicht gefunden.** Die Regel „Aufbiegung ≈ Zungendicke“ ist nur als Zusammenfassung belegt.

---

## 4 Basszungen und Gewichte

| Größe | Wert | Einheit | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|---|---|
| Gewichte grundsätzlich | die tiefsten Zungen tragen eine Last an der Spitze, um die Frequenz ohne Mehrlänge zu senken | — | tiefste Töne | Q1 S. 2 (mit Verweis auf Benetoux) | B |
| Bild dazu | in dem gezeigten Stimmstock sind die drei tiefsten Zungen beschwert | — | — | Q1 S. 2, Figura 1 | B |
| Wirkung der Last (Theorie) | (f₁/f₁′)² ≈ 1 + 4,1·(M/m) | — | Last M an der Spitze, Zungenmasse m | Q1 S. 6, Gl. 3 | B |
| Wirkung der Last (Messung) | (f₁/f₁′)² = 3,7·(M/m) + 0,98 | — | Zungen 1 und 2 | Q1 S. 6, Abb. 5 (Ausgleichsgerade) | B |
| Last an der Spitze, Zunge 1 | M/m 0,33 → −663 Cent; 0,19 → −438; 0,14 → −336 | Cent | 142,7 Hz | Q1 S. 7, Tabla 4 | B |
| Last an der Spitze, Zunge 2 | M/m 0,80 → −1188 Cent; 0,44 → −780; 0,32 → −701; 0,05 → −119 | Cent | 226,4 Hz | Q1 S. 7, Tabla 4 | B |
| Last in Zungenmitte | Zunge 1: −105 / −64 / −64 Cent; Zunge 2: −292 / −179 / −110 / −38 Cent (gleiche M/m wie oben) | Cent | — | Q1 S. 7, Tabla 4 | B |
| Klangfolge der Last | Verhältnis gerade/ungerade Teiltöne 0,8–0,9 mit Last, rund 0,4 ohne | — | — | Q1 S. 7 | B |
| Länge der E1-Zunge | 60 (Bugari 540/ARS/C) und 80 (AKKO Super de luxe), gemessen hinter den Nieten bis zur Spitze | mm | E1 | Q8, „debra“ #11, #13 (26.2.2018) | A, F |
| Basszungen Bauart Morino | alle gleich groß: 60 × 6 schwingende Fläche; Gewicht auf E1 14 × 2; auf E♭1 und D1 länger | mm | E1 … | Q8, „Geronimo“ #12 | A, F |
| Morino VI M, Basszunge | durchgehend 6 breit, ohne Verjüngung | mm | E1 | Q8, „Morne“ #5 (weitere Maße nur im Foto) | A, F |
| kürzere Basszunge | 53 gegenüber 60 beim Morino | mm | tiefster Ton G | Q8, „Sebastian Bravo“ #9 | A, F |
| größtes lieferbares Ventil (Italien) | 92,5 | mm | Anhalt für die längste Zunge | Q8, „debra“ #4 (mit eigenem Vorbehalt) | A, F |
| Ansprache | 1 s Verzögerung bei E1 gilt nicht als normal; je stärker beschwert, desto träger | s | E1 | Q8, „debra“ #7 | A, F |
| Basszungen ohne Gewicht | E2 war in einem Instrument eine flache Zunge ohne Gewicht | — | E2 | Q8, „Sebastian Bravo“ #1 | A, F |
| Stimmplatten Bass (Museum) | Platten 5,2–6,7 lang, 1,7–2,2 breit; eine Reihe 1,9–6,9 × 1,8–2,3 | cm | Stradella, historische Sätze | Lombardia Beni Culturali (mehrere Karteikarten) | Z |

**Ab welchem Ton beschwert wird:** keine belegte Grenze gefunden. Einziger Anhalt: Zunge 1 aus Q1 (142,7 Hz, unbeschwert, 42 mm) und das Forenbeispiel E2 ohne Gewicht. **Masse der Gewichte in Gramm, Helikon-Bässe:** nichts gefunden.

---

## 5 Kanzellen (Stimmstock)

**Für das Akkordeon habe ich keine Maßtabelle je Ton gefunden.** Gefunden: eine gerechnete Beispielreihe für die Konzertina und Regeln.

### 5.1 Beispielmaße einer Kanzelle über den Umfang (Konzertina, „vernünftige Werte“ des Autors, keine Messung)

Q2 S. 15, Table 1, jeweils die Zeile des Grundtons (Teilton 1). Alle Längen dort in **Zoll**; die Umrechnung in mm ist von mir (× 25,4).

| Ton | f (Hz) | Breite W | Länge L | Lochdurchmesser d | Lochlänge t | Höhe „H fixed“ | in mm: W × L × H, d |
|---|---|---|---|---|---|---|---|
| G1 | 49 | 1,00 | 3,00 | 0,40 | 0,25 | 0,80 | 25 × 76 × 20, 10 |
| G2 | 98 | 1,00 | 2,50 | 0,40 | 0,25 | 0,75 | 25 × 64 × 19, 10 |
| G3 | 196 | 1,00 | 2,00 | 0,30 | 0,25 | 0,50 | 25 × 51 × 13, 7,6 |
| G4 | 392 | 1,00 | 1,80 | 0,30 | 0,25 | 0,50 | 25 × 46 × 13, 7,6 |
| C5 | 523 | 1,00 | 1,70 | 0,30 | 0,25 | 0,38 | 25 × 43 × 9,7, 7,6 |
| C6 | 1046 | 1,00 | 1,50 | 0,32 | 0,20 | 0,20 | 25 × 38 × 5,1, 8,1 |
| C7 | 2093 | 1,00 | 1,25 | 0,32 | 0,18 | 0,15 | 25 × 32 × 3,8, 8,1 |
| C8 | 4186 | 0,80 | 1,00 | 0,32 | 0,13 | 0,15 | 20 × 25 × 3,8, 8,1 |

Kennzeichen: **B**. Einschränkungen aus der Quelle selbst: Die Werte sind gewählte Beispiele für die Rechnung (Q2 S. 16); eine Konzertina-Kanzelle fasst zwei Zungen nebeneinander (daher die Breite 1 Zoll); der Kopfvermerk der Netzfassung sagt, dass zwei Gleichungen der Druckfassung fehlerhaft waren und hier berichtigt sind (Q2 S. 1). Eine wirkliche Kanzelle hätte nach Q2 S. 16 eine Höhe um 0,5 Zoll.

### 5.2 Regeln

| Aussage | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|
| Helmholtz-Resonanz der Kanzelle: f₀ = (c/2π)·√(A / (V·(t + k·d))), k rund 0,43–0,80 | — | Q2 S. 9, Gl. 2 | B |
| Viertelwellen-Resonanz: λ₀/4 = L + t + k·d | — | Q2 S. 11–12, Gl. 4, 5 | B |
| Liegt die Helmholtz-Resonanz auf oder etwas unter dem Grundton, wird die Zunge gestört bis erstickt; etwas darüber nicht (eigene Versuche des Autors) | — | Q2 S. 7–8 | B |
| Störung durch den Grundton ist zwischen C5 und C6 zu erwarten (für die Beispielmaße); ab etwa C7 Viertelwellen-Resonanz mit dem Grundton | C5–C8 | Q2 S. 18–19 | B |
| Abhilfen der Bauer: Entlüftungsloch oder abgefeilte Zungenecke; Ventile an beiden Zungen weglassen; Kanzellenhöhe aufs Kleinste; Zungenspitze nahe an der Luftöffnung | hohe Töne | Q2 S. 4 | B |
| Beim Akkordeon liegt die ganze Zunge außerhalb der Kanzelle | — | Q2 S. 9 | B |
| Resonanzen der Kanzellen sind wegen ihrer Maße unwirksam, ausgenommen die viergestrichene Oktave | ab c⁴ | Q7 S. 852 (mit Verweis auf Richter, Teil 1, Kap. 5.3) | B |
| Kanzellen-Resonanzen liegen hoch; bei hohen Akkordeonzungen kann eine Resonanz nahe der Zungenfrequenz liegen und das Anschwingen hemmen oder verhindern | hohe Töne | Cottingham, ISMA 2017, Beitrag 52 (nur Kurzfassung, 1 Seite) | B |
| Piccolo-Kanzellen 1,5–2 mm tief; Resonanzen der Kanzellen 3- bis 4-mal höher als der Ton | Piccolo | Q9, unbenannter Absender, 9.12.2004 | A, F |
| Masse des Stimmstocks: 103–451 g ohne Platten, sieben baugleiche Stöcke aus verschiedenen Werkstoffen; Einfluss auf den Schall klein | Steirische | Q7 S. 853, Table 1, und S. 855 | B |
| Dichtere Werkstoffe als Holz sind unkritisch, deutlich leichtere nach Richter zu meiden | — | Q7 S. 852 | B |

**Öffnung (Klappenloch) je Ton, Stufung von Tiefe/Breite/Länge beim Akkordeon:** nichts Belegtes. Schon bekannt bleibt der eine Wert von Adachi (14,8 × 14,6 mm² bei C4).

---

## 6 Balgdruck im Spiel

**Eine Messung des Balgdrucks am gespielten Akkordeon von pp bis ff habe ich nicht gefunden.**

| Größe | Wert | Einheit | Geltung | Quelle, Stelle | Kz. |
|---|---|---|---|---|---|
| typischer Spieldruck, Harmoniumzunge im Instrument | 0,6–1,0 | kPa | Zungenorgel, C3 | Q5 S. 762 | B |
| „etwa normaler Spieldruck“ | 0,8 | kPa | Zungenorgel, A♯ 118 Hz | Q3 S. 4 | B |
| Ansprechschwelle | rund 0,2 | kPa | Harmoniumzungen 100–200 Hz | Q3 S. 1 | B |
| Anfachung wird positiv ab | ≈ 0,05–0,1 | kPa | Harmonium C3 | Q4 S. 4, Abb. 4 (abgelesen) | B |
| Winddruck einer Tanzorgel mit Akkordeon | 2–3 Zoll Wassersäule (0,5–0,75 kPa) | — | Orchestrion; Angabe „anderswo gefunden“ | Q14 | A, F |
| Balgdruck im Spiel, Instrument **ohne Zungen** | meist 100–220, höchstens rund 260 | Pa | MIDI-Akkordeon, Barometer-Sensor BMP180; der Autor sagt selbst, ein echtes Akkordeon brauche mehr Kraft | Q13 | A, F — **nicht übertragbar** |
| Dynamik als Schallpegel | mezzoforte rund 70 dBA, piano rund 55 dBA in 50 cm | dBA | Pigini Sirius | Q1 S. 2 | B |
| Messbereich Laborversuch | 0–3 kPa (0–40 cm WS) | — | Harmonium C3 | Q4 S. 3, Abb. 3 | B |

**Luftverbrauch je Zunge:** nur eine Formel für das Harmonium — der Gleichanteil des Volumenstroms folgt U₀ ∝ P₀^z mit z = 0,67 (Q4 S. 3, Q5 S. 761; B), ohne Vorfaktor in Zahlen. Braasch & Cottingham 2023 nennen laut Suchergebnis für eine Mundharmonika-Kanzelle „einige hundert Milliliter je Sekunde“ (Z; PDF nicht gelesen).

---

## 7 Einschwingzeiten, Amplitude, Tonhöhe über dem Druck

### 7.1 Einschwingzeiten (Akkordeon)

Q1 S. 3: Zeit des ersten Teiltons von −50 dB bis −5 dB unter dem Höchstpegel, Ton **A4**, Pigini Sirius. Kennzeichen **B** (Tabla I).

| Anschlag | mezzoforte | piano |
|---|---|---|
| Finger (Balg vorgespannt, dann Taste) | 60 ms | 90 ms |
| Finger, langsam gedrückt | 80 ms | 220 ms |
| Balg (Taste zuerst, dann Balg) | 390 ms | 410 ms |

- Höhere Teiltöne von A4, mezzoforte: Fingeranschlag rund 100 ms, Balganschlag 400–600 ms (Q1 S. 4, Text und Abb. 3; B).
- Q1 S. 3 sagt, die übrigen gemessenen Töne verhielten sich ähnlich wie A4, nennt aber keine Zahlen je Ton — die stehen in der Arbeit Acta Acustica 2014 (nicht erreicht).
- Basszungen: E1 mit rund 1 s Verzögerung wird als Mangel beschrieben (Q8 #1, #7; A, F); „über 1 s“ bei tiefen und mittleren Basszungen eines Instruments (Q10, *Slow reed – tip clearance issue?*, 23.10.2024; A, F).
- Harmonium C3: Anfachungszahl ε (1/s) über dem Druck, abgelesen aus Q4 S. 4, Abb. 4: ≈ 4,5 bei 0,2 kPa; ≈ 8,5 bei 0,5 kPa; ≈ 11,5 bei 1,0 kPa (Höchstwert); ≈ 10,3 bei 1,5; ≈ 9 bei 2,0; ≈ 7 bei 2,5; ≈ 3,3 bei 3,0 kPa (B, Ablesung). Der Text nennt das Maximum bei 1,0–1,5 kPa. Daraus folgt eine Zeitkonstante des Anschwingens von rund 90–220 ms.

### 7.2 Schwingungsamplitude

| Größe | Wert | Druck | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|---|---|
| Amplitude nach der Schwelle | springt auf einige Millimeter und bleibt dann nahezu gleich, sinkt erst bei hohem Druck | ab 0,2 kPa | Harmonium 100–200 Hz | Q3 S. 1 | B |
| Ausschlag in Zungenmitte (2 cm vor der Spitze) | ≈ +1,0 … +1,15 mm über und ≈ −1,2 … −1,45 mm unter der Ruhelage | 3–12 cm WS | A♯ | Q3 S. 2, Abb. 2 (abgelesen) | B |
| dasselbe bei hohem Druck | oberer Ausschlag fällt auf ≈ +0,4 mm, Ruhelage wandert auf ≈ −0,6 mm | 30 cm WS | A♯ | Q3 S. 2, Abb. 2 (abgelesen) | B |
| Amplitude an der Spitze (Laser) | ≈ 1,6 mm bei 4,2 cm Zungenkoordinate; Biegelinie wie erster Balkenmode | 4 und 6 cm WS | A♯ | Q3 S. 3, Abb. 5 | B |
| Bewegung | bei 0,8 kPa nahezu sinusförmig; bei 1,2 kPa unregelmäßiger und etwas kleiner | 0,8 / 1,2 kPa | A♯ 118 Hz | Q3 S. 4, Abb. 6 | B |
| zweiter Balkenmode | bei Druck über dem üblichen sichtbar, f₂ = 6,27·f₁ | 0,9 kPa | C3 | Q3 S. 2–3, Abb. 4 | B |

Hinweis zur Gegenprüfung: Abb. 6 in Q3 beschriftet die Achse mit cm/s und zeigt rund ±1100. Aus 1,6 mm bei 118 Hz folgen rund 1,2 m/s — die Achse meint also wohl mm/s. Die Abb. 2 und 5 widersprechen sich scheinbar (1,0–1,5 mm in der Mitte, 1,6 mm an der Spitze); Abb. 2 gibt den Ausschlag je Richtung an der Messstelle, Abb. 5 eine Amplitude aus der Geschwindigkeit bei kleinerem Druck. **Für das Akkordeon selbst: keine neue Messung gefunden** (bekannt bleibt Cottingham 2011, rund 15 % der Zungenlänge).

### 7.3 Tonhöhe über dem Druck

| Zunge | Verlauf | Steigung | Quelle, Stelle | Kz. |
|---|---|---|---|---|
| Harmonium C3 (Versuch) | ≈ 132 Hz nahe der Schwelle, fällt auf ≈ 128 Hz bei ≈ 25 cm WS, steigt darüber steil auf ≈ 134,5 Hz bei ≈ 42 cm WS | ≈ −1,6 Hz/kPa im fallenden Ast (von mir aus den Endpunkten; rund −21 Cent/kPa) | Q4 S. 3, Abb. 3 | B |
| Harmonium C (Wolfinger), Versuch | ≈ 136,8 Hz bei 0,1 kPa bis ≈ 134,3 Hz bei 1,2 kPa | ≈ −2,3 Hz/kPa (von mir; rund −29 Cent/kPa) | Q5 S. 762, Abb. 2 | B |
| dieselbe, Modell | ganz unten ein kleiner Bereich steigender Frequenz, dann Abfall, Minimum bei ≈ 2 kPa, danach Anstieg | — | Q4 S. 4, Abb. 5; Text S. 3 | B |
| Akkordeon A♯2–A3, Bending mit Balg und Finger | −16,0 bis −33,8 Cent, Mittel −22,4 (Q1) bzw. −24,4 (Q6) | kein Druck gemessen | Q1 S. 5, Tabla 2; Q6 S. 4, Table 1 (**schon bekannt**) | B |
| Akkordeon C6–C♯8 | kein Bending möglich | — | Q6 S. 5, 6 | B |

Die beiden Mittelwerte −22,4 und −24,4 Cent stammen aus zwei Veröffentlichungen derselben Gruppe zur selben Messreihe und weichen voneinander ab; Höchst- und Kleinstwert (−33,8/−33,7 und −16,0) stimmen überein.

Damit liegt die Tonhöhe über dem Druck für **zwei Harmoniumzungen** (beide um C3) und — schon bekannt — **eine Akkordeonzunge** (C4, −2,5 Hz/kPa) vor. Für weitere Tonhöhen des Akkordeons: nichts gefunden.

---

## 8 Lederventile

| Aussage | Ton | Quelle, Stelle | Kz. |
|---|---|---|---|
| Bei mittleren und großen Zungen deckt ein Streifen aus Kunststoff oder Leder den Schlitz der nicht tätigen Zunge | mittel, tief | Q1 S. 1–2 | B |
| Die Töne C6 bis C♯8 haben Zungen ohne Streifen | C6–C♯8 | Q6 S. 6 | B |
| Ohne Streifen strömt Luft durch den weiten Spalt an der untätigen Zunge; deshalb dort kein Bending | C6–C♯8 | Q6 S. 6 | B |
| Ventile entfallen typisch ab etwa C6 oder D6; an den ersten ventillosen Tönen fällt der Luftverlust billiger Zungen am meisten auf | ab C6/D6 | Q10, „debra“ 11.3.2025 | A, F |
| Sehr hohe Töne sprechen nur an, wenn die Nachbarzunge **kein** Ventil hat | sehr hoch | Q10, *Slow reeds*, „debra“ 22.7.2022 | A, F |
| Ein nicht richtig schließendes Ventil macht den Ton schwer ansprechend und schwach, vor allem bei tiefen Tönen | tief | Q10, *Slow reeds*, „debra“ | A, F |
| Ventile an beiden Zungen wegzulassen ist ein Mittel gegen das Ersticken hoher Zungen (wirkt wie ein Entlüftungsloch) | hoch | Q2 S. 4 | B |
| Für eine gewollte Kanzellen-Resonanz müssen beide Zungen Ventile tragen | — | Q2 S. 5–6 | B |
| Große Ventile tragen Andrückfedern: Stahlfolie 0,0015 Zoll (Italien), Filmstreifen (Russland); Kunststoffventile aus 2–3 Lagen von 0,001–0,003 Zoll | tief | Q9, „Rashid“ | A, F |

---

## 9 Nicht gefunden / nicht erreicht

**Nicht erreicht**

| Was | Adresse | Grund |
|---|---|---|
| Ricot, Caussé, Misdariis, JASA 117 (2005) 2279–2290, Volltext | `hal.archives-ouvertes.fr/hal-01106161`, `hal.science/hal-01106161/document` | Bot-Schutz (Anubis) weist ab |
| dasselbe bei IRCAM | `articles.ircam.fr/textes/Ricot05a/index.pdf` (geratene Adresse) | Verbindung verweigert |
| HAL-Dokument hal-01613185 (Inhalt unbekannt, Treffer zur Balgdruck-Suche) | `hal.sorbonne-universite.fr/hal-01613185/document` | Bot-Schutz |
| Llanos Vázquez, Dissertation *Acústica del Acordeón*, UPV/EHU 2015 | `addi.ehu.es/handle/10810/16562` (auch `addi.ehu.eus`, `?show=full`) | HTTP 404 beim Abruf; die Suche kennt die Seite |
| TU Delft, Abschlussarbeit zu Pigini-Akkordeons (laut Suche mit Anhang zur Kanzellentiefe nach Tonon) | `repository.tudelft.nl/file/File_696f9fd3-6fab-4d0d-9d5c-0d49104301b7` | Verbindung verweigert |
| Acta Acustica 2025, 9, 5 (Suchtreffer) | `acta-acustica.edpsciences.org/10.1051/aacus/2024082` | HTTP 403; nachträglich als **Fehltreffer** erkannt (Klarinetten-/Saxophonblatt, Gazengel u. a.) |
| Zwei Tagungsbeiträge SEA, Guimarães 2004 (ID9, ID10; Inhalt unbekannt) | `sea-acustica.es/archivo/publicaciones/Guimaraes04_ID9.pdf`, `…ID10.pdf` | HTTP 429 |
| 1accordion.net, Seite zum Stimmen | `1accordion.net/tuning.html` | Zertifikatsfehler |
| Vortragsfolien einer Dissertation (Suchtreffer zu Tonon) | `caelinux.com/…/Presentation-these.pdf` | HTTP 503 |
| Llanos-Vázquez/Elejalde/Macho, *Controllable pitch-bending effects in the accordion playing*, Acoustics'08 Paris | `conforg.fr/acoustics2008/cdrom/data/articles/001900.pdf` | abgerufen (436 KB), aber **nicht gelesen**; `curl -sI` über http meldet 404 |
| Braasch & Cottingham, *Free Reeds*, Acoustics Today 2023 | siehe Ladeliste | abgerufen, **nicht gelesen** |
| Philharmonie de Paris, `accordeon-mai-23.pdf` | `ressourceseveil.philharmoniedeparis.fr/…` | abgerufen, **nicht gelesen** (nur Zusammenfassung) |
| patmissin.com | — | wegen No-AI-Vermerk nicht aufgerufen |

**Nicht gefunden (trotz Suche)**

1. **Maßtabelle der Zungen je Ton** (Länge, Breite an Fuß und Spitze, Dicke/Profil) — bei keinem Hersteller (Voci Armoniche, Harmonikas Louny/Titlbach, Binci, Cagnoni, Antonelli) öffentlich; Foren sagen, Harmonikas fertige nach Maßangabe in mm, veröffentliche aber nichts.
2. **Schlitzmaße und Plattendicke je Ton.**
3. **Aufbiegung je Tonhöhe** als Zahl; die Faustregel „≈ Zungendicke“ nur als Zusammenfassung.
4. **Masse und Maße der Bassgewichte**, Grenzton der Beschwerung, **Helikon-Bässe**.
5. **Kanzellenmaße je Ton beim Akkordeon**, Klappenloch je Ton.
6. **Balgdruck pp–ff am gespielten Akkordeon**, Höchstdruck, **Ansprechdruck je Tonhöhe**, **Luftverbrauch je Zunge**.
7. **Einschwingzeit je Tonhöhe und Druck** (nur A4 bei zwei Lautstärken).
8. **Amplitude der Spitze über dem Druck am Akkordeon.**
9. Arbeiten des **IfM Zwota** zum Akkordeon: zwei Suchen, nur fachfremde Treffer.
10. Richter, *Akkordeon — Handbuch für Musiker und Instrumentenbauer*: kein Netzinhalt; gefunden nur der Verweis auf seine zwei Bände *Akustische Probleme …* (1985).
11. Russische Quellen: der Forenhinweis, es habe schon 1934 Untersuchungen zur genauen Größe der Resonatoren und veröffentlichte „optimale Zungengrößen“ gegeben (Q9, „Rashid“; A, F), ohne Titel.

**Fehltreffer (gelesen, ohne Nutzen):** Gazengel u. a., Acta Acustica 2025 (Einfachrohrblatt); Pàmies-Vilà u. a., ISMA 2017 Beitrag 8 (Klarinettenblatt); DAFx25 Beitrag 25 (Anschlag-Schätzung, kein Akkordeon); Patente US 2 787 928, DE 855 499 (ohne Maße).

---

## 10 Ladeliste

Größe aus `curl -sI` (Content-Length) am 11.11.2026.

| Nr. | URL | Bytes | Wofür | Rechtslage |
|---|---|---|---|---|
| L1 | `https://www.sea-acustica.es/archivo/publicaciones/AMS-0_003_CI.pdf` | 603 465 | Q1: zwei Akkordeonzungen mit L, m, f; Einschwingzeiten A4; beschwerte Zungen (Formel, Tabla 4) | kein Vermerk im Dokument gefunden |
| L2 | `https://www.concertina.org/wordpress/wp-content/uploads/2020/07/reed_cavity_design_resonance.pdf` | 677 222 | Q2: Kanzellen-Resonanz, Formeln, Table 1 über G1–C8 | im PDF keiner; Seite: Copyright International Concertina Association |
| L3 | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF` | 607 160 | Q3: Amplitude über Druck, Schwelle, Biegelinie (Harmonium) | kein Vermerk gefunden |
| L4 | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF` | 671 902 | Q4: Frequenz über Druck, Anfachung über Druck, Parameterliste | kein Vermerk gefunden |
| L5 | `https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF` | 156 110 | Q5: Federkonstante, wirksame Masse, Dämpfung, Spieldruck 0,6–1,0 kPa | kein Vermerk gefunden |
| L6 | `https://www.euracoustics.org/documents/14/AiP_2021_02_Accordion_acoustics.pdf` | 1 106 560 | Q6: Ventile fehlen C6–C♯8; Bending je Ton (falls noch nicht in der Ablage) | S. 1: alle Rechte vorbehalten, Nutzung durch Dritte nur mit schriftlicher Zustimmung |
| L7 | `https://pub.dega-akustik.de/DAGA_2021/data/articles/000550.pdf` | 3 925 229 | Q7: Stimmstock-Werkstoff; Verweis auf Richter Kap. 5.3 (nachrangig) | kein Vermerk gefunden |
| L8 | `https://acousticstoday.org/wp-content/uploads/2023/11/Free-Reeds-An-Intertwined-Tale-of-Asian-and-Western-Musical-Instruments-Jonas-Braasch-and-James-P.-Cottingham-3.pdf` | 406 964 | ungelesen; laut Suche Volumenstrom-Verlauf einer Akkordeonzunge und Durchsatz-Größenordnung — **vor dem Ablegen prüfen**, ob es der ganze Aufsatz ist (das Werkzeug meldete nur 1 Seite) | nicht geprüft |

Nicht für die Ablage, aber als Fundstellen festzuhalten (Foren, Netzseiten): Q8–Q16.

**Beschaffung außerhalb des Netzes (aus meiner Sicht der eigentliche Engpass):** Richter, *Akustische Probleme bei Akkordeons und Mundharmonikas*, Teil 1 und 2 (Schmülling 1985); Richter, *Akkordeon — Handbuch …*; Benetoux, *The Ins and the Outs of the Accordion* (2002); Llanos-Vázquez u. a., Acta Acustica united with Acustica 100 (2014) 375–384; Ricot u. a., JASA 2005; die Dissertation Llanos Vázquez (UPV/EHU 2015, im Repositorium ADDI, mit Browser vermutlich erreichbar).

---

## 11 Eigene Einschätzung

*Dieser Abschnitt ist meine Bewertung, kein Befund aus den Quellen. Die Rechnungen darin sind von mir und ungeprüft.*

**Reicht das, um Länge, Breite, Dicke, Aufbiegung und Kanzelle je Taste für ein Pianoakkordeon (Diskant F3–A6, Bass F1–E3) herzuleiten? Nein — nicht als belegte Vorgabe je Taste.** Es reicht für eine **gestützte Mensur mit wenigen Ankern**, deren Zwischenwerte gerechnet und als solche ausgewiesen werden müssen.

**Was trägt**

- **Länge:** vier Anker im und unter dem Diskant — 42 mm bei 142,7 Hz, 37 mm bei 226,4 Hz (Q1), 32,8 mm bei 267,6 Hz (Adachi), dazu Harmonium rund 40 mm um 118–131 Hz als Größenordnung; im Bass 53–80 mm bei E1 aus Foren. Auffällig: Zwischen 226 und 268 Hz fällt die Länge um 11 % bei 18 % Frequenzzuwachs (Exponent rund −0,7), zwischen 143 und 226 Hz nur um 12 % bei 59 % (Exponent rund −0,3). Die Länge folgt also **keinem** einheitlichen Potenzgesetz; im tiefen Bereich übernimmt das Profil (und im Bass das Gewicht) die Stimmung. Zwei Instrumente, drei Zungen — das ist für eine Kurve dünn, und oberhalb von C4 gibt es **gar keinen** Messpunkt bis auf die Aussage „unter 6 mm ganz oben“ (nur Zusammenfassung).
- **Breite:** nur ein Messwert (3,45 mm bei C4) und die Verhältnisregel 8–10 : 1, gestützt durch zwei kleine Reihenmessungen an Konzertinas (Mittel 7,4 und 8,6). Das trägt eine Vorgabe „Breite = Länge / 8…10“ mit Belegkraft *Faustregel*; der Bass weicht ab (60 × 6 mm = 10 : 1 passt noch).
- **Dicke:** kein gemessener Wert je Ton. Rückrechnen lässt sie sich aus Länge und Frequenz über den einseitig eingespannten Balken: gleichmäßige Ersatzdicke rund 0,31 mm (Q1 Zunge 1), 0,38 mm (Zunge 2), 0,35 mm (Adachi C4). Die Massen passen dazu nur grob — bei Adachi ergibt die Masse eine mittlere Dicke um 0,28 mm, bei Q1 Zunge 1 je nach Breite 0,39–0,49 mm —, was auf Profil (dünner Fuß, dickere Spitze bei tiefen Zungen) oder auf mitgewogene Füße hindeutet. Für ein Modell mit einer Ersatzdicke je Taste genügt die Rückrechnung; ein **Profil** ist damit nicht belegt.
- **Bass:** Die Formel (f/f′)² = 0,98 + 3,7·M/m (Q1, gemessen) erlaubt, aus einer Grundzunge und der Zieltonhöhe die Last zu rechnen — das ist der beste Fund für die Bass-Seite. Geometrie: 60 × 6 mm für die ganze Bassreihe (Bauart Morino) als einfache, durch ein Forum gestützte Annahme.
- **Ventile:** Grenze C6 aus einer begutachteten Quelle (Q6), gestützt durch ein Forum (C6/D6). Für F3–A6 heißt das: Ventile bis H5, ab C6 keine.
- **Betrieb:** Schwelle rund 0,2 kPa und Spieldruck 0,6–1,0 kPa (Harmonium), Versuchsbereich 0–2,5 kPa (Adachi), Tonhöhe −1,6 bis −2,5 Hz/kPa bei drei Zungen zwischen C3 und C4 — in Cent sind das rund −16 bis −29 Cent/kPa, also keine erkennbare Abhängigkeit von der Tonhöhe, aber eben nur über eine Oktave.
- **Einschwingen:** A4 mit 60–90 ms (Finger) und rund 400 ms (Balg) ist ein belastbarer Prüfpunkt für das Modell; der Balganschlag misst allerdings den Druckaufbau des Spielers, nicht die Zunge.

**Was fehlt, nach Gewicht**

1. **Zungenmaße oberhalb C4 und zwischen den Ankern** — die halbe Diskanttastatur (C♯4–A6) hat keinen einzigen Messpunkt. Das ist die größte Lücke.
2. **Dickenprofil** (Schliff) je Lage — bestimmt Teiltonlage des zweiten Modes und die Biegelinie.
3. **Aufbiegung je Ton** — nur ein Rechenwert bei C4 und eine unbelegte Faustregel. Für die Ansprechschwelle des Modells ist das die empfindlichste Größe.
4. **Kanzelle je Ton am Akkordeon** — ein Wert bei C4; Tonons Reihe ist eine Konzertina-Annahme. Die Formeln (Helmholtz, Viertelwelle) erlauben aber, eine angenommene Stufung auf Resonanznähe zu prüfen, und die Aussage „wirksam erst in der viergestrichenen Oktave“ (Richter über Q7) deckt den Umfang bis A6 ab: bis dahin darf die Kanzelle im Modell als reiner Strömungswiderstand gelten.
5. **Balgdruck im Spiel** — keine Akkordeon-Messung. Die Zuordnung pp…ff → Pa bleibt eine Setzung; 0,2–2,5 kPa ist als Rahmen vertretbar, die Stufen dazwischen nicht belegt.
6. **Bass:** Maße und Massen der Gewichte, Grenzton, Helikon.

**Wie die Lücken am ehesten zu schließen sind**

- Die zwei Bände von **Richter 1985** und sein Handbuch sind nach allem, was die Quellen andeuten, die Stelle, an der Mensur- und Kanzellenregeln mit Zahlen stehen (Bibliothek oder Antiquariat).
- Die **Dissertation Llanos Vázquez** dürfte die Zungen von Q1 genauer beschreiben und Anschlagzeiten für mehr Töne enthalten.
- **Selbst messen:** Ein ausgebauter Stimmstock und ein Messschieber liefern Länge, Breite, Aufbiegung und Kanzelle für alle 41 Töne in einer Stunde — genauer als alles, was das Netz hergibt. Dicke am Fuß und an der Spitze mit einer Bügelmessschraube.
- **Hersteller anfragen:** Harmonikas (Louny) fertigt laut Forum nach Maß in mm; eine Maßliste eines Standardsatzes wäre dort am ehesten zu bekommen.
