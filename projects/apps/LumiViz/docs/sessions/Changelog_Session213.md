# Changelog Session 213 (2026-11-22)

> Diese Sitzung hat die Prämissen von KQ56 a geprüft. Die Ziele aus S212 verglichen die besaitete Rechnung mit Bells
> Werten ohne Saiten; gegen die besaiteten liegt nur das Paar mit Luft daneben. Ursache ist nicht der Bauplan: das
> Modell koppelt den Boden drei- bis viermal zu stark an die Luft. Kein Programmcode.

## Neu

- **KQ57 a:** die Lagen des gebauten Körpers der Harfe gelten gegen Bell besaitet (187 / 207 / 257 / 334 / 430 / 555 /
  697 Hz, je 7 %); die Werte ohne Saiten bleiben Bericht.
- **KQ58 a:** erst die Ursache der zu starken Kopplung an den Quellen klären (Bell Kap. 5 und 7, Le Carrou 2006
  Kap. 3–4; drei Kandidaten je mit Probe), dann das Modell erweitern. F4 bleibt gebaut.
- Proben `modellierung/gezupfte-saite/proben/2026-11-22_S213_harfe_kopplung.py` (rechnet `Luftraum.hpp` aus den Moden
  der Platte im Vakuum nach, mit Faktor an der Kopplung; schreibt einen Körper für die Lautheit) und
  `…_harfe_bauch.py` (der Boden als flache Schale unter dem Saitenzug).

## Geändert

- Harfenbericht `modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §21.
- `docs/audio/Instrumente_IC_Konzept.md` 0.82.0, `docs/Offene_Punkte.md` 1.424.0.

## Zahlen

**Der gebaute Körper (F4) gegen Bell besaitet:** 173,1 / 228,7 / 257,1 / 348,5 / 451,4 / 574,1 Hz gegen 187 / 207 /
257 / 334 / 430 / 555 Hz: −7,4 / +10,5 / 0,0 / +4,4 / +4,9 / +3,4 %, S(1,6) +2,6 %.

**Versteifung des Bodens durch die eingeschlossene Luft** (zwei gekoppelte Schwinger): Bell ohne Saiten 46 Hz, Bell
besaitet 14,5 Hz, Le Carrou 2007 besaitet 12–15 Hz, Le Carrou 2006 (Matrizen) 14 Hz, Modell F4 49 Hz. Verhältnis des
Paars: 1,27 / 1,11 / 1,12 / 1,10 / 1,32.

**Probe mit abgeschwächter Kopplung** (bei Faktor 1 trifft sie den gebauten Körper auf 0,012 % in der Frequenz):

| Faktor, Mündung | Paar gegen 187 / 207 Hz | Abstrahlung 33–22 | größter Sprung 50–62 mit Mitschwingen |
|---|---|---|---|
| 1, 2,3 (gebaut) | −7,4 / +10,5 % | 6 / 12 | 15,7 LU (54 → 58) |
| 0,5, 2,3 | −0,7 / +3,7 % | 6 / 12 | nicht gerechnet |
| 0,35, 2,3 | +1,3 / +1,7 % | 3 / 12 | 19,0 LU (52 → 56) |

**Flache Schale unter dem Saitenzug** (Stich 1–6 mm): erste Mode 204 → 226 Hz, ihr Volumen 0,148 → 0,147 m³/√kg —
verworfen.

**Bell Abb. 7.23 am Seitenbild:** der Schalldruck des Grundtons steigt zwischen den Saiten 28 und 27 (165 → 185 Hz)
um 18 dB.
