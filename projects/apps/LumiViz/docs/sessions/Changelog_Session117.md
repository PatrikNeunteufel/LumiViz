# Changelog Session 117 (2026-10-12 / 13)

> **Eigene Instrumente haben jetzt einen Ort, und die Drums gibt es auch
> gerechnet.** Wer im Instrument-Creator speichert, wählt nur noch einen
> Namen — die Datei kommt immer zu den eigenen Instrumenten. Ein
> mitgeliefertes Instrument wird nie überschrieben. Unter **Drums** stehen
> sechs gerechnete Schläge, den Klängen des Kits nachgebaut. Dazu ist
> entschieden, wie Filter und Effekte zu einer frei geordneten Reihe werden
> (**Signal path**) — gebaut ist davon noch nichts.
> **Im Programm gesehen oder gehört hat davon niemand etwas.** Alles ist mit
> Unit-Tests und Messungen geprüft.

## Neu

- **Sechs gerechnete Drum-Instrumente** im Fenster Instruments unter
  **Drums:** Kick, Tom, Snare, Clap, Hi-Hat Closed, Hi-Hat Open. Sie bestehen
  aus Schichten und lassen sich im Instrument-Creator ansehen und ändern.
  Sie sind **für die Note C2 gebaut** (die Note, die eine Drum-Spur
  vorbelegt) und **leiser als die Klänge des Kits**. Gegen das Kit gemessen,
  nicht gehört — am nächsten sind Tom, Snare und die Hi-Hats, der Kick ist
  kürzer, der Clap etwas anders gefärbt. Die mitgelieferten Loops spielen
  weiter das Kit.
- **Load instrument...** fragt nach der Wahl einer Datei, wo sie bleiben
  soll: **Copy the file to My files (recommended)** oder **Use it where it
  is**. Die Kopie steht danach unter My files und geht mit einem Export.
- **Benutzerhandbuch:** §16 Loop (die gerechneten Drums, Save as..., Load
  instrument...).

## Geändert

- **Save as...** im Instrument-Creator öffnet keinen Dateidialog mehr. Es
  fragt nach einem Namen und legt eine Kopie zu den eigenen Instrumenten
  (`instruments` im Benutzerordner; der Ordner wird angelegt). Vorher konnte
  die Datei an einem beliebigen Ort landen, und das Fenster Instruments
  zeigte sie dann nicht.
- **Der Dialog „Save instrument"** sagt bei einem mitgelieferten Instrument
  ausdrücklich, dass Originale nicht überschrieben werden; der vorgeschlagene
  Name lässt sich behalten oder ändern.
- **Fünfundfünfzig mitgelieferte Instrumente** statt neunundvierzig.

## Entschieden, nicht gebaut

- **Signal path** (`Loop_Konzept.md` §4j): Filter und Effekte werden eine
  Reihe von Bausteinen, die sich hinzufügen, entfernen und umordnen lassen —
  je Schicht im Creator (Filter, Drive, Envelope) und je Clip (Filter,
  Distortion, Echo, Reverb), höchstens acht je Reihe. Die Baustufen 16a bis
  16e sind vorgelegt, die Freigabe steht aus.

## Für Entwickler

- `asset/loop/make_drums.py` schreibt die sechs `drum_*.lvinst` (Fassung 6).
  `instrument_katalog.py` lässt einer Datei ihre Fassung, wenn sie über 3
  liegt.
- `loop/Instrument`: `nimmZuEigenenInstrumenten`. `loop/InstrumentCreator`:
  `kopieSatz`, `frageNachDatei` entfällt. `loop/InstrumentEditor`:
  `nimmInstrumentDatei`.
- Tests: 1142 Cases (2 neu), voller Lauf 1142 / 1142, 0 Skips unter
  Testing-MSVC; unter Release-clang 173 Fälle zu Loop, Instrumenten, Drums,
  Mitgeliefertem und Datensicherung. `LUMIVIZ_11F_WAV=<Ordner>` legt je
  Drum-Instrument einen Schlag ab. Die Drums sind von der Prüfung „klingt auf
  der gespielten Note" ausgenommen.

## Offen

- Patriks Ohr an den Hörproben zu den Drums (und denen aus 11d und 11e) —
  daran hängt, ob die Hi-Hat-Schwingung gebaut wird und ob die gerechneten so
  laut sein sollen wie das Kit.
- Die Freigabe der Baustufen zum Signal path; davor K5 (mehrere markierte
  Clips mit verschiedenen Reihen).
- Im Programm ungesehen: alles aus dieser Sitzung.
