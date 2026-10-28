# Changelog Session 159 (2026-10-28)

> **Der Flügel hat einen neuen Hammer, und die Messung hört wie ein Mensch.** Die Diagnose zum Hörurteil S158 fand
> keinen Fehler im Rechenweg, aber drei Ursachen: die zweite Abklingstufe war nie gesetzt, die erste an ein
> Messziel gesetzt, das die hörbare Stufe verfehlt, und der Hammer nach RT-0425 wurde mit der Anschlagstärke kaum
> heller und war oben zu weich. Der Hammer rechnet jetzt nach Stulov, mit Hysterese des Filzes; die Messung bekommt
> die Hüllkurve vorwärts als Prüfgröße. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (HU1 a, HU2 a,
> HU3-1 c).

## Geändert

- **Hammer nach Stulov** (HU3-1 c): Masse, Exponent, Steife und neu Hysterese ε und Relaxation τ je Taste nach
  Stulov & Kartofelev 2009, Gl. 9; die Kraft im Kontakt nach Stulovs Gl. 2 (härter beim Laden, weicher beim
  Entlasten). Der Kontakt an C8 dauert 0,86 ms statt 1,75 ms, der Ton wird mit der Stärke heller wie an der
  Aufnahme (Helligkeit je Stärke an C4 bis auf 0,2 dB/dB). `.lvinst` Fassung 20 (`hammerHysteresis`,
  `hammerRelaxation`); ältere Dateien bekommen die Vorgabe. *Grand Piano* neu geschrieben, Level 4,0355.

## Intern

- Diagnose HU1 a mit dem Messfall `LUMIVIZ_HU1_WAV` (Kraft am Steg, Antwort des Bodens, Ton je Taste in sechs
  Fassungen); Befund mit Tabellen im Konzept.
- Messung: je Teilton die Hüllkurve vorwärts (Gerade der ersten 0,5 s nach der Spitze, Tiefe des Knicks), der ganze
  Ton ab der Spitze bis −20/−30/−40 dB, beides aus dem Kanalmittel; `salamander_c5.txt` neu gemessen (+272 Zeilen).
- HU3 a (R_H nachtragen) vor dem Bau verworfen: die Formel aus dem PDF-Text hatte ein verlorenes Minus, R_H ist
  wirkungslos.
- Klangfassung 4; `loop_werte.txt` fünf Zeilen des Flügels; Chabassier-Fall bewacht die Auslenkung als Band.
- Tests: 1643 Cases (+3); Bereich 146 / 146 unter Release-clang und Testing-MSVC, voller Lauf steht aus.
