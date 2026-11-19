# Changelog Session 208 (2026-11-19)

> Diese Sitzung hat geklärt, warum die Harfe am Übergang von Metall- zu Darmsaiten springt: Die Saiten sind es nicht,
> der Körper strahlt unter 185 Hz und um S(1,2) zu stark. Außerdem ist der Bericht der Research-Werkbank zur Oud
> abgeholt und gegengelesen. Kein Programmcode.

## Neu

- **KQ39 a:** KQ34 a misst die Harfe mit Mitschwingen nach GZ8 a.
- **KQ40 a:** Der Harfenkörper bekommt zwei weitere Ziele:
  - die Lage von S(1,1)± auf 7 %;
  - die Abstrahlung an den Saiten 33–22 gegen Bell 1988 Abb. 7.23 auf 6 dB.

  Die Ursache wird im Bauplan gesucht.
- **KQ41 a:** Ziel 2 von KQ38 a (Oud) gilt im Band 1,5–2,0: die Hauptspitze der Abstrahlung liegt beim 1,5- bis
  2,0-Fachen der Luftmode.
- Proben `docs/modellierung/gezupfte-saite/proben/2026-11-19_S208_*`:
  - Zerlegung der Harfe am Übergang;
  - Mitschwingen mit 20 Saiten;
  - Abstrahlung gegen Bell und Le Carrou.
- Bericht der Research-Werkbank `docs/modellierung/gezupfte-saite/recherche/2026-11-19_S208_oud_basschor_werkbank.md`,
  mit Kopf und Hilfsskripten.

## Geändert

- `2026-11-19_S207_spruenge_je_taste.md` 1.5.0 (§10 Harfe, §11 Oud-Bericht).
- `docs/audio/Instrumente_IC_Konzept.md` 0.72.0.
- `docs/Offene_Punkte.md` 1.414.0.

## Zahlen

**Harfe 43 / 47**

- Die Tasten 43 und 45 liegen bei −16,8 / −26,3 LUFS, die echte Harfe dort bei −11,6 / −8,7 LUFS.
- Gegen die Darmsaiten 52–56 ist das Metallregister im Modell 4,5 LU leiser, in der Aufnahme 1,6 LU lauter.
- Mit Mitschwingen (Spanne / größter / mittlerer Sprung): 20,9 / 13,3 / 3,1 LU.
- Abstrahlung an den Ösen:
  - gegen Bell 130–165 Hz +11 … +16 dB, S(1,2) +20 dB;
  - gegen Le Carrou 10–28 dB zu stark;
  - ab 185 Hz im Median rund 1 dB.

**Oud**

- Pegel je Chor an einer echten Oud: nicht gefunden, KQ37 a bleibt.
- Zug: C2 gegen den F2-Chor −3,9 dB.
- Teiltöne 2 und 3 des Basschors: gemessen 0 … −8 dB unter dem Maximum, im Modell −27 / −18 dB.
- Verhältnis der Spitzen: gemessen 1,5–1,7, im Modell 2,7.

**Offen:** die Bauplan-Suche für Oud (KQ38 a) und Harfe (KQ40 a), beide mit Fable.

## Tests

Keine (kein Programmcode); **1923 Cases** unverändert.
