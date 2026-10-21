# Changelog Session 142 (2026-10-21)

> **Töne werden im Hintergrund gerechnet.** Jeder Ton eines Instruments wird einmal gerechnet und
> dann aus einem Speicher geholt; was fehlt, rechnen mehrere Kerne, ohne dass die Oberfläche stockt.
> Play wartet im Loop-Panel und im Composer, bis alles da ist, und sagt, wie weit es ist. Das ist
> das Fundament für teure Instrumente wie den Flügel.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.5 (TV1 a bis TV7 a, WX1 a).

## Neu

- **Play wartet auf die Töne:** im Loop-Panel steht an der Stelle von „Bar · Step“ „Preparing
  sounds… 12 / 40“, im Composer im Feld Zeit; Stop (bzw. Pause) bricht ab. Sind alle Töne schon
  da, beginnt Play sofort.
- **Vorladen:** beim Öffnen eines Loops und einer Komposition werden die Töne schon im Hintergrund
  gerechnet.
- **Bearbeiten während des Spielens:** die bisherigen Töne klingen, bis die neuen fertig sind
  („· updating sounds 3 / 5“).
- **Im Composer nach einem Sprung:** fehlen Töne, schweigen nur die fehlenden Noten, die Zeit sagt
  es in Warnfarbe.
- **Hinweiszeilen:** Instrument- und Drum-Editor sagen, wenn *Play pattern* oder ein angespielter
  Ton noch wartet; der Drum-Editor hat dafür eine neue Zeile.

## Geändert

- Ein angespielter Ton zählt seine Dauer erst, wenn sein Klang da ist.
- Klänge der Drum-Spuren werden wie die Töne nur einmal dekodiert und geteilt.

## Intern

- `loop/Tonspeicher` (Grenze 1 GB, der am längsten nicht gebrauchte Ton geht zuerst, Schlüssel als
  Fingerabdruck der Eingaben), `loop/Tonauftrag` (Thread-Pool, Fortschritt, Abbruch), Typ `Klaenge`
  (geteilte Zeiger); `instrumente()` threadfest; Spieler, Sitzung und Tonführung laden über
  Aufträge; Lebenszyklus-Regel `Bootstrap_Integration.md` §6.7.
- Neue Tests `test_LoopTonThreads`, `test_LoopTonspeicher`, `test_LoopTonauftrag` (mit BASS auf
  8 Threads), Fälle in `test_LoopSpieler` und `test_ComposerPanel`; 1461 Fälle. Bis 29d unter beiden
  Compilern grün, 29e unter Release-clang; **kein voller Lauf**.
- Konzept: Export eines Clips als Wellenform (Stufe 30, vor Freeze), Audio-Clip im Loop als
  Stufe 31, der Flügel ab Stufe 32.
