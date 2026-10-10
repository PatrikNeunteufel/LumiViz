# Changelog Session 108 (2026-10-10 bis 2026-10-11)

> **Neu: der Loop.** LumiViz kann eine Datei `.lvloop` laden und im Kreis
> spielen — Drums aus einem mitgelieferten Kit oder aus eigenen Samples. Das
> ist der Anfang einer Erweiterung für eigene Songs; bauen lässt sich ein Loop
> im Programm noch nicht, nur laden und abspielen. **Im Programm hat es noch
> niemand gehört oder gesehen.**

## Neu

- **Panel „Loop"** (View → Panels → Loop): eine Datei-Zeile mit **Load…** und
  **Play / Stop**. Darunter stehen Tempo, Zahl der Kanäle, Takte und Dauer
  des Loops, beim Spielen die Stelle als „Bar · Step". Der zuletzt geladene
  Loop steht nach dem Neustart wieder im Panel; gespielt wird nicht von
  selbst.
- **Wer spielt, hat den Ton — Composer vor Loop-Panel vor Player:** solange
  der Loop spielt, schweigt der Player und spielt danach weiter, wenn er
  vorher spielte. Führt der Composer den Ton, ist Play im Loop-Panel gesperrt
  und sagt, warum; ein laufender Loop hält dann an. Lautstärke und Mute des
  Players gelten auch für den Loop.
- **Die Visuals reagieren auf den Loop**, solange er spielt.
- **Datei `.lvloop`:** ein Arrangement in Takten — Kanäle, darin Clips; ein
  Clip spielt ein Muster und wiederholt es. Das Muster trägt sein Drumkit
  (je Spur ein Klang, Lautstärke, Panorama, Mute) und je Spur die Schritte
  mit ihrer Lautstärke. Tempo, Raster und Swing gelten für den ganzen Loop.
  Clips mit demselben Muster bleiben verbunden.
- **Mitgeliefertes Kit** unter `asset/loop/kit`: Kick, Snare, Hi-Hat
  geschlossen und offen, Clap, Tom. Die Klänge sind gerechnet, nicht
  aufgenommen (`make_kit.py` daneben); an ihnen gibt es keine fremden Rechte.
- **Beispiel** `asset/loop/drums_demo.lvloop`: zwei Kanäle, 16 Takte bei
  120 BPM.

## Für Entwickler

- Konzept mit allen Entscheiden und Stufen:
  [Loop_Konzept.md](../audio/Loop_Konzept.md).
- Die Audio-Engine kennt eine Quelle, die ihren Ton selbst rechnet
  (`IAudioGenerator`, `mixerAddGenerator`), und dekodiert eine Datei ganz
  (`decodeStereo`).

## Offen

- Das Loop-Panel als Arrangement und der Drum-Editor als eigenes Fenster
  (beide als Skizze gebilligt), der Loop als Clip im Composer, das
  Benutzerhandbuch.
- Das Kit liegt unter `asset/` und ist im Release-Paket nicht neben der Exe.
- Debug-MSVC nicht gebaut.

## Tests

- 1002 Cases (28 neu), voller Lauf 1002 / 1002, 0 Skips (Testing-MSVC); die
  26 Fälle zum Loop auch unter Release-clang grün.
