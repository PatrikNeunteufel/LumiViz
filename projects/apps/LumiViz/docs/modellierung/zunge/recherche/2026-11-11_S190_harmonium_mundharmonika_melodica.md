# Recherche: Maße und Betriebsgrößen der durchschlagenden Zunge — Harmonium, Mundharmonika, Melodica

- **Typ:** Agentenbericht (unverändert abzulegen)
- **Datum:** 2026-11-11
- **Sitzung:** LumiViz S190, Stufe 34j-4
- **Auftrag:** Belegte Maße (Länge, Breite, Dicke, Aufbiegung, Kanzelle) und Betriebsgrößen (Winddruck, Schwelle,
  Amplitude, Einschwingen) der durchschlagenden Zunge je Ton für Harmonium, Mundharmonika und Melodica suchen, als
  Grundlage für Vorgaben je Taste. Nur Recherche, keine Änderung am Code.
- **Werkzeuge:** WebSearch, WebFetch, `curl -sI` (nur Kopfzeilen für Dateigrößen). Kein Browser. Kein git.
- **Hinweis zum Lesen der PDFs:** WebFetch konnte die meisten PDFs nicht als Text liefern (komprimierte Ströme,
  Scans). Das Werkzeug legt den abgerufenen Inhalt von sich aus im Sitzungsordner `tool-results` ab; von dort habe ich
  die Seiten mit dem Lesewerkzeug als Bild/Text gelesen. Im Projekt und in den Ablagen wurde nichts gespeichert. Was
  so gelesen ist, trägt das Kennzeichen **gelesen** (ich habe die Seite selbst gesehen). **Werkzeug-Zitat** heißt: das
  Abrufwerkzeug hat einen Wortlaut geliefert, den ich nicht selbst an der Seite gesehen habe. **Zusammenfassung** heißt:
  nur die Zusammenfassung eines Werkzeugs, kein Beleg.
- **Zitate:** bewusst nur kurze Bruchstücke (Zahl mit zwei, drei Wörtern); die Fundstelle (Seite, Abbildung, Tabelle)
  steht dabei.

## Reichweite der Suche

Rund 45 Suchanfragen (englisch, deutsch, französisch, japanisch) und rund 45 Abrufe. Gesucht nach: Mensurtabellen
für Harmonium- und Reed-Organ-Zungen (Milne, Gellerman, Estey, Mustel, Reed Organ Society, GdO), Winddruck
(mm WS, inches of water), Arbeiten der Gruppe Cottingham (Coe College), Puranik/Scavone, St. Hilaire, Millot,
Johnston, Bahnson, Adachi; Zungen- und Schlitzmaße der Mundharmonika (Harp-L, Foren, Hersteller), Atemdruck; Melodica
(englisch, japanisch, Forum melodicaworld). **Nicht** gesucht bzw. nicht erreichbar: Datenbanken hinter Bezahlschranke
(JASA, JFM, POMA), Reed Organ Society Bulletin, HAL-Volltexte von Millot, Hindi-Quellen (eine Anfrage, kein Treffer),
Patentschriften im Volltext. Kein Treffer ist kein Beweis.

---

## 1 Quellen

| Nr. | Quelle | Zugang | Rechtsvermerk (Fundort) |
|---|---|---|---|
| Q1 | N. Puranik, G. Scavone: *Physically inspired signal model for harmonium sound synthesis*, DAFx23, Kopenhagen 2023. https://www.dafx.de/paper-archive/2023/DAFx23_paper_47.pdf | frei, 4 Seiten **gelesen** | Fußnote S. 1: Open Access unter „Creative Commons Attribution 4.0 International License“ |
| Q2 | S. Adachi, H. Okada, T. Samejima, M. Takeda, K. Nishimiya: *Aerodynamic mechanism of accordion pitch bending: simulation and theoretical analysis*, Forum Acusticum 2026, Graz. https://dael.euracoustics.org/confs/fa2026/data/articles/000418.pdf | frei, 6 Seiten **gelesen**; Wasserzeichen „Preliminary document“ | Fußnote S. 1: „Creative Commons Attribution 3.0 Unported License“ |
| Q3 | J. P. Cottingham: *Theoretical and experimental investigation of the air-driven free reed*, ICA/ASA Seattle 1998, S. 761–762. https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF | frei (Kopie auf Fakultätsseite), 2 Seiten **gelesen** | keiner gefunden |
| Q4 | J. P. Cottingham, C. J. Lilly, C. H. Reed: *The motion of air-driven free reeds* (Berlin 1999, ASA/EAA/DAGA). https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF | frei, 4 Seiten **gelesen** | keiner gefunden |
| Q5 | J. P. Cottingham, C. H. Reed, M. Busha: *Variation of frequency with blowing pressure for an air-driven free reed* (Berlin 1999). https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF | frei, 4 Seiten **gelesen** | keiner gefunden |
| Q6 | H. T. Bahnson, J. F. Antaki, Q. C. Beery: *Acoustical and physical dynamics of the diatonic harmonica*, JASA 103(4), 2134–2144, 1998. https://www.cs.bu.edu/fac/snyder/TablatureWeb/Bahnson%20JASA%201998.pdf | frei (Kopie auf Fakultätsseite), 11 Seiten **gelesen** — war bisher „nicht erreicht“ | Fußzeile: „© 1998 Acoustical Society of America“ — keine freie Lizenz |
| Q7 | J. Cottingham, D. Wolff: *Experimental study of free reed initial transients*, Vienna Talk 2015, S. 176. https://viennatalk2015.mdw.ac.at/abstracts/ViennaTalk2015_submission_39.pdf | frei, nur Abstract (1 Seite) **gelesen** | keiner gefunden |
| Q8 | J. Cottingham: *Reed chamber resonances and reed tongue vibrational modes in free reed excitation*, ISMA 2017, Montreal. https://isma2017.cirmmt.mcgill.ca/proceedings/pdf/ISMA_2017_paper_52.pdf | frei, nur Abstract (1 Seite) **gelesen** | keiner gefunden |
| Q9 | H. F. Milne: *The Reed Organ: Its Design and Construction*, London 1930. https://archive.org/details/reedorganitsdesi0000hfmi (OCR-Text: `…/stream/reedorganitsdesi0000hfmi/reedorganitsdesi0000hfmi_djvu.txt`) | OCR-Text abrufbar, **Werkzeug-Zitat**; Brüche im OCR verstümmelt | keiner angegeben; Autor laut Katalog 1886–1959, also nicht gemeinfrei |
| Q10 | H. Goldbrunner: *Harmonium Reparaturanleitung für das Saugwindharmonium*, 2014, 48 S. https://www.gdo.de/fileadmin/_migrated/content_uploads/Goldbrunner_Harmoniumreparatur.pdf | frei, S. 1–28 **gelesen** | keiner gefunden (S. 1–28) |
| Q11 | *Das Titz-Kunstharmonium. Technische Beschreibung* (Carl Simon, Berlin, o. J.). https://www.harmonium.gdo.de/fileadmin/ak_harmonium/veroeff/Titz-Kunst-Harmonium.pdf | frei, S. 1–6 **gelesen** | keiner gefunden |
| Q12 | M. Fouché: *Étude expérimentale des tuyaux à anche. Phénomène de Weber*, Ann. Fac. Sci. Toulouse (3) 20, 237–336, 1928. https://afst.centre-mersenne.org/item/10.5802/afst.350.pdf | frei, S. 237–243 **gelesen** (100 Seiten insgesamt) | Deckblatt: „© Université Paul Sabatier, 1928, tous droits réservés“, Nutzung nach den Bedingungen von Numdam |
| Q13 | Mechanical Music Digest, H. Wyman, 5.12.1995. https://www.mmdigest.com/Archives/Digests/199512/1995.12.05.07.html | frei, **Werkzeug-Zitat** | keiner gefunden; Forum, geringe Belegkraft |
| Q14 | G.-W. Raes: *\<HarmO\>*, Logos Foundation, 2009/2010 (automatisiertes Saugwind-Harmonium). https://www.logosfoundation.org/instrum_gwr/harmo.html | frei, **Werkzeug-Zitat** | keiner gefunden |
| Q15 | D. Payne: „Interesting little Seydel reedslot legacy“, Harp-L, 5.9.2009. https://harp-l.org/pipermail/harp-l/2009-September/msg00200.html | frei, Tabelle vom Werkzeug ausgelesen (**Werkzeug-Zitat**) | keiner gefunden; Forum, geringe Belegkraft |
| Q16 | K. Nilsson: „Table of reedplate dimensions? *REVISED*“, Forum brendan-power.com, 19.1.2018. https://brendan-power.com/forum/viewtopic.php?p=993 | frei, **Werkzeug-Zitat** | keiner gefunden; Forum, geringe Belegkraft |
| Q17 | *Euphonics: The science of musical instruments*, Abschnitt 11.6.3 „Modelling the harmonica“. https://euphonics.org/11-6-3-modelling-the-harmonica/ | frei, **Werkzeug-Zitat** | Seitenfuß laut Werkzeug: „Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License“ |
| Q18 | bluesharmonica.com, Forum „feeler gauge measurements“, 2012–2018. https://bluesharmonica.com/feeler_gauge_measurements | frei, **Werkzeug-Zitat** | keiner gefunden; Forum, geringe Belegkraft |
| Q19 | B. Power: „Reed Profiling Overview“, Harmonica World, Juni/Juli 2014, S. 10–11. https://www.brendan-power.com/blogs/HWJuneJuly14_Brendan.pdf | frei, **gelesen** | keiner gefunden |
| Q20 | S. Adachi, H. Okada, T. Samejima, K. Nishimiya: *Time-domain simulation of harmonica pitch bending and overblowing*, JASA 159(5), 4191–4203, 2026, DOI 10.1121/10.0043784. https://pubs.aip.org/asa/jasa/article/159/5/4191/3391393/ | **Bezahlschranke**; nur Abstract-Seite, **Zusammenfassung** | laut Werkzeug kein Open-Access-Vermerk |
| Q21 | CMUSE: „Reed Frequency Calculator“, 7.7.2026. https://www.cmuse.org/reed-frequency-calculator/ | frei, **Werkzeug-Zitat** | „© 2026 CMUSE.org“; ohne Quellenangaben, sehr geringe Belegkraft |
| Q22 | melodicaworld.com, Foren „Gapping 101“, „Reed Failure & Gapping“ u. a. | nur Suchtreffer, **Zusammenfassung** | Forum, geringe Belegkraft |
| Q23 | A. J. Hipkins: „Harmonium“, in Grove, *A Dictionary of Music and Musicians*, Bd. 1. https://en.wikisource.org/wiki/A_Dictionary_of_Music_and_Musicians/Harmonium | frei, **Werkzeug-Zitat**; keine Zahlen zu Zunge oder Wind | gemeinfrei (Wikisource) |
| Q24 | pumporganrestorations.com: „Physics of the pump organ“. https://www.pumporganrestorations.com/physics_of_the_pump_organ.htm | frei, **Werkzeug-Zitat** | „All content Copyright © 2009-2026 Pump Organ Restorations · All Rights Reserved“ |

Schon bekannt und nur zum Gegenprüfen benutzt: Puranik & Scavone FA2023, Millot & Baumann (arXiv 2401.01606 — laut
Werkzeug arXiv-Lizenz „non-exclusive distribution“, kein CC), Cottingham Physics Today 2011.

---

## 2 Harmonium / Reed Organ

### 2.1 Zungenmaße

| Wert | Einheit | Ton / Zunge | Quelle, Stelle | Kennzeichen |
|---|---|---|---|---|
| Zungenlänge „about 4 cm“ | cm | Reed-Organ-Zungen 100–200 Hz (A♯2 118 Hz, C3 ≈ 131–137 Hz) | Q4 S. 1; Q5 Abb. 1 Unterschrift | gelesen |
| Profil in Q4 Abb. 5 reicht bis ≈ 4,2 cm | cm | dieselbe Zunge | Q4 Abb. 5 (Achse „reed coordinate [cm]“) | gelesen (aus Abbildung, ±0,1) |
| Breite b = 4,0 | mm | Wolfinger C3 (Reed Organ) | Q3 S. 762; Q5 S. 3 „[4 mm]“ | gelesen, zweifach |
| Eigenkreisfrequenz ω_r = 860 (≈ 136,9 Hz) | s⁻¹ | Wolfinger C3 | Q3 S. 761; Q5 Abb. 5 „reed freq = 136.9 Hz“ | gelesen, zweifach |
| Federkonstante k = 150 | N/m | Wolfinger C3, an der Spitze gemessen | Q3 S. 761 | gelesen |
| wirksame Masse M_r = 2,1·10⁻⁴ | kg | Wolfinger C3 | Q3 S. 761 — **Exponent in der Textebene verloren** („2.1x 104“), am Bild als 10⁻⁴ gelesen; Gegenprobe k/ω² = 150/860² = 2,03·10⁻⁴ kg stimmt | gelesen, gegengeprüft |
| Dämpfung D = 0,012 | – | Wolfinger C3 | Q3 S. 762 | gelesen |
| wirksame Länge a = 8,0 (aus S_r = a·b, **nicht** die geometrische Länge) | mm | Wolfinger C3 | Q3 S. 762 | gelesen |
| Volumenstrom U₀ ∝ P₀^z mit z = 0,67 | – | Wolfinger C3 | Q3 S. 761; Q5 S. 3 | gelesen, zweifach |
| Krümmung nahe der Spitze ist beim amerikanischen Reed Organ üblich | – | alle | Q5 S. 1 | gelesen |
| „Höcker“ am freien Ende, Höhe nimmt mit der Tonhöhe ab, fehlt bei den höchsten Tönen | – | Saugwind | Q10 S. 24 | gelesen |
| Tiefe Zungen breit, dick, lang; nach oben kürzer, dünner, schmaler; drei Mensuren (normal, schmal, breit) im Bild | – | Saugwind | Q10 S. 24, Abb. 3 | gelesen (ohne Zahlen) |
| Werkstoff: Messinglegierung, in Leichtmetall- oder Messingrahmen genietet; Druckwind: auf Metallplatten | – | – | Q10 S. 24 | gelesen |
| Weber-Zunge: Länge 28,3, Breite 5,6, Dicke 0,49; Messing; 388 Hz (sol₃) | mm | freie Zunge, eingespannt | Q12 S. 242 (§ 4) | gelesen |
| Zungenbreite der Diapason-Reihe etwa ein Viertel der schwingenden Länge | – | Reed Organ | Q9 S. 27 | Werkzeug-Zitat |
| Länge der tiefsten C-Zunge: 16′ ≈ 4 in; 8′ „33in“ (vermutlich 3¾ in); 4′ ≈ 3 in | in | tiefstes C je Fußlage | Q9 S. 39 | Werkzeug-Zitat, **OCR-Brüche verstümmelt**; unklar, ob Zunge oder Rahmen gemeint ist |
| Zunge für CC 8′ „about 4in. long“; höchste Diskantzunge unter „4in“ (verstümmelt, wohl ¼ oder ½ in) | in | CC, Diskant | Q9 S. 138 | Werkzeug-Zitat, OCR verstümmelt |
| Belastete Basszungen: Bleigewicht an der Spitze („94 grains“, Zahl unsicher) | – | Bass | Q9 S. 138 | Werkzeug-Zitat, OCR unsicher |
| Harmonium-Rahmen im Bass deutlich dicker als im Diskant; amerikanische Rahmen viel dünner (Zahlen verstümmelt) | – | – | Q9 S. 142 | Werkzeug-Zitat |
| Zungen stets leicht von Ferse zu Spitze gekrümmt; wenig Krümmung = laut, hart | – | – | Q9 S. 141 | Werkzeug-Zitat |
| Amerikanische Zungen auf Druckwind: Zunge leicht in den Rahmen drücken | – | – | Q9 S. 102, 111 | Werkzeug-Zitat |

Zum Vergleich (Akkordeon, Stahl, nicht gesucht, aber vollständiger Satz): C4-Zunge L = 32,8 mm, W = 3,45 mm,
Eigenfrequenz 267,6 Hz, Masse (2/3)·0,248 g, Dämpfungsgrad ζ = 0,003, Spalt Zunge–Platte a = 0,14 mm, Ruhelage
x_rest = 0,3 mm, Kanzellenquerschnitt 14,8 × 14,6 mm² — Q2 Tab. 1, **gelesen**.

### 2.2 Winddruck, Schwelle, Amplitude

| Wert | Einheit | Bezug | Quelle, Stelle | Kennzeichen |
|---|---|---|---|---|
| Schwelle „about 0.2 kPa“ | kPa | Reed-Organ-Zungen 100–200 Hz (Koopman & Cottingham 1997, hier in zweiter Hand zitiert) | Q4 S. 1 | gelesen — die bisher fehlende Stelle; die Primärquelle (ROS Bulletin 15, 3–4, 17–23) bleibt unerreicht |
| „0.6–1.0 kPa“ = Bereich, der die üblichen Spieldrücke der Zunge im Instrument einschließt | kPa | Wolfinger C3 | Q3 S. 762 | gelesen |
| 0,8 kPa „approximately normal playing pressure“ | kPa | A♯2 (118 Hz) | Q4 S. 4 | gelesen |
| bei 1,2 kPa Wellenform unregelmäßiger, Amplitude etwas kleiner | kPa | A♯2 | Q4 S. 4 | gelesen |
| Auslenkung in Zungenmitte (2 cm vor der Spitze): etwa +1,1 / −1,3 mm bei 3–12 cm H₂O; obere Auslenkung fällt bis 30 cm H₂O auf ≈ 0,4 mm, untere wächst auf ≈ −1,7 mm | mm | A♯-Zunge | Q4 Abb. 2 | gelesen (aus Abbildung, ±0,1) |
| Spitzenamplitude: „several millimeters“ gleich nach der Schwelle, dann fast konstant, bei hohem Druck fallend | mm | 100–200 Hz | Q4 S. 1 | gelesen |
| Spitzenamplitude ≈ 1,6 mm bei 4 und 6 cm H₂O | mm | Zunge aus Abb. 5 | Q4 Abb. 5 | gelesen (aus Abbildung) |
| Ruhelage wandert um 1–2 mm in Strömungsrichtung | mm | 100–200 Hz | Q4 S. 1 | gelesen |
| Spitzengeschwindigkeit ≈ ±1,1 m/s (Achse in cm/s) | m/s | A♯2, 0,8 kPa | Q4 Abb. 6a | gelesen (aus Abbildung) |
| Frequenz fällt von ≈ 132 Hz (nahe 0) auf ≈ 128 Hz (≈ 25 cm H₂O), steigt darüber wieder (≈ 134 Hz bei ≈ 41 cm H₂O) | Hz | C3, gemessen | Q5 Abb. 3 | gelesen (aus Abbildung) |
| Frequenz ≈ 136,5 Hz (0,3 kPa) → ≈ 134,5 Hz (1,2 kPa) | Hz | Wolfinger C3 | Q3 Abb. 2 | gelesen (aus Abbildung) |
| Harmonium: Winddruck „generally about 2in.“ ohne Expression (≈ 0,5 kPa) | in WS | Druckwind-Harmonium | Q9 S. 138; ebenso S. 58 „about two inches“ | Werkzeug-Zitat, an zwei Stellen gleich |
| Beim Stimmen Balg auf etwa halbem Druck | – | – | Q9 S. 148 | Werkzeug-Zitat |
| Federn: Magazin zwei Federn zu 18 lb, Schöpfer zwei zu 9 lb | lb | Reed Organ | Q9 S. 155 | Werkzeug-Zitat |
| Federn: Magazinbalg ≈ 8 kg, Schöpfbalg 2,5–3 kg | kg | Saugwind | Q10 S. 14 | gelesen |
| Saugwind: „2 or 3 inches of water at the most“ (≈ 0,5–0,75 kPa), mit ungeeignetem Zeigergerät geschätzt | in WS | Aeolian Player Reed Organ 1895 | Q13 | Werkzeug-Zitat, geringe Belegkraft |
| Ersatzgebläse 800 Pa (80 mm H₂O) an einem umgebauten Saugwind-Harmonium | Pa | \<HarmO\> | Q14 | Werkzeug-Zitat (Auslegung eines Umbaus, kein Originalwert) |
| „40 and 130 nm“ für eine Parlor Organ (Einheit offenkundig verdruckt, wohl mm WS; Quelle „Anderson, p. 100“) | ? | – | Q24 | Werkzeug-Zitat, **nicht verwendbar** ohne die Primärquelle |
| Indisches Harmonium: f0 ändert sich mit dem Balgdruck „typically less than 1-2 Hz“ | Hz | – | Q1 S. 3 (§ 4.3) | gelesen |
| Expression: Magazinbalg abgeschaltet, Druck folgt den Füßen | – | Druckwind | Q10 S. 14; Q11 S. 6 | gelesen (ohne Zahlen) |

### 2.3 Kanzelle

| Wert | Einheit | Bezug | Quelle, Stelle | Kennzeichen |
|---|---|---|---|---|
| Zellenlänge 16′: tiefstes C „4#in“ (verstümmelt), höchstes C ≈ 1 in | in | Reed Organ, Bauanleitung | Q9 S. 40 | Werkzeug-Zitat, OCR verstümmelt |
| Zellen 8′: bei CC etwa 1¼ (?) in tief × 3¾ (?) in lang; 4′: 1 in × 3 in bei CC | in | hintere Reihe | Q9 S. 44 | Werkzeug-Zitat, OCR verstümmelt |
| Druckwind-Kanzelle 16′ tiefstes C: 4½–5 in lang (Tiefe verstümmelt); 8′: ≈ 1 in tief am tiefsten C | in | Harmonium (reed pan) | Q9 S. 34 | Werkzeug-Zitat, OCR verstümmelt |
| Zungenöffnung 16′ ≈ 2½ (?) in lang am tiefsten C; 8′ von ≈ 1¾ (?) in im Bass abnehmend | in | – | Q9 S. 35 | Werkzeug-Zitat, OCR verstümmelt |
| Zellenhöhe nimmt vom Bass zum Diskant ab (Zahlen verstümmelt) | in | Saugwind | Q9 S. 30 | Werkzeug-Zitat |
| Kanzellenvolumen klein, Resonanzen hoch, Einfluss auf die Anregung meist klein; bei hohen Zungen (Akkordeon, Mundharmonika) kann die Kammerresonanz die Zunge hemmen | – | – | Q8 Abstract | gelesen |
| Kanzelle senkt die Frequenz und hebt höhere Teiltöne (ASA-Abstracts 1996/1997) | – | Reed Organ | Suchtreffer auditory.org | Zusammenfassung |
| Indisches Harmonium: Modell hängt von „reed-chamber volume, reed clearance“ ab, von Hand je Ton eingestellt; keine Werte | – | – | Q1 S. 3 (§ 4.1) | gelesen |

**Befund:** Milne gibt nur Endpunkte je Fußlage (tiefstes und höchstes C), keine Tabelle je Ton; die Brüche sind im
OCR nicht verlässlich lesbar. Die Seiten 27–45 und 138–142 müssten am Scan gelesen werden (Ladeliste).

### 2.4 Einschwingen

| Wert | Einheit | Bezug | Quelle, Stelle | Kennzeichen |
|---|---|---|---|---|
| Wachstumsrate ε: < 0 nahe 0 kPa; ≈ 4–5 s⁻¹ bei 0,2–0,3 kPa; ≈ 8 bei 0,5; ≈ 11,5 bei 1,0; ≈ 10 bei 1,5; ≈ 9 bei 2,0; ≈ 7 bei 2,5; ≈ 3,5 bei 3,0 kPa | s⁻¹ | C3-Zunge, gemessen | Q5 Abb. 4 | gelesen (aus Abbildung, ±0,5) |
| größtes Wachstum „around 1.0–1.5 kPa“ | kPa | C3 | Q5 S. 4 | gelesen |
| Ablauf: Verschiebung der Ruhelage, wenige unregelmäßige Perioden, Anwachsen bei sich festigender Frequenz, weiteres Anwachsen; 2. Biegemode und 1. Torsionsmode früh sichtbar | – | Reed Organ | Q7 Abstract | gelesen (ohne Zahlen) |
| Krümmen der Zunge kann das langsame Ansprechen großer, tiefer Zungen etwas mildern | – | – | Q8 Abstract | gelesen |
| große Zungen langsam, „especially in the bottom octave“; sehr kleine sprechen auf hohem Druck schlecht an | – | Reed Organ | Q9 S. 149 | Werkzeug-Zitat |
| breite Mensur: voller Ton, langsamere Ansprache; schmale: scharf, schnell | – | – | Q9 S. 138; Q23 | Werkzeug-Zitat |
| 2. Biegemode bei f₂ = 6,27 f₁ im Spektrum oberhalb des üblichen Drucks | – | C3, 0,9 kPa | Q4 S. 2, Abb. 4 | gelesen |

Zahlen zur Einschwingzeit **je Tonhöhe** (Cottingham & Wolff) stehen im frei erreichbaren Abstract nicht; der Volltext
wurde nicht gefunden.

### 2.5 Gehäusefilter des indischen Harmoniums (Q1)

| Angabe | Quelle, Stelle | Kennzeichen |
|---|---|---|
| Der IAIF-Filter hat 49 Koeffizienten (All-Pol); die Antwort verschiedener Harmonien zeigt 8–9 Spitzen | Q1 S. 3 (§ 4.2) | gelesen |
| Näherung mit 10 Abschnitten zweiter Ordnung (10 Pole, 10 Nullstellen; `stmcb`, `tf2sos`) oder über 9 Spitzen und 9 Täler (`pchip`, `yulewalk`) | Q1 S. 3 | gelesen |
| **Keine Tabelle** der Frequenzen, Güten oder Koeffizienten. Die Werte stehen nur als Kurve in Abb. 5 („concert quality harmonium“) | Q1 Abb. 5 | gelesen |
| Aus Abb. 5 grob abgelesen: Spitzen bei etwa 0,9 / 2,2 / 3,2 / 4,2 / 5,2 / 6,0 / 6,7 / 7,5 / 8,8 kHz; größte bei ≈ 6,0 und ≈ 6,7 kHz (rund 32 dB), dann ≈ 3,2 kHz (rund 27 dB); oberhalb 10 kHz flach nahe 0 dB | Q1 Abb. 5 | gelesen (aus kleiner Abbildung, ±0,3 kHz, ±3 dB — **nur als Anhalt**) |
| Quelle: Wavetable aus 37 Teiltongewichten des LTAS einer F4-Note (349 Hz); die Gewichte stehen in Abb. 6 (Pd-Patch: 1, 0.2121, 0.3841, 0.1315, 0.1089, 0.1501, 0.0503, 0.0593, 0.0834, 0.0297, 0.0408, 0.0623, 0.0212, 0.0334, 0.0498, 0.0171, 0.0294, 0.0393, 0.0132, 0.024, 0.0329, 0.0089, 0.0186, 0.0267, 0.069, 0.0154, 0.0203, 0.0046, 0.0126, 0.0162, 0.0031, 0.013, 0.0153, 0.0045, 0.0125, 0.0129, 0.0033) | Q1 Abb. 6 | gelesen (kleine Schrift im Bild; einzelne Ziffern können falsch sein, der 25. Wert 0.069 fällt aus der Reihe) |
| Vorgänger mit dem physikalischen Modell: Puranik & Scavone, POMA 49, 035015 (Vienna Talk 2022) | Q1 Lit. [4] | gelesen; Volltext nicht erreicht |
| Dissertation Puranik (McGill): **nicht gefunden** (eine Suche; eScholarship nicht direkt durchsucht) | – | – |

---

## 3 Mundharmonika (diatonisch, Richter)

### 3.1 Zungen- und Schlitzmaße

| Wert | Einheit | Bezug | Quelle | Kennzeichen |
|---|---|---|---|---|
| Schlitzlängen Blaszungen, Kanzelle 1–10: 17 / 16 / 15 / 14 / 13 / 12 / 10,5 / 9,5 / 8 / 7; Breite durchweg 2 | mm | Seydel Bandmaster in **G**, um 1915 | Q15 | Werkzeug-Zitat, Forum; Messung mit 0,5 mm Auflösung |
| Schlitzlängen Blaszungen 1–10: 16,5 / 15,5 / 14,5 / 13,5 / 13,5 / 12 / 12 / 10 / 9 / 8; Breite 2 | mm | Seydel 1847 (Tonart nicht genannt) | Q15 | Werkzeug-Zitat; doppelte Werte bei 4/5 und 6/7 können Abschreibfehler sein; **keine Ziehzungen** |
| Hohner „handmade“ lange Schlitze 16,6 (K. 1) … 8,5 (K. 10), kurze 14,8 … 7,6; Breite 2,05 (K. 1–5) und 1,9 (K. 6–10); Einheit nicht genannt (wohl mm) | (mm) | Hohner Marine Band u. ä. | Suchtreffer zu harp-l.org 2008-July/msg00886 | **Zusammenfassung**; die Tabelle stammt laut Treffer von Pat Missin — wegen des No-AI-Vermerks seiner Seite **nicht** weiter ausgelesen |
| Hohner: gleiche Zungenlängen vom Doppel-Tief-F bis zum C; je Länge über 1,5 Oktaven Tonumfang — die Tonhöhe kommt aus dem Dickenprofil | – | Hohner | Q19 S. 10 | gelesen |
| Marine Band: lange Schlitze von tief D bis C, kurze von Des bis hoch G (nach R. Sleigh) | – | Hohner | Suchtreffer harp-l 2011 | Zusammenfassung |
| Stimmplatte: Länge × Breite × Dicke; Schlitzabstand; Schlitzbreite — Hohner Special 20 (C): 96,8 × 25,3 × 0,9; 7,5; 1,9–2,1. Hohner MS (A): 102,0 × 28,0 × 0,95; 7,5; 2,0–2,2. Easttop T008K (C): 101,9 × 27,4 × 1,2; 7,5; 2,0–2,2. Kongsheng (G): 100,9 × 27,4 × 1,0; 7,5; 1,7–2,2 | mm | – | Q16 | Werkzeug-Zitat, Forum |
| Plattendicke Marine Band 0,9 mm (Messing) | mm | Hohner | Händlerseiten (Suchtreffer) | Zusammenfassung |
| Modellzungen: 14,5 × 2 × 0,13 (440 Hz) und 15 × 2 × 0,13 (392 Hz); Spalt um die Zunge 0,2; Abstand in Ruhe 0,5; Dichte Messing 8553 kg/m³; Güte 95 | mm | Lehrbuchmodell „nach Millot“ | Q17 | Werkzeug-Zitat; Modellannahmen, keine Messung |
| Zungenlänge K. 10: 12,14 mm (Fis) und 12,16 mm (G) | mm | Hohner Blues Harp | Suchtreffer bluesharmonica.com | Zusammenfassung, Forum |
| Zungenwerkstoff: Messing (Hohner und die meisten), Phosphorbronze (Suzuki), Edelstahl (Seydel); dickste Stelle am Nietfuß | – | – | Q19 S. 10 | gelesen |
| Aufbiegung an der Spitze ab Werk: 0,009 in (K. 10) bis 0,021 in (K. 1), also ≈ 0,23–0,53 mm | in | Blues Harp in G | Q18 | Werkzeug-Zitat, Forum |
| Aufbiegung nach dem Einstellen: 0,006–0,008 in (≈ 0,15–0,20 mm); Startwert 0,007 in (0,18 mm) | in | – | Q18 | Werkzeug-Zitat, Forum |
| Faustregel: Spalt an der Spitze etwa gleich der Dicke der Zungenspitze; lange Zungen brauchen mehr | – | – | Suchtreffer (bluesharmonica.com, dummies.com) | Zusammenfassung |
| Bahnson 1998: **keine** Zungenmaße, **keine** Drücke — nur relative Auslenkungen, Frequenzen (Abb. 2: C4 262 … C7 2093 Hz), Mundraumvolumen 15–90 cm³ (Abb. 14), Oberton der Zunge bei 6,26 f₁ (S. 2143) | – | Golden Melody in C | Q6 | gelesen, alle 11 Seiten |
| Übliche Bereiche (ohne Quelle): Diskantzunge 7–16 mm lang, 0,06–0,14 mm dick; tiefe Zunge 14–28 mm, 0,10–0,24 mm | mm | – | Q21 | Werkzeug-Zitat, **sehr geringe Belegkraft** |

### 3.2 Atemdruck, Luftfluss, Kanzelle

| Wert | Einheit | Bezug | Quelle | Kennzeichen |
|---|---|---|---|---|
| Kanal im Modell: Länge 28 mm (schließt die Lippen ein), Querschnitt 20 mm²; Helmholtz-Güte 4 | mm, mm² | Lehrbuchmodell | Q17 | Werkzeug-Zitat |
| Gemessener Atemdruck beim Spielen (laut/leise), Luftfluss | – | – | – | **nichts gefunden** (drei Suchen, englisch) |
| Künstliches Anblasen: bis 800 mm WS mit dem Mund möglich (Fouché, allgemeine Zungenversuche, nicht Mundharmonika) | mm WS | – | Q12 S. 240 | gelesen — kein Spielwert |

### 3.3 Adachi et al. JASA 2026 (Q20)

- Zitat geklärt: JASA **159(5), 4191–4203 (2026)**, DOI 10.1121/10.0043784 (Suchtreffer; die Literaturliste von Q2,
  Nr. [5], nennt „vol. 159, no. 1“ bei gleichen Seiten — **gelesen**, dort offenbar ein Tippfehler).
- Zugang: **Bezahlschranke**; kein Preprint gefunden (arXiv, ResearchGate nur als Treffer, nicht geöffnet). PubMed
  verlangte Cookies — nicht erreicht.
- Inhalt (**Zusammenfassung**): Zungenschwingung, Strömung durch beide Zungenöffnungen und Resonanz auf der Mundseite
  gekoppelt; Resonator ist eines von drei zylindrischen Rohren, Länge veränderlich; Blas-Bending Kanzelle 7,
  Zieh-Bending Kanzelle 4, Overblow Kanzelle 6. Ein Suchtreffer nennt eine Blaszunge mit Eigenfrequenz 1067,4 Hz.
- Parametertabelle: **nicht erreicht.** Die Tabelle 1 des Akkordeon-Aufsatzes derselben Gruppe (Q2) zeigt, welche
  Größen dort zu erwarten sind (Eigenfrequenz, Masse, Dämpfungsgrad, L, W, Spalt, Ruhelage, Kammerquerschnitt).
- In Q2 außerdem **gelesen**: Schwelle des Akkordeon-Modells „approximately 200 Pa“ (S. 6), Amplitude 0,5–3 mm
  zwischen 0,2 und 2,5 kPa (Abb. 7), Steigung −2,5 Hz/kPa gemessen (Abb. 5).

---

## 4 Melodica

| Wert | Einheit | Bezug | Quelle | Kennzeichen |
|---|---|---|---|---|
| Zunge: aktive Länge 9–26 mm, Dicke 0,07–0,20 mm | mm | „Melodica reed“ allgemein | Q21 | Werkzeug-Zitat, ohne Quelle, **sehr geringe Belegkraft** |
| Faustregel: Spalt zwischen Zunge und Platte etwa gleich der Zungendicke; tiefe (größere) Zungen haben größere Spalte | – | Yamaha, Hohner, Suzuki | Q22 | Zusammenfassung, Forum |
| Suzuki-Zungen ab Werk mit etwas weiteren Spalten | – | Suzuki M-37C | Q22 | Zusammenfassung, Forum |
| Zungenwerkstoff Phosphorbronze | – | Suzuki S-32C, Pro-37 V3 | Händlerseite Thomann (Suchtreffer) | Zusammenfassung |
| Gehäusemaße 430 × 50 × 100 mm (S-32C), 500 × 65 × 110 mm (Pro-37 V3) | mm | Suzuki | Händlerseite (Suchtreffer) | Zusammenfassung |
| Schlauch von rund 15 Fuß ändert den Ton nicht merklich | – | – | melodicaworld (Suchtreffer) | Zusammenfassung, Forum |
| Zungenmaße je Ton, Stimmplatte, Kammer, Blasdruck: **keine Fachquelle, keine Messung gefunden** | – | – | – | drei Suchen englisch, zwei japanisch, eine auf melodicaworld beschränkt |

---

## 5 Nicht gefunden / nicht erreicht

- **Koopman & Cottingham**, Reed Organ Society Bulletin 15 (3–4), 17–23 (1997): Primärquelle der Schwelle 0,2 kPa —
  nicht online gefunden. Die Angabe ist jetzt aber in Q4 im Wortlaut gelesen.
- **Cottingham & Wolff 2015**, Volltext mit Einschwingzeiten: nur Abstract erreichbar.
- **Puranik & Scavone, POMA 49, 035015 (2022)** und **Dissertation Puranik**: nicht erreicht bzw. nicht gefunden.
- **St. Hilaire, Wilson, Beavers, JFM 49 (1971)**: nur Abstract-Seiten (Cambridge), Maße der Zunge nicht erreicht.
- **Johnston 1987** (Acoust. Aust. 15, 69–75): kein Volltext gefunden; Heftnummer in den Zitaten uneinheitlich (15/2
  oder 15/3).
- **Millot, Dissertation 1999** und **Millot, Cuesta, Valette, Acta Acustica 87 (2001), 262–270**: nicht gefunden.
- **Adachi et al. JASA 2026**: Bezahlschranke (siehe 3.3). PubMed: Cookie-Sperre.
- **Gellerman, The American Reed Organ**: auf archive.org nur der *Reed Organ Atlas* (1985) gefunden, nicht geöffnet.
- **Estey / Mason & Hamlin / Mustel / Alexandre**: keine Mensurtabelle, kein Winddruck in Zahlen gefunden.
- **Europäisches Druckwind-Harmonium, deutschsprachig**: Goldbrunner (Q10, S. 1–28) und Titz (Q11, S. 1–6) nennen
  keinen Winddruck in mm WS und keine Zungenmaße. Goldbrunner S. 29–48 (Stimmen, Stimmtabelle) nicht gelesen.
- **Indisches Harmonium**: keine Zungenmaße je Ton über Puranik & Scavone hinaus; Händlerseiten ohne Maße.
- **Fouché 1928 (Q12)**: nur S. 237–243 gelesen; die Abschnitte zu Amplituden und Durchfluss (§ 14) können weitere
  Zahlen zu Harmoniumzungen enthalten.
- **Pat Missin (SLOTS.TXT)** und **Richard Sleigh (Reed Chart)**: bewusst nicht ausgelesen (No-AI-Vermerk) bzw. nicht
  frei zugänglich. Dort stehen nach den Forenhinweisen die Schlitzlängen je Kanzelle für Hohner.
- **Mundharmonika: Dickenprofil der Zungen, Ziehzungen, Kanzellenmaße des Kamms, Atemdruck**: nichts Belegtes.
- **Melodica**: nichts Belegtes (siehe 4).

---

## 6 Ladeliste

Größen aus `curl -sI` (Content-Length), Stand 2026-11-11.

| URL | Bytes | Wofür | Lizenz |
|---|---|---|---|
| https://www.dafx.de/paper-archive/2023/DAFx23_paper_47.pdf | 5 530 018 | Q1: Abb. 5 (Gehäusefilter) und Abb. 6 (Teiltongewichte) in voller Auflösung ablesen | CC BY 4.0 (Fußnote S. 1) |
| https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF | 607 160 | Q4: Schwelle, Amplituden, Spieldruck | kein Vermerk gefunden |
| https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF | 671 902 | Q5: Wachstumsrate ε(p), Frequenz über Druck, Formeln nach Fletcher | kein Vermerk gefunden |
| https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF | 156 110 | Q3: vollständiger Parametersatz der C3-Zunge | kein Vermerk gefunden |
| https://www.cs.bu.edu/fac/snyder/TablatureWeb/Bahnson%20JASA%201998.pdf | 1 721 345 | Q6: Verhalten beider Zungen je Kanzelle (Tab. I, Abb. 4) — keine Maße | © 1998 Acoustical Society of America |
| https://dael.euracoustics.org/confs/fa2026/data/articles/000418.pdf | 1 323 922 | Q2: vollständige Tabelle einer Akkordeonzunge, Modell derselben Gruppe wie Q20 | CC BY 3.0 Unported (Fußnote S. 1) |
| https://afst.centre-mersenne.org/item/10.5802/afst.350.pdf | nicht ermittelt (HEAD abgelehnt, 405; das Abrufwerkzeug meldete 7,2 MB) | Q12: Versuche an Harmoniumzungen, Weber-Zunge | © Université Paul Sabatier 1928, Bedingungen von Numdam |
| https://www.gdo.de/fileadmin/_migrated/content_uploads/Goldbrunner_Harmoniumreparatur.pdf | 6 911 743 | Q10: Zungenformen im Bild (Abb. 3, 4), Stimmtabelle S. 46 | kein Vermerk gefunden |
| https://archive.org/details/reedorganitsdesi0000hfmi | Scan nicht ermittelt; OCR-Text (`…_djvu.txt`) 296 265 | Q9 Milne: S. 27–45 und 138–149 **am Scan** lesen (Brüche); einzige gefundene Quelle mit Zungen- und Zellenmaßen je Fußlage und mit dem Winddruck | keiner angegeben; nicht gemeinfrei — eher ausleihen oder antiquarisch |

Nicht auf der Ladeliste, aber zu beschaffen (Bibliothek): Adachi et al. JASA 2026 (Q20); Koopman & Cottingham 1997;
St. Hilaire et al. 1971; Johnston 1987; Gellerman, *The American Reed Organ*.

---

## 7 Eigene Einschätzung (keine Quelle, nur meine Lesart)

**Harmonium / Reed Organ (C3–F6) — teilweise ausreichend.**
- *Windbereich:* brauchbar belegt. Schwelle ≈ 0,2 kPa (100–200 Hz), üblicher Druck 0,6–1,0 kPa an der Reed-Organ-Zunge
  (Q3, Q4); Milne nennt für das Druckwind-Harmonium rund 2 in WS ≈ 0,5 kPa (Q9, zwei Stellen). Für die Expression
  (oben offen) und für die Schwelle im Diskant fehlt jede Zahl.
- *Eine Zunge vollständig:* C3 mit ω_r, k, M, D, b, wirksamer Fläche, Durchflussexponent (Q3) und dazu Amplitude und
  Wachstumsrate über dem Druck (Q4, Q5). Das ist ein Ankerpunkt mit Messwerten, an dem ein Modell geprüft werden
  kann — auch das Einschwingen: ε ≈ 8–11 s⁻¹ bei 0,5–1 kPa heißt Zeitkonstante rund 0,09–0,13 s für C3.
- *Über den Umfang:* nur Milnes Endpunkte (tiefstes C je Fußlage rund 3–4 in, höchste Zunge unter 1 in) und die Regel
  Breite ≈ ¼ Länge für breite Mensur — verstümmelt und nicht eindeutig (Zunge oder Rahmen). Die geometrische Länge
  ≈ 4 cm bei C3/A♯2 (Q4, Q5) passt nicht ohne Weiteres zu „4 in“ für CC 8′ bei Milne, wenn man nur mit 1/L² rechnet;
  Dicke und Profil sind also die offene Größe. **Dicke je Ton fehlt ganz.**
- *Gegenprobe der Balkenformel* f = 0,1615·(t/L²)·√(E/ρ): Weber-Zunge 28,3 × 0,49 mm, 388 Hz verlangt √(E/ρ) ≈ 3930 m/s
  — das ist Messing (E ≈ 131 GPa bei 8500 kg/m³) und stimmt mit Puranik & Scavone (16 × 0,4 mm, ≈ 990 Hz,
  E 125 GPa) überein. Für gleichmäßig dicke Messingzungen trägt die Formel also; für profilierte und gekröpfte
  Basszungen nicht.
- *Kanzelle:* nur Milnes Endpunkte. *Gehäusefilter (indisch):* keine Werte, nur Abb. 5 in Q1 — grob ablesbar.
- **Fehlt für eine Herleitung je Taste:** Dicke/Profil über den Umfang, Aufbiegung je Ton, Kanzelle je Ton als
  Tabelle, Schwelle und Einschwingzeit im Diskant. Der sauberste nächste Schritt wäre Milne am Scan (und Gellerman)
  und ein vermessener Zungensatz — die Literatur allein trägt keine Tabelle je Taste.

**Mundharmonika (C4–C7) — Länge und Breite ja, der Rest nein.**
- *Länge, Breite:* Schlitzlängen je Kanzelle für zwei Seydel-Modelle (Q15, Forum, nur Blaszungen, eines davon in G)
  und Endpunkte für Hohner (16,6 … 8,5 mm, Breite 2,05/1,9 — nur Zusammenfassung). Das reicht für einen glatten
  Verlauf der Länge über zehn Kanzellen; Ziehzungen müssten gleich angenommen oder gemessen werden.
- *Dicke:* nur zwei Einzelwerte (Millot 110 µm bei 444 Hz; Lehrbuchmodell 0,13 mm bei 440 Hz). Weil Hohner dieselbe
  Länge über 1,5 Oktaven nutzt (Q19), ist die Dicke je Ton **nicht** aus der Länge herleitbar, nur über die
  Balkenformel zurückzurechnen — und die liegt bei den beiden Einzelwerten 10–15 % daneben (profilierte Zungen).
- *Aufbiegung:* Forenwerte 0,15–0,2 mm eingestellt, 0,23–0,53 mm ab Werk (Q18); Millot 50 µm ist ein anderer Begriff
  (seitlicher Spalt). Plattendicke 0,9 mm ist gut belegt (Hersteller).
- *Kanzelle, Atemdruck:* nichts Gemessenes. **Fehlt:** Adachi 2026 (Parametertabelle für Kanzelle 4, 6, 7),
  Johnston 1987, Millot 2001 — alle drei über eine Bibliothek zu beschaffen; sie wären der belegte Satz.

**Melodica (F3–C6) — reicht nicht.**
- Es gibt keine Fachquelle und keine Messung mit Zahlen; die einzige Spanne (9–26 mm, 0,07–0,20 mm) ist ohne Herkunft.
  Für Vorgaben je Taste bleibt nur: ein Instrument vermessen (Stimmplatte, Zungen, Kammer) oder die Melodica als
  Ableitung der Mundharmonika-/Akkordeonzunge mit eigener Kammer ansetzen und das ausdrücklich als Annahme mit
  geringer Belegkraft führen. Blasdruck: nichts gefunden; als Anhalt taugt allenfalls der Bereich der
  Reed-Organ-Zungen (0,2–1,5 kPa), belegt ist er für die Melodica nicht.
