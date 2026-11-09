# Befund S182: Lautheit über den Umfang — Wurlitzer, Kalimba, Clavinet

> **Datum:** 2026-11-09 (Session 182) · **Anlass:** Patrik hört in `bars_demo` die Stabspiele kaum, das Reed EP
> übersteuert (Takt 5–8) · **Entscheid:** LP1 a (Ursache klären, an ihrer Stelle beheben, Wächter erweitern)
> **Stand:** Ursache für das Wurlitzer belegt (Probe), für die Kalimba aus dem Code, für das Clavinet offen

## 1 Messung

Messweg: je Instrument ein Loop mit einem Ton je Takt (120 bpm, 2 s je Ton), alle 3 Halbtöne über den Umfang, Stärke
0,7, ein Viertel lang, Loop und Muster mit Lautstärke 1; gerechnet über `LUMIVIZ_LOOP_WAV` (Release-clang), gemessen
nach ITU-R BS.1770-4 je 2-s-Abschnitt (absolutes Gate −70 LUFS, ohne relatives Gate). Skripte im Scratchpad der
Sitzung (`pegel_proben.py`, `varianten.py`), nicht im Repo.

Mit der Standardprobe (Tonleiter und Akkord in einer Lage) liegen alle 15 Instrumente mit Kern auf −13,4 LUFS — der
Wächter „Loop Pegel 27c“ ist grün und sieht das Gefälle nicht.

| Instrument | Spanne über den Umfang | Verlauf |
|---|---|---|
| reed_electric_piano (IC Wurlitzer) | 37,6 LU | Taste 33–54 −14 … −9 LUFS, Spitze 1,0 (übersteuert); fallend bis −46 an Taste 96 |
| kalimba | 33,7 LU | Taste 50 −54, steigend bis −20 an Taste 86 |
| clavinet_modal | 23,8 LU | Taste 29 −40, steigend bis −16 |
| upright, grand, honky-tonk, glockenspiel, marimba, xylophone, celesta, CP-70 | 14–21 LU | meist ganz oben leiser (kurze Töne) |
| tine_electric_piano, vibraphone | 9–10 LU | fast eben |
| timpani, tubular_bells | 2 LU | schmaler Umfang |

`bars_demo` je Kanal (nur dieser Kanal, Lautstärken des Loops): Reed piano −15,5 LUFS (Spitze 1,0), die übrigen
−32,8 … −44,4 LUFS.

## 2 IC hat dasselbe Gefälle

IC `hoertest/listen/*/liste.json`, Messwert `lufs_vorher` (vor dem Angleichen auf −23 LUFS), Stärke a-12:

| IC-Instrument | tief | Mitte | hoch |
|---|---|---|---|
| Wurlitzer | H1 −22,8 | E4 −46,7 | Ais6 −300 (stumm) |
| Kalimba | E3 −59,7 | Gis4 −43,8 | C6 −27,2 |
| Clavinet | A1 −36,6 | Ais3 −25,7 | C6 −24,1 |

Die Ursache liegt in den Vorgaben bzw. im Modell von IC, nicht in der Übernahme (die Abdrücke an der mittleren Taste
sind Bit für Bit gleich).

## 3 Ursachen

### 3.1 Wurlitzer — belegt durch Probe

Vorgabe (`stab/Vorgaben.cpp`, `baueVorgabeWurlitzer`, aus IC): Masse der Zunge **fest 2 g** über alle 64 Tasten
(„offen: 2 g (Einschaetzung, Stahlzunge)“), **ein** Filzhammer für alle Tasten (Stulov wie der Flügel an Taste 40,
„uebertragen“), elektrostatischer Tonabnehmer 3 mm.

Probe mit Varianten der `.lvinst` (je eine Änderung, ohne neu zu stimmen — es zählt die Spanne):

| Variante | Spanne | tief | hoch |
|---|---|---|---|
| A heute | 37,6 LU | −14 (übersteuert) | −46 |
| B Hammer 30-mal steifer | 27,1 LU | −14 (übersteuert) | −36 |
| C Masse je Taste wie die Tine des Rhodes (25 g → 3 g) | 15,7 LU | −34 | −50 |
| D = B + C | **5,0 LU** | −34 | −39 |

- Die feste, leichte Zunge ist im Bass zu nachgiebig: sie schwingt so weit aus, dass sie den Tonabnehmer fast
  berührt (Spitze 1,0) — das übersteuerte Bass-Drittel.
- Der eine weiche Hammer hat einen Kontakt, der lang gegen die Periode der hohen, steifen Zungen ist: der Diskant
  wird kaum angeregt (IC: Ais6 stumm).
- C und D sind **Proben**, keine Werte: die Masse des Rhodes ist aus der Länge der Tine gerechnet (Falaize & Hélie),
  für Zungen des Wurlitzer gibt es in der Vorgabe keine Quelle.

### 3.2 Kalimba — aus dem Code

`Stab.cpp`, Zupfen: die Zunge wird statisch um δ = `velocityMax` mm ausgelenkt (bei jeder Taste gleich, 1 mm), die
Ausgabe ist akustisch die **Beschleunigung**. Bei fester Auslenkung wächst sie mit ω²: über drei Oktaven
(Taste 50–86) +36 dB, gemessen +33,7 LU. Masse fest 3 g („Einschaetzung“). Ob ein Spieler eher gleiche Auslenkung
oder gleiche Kraft gibt, und wie die Zungen über den Umfang gebaut sind, ist ohne Quelle.

### 3.3 Clavinet — offen

Vorgabe: Saitenlänge fest 0,5 m, Saitenmasse fest (relativ 1), Tonabnehmer an festen Stellen. Vermutet ist dasselbe
Muster (feste Bauwerte über den Umfang); nicht geprüft.

## 3a Maß für einen Wächter

Die integrierte Lautheit je 2-s-Abschnitt bestraft Töne, die schnell ausklingen (hohe Klaviertöne). Das
**Momentary-Maximum** je Ton (lautester 400-ms-Block) misst den Anschlag selbst. Spanne über den Umfang so gemessen:

| Spanne | Instrumente |
|---|---|
| 2–13 LU | timpani 2,3 · tubular_bells 2,6 · tine_electric_piano 7,8 · vibraphone 8,4 · electric_grand_piano 9,2 · honky_tonk_piano 10,4 · celesta 11,2 · marimba 12,2 · upright_piano 12,6 · grand_piano 12,7 |
| 17–20 LU | glockenspiel 17,3 (Einbruch an Taste 103) · xylophone 19,5 (Einbruch an Taste 107) |
| über 20 LU | clavinet_modal 22,0 · reed_electric_piano 34,3 · kalimba 37,4 |

Wurlitzer-Varianten im selben Maß: A 34,3 · B 22,9 · C 13,7 · D 2,4 LU.

## 4 Folgerung

Die drei Gefälle kommen aus Bauwerten, die über den Umfang **fest** gesetzt sind, wo das echte Instrument sie je
Taste abstuft — alle mit Belegkraft *offen* (Einschätzung). Die Berichtigung gehört in die Vorgaben von IC (SSOT der
Rechenkerne, die Abdrücke Bit für Bit), mit Quellen je Taste. Für LumiViz: ein Wächter der Spanne über den Umfang je
Instrument.
