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
