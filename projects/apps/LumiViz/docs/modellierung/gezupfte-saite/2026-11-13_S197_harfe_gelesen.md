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

**Folge für KQ18 a:** der Block `rim` bleibt nötig (die Vorgabe ist eingespannt), die Harfe bekommt aber
`support: elastic` mit `rotationalStiffness: 0` — gestützt. Die Federwerte aus §9 und §10 sind hinfällig.
