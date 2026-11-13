# 34k-5, zweiter Fall nach KQ7 a und KQ8 a: die Oud mit ihren drei Schalllöchern und mit Luftraum

> **Sitzung:** S196 (2026-11-13). Quelle: Abdallah u. a. 2026 (Meccanica 61:89, CC BY 4.0), Tab. 3 am Original als Bild
> gelesen (S. 10). Programme unter `proben/`: `2026-11-13_S196_oud_bauplan_loecher.py` (schreibt den Bauplan in
> Fassung 4; Schalter `--ohne-loecher`, `--ohne-balken`, `--steg`, `--faser <grad>`, `--balken-bis-rand`),
> `2026-11-13_S196_formen_bild.py` (zeichnet die Formen aus `InstrumentBodyTool --shapes`, mit den Kennzahlen *sym* und
> *netto*). Gerechnet mit `InstrumentBodyTool <plan> --to 500 --statistics-to 0 --shapes <datei>` (Release-clang, rund
> eine Sekunde je Lauf). Kein Bauplan unter `asset/` abgelegt.

## 1 Was neu ist

`InstrumentBody` rechnet Löcher (KQ7 a): im Bauplan `plate.holes`, im Netz ausgespart, der Lochrand frei, ohne Fläche
in Abstrahlung und Masse. Geprüft am Kreisring (außen eingespannt, innen frei) gegen die geschlossene Lösung nach
Kirchhoff: die ersten fünf Moden auf 0,09 %. Die Läufe „ohne Löcher“ geben die Zahlen von S195 genau wieder.

Die Löcher der Oud: Ø 10,5 cm in 30,0 cm Höhe; zwei Ø 3,9 cm in 20,4 cm Höhe, ±10,3 cm (aus Abb. 3, S195). Kein Balken
kreuzt ein Loch.

## 2 Ergebnis: die Schwelle ist wieder verfehlt

Gemessen (Tab. 3): **142 / 226 / 242 / 280 Hz**. Rechnung der Quelle mit Balken: 140 / 210 / 239 / 303 Hz.

Faser längs (wie gebaut und wie die Quelle sie beschreibt), mit Balken, ohne Steg, Rand eingespannt:

| Decke | mit Löchern: erste Moden (Hz) | ohne Löcher (S195) | Abstand mit Löchern zu 142 / 226 / 242 / 280 |
|---|---|---|---|
| 1,5 mm | 143 / 169 / 260 / 276 / 281 | 138 / 162 / 260 / 279 / 294 | +1 % / −25 % / +7 % / −1 % |
| 1,75 mm | 148 / 186 / 291 / 304 / 317 | 143 / 178 / 290 / 306 / 330 | +4 % / −18 % / +20 % / +9 % |
| 2,0 mm | 153 / 202 / 318 / 328 / 348 | 147 / 194 / 319 / 330 / 359 | +8 % / −11 % / +31 % / +17 % |
| 2,25 mm | 157 / 218 / 343 / 349 / 375 | 150 / 210 / 345 / 350 / 385 | +11 % / −4 % / +42 % / +25 % |
| 2,5 mm | 161 / 234 / 365 / 369 / 399 | 154 / 225 / 368 / 371 / 407 | +13 % / +4 % / +51 % / +32 % |
| 3,0 mm | 170 / 266 / 402 / 407 / 439 | 163 / 255 / 398 / 420 / 446 | +20 % / +18 % / +66 % / +45 % |

**Die Löcher ändern wenig:** die erste Mode steigt um 3–5 %, die zweite um 4–5 %, die dritte bleibt. Mit keiner Dicke
zwischen 1,5 und 3 mm liegen alle vier innerhalb von 10 %. Die Annahme aus S195, die fehlenden Löcher seien die
Ursache, ist damit widerlegt.

## 3 Zuordnung der Formen

Die Quelle zeigt Formen nur für ihre Rechnung (Tab. 3, Betrag der Auslenkung ohne Vorzeichen); die vier **gemessenen**
Frequenzen ordnet sie nur über die Frequenz zu. Zugeordnet ist hier also Rechnung gegen Rechnung.

| Form (Quelle, mit Balken) | Quelle | hier, 1,5 mm | hier, 1,75 mm | hier, 2,0 mm | Kennzahlen hier |
|---|---|---|---|---|---|
| Bauch am großen Loch und Bauch im unteren Feld | 140 | 143 (+2 %) | 148 (+6 %) | 153 (+9 %) | sym 1,00, netto 1,00 |
| Bauch im unteren Feld (wo der Steg sitzt) | 210 | 169 (−20 %) | 186 (−11 %) | 202 (−4 %) | sym 1,00, netto 0,25–0,34 |
| Band im mittleren Feld, schwächer oben und unten | 239 | 260 (+9 %) | 291 (+22 %) | 318 (+33 %) | sym 1,00, netto 0,13–0,24 |
| zwei Bäuche nebeneinander im unteren Feld | 303 | 276 (−9 %) | 304 (0 %) | 328 (+8 %) | sym 0,00 |

- **Die Arten der Formen stimmen überein**, auch die Reihenfolge (drei links-rechts-gleiche, dann die erste
  gegengleiche). Die Lesart von S195, die zweite Mode habe „zwei Bäuche nebeneinander“, war falsch: sie ist
  links-rechts-gleich, Loch-Feld und unteres Feld schwingen gegeneinander (deshalb strahlt sie schwach).
- **Die Abstände lassen sich nicht mit einer Dicke schließen:** die zweite Form will 2,0–2,25 mm, die dritte höchstens
  1,5 mm. Das Verhältnis dritte zu zweiter Form ist hier 1,53–1,58, in der Rechnung der Quelle 1,14, in der Messung
  1,07 — unabhängig von der Dicke.
- In der Quelle sind Loch-Feld und unteres Feld zwei fast getrennte Bäuche (140 und 210 Hz); hier spannt die erste
  Mode als ein Bauch über die Balken 2 und 3 hinweg (Bild `proben/2026-11-13_S196_oud_formen_laengs_1.5mm.png`).

## 4 Gegenproben: woran die Rechnung hängt

| Annahme | Lauf | erste Moden (Hz) | Befund |
|---|---|---|---|
| Balkenenden | wie Abb. 3: sie enden 1,3–1,5 cm vor dem Rand (1,75 mm) | 148 / 186 / 291 / 304 | — |
| | bis an den eingespannten Rand verlängert (1,75 mm) | 228 / 328 / 362 / 427 | **die erste Mode +54 %** |
| Faser der Decke | längs (2,0 mm) | 153 / 202 / 318 / 328 | Folge der Formen S, S, S, A |
| | quer (2,0 mm) | 135 / 187 / 206 / 228 | Folge S, S, S, A; −5 % / −17 % / −15 % / −19 % zur Messung |
| | quer (2,5 mm) | 156 / 208 / 240 / 285 | **+10 % / −8 % / −1 % / +2 % zur Messung** |
| ohne Balken, Faser längs (4,0 mm) | | 108 / 173 / 241 / 281 | zweite Mode gegengleich; Quelle ohne Balken: 117 / 182 / 204 / 275 mit gegengleicher Form erst an vierter Stelle |
| ohne Balken, Faser quer (3,0 mm) | | 120 / 162 / 219 / 260 / 282 | Folge wie die Quelle (gegengleich an fünfter Stelle), 162 gegen 182 (−11 %) |
| Steg (S195, ohne Löcher, 1,75 mm) | 13 cm × 12 mm × 8 mm | 139 / 172 / 276 / 301 | senkt alles um 2–5 %, hebt die zweite Form nicht |

Drei Dinge folgen daraus:

1. **Der Bauplan ist aus dieser Quelle nicht bestimmt.** Frei sind die Deckendicke, der Anschluss der Balkenenden an
   die Schale, die Nachgiebigkeit des Rands (die Quelle rechnet eine ganze Schale und hält sie nur stellenweise fest),
   Maße und Lage des Stegs — und vier Frequenzen ohne gemessene Form reichen nicht, um sie festzulegen. Allein der
   Anschluss der Balkenenden verschiebt die erste Mode von 148 auf 228 Hz.
2. **Mit quer liegender Faser und 2,5 mm trifft die Rechnung alle vier gemessenen Frequenzen innerhalb von 10 %**
   (156 / 208 / 240 / 285). Das ist kein Beleg: eine Oud-Decke hat die Faser längs, und die Quelle beschreibt sie so
   (S. 5: „the longitudinal axis (L) runs along the grain, parallel to the instrument's neck-to-bridge axis“). Es zeigt
   nur, dass eine falsche Annahme die Schwelle erfüllen kann — die Schwelle „vier Frequenzen innerhalb 10 %“ prüft den
   Bauplan also nicht.
3. **Die Rechnung der Quelle ohne Balken passt in der Folge der Formen besser zu „Faser quer“** (zweite Mode
   längs geteilt bei 182 Hz, gegengleich erst bei 275 Hz). Bei einer Platte mit der steifen Achse längs ist das
   Verhältnis umgekehrt zu erwarten (D_L/L⁴ gegen D_R/B⁴ rund 4 : 1 bei 50 cm × 37 cm). Ob das an der Schale, am Rand
   oder an den Achsen in ihrem Modell liegt, steht nicht im Text — eine Vermutung, kein Befund.

## 5 Was die Messung nicht hergibt

Die vier gemessenen Spitzen stammen von einer bespielbaren Oud mit Luftraum und Saitenzug. Ohne Luftraum rechnet weder
die Quelle noch `InstrumentBody`; bei Gitarren verschiebt die Kopplung von erster Deckenmode und Luftraum beide um
zweistellige Prozent (Zwei-Oszillator-Modell der Gitarre — hier aus dem Gedächtnis, nicht an einer Quelle der Ablage
nachgelesen und für die Oud nicht nachgerechnet). Welche der vier Spitzen zu welcher Form gehört, ist nicht gemessen.

**Überschlag, kein Befund:** Die Quelle nennt weder Tiefe noch Volumen der Schale. Mit einer angenommenen Tiefe von
19 cm (halbes Ellipsoid 50 × 37 × 19 cm, rund 18 l) und dem großen Loch allein (Ø 10,5 cm, Endkorrektur 1,7 r) liegt
die Helmholtz-Frequenz bei rund 125 Hz — in der Nähe der ersten gemessenen Spitze (142 Hz). Wären 142 und 226 Hz das
Paar aus Luftraum und erster Deckenmode (zwei gekoppelte Schwinger, f₋² + f₊² = f_Luft² + f_Decke²), läge die
ungekoppelte erste Deckenmode bei rund 230 Hz; der Lauf „Balken bis an den Rand“ gibt 220–235 Hz bei 1,5–2,0 mm. Das
ist eine Rechnung mit einem angenommenen Volumen und erklärt 242 und 280 Hz nicht (die nächsten Moden jenes Laufs
liegen bei 320–390 Hz). Prüfen lässt es sich erst mit dem Luftraum in `InstrumentBody` (F-K1 a, in 34k-5 ohnehin
vorgesehen) und einer belegten Schalentiefe.

## 6 Befund

- KQ7 a ist gebaut und geprüft; die Löcher gehören zur Geometrie und bleiben.
- Die Oud verfehlt die Schwelle aus KQ6 a auch mit Löchern. Die Zuordnung zeigt: Rechnung und Quelle meinen dieselben
  Arten von Formen, aber die Abstände je Form (bis −20 % und +33 %) lassen sich nicht auf eine einzelne Annahme
  zurückführen — mehrere unbelegte Annahmen wirken je für sich stärker als die Schwelle.
- Nach KQ7 a kommt der Weg damit noch einmal zur Vorlage. **Entschieden: KQ8 a** (Patrik S196) — erst der Luftraum,
  dann die Oud noch einmal (§7).

## 7 Mit Luftraum (KQ8 a)

`InstrumentBody` koppelt die Moden der Platte über den Hohlraum (`Luftraum.hpp`, F-K1 a; im Bauplan der Block
`cavity`). Geprüft gegen die geschlossenen Lösungen (Helmholtz, Kasten, zwei gekoppelte Schwinger) und gegen den
dichten Löser von Eigen.

**Belegt aus Abb. 3 (am Bild gelesen, S196):** die Schale ist **21 cm tief** (Maßzahl der Seitenansicht), die Löcher
haben **R 5,3 cm** und **R 2,0 cm** (Maßzahlen; S195 hatte 10,5 und 3,9 cm Durchmesser aus dem Bild gemessen).
**Annahme:** jeder Querschnitt der Schale eine halbe Ellipse, ihre Tiefe proportional zur Breite der Decke an der
Stelle — das gibt **20,6 l**. Mündungskorrektur 1,7 Radien, Güte der Luft 30 (beide gesetzt). Helmholtz bei starren
Wänden mit allen drei Löchern: 154 Hz.

Gemessen: **142 / 226 / 242 / 280 Hz**. Gerechnet mit Luftraum, Faser längs, in Klammern der Anteil der
Bewegungsenergie in der Luft der Löcher:

| Balkenenden | Decke | gekoppelte Moden (Hz) | Platte allein (Hz) |
|---|---|---|---|
| 1,4 cm vor dem Rand (wie in Abb. 3 gelesen) | 1,5 mm | 100 (0,65) / 168 / 208 (0,29) / 268 / 275 / 281 | 143 / 169 / 260 / 276 |
| | 2,0 mm | 110 (0,66) / 201 / 209 (0,32) / 323 / 331 / 345 | 153 / 202 / 318 / 328 |
| | 2,5 mm | 117 (0,69) / 208 (0,30) / 233 / 368 / 372 / 396 | 161 / 234 / 365 / 369 |
| 0,3 cm vor dem Rand | 2,0 mm | 128 (0,80) / 226 (0,20) / 292 / 365 | — |
| **am Rand** | **1,5 mm** | **132 (0,87) / 243 (0,12) / 305 / 324 / 378** | 213 / 302 / 324 / 378 |
| | 1,75 mm | 135 (0,88) / 249 (0,11) / 311 / 357 | 221 / 309 / 357 |
| | 2,0 mm | 137 (0,90) / 252 (0,10) / 319 / 386 | 227 / 318 / 386 |

Die Zeilen „am Rand“ sind mit drei Netzen gerechnet (Kante für 500, 3000 und 6000 Hz: 1 670 bis 6 860 Knoten) — die
Frequenzen ändern sich nicht. Zwischen 0,3 und 0,6 cm Abstand kippt das Ergebnis: darunter wirkt der Balken wie am Rand
befestigt, darüber wie frei endend; die Annahme ist praktisch zweiwertig.

**Lesart:**

- **Die erste gemessene Spitze (142 Hz) ist die Mode der Luft** (Anteil der Luft 0,87–0,90), nicht die erste
  Deckenmode — das hatte der Vergleich ohne Luftraum verwechselt. Mit Balken am Rand liegt sie bei 132–137 Hz
  (−7 … −4 %), mit frei endenden Balken bei 100–117 Hz (−30 … −18 %).
- **Die zweite (226 Hz) ist die erste Deckenmode, von der Luft nach oben geschoben:** 243–252 Hz mit Balken am Rand
  (+7,5 … +11,5 %), 226 Hz bei 0,3 cm Abstand und 2,0 mm.
- **Die vierte (280 Hz)** hat eine eigene gerechnete Mode bei 305–319 Hz (+9 … +14 %; 292 Hz bei 0,3 cm).
- **Die dritte (242 Hz) hat keine eigene gerechnete Mode.** Sie liegt 0,4 % neben der 243 Hz der Zeile „am Rand,
  1,5 mm“ — aber diese Mode ist dann schon der zweiten Spitze zugeordnet. 226 und 242 Hz liegen 7 % auseinander; ob
  beide Deckenmoden sind (oder eine davon Schale, Hals oder eine gegengleiche Form), sagt die Quelle nicht.

**Gegen die Schwelle aus KQ8 a** („jede gemessene Spitze hat eine gerechnete Mode innerhalb von 10 %“): mit Balken am
Rand und 1,5 mm dem Wortlaut nach erfüllt (−7 % / +7,5 % / +0,4 % / +9 %), aber nur, weil eine gerechnete Mode zwei
Spitzen bedient. Mit je eigener Mode: drei von vier (142, 226, 280), die Spitze bei 242 Hz bleibt ohne Gegenstück.
Die Zuordnung der Formen ist mit dem Bild `proben/2026-11-13_S196_oud_formen_luftraum_balken_am_rand_1.75mm.png`
belegt; gemessene Formen gibt es nicht.

**Benannte Annahmen dieses Bauplans:** Deckendicke 1,5–2,0 mm; Balkenenden am Rand befestigt (Abb. 3 zeichnet sie
kurz davor endend; am Instrument sitzen sie an der Schale — nicht belegt); Querschnitt der Schale; Mündungskorrektur
und Güte der Luft; kein Steg; Rand eingespannt, Schale starr.

## 8 Die gemessene Kurve (Abb. 6) — nach KQ9 a gelesen

Beim Bauplan für `asset/` (Steglage, Dämpfung) wurde Abb. 6 der Quelle erstmals **als Bild** gelesen: die gemessene
Übertragungskurve (Beschleunigung je Kraft, 0–400 Hz, Antwortpunkt 3 nahe dem Steg, drei Hammerspitzen). Bisher
stützte sich alles auf Tab. 3 (142 / 226 / 242 / 280 Hz). Die blaue Kurve ist aus dem Bild digitalisiert
(`proben/2026-11-13_S196_oud_abb6_kurve.txt`, Seite 8 bei 300 dpi; Achsen am Rahmen gefunden).

**Spitzen der Messung:** **101 / 139 / 197 · 203 · 210 / 280 / 362 / 385 Hz**, dazu eine Schulter bei 222–226 Hz. Die
stärkste Spitze ist die Gruppe um 203 Hz. **Bei 242 Hz liegt ein Tal** (die Kurve fällt dort auf ihren kleinsten Wert
zwischen 100 und 400 Hz). Unter 25 Hz zwei Spitzen der Aufhängung — das Instrument war frei gelagert.

Gegen Tab. 3: 142 ≈ die Spitze bei 139, 226 ≈ die Schulter, 280 = die Spitze bei 280; 242 Hz hat an diesem
Antwortpunkt keine Spitze; die Spitzen bei 101, 203 und 362 Hz nennt die Tabelle nicht. Die Tabelle ordnet die
Messung den vier Moden der FE-Rechnung der Quelle zu (Schale und Hals fest, ohne Luftraum) — sie ist keine Liste der
gemessenen Spitzen.

**Gerechnet** am selben Punkt (`proben/2026-11-13_S196_oud_frf_gegen_abb6.py`: aus den gekoppelten Formen die
Beschleunigung je Kraft am Antwortpunkt, Anregung auf dem 5-cm-Raster aus Abb. 5 b, η einheitlich 0,02; Lochradien
nach den Maßzahlen, Luftraum 20,6 l). Verglichen werden die Lagen der Spitzen — welche Anregung Abb. 6 zeigt, sagt die
Quelle nicht:

| Fassung | gekoppelte Moden unter 400 Hz | 101 | 139 | 203 | 280 | 362 | 385 |
|---|---|---|---|---|---|---|---|
| Balken wie in Abb. 3 gezeichnet, 1,3 mm | 95 / 155 / 205 / 247 / 248 / 251 / 319 / 328 / 371 / 391 | −6 % | +11 % | +1 % | −10 % | +2 % | +2 % |
| 1,4 mm | 98 / 162 / 208 / 259 / 263 / 266 / 336 / 346 / 394 | −3 % | +17 % | +2 % | −5 % | −7 % | +2 % |
| **1,5 mm** | **100 / 169 / 209 / 270 / 276 / 280 / 352 / 363** | **0 %** | +21 % | **+3 %** | **−1 %** | **0 %** | −6 % |
| 1,6 mm | 103 / 176 / 209 / 282 / 288 / 295 / 367 / 380 | +2 % | +26 % | +3 % | +3 % | +1 % | −1 % |
| Balken am Rand, 1,5 mm (KQ9 a) | 132 / 243 / 305 / 324 / 378 | +31 % | −5 % | +20 % | −13 % | +5 % | −2 % |

(je gemessene Spitze die nächste Spitze der gerechneten Kurve; Bild
`proben/2026-11-13_S196_oud_frf_gegen_abb6.png`.)

**Lesart:**

- **Mit den Balken, wie Abb. 3 sie zeichnet (1,3–1,5 cm vor dem Rand endend), und 1,5–1,6 mm Decke trifft die Rechnung
  vier bis fünf der sechs Spitzen innerhalb von 3 %:** 101 Hz ist die Mode der Luft (Luftanteil 0,65), die Gruppe um
  203 Hz die von ihr verschobene erste Deckenmode (209 Hz, Luftanteil 0,29), dazu 280 und 362 Hz. Das ist das Bild
  der Gitarre (Luftmode um 100 Hz, erste Deckenmode um 200 Hz) und passt zu „around 120 Hz and 180–220 Hz“ bei
  Değirmenli (zehn Ouds; nur Zusammenfassung gelesen).
- **Die Fassung nach KQ9 a trifft nur 139 und 385 Hz**; sie hat keine Mode bei 101 Hz und liegt an der stärksten
  Spitze 20 % zu hoch. Die Lesart aus §7 („142 Hz ist die Mode der Luft“) hält gegen die Kurve nicht.
- **Die Spitze bei 139 Hz hat in der Fassung „wie gezeichnet“ kein Gegenstück**; die Rechnung hat stattdessen eine
  Mode bei 169 Hz (links und rechts gegengleich, Luftanteil 0,004), die in der Messung nur als kleine Welle zwischen
  150 und 180 Hz erscheinen könnte. Was 139 Hz ist, sagt die Quelle nicht — möglich ist eine Mode des ganzen
  Instruments (Hals gegen Schale): es war frei gelagert, die Rechnung hält Schale und Rand fest.
- Die Annahme „Balkenenden am Rand“ (§7, unbelegt) entfällt damit; „wie gezeichnet“ ist aus Abb. 3 gelesen.

**Grenzen:** die Höhen der Spitzen stimmen nicht (gerechnet ist 276 Hz die stärkste, gemessen 203 Hz) — Anregungsort
und Dämpfung sind nicht belegt; die Kurve ist aus einem Bild gelesen (±1 Hz); eine Kurve an einem Antwortpunkt.
Die Lage der kleinen Löcher ist in Abb. 3 (±10,3 cm) und Abb. 5 b (rund ±7 cm) verschieden gezeichnet; gerechnet ist
Abb. 3.

**Gegenproben zur Spitze bei 139 Hz** (wie gezeichnet, 1,5 mm; gekoppelte Moden in Hz):

| Fassung | Moden | 101 | 139 | 203 | 280 | 362 |
|---|---|---|---|---|---|---|
| wie gezeichnet | 100 / 169 / 209 / 270 / 276 / 280 / 352 / 363 | 0 % | +21 % | +3 % | −1 % | 0 % |
| kleine Löcher bei ±7 cm (Abb. 5 b) | 101 / 169 / 209 / 268 / 276 / 280 / 350 / 367 | 0 % | +22 % | +3 % | −1 % | +1 % |
| Steg 14 cm × 10 × 6 mm (angenommen, Fichte) | 99 / 164 / 208 / 257 / 271 / 280 / 348 | −1 % | +18 % | +3 % | −3 % | −4 % |
| Steg 17 cm × 15 × 10 mm (angenommen) | 97 / 156 / 208 / 244 / 254 / 280 / 344 | −3 % | +12 % | +2 % | −9 % | −5 % |

Die Lage der kleinen Löcher ändert nichts. Ein Steg zieht die gegengleiche Mode von 169 auf 156–164 Hz, erreicht 139 Hz
aber nicht und verschlechtert 280 und 362 Hz; seine Maße sind nicht belegt (Länge aus Abb. 3 und 5 b 14–17 cm).
Die Mode der Luft und die erste Deckenmode bleiben in allen vier Fassungen bei 97–101 und 208–209 Hz.
**Die Spitze bei 139 Hz erklärt keine der Gegenproben.**

## 9 Der Bauplan unter `asset/` (KQ10 a)

`asset/loop/instrument_bodies/make_oud.py` schreibt `oud.lvinstbodyplan`; `InstrumentBodyTool … --to 5000` den
Körper `oud.lvinstbody` (Netz für 5 kHz: 5 629 Knoten, 243 Moden, 1,5 min; statistischer Teil bis 20 kHz).

| Größe | Wert | Herkunft |
|---|---|---|
| Umriss, Balken (Lage, Enden), Löcher, Schalentiefe, Steglage | wie §1, §7, §8 | Abb. 3 (gelesen; Maßzahlen R 5,3 / R 2,0 / 21 / 41 cm) |
| Holz, Balkenmaße | Tab. 1, Tab. 2 | Quelle |
| Deckendicke | 1,5 mm | **Annahme** (KQ10 a) |
| Hohlraum | 20,61 l; Mündung 1,7 Radien, Güte 30 | Schalentiefe belegt; Querschnitt, Mündung, Güte **Annahme** |
| Stegpunkte | je Chor einer, Tasten 36 / 41 / 45 / 50 / 55 / 60 (C2 F2 A2 D3 G3 C4), y = 9 cm, x = −2,75 … +2,75 cm | Stimmung D'Addario EJ95A (KQ5 a); Abstand der Chöre 11 mm **Annahme** |
| Dämpfung | α = 0,06 f + 23 1/s (η 4,6 % bei 100 Hz, 2,8 % bei 200 Hz, 1,3 % bei 1 kHz) | Halbwertsbreiten der frei stehenden Spitzen in Abb. 6: η 3,4 / 3,3 / 1,6 / 1,5 % bei 139 / 280 / 362 / 385 Hz (`proben/2026-11-13_S196_oud_abb6_daempfung.py`), kleinste Quadrate; über 400 Hz fortgeschrieben |
| Hörort | 0,5 m vor der Decke | **Annahme** |

**Ergebnis** (gekoppelte Moden, in Klammern der Anteil der Luft): 99,9 (0,65) / 168,6 (0,00) / 207,8 (0,29) /
269,4 / 274,0 / 279,5 / 351,3 / 361,6 Hz. Gegen die gemessenen Spitzen 101 / 203 / 280 / 362 Hz: **−1 / +2 / −2 /
0 %** (Spitzen der gerechneten Kurve am Antwortpunkt: 100 / 208 / 274 / 362 Hz). Der Test „Loop Koerper 34k-5
Luftraum die mitgelieferte Oud (KQ10 a)“ hält das fest (3 %).

**Offen:** die Spitze bei 139 Hz ohne Gegenstück; die Höhen der Spitzen (Anregungsort der Messung unbekannt);
kein Steg; die Dämpfung über 400 Hz; je Chor ein Stegpunkt — die zwei Saiten eines Chors liegen rund 3 mm
auseinander und teilen ihn.
