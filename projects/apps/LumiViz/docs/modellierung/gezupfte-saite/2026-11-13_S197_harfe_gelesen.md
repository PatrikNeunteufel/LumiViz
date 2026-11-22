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

## 8 Kap. 7 gelesen (S198; Bild, PDF 270–303, gedruckt S. 144–160 a und Abb. 7.1–7.16)

Bell misst **einen zweiten Kasten G** in drei Zuständen: lose, in der fertigen Harfe ohne Saiten, in der fertigen Harfe
mit Saiten auf Stimmung (Saiten mit Kunststoffband stark gedämpft; „fertig“ heißt das Holz ohne Mechanik und Lack).
Eingangsadmittanz an denselben 40 Messorten längs der Deckleiste, Bezug 116 dB = 1 s/kg; Schalldruck in 1,5 m.

| Mode | G lose | G in der Harfe, ohne Saiten | G in der Harfe, besaitet | zum Vergleich F lose (Tab. 6.1) |
|---|---|---|---|---|
| B1 (ganze Harfe) | – | 85 | 92 | – |
| S(1,1)− | 153 (Q 23) | 170 (Q 11) | 187 (Q unbestimmbar) | 186 |
| S(1,1)+ | 200 | 216 (Q 16) | 207 | 204 |
| S(1,2) | 221 (Q 13) | 249 (Q 39) | 257 (Q 32) | 250 |
| S(1,2)′ | 251 (Q 36) | 249 (eine Mode) | 284 (Q 30) | 289 |
| S(1,3) | 320 (Q 36) | 316 (Q 45) | 334 (Q 27) | 332 |
| S(1,4) | 420 (Q 38) | 432 (Q 43) | 430 (Q 28) | 421 |
| S(1,5) | 557 (Q 32) | 545 (Q 31) | 555 (Q 19) | 535 |
| S(1,6) | 702 (Q 28) | 709 (Q 10) | 697 (Q 12) | 660 |
| S(1,7) | 768 (Q 44) | 757 (Q 33) | 753 (Q 17) | 775 |
| S(1,8) | 912 (Q 26) | 905 (Q 18) | 895 (Q 18) | 970 |

(Frequenzen und Güten aus den Bildunterschriften der Abb. 7.8–7.16; dass 289 Hz bei F die S(1,2)′ von Kap. 7 ist,
ist meine Zuordnung — Tab. 6.1 nennt sie „Kopplung S(1,2) mit Luftmode A1“. Ein dritter Kasten A steht in §4.)

Was das für das Ziel heißt:

- **Zwei Kästen desselben Modells liegen bis 6 % auseinander** (S(1,6): 702 gegen 660; S(1,8): 912 gegen 970),
  S(1,1)− sogar 18 % (153 gegen 186). Eine Schwelle unter rund 6 % an der Lage wäre enger als die Streuung des Baus.
- **Ab S(1,3) ändert der Einbau und das Besaiten die Lage um höchstens 2–4 %** (Bell: „maximum of 2 %“; aus den
  Bildunterschriften bis 4 % bei S(1,3)). Der Rand der höheren Moden hängt also nicht an der Bewegung des ganzen
  Kastens.
- **Darunter (S(1,1)− bis S(1,2)′) bewegt sich der ganze Kasten mit**; in der Harfe ist das verhindert (S. 155 f.).
  S(1,1)− steigt lose → eingebaut → besaitet 153 → 170 → 187 Hz, S(1,2) 221 → 249 → 257 Hz.
- **Die Saiten** heben S(1,1)− um 10 %, senken die Admittanz an den Bäuchen um bis zu 7 dB und halbieren etwa die
  Güten der höheren Moden (44 → 17, 32 → 19). Das ist dieselbe Last wie bei der Laute (KQ11 a).
- Die Höhen der Bäuche stehen in den Kurven (besaitet etwa 90–103 dB, also −26 … −13 dB re 1 m/s/N) — noch nicht
  abgegriffen.
- Zwei Moden der ganzen Harfe (B1 85 / 92 Hz, B2) liegen unter dem Kasten; sie gehören nicht zum Boden.
- Anregung im Saitenwinkel (30°) statt senkrecht: Unterschied selten über ±3 dB (Abb. 7.19 / 7.20).
- 50 Hz–10 kHz (Abb. 7.21, noch nicht gelesen): Plateau der Maxima 1–1,5 kHz, darüber fällt die Admittanz wie
  eine Masse.
- **Die Saiten sitzen auf der Knotenlinie aller S(2,n)** (Ösen in der Mitte der Deckleiste; Bell S. 157: „the
  central bar of the soundbox is a node for such vibrations“). Für den Ton zählt die Familie S(1,n).

**Wie der Rand gebaut ist (Abb. 2.6, PDF 64, am Bild):** der Rand des Bodens steckt zwischen einer Halteleiste außen
(„holding bar“, greift über die Kante) und der Randleiste innen („rim bar“), beide auf der Schale; vier Streben
stützen die Randleisten gegen die Innenleisten (Abb. 2.7). Der Rand ist also in w gehalten und gegen Drehen
**teilweise** gehalten — eine Drehfeder. Die Skizze hat keinen Maßstab.

## 9 Probe: der Rand als Feder (S198; `proben/2026-11-13_S198_harfe_rand_feder.py`)

> **Überholt (S198, §11):** die Zuordnung der gerechneten Formen zu Bells S(1,n) in diesem Abschnitt war um eins versetzt. Die Federwerte 3000 / 4500 N treffen die Frequenzen, aber nicht die Lage der Bäuche; der gestützte Rand trifft beides.

Ohne Eingriff in `InstrumentBody`: die Moden des Bodens mit Leisten im Vakuum (geeichtes Holz aus §7.1, Werkzeug mit
`--shapes`, bis 8 kHz) als Ansatz, die Federn am Rand im Modenraum dazu. Zwei Basen: Rand gestützt (300 Moden; nur
Drehfeder k_d in N, das ist N m/rad je m Rand) und Rand frei (424 Moden plus drei Starrkörperformen; Wegfeder k_w in
N/m² und Drehfeder). Zwei Gesetze: k_d gleich über die Länge, oder k_d(y) = k_d · b(0) / b(y) (wächst zum Diskant
wie 1 / Breite).

**Gegenproben.** k_d = 0 gibt die gestützten Werte zurück (200 / 270 / 346 / 430 / 529 / 650 / 790). k_d → ∞ gibt
411 / 525 / 644 / 768 / 897 / 1041 Hz gegen eingespannt gerechnete 368 / 462 / 562 / 669 / 781 / 901 — 12–15 % zu
hoch: bei harter Feder taugt der abgeschnittene Ansatz nicht. Im Bereich der Lösung ändert die Größe der Basis wenig
(75 → 150 → 300 Moden: S(1,2) 246 → 245 → 244, S(1,7) 811 → 790 → 788 Hz). Die freie Basis trifft die gestützten
Werte bei k_w = 10⁸ N/m² auf etwa 4 %, bei härterer Feder nicht mehr — **die Zahlen mit Wegfeder sind nur grob**.
Die Werte von k_d sind eher zu klein (der Ansatz liegt immer über dem wahren Wert); nach dem Einbau neu suchen.

**Ergebnis** (Abweichung der Platte im Vakuum, S(1,2) … S(1,8), dann S(2,1); „–“ = kein Ziel):

| Rand | F lose (Tab. 6.1) | G in der Harfe, besaitet | S(2,1) (F) |
|---|---|---|---|
| eingespannt (heute) | +47 / +39 / +34 / +25 / +18 / +16 / – | +43 / +38 / +31 / +21 / +12 / +20 / – | +66 |
| gestützt | −20 / −19 / −18 / −20 / −20 / −16 / −19 | −22 / −19 / −19 / −22 / −24 / −14 / −12 | +12 |
| Drehfeder, gleich, 2500 N (F) bzw. 3160 N (G) | −2 / 0 / +1 / −2 / −4 / −3 / (+6) | −2 / +2 / +1 / −3 / −7 / +2 / 0 | +20 |
| **Drehfeder ∝ 1 / Breite, 1585 N am Bass** | **−2 / +1 / +4 / +2 / 0 / +2 / −5** | **−5 / +1 / +2 / −2 / −5 / +5 / +3** | +17 |
| Weg- und Drehfeder (10⁷ N/m², 3160 N; grob) | +6 / +2 / +2 / −7 / −11 / −11 / (+7) | – | +7 |
| gestützt, E quer 11,0 statt 7,3 GPa („stellen“) | −11 / −11 / −10 / −13 / −14 / −10 / −13 | – | +31 |

(Werte in Klammern: die Zuordnung der Form ist dort unsicher.)

- **Eine Drehfeder bei gehaltenem w bringt die ganze Familie S(1,n) auf ±5 %** — an beiden Kästen, lose wie
  besaitet, mit einem Wert. Das ist innerhalb der Streuung zweier Kästen (6 %, §8). Zwischen den beiden Gesetzen
  entscheidet die Probe nicht (Mittel ∝ 1 / Breite gegen gleich: 4,3 gegen 4,7 % an F, 3,0 gegen 2,4 % an G
  besaitet).
- **S(2,1) bleibt 17–20 % zu hoch.** Eine Wegfeder senkt sie (+7 %), verdirbt aber S(1,5)–S(1,7) (−11 %) — und am
  freien Boden mit Leisten lag (2,n) 7–18 % zu **tief** (§7.2). Das ist kein Befund über den Rand; offen. Für den
  Ton ohne Belang, solange die Stegpunkte auf der Mittellinie liegen (§8).
- **Holz oder Leisten stellen trägt nicht:** mit E quer × 1,5 bleibt S(1,n) 10–14 % zu tief, S(2,1) steigt auf
  +31 %, und die Eichung am freien Boden geht verloren ((2,n) aus Tab. 3.2 dann +22 / +35 % statt +1 / 0 %).
- Größenordnung: k_d · b / D (D = Biegesteife der Platte quer) ist am Bass etwa 0,8–1,6, am Diskant 6–17 — der
  Rand ist am dünnen Diskant fast eingespannt, am dicken Bass fast gestützt. Das passt zur Bauweise (Abb. 2.6).
- Nicht gerechnet: die gekoppelten S(1,1)∓ mit der Luft unter der Feder, die Höhen der Admittanz, die Güten.

## 10 KQ18 a: die Drehfeder im Element (S198; `proben/2026-11-13_S198_harfe_rand_werkzeug.py`)

> **Überholt (S198, §11):** die Zuordnung der gerechneten Formen zu Bells S(1,n) in diesem Abschnitt war um eins versetzt. Die Federwerte 3000 / 4500 N treffen die Frequenzen, aber nicht die Lage der Bäuche; der gestützte Rand trifft beides.

Entscheid Patrik **KQ18 a**: Drehfeder am Rand, die Auslenkung bleibt gehalten; keine Wegfeder. `baueSystem` nimmt
die Feder als Randterm ∫ k_d φ_n² ds (`drehfederAmRand`, wirkt mit `Auflager::Gestuetzt`); geprüft gegen Lévy
(Rechteck, Federn an zwei Rändern, 1 %), gegen beide Grenzfälle und gegen ein gedrehtes Netz (Fall „KQ18 Drehfeder
am Rand“ in `test_LoopKoerperPlatte`). Das Werkzeug hat dafür die Probe-Schalter `--rim-spring <N>` und
`--rim-spring-end <N>` (1 / Feder linear in y; für den Harfenboden ist das Feder ∝ 1 / Breite).

Boden mit Leisten im Vakuum, geeichtes Holz; S(1,2) … S(1,7), dahinter S(2,1) gegen F:

| Feder (Bass → Diskant) | F lose (Tab. 6.1) | G besaitet | S(2,1) |
|---|---|---|---|
| 0 (gestützt) | −20 / −19 / −18 / −20 / −20 / −16 | −22 / −19 / −19 / −22 / −24 / −14 | +12 |
| 2000 N gleich | −6 / −4 / −3 / −6 / −8 / −7 | −8 / −5 / −5 / −9 / −13 / −4 | +18 |
| **3000 N gleich** | **−1 / 0 / +1 / −2 / −5 / −4** | −4 / −1 / −2 / −6 / −10 / −2 | +20 |
| **4500 N gleich** | +4 / +5 / +5 / +1 / −2 / −2 | **+1 / +4 / +3 / −3 / −7 / +1** | +23 |
| 7000 N gleich | +10 / +10 / +9 / +5 / +1 / +1 | +7 / +9 / +7 / +1 / −4 / +4 | +27 |
| 1000 → 4100 N | −8 / −5 / −3 / −4 / −6 / −5 | −10 / −5 / −5 / −8 / −11 / −2 | +15 |
| **1600 → 6560 N** | **−3 / 0 / +1 / −1 / −3 / −2** | −6 / −1 / −1 / −4 / −8 / +1 | +17 |
| **2500 → 10250 N** | +2 / +4 / +6 / +3 / 0 / 0 | **−1 / +4 / +3 / −1 / −5 / +3** | +20 |
| 4000 → 16400 N | +8 / +9 / +10 / +6 / +3 / +3 | +5 / +9 / +8 / +3 / −2 / +6 | +23 |
| 10⁹ N (hart) | +45 / +37 / +32 / +24 / +17 / +15 | – | +63 |

- Die Probe im Modenraum (§9) ist bestätigt; die Federwerte liegen im Element rund 20–40 % höher, wie erwartet.
- Die harte Feder trifft den eingespannten Rand auf 1 % (363 / 456 / 555 gegen 368 / 462 / 562 Hz).
- **Gleiche Feder gegen Feder ∝ 1 / Breite:** am selben Ziel größte Abweichung 5 gegen 3 % (F), 7 gegen 5 %
  (G besaitet) — beides innerhalb der Streuung zweier Kästen (6 %). Die Daten entscheiden die Form nicht. Für die
  gleiche Feder spricht die Bauweise (Halte- und Randleiste laufen über die ganze Länge, Abb. 2.6); für den Verlauf
  gibt es nur die etwas glattere Anpassung.
- S(1,8) ist in der Zuordnung der Formen unsicher (mehrere Formen mit sieben Wechseln längs) und oben weggelassen.

## 11 Gegenprobe an der Lage der Bäuche: der Rand ist gestützt (S198; `proben/2026-11-14_S198_harfe_admittanz.py`)

Für die Höhen der Admittanz (Ziel der Harfe) sind die Profile der Abb. 7.8–7.16 abgegriffen: je Mode die Admittanz
über die 40 Messorte längs der Deckleiste (Ort 40 am Bass, Ort 1 am Diskant; 3,5 cm Abstand, Bell S. 127; 116 dB =
1 s/kg). Der stärkste Bauch liegt bei jeder Mode am Diskant und wandert mit n dorthin:

| Bell, Kasten G in der Harfe | ohne Saiten: Hz, Q, Pegel, Ort | besaitet: Hz, Q, Pegel, Ort | weitere Bäuche (lose, Bild 1) |
|---|---|---|---|
| S(1,2) | 249, 39, 96 dB, Ort 12 | 257, 32, 92,5 dB, Ort 12 | 30 |
| S(1,3) | 316, 45, 104 dB, Ort 8 | 334, 27, 101 dB, Ort 8 | 21, 32 |
| S(1,4) | 432, 43, 105,5 dB, Ort 5 | 430, 28, 99,5 dB, Ort 4 | 14, 23, 32–35 |
| S(1,5) | 545, 31, 101,5 dB, Ort 3 | 555, 19, 103 dB, Ort 3 | 10, 18, 25, 35 |
| S(1,6) | 709, 10, 100,5 dB, Ort 3 | 697, 12, 103 dB, Ort 2 | 8, 14, 21, 26 |
| S(1,7) | 757, 33, 99,5 dB, Ort 3 | 753, 17, 101 dB, Ort 2 | 5, 8, 14, 20, 26 |
| S(1,8) | 905, 18, 102 dB, Ort 4 | 895, 18, 103,5 dB, Ort 3 | – |

(Am Bild gelesen, ±1–2 dB, ±1 Ort. Saiten: Tab. 2.1 / 2.2 am Bild; Saitenorte Abb. 2.9 von Hand,
`proben/2026-11-14_S198_harfe_saitenorte.json` — das Skript von S197 findet die Striche am neu gerasterten Bild nicht
mehr.)

**Gerechnet: wo liegt der stärkste Bauch, und bei welcher Frequenz?** (Boden mit Leisten, geeichtes Holz; Formen auf
der Mittellinie.)

| Rand | Formen mit stärkstem Bauch an Ort … bei … Hz |
|---|---|
| gemessen (G in der Harfe, ohne Saiten) | 12 → 249 · 8 → 316 · 5 → 432 · 3 → 545 · 3 → 709 |
| **gestützt (Feder 0)** | **12 → 270 · 8 → 346 · 5 → 430 · 4 → 529 · 3 → 650** |
| Drehfeder 2000 N | 20 → 236 · 15 → 318 · 11 → 407 · 8 → 504 · 5 → 606 · 4 → 720 |
| Drehfeder 4500 N | 21 → 261 · 15 → 347 · 11 → 441 · 8 → 541 · 6 → 647 · 4 → 762 |
| eingespannt | 21 → 363 · 16 → 456 · 12 → 555 · 9 → 661 · 7 → 772 |

- **Mit der Feder liegen die Bäuche 6–9 Messorte (20–30 cm) zu weit am Bass.** Die Form, die bei 4500 N auf 441 Hz
  liegt, hat ihren Bauch an Ort 11; gemessen ist bei 432 Hz Ort 5. Am gemessenen Ort liegt die Admittanz dann
  18–28 dB unter der Messung.
- **Mit gestütztem Rand stimmen Ort und Frequenz zugleich.** Die Form zu Bells S(1,n) hat in der Rechnung n + 1
  Bäuche; der letzte am Bassende (Ort 37–39, 7–13 dB schwächer) liegt bei Bell am Rand der Messorte und ist in den
  Profilen höchstens als Schulter zu sehen. Die Zuordnung in §7.3, §9 und §10 zählte die Bäuche und war deshalb um
  eins versetzt — daher „gestützt 16–20 % zu tief“.
- **Auch die Zahl der Moden stimmt nur so:** mit Luft (Helmholtz starr 185 Hz) rechnet der gestützte Kasten 129 / 192 /
  208 / 275 / 347 / 431 / 529 / 650 / 790 / 941 Hz; gemessen sind an G lose 153 / 200 / 221 / 251 / 320 / 420 / 557 /
  702 / 768 / 912 Hz — zehn Moden gegen zehn. Mit der Feder fehlt eine Mode unter 350 Hz (Bells S(1,2)′).

**Gestützter Kasten mit Luft gegen Bell** (η = 0,027 aus den Güten ohne Saiten; Saitenlast je Saite 0,25 · Z an ihrer
Öse, Z = Wellenwiderstand quer aus Tab. 2.1 / 2.2, Summe 62,6 N s/m — der Faktor ist eine Annahme für den Winkel):

| Mode | ohne Saiten: Lage, Ort, Höhe | besaitet (0,25 Z): Lage, Ort, Höhe, Q gerechnet / gemessen |
|---|---|---|
| S(1,2) | +10 %, 13 / 12, −0,3 dB | +7 %, 13 / 12, +2,5 dB, 30 / 32 |
| S(1,3) | +10 %, 8 / 8, −5,8 dB | +4 %, 8 / 8, −4,0 dB, 32 / 27 |
| S(1,4) | 0 %, 5 / 5, −6,2 dB | 0 %, 5 / 4, −1,2 dB, 33 / 28 |
| S(1,5) | −3 %, 4 / 3, −2,7 dB | −5 %, 4 / 3, −5,0 dB, 34 / 19 |
| S(1,6) | −8 %, 3 / 3, −3,8 dB | −7 %, 3 / 2, −6,9 dB, 35 / 12 |
| S(1,7) | +4 %, 3 / 3, −5,1 dB | +5 %, 3 / 2, −7,0 dB, 35 / 17 |
| S(1,8) | +4 %, 2 / 4, −10,1 dB | +5 %, 2 / 3, −11,9 dB, 36 / 18 |

Gegen F lose (Tab. 6.1: 250 / 332 / 421 / 535 / 660 / 775 / 970): +10 / +5 / +2 / −1 / −2 / +2 / −3 %.

- Lage innerhalb +10 / −8 %; drei Kästen desselben Modells streuen bis 14 % (S(1,5): 557 / 535 / 485 an G, F, A).
- **Die Höhen liegen 0–7 dB zu tief, zum Diskant hin mehr** (S(1,8) −10 … −12 dB). Ursache offen: die Rechnung endet
  bei 1100 Hz (die höheren Moden fehlen am dünnen Diskantende), Masse und Dicke am Diskant, die Dämpfung je Mode.
- Die Saitenlast senkt die Güten ab S(1,5) zu wenig (35 gegen 12–19) — 0,25 Z ist dort zu klein oder die Last ist
  nicht nur ein Widerstand quer.

**Gegenproben zu den Höhen.**

- Bells Bezug am Bild nachgelesen (S. 133): „0 dB = 1,58 × 10⁻⁶ s/kg … 116 dB = 1 s/kg“ — stimmt.
- Rechnung bis 5 kHz statt 1,1 kHz (144 Moden): die Höhen ändern sich um höchstens 0,1 dB. Die abgeschnittenen Moden sind nicht die Ursache.
- Mit der gemessenen Güte je Mode statt η = 0,027 (Höhe ∝ Q): S(1,2) 0, S(1,3) −4,1, S(1,4) −4,9, S(1,5) −4,2, S(1,7) −5,6 dB (S(1,6) mit Q 10 nicht vergleichbar — zwei Moden übereinander). Es bleibt ein gleichmäßiger Abstand von 4–6 dB ab S(1,3): die Rechnung ist am Bauch um den Faktor 1,6–1,9 zu schwer. Deck- und Zugleiste tragen dort rund die Hälfte der bewegten Masse; ganz ohne ihre Masse wären es etwa +3 dB. Ursache offen.
- Saitenlast 0,25 / 0,75 / 1 / 3 Z: die Güten ab S(1,5) erreichen die gemessenen (12–19) erst bei 3 Z, dann liegen die Höhen 8–12 dB zu tief. Gemessen **steigen** die Höhen von S(1,5)–S(1,8) mit Saiten um 1–3 dB, obwohl die Güten sich halbieren — das bildet ein Widerstand an den Ösen nicht ab. Für Höhe und Güte ist der Zustand ohne Saiten der saubere Vergleich.

**Probekörper gegen KQ19 a** (`make_harfe.py`, bis 5 kHz, Dämpfung α = 2,6·10⁻⁴ f² + 20,5): Lage S(1,2)–S(1,7) +10,4 / +9,8 / −0,3 / −2,9 / −8,3 / +4,4 % ohne Saiten, Ort überall unter 1 Messort, Höhe +1,6 / −4,6 / −5,5 / −3,1 / – / **−7,3 dB**, S(1,8) −10,6 dB. Zwei Ziele knapp verfehlt (S(1,2) Lage, S(1,7) Höhe).

**Ursache der Höhen gesucht — vier Proben.**

- **Masse der Leisten:** Deck- und Zugleiste tragen am Diskant rund die Hälfte der bewegten Masse; ganz ohne sie wären es höchstens +3 dB. Scheidet als alleinige Ursache aus.
- **Wegfeder am ganzen Rand** (Basis „Rand frei“, im Modenraum, 10¹² … 3·10⁶ N/m²): die Höhen am stärksten Bauch ändern sich um höchstens ±0,5 dB, die Frequenzen fallen um bis 20 %. Scheidet aus.
- **Schmalerer freier Boden** (Halteleiste überdeckt 1 / 1,5 cm je Seite; Werkzeug, bis 1,1 kHz): die Bäuche wandern 5–9 Messorte zum Bass, die Höhen werden schlechter (−7 … −9 dB). Scheidet aus — die volle Breite 41 → 10 cm schwingt.
- **Frequenzgang statt Spitzen** (Bell Abb. 6.6, PDF 260, Kasten F lose, am Bild): an Ort 5 liegen die Täler gemessen bei 71 / 72 / 66 dB (470 / 620 / 800 Hz), gerechnet 69 / 72 / 61 dB — Masse und Steife stimmen dort; an Ort 10 Spitze 332 Hz 100 dB gegen 98 dB. **An Ort 2 liegt die Messung überall 10–20 dB über der Rechnung, auch zwischen den Resonanzen** (300 Hz: 86 gegen 74 dB; 500 Hz: 94 gegen 84 dB; Spitze 665 Hz 108 gegen 94 dB). Bell S. 134: der höchste Wert des Kastens F ist 108 dB bei 660 Hz an Ort 3; die Profile Abb. 6.9 steigen zum Diskantende hin an.

**Messorte berichtigt (Abb. 6.3, PDF 258, am Bild):** die 40 Orte liegen im Abstand 3,5 cm, Ort 1 an der Diskantkante des Bodens, Ort 40 5,7 cm unter seinem Bassende (der Kasten ist 137 cm lang; die Saitenorte der Abb. 6.3 liegen 5,4–6,7 cm über denen aus Abb. 2.9): y = 0,035 · (40 − Ort) − 0,057 m. Gegen die bisherige Annahme (gleichmäßig über 131 cm) am Diskant höchstens 1,2 cm, am Bass 5 cm. Ort 2 liegt 3,7 cm vor der Diskantkante.

**Befund: die Diskantkante des Bodens ist nicht fest gehalten.** Probe im Modenraum (Seiten und Bassende gestützt, Wegfeder nur an der Diskantkante; Q fest 40):

| Diskantkante | Formen: Hz → stärkster Bauch, Höhe |
|---|---|
| gehalten | 439 → Ort 6, 101,6 dB · 552 → 4, 101,1 · 658 → 4, 97,1 · 805 → 3, 96,8 · 952 → 3, 93,0 |
| 10⁷ N/m² | 437 → 6, 101,0 · 547 → 4, 101,1 · 655 → 3, 97,9 · 796 → 3, 97,3 · 939 → 2, 93,6 |
| 10⁶ N/m² | 437 → 6, 101,0 · 542 → 4, 101,9 · 633 → 2, 102,5 · 747 → 1, 106,7 · 873 → 1, 104,5 |
| frei | 436 → 6, 101,0 · 533 → 1, 105,5 · 596 → 1, 112,0 · 707 → 1, 108,3 · 849 → 1, 102,7 |

Gemessen (G in der Harfe, ohne Saiten): 545 → Ort 3, 101,5 dB · 757 → 3, 99,5 · 905 → 4, 102. Eine weiche Diskantkante hebt genau die Moden ab S(1,5), die zu tief liegen (S(1,7), S(1,8)), um 5–10 dB und lässt S(1,2)–S(1,4) unberührt; ganz frei ist zu viel (Bauch an Ort 1 statt 3). Die Basis ist grob (Vakuum, älterer Bauplan, abgeschnitten) — die Zahlen zeigen die Richtung, kein Federwert. S(1,3) und S(1,4) bleiben 3–5 dB zu tief (innerhalb der Schwelle); dafür ist keine Ursache gefunden.
(Proben: `proben/2026-11-14_S198_harfe_wegfeder.py`, `…_harfe_diskantkante.py`, `…_harfe_frequenzgang.py`.)

**Folge für KQ18 a:** der Block `rim` bleibt nötig (die Vorgabe ist eingespannt), die Harfe bekommt aber
`support: elastic` mit `rotationalStiffness: 0` — gestützt. Die Federwerte aus §9 und §10 sind hinfällig.

## 12 KQ20 a: die Diskantkante als Abschnitt des Rands, am vollen Kasten gerechnet (S198)

KQ20 a (Patrik): Halt je Randstück. `rim.sections` (Fassung 7): Kanten des Umrisses mit eigener Wegfeder (N/m²) und
Drehfeder (N); im Abschnitt ist w nicht gehalten. Geprüft gegen die Lévy-Lösung einer Platte mit einer freien Kante
(1 %) und an den Grenzfällen (harte Feder = gestützt / eingespannt). Der Harfenkasten mit Luft, Diskantkante
(Kante 2 des Umrisses) mit Wegfeder, bis 1,1 kHz; 10¹² N/m² gibt den gestützten Kasten auf die letzte Stelle zurück.

**An den Ösen der Saiten** (wie KQ19 a; die oberste Öse liegt 6,4 cm vor der Kante):

| Wegfeder N/m² | S(1,5): Hz, Lage, Höhe | S(1,6): Hz, Lage | S(1,7): Hz, Lage, Höhe | S(1,8): Lage, Höhe |
|---|---|---|---|---|
| gehalten | 529, −2,9 %, −3,1 dB | 650, −8,3 % | 791, +4,4 %, −7,4 dB | −12 %, −10,5 dB |
| 10⁷ | 529, −3,0 %, −2,9 dB | 647, −8,7 % | 783, +3,5 %, −6,3 dB | −12 %, −11,3 dB |
| **3·10⁶** | **527, −3,3 %, −2,6 dB** | **641, −9,6 %** | **769, +1,6 %, −5,4 dB** | −12 %, −13,9 dB |
| 10⁶ | 524, −3,8 %, −2,2 dB | 627, −11,5 % | 740, −2,2 %, −7,4 dB | −4 %, −16,9 dB |
| frei | 513, −5,8 %, +1,0 dB | 698 (Bauch 2 Orte daneben) | 698, −7,7 %, −10,1 dB | −7 %, −15,0 dB |

S(1,2)–S(1,4) ändern sich nicht (+10,4 / +9,8 / −0,3 %; +1,6 / −4,6 / −5,5 dB). Mit 3·10⁶ N/m² liegen S(1,3)–S(1,7)
in den Schwellen von KQ19 a; S(1,2) bleibt bei +10,4 % Lage, S(1,8) wird schlechter.

**An Bells Messorten** (Stegpunkte an Ort 1–5 und 10 angehängt; Pegel zwischen den Resonanzen bei 300 / 400 / 500 /
600 Hz, gegen Kasten F lose, Abb. 6.6):

| | Ort 2 | Ort 5 |
|---|---|---|
| gemessen (F) | 86 / 91 / 94 / 96 dB; Spitzen 665 Hz 108, 975 Hz 108 | 81 (250 Hz); Spitzen 421 Hz 104, 535 Hz 103 |
| gehalten | 67 / 73 / 77 / 74; Spitzen 87–88 | 78 / 85 / 80 / 78; Spitzen 431 Hz 100, 530 Hz 97 |
| 3·10⁶ | 72 / 78 / 84 / 84; Spitzen 640 Hz 95, 767 Hz 95 | wie gehalten |
| 10⁶ | 75 / 81 / 88 / 91; Spitzen 626 Hz 100, 738 Hz 99 | Spitzen 431 Hz 100, 525 Hz 96 |
| frei | 78 / 85 / 98 / 100; Spitzen 513 Hz 103, 589 Hz 104 | Spitzen 430 Hz 100, 514 Hz 94 |

- An Ort 2 schließt die Feder 3·10⁶ N/m² 5–10 dB der Lücke von 17–22 dB; erst die freie Kante erreicht den
  gemessenen Pegel, legt dann aber die Moden falsch (513 / 589 / 701 Hz gegen 535 / 660 / 775 Hz an F).
- **Keine Feder an der Diskantkante bringt Frequenzen und Pegel am Diskantende zugleich hin.** Der Rest muss woanders
  liegen (der Boden am Diskantende weicher oder dünner als 3 mm + Furnier, der Kopfblock schwingt mit, die Leisten
  enden vor der Kante) — nicht untersucht.
- Die grobe Probe in §11 (Modenraum) hat die Wirkung an den Saiten überschätzt: sie las an Ort 1–2, wo keine Saite
  sitzt.

**KQ21 a (Patrik):** Fassung 7 bleibt, die Harfe bekommt 3·10⁶ N/m² an der Diskantkante — angepasst, nicht
gemessen. Der Körper `concert_harp` (bis 5 kHz, 144 Moden) gegen KQ19 a: Lage +10,4 / +9,7 / −0,3 / −3,3 / −9,6 /
+1,6 % ohne Saiten, +7,0 / +3,8 / +0,2 / −5,1 / −8,1 / +2,1 % besaitet; Ort überall unter 1 Messort; Höhe +1,6 /
−4,6 / −5,5 / −2,6 / – / −5,4 dB; S(1,8) −14,1 dB. Im Test „Luftraum die mitgelieferte Harfe“ festgehalten.

## 13 KQ22 a: je chromatischer Taste ein Stegpunkt (S199; `proben/2026-11-15_S199_harfe_nachbarpunkte.py`)

**Befund am Code:** `stegpunktVon` (`src/loop/Boden.cpp`) sucht die Taste genau; `bodenAntwortFuer` gibt sonst keinen
Klang, der Ton läuft ohne Faltung (`Instrument.cpp`, Kern-Pfad). Mit 46 Stegpunkten auf den weißen Tasten 26–103
hätten die 32 schwarzen Tasten dazwischen und die Tasten 25 (tiefste Saite, Pedal oben) und 104 (höchste Saite,
Pedal unten) keinen Körper.

**Nachgerechnet** (145 diskrete Moden, 100–2500 Hz, erster Hörort, ohne den statistischen Teil): der Unterschied der
Körperantwort zwischen Saiten im Abstand von einer Saite beträgt je Terzband im Mittel 1,5 dB (90 % unter 3,3 dB,
größter 10,4 dB an einem Knoten), breitbandig im Mittel 0,4 dB (größter 1,7 dB); im Abstand von zwei Saiten 2,6 /
5,5 / 15,1 dB und breitbandig 0,7 / 2,6 dB. Ob eine schwarze Taste an der Saite darunter oder darüber liegt, bleibt
damit unter der Schwelle von 6 dB aus KQ19 a.

**KQ22 a (Patrik):** `make_harfe.py` schreibt 80 Stegpunkte für die Tasten 25–104 an den 46 Ösen — die weiße Taste
an ihrer Saite (Pedal in der Mitte), die schwarze an der Saite darunter (Pedal unten), die Taste 25 an der tiefsten
Saite (Pedal oben), je Punkt mit Vermerk. Der Körper ist neu gerechnet (97 s) und Feld für Feld verglichen: gleich
bis auf Abdruck und Rechenzeit; an den 46 alten Tasten sind die Werte je Mode und die Streuung des statistischen
Teils Bit für Bit gleich, die 34 neuen Spalten tragen den Wert ihrer Saite. Die Zahlen gegen KQ19 a (§12) gelten
unverändert. Kippt, wenn die Pedalstellung bestimmen soll, welche Saite eine Taste spielt (dann trägt der Stegpunkt
die Saite, neue Bauplan-Fassung), oder wenn in 34k-6 an einer schwarzen Taste ein Sprung über 6 LU entsteht.

## 14 Die Mode S(1,1)+ bei 208 Hz und die Tasten 55–57 (S201, 34k-6 Punkt (1))

Anlass: im Prüfstand sind die Tasten 55–57 (196–220 Hz) 8–13 LU lauter als ihre Nachbarn und fallen mit 20–84 dB/s ab;
ihr Grundton liegt auf der Mode 208,4 Hz des Körpers. Die Höhe von S(1,1) war in S198 nicht geprüft. Proben:
`proben/2026-11-16_S201_harfe_gegen_bell_7_22.py`, `…_harfe_guete_s11.py`, Rohausgaben daneben.

**Gelesen (Bild).**

- Bell Abb. 7.9 (PDF 295): S(1,1)+ in der Harfe ohne Saiten bei 216 Hz, Q 16, Profil 70–82 dB; besaitet 207 Hz, Q
  unbestimmbar, 78–86 dB. Abb. 7.8: S(1,1)− in der Harfe 170 Hz, Q 11. S. 152: S(1,1)+ ist „considerably weaker“ als
  S(1,1)−.
- Bell Abb. 7.22 / 7.23 (PDF 309): Admittanz und Schalldruck **an der Öse jeder Saite bei deren Frequenz**, fertige
  Harfe besaitet, Saiten 33 … 10 (104 … 988 Hz). Admittanz (dB, 116 dB = 1 s/kg, ±1 dB): 80 / 89 / 74 / 79 / 81 / 82 /
  85 / 86 / 84 / 87 / 88 / 89 / 95 / 85 / 89 / 89 / 91 / 90 / 93 / 93 / 99 / 103 / 98 / 99. Text S. 159: mittlerer
  Schritt zwischen Nachbarsaiten rund 3 dB, größter 15 dB (Saiten 32 / 31); zwei Spitzen, B2 und S(1,3). Schalldruck
  (Bezug nicht genannt): Anstieg um rund 20 dB zwischen den Saiten 28 und 26, „due to the S(1,1) modes“.
- Bell Abb. 7.6 (PDF 293), Messort 17: ohne Saiten 65–93 dB, besaitet 76–90 dB — S. 150: besaitet ist die Admittanz
  „more even“, die Spitzen niedriger, die Täler höher.
- Le Carrou, Gautier & Foltête 2007 (JASA 121, Camac Atlantide Prestige, besaitet, Saiten mit Papier gedämpft), Abb. 7:
  Schalldruck je Kraft in 2,35 m (Fernfeld, reflexionsarm), Kraft auf der Mittellinie des Bodens: Spitze 27 · 10⁻³ Pa/N
  bei 204 Hz, 13–15 · 10⁻³ bei 150–190 Hz, 4 · 10⁻³ bei 240–250 Hz, 2 · 10⁻³ bei 290 Hz — **+16,6 dB bei 204 Hz gegen
  240–250 Hz**. S. 563: „in a playing configuration, the harp seems to radiate the sound more efficiently in the range
  140–230 Hz“. Abb. 5: Mobilität an Punkt 34 des Bodens 0,5–6 · 10⁻³ m/s/N (50–72 dB in Bells Maß).

**Gerechnet (Körperdatei, an der Öse bei der Frequenz der Saite; Saitenlast 0 / 0,25 / 1 Z ändert höchstens 2 dB).**

- **Die Mode hat die richtige Höhe:** an Saite 26 (207 Hz) 87,3 dB gegen 86,1 dB bei Bell (+1,2 dB).
- **Die Güte ist zu hoch:** die Datei trägt 129,0 / 191,8 / 208,4 Hz mit Q 25 / 27 / 27; gemessen in der Harfe Q 11
  (S(1,1)−) und 16 (S(1,1)+). Der Bauplan nimmt die Güte der Luft (`cavity.q` 16) vom losen Kasten F (Tab. 6.1: 18, 14).
- **Der Druck je Kraft bei 207 Hz liegt 19,8 dB über dem bei 247 Hz** (Le Carrou, andere Harfe: +16,6 dB bei 204 Hz).
  Die laute Stelle um 200 Hz ist also eine Eigenschaft der Konzertharfe, nicht ein Fehler der Mode; mit den
  gemessenen Güten wären es rund 16 dB.
- **Abseits der Resonanzen liegt die Admittanz der Datei 10–37 dB unter Bells Abb. 7.22** (Median −23 dB; Schritt
  zwischen Nachbarsaiten im Mittel 7,1 dB gegen 3,7 dB). Die Datei ist der Kasten ohne Saiten — an den Spitzen und in
  den Tälern passt sie zu Bells Messungen ohne Saiten (§11, Abb. 6.6). Was die besaitete Harfe bei Bell so eben macht,
  ist nicht geklärt: ein Widerstand je Saite (bis 1 Z) erklärt es nicht, und Le Carrous Mobilität an einer besaiteten
  Harfe (50–72 dB) liegt beim Modell, nicht bei Bells 80–100 dB. **Zwei Quellen widersprechen sich hier; nicht
  entschieden.** Am Diskant (Saiten 13–10: Modell 67–70 dB, Bell 98–103 dB) ist es der bekannte offene Punkt aus §12.

**Probe: Güte der Luft 5,5 statt 16** (Bauplan-Kopie, `InstrumentBodyTool … --to 5000`, 91 s): die drei Moden bekommen
Q 13 / 13 / 13 (gemessen 11 und 16: +1,5 / −1,8 dB an den Spitzen), S(1,2) 274,8 Hz Q 41 → 35 (gemessen 39).
Prüfstand je Taste (`proben/2026-11-16_S201_pruefstand_harfe_je_taste_luftguete_5.5.txt`):

| | Körper heute (Q der Luft 16) | Q der Luft 5,5 | Aufnahme KSHarp f |
|---|---|---|---|
| Spanne ab D2, jede Taste | 23,3 LU | 19,6 LU | – |
| größter Sprung (Paare bis 4 Halbtöne) | 13,5 LU (56 / 60) | 11,0 LU (56 / 60) | – |
| mittlerer Sprung | 2,8 LU | 2,9 LU | – |
| Spanne an den 20 Tasten der Aufnahme | 21,0 LU | 17,5 LU | 10,8 LU |
| Taste 55 gegen das Mittel von 52 und 59 | +9,4 LU | +6,5 LU | +4,5 LU |
| Tasten 55 / 56 / 57 | −3,3 / −1,3 / −3,7 LUFS | −6,9 / −5,1 / −6,2 LUFS | – |
| früher Abfall an Taste 56 | 84 dB/s | 81 dB/s | – |

Die Aufnahme trägt nur jede dritte bis vierte Taste (52, 55, 59 …); die Taste 56 auf der Mode fehlt ihr. Mit Q 5,5
hält die Harfe den größten Sprung nach GZ14 a (≤ 12 LU); die Spanne bleibt über 15 LU — die leisesten Tasten sind
92–94 (−23 … −25 LUFS) am Diskantende, wo die Admittanz zu tief liegt (§12).

**KQ29 a (Patrik):** Güte der Luft 5,5. `make_harfe.py` (`cavity.q`, Quelle neu), Körper mit `InstrumentBodyTool concert_harp.lvinstbodyplan --to 5000` neu (96 s): gegen den alten Körper ändern sich nur die Verlustfaktoren (143 von 145 Moden), gegen den Probekörper nur der Abdruck des Bauplans — die Zahlen der Tabelle gelten für die mitgelieferte Datei. Der Test „Luftraum die mitgelieferte Harfe“ prüft `cavity.q` und die Güte der Moden 191,8 / 208,4 Hz (zwischen 11 und 16).

**Offen:** das Diskantende (§12) — an ihm hängt jetzt die Spanne; warum Bells
besaitete Harfe eben ist (Le Carrou 2006, Thèse, auf Mobilitäten an den Saitenorten durchsehen); der schnelle Abfall
an Taste 56 (81 dB/s) gegen eine Aufnahme dieser Taste.


## 15 KQ31 a: die Admittanz ohne Saiten über 200 Hz — zwei Ursachen (S205)

Auftrag (KQ31 a): an Bells Ort 17 liegt das Modell über 200 Hz 6–10 dB unter Abb. 7.6 — bis zur Ursache untersuchen.
Proben: `proben/2026-11-18_S205_harfe_ort17_kurve.py` (die ganze Kurve statt 23 Stützstellen), `…_harfe_profile.py`,
`…_harfe_bauplan_zugleiste.py`, `…_harfe_bewegte_masse.py`, `…_harfe_je_taste.py`, Ausgaben und Bilder daneben. Gelesen am
Seitenbild: Bell Abb. 7.6 (PDF 293, neu abgegriffen, deckt sich mit Bericht 22), Abb. 6.6 (PDF 260), die Profile
Abb. 7.10–7.17 (PDF 296–303), RT-0425 §2.1 (PDF 7).

**Ausgangslage, ganze Kurve** (mitgelieferter Körper, Ösen 20 / 21 gegen Abb. 7.6 Feld 2): je Terzband 200 / 250 / 315 /
400 / 500 / 630 / 800 Hz +1,9 / −13,4 / −2,2 / −5,4 / −7,6 / −12,9 / −14,0 dB; 200–790 Hz energetisch −8,4 dB, Median
−11,9 dB, 10. Perzentil −12,3 dB.

### 15.1 Ursache im Bauplan: die Zugleiste fehlt in der Rechnung

- **Befund am Code:** `steifeBei` (`InstrumentBody/Platte.hpp` Z. 135–157) nimmt an einem Punkt die erste Leiste, auf
  der er liegt, Stege vor Rippen („der Steg geht vor“). Im Bauplan der Harfe liegen die Deckleiste (Steg, 33 → 18 mm
  breit, 5 → 3 mm hoch) und die Zugleiste (Rippe, 32–39 mm breit, 20–25 mm hoch) auf derselben Linie. Gerechnet wird in
  der Breite der Deckleiste also Platte + 5 mm Buche; von der Zugleiste bleiben nur die Streifen, die sie breiter ist
  (1,5–6 mm je Seite).
- **Beleg an der Masse:** das Werkzeug meldet für den mitgelieferten Bauplan 1,764 kg, mit beiden Leisten übereinander
  2,179 kg — der Unterschied 0,415 kg ist die Zugleiste (aus den Maßen 0,49 kg, abzüglich der Streifen).
- **Die übrigen Baupläne** (`proben/2026-11-18_S205_leisten_uebereinander.py`, Ausgabe daneben): an der Harfe verdeckt die Deckleiste 551 cm³ Zugleiste (0,41 kg Buche); am Steinway D liegen 157 cm² von 988 cm² Stegfläche über Rippen, 278 cm³ Rippe sind dort nicht gerechnet (auch an den beiden Glasböden); am Cembalo 0,5 cm², an Laute und Oud nichts. RT-0425 §2.1 nennt vier Zonen („each point belonging to a rib or not, and to a bridge or not“), gibt die Dicke aber nur für drei an (Platte 6–9 mm, Steg 29–69 mm, Rippen 9–35 mm).
- **Probe-Bauplan** (`…_harfe_bauplan_zugleiste.py`: die Deckleiste trägt in ihrer Breite die Höhe Deck- plus Zugleiste,
  die Zugleiste bleibt für die Streifen; sonst unverändert, Körper bis 5 kHz in 47–150 s; eine zweite Fassung mit einer
  einzigen Leiste in der Breite der Zugleiste liegt auf 1 Hz und 0,3 dB gleich):

| Mode (Bell, G in der Harfe ohne Saiten) | mitgeliefert: Hz, Lage | mit Zugleiste: Hz, Lage |
|---|---|---|
| S(1,1)− 170 Hz (lose 153) | 129,0 | 134,2 (−21 %; lose −12 %) |
| S(1,1)+ 216 Hz (lose 200) | 191,8 / 208,4 (zwei Moden teilen sich die Luft) | 198,3 (−8 %; lose −1 %) |
| S(1,2) 249 Hz | 274,8, +10,4 % | 233,0, −6,4 % |
| S(1,3) 316 Hz | 346,7, +9,7 % | 328,5, +4,0 % |
| S(1,4) 432 Hz | 430,8, −0,3 % | 433,4, +0,3 % |
| S(1,5) 545 Hz | 526,8, −3,3 % | 553,7 / 559,2, +1,6 % |
| S(1,6) 709 Hz | 640,7, −9,6 % | 698,5, −1,5 % |

- **Zahl und Lage aller Bäuche stimmen jetzt** (Ort, vom Diskant; Bell am losen Kasten und in der Harfe, ±1 Ort):

| Mode | Bell | mitgeliefert | mit Zugleiste |
|---|---|---|---|
| S(1,2) | 12, 30; Knoten 23 | 13, 24, 34; Knoten 19,5 und 29,5 | 15, 31; Knoten 25 |
| S(1,3) | 8, 18,5–20,5, 31–32 | 9, 19, 27, 35 | 9, 20,5, 33 |
| S(1,4) | 5, 14, 23, 32–35 | 6, 13, 20,5, 28, 36 | 6, 14,5, 24, 34 |
| S(1,5) | 3, 10, 18, 25, 35 | 4, 10, 16, 22, 28, 35 | 3, 10, 17,5, 25,5, 35 |
| S(1,6) | 3, 8, 14, 21, 26 | 3, 8, 13, 19, 24, 30, 35 | 3, 8,5, 14, 21, 28,5, 35,5 |

  Der mitgelieferte Körper hat im Bassdrittel bei jeder Mode einen Bauch zu viel — das ist „n + 1 Bäuche“ aus §11. Dort
  galt der überzählige Bauch als Schulter am Rand der Messorte; die Profile zeigen ihn nicht (bei S(1,3) liegt Bells
  Bauch 31–32 auf dem Knoten des Modells). **Die Zuordnung in §11 stimmte am stärksten Bauch und an der Frequenz, nicht
  an den übrigen Bäuchen.** Der Rand bleibt gestützt: mit der Zugleiste stimmen Zahl, Lage und Frequenz zugleich.
- **Die Höhen am stärksten Bauch liegen mit der Zugleiste tiefer:** S(1,2)–S(1,6) 91,8 / 95,5 / 96,1 / 94,7 / 92,4 dB
  gegen 96 / 104 / 105,5 / 101,5 / 100,5 dB: −4,2 / −8,5 / −9,4 / −6,8 / −8,1 dB (mitgeliefert +1,6 / −4,6 / −5,5 /
  −2,6 dB — mit einer um 0,4 kg zu leichten Mitte). Die Schwelle von KQ19 a (6 dB ohne Saiten) hält damit nur S(1,2).
  Mode 328,5 Hz zerlegt (`…_bewegte_masse_327.txt`): wirksame Masse am Bauch 216 g; 58 % der Bewegungsenergie im
  Diskantbauch, 38 % in Mitte und Bass; rund 70 % auf den Leisten. Aus Bells Pegel und Güte folgen 90 g
  (Q / (ω Y) = 45 / (1985 · 0,25)). Die Verhältnisse der Bäuche untereinander stimmen (Bell −15 / −20 dB, Modell
  −16,8 / −19,7 dB); es fehlt ein Faktor 2,4 in der Masse bei gleicher Frequenz, also auch in der Steife. Ein
  Überschlag aus Bells eigenen Maßen (Platte und Leisten am Diskantbauch, halbe Sinuswelle quer) gibt rund 190 g —
  **Bells Pegel an den Spitzen liegt rund 6 dB über dem, was seine Maße erlauben. Ursache offen** (Eichung, Güte, Maße
  am Diskant; „12 mm at the bass to 2 mm at the treble“ bei Bell & Firth gegen 10 → 3 mm in Abb. 2.4).

### 15.2 Ursache in der Messung: ein federartiger Untergrund bei Bell

- **Muster:** bei Bell liegt jedes Tal knapp **über** einer Spitze, danach steigt die Kurve langsam zur nächsten (Ort 17:
  Spitzen 251 / 319 / 436 / 564 Hz, Täler 290 / 385 / 454 / 606 Hz; ebenso an den Orten 2, 5 und 10 des losen Kastens,
  Abb. 6.6). So sieht ein Untergrund aus, der wie eine Feder wirkt (Y = i ω / k dazu). Im Modell liegen die Täler knapp
  **unter** den Spitzen (258 / 318 / 477 / 608 / 722 Hz) — der Untergrund ist dort die Masse der tieferen Moden. Bell
  S. 134 selbst: die Admittanz steige an allen Orten mit der Frequenz, „dominated by stiffness elements“.
- **Die Feder unabhängig abgelesen:** in den Profilen des losen Kastens (Feld 1 der Abb. 7.10–7.16) ist der Pegel an den
  Orten 37–40 waagrecht. Die Orte 39 und 40 liegen nach Abb. 6.3 nicht mehr auf dem Boden, sondern auf dem Bodenblock;
  dort bewegt sich nichts. Abgelesen (±1 dB): 221 Hz 70,5 dB · 320 Hz 73,8 · 420 Hz 76,0 · 557 Hz 79,5 · 702 Hz 80,0 ·
  768 Hz 80,5–82 dB. Das sind 6 dB je Oktave, und aus jedem Wert folgt dieselbe Feder: k = ω / Y = 2,6 / 2,6 / 2,6 /
  2,3 / 2,8 / 2,6 · 10⁵ N/m. (S(1,8) bei 912 Hz passt nicht: 94 dB, das ganze Profil liegt dort bei 89–100 dB.)
- **Mit dieser Feder, ohne Anpassung** (Ort 17, 200–790 Hz, Modell minus Bell):

| | Terzbänder 200 / 250 / 315 / 400 / 500 / 630 / 800 Hz | energetisch | Median | 10. Perzentil |
|---|---|---|---|---|
| mitgeliefert, ohne Feder | +1,9 / −13,4 / −2,2 / −5,4 / −7,6 / −12,9 / −14,0 | −8,4 | −11,9 | −12,3 |
| mitgeliefert, Feder 2,6 · 10⁵ N/m | +2,5 / −12,5 / −0,8 / −1,7 / 0,0 / +1,7 / +3,4 | −2,4 | +1,4 | +0,4 |
| mit Zugleiste, ohne Feder | −4,5 / −5,8 / −15,0 / −9,4 / −12,8 / −12,7 / −28,8 | −7,5 | −17,2 | −20,4 |
| mit Zugleiste, Feder 2,6 · 10⁵ N/m | −0,4 / −6,1 / −7,2 / +1,3 / +0,1 / +1,4 / +3,5 | −1,9 | +1,2 | +2,4 |

  Die Hochfläche 600–800 Hz in Abb. 7.6 (73 → 79 dB) ist die Feder. Es bleiben die Bänder 250 Hz (S(1,2): Lage −6 %,
  Höhe −4 dB) und 315 Hz: Ort 17 liegt im Modell fast auf dem Knoten von S(1,3) (Ort 16,5), bei Bell in der Harfe
  2–3 Orte daneben (Knoten bei 14–14,5) — ein Unterschied von 7 cm in der Lage eines Knotens macht dort 20 dB.
- **Woher die Feder kommt, ist nicht belegt.** Bell regt mit einem „small pin“ an einem Impedanzkopf B&K 8001 an und
  nennt weder Maße noch Befestigung (Bericht 22 §1.1). Eine Stiftspitze auf Holz quer zur Faser gibt nach
  k ≈ 2 a E / (1 − ν²) mit E ≈ 1–2 GPa und einem Kontaktradius von 0,1 mm 2–4 · 10⁵ N/m — eigene Abschätzung, keine
  Quelle. Dass der Wert an Orten über 14 cm, auf Block und Boden, gleich ist, spricht für die Messkette und gegen den
  Bau. Bericht 22 hatte die Reihenfeder verworfen, weil die kleinsten Werte der Profile unter der Federlinie liegen;
  das gilt nur ohne Auslöschung, und die Täler knapp über den Spitzen sind Auslöschungen.
- **Bells Pegel abseits der Resonanzen sind untereinander nicht stimmig:** an Ort 17 in der Harfe ohne Saiten lesen
  sich Profil und Abb. 7.6 bei 249 / 316 / 545 / 709 / 757 Hz zu 92 / 88 / 77 / 85 / 85 gegen 93 / 84 / 83 / 77,5 /
  79,5 dB (von Auge, ±2 dB) — bis 7 dB Unterschied, an der Resonanz 1 dB.
- **Für den Klang zählt die Feder nicht:** sie ist ein Blindanteil und nimmt der Saite keine Energie. Säße sie auch an
  der Öse, verlängerte sie die Saite um T / k (Saite 00: 43 N, 0,12 mm auf 71 mm, 2,9 Cent, in der Stimmung enthalten).

### 15.3 Zielgrößen mit der Zugleiste (Prüfstand je Taste, `…_harfe_je_taste_zugleiste.txt`)

| | mitgeliefert (S201) | mit Zugleiste | Schwelle KQ34 a |
|---|---|---|---|
| Spanne ab D2 | 19,6 LU | 21,7 LU (leiseste Taste 90, lauteste 64) | ≤ 22 LU |
| größter Sprung (Paare bis 4 Halbtöne) | 11,0 LU (56 / 60) | **14,6 LU (78 / 80)** | ≤ 12 LU |
| mittlerer Sprung | 2,9 LU | 3,0 LU | ≤ 3 LU |
| Moden unter 300 Hz (Hz / Q) | 129 / 13, 192 / 13, 208 / 13, 275 / 35 | 134 / 14, 198 / 8, 232 / 34 | Bell: 170 / 11, 216 / 16, 249 / 39 |

Die Taste 58 (233 Hz) liegt jetzt auf S(1,2) (−5,8 LUFS, früher Abfall 57 dB/s; Nachbarn −11 … −13 LUFS); die laute
Stelle der Tasten 55–57 aus §14 ist weg. Die Güte der Luft 5,5 (KQ29 a) und die Feder der Diskantkante 3 · 10⁶ N/m²
(KQ21 a) waren am Bauplan ohne Zugleiste gestellt und sind am neuen nicht nachgestellt; der Prüfstand rechnet ohne die
Last der übrigen Saiten (besaitet misst Bell ab S(1,5) die halbe Güte).

**Stand:** zwei Ursachen gefunden, der Körper im Repo ist unverändert. Entschieden (Patrik S205): **KQ31-1 a** — der Bauplan sagt, wie Leisten
übereinander zählen (Fassung 8, die Harfe „addieren“); **KQ31-2 a** — gemessen wird an den Moden (Lage 7 %, Zahl und Lage der Bäuche, Höhe der übrigen Bäuche gegen den stärksten 6 dB), die Höhe des stärksten Bauchs ist bekannte Abweichung; das Ziel „3 dB je Terzband an Ort 17“ misst
ohne die Feder die Messkette und hängt an der Lage eines Knotens. Danach: Körper neu, Güte der Luft und Diskantkante
nachstellen, KQ19 a und KQ34 a neu messen. Offen ohne Weg: die Höhe der Spitzen (−4 … −9 dB).

## 16 S206: die Harfe mit beiden Leisten gebaut und nach KQ31-2 a gemessen

Probe `proben/2026-11-18_S206_harfe_kq31_2.py` (Ausgabe `…_harfe_kq31_2.txt`); Prüfstand je Taste wie in §15.3
(Ausgabe `proben/2026-11-18_S206_harfe_je_taste.txt`). Bells Bäuche aus Abb. 7.10–7.14 Feld 2 (Kasten G in der Harfe
ohne Saiten; PDF 296–301) in S206 am Seitenbild von Auge gelesen, ±1 Ort, ±2 dB.

### 16.1 Bau (KQ31-1 a)

`.lvinstbodyplan` Fassung 8 trägt `barOverlap` (`first` / `add`), die Harfe `add`: an einem Punkt zählen die Höhen
aller Leisten zusammen, Werkstoff und Faserrichtung sind die der ersten (an der Harfe beide Buche, gleiche Richtung).
Der Körper rechnet wie der Probekörper aus §15.1: Masse der Rechnung 2,179 kg, 131 Moden bis 5 kHz, die ersten bei
134,2 / 198,3 / 233,0 / 328,5 / 433,3 / 553,4 Hz, Luftanteil 0,28 (134 Hz) und 0,67 (198 Hz). Das Werkzeug meldet
die Zugleiste unter der Deckleiste als Hinweis mit Fläche und Volumen.

### 16.2 Diskantkante (KQ21 a) nachgestellt: 3 · 10⁶ N/m² bleibt

| Wegfeder der Kante N/m² | S(1,5) Lage | S(1,6) Lage |
|---|---|---|
| gehalten | +5,2 % | +3,7 % |
| 10⁷ | +4,1 % | +1,6 % |
| **3 · 10⁶** | **+2,5 %** | **−1,5 %** |
| 10⁶ | −0,1 % | −5,6 % |
| 3 · 10⁵ | −2,6 % | −8,3 % |

S(1,2)–S(1,4) ändern sich mit der Feder nicht.

### 16.3 Güte der Luft (KQ29-1 a): 8,5

Mit der Zugleiste teilen sich die zwei Moden die Luft ungleich; ein Wert kann nicht beide Güten des Kastens in der
Harfe treffen (Bell Abb. 7.8, 7.9: 170 Hz Q 11, 216 Hz Q 16 — im Modell ist die Reihenfolge umgekehrt).

| Güte der Luft | Q bei 134 / 198 Hz | Spanne ab D2 (≤ 22) | größter Sprung (≤ 12) | mittlerer Sprung (≤ 3) |
|---|---|---|---|---|
| 5,5 (KQ29 a) | 13,7 / 7,7 | 21,9 LU | 14,6 LU (78 / 80) | 3,0 LU |
| **8,5** | **18,2 / 11,5** | **21,9 LU** | **14,5 LU (78 / 80)** | **3,3 LU** |
| 12,4 | 22,6 / 16,0 | 22,1 LU | 15,5 LU (43 / 47) | 3,5 LU |

Entschieden (Patrik S206): **KQ29-1 a** — 8,5, der Wert mit der kleinsten gemeinsamen Abweichung von beiden Güten im
Logarithmus (Faktor 1,65 und 1,39). Der Prüfstand rechnet ohne die Last der übrigen Saiten. **Der größte und der
mittlere Sprung liegen über den Schwellen von KQ34 a** (14,5 gegen 12 LU, 3,3 gegen 3 LU) — offen.

### 16.4 Messung nach KQ31-2 a (Güte 8,5, Kante 3 · 10⁶, ohne Feder im Messweg)

| Mode | Hz (Bell) | Lage | Bäuche | Lage der Bäuche gegen Bell in Messorten | Höhe der übrigen Bäuche gegen Bell in dB | stärkster Bauch |
|---|---|---|---|---|---|---|
| S(1,2) | 233,1 (249) | −6,4 % | 2 / 2 | **+2,1** / +1,6 | +2,5 | −3,5 dB |
| S(1,3) | 328,6 (316) | +4,0 % | 3 / 3 | +1,5 / **+2,5** / +1,2 | −1,8 / +0,1 | −8,3 dB |
| S(1,4) | 433,4 (432) | +0,3 % | 4 / 4 | −0,4 / +0,2 / +1,3 / +1,7 | +0,1 / −1,3 / −2,4 | −9,4 dB |
| S(1,5) | 558,9 (545) | +2,5 % | 5 / 5 | −0,6 / +0,2 / +1,5 / +1,0 / +1,0 | −4,0 / +0,2 / **−9,1 / −11,2** | −6,7 dB |
| S(1,6) | 698,7 (709) | −1,5 % | 6 / 6 | −0,7 / −0,3 / +0,1 / +0,7 / +0,4 / +1,1 | −5,8 / **−6,5 / −8,2 / −15,4 / −16,5** | −8,6 dB |

Frequenz und Zahl der Bäuche stimmen an allen fünf Moden. Über der Schwelle liegen zwei Lagen knapp (fett) und sechs
Höhen: die schwachen Bäuche von S(1,5) (Orte 25, 34) und S(1,6) (Orte 14–34). Als Bericht: über S(1,6) hat das Modell
Moden bei 683 / 698 / 809 / 841 / 940 Hz, alle schwach am Diskantende; S(1,8) liegt 16 dB unter Bell (bekannt, §12).

### 16.5 Die schwachen Bäuche und die Feder aus §15.2

Bells schwache Bäuche liegen nur 6–9 dB über der Linie der Feder 2,6 · 10⁵ N/m (bei 545 Hz 78,4 dB). Mit der Feder in
Reihe trifft das Modell Bells Pegel an diesen Orten relativ zum stärksten Bauch auf rund 4–6 dB — aber nur, weil sein
stärkster Bauch 7–9 dB zu tief liegt; und mit der Feder verschwinden die schwachen Bäuche im Modell als Maxima. Die
Feder allein erklärt sie also nicht.

Entschieden (Patrik S206): **KQ31-3 a** — die Schwellen von KQ31-2 a bleiben; die zwei Lagen und die sechs Höhen stehen
als bekannte Abweichungen mit ihren Werten im Test „Luftraum die mitgelieferte Harfe“ (rot, wenn eine wächst oder
verschwindet). Ursache offen, zusammen mit der Höhe der Spitzen.

**Stand:** der Körper der Harfe im Repo ist der neue (Fassung 8, `add`, Güte 8,5). Offen ohne Weg: die Höhe der
stärksten Bäuche (−3,5 … −9,4 dB), die schwachen Bäuche von S(1,5) / S(1,6) (−6,5 … −16,5 dB), die Sprünge je Taste
über KQ34 a.

## 17 S209: Suche am Bauplan nach KQ40 a — Kandidat, Saiten als Federn (KQ46 a–KQ48 a)

Suchgerüst und Ausgaben: `proben/2026-11-19_S209_harfe_suche/` (`harfe_suche.py` rechnet eine Fassung mit 16 Hörorten
in 2,06 m bis 1,2 kHz in 18 s und misst L1 = Lage der zwei Moden mit Luft gegen 170 / 216 Hz, L2 = Lage S(1,2)–S(1,6),
A = \|p/F\| an der Öse der Saiten 33–22 gegen Bell Abb. 7.23; Basis `harfe_ausgang.lvinstbodyplan`). **Am Repo ist
nichts geändert** — der Körper der Harfe ist der aus S206.

**Ausgang:** 134,2 / 198,3 Hz (−21 / −8 %), L2 −6,4 / +4,0 / +0,3 / +2,5 / −1,4 %, A 4 / 12 (größte 19,6 dB) — wie S208.

**Einzelne Stellgrößen** (`abtasten1.txt`, `abtasten2.txt`):

| Stellgröße | Moden mit Luft | L2 S(1,2)–S(1,6) | A |
|---|---|---|---|
| Bassende eingespannt | 148 / 204 | +0,9 / +10,5 / +5,7 / +7,1 / +2,0 % | 6 / 12 |
| Deck- und Zugleiste bis an die Bassenkante | wie Ausgang | wie Ausgang | 4 / 12 |
| Mündung 2,2 / 3,0 | 137 / 209 · 131 / 190 | unverändert | 3 / 12 · 6 / 12 |
| Hohlraum 23 / 31 l | 136 / 211 · 133 / 188 | unverändert | 2 / 12 · 5 / 12 |
| Drehfeder am ganzen Seitenrand 300 / 1000 / 3000 N | 138 / 200 · 145 / 202 · 158 / 209 | bis +12 % | 3–6 / 12 |
| ohne Luft | 146 (Platte allein) | −7,2 / +3,3 / +0,3 / +2,5 / −1,4 % | — |
| Boden 12 → 2 mm | 145 / 201 | −4,6 / +0,3 / −4,9 / −1,5 / −3,1 % | 5 / 12 |
| Boden 12 → 3 / 11 → 3 / 13 → 3 mm | 150 / 204 · 143 / 201 · 157 / 208 | bis +12 % | 3–6 / 12 |
| Drehfeder nur an den unteren 60 cm der Seiten, 10⁴ N | 163 / 219 | −2,3 / +7,1 / +2,8 / +4,4 / +0,7 % | 2 / 12 |

Die Platte ist im Bassteil zu weich und stimmt am Diskant: was nur den Bass versteift (Dicke 12 → 2 mm, Feder am
Bassteil), hebt S(1,1)−, ohne die höheren Moden zu verschieben.

**Die Saiten als Federn an den Ösen** (`harfe_saiten.py`, `saiten1.txt`; im Modenraum: K = diag(ω²) + Σ k φφᵀ, die
Formen sind samt Luft massennormiert). k je Saite = c · E·A/L; E Nylon 4, Darm 5,5, Stahlkern 200 GPa (Annahmen),
Durchmesser und Längen Bell Tab. 2.1 / 2.2; Summe E·A/L: Nylon 1,1 · 10⁵, Darm 4,1 · 10⁵, Metall 1,3 · 10⁶ N/m. Mit
c = 0,2 am Ausgang: 134,2 → 146,1 Hz (+9 %), S(1,2) 233 → 243 (+4 %), S(1,3) +2,4 %, S(1,4) +1,5 %, S(1,5) +0,9 %.
Bell beim Besaiten: S(1,1)− 170 → 187 (+10 %), S(1,2) 249 → 257 (+3 %), S(1,3) +5,7 %, S(1,4) −0,5 %, S(1,5) +1,8 %.
Ein Faktor gibt die zwei tiefsten Verschiebungen wieder. **Das Ziel der Lagen (KQ40 a) gilt ohne Saiten, das der
Abstrahlung besaitet — ein Körper ohne Saiten kann nicht an beiden liegen.**

**Raster** (`raster1.txt`: Dicke am Bass 12 / 13 / 14 mm bei 2 mm am Diskant × Mündung 1,7 / 2,0 / 2,3 × Bassende
gestützt / eingespannt, je ohne und mit Saiten c = 0,2). **Kandidat: Boden 12 → 2 mm, Mündung 2,0, Bassende
eingespannt.**

| Zielgröße | Bauplan heute | Kandidat | Kandidat mit Saiten |
|---|---|---|---|
| Moden mit Luft gegen 170 / 216 Hz (7 %) | 134 / 198 (−21 / −8 %) | 164 / 222 (−4 / +3 %) | 171 / 225 |
| S(1,2)–S(1,6) (7 %) | −6,4 / +4,0 / +0,3 / +2,5 / −1,4 % | +1,7 / +5,9 / +0,3 / +3,2 / +0,7 % | +5,1 / +8,5 / +1,8 / +4,1 / +1,1 % (gegen Bell ohne Saiten) |
| KQ31-2 a (bis 1,2 kHz gerechnet) | 2 Lagen der Bäuche, 6 Höhen außerhalb | 4 Höhen außerhalb (S(1,5): −9,8 / −11,5; S(1,6): −13,4 / −13,2 dB) | nicht gemessen |
| stärkster Bauch S(1,2)–S(1,6) gegen Bell | −3,5 / −8,3 / −9,4 / −6,7 / −8,6 dB | −2,4 / −6,1 / −7,4 / −5,1 / −9,6 dB | — |
| A Saiten 33–22 (6 dB) | 4 / 12, größte 19,6 dB | 6 / 12, größte 16,3 dB | 6 / 12, größte 13,8 dB |
| Lautheit 38–104, 16 Hörorte, ohne Mitschwingen (`lautheit1.txt`; ≤ 22 / 12 / 3 LU) | 20,0 / 12,8 / 3,0 | 16,8 / 13,0 / 2,7 | **17,8 / 10,5 / 2,8** |

Der heutige Bauplan mit Saiten: 19,2 / 14,1 / 2,9 LU. Im Kandidaten ist die Stufe 43 / 47 weg (43 −23,2, 45 −21,4,
47 −23,6 LUFS).

**A hält in keiner Fassung mehr als 6 von 12.** Der Überschuss bei 155 / 165 Hz und um 233 Hz bleibt 10–15 dB. Bells
Wert an Saite 26 (207 Hz) liegt an der besaiteten Harfe auf einer Mode (187 / 207 Hz); im Modell liegen die beiden
Moden mit Luft 50–60 Hz auseinander, 207 Hz fällt zwischen sie. Mit zwei gekoppelten Schwingern gerechnet: der Ausgang
(Platte 146, Helmholtz 185 → 134 / 198 Hz) hat eine Kopplung κ ≈ 4600 Hz²; Bells Paar ohne Saiten 170 / 216 Hz verlangt
damit Platte ≈ 174 und Helmholtz ≈ 213 Hz, sein Paar besaitet 187 / 207 Hz verlangt κ ≤ 3900 Hz² — die beiden Paare
passen nicht zu einer festen Kopplung. Bell nennt für S(1,1)− besaitet keine Güte („unbestimmbar“).

**Entschieden (Patrik S209):** KQ46 a (Federn an Punkten im Bauplan, Fassung 9, der Körper ist die besaitete Harfe),
KQ47 a (der Kandidat, nach neuer Eichung des Holzes fein gesucht), KQ48 a (A bekannte Abweichung mit Wert).

**Offen vor dem Bau:** die Eichung des Holzes am freien Boden mit 12 → 2 mm (§7.1 eichte mit 10 → 3 mm); die Feinsuche
am Netz des Körpers gegenprüfen (an der Oud lag eine Mode am feinen Netz 1,5 % tiefer); der Faktor der Saiten am
neuen Bauplan neu stellen; die Lautheit mit Mitschwingen (KQ39 a).

## 18 S210: Federn an Punkten gebaut (KQ46 a), ihr Verlust (KQ49 a)

**Gebaut:** `.lvinstbodyplan` Fassung 9 mit `pointSprings` (je Feder `x`, `y`, `stiffness` in N/m, `lossFactor`,
`source`; immer geschrieben). Die Rechnung setzt je Feder k · N Nᵀ auf die Auslenkungen des Elements, in dem der Punkt
liegt (`setzePunktfedern`, `baueSystem(netz, element, plan)`); `InstrumentBody` 0.25.0. Der Bauplan der Harfe im Repo
hat noch keine Federn — er bekommt sie mit dem Kandidaten (KQ47 a), nach der Eichung des Holzes.

**Geprüft** (Test „KQ46 Punktfedern“, frei aufliegende Prüfplatte 50 × 40 cm, 3 mm, eine Feder, die die erste Mode von
36,7 auf 43,6 Hz hebt): gegen die Reihe nach Navier 43,637 gegen 43,593 Hz (0,1 %), zweite Mode 67,220 gegen 67,142 Hz;
gegen die Rechnung im Modenraum, wie `harfe_saiten.py` sie führt (K = diag(λ) + k φ φᵀ), mit allen Moden des Netzes
gleich, mit den ersten 60 Moden 43,652 Hz (0,03 %); am feineren Netz (4,5 statt 7 cm) 43,616 Hz (0,05 % tiefer — die
Punktnachgiebigkeit der schubweichen Platte wächst mit dem Netz langsam). **Die Zahlen der Suche aus §17, im Modenraum
gerechnet, gelten damit für die Rechnung im Netz bis auf Zehntelprozent in der Lage** — mit dem Unterschied, dass
`harfe_saiten.py` die Federn nach der Kopplung mit der Luft setzt und das Werkzeug davor; das zeigt erst der Bau des
Kandidaten.

**KQ49 a (Patrik S210) — der Verlust der Federn:** der Anteil der Verformungsenergie einer Mode, der in den Federn
steckt, trägt deren Verlustfaktor (`lossFactor`, 0 = ohne Verlust), der Rest den der Platte:
η = (1 − Anteil) · η_Platte + Σ a_f η_f. Die Harfe schreibt 0 (Annahme: die Längssteife der Saite verliert nichts; so
rechnete die Probe aus S209). Verworfen: die Energie der Federn in der Dämpfung nicht zählen (die Mode bekäme das
Gesetz des Holzes an ihrer neuen Frequenz). Am Kandidaten mit Saiten c = 0,2 gerechnet
(`proben/2026-11-20_S210_kq49_federverlust.py`, `.txt`):

| Zielgröße | a Federn ohne Verlust | b nicht gezählt |
|---|---|---|
| Lagen | gleich | gleich |
| η der Moden 170,8 / 225,3 / 261,6 Hz | 0,0521 / 0,0801 / 0,0251 | 0,0559 / 0,0826 / 0,0268 |
| größter Unterschied in η (129 Moden) | — | +7,2 % (0,6 dB an der Spitze) |
| Lautheit 38–104, 16 Hörorte, ohne Mitschwingen | 17,8 / 10,5 / 2,8 LU | 17,6 / 10,7 / 2,7 LU |

Größter Unterschied an einer Taste 0,5 LU (48). Kippen würde a zu einem Wert größer 0, wenn eine Messung der Güte
besaiteter Moden vorläge; Bell nennt für S(1,1)− besaitet keine.

**Offen wie in §17:** Eichung des Holzes mit 12 → 2 mm, Feinsuche (am Netz für 5 kHz gegengeprüft), Faktor der Saiten,
`make_harfe.py` mit den Federn, Körper, Test, Lautheit mit Mitschwingen (KQ39 a).

## 19 S210: Eichung des Holzes mit 12 → 2 mm — der Kandidat aus §17 hält nicht (KQ50 a)

Proben: `proben/2026-11-20_S210_harfe_eichung.py` (freier Boden ohne Leisten, Basis eingefroren in
`…_harfe_eichung_basis.lvinstbodyplan`), `…_harfe_kandidat_geeicht.py`, `…_harfe_raster2.py`; Ausgaben je `.txt`,
dazu `…_harfe_raster2_lautheit_kq31_2.txt`. **Am Repo ist nichts geändert** — Bauplan und Körper der Harfe sind die
aus S206 (auf Fassung 9).

**19.1 Zuordnung der (2,n)-Moden.** Gerechnet hat die Familie mit zwei Knotenlinien längs 0 / 1 / 2 / 3 / 4
Knotenlinien quer bei 279 / 346 / 400 / 451 / 502 Hz (10 → 3 mm, Holz aus §7.1; an den Vorzeichenbildern der Formen
gelesen). Bell Tab. 3.2 nennt (2,0) 277, (2,2) 345, (2,3) 394, (2,4) 440 Hz und (2,1) „nicht gefunden“. §7.1 hat die
vier Werte der Reihe nach zugeordnet, also Bells (2,2) auf die gerechnete (2,1). Nach Bells Zählung lägen (2,2)–(2,4)
+16 / +15 / +14 % daneben, während (2,0) auf 1 % stimmt; die Abstände (gemessen 68 / 49 / 46 Hz, gerechnet 67 / 54 /
51 Hz) passen nur der Reihe nach. **Annahme, wie §7.1: der Reihe nach** — nicht belegt, Bells Chladni-Bilder dieser
Moden sind nicht gegengelesen. Die Moden sitzen am Bassende.

**19.2 Eichung.** Elf Moden (Tab. 3.1 (0,6)–(0,12), Tab. 3.2), angepasst E quer zum Boden, E längs, G; am Netz für
520 / 1500 / 5000 Hz gleich.

| Dicke (+ 1 mm Furnier) | Dichte (Masse) | E quer / längs / G in GPa | größte Abweichung | Wurzel aus dem Mittel der Quadrate |
|---|---|---|---|---|
| 10 → 3 mm, Holz aus §7.1 | 500 (1,371 kg) | 7,3 / 0,855 / 0,45 | 2,6 % | 1,35 % |
| 10 → 3 mm, neu angepasst | 500 (1,371 kg) | 7,33 / 0,865 / 0,368 | 2,0 % | 0,94 % |
| 12 → 2 mm, Holz aus §7.1 | 456,7 (1,375 kg) | 7,3 / 0,855 / 0,45 | 23 % ((2,n) +19 … +23 %, (0,n) +4 … +7 %) | 12,2 % |
| **12 → 2 mm, neu angepasst** | 456,7 (1,375 kg = Mittel der drei Böden) | **4,77 / 0,768 / 0,387** | 1,8 % | 0,83 % |
| 12 → 2 mm, neu angepasst | 500 (1,505 kg) | 5,23 / 0,844 / 0,429 | 1,7 % | 0,79 % |

**Der freie Boden unterscheidet die beiden Dicken nicht** (0,94 gegen 0,83 %) — das Holz gleicht aus. Mit 12 → 2 mm
ist E quer zum Boden 35 % kleiner; die Biegesteife quer am Bassende (E h³) liegt damit nur 8 % über der des heutigen
Bauplans, nicht 65 % wie im Kandidaten aus §17, der 12 → 2 mm mit dem Holz für 10 → 3 mm rechnete.

**19.3 Der Kandidat mit geeichtem Holz** (Mündung 2,0, Bassende eingespannt; Lagen ohne Saiten):

| Fassung | Moden mit Luft gegen 170 / 216 Hz | S(1,2)–S(1,6) | A ohne / mit Saiten |
|---|---|---|---|
| heute (10 → 3, Mündung 2,6, Bassende gestützt) | 134 / 198 (−21 / −8 %) | −6,4 / +4,0 / +0,3 / +2,5 / −1,4 % | 4 / 4 von 12 |
| Kandidat §17 (Holz für 10 → 3) | 164 / 222 (−4 / +3 %) | +1,7 / +5,9 / +0,3 / +3,2 / +0,7 % | 6 / 6 |
| **Kandidat, Holz geeicht** | **145 / 214 (−15 / −1 %)** | −7,9 / −3,0 / −5,0 / −0,1 / −1,8 % | 4 / 5 |
| Kandidat, Holz geeicht, die zwei Fichtenleisten mit dem Holz aus §7.1 | 145 / 214 | −7,9 / −2,9 / −4,8 / +0,3 / −1,4 % | 4 / 5 |
| Kandidat, Holz geeicht mit 500 kg/m³ | 148 / 215 | −7,1 / −2,4 / −5,2 / −0,8 / −2,9 % | 3 / 5 |
| 10 → 3 mm, Mündung 2,0, Bassende eingespannt | 154 / 220 (−10 / +2 %) | +1,1 / +10,5 / +5,7 / +7,1 / +2,0 % | 4 / — |

Der Gewinn des Kandidaten an S(1,1)− kam aus dem ungeeicht steiferen Holz. Mit Saiten (c = 0,2) liegt die geeichte
Fassung bei 154 / 219 Hz.

**19.4 Raster nach der Eichung** (`…_harfe_raster2.txt`: Dicke mit ihrem Holz × Mündung 2,0 / 2,6 × Bassende
gestützt / eingespannt × Drehfeder an den unteren 60 cm der Seiten 0 / 10³ / 3 · 10³ / 10⁴ N). Ohne die Drehfeder
kommt keine Fassung an S(1,1)− über 154 Hz. Vier Fassungen bis 5 kHz gerechnet, mit Lautheit je Taste (16 Hörorte,
ohne Mitschwingen; Grenzen 22 / 12 / 3 LU) und der Messung nach KQ31-2 a:

| Zielgröße | A: 12 → 2 geeicht, eingespannt, ohne Feder (KQ47 a wörtlich) | B: 12 → 2 geeicht, eingespannt, Feder 10⁴ N | C: 10 → 3, gestützt, Feder 10⁴ N, Mündung 2,0 | D: wie C, Mündung 2,6 |
|---|---|---|---|---|
| Moden mit Luft (7 %) | 145 / 214 (−15 / −1 %) | 169 / 226 (−1 / +5 %) | 174 / 229 (+2 / +6 %) | 163 / 218 (−4 / +1 %) |
| Güte der zwei Moden (Bell 11 / 16) | 21 / 14 | 16 / 13 | 15 / 14 | 12 / 18 |
| S(1,2)–S(1,6) (7 %) | −7,9 / −3,0 / −5,0 / −0,1 / −1,8 | −4,0 / −0,3 / −2,7 / +2,2 / +0,4 | −2,1 / +7,1 / +2,8 / +4,4 / +0,6 | −2,3 / +7,1 / +2,8 / +4,4 / +0,6 |
| KQ31-2 a: Bäuche in der Lage außerhalb (2 Orte) | 3 | 5 (alle zum Diskant verschoben, bis 3,5 Orte) | 0 | 0 |
| KQ31-2 a: Höhen außerhalb (6 dB) | 4 | 4 | 6 | 7 |
| stärkster Bauch S(1,2)–S(1,6) gegen Bell | −7,5 / −4,7 / −6,7 / −4,9 / −10,1 dB | −0,1 / −5,1 / −6,7 / −5,2 / −10,5 | −3,8 / −8,3 / −9,6 / −6,9 / −8,6 | −2,9 / −8,2 / −9,6 / −6,9 / −8,6 |
| A Saiten 33–22 ohne / mit Saiten (6 dB) | 4 / 5, größte 16,4 / 24,2 dB | 4 / 6, 16,4 / 18,1 | 7 / 5, 18,1 / 20,8 | 2 / 4, 14,5 / 16,9 |
| Lautheit ohne Saiten | 20,6 / 12,8 / 2,7 LU | 16,8 / 13,8 / 2,3 | 17,6 / 13,9 / 2,6 | 18,0 / 14,5 / 2,5 |
| Lautheit mit Saiten c = 0,2 | 20,7 / 12,7 / 2,9 | 18,2 / 13,4 / 2,4 | 18,4 / 13,2 / 2,7 | 17,6 / 14,0 / 2,7 |

(Heute: 20,0 / 12,8 / 3,0 LU, mit Saiten 19,2 / 14,1 / 2,9; der Kandidat aus §17 mit Saiten 17,8 / 10,5 / 2,8.)

**Befunde.** (1) Die Lage der Bäuche trennt die Dicken, wo der freie Boden es nicht tut: mit 12 → 2 mm sitzen die
Bäuche von S(1,2)–S(1,4) in jeder Fassung 1–3,5 Messorte zu weit am Diskant, mit 10 → 3 mm alle innerhalb von
2 Orten. (2) Die fehlende Steife im Bass liefert in diesem Raster nur die Drehfeder an den Bassseiten — ein
angepasster Wert ohne Quelle (wie die Diskantkante, KQ21 a). (3) Den größten Sprung ≤ 12 LU hält keine der vier
Fassungen; die 10,5 LU des Kandidaten aus §17 gehörten zum ungeeichten Holz. (4) Die Feinsuche (Länge und Wert der
Feder, Mündung, Bassende, Faktor der Saiten) steht aus.

**KQ50 a (Patrik S210; hebt KQ47 a auf):** der Boden bleibt 10 → 3 mm (Abb. 2.4 a) mit dem Holz aus §7.1; die Steife
im Bass kommt aus einer Drehfeder an den Bassseiten als Randabschnitt (angepasst, ohne Quelle, wie die Diskantkante);
Feinsuche ab den Fassungen C / D. Verworfen: 12 → 2 mm geeicht mit Feder (B), KQ47 a wörtlich (A).

## 20 S210: Feinsuche nach KQ50 a

Probe `proben/2026-11-20_S210_harfe_feinsuche.py`; Ausgaben `…_feinsuche1.txt` (27 Fassungen; der Lauf brach an einer
Fassung ohne zwei Moden mit Luft ab), `…_feinsuche2.txt` (36), `…_feinsuche_lautheit_kq31_2.txt`,
`…_feinsuche_saitenfaktor.txt`. Boden 10 → 3 mm, Holz aus §7.1, Bassende gestützt (eingespannt lägen S(1,3)–S(1,5)
+7 … +12 %, §19.4). Stellgrößen: Drehfeder an den Seiten vom Bassende bis y = `bis`, ihr Wert, die Mündung. **Am Repo
ist nichts geändert.**

**20.1 Raster.** Maß = Wurzel aus dem Mittel der Quadrate der sieben Lagen ohne Saiten (zwei Moden mit Luft, S(1,2)–
S(1,6)). Das Feld ist flach: `bis` 0,6–0,7 m, Feder 3 000–7 000 N, Mündung 2,0–2,3 liegen bei 3,4–3,7 %, alle sieben
Lagen innerhalb 7 %. Außerhalb davon: Feder nur bis 0,35 m hebt S(1,1)− auch mit 10⁵ N nicht über 159 Hz; Feder bis
0,85 m und länger verschiebt S(1,2) um +10 % und mehr; Mündung 2,6 lässt S(1,1)− bei 146–164 Hz. S(1,3) liegt in
allen Fassungen +5 … +8 % (ohne Feder +4,0 %) — die Feder kostet dort rund 2 %.

**20.2 Vier Fassungen bis 5 kHz** (die Lagen am Netz für 5 kHz gleich denen am Netz für 1,2 kHz, auf 0,1 Hz):

| Zielgröße | F1: Mündung 2,15, bis 0,6 m, 7 000 N | F2: 2,0, 0,6 m, 5 000 N | F3: 2,0, 0,7 m, 3 000 N | F4: 2,3, 0,6 m, 7 000 N |
|---|---|---|---|---|
| Moden mit Luft gegen 170 / 216 Hz (7 %) | 166,7 / 222,1 (−1,9 / +2,8 %) | 164,1 / 223,0 (−3,5 / +3,2 %) | 161,5 / 220,1 (−5,0 / +1,9 %) | 164,4 / 218,9 (−3,3 / +1,3 %) |
| Güte der zwei Moden (Bell 11 / 16) | 15 / 14 | 17 / 13 | 18 / 13 | 14 / 15 |
| S(1,2)–S(1,6) (7 %) | −3,0 / +6,5 / +2,3 / +4,1 / +0,2 | −3,4 / +6,0 / +1,9 / +3,7 / −0,1 | −1,9 / +6,2 / +1,4 / +3,6 / −0,3 | −3,1 / +6,5 / +2,3 / +4,1 / +0,2 |
| KQ31-2 a: Bäuche in der Lage außerhalb | 0 | 1 (S(1,3), +2,1 Orte) | 1 (S(1,3), +2,2 Orte) | 0 |
| KQ31-2 a: Höhen außerhalb (6 dB) | 6 | 6 | 6 | 6 |
| stärkster Bauch S(1,2)–S(1,6) gegen Bell | −3,6 / −8,3 / −9,6 / −6,8 / −8,6 dB | −5,0 / −8,4 / −9,5 / −6,8 / −8,6 | −5,5 / −8,6 / −9,5 / −6,7 / −8,7 | −3,2 / −8,2 / −9,6 / −6,8 / −8,6 |
| A Saiten 33–22 ohne / mit Saiten 0,2 (6 dB) | 4 / 5 | 5 / 6 | 6 / 6 | 2 / 6 |
| Lautheit ohne Saiten, ohne Mitschwingen (22 / 12 / 3 LU) | 17,2 / 12,4 / 2,5 | 18,1 / 11,4 / 2,7 | 20,1 / 12,5 / 2,7 | 17,8 / 13,1 / 2,6 |
| Lautheit mit Saiten 0,2, ohne Mitschwingen | 18,1 / 12,5 / 2,7 | 18,3 / 11,4 / 2,8 | 17,7 / 9,7 / 2,7 | 17,6 / 12,1 / 2,7 |

Die sechs Höhen außerhalb sind in allen vier dieselben: die zwei bassseitigen Bäuche von S(1,5) (−8 / −10 dB) und
die vier von S(1,6) (−6 … −15 dB) — wie §16.5 (die Feder im Messweg bei Bell). **Der größte Sprung der Lautheit
schwankt zwischen Nachbarn des Rasters um 3 LU** und trennt die Fassungen ohne Mitschwingen nicht.

**20.3 Faktor der Saiten** (k = c · E·A/L, im Modenraum; Bell beim Besaiten: S(1,1)− +10 %, S(1,2) +3 %, S(1,3)
+5,7 %). An F2:

| c | S(1,1)− | S(1,2) | S(1,3) | Moden mit Luft (Bell besaitet 187 / 207 Hz) | A |
|---|---|---|---|---|---|
| 0,2 | +5,3 % | +3,8 % | +2,5 % | 172,8 / 228,8 | 6 / 12 |
| 0,3 | +7,4 % | +6,1 % | +3,6 % | 176,3 / 231,4 | 6 / 12 |
| 0,4 | +9,3 % | +8,3 % | +4,8 % | 179,4 / 233,9 | 6 / 12 |
| 0,5 | +10,9 % | +10,3 % | ≥ +5,6 % (Rand des Suchfensters) | 182,0 / 236,5 | 6 / 12 |

Am Bauplan aus S206 gab c = 0,2 beide tiefen Verschiebungen wieder (+9 / +4 %, §17). Mit der Feder am Bass
verschiebt sich S(1,1)− beim Besaiten weniger: ein Faktor trifft nicht mehr beide (kleinste Summe der Quadrate über
die drei Verschiebungen bei c ≈ 0,3). F3 und F4 verhalten sich gleich.

**20.4 Lautheit mit Mitschwingen** (KQ39 a, O+Q nach GZ8 a; 67 Tasten 38–104, 16 Hörorte, Saiten 0,2;
`proben/2026-11-19_S208_harfe_mitschwingen_16.py`, Ausgabe `proben/2026-11-20_S210_harfe_feinsuche_mitschwingen.txt`).
Spanne / größter Sprung / mittlerer Sprung gegen 22 / 12 / 3 LU:

| Fassung | nur die gespielte Saite | mit Mitschwingen |
|---|---|---|
| F1 (Mündung 2,15, Feder bis 0,6 m, 7 000 N) | 18,1 / 12,5 / 2,7 | 18,9 / **12,6** / 2,4 |
| F2 (Mündung 2,0, Feder bis 0,6 m, 5 000 N) | 18,3 / 11,4 / 2,7 | 19,3 / 11,5 / 2,7 |
| F3 (Mündung 2,0, Feder bis 0,7 m, 3 000 N) | 17,7 / 9,7 / 2,7 | 19,2 / 10,7 / 2,5 |
| F4 (Mündung 2,3, Feder bis 0,6 m, 7 000 N) | 17,6 / 12,1 / 2,7 | 18,4 / **12,2** / 2,5 |

F2 und F3 halten alle drei Grenzen, F1 und F4 verfehlen den größten Sprung um 0,6 / 0,2 LU — weniger als die 3 LU,
um die er zwischen Nachbarn des Rasters schwankt (§20.2). Das Mitschwingen ändert die Lautheit je Taste im Median um
−0,1 LU, an einzelnen Tasten um +0,5 bis −4,2 LU (über 1 LU leiser in allen vier: 41, 45, 47, 48, 57, 65, 69; am
stärksten 47 mit −4,2 / −4,0 / −2,3 / −4,2 LU und 45 mit −3,2 / −2,7 / −3,0 / −3,6 LU). Es
hebt die Spanne um 0,8 / 1,0 / 1,5 / 0,8 LU (F1–F4) und senkt den mittleren Sprung um 0,3 / 0,0 / 0,2 / 0,2 LU.
Der größte Sprung liegt in F1, F2, F4 zwischen den Tasten 54 und 58 (F1 −21,2 → −8,6, F2 −20,7 → −9,2, F4 −21,2 →
−9,0 LUFS; F3 dort 9,8 LU), in F3 zwischen 44 und 48 (−18,9 → −29,5 LUFS; F1 / F2 / F4 dort 8,1 / 8,8 / 8,8 LU). Der
leiseste Bereich liegt in allen vier an den Tasten 47–52, der lauteste an 58–59. Die Messung gilt für den Faktor 0,2
der Saiten; für 0,3 und 0,4 ist die Lautheit nicht gerechnet.
Die Wahl der Fassung (KQ51) und des Faktors der Saiten (KQ52) ist offen.

**20.5 Stand der Vorlage** (Ende S210; **S211: KQ51 a — F4, KQ52 a — Faktor 0,3**, der größte Sprung 12,8 LU wird bekannte Abweichung).
KQ51 war Patrik vorgelegt: a F4, b F3, c F2, d F1.
Empfohlen F4 — die Lage der Bäuche folgt der Feder (beide Fassungen mit 7 000 N treffen alle Profile, beide schwächeren
verfehlen S(1,3)), der größte Sprung schwankt zwischen Nachbarn um 3 LU und trennt nicht; gegen F1 liegt F4 an zweiter
Mode, Güte, Sprung und Abstrahlung näher. Kipppunkte: die Grenze 12 LU ohne bekannte Abweichung (dann F3); ein Sprung
von F4 deutlich über 12 mit dem Faktor 0,3 / 0,4. Für KQ52 liegt an F4 die Lautheit ohne Mitschwingen vor
(Spanne / größter / mittlerer Sprung): Faktor 0,2 17,6 / 12,1 / 2,7, Faktor 0,3 18,4 / 12,7 / 2,9, Faktor 0,4 19,1 /
12,7 / 2,8 LU. Mit Mitschwingen ist für 0,3 / 0,4 nichts gerechnet (ein Lauf wurde bei 4 von 67 Tasten abgebrochen).

**20.6 Faktor der Saiten an F4** (S211; Lauf wie §20.4 mit 0,3 und 0,4, je rund 65 min; Ausgaben lokal
`.claude/handover/s211_kq52_f4_<c>.log`). Verschiebung beim Besaiten gegen den Körper ohne Saiten (164,4 / 218,9 Hz,
S(1,2)–S(1,5) 241,4 / 336,6 / 441,9 / 567,1 Hz), Bell in Klammern:

| c | S(1,1)− (+10) | S(1,2) (+3) | S(1,3) (+5,7) | S(1,4) (−0,5) | S(1,5) (+1,8) | Σ Quadrate (5 / 3) | Moden mit Luft (Bell 187 / 207) | Lautheit mit Mitschwingen |
|---|---|---|---|---|---|---|---|---|
| 0,2 | +3,8 % | +4,3 % | +2,4 % | +1,4 % | +0,8 % | 56 / 51 | 170,7 / 225,6 | 18,4 / 12,2 / 2,5 |
| 0,3 | +5,3 % | +6,5 % | +3,6 % | +2,1 % | +1,2 % | 46 / 39 | 173,1 / 228,7 | 19,2 / 12,8 / 2,5 |
| 0,4 | +6,5 % | +8,7 % | +4,8 % | +2,8 % | +1,6 % | 57 / 46 | 175,1 / 231,9 | 19,6 / 12,8 / 2,5 |

Ohne Mitschwingen: 17,6 / 12,1 / 2,7 · 18,4 / 12,7 / 2,8 · 19,1 / 12,7 / 2,8 LU. Der größte Sprung liegt in allen
drei zwischen den Tasten 54 und 58. Zwischen 0,3 und 0,4 ändern sich die Orte der Bäuche um höchstens 0,2, die Höhen um
höchstens 0,6 dB. Abstrahlung an den Saiten 33–22 gegen Bell Abb. 7.23 (KQ48 a, nachgerechnet nach dem Entscheid):
ohne Saiten 2 / 12, mit 0,2 / 0,3 / 0,4 je 6 / 12; die größte Abweichung wächst mit dem Faktor, 15,3 / 18,5 / 21,0 dB
(an Saite 25). Die Zahl, die KQ48 a festhält, ist für alle drei gleich.

**KQ52 a (Patrik S211): Faktor 0,3** — die kleinste Summe der Quadrate gegen Bells fünf Verschiebungen; der
Unterschied im größten Sprung (12,8 gegen 12,2 LU bei 0,2) liegt unter der Schwankung zwischen Nachbarn. Verworfen: 0,2, 0,4.

**20.7 Der Bau** (S211; KQ51 a–KQ54 a). `make_harfe.py` schreibt F4: Umriss mit einer Ecke je Seite bei y = 0,6 m,
die Bassseiten als Randabschnitte (Auslenkung gehalten mit 10¹² N/m², wie in der Suche; Drehfeder 7 000 N), Mündung
2,3, 46 `pointSprings` mit 0,3 · E·A/L (Summe 5,394 · 10⁵ N/m), ohne Verlust. Gegen den Bauplan der Suche gleich bis
auf Quellen, Hörorte und die Federn. Der Körper (85 s, 129 Moden): mit den Federn in der Platte 173,08 / 228,74 /
257,12 / 348,54 / 451,39 / 574,05 Hz, die Suche im Modenraum 173,1 / 228,7 / 257,1 / 348,6 / 451,4 / 574,1 Hz.

Der Fall „Luftraum die mitgelieferte Harfe“ rechnet die Platte ohne Federn bis 1,5 kHz am Netz für 5 kHz (13 s;
gegen bis 5 kHz auf 0,1 dB gleich) und misst daran KQ31-2 a und KQ40 a: Moden mit Luft 164,4 / 218,9 Hz (Q 13,9 /
14,7), S(1,2)–S(1,6) −3,1 / +6,5 / +2,3 / +4,1 / +0,2 %, alle Bäuche in der Lage innerhalb von 2 Messorten (knapp:
der zweite Bauch von S(1,3) mit +1,99), bekannte Abweichungen die Höhe des stärksten Bauchs je Mode (−3,2 / −8,3 /
−9,6 / −6,8 / −8,6 dB) und sechs Höhen an S(1,5) / S(1,6) (−7,9 / −10,2; −6,1 / −7,7 / −14,5 / −15,3 dB), S(1,8)
−16,9 dB. Mit den Federn: die Verschiebung beim Besaiten (+5,3 / +6,5 / +3,6 / +2,1 / +1,2 %) und die Abstrahlung an
den Saiten 33–22 mit den 16 Hörorten von KQ36 a (KQ54 a).

**Volle Rechnung gegen Modenraum.** Die Abstrahlung am gebauten Körper weicht an den Basssaiten bis 5 dB von der Suche
ab (Saite 33 … 22, Modell − Bell): −5,1 / −4,0 / +5,0 / +10,0 / +16,8 / +15,2 / −6,9 / 0 / +20,3 / +9,6 / −1,4 / −2,8 dB
gegen −9,2 / −8,3 / +0,7 / +5,3 / +11,5 / +10,3 / −3,9 / 0 / +18,5 / +8,6 / −3,3 / −5,0 dB; beide halten 6 / 12.
Formen, Frequenzen und Werte an den Ösen sind gleich; verschieden sind Luftanteil und Güte der beiden Moden mit Luft:
0,65 / 0,34 und Q 11,9 / 19,0 mit den Federn in der Platte, 0,50 / 0,45 und Q 15,1 / 16,7 im Modenraum. Der Modenraum
führt Luftanteil und Dämpfung nur auf der Diagonale (die Kreuzglieder zwischen den Moden fehlen); mit 409 statt 129
Moden ändert sich nichts (Gegenprobe). Die volle Rechnung gilt. **Folge:** Die Lautheit je Taste mit Saiten in §20.2,
§20.4 und §20.6 (auch die 12,8 LU von KQ51 a / KQ52 a) ist am Modenraum gemessen; die Verschiebungen beim Besaiten und
alles ohne Saiten nicht betroffen.

**Lautheit am gebauten Körper** (S211, KQ39 a, 67 Tasten 38–104, 16 Hörorte, 55 min; Ausgabe lokal
`.claude/handover/s211_gebaut.log`): nur die gespielte Saite 17,6 / 15,6 / 2,7 LU, mit Mitschwingen 18,1 / **15,7** / 2,3 LU
(im Modenraum 18,4 / 12,7 / 2,8 und 19,2 / 12,8 / 2,5). Der größte Sprung liegt wie dort zwischen den Tasten 54 und
58: −23,4 → −7,7 LUFS (Modenraum −20,7 → −8,0) — Taste 54 (185 Hz) liegt zwischen den beiden Moden mit Luft und wird
mit der richtigen Verteilung der Luft 2,7 LU leiser. **Damit greift der Kipppunkt von KQ51 a** (der Sprung von F4 mit
dem Faktor nach KQ52 deutlich über 12 LU): KQ51 ist neu vorzulegen, und die Lautheit der übrigen Fassungen F1–F3 ist
ebenfalls am Modenraum gemessen.

**20.8 F1–F3 an der vollen Rechnung, Probe an den Tasten 50–62** (S212; Saiten mit 0,3 als Federn in der Platte,
16 Hörorte, KQ39 a; Läufe lokal `.claude/handover/s211_fassung_lauf.sh`, `s212_varianten_lauf.sh`, Ausgaben
`s211_voll_*.log`, `s212_var_*.log`). Je Fassung rund 70 min, 67 Tasten 38–104:

| Fassung | Moden mit Luft (Bell 170 / 216 mit Saiten) | S(1,2) / S(1,3) / S(1,4) / S(1,5) / S(1,6) | Abstrahlung 33–22 | ohne Mitschwingen | mit Mitschwingen |
|---|---|---|---|---|---|
| F1 | 176,3 / 231,2 Hz (Q 13 / 17) | +3,3 / +10,3 / +4,5 / +5,3 / +0,9 % | 7 / 12, größte 21,8 dB | 17,7 / 15,1 / 2,7 | 18,3 / 15,2 / 2,4 |
| F2 | 176,3 / 231,3 Hz (Q 15 / 15) | +2,5 / +9,8 / +4,1 / +5,0 / +0,5 % | 7 / 12, 20,2 dB | 17,5 / 14,0 / 2,8 | 18,1 / 14,0 / 2,4 |
| F3 | 175,3 / 229,6 Hz (Q 15 / 15) | +3,1 / +9,9 / +3,6 / +4,9 / +0,3 % | 7 / 12, 18,6 dB | 17,1 / 13,2 / 2,7 | 18,4 / 13,2 / 2,4 |
| F4 (§20.7) | 173,1 / 228,7 Hz | | 6 / 12 | 17,6 / 15,6 / 2,7 | 18,1 / 15,7 / 2,3 |

Alle vier verfehlen den größten Sprung 12 LU, immer zwischen den Tasten 54 (−22,4 bis −23,4 LUFS) und 58 (−7,7 bis
−9,2 LUFS). Taste 58 (233 Hz) sitzt auf der zweiten Mode mit Luft; Bell misst diese Mode bei 216 Hz, an Taste 57.
**An der vollen Rechnung liegt S(1,3) in allen Fassungen 9,8–10,3 % zu hoch** — die Suche im Modenraum hielt alle
Lagen auf 7 %.

**Probe KQ55 a** (Patrik S212): F3, je ein Hebel, Lautheit nur an den Tasten 50–62 (je rund 13 min). Größter Sprung
mit Mitschwingen im Fenster (F3 selbst 13,2 LU, 54 → 58):

| Hebel | Moden mit Luft | S(1,2) / S(1,3) | Abstrahlung | Sprung 54 → 58 | größter im Fenster |
|---|---|---|---|---|---|
| Saiten 0,2 | 171,4 / 226,4 Hz (+1 / +5 %, Q 16 / 14) | +1,4 / +8,7 % | 7 / 12, 17,2 dB | 12,02 | 12,0 |
| Saiten 0,4 | 178,7 / 232,7 Hz (+5 / +8 %) | +4,9 / +11,2 % | 6 / 12, 21,3 dB | 13,3 | 13,5 (55 → 58) |
| Güte der Luft 5,5 | 175,3 / 229,6 Hz (Q 10 / 10) | +3,1 / +9,9 % | 6 / 12, 17,7 dB | 12,6 | 12,6 |
| Güte der Luft 12 | 175,3 / 229,6 Hz (Q 20 / 20) | +3,1 / +9,9 % | 7 / 12, 20,0 dB | 13,6 | 13,6 |
| Mündung 1,8 | 178,8 / 235,0 Hz (+5 / +9 %) | +3,3 / +10,0 % | 6 / 12, 19,6 dB | 11,8 | 12,3 (55 → 59) |
| Mündung 2,2 | 171,6 / 225,3 Hz (+1 / +4 %) | +3,0 / +9,9 % | 6 / 12, 16,5 dB | 13,0 | 13,0 |

Kein Hebel bringt das Paar unter 12 LU, jeder verschiebt den Sprung um höchstens 1,2 LU. Am nächsten kommt der Faktor
0,2 mit 12,02 LU. Er verbessert dabei auch die Lagen und die Abstrahlung, verfehlt aber Bells Verschiebung beim
Besaiten deutlicher (KQ52 a, §20.6). Die Mündung 1,8 verschiebt den Sprung nur auf das Paar 55 → 59 und legt die zweite
Mode mit Luft 9 % zu hoch. Eine kleinere Güte der Luft ebnet die Kurve, nimmt aber KQ29-1 a zurück. Damit trägt kein
Hebel am Bauplan F1–F4 die Schwelle. Ursachen in der Struktur sind die Lage der zweiten Mode mit Luft (+4 bis +9 %
gegen Bell) und S(1,3) (+9 bis +11 %).

**KQ56 a (Patrik S212): der Bauplan wird in einer eigenen Sitzung mit Fable neu gedacht**, gesucht an der vollen
Rechnung (Federn in der Platte), mit den Zielen zweite Mode mit Luft bei 216 Hz, S(1,3) auf 7 % und das Paar 54–58
unter 12 LU; F4 bleibt bis dahin gebaut. Verworfen: Faktor 0,2 an F3 voll rechnen.
