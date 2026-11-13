# 34k-5: die Harfe — was aus Bell 1988 gelesen ist (Stand vor dem Bauplan)

> **Sitzung:** S197 (2026-11-13). Quelle (außerhalb des Repos unter `…\cmake\quellen\gezupfte-saite\`):
> A. J. Bell, „An Acoustical Investigation of the Concert Harp“, PhD, University of St Andrews 1988 (Salvi
> „Orchestra“; KQ1 a). Seiten als Bild gelesen (PDF-Seite in Klammern); die Textebene der Datei ist OCR mit
> Lesefehlern — **Zahlen nur vom Bild.** Noch kein Bauplan, nichts gerechnet. Offen: KQ17.

## 1 Wo was steht

| Inhalt | gedruckte Seite | PDF-Seite |
|---|---|---|
| Kap. 2 Beschreibung, Text | 31–39 | 51–59 |
| Abb. 2.4 (a)–(c) Boden, Deckleiste, zwei Leisten | – | 62 |
| Abb. 2.4 (d) Zugleiste (Buche) | – | 63 |
| Abb. 2.7 Kasten, Abb. 2.8 Öffnungen, Abb. 2.9 Lage der Saiten | – | 65, 66, 67 |
| Tab. 2.1 / 2.2 Saiten | – | 69–70 |
| Kap. 3 freier Boden (Tabellen 3.1–3.5) | 40–67 | 71–125 |
| Kap. 4 Luft im Kasten; Tab. 4.1 (Öffnungen), Tab. 4.2 (Helmholtz je Fall) | 68–94 | 126–170; 153, 154 |
| Kap. 5 Kasten: Chladni und Hologramme (Kasten A) | 95–122 | 171–233 |
| Kap. 6 Eingangsadmittanz Kasten F; **Tab. 6.1** | 123–143 | 234–269; 255 |
| Kap. 7 fertige Harfe mit Saiten (Kasten G), Admittanz 50 Hz–10 kHz, Werte an den Saitenorten | 144–160 | 270–… |
| Anhang 1 Fichte, Furnier; Anhang 3 Leisten ändern | 211–233 | – |

## 2 Maße (Abb. 2.4, 2.8; am Bild gelesen)

- **Boden** (Fichte): 131 cm lang, 41 cm (Bass) → 10 cm (Diskant) breit, 10 mm → 3 mm dick; **Faser quer zum Boden**;
  vorn ein Fichtenfurnier 1 mm, Faser längs.
- **Deckleiste** (Buche, außen, Mitte): 131 cm; unten 33 mm (oben an der Leiste 28 mm) breit und 5 mm hoch, am
  Diskant 18 mm (16,5 mm) breit und 3 mm hoch; Faser längs.
- **Zwei Leisten** (Fichte, „harmonic bars“, innen): 105 cm; unten 11,5 mm (7 mm) breit und 15 mm hoch, oben 5 mm
  (3 mm) breit und 7 mm hoch; sie halbieren etwa den Abstand von der Mitte zum Rand (Text S. 35).
- **Zugleiste** (Buche, innen, Mitte; Abb. 2.4 d mit eigenem Maßstab): Breite erst schmaler, dann etwas breiter, dann
  zum Diskant dünner; Dicke 2 cm → 2,5 cm, dann dünner (Text S. 35). **Maße aus der Zeichnung noch nicht
  abgegriffen.**
- **Rand:** der Boden liegt zwischen Halteleiste und Randleiste des Kastens, mit Epoxid verklebt; am Bass eine
  Schraube 7 cm durch Deck- und Zugleiste in den Bodenblock.
- **Kasten:** Schale aus Fichtenlaminat 3 mm, halbkonisch; Blöcke oben und unten, zwei Randleisten, zwei
  Innenleisten, vier Streben; um die Öffnungen aufgedoppelt, „the depth of the apertures is approximately 1cm“.
  **Maße des Kastens (Radien, Tiefe) noch nicht abgegriffen**; das Luftvolumen steht in Tab. 4.1.
- **Sechs Öffnungen in der Rückseite der Schale** (nicht im Boden), Abb. 2.8, A × B in cm: 16,5 × 6,0 / 18,0 × 5,5 /
  18,4 × 4,8 / 18,4 × 4,1 / 17,2 × 3,5 (Nr. 1–5, vom Bass zum Diskant, fast rechteckig mit runden Enden) und
  11,5 × 6,5 (Nr. 6, oval, im Bodenblock).
- **46 Saiten**, sie laufen von Ösen in der Mitte der Deckleiste; Lage je Saite Abb. 2.9 (noch nicht abgegriffen).
  Gesamtzug 9350 N.

## 3 Luft (Tab. 4.1, PDF 153; am Bild)

| Öffnung | Fläche (10⁻⁴ m²) | Lippenlänge (10⁻² m) |
|---|---|---|
| 1 | 91 | 1,6 |
| 2 | 93 | 1,5 |
| 3 | 77 | 1,5 |
| 4 | 68 | 1,3 |
| 5 | 58 | 1,2 |
| 6 | 67 | 2,1 |

„The total volume of the enclosed air in the soundbox is (2.7 ± 0.2) × 10⁻² m³.“ Mit allen Öffnungen offen und
starren Wänden liegt die Helmholtz-Mode bei **190 Hz** (S. 85, Textebene — am Bild nachlesen). Höhere Luftmoden
A1–A5 längs des Kastens (Kap. 4.3; Bildunterschriften, Öffnung 6 offen: A2 283 Hz, A3 414 Hz, A4 545 Hz, A5 680 Hz —
Textebene, am Bild nachlesen).

Überschlag mit der Formel von `InstrumentBody` (je Öffnung Leitwert S / (Lippe + 1,7 r), Volumen 27 l): 225 Hz,
18 % über den gemessenen 190 Hz — die Öffnungen liegen dicht nebeneinander, Bell rechnet mit einer Ersatz-Ellipse.

## 4 Gemessene Moden des Kastens (Tab. 6.1, PDF 255; am Bild) — Kasten F, Eingangsadmittanz

| Hz | Güte | Deutung der Quelle |
|---|---|---|
| 186 | 18 | S(1,1)− |
| 204 | 14 | S(1,1)+ |
| 250 | 17 | S(1,2) |
| 289 | 23 | Kopplung S(1,2) mit Luftmode A1 |
| 332 | 30 | S(1,3) |
| 421 | 28 | S(1,4) |
| 468 | 23 | S(2,1) |
| 535 | 40 | S(1,5) |
| 550 | – | Kopplung S(1,5) und S(2,2) |
| 637 | 25 | Wirkung der Luftmode A4 |
| 660 | 44 | S(1,6) |
| 720 | – | Kopplung S(1,6) mit Luftmode A5 |
| 740 | 37 | Kopplung S(1,6) und S(2,3) |
| 775 | 44 | S(1,7) |
| 850 | – | Kopplung S(1,7) und S(2,4) |
| 900 | 30 | nicht bekannt |
| 970 | 38 | S(1,8) |

S(1,1)− und S(1,1)+ sind die gekoppelten Moden aus Helmholtz-Luft und erster Bodenmode (Kap. 5.5.1). An einem
anderen Kasten (A, Hologramme, Kap. 5.4; Bildunterschriften der Textebene): 176 / 201 / 250 / 364 / 485 / 604 /
797 Hz für S(1,1)− bis S(1,6), S(2,1) 490, S(2,2) 583, S(2,3) 748 Hz — die Kästen streuen. In allen Moden bewegt
sich auch die Rückseite des Kastens (S. 121). Kap. 7: an der fertigen Harfe mit Saiten ändern sich die Moden; zwei
Moden des ganzen Instruments (B1, B2) — **noch nicht gelesen.**

## 5 Was `InstrumentBody` heute kann und was nicht (am Code gelesen)

- Dicke als Raster: ja (`Bauplan.hpp`, `Platte::dickeRaster`, `plate.thicknessGrid`).
- Leisten beiderseits der Platte: als Leisten mit Höhe über der Platte (Rippen, Stege).
- **Öffnungen des Hohlraums nur als Löcher der Platte:** `luftraumGroessen` (`Luftraum.hpp` Z. 216–237) nimmt die
  Luft allein aus `plan.platte.loecher`; `Hohlraum` (`Bauplan.hpp` Z. 229–245) trägt Volumen, Mündungskorrektur und
  Güte, keine eigenen Öffnungen. Die Öffnungen der Harfe liegen in der Schale.
- **Nur die Helmholtz-Mode:** höhere Luftmoden im Kasten rechnet `Luftraum.hpp` nicht (Grenze aus S196). Bei der
  Harfe koppeln A1, A4, A5 nach Tab. 6.1 mit Bodenmoden (289, 637, 720 Hz).
- Das Gehäuse ist starr, der Rand eingespannt.

**Seit KQ17 a (S197):** Öffnungen außerhalb der Platte gibt es (`cavity.openings`, Bauplan Fassung 5).

## 6 Weiter gelesen (Bild)

- **Tab. 3.1 (PDF 122), freier Boden ohne Leisten, (0,n)-Moden, Mittel dreier Böden:** n = 6 … 12: 75 / 105 / 139 /
  174 / 216 / 258 / 313 Hz; Massen 1,513 / 1,333 / 1,278 kg. **Tab. 3.2:** (2,0) 277, (2,2) 345, (2,3) 394,
  (2,4) 440 Hz; (2,1) nicht gefunden.
- **Tab. 3.3 (PDF 123), freier Boden mit Deck- und Zugleiste, (1,n), n = 1 … 12:** Boden A 65 / 102 / 135 / 176 /
  210 / 245 / 277 / 310 / 341 / 380 / 422 / 445; Boden B 67 / 110 / 147 / 190 / 225 / 265 / 302 / 342 / 382 / 417 /
  460 / 520; Boden C 65 / 107 / 157 / 212 / 267 / 327 Hz — die Böden streuen bis 30 %. **Tab. 3.4:** (2,n) mit
  Mittelleisten, ohne / mit den zwei Fichtenleisten: (2,2) 283 / 276, (2,3) 396 / 350, (2,4) 486 / –, (2,5) 635 / 658,
  (2,6) 714 / 777, (2,7) 825 / 901 Hz.
- **Zugleiste (Abb. 2.4 d, PDF 63; mit den Maßstäben der Zeichnung abgegriffen, ±1 mm Höhe, ±2 mm Breite):** vom
  Bassende 0 / 29 / 55 / 80 / 105 / 128 cm: Höhe 20 / 25 / 19 / 13 / 7,2 / 5,6 mm, Breite 32 / 36 / 39 / 34 / 27 /
  20 mm.
- **Saitenorte (Abb. 2.9, PDF 67 bei 200 dpi; `proben/…_harfe_saitenorte.py`):** 45 Querstriche automatisch gefunden,
  der für Saite 00 von Hand (y ≈ 340 px); Boden von y 252 (Diskant) bis 2174 px (Bass) = 131 cm. Saite 44 liegt
  6,4 cm, Saite 00 rund 125 cm vom Bassende. Der gezeichnete Maßstab (10 cm = 140 px) gäbe 137 cm für den Boden —
  genommen ist die Länge 131 cm aus Abb. 2.4 a.
- **Lage der Öffnungen längs (Abb. 2.8, grob):** Nr. 1–5 bei 19 / 36 / 52 / 69 / 87 % der Länge vom Bassende, Nr. 6
  bei 3 %.

## 7 Proben (S197; `proben/2026-11-13_S197_harfe_bauplan.py`, `…_moden.py`)

Das Werkzeug hat dafür zwei Probe-Schalter: `--free-rim` (Rand frei) und `--supported-rim` (nur w gehalten, beide
Drehungen frei; `Auflager::Gestuetzt`). Ein so geschriebener Körper ist keiner für das Programm.

**7.1 Freier Boden ohne Leisten — Eichung des Holzes.** Platte 131 cm, 41 → 10 cm, Dicke 10 → 3 mm plus 1 mm
(Furnier), Faser quer. Mit 500 kg/m³ wiegt sie 1,371 kg (gemessen 1,28–1,51). Zuordnung der Formen über die
Vorzeichenwechsel quer (m) und längs (n).

| Holz | (0,6) … (0,12) gegen Tab. 3.1 | (2,n) gegen Tab. 3.2 |
|---|---|---|
| E quer zum Boden 11, längs 0,65, G 0,66 GPa (Fichte nach Chabassier) | −13 / −14 / −14 / −12 / −12 / −11 / −12 % | zu hoch (341 gegen 277 Hz) |
| **7,3 / 0,855 / 0,45 GPa** | −1 / −2 / −2 / 0 / 0 / +1 / 0 % | 279 / 346 / 400 / 451 gegen 277 / 345 / 394 / 440: +1 / 0 / +2 / +3 % |

Elf gemessene Moden innerhalb von 3 %. Das Furnier (Faser längs) hebt die Steife längs nur auf 0,855 GPa — viel
weniger als ein Überschlag über die Schichten erwarten ließ.

**7.2 Freier Boden mit Leisten.** Mit Deck- und Zugleiste (Buche nach Chabassier): (1,n) gerechnet 76 / 110 / 146 /
186 / 227 / 272 / 319 / 368 / 419 / 473 / 530 Hz (n ab 2), gegen Boden B 67 / 110 / 147 / 190 / 225 / 265 / 302 /
342 / 382 / 417 / 460: +13 / 0 / −1 / −2 / +1 / +3 / +6 / +8 / +10 / +13 / +15 % (die Zählung n ist um eins
versetzt angenommen — nicht belegt). (2,n) mit Mittelleisten: 262 / 343 / 426 / 523 / 626 gegen 283 / 396 / 486 /
635 / 714: −7 / −13 / −12 / −18 / −12 %.

**7.3 Kasten: Rand eingespannt gegen Rand gestützt.** Alle Leisten, Hohlraum 27 l mit den sechs Öffnungen;
Mündungskorrektur 2,6 Radien (damit Helmholtz bei starrer Wand 185 Hz gegen gemessene 190; mit 1,7 sind es 223 Hz).
Moden der Platte im Vakuum gegen Tab. 6.1:

| Mode (Bell) | gemessen | eingespannt | gestützt |
|---|---|---|---|
| S(1,1) (ungekoppelt; gekoppelt gemessen 186 / 204) | – | 291 | 142 |
| S(1,2) | 250 | 368 (+47 %) | 200 (−20 %) |
| S(1,3) | 332 | 462 (+39 %) | 270 (−19 %) |
| S(1,4) | 421 | 563 (+34 %) | 347 (−18 %) |
| S(1,5) | 535 | 669 (+25 %) | 430 (−20 %) |
| S(1,6) | 660 | 781 (+18 %) | 529 (−20 %) |
| S(1,7) | 775 | 902 (+16 %) | 650 (−16 %) |
| S(1,8) | 970 | – | 790 (−19 %) |
| S(2,1) | 468 | 778 (+66 %) | 523 (+12 %) |

Gekoppelt mit der Luft: eingespannt 180 / 298 Hz (gemessen 186 / 204); gestützt 129 / 192 / 208 Hz (drei Moden
teilen sich die Luft). **Die Messung liegt zwischen beiden Rändern.** Mit gestütztem Rand liegt die Familie S(1,n)
gleichmäßig 16–20 % zu tief, S(2,1) 12 % zu hoch. Der Rand des Bodens gibt also nach (Bell: „In all of the modes,
the back of the soundbox is also in motion“; die Schale ist 3 mm Laminat) — weder eingespannt noch gestützt bildet
ihn ab. Entscheid KQ18.
