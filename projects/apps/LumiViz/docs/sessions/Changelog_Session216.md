# Changelog Session 216 (2026-11-23)

> Diese Sitzung hat gesucht, warum die Plattenmoden der Harfe 21–69 % Luft tragen (KQ67 a). Es liegt an der Stärke der
> Kopplung zwischen Platte und Luft, nicht an der Lage der Luftmoden; die Ursache der zu starken Kopplung ist nicht
> belegt, übrig bleibt die Annahme starrer Wände. Die Schale wird als elastisches Bauteil geprüft (KQ68 a). Kein
> Programmcode.

## Neu

- **KQ68 a:** die Schale der Harfe wird als elastisches Bauteil geprüft, zuerst als Probe — der Querschnitt als Ring aus
  Boden und Schale, gegen Bells Hologramme (Abb. 5.3–5.19), die Versteifung der ersten Bodenmode 25 Hz, das Paar lose /
  in der Harfe und die Güten; danach die Vorlage, ob und wie die Schale in `InstrumentBody` kommt. Die Harfe bleibt
  gebaut. Noch nicht begonnen.
- **KQ69 a:** die gegenseitige Belastung der Luftpfropfen der Öffnungen (Massenmatrix aus der Geometrie statt der
  angepassten Mündung) bleibt Probe und wird mit dem Ergebnis von KQ68 vorgelegt; bis dahin gilt KQ64 a.
- Proben `modellierung/gezupfte-saite/proben/2026-11-23_S216_harfe_oeffnungen_gegenseitig.py` (Rohr allein mit der
  Massenmatrix der Öffnungen gegen Bell Kap. 4; `rohr`, `raster`) und `…_harfe_kasten_balken.py` (der Kasten als
  Balken mit starren Querschnitten, frei oder an den Enden gehalten; `raster`).

## Geändert

- Harfenbericht `modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §24.
- `docs/audio/Instrumente_IC_Konzept.md` 0.85.0, `docs/Offene_Punkte.md` 1.427.0.

## Zahlen

**Aus der Quelle (am Seitenbild):** Bell misst je Luftmode sechs Zustände (Abb. 4.8, 4.11–4.15; die Öffnungen vom
Diskant her eine nach der anderen geöffnet) — 36 Werte, Tabelle im Harfenbericht §24.1; ein Wert (A1 mit den Öffnungen
3–6) ist am Bild nicht sicher. Kap. 5 (Hologramme am losen Kasten A): die Rückseite bewegt sich in jeder Mode; bei
S(1,1)− gehen Boden und Rückseite in der Mitte in dieselbe Richtung, bei S(1,1)+ beide nach innen, die Enden jeweils
dagegen; die tiefen Moden sind „whole body resonances“ (S. 119).

**Das Rohr allein gegen Bell Kap. 4:**

| | nur 6 offen | alle offen | 12 Werte rms / größte | 36 Werte rms |
|---|---|---|---|---|
| gebaut (Mündung 1,83) | −8 / 0 / 0 / −2 / −2 / −3 % | 0 / +3 / −12 / −13 / −9 / −6 % | 6,6 / 13 % | 7,2 % |
| Massenmatrix, Öffnungen in einer Wand | −4 / +1 / 0 / −2 / −3 / −3 % | +1 / +8 / −4 / −9 / −8 / −6 % | 5,1 / 9 % | 6,0 % |

Mit der Massenmatrix liegt die Helmholtz-Mode mit allen Öffnungen ohne Anpassung bei 192 Hz (Bell 190).

**Gekoppelt (Platte im Vakuum aus S213, Güten 4,4 / 28), Luftanteil und Güte je Plattenmode:**

| | S(1,1)+ | S(1,2) | S(1,3) | S(1,4) | S(1,5) |
|---|---|---|---|---|---|
| gebaut | 0,38, Q 13 | 0,21, Q 18 | 0,52, Q 15 | 0,48, Q 21 | 0,44, Q 24 |
| Massenmatrix, alle Rohrmoden × 1,10 | 0,38, Q 12 | 0,14, Q 22 | 0,30, Q 14 | 0,53, Q 14 | 0,36, Q 22 |
| Massenmatrix, Kopplung Platte–Luft × 0,6 | 0,30, Q 14 | 0,08, Q 30 | 0,12, Q 28 | 0,30, Q 22 | 0,44, Q 23 |
| Bell besaitet (ohne Saiten) | – (16) | Q 32 (39) | Q 27 (45) | Q 28 (43) | Q 19 (31) |

Mit Kopplung × 0,6 liegt das Paar bei 183 / 212 Hz (Ziel 187 / 207). Lautheit, Abstrahlung und Profile sind dafür
nicht gerechnet.

**Versteifung der ersten Bodenmode durch die Luft** (Bells Zwei-Massen-Rechnung am Paar): Kasten A lose 25 Hz (22–27),
Modell 54 Hz (ein Druck), 42 Hz (Rohr mit Massenmatrix).

**Kasten als Balken mit starren Querschnitten** (2 / 4 / 8 kg, erste Biegefrequenz 150 … 5000 Hz, frei / gehalten;
Massen und Steife angenommen): obere Mode des Paars 235–270 Hz statt 217 Hz, Versteifung 46–51 Hz, frei gegen
gehalten unter 10 Hz (Bell +16 Hz) — in dieser Form nicht bestätigt.
