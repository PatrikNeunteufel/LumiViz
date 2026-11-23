# Changelog Session 217 (2026-11-23)

> Diese Sitzung hat geprüft, ob eine elastische Schale die Kopplung des Bodens der Harfe an die Luft schwächt (KQ68 a).
> Sie tut es nicht. Dafür ist die Prämisse dahinter gefallen: für den gehaltenen Kasten ohne Saiten trifft das Modell
> Bells Paar auf 3 %; die enge Lage des Paars an den losen Kästen kommt zum größeren Teil von der Biegung des Kastens,
> die besaitete Lage ist ungeklärt (KQ70 a). Die Güten der Luftmoden folgen aus der Abstrahlung der Öffnungen (KQ71 a).
> Kein Programmcode.

## Neu

- **KQ70 a:** die Prämisse „das Modell koppelt den Boden zu stark an die Luft“ entfällt, die Kopplung Platte–Luft bleibt
  wie gerechnet. Die Harfe wird in zwei Zuständen gemessen: ohne Saiten gegen Bell Kasten G in der Harfe ohne Saiten
  (170 / 216 / 249 / 316 Hz), besaitet weiter nach KQ57 a. Die Abweichung des Paars im besaiteten Zustand heißt
  „Wirkung des Besaitens“ und wird als eigene Frage geprüft (Rahmen der Harfe, Saiten zwischen Boden und Hals) — als
  Probe, mit Lautheit, Abstrahlung und Profilen vor jeder Vorlage. Die Harfe bleibt gebaut.
- **KQ71 a (löst KQ69 a ab):** für Massen- und Strahlungsmatrix der Öffnungen werden zuerst die Zielgrößen gerechnet
  (Lautheit der Tasten 50–62, Abstrahlung 33–22, Güten ohne und mit Saiten, Wächter gegen Bells 36 Werte), dann wird
  der Bau vorgelegt; bis dahin gelten KQ63 a und KQ64 a.
- Proben unter `modellierung/gezupfte-saite/proben/`:
  `2026-11-23_S217_harfe_ring.py` (der Querschnitt als Ring aus Boden und Schale),
  `…_harfe_kasten_balken.py` (der Kasten als Balken mit der Biegesteife aus der Geometrie; `raster`, `formen`),
  `…_harfe_daempfung_komplex.py` (Güten nach Energieanteilen gegen komplexe Eigenwerte; `strahlung`: Abstrahlung der
  Öffnungen als Matrix), `…_harfe_hals.py` (das obere Ende der Saiten als ein Schwinger).

## Geändert

- Harfenbericht `modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md`: §25 neu, §24.6 berichtigt (die
  Balken-Probe aus S216 hatte eine rund 10⁴-mal zu kleine Biegesteife).
- `docs/audio/Instrumente_IC_Konzept.md` 0.86.0, `docs/Offene_Punkte.md` 1.428.0.

## Zahlen

**Ring (eine Scheibe des Kastens ohne Halt längs, Breite 0,40 / 0,30 / 0,20 m):** Versteifung der Bodenmode durch die
eingeschlossene Luft mit starrer Schale 48 / 51 / 54 Hz, mit elastischer Schale (2 / 5 / 10 GPa) und gehaltenen Rändern
dasselbe; Helmholtz-Mode durch die nachgiebige Schale × 0,995–0,999. Boden biegesteif statt gelenkig am Rand:
41 / 43 / 45 Hz.

**Platte im Vakuum ohne Saiten** (neu gerechnet, Bauplan F4 ohne Federn): 186,2 / 240,3 / 334,5 / 441,9 / 566,6 Hz.

**Der Kasten als Balken, Platte ohne Saiten, Luft als Rohr mit Massenmatrix:**

| | Moden bis 300 Hz | Öffnungen zu | „Versteifung“ nach Bells Rechnung |
|---|---|---|---|
| Bell Kasten A lose | 176 / 201 / 250 | 191 | 26 Hz |
| Bell Kasten F lose | 185 / 204 / 250, S(1,2)′ 284 | 196 | 24 Hz |
| starrer Rahmen (heute) | 165,2 / 210,8 / 232,9 | 191 | 46 Hz |
| frei, erste Biegemode des Kastens 260 Hz | 170,0 / 207,7 / 239,9 / 288,6 | 195 | 37 Hz |
| frei, 217 Hz (Steife × 0,7) | 167,2 / 197,2 / 238,7 / 264,6 | 183 | 30 Hz |
| frei, 221 Hz (Masse × 1,5) | 166,8 / 199,4 / 237,1 / 257,4 | 185 | 32 Hz |

Steife und Masse des Kastens sind angenommen.

**Der gehaltene Kasten ohne Saiten gegen Bell Kasten G in der Harfe ohne Saiten:** 165,2 / 210,8 / 232,9 / 319,7 Hz
gegen 170 / 216 / 249 / 316 Hz (−2,8 / −2,4 / −6,5 / +1,2 %), Abstand des Paars 46 gegen 46 Hz. Besaitet: Bell
187 / 207 Hz, Modell 174,3 / 217,5 Hz.

**Güten der Luftmoden mit allen Öffnungen** (Strahlungswiderstand als Matrix, innerer Verlust aus Bells Güten mit nur
Öffnung 6 offen, kein angepasster Wert):

| | Helmholtz | A1 | A2 | A3 | A4 | A5 |
|---|---|---|---|---|---|---|
| Bell | 6 | 6 | 7 | 10 | 15 | 30 |
| Matrix, Wand | 4,5 | 5,9 | 7,7 | 13,1 | 20,9 | 26,9 |
| Matrix, freier Raum | 6,3 | 8,4 | 11,4 | 17,5 | 24,9 | 30,6 |

**Güten der Plattenmoden S(1,2)–S(1,5):** ohne Saiten nach Energieanteilen 19 / 17 / 11–15 / 19–22, mit komplexen
Eigenwerten 19 / 20 / 11–15 / 19–25, mit der Strahlungsmatrix 19–22 / 21–24 / 12–19 / 19–29 (Bell 39 / 45 / 43 / 31);
besaitet mit der Strahlungsmatrix (freier Raum) 25 / 25 / 16–21 / 23–30 (Bell 32 / 27 / 28 / 19, gebaut 18 / 15 / 17 /
24).

**Hals als ein Schwinger** (1 / 3 kg, 200–400 Hz, Federn × 1 / 2 / 3,33): untere Mode 171–183 Hz, obere 207–234 Hz;
187 / 207 Hz erreicht kein Fall.

Lautheit der Tasten 50–62, Abstrahlung 33–22 und Profile sind für keine der Proben gerechnet.
