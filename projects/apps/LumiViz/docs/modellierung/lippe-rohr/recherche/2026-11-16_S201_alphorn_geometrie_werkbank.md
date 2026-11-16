> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_alphorn_geometrie`
> (gestellt in LumiViz S200), Lauf vom 2026-11-16 18:34–18:42 über das Agent SDK (claude-opus-5-5, 66 Runden,
> regulär beendet, 2,65 USD). **Unverändert abgelegt in S201** — nur dieser Kopf ist dazugekommen.
> **Nicht gegengeprüft:** keine Zahl daraus ist von der LumiViz-Sitzung an der Quelle nachgelesen. Vor dem Gebrauch in
> 34m prüfen, zuerst die Punkte in Abschnitt 9 des Berichts (Radius oder Durchmesser bei Marcinnò, Abb. 3b, Young 2009).
> Im Lauf war die Shell gesperrt (`quelle_suchen.py`, `curl` fielen aus): PDFs sind nur als Seitenbild gelesen.

# Bericht: Geometrie und Prüfgrößen des Alphorns (Vorbereitung Stufe 34m)

- **Auftrag:** `auftraege/2026-11-16_alphorn_geometrie.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5 (Lauf LVR über `auftrag.py`)
- **Reichweite:**
  - 20 Websuchen und 26 Seitenabrufe.
  - Dazu habe ich PDFs seitenweise als Bild gelesen:
    - Marcinnò et al., S. 3–5, 9–12 und 18–20, aus dem Werkzeug-Cache des Laufs vom 2026-10-26;
    - Wachter 2008, PDF-S. 1–4, 59–60 und 62–64;
    - Ziegenhals, DAGA 2010, beide Seiten.
  - **Shell war gesperrt** (Bash und PowerShell im Modus „don't ask“ abgewiesen). Damit fielen `quelle_suchen.py` und `curl` aus. PDFs habe ich deshalb nur über ihre Seitenbilder gelesen, nicht im Volltext durchsucht.
  - **Nicht erreicht:**
    - `emuseum.nmmusd.org` (National Music Museum): 403, dreimal versucht;
    - academia.edu und researchgate.net (Kammermann/Wey): 403;
    - Airtable „Historic Alphorn Archive“: „browser not supported“;
    - smb.museum-digital.de: 404;
    - kirstein.de, Seite „Natur Fis 347“: 404;
    - POMA-Volltext von Young 2009: Die Seite zeigt nur „This content is only available via PDF“; das PDF selbst habe ich nicht abgerufen.
  - **Sprachen:** Deutsch, Englisch, Französisch.
- **Kennzeichnung:** wie in `CLAUDE.md` – `[wörtlich]`, `[Werkzeug]`, `[nur Zusammenfassung]`, `[eigene Rechnung]`, `[eigene Einschätzung]`. `[abgelesen]` heißt: Ich habe den Wert selbst an einer Abbildung abgelesen, die ich als Seitenbild gesehen habe.

**Worauf ich aufbaue:** `offen.md` (Zeile „Alphorn: R1, Mundstück, Becher, Länge“), `modell.md` (Alphorn-Spalte), `modell-entwurf.md` und `recherche/2026-10-26_alphorn_didgeridoo.md`. Was dort steht, wiederhole ich nicht, sondern ergänze es:

- Marcinnò Tab. 3;
- Tab. 1 mit den gemessenen Tönen;
- Lechgold-Mundstück 19 mm / 5,5 mm;
- Fis-Horn 347 cm;
- Freesound und Commons.

---

## 0. Kurzfassung

1. **Kein vollständiges, gemessenes Alphorn-Profil gefunden.** Die beste Quelle bleibt Marcinnò et al. (arXiv 2607.22823). Bei erneutem Lesen ergeben sich daraus aber **drei neue Geometrie-Angaben**, die LumiViz bisher anders setzt:
   - **R5 ≈ 51 mm** statt eines zylindrischen Kegels 4;
   - **Becher ≈ 300 mm fast zylindrisch (r ≈ 51–52 mm), danach Aufweitung auf r ≈ 90 mm**;
   - **Becher-Mittellinie ≈ 0,58 m Bogenlänge**: Die 500 mm in Abb. 3b sind die waagrechte x-Koordinate, nicht die Bogenlänge.
2. **Widerspruch zur Weite der Bohrung:** Marcinnò bezeichnet R2–R4 ausdrücklich als *Radien* (16 / 28,5 / 44 mm). Zwei unabhängige Angaben zu realen Hörnern nennen etwa **halb so große Werte als Durchmesser**:
   - ein Museumshorn in F von Stocker, Kriens 1993: 16→27,5→47,2 mm Durchmesser;
   - Kammermann/Wey 2020: Bohrung am Mundrohr 1,5–1,7 cm.

   Beide Angaben sind nur als Zusammenfassung belegt. Die LumiViz-Sitzung muss das prüfen (Abschnitt 5).
3. **Länge:** Laut der Stimmungstabelle der deutschen Wikipedia hat ein Fis/Ges-Horn 3,47 m, ein **Gis/As-Horn 3,09 m** `[Werkzeug]`. Die heute gerechneten 3,08 m entsprechen also der Länge eines Gis/As-Horns. Das deckt sich mit dem Befund „rund drei Halbtöne zu hoch“ nur grob `[eigene Einschätzung]`.
4. **Mundstück:**
   - Lechgold nennt für alle Größen „Kesseltiefe: 25 mm“ und „Durchgangsbohrung: 5,5 mm“, dazu Kesselweiten von 17–28 mm `[Werkzeug]`.
   - Ein Profil (Durchmesser gegen Länge) gibt es nur für ein **Waldhorn**-Mundstück (Wachter 2008, Abb. 52) `[wörtlich]`.
5. **Prüfgrößen:**
   - **Gemessene Spieltöne** dreier Hörner: Marcinnò Tab. 1, schon bekannt.
   - **Gerechnete Impedanzspitzen mit Cent-Abweichung je Naturton** für ein Littleton-Alphorn in Es: Young 2009, POMA 6, 035002 `[Werkzeug]`. Die Bohrungsdaten stehen nur im PDF.
   - **Gemessene Eingangsimpedanz eines Alphorns:** nicht gefunden.
6. **Aufnahmen:** nichts Neues gegenüber `aufnahmen.md`.

---

## 1. Bohrungsprofil

### 1.1 Marcinnò et al. 2026 – was die Seiten zusätzlich zu Tab. 3 hergeben

> Marcinnò, F., Froidevaux, C., Bocchieri, L., Tenderini, R., Pot, G., Deparis, S.: „Can the perfect Swiss alphorn be designed? …“, arXiv:2607.22823v1, 24 Jul 2026. https://arxiv.org/pdf/2607.22823 – CC BY 4.0 (laut Bericht vom 2026-10-26).

Gelesen habe ich S. 3, 4, 5, 9, 10, 11, 12, 18, 19 und 20 (Seitenbilder).

**Aufbau (S. 3, Abb. 2 und Text)** `[wörtlich]`
- „Subdivision of the alphorn in 6 principal pieces. L_k are the lengths of the pieces and R_k are the radii.“ (Bildunterschrift Abb. 2)
- Die Zeichnung zeigt folgende Kette:
  - Mundstück: R_inlet → R0 → R1;
  - Cone 1: R1 → R2;
  - Cone 2: R2 → R3;
  - Cone 3: R3 → R4;
  - **Cone 4: R4 → R5**;
  - Bell: R5 → R_bell.
  - L = L1 + L2 + L3 + L4; der Becher gehört **nicht** zu L.
- „All the mentioned radii are given from direct measurements.“ (S. 3)
- Mundstück: „the first part (between R_inlet to R0) is non linear … While the second one, between R0 and R1, is a conical piece.“ (S. 3)
- Becher: „R5, R_bell and the starting point of the bell are available measurements. No information regarding the centerlines and the radii in between are given.“ (S. 3)
- „All the three instruments share the same mouthpiece and bell geometries.“ (S. 9)
- Halbkugel für die Abstrahlung: „R = 0.447 m“ (S. 5).
- **Zahlenwerte für R_inlet, R0, R1, R5 und R_bell stehen nirgends im gelesenen Text.** S. 12 bestätigt das: „The boundary radii R1 and R5 are fixed“.

**Ergebnis der Optimierung (S. 12)** `[wörtlich]`
- „μ_best-forward = (0.19, 0.09, 0.72, 6.50, 6.50, 29.90)“: die drei Längenanteile und R2, R3, R4.
- Eine Einheit ist nicht genannt. Nebenbedingung (12): „R_i < R_{i+1}“.
- Wenn die Einheit mm ist, folgt daraus R1 < 6,50 mm `[eigene Einschätzung]`. Das ist die einzige Schranke für R1, die in der Arbeit steht.

**Becher (S. 4, Abb. 3b, „Smoothing of the geometry of the bell“)** `[abgelesen]`

Achsen: oben x (mm) 0–500 gegen y (mm) −50…300, unten x (mm) 0–500 gegen „radius (mm)“ 50–90. Die Gitterlinien liegen in 100-mm-Schritten (x) und 5-mm-Schritten (Radius). Ich schätze die Ablesegenauigkeit auf ±10 mm in x und y und ±1 mm im Radius.

| x (mm) | Mittellinie y (mm) | Radius (mm) |
|---|---|---|
| 0 | 0 | 51 |
| 100 | ≈ 2 | ≈ 52 |
| 200 | ≈ 8 | ≈ 51 |
| 300 | ≈ 25 | ≈ 52 |
| 350 | ≈ 50 | ≈ 56 |
| 400 | ≈ 85 | ≈ 60 |
| 450 | ≈ 150 | ≈ 70 |
| 490 | ≈ 232 | ≈ 90 |

- **Bogenlänge der Mittellinie:** über die Stützpunkte 0/100/200/300/400/490 ≈ 100 + 100 + 101 + 117 + 167 ≈ **585 mm** `[eigene Rechnung aus Abgelesenem, ±5 %]`.
- **R5 ≈ 51 mm** und **R_bell ≈ 90 mm** (Durchmesser ≈ 180 mm) `[abgelesen]`.
- **Öffnungswinkel am Rand:** Der Radius steigt zuletzt um etwa 20 mm auf rund 40 mm x-Strecke. Entlang der Mittellinie, die dort schon steil ist, ist das weniger. Ein belastbarer Winkel lässt sich daraus nicht angeben `[eigene Einschätzung]`.
- **Folgerung für Kegel 4:** Nach Abb. 2 ist Cone 4 ein Kegel von R4 nach R5. Mit R4 = 44,00 mm (Tab. 3) und R5 ≈ 51 mm läuft er also über 444,94 mm von 44 auf ≈ 51 mm `[eigene Einschätzung]`, ist also kein Zylinder.

**Gesamtlänge Horn 3** `[eigene Rechnung]`
- L1…L4 = 539,92 + 844,91 + 754,84 + 444,94 = **2 584,61 mm** (Tab. 3).
- Dazu kommt der Becher mit ≈ 585 mm Bogenlänge, zusammen ≈ **3,17 m ohne Mundstück**.
- Die Länge des Mundstücks steht nicht in der Arbeit.
- Zum Vergleich: Herstellerangabe für Fis 347 cm (siehe 1.3).

**Gemessene Töne:** Tab. 1, S. 10, ist schon im LumiViz-Stand. Neu abgelesen habe ich nur:
- Abb. 6 (S. 11) zeigt die gerechnete Impedanz aller drei Hörner von ≈ 40 bis 610 Hz auf logarithmischer Achse (10⁻² bis 10⁴, „Impedance“ ohne Einheit).
- Die erste Spitze liegt bei etwa 45–50 Hz `[abgelesen]`. Sie ist **gerechnet, verlustfrei**, nicht gemessen.

### 1.2 Museumshorn F, Josef Stocker, Kriens 1993 (National Music Museum, Vermillion)

URL: https://emuseum.nmmusd.org/objects/10608/alphorn-f. **Die Seite war nicht abrufbar (403).** Alles hier stammt aus Zusammenfassungen zweier Suchen und ist **[nur Zusammenfassung]**.

| Angabe | Wert | Marke |
|---|---|---|
| Hersteller / Ort / Jahr | Josef Stocker, Kriens (Luzern), 1993, Seriennummer „3004“ | [nur Zusammenfassung] |
| Stimmung | F | [nur Zusammenfassung] |
| Rohrlänge | „Tube length: 3870 mm“ | [nur Zusammenfassung] |
| Bohrung, Teil 1 | „16-27.5 mm (first segment)“ | [nur Zusammenfassung] |
| Bohrung, Teil 2 | „27.5-47.2 mm (second segment)“ | [nur Zusammenfassung] |
| Bohrung, Teil 3 | „47.5 mm (initial, third segment)“ | [nur Zusammenfassung] |
| Becher | 170 mm | [nur Zusammenfassung] |

Weitere Hörner derselben Sammlung, ebenfalls **[nur Zusammenfassung]**:
- Alphorn B (Nr. 10397): Rohr 2 717 mm, Anfangsbohrung 15,2 mm, Becher 145 mm.
- Rundalphorn Gis/As (Nr. 10609): Rohr ca. 6 660 mm, Anfangsbohrung 15,7 mm, Becher innen ca. 68 mm.

Die Längen der Teile fehlen. Mit drei Durchmesserstufen an den Teilgrenzen wäre das ein **grobes, aber gemessenes** Profil, wenn die Seite selbst gelesen würde.

### 1.3 Herstellerangaben (Lechgold über kirstein.de)

| Modell | Länge | Stimmung | Becher | Wand | Mensur | Marke |
|---|---|---|---|---|---|---|
| Wanderalphorn Fichte Fis, 6-teilig | „Gesamtlänge: 347cm“ | „Stimmung: Fis / Ges“, „Auf 440 Hz gestimmt“ | „Becherdurchmesser: 22 cm“ | „Wandstärke: 7 mm“ | nicht angegeben | [Werkzeug] |
| Spitzenklasse F, 3-teilig | „Gesamtlänge: 363 cm“ | „Auf 442 Hz gestimmt“ | „Becherdurchmesser: 25 cm“ | „Wandstärke: 8,5 mm, gleichbleibend“ | „Breite Mensur“, ohne Zahl | [Werkzeug] |

Der Wikipedia-Artikel „Alphorn“ (de), Abschnitt „Notation und Stimmung“, enthält eine Tabelle mit Stimmung, Länge und tiefstem Ton. Sie gilt als `[Werkzeug]` (vom Abrufwerkzeug als Tabelle wiedergegeben); Belege der Tabelle selbst habe ich nicht geprüft:

| Stimmung | Länge |
|---|---|
| Es | 4,05 m |
| E | 3,89 m |
| F | 3,68 m |
| Fis/Ges | 3,47 m |
| G | 3,27 m |
| **Gis/As** | **3,09 m** |
| B | 2,75 m |
| C | 2,45 m |

### 1.4 Kammermann/Wey 2020 (Galpin Society Journal 73, 65–84)

> Wey, Y., Kammermann, A.: „The Swiss Alphorn: Transformations of Form, Length and Modes of Playing“, Galpin Society Journal 73 (2020) 65–84. Volltext bei academia.edu (110284196, 114759950) und ResearchGate (344902143), **beide 403**.

Nach den Suchzusammenfassungen gilt **[nur Zusammenfassung]**:
- Die Arbeit enthält eine Tabelle vieler historischer Hörner aus der Zeit 1600–1950 mit Länge, Bohrung, Becherdurchmesser, Herkunft und Quelle.
- Beispiele: 223 cm / Becher 6 cm / Bohrung 1,7 cm sowie 218 cm / Becher 9,5 cm / Bohrung 1,5 cm.
- Kernaussage laut Zusammenfassung: „The leadpipe bore is remarkably similar across all periods and regions.“

Das ist **kein Profil**: je Horn nur ein Bohrungswert, und es geht um historische Hörner. Für R1 wäre es aber der beste Anker.

### 1.5 Geprüft und verworfen

- **craftisian.com „Alphorn Experiment #1“:**
  - Maße für einen Eigenbau, übernommen aus „hand-drawn sketches“ einer anderen Website;
  - Rohr ≈ 120″, innen „just under 1"“ am schmalen Ende, 3,385″ am Becheransatz, Becher 7,44″ `[Werkzeug]`.
  - Kein gemessenes Instrument, keine Stimmung, daher nicht verwendbar.
- **Ziegenhals, DAGA 2010, S. 783–784:**
  - Die Suchzusammenfassung beschrieb die Arbeit als „three-part alphorn model“.
  - Tatsächlich geht es um die **Vuvuzela**, mit Mensur in Abb. 3 und Impedanz in Abb. 6 `[wörtlich, Titel]`.
  - Ein Beispiel dafür, dass Suchzusammenfassungen falsche Inhalte erzeugen.
- **Fehlmann 1994 (NTNU):** über NTNU Open und Suche nicht gefunden.

---

## 2. Mundstück

| Quelle | Angabe | Marke |
|---|---|---|
| Lechgold MUA19 (kirstein.de) | „Kesselweite: 19 mm“; „Kesseltiefe: 25 mm“ (Spezifikation: „Kesseltiefe Medium“); „Durchgangsbohrung: 5,5 mm“; „Schaft Alphorn (11 - 18 mm)“; Ahorn | [Werkzeug] |
| Lechgold-Serie (Suchergebnis) | Kesselweiten 17, 18, 19, 20, 21, 22, 23, 25, 28 mm; „Bei allen Modellen beträgt die Kesseltiefe 25 mm und die Durchgangsbohrung 5,5 mm.“ | [nur Zusammenfassung] |
| Lechgold Spitzenklasse F | „Ahorn-Mundstück mit 19 mm Kesselinnendurchmesser“, „einer Bohrung von 5,5 mm“, „Standard-Holzmundstück 19 mm, mitteltief“ | [Werkzeug] |
| Marcinnò et al. | Mundstück gemessen (R_inlet, R0, R1), Zahlen nicht veröffentlicht; Abb. 3c zeigt nur die Netzform | [wörtlich] / Lücke |

**Einschätzung** `[eigene Einschätzung]`:
- „Kesseltiefe 25 mm“ bei 19 mm Kesselweite und „mitteltief“ wirkt für einen Kessel allein sehr tief. Vielleicht ist die Tiefe bis zur Seele gemeint, mit Übergangskegel.
- Unklar bleiben Länge und Konus der Rückbohrung sowie das Kesselvolumen.
- „Schaft 11–18 mm“ ist vermutlich der Außendurchmesser des Schafts. Das wäre eine **obere Schranke für die Bohrung am Mundrohr**: rund 11–18 mm Durchmesser, also r ≈ 5,5–9 mm. Das passt zu Kammermann (1,5–1,7 cm) und zum Museumshorn (16 mm) aus 1.2/1.4.

**Profil eines Waldhorn-Mundstücks** (für Punkt 5 der Aufgabe, liegt am Weg)

> Wachter, S.: „Untersuchungen zum Stopfen bei Hörnern“, Diplomarbeit, Institut für Wiener Klangstil (mdw Wien) / IEM Graz, Oktober 2008. https://phaidra.kug.ac.at/download/o:11041 – S. 55 (PDF-S. 60), Abb. 52 „Mundstück Hermann Ebner“. Die Daten stammen aus Emmerich, B., Diplomarbeit mdw 1999, S. 32. **Lizenz:** auf den gelesenen Seiten nicht vermerkt.

Die Werte stehen als Tabelle im Bild. `[wörtlich]` heißt hier: am Seitenbild gelesen. Einheit mm laut Achse der Nebengrafik.

| Länge | 0 | 5 | 10 | 15 | 20 | 25 | 30 | 30,5 | 31,5 | 35,9 | 45,9 | 50,9 | 55,9 | 60,9 | 65,9 | 67,38 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Durchmesser | 17,5 | 14,77 | 12,77 | 10,3 | 8,4 | 6,23 | 5,2 | 4,75 | 4,65 | 4,7 | 4,73 | 4,73 | 5,04 | 5,5 | 5,8 | 6,3 |

Gemessen wurde an einem Doppelhorn F/B von Engelbert Schmid, Seriennummer 777 (S. 54). Die Arbeit zeigt **gemessene Eingangsimpedanzen** dieses Horns (BIAS) in Abb. 55 und 58, aber **keine Bohrung des Horns**.

---

## 3. Gemessene Prüfgrößen

| Quelle | Was | gemessen/gerechnet | Marke |
|---|---|---|---|
| Marcinnò Tab. 1 (S. 10) | Spieltöne dreier Fis-Hörner gegen die Zielskala | Spieltöne gemessen, Impedanz nur FEM | bekannt, [wörtlich] |
| Marcinnò Abb. 6 (S. 11) | Impedanzkurven 40–610 Hz | **gerechnet**, verlustfrei | [abgelesen] |
| **Young, F. J.: „The input impedance of an alp horn including an Alexander mouthpiece“, Proc. Mtgs. Acoust. 6, 035002 (2009), doi:10.1121/1.3187533** | Impedanzspitzen je Naturton mit Cent-Abweichung; Alphorn von Joseph Littleton (Hammondsport, NY) | **gerechnet** (Webster-Gleichung aus „bore dimensions“) | siehe unten |
| Young, JASA 125(4 Suppl.), 2598 (2009), doi:10.1121/1.4783889 | Kurzfassung desselben Vortrags | – | [Werkzeug, Crossref-Feld] |

**Young 2009, Seite pubs.aip.org (article-minimal 827933)** `[Werkzeug]`. Ich habe nur die Antwort des Abrufwerkzeugs gesehen, nicht das PDF.
- „The Alp horns built and sold by the late Joseph Littletown of Hammondsport, NY are analyzed.“
- „The fundamental, rarely used, is 12 cents sharp from Eb at 51.9 Hertz.“
- Abweichungen in Cent laut Werkzeug, nach Naturton:

| Naturton | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Name laut Seite | C1 | C2 | G3 | C4 | E5 | G6 | B7b | C8 | D9 | E10 | F11 | G12 | A13 | B14b | B15 | C16 | C17# | D18 |
| Cent | 269 | 30 | 63 | −39 | −41 | −25 | −13 | −7 | −11 | −1 | 2 | 11 | 11 | 15 | 15 | 20 | 24 | 28 |

- **Nicht gegengeprüft.** Der Bezug (harmonische Reihe oder gleichstufige Note) ist unklar. Bei Naturton 7, 11 und 13 deuten die kleinen Werte auf den Bezug zur **harmonischen Reihe** `[eigene Einschätzung]`.
- Das Horn steht in Es, nicht in Fis.
- Lizenz: „© 2009 Acoustical Society of America.“

**Gemessene Eingangsimpedanz eines Alphorns:** nicht gefunden (siehe Abschnitt 7). **Gemessene Spektren oder Pegel je Naturton:** nicht gefunden.

---

## 4. Aufnahmen

Gegenüber `aufnahmen.md` gibt es nichts Neues.
- **Freesound, Suche „alphorn“:** 7 Treffer, davon laut Filter 4 mit „Attribution“ und 3 mit „Creative Commons 0“ `[Werkzeug]`. Es sind die vier Metzeral-Ensembleaufnahmen (755354, 755422, 755780, 755817), „swissbalcony.wav“ (59723, Atmosphäre) und „Alphorn“ (784885).
- **Keine Einzeltöne.**
- **Commons, Mediensuche „alphorn“, Audio:** Alphorn-Fa.ogg (synthetisch); De-Alphorn.ogg und De-Alphorns.ogg sind dem Namen nach Aussprachedateien; dazu rumänische Bucium/Tulnic-Dateien `[Werkzeug]`.
- **Keine Alphorn-Einzeltöne mit klarer Lizenz gefunden.**

---

## 5. Widerspruch: Radius oder Durchmesser?

| Ort im Rohr | Marcinnò Horn 3 (als **Radius** bezeichnet, Tab. 3) | Durchmesser daraus | Museumshorn Stocker F (Durchmesser) [nur Zusammenfassung] | Kammermann (Bohrung Mundrohr) [nur Zusammenfassung] |
|---|---|---|---|---|
| Anfang Rohr | R1 < 6,5 mm (S. 12, siehe 1.1) | < 13 mm | 16 mm | 15–17 mm |
| nach ≈ 0,54 m | R2 = 16,0 mm | 32 mm | (Teil 1: 16 → 27,5) | – |
| nach ≈ 1,38 m | R3 = 28,5 mm | 57 mm | (Teil 2: 27,5 → 47,2) | – |
| nach ≈ 2,14 m | R4 = 44,0 mm | 88 mm | 47,5 (Anfang Teil 3) | – |
| Becheransatz | R5 ≈ 51 mm (Abb. 3b) | ≈ 102 mm | – | – |
| Becherrand | ≈ 90 mm (Abb. 3b) | ≈ 180 mm | 170 mm | – |

**Einschätzung** `[eigene Einschätzung]`:
- Die Werte für R2–R4 decken sich zahlenmäßig auffällig mit den **Durchmessern** des Museumshorns: 16 / 27,5 / 47 gegen 16 / 28,5 / 44.
- Der Becher in Abb. 3b (r 51 → 90, also ≈ 180 mm Durchmesser) passt dagegen zur Lesart als Radius und zu den Becherangaben der Hersteller (200–250 mm).
- Liest man R2–R4 als Durchmesser, entstünde zwischen Kegel 4 (Ende 22 mm Radius) und Becher (51 mm Radius) ein Sprung auf mehr als das Doppelte. Ohne die nicht veröffentlichte R5-Zahl kann ich das nicht entscheiden.
- Wichtig: Die Arbeit sagt „R_k are the radii“ `[wörtlich]`. Der Vergleich stützt sich auf **ein** Museumshorn in F, belegt nur als Zusammenfassung, mit unbekannten Teillängen.
- Wer die Bohrung baut, sollte das klären, bevor eine Länge angepasst wird: Eine doppelt so weite Bohrung verschiebt die Resonanzen anders als eine fehlende Länge.

---

## 6. Reicht das für eine Bohrung?

**Belegt bauen lässt sich (Horn 3 nach Marcinnò):**
- L1…L4 und R2…R4 (Tab. 3, `[wörtlich]`), mit dem Vorbehalt aus Abschnitt 5;
- Kegel 4 von R4 nach R5 (Abb. 2, `[wörtlich]`);
- R5 ≈ 51 mm, Becher fast zylindrisch über ≈ 300 mm x, dann Aufweitung auf ≈ 90 mm, Mittellinie ≈ 585 mm lang (Abb. 3b, `[abgelesen]`, ±1 mm / ±5 %);
- Stimmung Fis mit Zielskala und gemessenen Spieltönen (Tab. 1).

**Bliebe Annahme:**
- **R1 und das Mundstück:** Durchgangsbohrung 5,5 mm, Kessel 19 mm, „Kesseltiefe 25 mm“ (Hersteller). Die Rückbohrung ist unbekannt. R1 ist nur durch R1 < 6,5 mm (S. 12, Einheit erschlossen) und Schaft 11–18 mm eingegrenzt.
- **Mundstücklänge:** nicht belegt. Die Gesamtlänge ergibt sich nur aus ≈ 3,17 m plus Mundstück, gegen 3,47 m (Hersteller und Wikipedia für Fis).
- **Öffnungswinkel am Becherrand:** nur grob ablesbar.
- **Radius oder Durchmesser:** Abschnitt 5.
- **Wandverluste:** unverändert ohne Messung.

**Prüfgröße:** Für Horn 3 gibt es nur gemessene Spieltöne, keine gemessene Impedanz.

---

## 7. Nicht gefunden – Suchwege

| Lücke | wo gesucht | Ergebnis |
|---|---|---|
| vollständiges Profil Alphorn | WebSearch EN/DE/FR (bore profile, Mensur, Bauplan, perce, plans/taper); kirstein.de; craftisian; NMM eMuseum; Airtable „Historic Alphorn Archive“; Wikipedia de; SMB museum-digital | kein Profil; NMM nur grob und blockiert; Airtable nicht lesbar |
| Fehlmann 1994 (NTNU) | WebSearch Titel + NTNU | nicht gefunden |
| Kammermann/Wey 2020, Volltext | academia.edu, ResearchGate, ProQuest-Treffer | 403 bzw. kein PDF |
| gemessene Eingangsimpedanz Alphorn | WebSearch (BIAS, reflectometry, Wiener Klangstil, Forum Acusticum, ISMA, Acta Acustica, DAGA); Crossref; PubMed | nur Rechnungen (Young 2009, Marcinnò); der DAGA-Treffer war Vuvuzela |
| Mundstück: Rückbohrung, Volumen | kirstein.de (Lechgold), WebSearch DE/FR | nur Kessel, Tiefe, Bohrung |
| Spektren/Pegel je Naturton | WebSearch | nichts |
| Einzelton-Aufnahmen | Freesound-Suche, Commons-Mediensuche | nichts Neues |
| Waldhorn/Tuba-Bohrung | nicht eigens gesucht (Auftrag Punkt 5) | nur Waldhorn-Mundstück (Wachter) |

---

## 8. Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Young_2009_alphorn_impedance.pdf` | https://doi.org/10.1121/1.3187533 (POMA 6, 035002) | unbekannt | „© 2009 Acoustical Society of America“; frei zugänglich? ungeprüft | Bohrungsmaße Littleton-Alphorn und Impedanzspitzen in Cent – **wichtigster offener Kandidat für ein vollständiges Profil** |
| `Wey_Kammermann_2020_swiss_alphorn.pdf` | https://www.academia.edu/110284196/ oder https://www.researchgate.net/publication/344902143 | unbekannt | Galpin Society Journal, Rechte des Verlags; Konto nötig | Tabelle Länge/Bohrung/Becher vieler Hörner (R1-Anker) |
| `Wachter_2008_stopfen_hoerner.pdf` | https://phaidra.kug.ac.at/download/o:11041 | 5,3 MB | Lizenz nicht gesehen (Phaidra-Datensatz prüfen) | Waldhorn-Mundstückprofil (Abb. 52), gemessene Impedanz F/B-Horn |
| `Marcinno_2026_alphorn.pdf` | https://arxiv.org/pdf/2607.22823 | ≈ 5 MB | CC BY 4.0 (Metadaten) | nur falls noch nicht in der Ablage; Abb. 3b zum Ablesen |
| (Webseite) NMM 10608 | https://emuseum.nmmusd.org/objects/10608/alphorn-f | – | Museum, ungeprüft | Bohrungsstufen von Hand im Browser prüfen |

---

## 9. Was nicht geliefert ist – was die LumiViz-Sitzung prüfen muss

**Nicht geliefert:**
- ein vollständiges, gemessenes Profil eines Fis-Horns;
- Rückbohrung und Volumen des Mundstücks;
- gemessene Impedanz;
- Spektren;
- Einzeltöne;
- Tuba.

**Selbst prüfen:**
1. **Radius oder Durchmesser** in Marcinnò Tab. 3 (Abschnitt 5). Möglichst über die Autoren (EPFL, Deparis) oder über die NMM-Seite im Browser.
2. **Abb. 3b selbst ablesen** (S. 4): die Bechertabelle in 1.1 und die Bogenlänge ≈ 585 mm. Das Modell setzt heute 0,5 m und 44 → 90 mm.
3. **Kegel 4** nach Abb. 2 als Kegel R4 → R5, nicht als Zylinder.
4. **Young 2009** im Browser öffnen: Sind die Bohrungsmaße tabelliert? Worauf beziehen sich die Cent-Werte?
5. **„Kesseltiefe 25 mm“** (Lechgold): Plausibilität prüfen, eventuell beim Hersteller nachfragen.
6. Die Wikipedia-Längentabelle ist nur `[Werkzeug]`. Ihre Belege habe ich nicht gesehen.
