# Recherche Cembalo: Quellen mit Zahlen für ein Modell je Taste (Kern `pluckedString`)

> **Auftrag:** Quellen mit Zahlen für Saiten, Zupfpunkt, Kiel, Tastenstoß, Resonanzboden und Ziele des Cembalos
> je Taste; Ladeliste ohne Laden. Ergänzt `ic/recherche/2026-10-26_physik_cembalo_harfe_koto_oud.md` (dort Q1–Q28)
> und `ic/quellen.md`; Bekanntes ist nicht wiederholt.
> **Datum:** 2026-11-12 (S195) · **Bericht eines Agenten, unverändert.**
> **Reichweite:** rund 50 Abrufe: 7 Websuchen, 17 WebFetch-Aufrufe, rund 25 curl-Aufrufe (HTML-Text, HAL-API,
> Crossref, GitHub-API, HEAD; davon 12 Datenblätter der Edinburgh-Sammlung in einem Lauf). PDFs, die WebFetch
> von sich aus im Sitzungsordner `tool-results` ablegte, wurden dort mit `pdftotext` gelesen; nichts ins Projekt
> geladen. Gescheitert: Springer-Volltext Välimäki 2004 (Login-Umleitung), HAL-Volltexte über WebFetch
> (Bot-Schutz „Anubis“, nicht umgangen; HEAD per curl ging), JAMIS 2012 (über 10 MB), Crossref nach 8 Anfragen
> (HTTP 429). Kein Treffer ist kein Beweis.
> **Kennzeichnung:** `[wörtlich, pdftotext]` / `[wörtlich, HTML]` / `[wörtlich, API]` = Wortlaut aus dem
> Rohtext, den ich selbst gelesen habe (Seite/Abschnitt angegeben); `[wörtlich, Werkzeug]` = WebFetch gab den
> Wortlaut als Zitat; `[nur Zusammenfassung]`; `[eigene Rechnung]`; `[Werkzeug, Tabelle umgestellt]`.

---

## 1 Quellen

Nummern C1 … fortlaufend; Q-Nummern verweisen auf den Bericht vom 26.10.2026.

| Nr. | Angabe | Fundort | frei? | Rechtsvermerk (Wortlaut, Ort) | Abruf |
|---|---|---|---|---|---|
| C1 | Di Veroli, C. (2011): „Taskin Harpsichord Scalings and Stringings Revisited“, Teil 1 | braybaroque.ie/harps/Taskin_stringing.htm | ja (HTML) | „Copyright © 2011 by Claudio Di Veroli, Bray Baroque, Bray, Ireland, March 2011“; Zitieren nur in Anführung, unverändert, mit Quelle, Autor per Mail benachrichtigen (Teil 3, „Terms and Conditions“) | 12.11. |
| C2 | Di Veroli (2011), Teil 3 „Appendixes and Bibliography“ (Tab. 7 Drahtnummern, Tab. 8 Besaitung Taskin 1769) | braybaroque.ie/harps/Taskin_stringing3.htm | ja (HTML) | wie C1; „Page last updated: 22-Mar-2016“ | 12.11. |
| C3 | Di Veroli (2011, rev. 2023): „Restringing the Hubbard-Di Veroli harpsichord“ (Besaitung, Mensur an allen F) | harps.braybaroque.ie/restringing.htm | ja (HTML) | „Copyright © 2011 by Claudio Di Veroli , Bray Baroque, Bray, Ireland, April 2011, rev. June 2023.“ | 12.11. |
| C4 | Di Veroli: Tabellenblatt „Stringing-Calculator2022-General.xls“ (Hüllkurven der Züge, Bruchgrenzen) | harps.braybaroque.ie/hubbard_images/… | ja, 216 064 B | „Copyright subsists, but feel free to edit any part of the spreadsheet for your own personal use.“ (C3, Abschn. 11) | nur HEAD |
| C5 | University of Edinburgh, St Cecilia's Hall: Datenblätter (Taskin 1769, Goermans/Taskin 1764/1784, Hass 1764, Kirkman 1755, Shudi 1766, Falkener 1773, Coston um 1725 u. a.) | collections.ed.ac.uk/stcecilias/record/96075 usw. | ja (HTML) | „Unless explicitly stated otherwise, all material is copyright © 2026 University of Edinburgh“ (Fuß der Seite) | 12.11. |
| C6 | Zuckermann Harpsichords International: „French Double Manual VI Stringing Schedule“, ZHI 2000 | zhi.net/s/FD-VI-SCH-…pdf (Squarespace) | ja, 54 710 B | kein Vermerk im Text; nur „ZHI 2000“ | 12.11. |
| C7 | Penttinen, H. (2006): „On the Dynamics of the Harpsichord and its Synthesis“, DAFx-06, Montréal, S. 115–120 | dafx.de/paper-archive/2006/papers/p_115.pdf | ja, 279 281 B | kein Vermerk im Text gefunden | 12.11. |
| C8 | Välimäki u. a. (2004), EURASIP JASP 2004:7, 934–948 (= Q1), Lehrkopie | ee.columbia.edu/~dpwe/e6820/papers/ValPK04-harpsi.pdf | lesbar, ~1,5 MB | „© 2004 Hindawi“ (laut quellen.md); Verlagsseite link.springer.com verlangt Login | 12.11. |
| C9 | Perng, Smith, Rossing (2010), DAFx-10, Graz, S. 127–130 (= Q4b) | dafx.de/…/PerngSmithRossing_DAFx10_P93.pdf | ja, ~456 KB | kein Vermerk | 12.11. |
| C10 | Perng, Smith, Rossing (2011), DAFx-11, Paris (= Q4a) | dafx.de/paper-archive/2011/Papers/24_e.pdf | ja, ~1,2 MB | kein Vermerk | 12.11. |
| C11 | Perng, C.-Y. J. (2012): „Physical modeling of the harpsichord plectrum-string interaction“, Diss. Stanford (Physik) | purl.stanford.edu/wp454hs7976 | ja, 21 540 441 B | „This work is licensed under a Creative Commons Attribution Non Commercial 3.0 Unported license (CC BY-NC).“; „(c) Copyright 2012 by Chao-Yu Jack Perng“ | Katalog, HEAD |
| C12 | Roy, A. (2015): „Développement d'une plate-forme robotisée pour l'étude des instruments de musique à cordes pincées“, Thèse | HAL tel-01323985 | Datei, 4 612 942 B | HAL-API: `about.hal.science/hal-authorisation-v1/` | API, HEAD |
| C13 | Roy u. a. (2015), CFM 2015 (= Q5) | HAL hal-03444591 | Datei, 309 604 B | hal-authorisation-v1 | API, HEAD; WebFetch: „Access Denied“ |
| C14 | Paté u. a. (2017), JASA 141 (= Q3) | HAL hal-01565218 | **nur Notiz**, keine Datei | – | API |
| C15 | Savage, Kottick, Hendrickson, Marshall (1992): „Air and structural modes of a harpsichord“, JASA 91, 2180–2189 | DOI 10.1121/1.403652 | Abstract | kein Lizenzfeld (Crossref) | API |
| C16 | Le Moyne, Le Conte, Ollivier (2008): „Couchet Harpsichord soundboard vibroacoustics behaviour: …IPNAH“, JASA 123, 3445 (Abstract) | DOI 10.1121/1.2934256 | Abstract | – | API |
| C17 | Kottick (1985): „The Acoustics of the Harpsichord: Response Curves and Modes of Vibration“, GSJ 38, 55 ff. | DOI 10.2307/841280 (JSTOR) | nein | – | nur Titel |
| C18 | Giordano, Winans (1999): „Plucked strings and the harpsichord“, JSV 224, 455–473 | DOI 10.1006/jsvi.1999.2187 | nein | Elsevier-TDM-Link (Crossref) | nur Titel |
| C19 | Griffel (1994): „The Dynamics of Plucking“, JSV 175, 289–297; Gibbs, Griffel (1994), JSV 175, 423 f. | DOI 10.1006/jsvi.1994.1329 / .1337 | nein | Elsevier-TDM-Link | nur Titel |
| C20 | Nemus-Project: harpsichord-interface-models, README (Maße Springer, Zunge, Kielschlitz) | github.com/Nemus-Project/… | ja | GPL-3.0 (Lizenzdatei, laut quellen.md) | GitHub-API |
| C21 | wirestrungharp.com: „Table 1 mechanical properties“ (MatWeb, Little Falls Alloys, Malcolm Rose) | wirestrungharp.com/material/strings/… | ja | kein Vermerk gefunden | 12.11. |
| C22 | Koster, J. (1995): „Early Flemish harpsichord string scalings and their acoustical implications“, ASA 130, 4pMU1 | auditory.org/asamtgs/asa95stl/4pMU/4pMU1.html | Abstract | – | 12.11. |
| C23 | Evans, D. (2014): „Copying a 17th-century French harpsichord“, Harpsichord & Fortepiano 19(1) | mircat.org/…/2014_19-1_06.pdf | ja | „Licensed under CC BY-NC 4.0.“ (Kopf, Peacock Press) | 12.11. |
| C24 | Irvin, P. Y. (2012): „Tailoring the Sound of Your Keyboard Instrument Part II“, HKI 16(2) | mircat.org/…/2012_16-2_20.pdf | ja | „Licensed under CC BY-NC 4.0.“ | 12.11. |

---

## 2 Saiten je Taste

### 2.1 Mensur und Zupfpunkt Taskin 1769 (Edinburgh) — das vollständigste gefundene Paar

Wortlaut C5, Datenblatt 96075 [wörtlich, HTML]: „Compass 61 notes F₁ - F₆ [FF - f''']. Three sets of strings,
2 × 8-ft, 1 × 4-ft“ und „String lengths (plucking points): F₆ 148 (91), C₆ 186 (96), F₅ 268 (107),
C₅ 357 (115), F₄ 535(127), C₄ 711 (137), F₃ 982 (51), C₃ 1178 (162), F₂ 1443 (179), C₂ 1681 (191),
F₁ 1765 (208).“ Einheit nicht genannt; mm nach Vergleich mit C1 (c″ 357). Register des Zupfpunkts nicht genannt.
**F₃ „(51)“ ist offenbar ein Schreibfehler** (Reihe 137 … 162; vermutlich 151) — am Original prüfen.

| Taste | L mm | Zupf mm | Zupf/L | Draht 8′ (C2 Tab. 8, a′ 400) | d mm | T N (a′ 400) | σ MPa |
|---|---|---|---|---|---|---|---|
| F₁ FF | 1765 | 208 | 0,118 | RB 00 .0244″ | 0,620 | 51,5 | 171 |
| C₂ C | 1681 | 191 | 0,114 | RB 1 .0204″ | 0,518 | 73,3 | 348 |
| F₂ F | 1443 | 179 | 0,124 | YB 3 .0161″ | 0,410 | 58,6 | 446 |
| C₃ c | 1178 | 162 | 0,138 | IR 5 .0133″ | 0,339 | 54,9 | 612 |
| F₃ f | 982 | (51) | (0,154 mit 151) | IR 6 .0118″ | 0,300 | 53,5 | 758 |
| C₄ c′ | 711 | 137 | 0,193 | IR 7 .0106″ | 0,269 | 50,8 | 892 |
| F₄ f′ | 535 | 127 | 0,237 | IR 8 .0096″ | 0,244 | 42,0 | 900 |
| C₅ c″ | 357 | 115 | 0,322 | IR 8 .0096″ | 0,244 | 42,0 | 900 |
| F₅ f″ | 268 | 107 | 0,399 | IR 9 .0084″ | 0,213 | 32,3 | 903 |
| C₆ c‴ | 186 | 96 | 0,516 | IR 9 .0084″ | 0,213 | 34,9 | 977 |
| F₆ f‴ | 148 | 91 | 0,615 | IR 9 .0084″ | 0,213 | 39,4 | 1102 |

Kennzeichnung: L, Zupf [wörtlich, HTML]; Draht [wörtlich, HTML] aus C2 Tab. 8; Zupf/L, T, σ [eigene Rechnung]:
gleichstufig, a′ = 400 Hz, T = μ(2Lf)², ρ = 8700 (RB), 8500 (YB), 7800 kg/m³ (IR) **angenommen** (nicht aus
den Quellen). Bei a′ = 415 Hz jeder Zug × 1,076. Gegenprobe: C3 nennt als historischen Mittelwert „6.4 Kg“ in
der tiefsten Oktave, „5.7 Kg“ am Übergang Messing–Eisen, „3.7 Kg“ bzw. „3.5 Kg“ im obersten Diskant
[wörtlich, HTML, C3 Abschn. 8] — das sind 63 / 56 / 34–36 N und passt zur Rechnung. Auffällig: σ ≈ 1100 MPa
bei f‴ liegt an der Bruchgrenze von Eisen (Abschn. 2.4).

**Widerspruch Mensur C:** C1 Tab. 1 gibt für Taskin 1769 „C“ 1617 mm (F 1764, c 1178, c′ 711, c″ 357), C5 gibt
C₂ 1681 mm; die übrigen Werte stimmen. Ungeklärt, welche Zahl stimmt.

### 2.2 Weitere Mensuren mit Zupfpunkt (C5, Edinburgh, alle F und C) [wörtlich, HTML]

| Instrument | F₁ | C₂ | F₂ | C₃ | F₃ | C₄ | F₄ | C₅ | F₅ | C₆ | F₆ |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Goermans/Taskin 1764/1784 (96081) | 1835 (193) | 1727 (174) | 1532 (162) | 1207 (148) | 993 (138) | 713 (124) | 550 (113) | 366 (105) | 271 (98) | 184 (87) | 142 (80) |
| Hass 1764, dt. (96074) | 1817 (203) | 1673 (187) | 1480 (176) | 1183 (161) | 974 (150) | 694 (135) | 532 (125) | 362 (100) | 271 (100) | 185 (86) | 141 (77) |
| Kirkman 1755, Doglegspringer (96082) | 1791 (186) | 1635 (168) | 1465 (152) | 1193 (131) | 971 (117) | 690 (102) | 521 (89) | 343 (75) | 257 (67) | 172 (55) | 132 (48) |
| Shudi 1766 (96088) | 1862 (175) | 1723 (160) | 1522 (147) | 1232 (130) | 1006 (119) | 701 (104) | 524 (95) | 344 (82) | 255 (74) | 170 (62) | 133 (54) |
| Falkener 1773 (96076) | 1825 (189) | 1684 (171) | 1488 (156) | 1184 (137) | 980 (125) | 693 (107) | 521 (96) | 350 (83) | 263 (77) | 182 (67) | 143 (63) |

Coston um 1725 (96078) G₁ … E₆ und zwei Spinette/Virginale (96071, 96073) ebenfalls mit Zupfpunkten; Barton 1709
(96112) nennt nur die Überschrift ohne Zahlen. Für Goermans/Taskin: „Gauge numbers stamped into the wrestplank,
they appear to date from Taskin's rebuilding“ [wörtlich, HTML, 96081] — die Nummern selbst stehen nicht online.

### 2.3 Besaitung und Mensur in mehr Tasten

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Taskin 1769, 8′ (Rekonstruktion a′ 400 Hz), Bereiche | e″–f‴ IR 9; f′–eb″ IR 8; a–e′ IR 7; e–g# IR 6; c–eb IR 5; Bb–B YB 5; G#–A YB 4; F–G YB 3; Eb–E YB 2; C#–D RB 2; BBb–C RB 1; GG#–AA RB 0; FF–GG RB 00 | C2 Tab. 8 | [wörtlich, HTML] |
| Taskin 1769, 4′ | g#″–f‴ IR 10; c#″–g″ IR 9; g#–c″ IR 8; d–g IR 7; Bb–c# IR 6; G–A YB 6; F–F# YB 5; Eb–E YB 4; C–D RB 4; GG#–BB RB 3; FF–GG RB 2 | C2 Tab. 8 | [wörtlich, HTML] |
| Mensuren Taskin, sieben Instrumente, 8′ und 4′ an F, C, c, c′, c″ | z. B. 1769: 8′ 1764/1617/1178/711/357, 4′ c 605, c′ 337, c″ 163 | C1 Tab. 1 | [Werkzeug, Tabelle umgestellt] |
| Französische Drahtnummer → Durchmesser (Di Veroli 2011) | 000 0,685; 00 0,620; 0 0,572; 1 0,518; 2 0,463; 3 0,410; 4 0,377; 5 0,339; 6 0,300 mm; 7 .0106″; 8 .0096″; 9 .0084″; 10 .0074″ (0,1891 mm) | C2 Tab. 7 | [wörtlich, HTML] (Spalte „Di Veroli 2011“) |
| Wechselstellen Frankreich 18. Jh. | „8': Red Brass to Yellow Brass at D/Eb, Yellow Brass to Iron at B/c“; „4': Red Brass to Yellow Brass at C#/D, Yellow Brass to Iron at Bb/B“ | C3 Abschn. 7 | [wörtlich, HTML] |
| Drahtstärken in Frankreich | „Red Brass: .025" to .014"“, „Yellow Brass: .018" to .012"“, „Iron: .013" to .007"“ | C3 Abschn. 7 | [wörtlich, HTML] |
| Hubbard-Di Veroli (a′ 415,3), Mensur Unter-8′ / 4′ | f‴ 153 / 74; f″ 277 / 124; f′ 547 / 255; f 971 / 486; F 1410 / 773; FF 1752 / 1073 mm | C3 Abschn. 10 | [wörtlich, HTML] |
| Hubbard-Di Veroli, Besaitung 8′ | c#‴–g‴ Stahl .007″; e″–c″[sic] Eisen .008″; f′–eb″ .009″; bb–e′ .010″; f#–a .011″; eb–f .012″; c–d .013″; B YB .013″; G#–Bb YB .014″; F–G .016″; Eb–E .018″; C#–D RB .018″; BBb–C .020″; GG#–AA .022″; FF–GG .024″ | C3 Abschn. 10 | [wörtlich, HTML] |
| Zuckermann French Double VI, a′ 415, 8′ | FF–GG .025 RB; GG#, AA .022; AA#, BB .022; C, C# .020 RB; D–F .018 YB; F#–G# .016; A–d .014 YB; d#, e .014 IR; f, f# .013; g–a# .012; b, c .011; c#–g# .010; a–g‴ .009 IR (keine Mensur) | C6 | [wörtlich, pdftotext] |
| Zugformeln | „(1) T= D(FL)²/2450“, „(2) P = 506.71 TW²“ (Einheiten im Text) | C1 | [wörtlich, Werkzeug] |
| Knif/Pelto 2000 (Messinstrument von C7/C8) | „strings are all made of brass“; „56 keys from G1 to D6“, 46–1100 Hz; „240 cm long“, „85 cm wide“ | C8 S. 936 | [wörtlich, pdftotext] |

### 2.4 Drahtkennwerte

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Malcolm Rose Eisen „Grade A“ | ρ 7,77 g/cm³, E 170 GPa, Zugfestigkeit „~1000+ (est.)“ MPa, empfohlene Höchstspannung 790 MPa | C21 | [Werkzeug, Tabelle umgestellt] |
| Malcolm Rose Rotmessing / „English Brass“ | 435 (empf.) / 620 MPa; 637 (empf.) / 895 MPa | C21 | [Werkzeug, Tabelle umgestellt] |
| Messing C22000 / C26000 (modern) | ρ 8,80 / 8,53 g/cm³, E 115 / 110 GPa | C21 | [Werkzeug, Tabelle umgestellt] |
| Historische Zugfestigkeit | „800-1 200 MPa“ Eisen, „700-1000 MPa“ Gelbmessing (37 bzw. 35 Werte) | Suchergebnis, Quelle unklar | [nur Zusammenfassung] |
| Bruchgefahr in der Planung | Eisen meist unter 80 %; „1 Kg of pull in the axis also representing 10% of breaking risk“ | C3 Abschn. 7 | [wörtlich, HTML] |
| Coulomb 1784 (Bruchlast, Dichte, Steifigkeit Pariser Drahts), ausgewertet von Goodway & Savage 1992 | nur Titel; die gefundene Kopie liegt auf einem fremden Zotero-Speicher (Rechte unklar, nicht geöffnet) | – | – |

---

## 3 Zupfpunkt je Taste und Register

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Zupfpunkt je F/C, sechs Instrumente | Tabellen 2.1 und 2.2; Anteil am Beispiel Taskin 1769 von 0,114 (C₂) bis 0,615 (F₆), gemessen ab Sattel | C5 | Zahlen [wörtlich, HTML], Anteil [eigene Rechnung] |
| Welches Register | im Datenblatt nicht genannt; Kirkman ausdrücklich „plucking point of dogleg“ | C5 96082 | [wörtlich, HTML] |
| Abstand der Springerreihen (NEMUS-Nachbau) | „Jack 1: 8.4000mm“, „Jack 2: 23.4000mm“, „Jack 3: 38.4000mm“; „Jack Pitch: 14.4875mm“ | C20 | [wörtlich, API] |
| Anordnung Frankreich | Reihe des 4′ zwischen den beiden 8′, vergrößert den Abstand der Unisono-Zupfpunkte | Britannica | [nur Zusammenfassung] |
| 8′ hinten / vorn | „The plucking point in the 8 back register is further away from the player.“ | C7 S. 115 | [wörtlich, pdftotext] |
| Knif/Pelto | „plucking point changes from 12% to about 50% of the string length in the bass and in the treble“ | C8 S. 936 | [wörtlich, pdftotext] |
| Lautenzug | keine Zahl gefunden; Perng 2011: „modeling of the lute stop“ als künftige Arbeit | C10 Abschn. 6 | [wörtlich, pdftotext] |

Für die drei Register getrennt: **nicht gefunden.** Mit 2.1 und dem Springerabstand (≈ 15 mm je Reihe, C20) lässt
sich das zweite 8′ und das 4′ nur schätzen [eigene Einschätzung].

---

## 4 Kiel und Zupf

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Kiel (Delrin), DAFx-10 | L 6 mm, W 2 mm, H 0,5 mm, I 0,029 mm⁴, E 3 GPa | C9 Tab. 1 | [wörtlich, pdftotext] |
| Kiel (Delrin), DAFx-11 | L 6 mm, W 4 mm, H 0,5 mm, I 0,029 mm⁴, E 5 GPa | C10 Tab. 1 | [wörtlich, pdftotext] |
| Saite im Modell | Stahl, T 92 N (2010) bzw. 135 N (2011), ρ 7850 kg/m³, d 0,37 mm, μ 0,84 g/m, Ls 0,5 m | C9, C10 Tab. 1 | [wörtlich, pdftotext] |
| Breite/Dicke | „tapered towards the free end, so the average values were taken“ | C9 Abschn. 3.1 | [wörtlich, pdftotext] |
| Springergeschwindigkeit | „physically reported to be between 0.1 and 10 m/s“ (C9); „regular playing speeds of 0.02 - 0.1 m/s“ (C10) | C9 3.2, C10 5.3 | [wörtlich, pdftotext] |
| Ablöseamplitude je Zupfverhältnis (v 1 m/s, ΔL 5 mm) | 1/2 2,2236; 1/3 1,8708; 1/4 1,6858; 1/5 1,5285; 1/6 1,3719; 1/7 1,2636 mm | C9 Tab. 2 | [wörtlich, pdftotext] |
| Einfluss der Geschwindigkeit | „amplitude does not vary more than 10%“ (0,02–0,1 m/s); Spitze „around 2 m/s“ | C10 5.3 | [wörtlich, pdftotext] |
| Abgleiten | „sharp steep rise in the horizontal displacement just prior to the release“ — „kick“ macht den Ton heller | C10 5.2 | [wörtlich, pdftotext] |
| Simulation | Zeitschritt „no more than 1 µs“, Kiel „no less than N = 10⁵“ Punkte (C9); fs 100 kHz, Δx 4 mm (C10) | C9 3.2, C10 5.1 | [wörtlich, pdftotext] |
| Springerweg (Nachbau) | „movement: ~7.22mm“; Kielschlitz 2,30 × 0,70 × 9,22 mm | C20 | [wörtlich, API] |
| Kraft in N, Kontaktdauer an einem echten Kiel | **nicht gelesen** — stehen nach Katalog in C11 (Diss., „aided by various experimental work“) und C12 (Harmonisierung von Kielen am Cembalo, Robotermessung) | C11, C12 | [nur Zusammenfassung] |

---

## 5 Tastenstoß (Knock)

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Ursprung | mit kräftigem Anschlag werden Boden und Moden „excited through mechanical coupling, separate from the string plucking mechanism“ | C7 S. 117 | [wörtlich, pdftotext] |
| Pegel je Stufe | „at least a general 10 dB level difference for the lowest body modes (< 300 Hz) from one dynamic level to another“ (C6-Ton) | C7 Abschn. 3.3 | [wörtlich, pdftotext] |
| Bei pp | pp „perhaps only has an emphasis at 50 Hz and the rest of the response is very noise-like“ | C7 3.3 | [wörtlich, pdftotext] |
| Wahrnehmung | Klopfen „present in the mf and especially in ff tones“ | C7 Abschn. 4 | [wörtlich, pdftotext] |
| Modell | eigenes „dynamic soundboard filter“ am Saitenmodell vorbei; Ziel C6 ff „already with the cascade of only six peak filters“ | C7 Abschn. 5, Abb. 8 | [wörtlich, pdftotext] |
| Unterscheidung | Körpermoden des Stoßes „decay fast (T60 < 0.5 s)“, Nachklang hinter dem Steg bis 4,5 s | C7 Abschn. 5 | [wörtlich, pdftotext] |
| Dauer des Stoßes in ms, Spektrum in Zahlen | **nicht gefunden** (nur Abb. 4, 5, 8) | – | – |
| Loslassgeräusch | Välimäki: ein Sample aus der Aufnahme je Note-off (bekannt, Q1) | C8 | – |

---

## 6 Resonanzboden

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Moden, flämische Kopie | „the soundboard has 36 vibrational modes over a frequency range of 0 to 600 Hz“; Gehäuse mit vielen Moden | C15 Abstract | [wörtlich, API] |
| Luftmoden | entsprechen „J0 and J9 solutions to the Bessel equation for a wedge shaped space“ | C15 Abstract | [wörtlich, API] |
| Abstrahlung | „reasonably flat over a frequency range of 50–2000 Hz“ | C15 Abstract; C8 S. 939 | [wörtlich, API / pdftotext] |
| Dicke allgemein | „about 2 to 4 mm, supported by several ribs“; Helmholtz „usually below 100 Hz“ | C7 S. 115, C8 S. 936 | [wörtlich, pdftotext] |
| Stegantwort Knif/Pelto, 8′-Steg bei C3 | T60 „about 0.5 seconds“ unter 350 Hz, „4.5 seconds“ 350–1000 Hz | C8 Abschn. 3.4, S. 939 | [wörtlich, pdftotext] |
| Saiten hinter dem Steg | „about 6 to 20 cm long“, „very inharmonic“ | C8 Abschn. 3.4, S. 939 | [wörtlich, pdftotext] |
| Messweg | Stege mit Impulshammer an „four and six points for the 4 and the 8 bridge“, Beschleunigung am 8′-Steg an drei Stellen | C8 Abschn. 4.1, S. 940 f. | [wörtlich, pdftotext] |
| Couchet 1652 (Musée de la musique) | Modalzerlegung „for [30-1500Hz] bandwidth“; Zahlen nicht im Abstract | C16 | [wörtlich, API] |
| Moden mit Frequenz und Güte, Stegadmittanz in Zahlen | **nicht gefunden** (C15, C17 hinter Bezahlschranken; Fletcher 1977 Acustica 37 ohne DOI, nicht erreicht) | – | – |
| Dulcken 1755 (lokal, Bader u. a.) | Dicken, Holz, 37 Hz, T60 235 ms — siehe Bericht 26.10. | Q2 | – |

---

## 7 Ziele (Ausklang, Pegel, Dynamik)

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Aufnahme | Knif/Pelto, a′ 415, Vallotti, 8′ hinten, alle C (C2 … C6) je pp/mf/ff, schalltoter Raum, Mikrofone „about 1 m above the soundboard“ | C7 Abschn. 2, 3.1 | [wörtlich, pdftotext] |
| Grundfrequenzen | „C2 = 62.9 Hz, C3 = 124.4 Hz, C4 = 247.6 Hz, C5 = 495.6 Hz, and C6 = 991.2 Hz“ | C7 3.1 | [wörtlich, pdftotext] |
| ff gegen pp, Teiltöne 1/2/3 bei t = 0,4 s | C4: „5 dB, 3 dB, and 0.5 dB“; C6: „0 dB, 2 dB, and 1 dB“ | C7 3.2.1 | [wörtlich, pdftotext] |
| Abklingen je Stärke | „no significant differences or deviating trends in decay times“ | C7 3.2.1 | [wörtlich, pdftotext] |
| Hörversuch (Lautheitsabgleich) | Mittel pp→ff 1,68 dB (σ 0,37), pp→mf 1,08, mf→ff 0,98; größtes Mittel „2.41 dB“ bei C3; „each dynamic step … is about the size of 1 dB“ | C7 Abschn. 4, Tab. 1 | [wörtlich, pdftotext] |
| Absolutpegel | „The ff C4 tone was calibrated to 68.5 dBA“ (nahe dem Ohr des Spielers, italienisches Instrument) | C7 Abschn. 4 | [wörtlich, pdftotext] |
| Tondauer der Aufnahmen | „between 10 and 25 seconds, because the bass tones … ring much longer“ | C8 Abschn. 4.1, S. 940 f. | [wörtlich, pdftotext] |
| T60 je Teilton | nur als Bild (C8 Abb. 4, 8: Beispiel f₀ 197 Hz; Teiltöne 10, 11, 16, 18 ausgenommen) | C8 | [nur Zusammenfassung] |
| Inharmonizität B je Taste | alle 56 Tasten als Bild (C8 Abb. 5) | C8 | [nur Zusammenfassung] |
| Pegel über den Umfang, T60 je Lage als Tabelle | **nicht gefunden** | – | – |

---

## 8 Ladeliste (nicht geladen — erst nach Freigabe)

| Datei (Vorschlag) | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Perng_2012_HarpsichordPlectrumStringInteraction_Diss.pdf` | stacks.stanford.edu/file/druid:wp454hs7976/Jack%20Perng%20Thesis-augmented.pdf | 21 540 441 B | CC BY-NC 3.0 | Kielkraft, Kontakt, Messungen am Springer |
| `Roy_2015_PlateformeRobotiseeCordesPincees_these.pdf` | theses.hal.science/tel-01323985/document | 4 612 942 B | hal-authorisation-v1 | Kiel/Harmonisierung, Anfangsbedingung der Saite |
| `Roy_LeCarrou_Fabre_Vitrani_2015_PlectreClavecin_CFM.pdf` | hal.science/hal-03444591/document | 309 604 B | hal-authorisation-v1 | Kiel als Balken, Messwerte |
| `Penttinen_2006_DynamicsHarpsichordSynthesis_DAFx06.pdf` | dafx.de/paper-archive/2006/papers/p_115.pdf | 279 281 B | kein Vermerk | Tastenstoß, Dynamik (Abschn. 3–5, Bilder) |
| `Perng_Smith_Rossing_2010_PlectrumString_DAFx10.pdf` | dafx.de/paper-archive/2010/DAFx10/PerngSmithRossing_DAFx10_P93.pdf | ~456 KB | kein Vermerk | Kielparameter Tab. 1, 2 |
| `Perng_Smith_Rossing_2011_PlectrumWaveguide_DAFx11.pdf` | dafx.de/paper-archive/2011/Papers/24_e.pdf | ~1,2 MB | kein Vermerk | Abgleiten, Geschwindigkeit |
| `Valimaki_et_al_2004_HarpsichordSynthesis_EURASIP.pdf` | ee.columbia.edu/~dpwe/e6820/papers/ValPK04-harpsi.pdf | ~1,5 MB | „© 2004 Hindawi“, Lehrkopie (Verlagsfassung hinter Login) | T60-Bilder, B je Taste, Stegantwort |
| `DiVeroli_2022_StringingCalculator.xls` | harps.braybaroque.ie/hubbard_images/Stringing-Calculator2022-General.xls | 216 064 B | „Copyright subsists … for your own personal use“ | Mensur je Taste, Hüllkurven der Züge |
| `Zuckermann_2000_FrenchDoubleVI_StringingSchedule.pdf` | zhi.net/s/FD-VI-SCH-2021_08_26-21_49_25-UTC.pdf | 54 710 B | kein Vermerk | Besaitung je Taste (modern) |
| Edinburgh-Datenblätter als HTML (96075, 96081, 96074, 96082, 96088, 96076, 96078) | collections.ed.ac.uk/stcecilias/record/… | je ~100 KB | © 2026 University of Edinburgh | Mensur und Zupfpunkt, Beleg sichern |

Nicht auf der Liste: Kottick 1985 (JSTOR), Savage 1992 (JASA), Giordano 1999 / Griffel 1994 (Elsevier), Paté 2017
(nur Notiz in HAL) — kein freier Volltext gefunden; Goodway & Savage 1992 (Kopie mit unklaren Rechten).

---

## 9 Nicht gefunden

- **Mensur jeder Taste** eines historischen Instruments (nur F und C; zwischen den Stützstellen interpolieren oder
  C4-Tabellenblatt öffnen). O'Brien 2000 (Russell Collection, Besaitung der Taskins) und Hubbard 1965 nicht online.
- **Zupfpunkt getrennt nach Register** (8′ vorn, 8′ hinten, 4′, Lautenzug); C5 nennt je Instrument eine Reihe.
- **Kielkraft in N, Kontaktdauer, Ablösegeschwindigkeit an einem echten Kiel**: nur Modellwerte (C9, C10); die
  Messungen stehen vermutlich in C11 und C12 (ungelesen).
- **Tastenstoß**: Dauer, Spektrum, Pegel in Zahlen (nur „≥ 10 dB je Stufe unter 300 Hz“).
- **Resonanzboden**: Modenfrequenzen mit Güte, Stegadmittanz (eine oder zwei Richtungen). C15 nennt nur die Zahl der
  Moden; Fletcher 1977 nicht erreicht; Kottick 1985 hinter JSTOR.
- **T60 je Lage als Tabelle** und Pegel über den Umfang.
- **Historischer E-Modul** von Eisen- und Messingdraht (nur moderne Werte, C21).
- Taskin-Gaugenummern der Goermans/Taskin-Stimmstockprägung als Zahlen.

---

## 10 Einschätzung (kein Beleg)

- **Saiten je Taste: reicht.** Taskin 1769 ist geschlossen belegbar: Mensur und Zupfpunkt an elf F/C (C5),
  Werkstoff und Drahtnummer für jede Taste beider Chöre (C2 Tab. 8), Durchmesser je Nummer (C2 Tab. 7), Stimmton
  a′ 400 Hz. Die gerechneten Züge (32–73 N) treffen die Mittelwerte von C3. Zwischen den F/C reicht eine glatte
  Kurve; für jede Taste liegt C4 bereit. Offen: der Widerspruch C 1617 / 1681 mm, F₃ „(51)“, die Dichten
  (angenommen), und das 4′ ohne Zupfpunkt.
- **Zupfpunkt je Taste: reicht für ein Register**, für drei nur geschätzt. Sechs Instrumente zeigen dieselbe
  Form (≈ 11 % im Bass bis > 50 % im Diskant, gemessen ab Sattel); das stützt die 12–50 % aus C8.
- **Kiel: Modellwerte reichen für ein erstes Kielmodell** (C9/C10: Maße, E, Ablöseamplituden je Zupfverhältnis);
  für gemessene Kräfte C11 und C12 laden.
- **Tastenstoß:** nur Richtung und Größenordnung (C7); als eigener Pfad am Saitenmodell vorbei, Pegel je Stufe
  ≈ 10 dB unter 300 Hz.
- **Körper: Weg „Plattenwerte + belegte tiefe Moden“.** Ein Bauplan mit allen Maßen eines französischen Doppels
  wurde nicht gefunden. Am besten belegt ist der Dulcken 1755 (Q2, CC BY: Dicken, Holz, Rippen, Helmholtz 37 Hz,
  T60 am Steg 235 ms) — er ist ein flämisch-brabanter Doppel, kein Taskin. Dazu als Prüfgrößen: 36 Moden bis
  600 Hz (C15), flache Abstrahlung 50–2000 Hz (C15, C8), Stegantwort T60 0,5 s / 4,5 s (C8). Ein Bauplan Taskin
  ließe sich aus den Mensuren (Steglinie aus L je Taste) und Maßen in C5 nur ungefähr bilden; „nichts“ wäre
  schlechter als Q2 plus diese Prüfgrößen.
