# Changelog Session 210 (2026-11-20)

> Diese Sitzung hat die Federn an Punkten in den Bauplan der Körper gebaut und den Bauplan der Harfe neu gesucht: der
> Boden bleibt wie bei Bell gezeichnet, die fehlende Steife im Bass kommt aus einer Drehfeder am Rand. Vier Fassungen
> sind durchgerechnet; welche gebaut wird, ist noch nicht entschieden. Die Harfe im Programm ist unverändert.

## Neu

- **KQ46 a gebaut:** `.lvinstbodyplan` Fassung 9 mit `pointSprings` (Ort, Steife, Verlustfaktor, Quelle; immer
  geschrieben). `InstrumentBody` 0.25.0. Fall „Loop Koerper KQ46 Punktfedern“.
- **KQ49 a:** Die Energie der Federn zählt in der Dämpfung mit, jede Feder trägt ihren Verlustfaktor.
- **KQ50 a (hebt KQ47 a auf):** Der Boden der Harfe bleibt 10 → 3 mm mit dem Holz aus S197. Die Steife im Bass kommt
  aus einer Drehfeder an den Bassseiten als Randabschnitt. Grund: mit 12 → 2 mm lässt sich das Holz am freien Boden
  eichen (elf Moden ≤ 1,8 %), der Kandidat aus S209 fällt damit aber auf 144,9 / 214,3 Hz.
- Proben der Eichung und der Feinsuche: `docs/modellierung/gezupfte-saite/proben/2026-11-20_S210_harfe_*`.

## Geändert

- Alle sieben Baupläne und die fünf Schreiber auf Fassung 9, alle sieben Körper neu gerechnet (gleich bis auf Abdruck
  und Rechenzeit).
- Harfenbericht `2026-11-13_S197_harfe_gelesen.md` §18–§20.
- `docs/audio/Instrumente_IC_Konzept.md` 0.77.0, `docs/Offene_Punkte.md` 1.419.0.

## Zahlen

**Harfe, vier Fassungen der Feinsuche (nicht gebaut).** Alle sieben Lagen innerhalb 7 %, je sechs Höhen außerhalb.

| Zielgröße | F1: Mündung 2,15, Feder bis 0,6 m, 7 000 N | F2: 2,0, 0,6 m, 5 000 N | F3: 2,0, 0,7 m, 3 000 N | F4: 2,3, 0,6 m, 7 000 N |
|---|---|---|---|---|
| Moden mit Luft gegen 170 / 216 Hz | 166,7 / 222,1 | 164,1 / 223,0 | 161,5 / 220,1 | 164,4 / 218,9 |
| Bäuche außerhalb (2 Orte) | 0 | 1 | 1 | 0 |
| Abstrahlung Saiten 33–22 mit Saiten 0,2 (von 12) | 5 | 6 | 6 | 6 |
| Lautheit mit Mitschwingen (22 / 12 / 3 LU) | 18,9 / 12,6 / 2,4 | 19,3 / 11,5 / 2,7 | 19,2 / 10,7 / 2,5 | 18,4 / 12,2 / 2,5 |

Heute im Repo: Moden 134 / 198 Hz, Lautheit ohne Mitschwingen 20,0 / 12,8 / 3,0 LU.

**Faktor der Saiten** (an F2, Bell beim Besaiten +10 / +3 / +5,7 %): 0,2 → +5,3 / +3,8 / +2,5 %; 0,3 → +7,4 / +6,1 /
+3,6 %; 0,4 → +9,3 / +8,3 / +4,8 %. An F4 Lautheit ohne Mitschwingen: 0,2 → 17,6 / 12,1 / 2,7; 0,3 → 18,4 / 12,7 /
2,9; 0,4 → 19,1 / 12,7 / 2,8 LU.

## Tests

Schritt 1: `*Loop Koerper*,*Klangfassung*` ohne `*KQ18 Drehfeder*,*KQ20*` und `*31c-5d*` 3 / 3 unter Release-clang
und Testing-MSVC, kein voller Lauf (die Rechnung rufen nur `InstrumentBodyTool` und die Körper-Tests). **1924 Cases.**

## Offen

- KQ51: Wahl der Fassung (vorgelegt, Empfehlung F4). KQ52: Faktor der Saiten (Lautheit mit Mitschwingen für 0,3 / 0,4
  fehlt).
- Harfe bauen: `make_harfe.py`, Körper, Test, README.
