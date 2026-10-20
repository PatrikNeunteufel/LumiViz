# Changelog Session 140 (2026-10-20)

> **Spielen über der Zeit.** Ein Instrument-Muster bekommt Spuren über der Zeit: Sustain- und
> Sostenuto-Pedal, Expression, Vibrato und Pitch, dazu eine Expression-Kurve über einen ganzen Clip.
> In dieser Session gebaut sind Datei, Planung und Klang; zeichnen lassen sich die Spuren erst mit
> dem Editor in der nächsten Stufe. Dazu das Konzept für die Spielweisen aller Instrumentfamilien
> bis zur ersten Stufe entschieden.
> Steuerdokumente: `docs/audio/Spielweisen_Konzept.md` (SF6 a bis SF10 a, Skizze §1a, SK1 a bis
> SK11 a, §3 Stufe 28), `docs/audio/Loop_Konzept.md` (Dateiformate).

## Neu

- **Pedale:** Sustain hält Noten, bis das Pedal losgelassen wird; Sostenuto nur die Noten, die beim
  Treten klingen. Am Ende eines Clips wird das Pedal losgelassen.
- **Expression:** die Stärke des Spielens über der Zeit, im Muster und als Kurve über den ganzen
  Clip; an einem Synth ein Pegel nach der MIDI-Empfehlung (0,5 = −12 dB).
- **Vibrato und Pitch** über der Zeit — an Synths, und an Saiten über die Spannung wie der Bend.
- **Pitch range** je Schicht im Creator (Vorgabe ±2 Halbtöne, bis ±24): so weit folgt die Schicht
  der Spur Pitch. Bei Vibrato-Rate 0 nimmt die Spur Vibrato 5,5 Hz.
- **Tremolo arm** in der Zeile *Tuning*: ein Vibratohebel mit eigener Spanne nach unten und oben;
  ohne Hebel überhört die Saite die Spur Pitch.

## Geändert

- *Repeat when lengthening* wiederholt auch die Spuren; ein neues Raster lässt sie an ihrer Stelle.
- Wo die Spur Vibrato Werte trägt, gilt ihre Tiefe statt der eigenen der Schicht.
- `.lvloop` Fassung 12 (`lanes`, `expression`), `.lvinst` Fassung 15 (`pitchRange`, `tremoloDown`,
  `tremoloUp`).

## Intern

- `loop/Zeitspur`, `loop/Pedalplan`; im Rechenkern der Verlauf `Ausdruck`; Töne mit Spuren je
  Vorkommen (`:q`, `:w` im Namen); `Handweg`/`Bahn` mit den Spuren.
- Neue Tests `test_LoopZeitspur`, `test_LoopPedal`, `test_LoopExpression`, `test_LoopVibratoPitch`,
  `test_LoopSaiteSpuren`; 1436 Fälle. Zuletzt `*Loop*,*Instrument*` 339 / 339 unter beiden
  Compilern; **kein voller Lauf** (an Stufe 28f).
- `Spielweisen_Konzept.md` 0.22.0, `Loop_Konzept.md` 0.102.0, `Offene_Punkte.md` 1.267.0.
