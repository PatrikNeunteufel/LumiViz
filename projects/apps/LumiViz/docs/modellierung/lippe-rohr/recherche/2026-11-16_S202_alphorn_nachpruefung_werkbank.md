> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_07_alphorn_nachpruefung` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 21:20–21:27, claude-opus-5-5, 58 Runden, success, 2.13 USD; Status `fertig`.
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Marcinnò u. a. 2026 unter `…\cmake\quellen\lippe-rohr\`):
> - **Alle Sätze aus Abschnitt 1.1 in der Textebene gefunden** (S. 7: „The radii 𝑅𝑖 are increasing …“, „𝐿min = 220 mm“,
>   „𝑅min = 6.5 mm …“, „𝑅max = 51.5 mm, so that the conical part connects consistently to the bell geometry“, „𝑟1 and
>   𝑟2 are the radii of the initial and ending sections“; S. 18 Überschriften; S. 1 „bore diameter“).
> - **Tab. 3 (S. 18):** alle zwölf Längen und die Radien 13,50 / 16,00 / 28,50 / 44,00 stehen so in der Textebene;
>   μ_best „6.50, 6.50, 29.90“ ebenso. Die Lesart **Radius** ist damit an der Quelle belegt.
> - **Young 2009:** 51,9 Hz liegt −0,4 Cent neben As1 (51,91 Hz) — nachgerechnet, die Unstimmigkeit im Abstract besteht.
> - **Werkzeug:** die Textebene setzt Formelbuchstaben kursiv (𝑅, 𝐿); `pdf_suche.py` fand „R min“ deshalb nicht. Seit
>   S202 findet der Helfer kursive Buchstaben („Rmin“ findet „𝑅min“) und fehlende ffi/ffl-Ligaturen.
> - **Nicht gegengelesen:** die Handschrift auf Abb. 3a, die Ablesung von Abb. 3b (Becher), die Herstellerlängen
>   (`[Werkzeug]`), das Mundstück.
> - **Nacharbeit:** Auftrag `2026-11-17_27_alphorn_nacharbeit` der Werkbank (Länge, Mundstück, Spielton gegen Spitze).
> Der Auftrag liegt seit S202 im Archiv der Werkbank (`archiv/auftraege/`, `archiv/ergebnisse/2026-11-16_07_alphorn_nachpruefung/` mit
> `seiten/` und Textauszügen, `archiv/laeufe/`). Wo der Bericht `ergebnisse/…` oder `eingang/…` nennt, ist
> `archiv/ergebnisse/…` bzw. `archiv/eingang/…` gemeint.
> **Abgerufene PDFs** liegen nur unter `…\cmake\quellen\` (der Bericht nennt die Namen der Werkbank):
> - `Marcinno_2026_alphorn.pdf` → `quellen\lippe-rohr\Marcinno_etal_2026_PerfectSwissAlphorn_arXiv2607.22823.pdf` — CC BY 4.0 laut arXiv [Werkzeug]
> - `Wachter_2008_stopfen_hoerner.pdf` → `quellen\lippe-rohr\Wachter_2008_StopfenBeiHoernern_Diplomarbeit_mdw.pdf` — In Copyright (rightsstatements InC 1.0) - nicht weitergeben
> **Rechnungen und Ausgaben des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_alphorn_nachpruefung_werkbank/`: `rechnung.py`.

# Bericht: Nachprüfung Alphorn (Bohrung, Becher, Länge, Mundstück)

- **Auftrag:** `auftraege/2026-11-16_07_alphorn_nachpruefung.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5 (Lauf LVR)
- **Reichweite:**
  - 9 Websuchen, 12 Seitenabrufe (WebFetch), 6 `curl`-Abrufe.
  - Marcinnò et al. im Volltext durchsucht (`werkzeuge/pdf_suche.py`) und S. 4, 7, 18 als Seitenbild gelesen. Abb. 3a und 3b
    zusätzlich als Ausschnitte bei 300 und 600 dpi (`pdftoppm`); die Bilder liegen unter `seiten/`.
  - Wachter 2008 nur geladen, nicht neu gelesen: Der Auftrag verlangt dort keine Prüfung, Abb. 52 ist aus dem ersten Lauf
    schon `[wörtlich]` belegt.
  - **Nicht erreicht:**
    - `emuseum.nmmusd.org`: HTTP 403 über `curl` und WebFetch, auch der Pfad `objects/details/10608`.
    - Wayback Machine: CDX-Abfrage „Internet Archive services are temporarily offline“; `wayback/available` meldet keine
      Kopie.
    - Young 2009 PDF: pubs.aip.org antwortet mit 403 und `Cf-Mitigated: challenge` (Cloudflare). Das ist eine Sperre und
      wurde nicht umgangen.
    - Wey/Kammermann 2020: keine frei zugängliche Fassung gefunden.
- **Marken:** `[wörtlich]`, `[Werkzeug]`, `[nur Zusammenfassung]`, `[abgelesen]` (selbst am Seitenbild abgelesen),
  `[eigene Rechnung]`, `[eigene Einschätzung]`.

**Worauf ich aufbaue:** Der Bericht `ergebnisse/2026-11-16_alphorn_geometrie/bericht.md` hat Marcinnò Tab. 1 und 3, den
Wortlaut „R_k are the radii“, μ_best (S. 12), die Lechgold-Angaben und Wachter Abb. 52 schon belegt. Das sammle ich nicht neu.
Tab. 3 habe ich nur noch einmal am Seitenbild (S. 18) angesehen, weil sie die Einheit nennt. Aus dem LumiViz-Repo habe
ich `ic/modell.md` Z. 40 und `ic/offen.md` Z. 24 gelesen. Dort steht heute: Kegel 4 als Zylinder, Becher 0,5 m 44 → 90 mm,
R1 = 2,75 mm, Länge 3,08 m.

---

## 0. Kurzfassung

1. **Radius, nicht Durchmesser.** Marcinnò S. 7 legt Einheit und Lesart fest: „R_min = 6.5 mm … R_max = 51.5 mm, so that
   the conical part connects consistently to the bell geometry“ `[wörtlich]`. Unabhängig davon trägt die fotografierte
   Becherschablone in Abb. 3a handschriftliche **Durchmesser** („Ø“), die genau das Doppelte der Tab.-3-Radien sind:
   - „27-13“ entspricht R2 = 13,5 und R1 = 6,5;
   - „56/28“ entspricht R3 ≈ 28,5;
   - „88/58“ entspricht R4 = 44;
   - „Ø 103“ entspricht R5 = 51,5 `[abgelesen]`.

   Der Widerspruch aus dem ersten Lauf ist damit **zugunsten der Lesart als Radius aufgelöst**.
2. **R1 = 6,5 mm und R5 = 51,5 mm.** Das sind die beiden festen Randradien der Optimierung (S. 7) `[wörtlich]`.
   LumiViz setzt heute R1 = 2,75 mm. Das ist der halbe Wert der Mundstück-Durchgangsbohrung von 5,5 mm, also die Seele des
   Mundstücks und nicht der Anfang von Kegel 1 `[eigene Einschätzung]`.
3. **Kegel 4 ist ein Kegel von 44 auf 51,5 mm** (S. 7, Abb. 2; Schablone Ø 89 → Ø 99/103). Er ist kein Zylinder.
4. **Becher nach eigener Ablesung von Abb. 3b:** r ≈ 51,5–52 mm bis x ≈ 300 mm, danach Aufweitung auf r ≈ 89,5 mm bei
   x ≈ 492 mm. Die Mittellinie ist **≈ 590 mm** lang (Bogenlänge, ±1 %). Das deckt sich mit dem ersten Lauf (585 mm).
5. **Länge:**
   - Fis/Ges: 347 cm (Lechgold, 440 Hz) und 343 cm (Thomann, 440 Hz) `[Werkzeug]`.
   - Gis/As: nur die Wikipedia-Tabelle (3,09 m). Diese Tabelle ist **rechnerisch aus 3,47 m gleichstufig skaliert**
     `[eigene Rechnung]`, also keine eigene Messung. Ein zweiter, unabhängiger Beleg für Gis/As fehlt.
6. **Young 2009:** PDF gesperrt, Bohrungstabelle daher **offen**. Der Abstract-Wortlaut ist jetzt gelesen, ebenso eine
   Unstimmigkeit darin: Die Frequenz 51,9 Hz ist As1, nicht Es.
7. **Mundstück „Kesseltiefe 25 mm“:** Ob das der Kessel allein ist oder die Tiefe bis zur Seele, bleibt **offen**. Keine
   Quelle nennt Rückbohrung, Länge oder Schaft des Mundstücks in Zahlen.

---

## 1. Radius oder Durchmesser in Marcinnò Tab. 3

> Marcinnò, F. et al.: „Can the perfect Swiss alphorn be designed? …“, arXiv:2607.22823v1 (24 Jul 2026),
> https://arxiv.org/pdf/2607.22823, CC BY 4.0. Datei: `eingang/2026-11-16_07_alphorn_nachpruefung/Marcinno_2026_alphorn.pdf`.

### 1.1 Stellen im Volltext, die Einheit und Lesart festlegen

| Stelle | Wortlaut | Marke |
|---|---|---|
| S. 7, Abschn. 3.2 | „The radii R_i are increasing, i.e. R_i ≤ R_{i+1} for i = 1, …, 4. The values of l_min, R1, and R5 are chosen based on experimental measurements reflecting practical manufacturing constraints.“ | [wörtlich], Seitenbild |
| S. 7, Abschn. 3.2 | „The minimum cone length is set to L_min = 220 mm. The minimum radius is set to R_min = 6.5 mm, which corresponds to the smallest radius that can be reliably constructed, while the maximum radius is fixed to R_max = 51.5 mm, so that the conical part connects consistently to the bell geometry.“ | [wörtlich], Seitenbild |
| S. 7, Abschn. 3.2.1 | Kegel als Ω_μc mit „√(y²+z²) ≤ r1 + (r2 − r1)/(x2 − x1)·(x − x1)“; „r1 and r2 are the radii of the initial and ending sections“ | [wörtlich], Seitenbild |
| S. 7 | „the parametrization fixes the total length L of the four conical pieces, as well as the geometry of the mouthpiece (both its position and radii, R_in, R0, R1) and of the bell and the hemisphere (both position and radii, R5 and R_bell)“ | [wörtlich], Seitenbild |
| S. 18, A.3 | „Table 3: Geometrical parameters of the three alphorns (in mm).“ Überschrift A.3: „Measurements of three real alphorns“ | [wörtlich], Seitenbild |
| S. 1 | „no standard specifications for the length or bore diameter“, die einzige Fundstelle für „diameter“ im Volltext | [wörtlich], Text |
| Volltext | „github“, „zenodo“, „repository“, „data availab“, „cross-section“: **0 Treffer**. Die arXiv-Seite listet unter „Code, Data and Media“ nur allgemeine Dienste, kein Repositorium | [Werkzeug] |

Tab. 3 (S. 18) gegengeprüft am Seitenbild `[wörtlich]`:

| Instrument | L1 | L2 | L3 | L4 | R2 | R3 | R4 |
|---|---|---|---|---|---|---|---|
| Alphorn 1 | 544,96 | 844,87 | 754,84 | 444,94 | 13,50 | 28,50 | 44,00 |
| Alphorn 2 | 534,95 | 799,86 | 804,85 | 444,94 | 13,50 | 28,50 | 44,00 |
| Alphorn 3 | 539,92 | 844,91 | 754,84 | 444,94 | 16,00 | 28,50 | 44,00 |

**Folgerung** `[eigene Einschätzung]`:
- Die Kette ist R1 = 6,5 ≤ R2 ≤ R3 ≤ R4 = 44 ≤ R5 = 51,5 mm.
- Beim Becher schließt R5 = 51,5 mm nahtlos an den gemessenen Becherradius von ≈ 51,5–52 mm (Abb. 3b, Abschnitt 3) an.
- Läse man R2–R4 als Durchmesser, endete Kegel 4 bei r = 22 mm. Das widerspräche „connects consistently to the bell
  geometry“ und der Nebenbedingung R4 ≤ R5 nicht, ließe aber einen Sprung auf 51,5 mm am Becheransatz. Die Arbeit
  schließt diese Lesart mit ihrer Definition der Kegel über Radien aus.
- μ_best (S. 12) = (…, 6.50, 6.50, 29.90) trifft R_min = 6,5 mm genau. Damit ist auch die Einheit mm für μ_best belegt.

### 1.2 Unabhängige Prüfung: Schablonenmaße auf der Becherform (Abb. 3a)

Abb. 3a (S. 4) zeigt ein Foto der Becherform („Mold of the bell“). Darauf stehen handschriftliche Notizen. Gelesen habe
ich sie an Ausschnitten bei 300 und 600 dpi (`seiten/abb3a_oben600-04.png`, `seiten/abb3a_unten600-04.png`). **Es ist
Handschrift auf einem Foto. Jede Ziffer ist `[abgelesen]`, Unsicheres ist mit „?“ markiert.**

| Notiz (wie gelesen) | Lesart | halbiert = Radius | Tab. 3 / S. 7 |
|---|---|---|---|
| „COR FEHLMANN \| PROTO 2.1 \| NOUVEAU“ | Hersteller/Prototyp | – | – |
| „Gabarit 75 FA 27-13“, „Gabarit 40 27-13“ | Kegel Ø 27 → Ø 13 | 13,5 → 6,5 | R2 = 13,5 (Horn 1, 2), R1 = R_min = 6,5 |
| „Gabarit 32 56/28 L 940“ (Skizze „925“) | Ø 56 / Ø 28, Länge 940 | 28 / 14 | R3 = 28,5, R2 = 13,5 |
| „Gabarit 43 88/58 L 825“ (Skizze „825“) | Ø 88 / Ø 58, Länge 825 | 44 / 29 | R4 = 44, R3 = 28,5 |
| „Ø 89“, „Ø 99“, „Ø 10[4?]“, „Ø 103“, „Gabarit 500 N° 64 99/89“, „Gabarit 69 102/89“ | Kegel 4 / Becheransatz Ø 89 → Ø 99…103 | 44,5 → 49,5…51,5 | R4 = 44 → R5 = 51,5 |
| „il faut tenir Ø 125 max“, „Douille 100“ | Hülse / Grenze | – | – |
| „Gabarit 74 106-89“, „Gabarit 84 110-89“, „1075“ (Skizze Becher) | weitere Schablonen | – | – |
| „Ø 38? 490 RM?“ | unsicher | – | – |

**Folgerung** `[eigene Einschätzung]`:
- Die Werkstattmaße sind Durchmesser mit „Ø“. Halbiert ergeben sie die Radien aus Tab. 3 und S. 7 auf ±1 mm.
- Das ist ein **zweiter, von der Arbeit unabhängiger Beleg** für die Lesart als Radius.
- Die Schablonenlängen (940, 825) passen **nicht** zu L2/L3 aus Tab. 3 (≈ 845 / 755). Vermutlich sind es Zuschnittlängen
  mit Steckverbindung („Douille“) oder ein anderer Prototyp („Proto 2.1“). Nicht verwenden.

### 1.3 Museumshorn NMM 10608 und andere Vergleichswerte

- **NMM 10608, 10397, 10609:** Selbst nicht gelesen (403, Archiv offline). Die Suchzusammenfassung wiederholt
  „first segment 16 to 27.5 mm, the second 27.5 to 47.2 mm, the third starts at 47.5 mm … bell diameter 170 mm … tube
  length 3870 mm“. Außerdem steht dort, „Stocker uses artificially sawn bells formed on templates“. Alles
  **[nur Zusammenfassung]**; ob Durchmesser oder Radius, steht in keiner gelesenen Antwort.
- **Einordnung** `[eigene Einschätzung]`:
  - Der erste Lauf hatte die zahlenmäßige Nähe 16 / 27,5 / 47 (NMM) gegen 16 / 28,5 / 44 (Tab. 3) als Hinweis auf
    Durchmesser gelesen. Nach 1.1 und 1.2 ist das ein **Zufall der Zahlen**.
  - Vergleicht man Durchmesser mit Durchmesser, beginnen beide Hörner ähnlich: NMM Ø 16 → 27,5, Fehlmann-Schablone Ø 13 → 27.
  - Danach ist das Horn aus Marcinnò weiter: Ø 57 / 88 gegen NMM Ø 47. Für ein F-Horn mit 3,87 m Rohr ist das plausibel,
    aber an der Museumsseite nicht geprüft.
- **Becher gegen Hersteller:**
  - Marcinnò R_bell ≈ 89,5 mm, also innen Ø ≈ 179 mm `[abgelesen]`;
  - NMM 170 mm `[nur Zusammenfassung]`;
  - Thomann Fis „Bell diameter: 200 mm“ `[Werkzeug]`;
  - Lechgold 22 / 25 cm bei 7 / 8,5 mm Wand (erster Lauf, `[Werkzeug]`), also innen ≈ 20,6 / 23,3 cm, falls außen
    gemessen `[eigene Rechnung]`.

  Marcinnòs Becher liegt am unteren Rand, gehört aber zur selben Größenordnung. Als Durchmesser gelesen (r ≈ 45 mm) läge
  er weit darunter. Auch das stützt die Lesart als Radius.
- **Wey/Kammermann 2020:** Laut Galpin Society stehen die Bände 1–73 (1948–2020) auf JSTOR. Eine frei zugängliche Fassung
  habe ich nicht gefunden (`gs.galpinsociety.org/open access.htm` nicht mit dem Artikel; academia/ResearchGate im ersten
  Lauf 403) `[nur Zusammenfassung]`. Die Tabelle ist nicht gelesen.

---

## 2. Young 2009 (POMA 6, 035002)

- **PDF:** nicht geladen, Sperre (siehe Kopf). Ob die Bohrung tabelliert ist, bleibt **offen**.
- **Abstract** (pubs.aip.org → `aipp.silverchair-cdn.com/article-minimal/827933`, über WebFetch) `[Werkzeug]`, wortgleich
  mit dem ersten Lauf, jetzt mit den fehlenden Sätzen:
  - „His dimensions of bore as a function of position are used in the solution of the Webster equation for impedance
    solved numerically earlier for the flügelhorn and the trombone[3].“
  - „The musical notation for the frequencies of the impedance peaks found by the solutions to the Webster equation are
    given below. Here the subscripts refer to the harmonic number.“
  - „The fundamental, rarely used, is 12 cents sharp from Eb at 51.9 Hertz.“
  - „The intonation errors for C1, C2, G3, … D18 are 269, 30, 63, -39, … 28 cents.“
- **Cent-Tabelle des ersten Laufs:** Sie stimmt mit dieser Antwort überein `[Werkzeug]`. Gegen das PDF ist sie **nicht
  geprüft**.
- **Bezug der Cent-Werte** `[eigene Einschätzung]`:
  - Die Indizes sind „harmonic number“. Die Notennamen sind geschrieben (transponierend in C: C, C, G, C, E, G, B♭ …).
  - Die kleinen Werte bei 7 (−13), 11 (+2) und 13 (+11) sprechen für die **Abweichung vom n-fachen Grundton** (harmonische
    Reihe). Gegen die gleichstufige Note läge der 7. Naturton schon ideal bei −31 Cent und der 11. bei −49 Cent.
  - „Intonation errors“ allein entscheidet das nicht.
- **Unstimmigkeit:** 51,9 Hz liegt 500 Cent über Es1 (38,89 Hz) und −0,4 Cent neben As1 (51,91 Hz) `[eigene Rechnung]`.
  „12 cents sharp from Eb at 51.9 Hertz“ passt also zu keiner Lesart, die ich ohne das PDF prüfen kann. Das ist offen.
- **Alexander-Mundstück:** keine Maße in der gelesenen Antwort.

---

## 3. Abb. 3b neu abgelesen (Marcinnò S. 4)

**Bilder:** `seiten/abb3b_mittellinie-04.png` und `seiten/abb3b_radius-04.png`, `pdftoppm -r 300`, Ausschnitt.
Abgelesen ist die schwarze Linie („Smoothed“). Die Punkte des Diagramms habe ich über die Achsbeschriftungen in Pixel
umgerechnet.

**Achsen, Maßstab und Genauigkeit:**
- **Oben (Mittellinie):** x 0–500 mm, Gitter 100 mm; y −50…300 mm, Gitter 50 mm. Maßstab ≈ 1,07 px/mm in x und
  ≈ 1,06 px/mm in y.
  - Die gemessene Kurve (rot) ist ≈ 10 px dick, also ≈ 9 mm; die geglättete (schwarz) ≈ 2 px, also ≈ 2 mm.
  - Ablesegenauigkeit **±3 mm** in x und y, ab x = 400 mm wegen der Steigung ±5 mm in y.
- **Unten (Radius):** x 0–500 mm, Gitter 100 mm; Radius 50–90 mm, Gitter 5 mm. Maßstab ≈ 1,09 px/mm in x und ≈ 9,9 px/mm
  im Radius.
  - Die schwarze Linie ist ≈ 2 px dick, also ≈ 0,2 mm.
  - Ablesegenauigkeit **±0,5 mm** im Radius, ±3 mm in x. Die Messpunkte streuen um ±1 mm um die Glättung.

| x (mm) | y Mittellinie (mm) | Radius (mm) | erster Lauf y / r | Marke |
|---|---|---|---|---|
| 0 | 0 | 51,7 | 0 / 51 | [abgelesen] |
| 100 | ≈ 0,5 | 51,5 | 2 / 52 | [abgelesen] |
| 200 | ≈ 8 | 51,6 | 8 / 51 | [abgelesen] |
| 300 | ≈ 28 | 51,8 | 25 / 52 | [abgelesen] |
| 350 | ≈ 53 | ≈ 57 | 50 / 56 | [abgelesen] |
| 400 | ≈ 92 | ≈ 62 | 85 / 60 | [abgelesen] |
| 450 | ≈ 158 | ≈ 71 | 150 / 70 | [abgelesen] |
| ≈ 492 (Rand) | ≈ 232 | ≈ 89,5 | 232 / 90 (bei 490) | [abgelesen] |

- Die Aufweitung beginnt bei x ≈ 305 mm `[abgelesen]`.
- **Bogenlänge der Mittellinie:** Der Streckenzug über die acht Punkte ergibt **589,5 mm**. Verschiebt man die y-Werte im
  gebogenen Teil um ±5 mm, liegt sie bei 586,6…592,4 mm. Ein Streckenzug unterschätzt eine Kurve leicht, also gilt
  **≈ 590 mm (±1 %)** `[eigene Rechnung]`, Skript `rechnung.py`.
- **R5 = 51,5 mm:** Die Ablesung (51,5–51,8) bestätigt den Text auf S. 7 `[wörtlich]`.
- **R_bell ≈ 89,5 mm** (Ø ≈ 179 mm) `[abgelesen]`.
- **Vergleich mit dem ersten Lauf:**
  - Die Radien stimmen auf ±2 mm.
  - Im gebogenen Teil lese ich y um 3–8 mm höher.
  - Die Bogenlänge ist 590 statt 585 mm; beide liegen innerhalb der Genauigkeit.
  - Der erste Lauf ist damit **bestätigt**.
- **Hinweis:**
  - x ist die waagrechte Koordinate. Der Radius ist laut Abb. über x aufgetragen, nicht über die Bogenlänge.
  - Wer das Profil über die Bogenlänge s baut, muss r(x) auf r(s) umrechnen. Im geraden Teil (x < 250) ist s ≈ x.
  - Am Rand gilt s ≈ 590 mm bei x ≈ 492 mm.

---

## 4. Länge je Stimmung

| Quelle | Stimmung | Länge | Stimmton | mit Mundstück? | Marke |
|---|---|---|---|---|---|
| Lechgold Wanderalphorn Fichte Fis (kirstein.de), erster Lauf | Fis/Ges | „Gesamtlänge: 347cm“ | „Auf 440 Hz gestimmt“ | nicht gesagt | [Werkzeug] |
| Thomann Matterhörnli Deluxe in F# (thomann.ae) | „Key: F# / Gb“ | „Total length: 343 cm“ | „Pitch: A'= 440 Hz“ | nicht gesagt; Mundstück „included“ | [Werkzeug] |
| Thomann Alphorn „Spitzenklasse“ in F# (thomann.co.uk) | F# | nicht angegeben | „Tuned to 442 Hz … 2 cm "pull out" for tuning to 440 Hz“ | – | [Werkzeug] |
| Wikipedia de „Alphorn“, Abschn. „Notation und Stimmung“ | Fis/Ges | 3,47 m | nicht gesagt | nicht gesagt | [Werkzeug] |
| dieselbe Tabelle | Gis/As | 3,09 m | nicht gesagt | nicht gesagt | [Werkzeug] |
| Austria-Forum (AustriaWiki „Alphorn“) | Fis/Ges, Gis/As | 3,47 / 3,09 m | – | – | [nur Zusammenfassung]; ist eine Kopie der Wikipedia |
| NMM 10608 (Stocker F) | F | „tube length 3870 mm“ | – | – | [nur Zusammenfassung] |

**Wikipedia-Tabelle:**
- Sie trägt **keinen eigenen Beleg**. Die Fußnoten 6 und 7 im Abschnitt hängen an Sätzen davor und danach (H.-J. Sommer,
  Notation; B. Weber, 432 Hz) `[Werkzeug]`.
- Alle Werte außer Es sind **genau 3,47 m × 2^(−n/12)**: E 3,895, F 3,676, G 3,275, Gis 3,091, B 2,754, C 2,454 m
  `[eigene Rechnung]`. Es weicht ab: 4,05 m in der Tabelle gegen 4,127 m gerechnet.
- Die Gis/As-Zeile ist also **eine Umrechnung, keine Messung**. Sie ist kein zweiter Beleg.

**Ergebnis:**
- **Fis/Ges:** zwei Hersteller, 343 und 347 cm bei 440 Hz. Ob mit Mundstück, sagt keiner.
- **Gis/As:** **kein** unabhängiger Beleg gefunden.
- Bei 442 Hz wäre ein Horn um den Faktor 0,9955 kürzer, also ≈ 1,5 cm auf 3,4 m `[eigene Rechnung]`. Thomann nennt dafür
  „2 cm pull out“.

**Bezug zu LumiViz** `[eigene Rechnung]`:
- Horn 3 nach Tab. 3 + Becher ≈ 2 584,6 + 590 = **≈ 3,17 m ohne Mundstück**.
- Zu 3,43–3,47 m fehlen 0,26–0,30 m. Ein Mundstück allein deckt das kaum.
- Ob Herstellerlängen außen über Mundstück und Becherrand gemessen sind, bleibt offen.
- Die Zielskala (Tab. 1: 92 Hz ≈ Fis2) belegt Fis für die gemessenen Hörner. Die 3,08 m im Modell (Becher 0,5 m) liegen
  ≈ 9 cm unter dem, was die Arbeit hergibt.

---

## 5. Mundstück

| Quelle | Angabe | Marke |
|---|---|---|
| Lechgold MUA19 (kirstein.de) | Spezifikation: „Kesseltiefe Medium“, „Schaft Alphorn (11 - 18 mm)“, „Kesselweite 19,0“; Merkmale: „Kesselweite: 19 mm“, „Kesseltiefe: 25 mm“, „Durchgangsbohrung: 5,5 mm“; „Bei allen Modellen beträgt die Kesseltiefe 25 mm und die Durchgangsbohrung 5,5 mm.“; keine Länge | [Werkzeug] |
| Thomann Matterhörnli Deluxe F# (Beigabe) | „wooden mouthpiece with inner cup diameter 19 mm, medium deep, bore 5.5 mm“ | [Werkzeug] |
| Alfons Neumann 17,5 (thomannmusic.com) | „Outer cup diameter: 28.5 mm“, „Inner cup diameter: 17.5 mm“, „Cup depth: medium deep“, „Bore: 4.4 mm“; keine Länge, kein Schaft | [Werkzeug] |
| Drittanbieter (bajaao.com), Thomann 23 | Bohrung 4,5 mm | [nur Zusammenfassung] |
| Marcinnò S. 3, 7 | Mundstück R_inlet → R0 nicht linear (Messpunkte), R0 → R1 Kegel, R1 = 6,5 mm; keine Zahlen für R_inlet, R0, Länge | [wörtlich] |

**„Kesseltiefe 25 mm“** `[eigene Einschätzung]`:
- Lechgold nennt für dasselbe Stück „Kesseltiefe Medium“ und „25 mm“, und die 25 mm gelten für **alle** Kesselweiten
  (17–28 mm).
- Ein Kessel von 25 mm Tiefe bei 17 mm Weite wäre kein „medium“ mehr. Die Zahl ist daher eher die Tiefe vom Rand **bis
  zur Seele** (Kessel + Übergang).
- Zum Vergleich das Waldhorn-Mundstück bei Wachter, Abb. 52: Ø 17,5 → Ø 4,65 mm erreicht es bei 31,5 mm Tiefe.
- Belegt ist das nicht. Fragen könnte man Lechgold über den Vertrieb kirstein.de.

**Folge für R1:** Mit „Durchgangsbohrung“ 4,4–5,5 mm (Seele, r 2,2–2,75 mm) und R1 = 6,5 mm (Marcinnò) muss die
Rückbohrung im Mundstück, also R0 → R1, von r ≈ 2,75 auf 6,5 mm aufgehen `[eigene Einschätzung]`. Ihre Länge ist nicht
belegt. „Schaft Alphorn 11–18 mm“ ist ein Passbereich; R1 = 6,5 mm (Ø 13) liegt darin.

---

## 6. Urteil je Prüfpunkt

**Prüfpunkte dieses Auftrags**

| Nr. | Punkt | Urteil | Beleg |
|---|---|---|---|
| A1 | R2–R4 in Tab. 3 Radius oder Durchmesser | **bestätigt: Radius** (Lesart Durchmesser widerlegt) | Marcinnò S. 7 (R_min 6,5 / R_max 51,5 mm, Kegel über r1, r2) [wörtlich]; Tab. 3 „(in mm)“ S. 18 [wörtlich]; Schablone Abb. 3a „27-13“, „56/28“, „88/58“, „Ø 103“ [abgelesen] |
| A1a | R1, R5 als Zahl | **R1 = 6,5 mm, R5 = 51,5 mm** (Radius) | S. 7 [wörtlich]; Zuordnung R1 = R_min, R5 = R_max aus „R1 and R5 are chosen …“ und R_i ≤ R_{i+1} [eigene Einschätzung, eng am Wortlaut] |
| A1b | NMM 10608/10397/10609 | **offen** (Seite 403, Archiv offline) | [nur Zusammenfassung] |
| A1c | Wey/Kammermann 2020 | **offen** (nur JSTOR / Kauf) | Galpin Society [nur Zusammenfassung] |
| A2 | Young 2009: Bohrungstabelle, Cent-Bezug, Mundstück | **offen** (PDF gesperrt); Cent-Tabelle mit Abstract übereinstimmend | Abstract [Werkzeug] |
| A3 | Abb. 3b neu ablesen | **bestätigt** (Abweichung zum ersten Lauf innerhalb der Genauigkeit) | Abschnitt 3 [abgelesen]; Bogenlänge 590 mm [eigene Rechnung] |
| A4 | Länge Fis/Ges und Gis/As aus zwei unabhängigen Quellen | **Fis/Ges bestätigt** (343 / 347 cm, 440 Hz); **Gis/As offen** (nur Wikipedia, rechnerisch skaliert) | Thomann, Lechgold [Werkzeug]; [eigene Rechnung] |
| A5 | Mundstück: Kesseltiefe, Rückbohrung, Länge, Schaft | **offen** | Lechgold, Thomann, Neumann [Werkzeug] |

**Prüfpunkte aus Abschnitt 9 des ersten Berichts**

| Nr. | Punkt | Urteil | Beleg |
|---|---|---|---|
| 9.1 | Radius oder Durchmesser | **Radius bestätigt** | wie A1 |
| 9.2 | Abb. 3b selbst ablesen, Bogenlänge ≈ 585 mm | **bestätigt** (≈ 590 mm ±1 %; r 51,5 → 89,5 mm) | Abschnitt 3 |
| 9.3 | Kegel 4 als Kegel R4 → R5 | **bestätigt** (44 → 51,5 mm über L4 = 444,94 mm) | S. 7 „R_max = 51.5 mm, so that the conical part connects consistently to the bell geometry“ [wörtlich]; Abb. 2 (erster Lauf); Schablone Ø 89 → 99/103 [abgelesen] |
| 9.4 | Young 2009 im Browser | **offen** (gesperrt, kein Browser erlaubt) | – |
| 9.5 | „Kesseltiefe 25 mm“ | **offen**; Deutung „bis zur Seele“ wahrscheinlicher | [eigene Einschätzung] |
| 9.6 | Wikipedia-Längentabelle | **teilweise**: ohne Beleg, Werte gleichstufig aus 3,47 m gerechnet; Fis 3,47 m durch zwei Hersteller gestützt | [Werkzeug], [eigene Rechnung] |

---

## 7. Reicht das für eine Bohrung?

**Jetzt belegt baubar (Horn 3, Marcinnò):**

| Abschnitt | Länge (mm) | r Anfang → Ende (mm) | Beleg |
|---|---|---|---|
| Kegel 1 | 539,92 | 6,5 → 16,0 | Tab. 3 + S. 7 [wörtlich] |
| Kegel 2 | 844,91 | 16,0 → 28,5 | Tab. 3 [wörtlich] |
| Kegel 3 | 754,84 | 28,5 → 44,0 | Tab. 3 [wörtlich] |
| Kegel 4 | 444,94 | 44,0 → 51,5 | Tab. 3 + S. 7 [wörtlich] |
| Becher | ≈ 590 (Bogen) | 51,5 bis x ≈ 305, dann → 89,5 nach Tabelle in Abschnitt 3 | Abb. 3b [abgelesen] |

Dazu kommen die Abstrahlung in eine Halbkugel mit R = 0,447 m (S. 5, erster Lauf) und die Zielskala bzw. gemessenen Töne
(Tab. 1).

**Bliebe Annahme:**
- **Mundstück:** Kessel (Ø 17–19 mm, Tiefe unklar), Seele 4,4–5,5 mm und Rückbohrung bis r = 6,5 mm. Die Länge ist nicht
  belegt.
- **Gesamtlänge:** Gegen Herstellerangaben fehlen ≈ 0,26–0,30 m. Es ist nicht belegt, ob die Hersteller außen mit
  Mundstück messen.
- **Becherform zwischen den Ablesepunkten** und der Öffnungswinkel am Rand: nur ±0,5 mm im Radius bzw. ±5 mm in y.
- **Wandverluste:** ohne Messung.
- **Gis/As-Länge:** nur gerechnet.

---

## 8. Nicht gefunden – Suchwege

| Lücke | wo gesucht | Ergebnis |
|---|---|---|
| NMM 10608/10397/10609 im Wortlaut | `curl` (403), WebFetch `objects/10608/alphorn-f` und `objects/details/10608` (403), Wayback `available` (keine Kopie) und CDX (Archiv offline), WebSearch | nur Zusammenfassung |
| Young 2009 PDF | pubs.aip.org PDF-Pfad (403, Cloudflare), Artikelseite (nur Abstract), WebSearch nach freier Kopie | keine freie Fassung |
| Wey/Kammermann 2020 | WebSearch (Galpin Society, open access, JSTOR) | nur JSTOR/Kauf |
| Code/Daten Marcinnò | Volltext (github, zenodo, repository, data availab: 0 Treffer), arXiv-Abstractseite | keins |
| Gis/As-Länge zweite Quelle | WebSearch DE/EN („Alphorn in As“, „A-flat“, „G#“, Hersteller) | keine; nur Wikipedia/Austria-Forum |
| Mundstück: Länge, Rückbohrung, Schaft, Zeichnung | WebSearch DE/EN (Kesseltiefe, Rückbohrung, Drechseln, backbore, shank); kirstein.de, thomann | nur Kessel, Kesseltiefe (qualitativ bzw. 25 mm), Bohrung |
| Bohrungsmaße von Bauern (Bernatone, Stocker, Neumann, Rocky Mountain) | WebSearch (zwei Suchen, ohne Treffer auf Herstellerseiten mit Maßen) | nichts |

---

## 9. Geladen / nicht geladen

| Datei | Status | Ort |
|---|---|---|
| Marcinnò 2026 (arXiv 2607.22823) | geladen, CC BY 4.0 | `eingang/2026-11-16_07_alphorn_nachpruefung/Marcinno_2026_alphorn.pdf` |
| Wachter 2008 (Phaidra o:11041) | geladen; **„All rights reserved“ (InC)** | `eingang/…/Wachter_2008_stopfen_hoerner.pdf` |
| Young 2009 (POMA) | **nicht geladen**: Cloudflare-Sperre (403, challenge) | Ladeliste |

`HERKUNFT.md` und `SHA256SUMS` liegen im `eingang/`-Ordner.

## 10. Ladeliste (nicht freigegeben oder nicht erreichbar)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Young_2009_alphorn_impedance.pdf` | https://doi.org/10.1121/1.3187533 | unbekannt | „© 2009 Acoustical Society of America“; Cloudflare-Prüfung, nur im Browser | Bohrungsmaße Littleton-Horn, Cent-Bezug, Alexander-Mundstück |
| `Wey_Kammermann_2020_swiss_alphorn.pdf` | JSTOR, GSJ LXXIII (2020) 65–84 | unbekannt | Rechte des Verlags; Konto/Kauf | Tabelle Bohrung am Mundrohr/Becher/Länge historischer Hörner |
| (Webseite) NMM 10608, 10397, 10609 | https://emuseum.nmmusd.org/objects/10608/alphorn-f | – | Museum; 403 für Abrufwerkzeuge | Bohrungsstufen im Wortlaut, Lesart Durchmesser/Radius |

**Wen man fragen könnte** (nur nennen): die Autoren der EPFL (S. Deparis, F. Marcinnò) nach R_inlet, R0, der Mundstücklänge
und den Schablonenlängen; die Werkstatt „Cor Fehlmann“ (Name auf der Schablone, Abb. 3a); Lechgold nach der Bedeutung der
„Kesseltiefe 25 mm“.

---

## 11. Was nicht geliefert ist – was die LumiViz-Sitzung prüfen muss

**Nicht geliefert:**
- Young-Tabelle;
- NMM-Wortlaut;
- Wey/Kammermann-Tabelle;
- zweite Quelle für Gis/As;
- Mundstücklänge und Rückbohrung.

**Selbst prüfen:**
1. Die Zuordnung **R1 = R_min = 6,5 mm, R5 = R_max = 51,5 mm** (S. 7) gegen den Wortlaut lesen. Die Arbeit schreibt nicht
   wörtlich „R1 = 6.5 mm“.
2. Die Handschrift der Schablone (`seiten/abb3a_*600-04.png`) selbst nachlesen, vor allem „27-13“, „56/28“, „88/58“ und
   „Ø 103“.
3. LumiViz setzt heute R1 = 2,75 mm. Das ist die halbe Durchgangsbohrung des Mundstücks, nicht R1 (Abschnitt 5).
4. Der Becher ist in Abb. 3b über x aufgetragen, nicht über die Bogenlänge (Abschnitt 3, Hinweis).
5. Die Restlücke zur Herstellerlänge (≈ 0,26–0,30 m) ist nach den Quellen nicht erklärt.
