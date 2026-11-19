# Changelog Session 209 (2026-11-19)

> Diese Sitzung hat den Bauplan der Oud neu gesucht und gebaut und für die Harfe einen Kandidaten gefunden: mit dickerem
> Boden am Bass, eingespanntem Bassende und den Saiten als Federn hält sie erstmals die Lautheit je Taste. Die Harfe ist
> noch nicht gebaut.

## Neu

- **KQ42 a (Oud):** Lesart der Messung Abdallah 2026 Abb. 6 — 139 Hz ist die Mode der Luft, 203 Hz die erste Deckenmode;
  die Spitze bei 101 Hz hat kein Gegenstück. Die Balken reichen bis 2 mm vor den Rand, der Steg trägt Masse und Steife.
- **KQ43 a (Oud):** Die Lagen werden mit dem Aufnehmer der Messung verglichen (5,8 g am Antwortpunkt 3, im Modenraum
  gerechnet); der Bauplan beschreibt das Instrument ohne ihn.
- **KQ44 a (Oud):** Die Rosetten gehen als Masse am Lochrand in den Bauplan, je Loch ein weicher Ring als Leiste.
- **KQ45 a (Oud):** Gebaut ist die Fassung mit den besten Lagen; Ziel 2 und KQ36 a bleiben bekannte Abweichung.
- **KQ46 a (Harfe):** Der Körper der Harfe ist die besaitete Harfe — Federn an Punkten im Bauplan (Fassung 9,
  `pointSprings`). Noch nicht gebaut.
- **KQ47 a (Harfe):** Bauplan mit Boden 12 → 2 mm, eingespanntem Bassende und Mündung um 2,0, nach neuer Eichung des
  Holzes fein gesucht. Noch nicht gebaut.
- **KQ48 a (Harfe):** Die Abstrahlung an den Saiten 33–22 gegen Bell Abb. 7.23 bleibt bekannte Abweichung mit Wert.
- Suchgerüste mit allen Ausgaben: `docs/modellierung/gezupfte-saite/proben/2026-11-19_S209_oud_suche/` und
  `…/2026-11-19_S209_harfe_suche/`; Messung der Oud nach KQ36 a `…/2026-11-19_S209_kq36_oud*`.

## Geändert

- `asset/loop/instrument_bodies/make_oud.py`, `oud.lvinstbodyplan`, `oud.lvinstbody` (194 Moden bis 5 kHz): Decke
  2,23 mm, Steg 17 cm × 25,8 × 8,6 mm (24,5 g), Balkenenden 2 mm vor dem Rand, Mündung 1,52, Rosetten 11,4 + 1 + 1 g.
- Test „Luftraum die mitgelieferte Oud (KQ10 a, KQ42 a bis KQ45 a)“ neu gefasst: Balkenenden, Steg, Rosetten, Mündung,
  fünf Lagen auf 1 %, Stegbewegung von Luft- und Deckenmode, Ziel 3.
- `asset/loop/instrument_bodies/README.md` (Zeile und Absatz der Oud).
- `2026-11-19_S207_spruenge_je_taste.md` 1.6.0 (§12 Oud), `2026-11-13_S197_harfe_gelesen.md` (§17 Harfe).
- `docs/audio/Instrumente_IC_Konzept.md` 0.74.0, `docs/Offene_Punkte.md` 1.416.0.

## Zahlen

**Oud (gebaut)**

| Zielgröße | vor S209 | neu |
|---|---|---|
| Moden des Instruments | 99,9 / 168,6 / 207,8 / 269,4 Hz | 141,1 / 214,1 / 274,7 / 372,5 / 389,2 Hz |
| mit Aufnehmer gegen 139 / 203 / 280 / 362 / 385 Hz | — | +0,3 / +0,2 / −1,9 / −3,0 / +1,1 % |
| Ziel 3 (Hauptspitze / Luftmode, Band 1,5–2,0) | 2,72 | 1,51 |
| Basschor T2 / T3 unter dem Maximum unter 300 Hz | −32,8 / −11,6 dB | −9,4 / −0,3 dB |
| Ziel 2 (Rastermittel) | −16,9 dB | −4,4 dB, verfehlt |
| KQ36 a: Spanne / größter / mittlerer Sprung | 18,8 / 14,7 / 1,8 LU | 19,8 / 14,3 / 2,9 LU, verfehlt |

**Harfe (Kandidat, nicht gebaut)**

| Zielgröße | Bauplan heute | Kandidat | Kandidat mit Saiten |
|---|---|---|---|
| Moden mit Luft gegen 170 / 216 Hz | 134 / 198 | 164 / 222 | 171 / 225 |
| S(1,2)–S(1,6) | −6,4 / +4,0 / +0,3 / +2,5 / −1,4 % | +1,7 / +5,9 / +0,3 / +3,2 / +0,7 % | — |
| bekannte Abweichungen nach KQ31-2 a | 8 | 4 | — |
| Abstrahlung Saiten 33–22 (6 dB) | 4 / 12 | 6 / 12 | 6 / 12 |
| Lautheit je Taste, ohne Mitschwingen (≤ 22 / 12 / 3 LU) | 20,0 / 12,8 / 3,0 | 16,8 / 13,0 / 2,7 | 17,8 / 10,5 / 2,8 |

Die Saiten als Federn (E·A/L mal 0,2) heben am heutigen Körper S(1,1)− um 9 % und S(1,2) um 4 %; Bell misst beim
Besaiten +10 % und +3 %.

## Tests

`*Loop Koerper*` ohne `*KQ18 Drehfeder*,*KQ20*`: 75 / 75 unter Release-clang und Testing-MSVC. Den Oud-Körper liest
kein Instrument und kein Loop, deshalb kein voller Lauf. **1923 Cases** (unverändert, ein Fall umbenannt).

## Offen

- Harfe bauen: Bauplan-Fassung 9 mit `pointSprings`, Eichung des Holzes mit Boden 12 → 2 mm, Feinsuche, Körper,
  KQ31-2 a und Lautheit mit Mitschwingen (KQ39 a).
- Oud: Ziel 2 (die gegengleiche Mode um 373 Hz ist am Antwortpunkt 3 rund 13 dB zu stark) und KQ36 a.
