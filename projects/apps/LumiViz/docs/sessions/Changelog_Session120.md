# Changelog Session 120 (2026-10-13)

> **Klang aus einer Form ist entworfen, und die ersten Stufen sind gebaut.**
> Aus einer geschlossenen Kontur soll ein Wellenzug werden — die Waveform
> **Shape** im Instrument-Creator. In dieser Sitzung entstand das Konzept mit
> allen Entscheiden und zwei gebilligten Skizzen; gebaut sind **Pan je
> Schicht**, **acht Schichten je Instrument** und die Form als Daten mit der
> Datei `.lvcontour`. Daneben: acht neue Datei-Icons.
> **Im Programm gesehen oder gehört hat davon niemand etwas.** Alles ist mit
> Unit-Tests und Bildern aus den Tests geprüft. Einen Klang aus einer Form
> gibt es noch nicht.

## Neu

- **Pan je Schicht** (Instrument-Creator): unter dem Kasten **Level** steht
  ein zweiter Regler **Pan**, für jede Waveform. „C" ist die Mitte, „L 100"
  ganz links, „R 100" ganz rechts. Zur Seite hin wird die andere Seite
  leiser — dasselbe Pan wie an einer Spur im Drum-Editor. Der Kasten nennt
  Pan, sobald es neben der Mitte steht („80 % · Pan L 20").
- **Acht neue Datei-Icons:** Loop, Instrument, Kontur, Blende und Stück des
  Curve Editors, Mute-Profil, Export der eigenen Daten, Farbverlauf — jedes
  das LumiViz-Icon mit einem eigenen Zeichen im Kreis. Das Skript
  `asset\img\logo\icons\register_file_icons.ps1 -Apply -Foreign` trägt sie im
  Explorer ein.
- **Die Icons in der App:** im Audio Browser trägt ein Loop sein eigenes Icon
  statt des Kreispfeils; im Panel Loop steht es vor dem Dateinamen, im
  Instrument-Editor das der Instrumente vor dem Namen; im Dialog Import data
  steht das Icon vor der Datei und in der Liste „Let me decide for each file"
  an jeder Zeile, deren Art eines hat.
- **Export und Import der eigenen Daten:** der Bereich **Shapes** (eigene
  Formen).
- **Sechs mitgelieferte Formen** unter `asset\contours`: Circle, Square,
  Triangle, Star, Heart, Heart and star. Noch zeigt oder spielt sie nichts.

## Geändert

- **Ein Instrument trägt bis zu acht Schichten** (vorher vier). Mit jeder
  steigt **Peak**; liegt er über 1, die Level der Schichten zurücknehmen.
- **`.lvinst` steht in Fassung 8** (Pan, acht Schichten). Ältere Dateien
  lesen sich wie bisher und klingen Bit für Bit gleich; ein älteres Programm
  liest eine Datei der Fassung 8 nicht.

## Für Entwickler

- **Neues Konzept:** `docs/audio/Shape_Konzept.md` (0.12.0) — Rechnung,
  Datei, Eingabewege, Module, zwei Skizzen als Beschreibung, die Stufen 18
  bis 20d. Entscheide F1 a, D1 a, D2 a, D3 a, D4 b, K1 a, K2 a, S1 a, B2 d,
  B1 (wählbar), M1 a, M2 b, M3 a, G1 a, G2 a.
- **Stufe 18:** `Schicht::pan` mit dem Gesetz von Spur und Muster,
  `kSchichtenMax` 8, `kInstrumentFassung` 8; `creatorPan` im Creator.
- **Stufe 19a:** neu `shape/Kontur` (`lumi::shape`) — `Zeichnung`, `Kontur`
  mit fester Kennung, der Pfad als Text (`M`, `L`, `C`, `Z`), die Datei
  `.lvcontour` Fassung 1, die Orte (`asset/contours`, `user:<name>`, Pfad),
  `abtasten`, `normiert`, `reihe`, `gewicht`, `wert`, `umriss`.
- **Icons:** `dateiArtIcon(<endung>)` und `dateiIconFuerEndung` in
  `UI/widgets/PresetTypeIcons.hpp`; die Zeichen in
  `asset/img/logo/icons/build_lvfx2_icons.py`.
- **Qt Svg** steht in der `Solution.json` (`Svg`) — für das Lesen von SVG in
  einer späteren Stufe.
- **Tests:** 1163 Cases (zehn neu), voller Lauf 1163 / 1163, 0 Skips unter
  Testing-MSVC; unter Release-clang 191 Fälle zu Shape, Datensicherung und
  Loop. Neu `test_ShapeKontur.cpp` (sechs Fälle, Bild mit
  `LUMIVIZ_SHAPE_BILD`), `test_ShapeBelege.cpp` (zwei Belege: SVG über ein
  mitschreibendes Zeichengerät, `QPainterPath` ohne Qt-Anwendung), ein Fall
  zu Pan in `test_LoopInstrument.cpp`, ein Messfall zur Rechenzeit je Zahl
  von Schichten in `test_LoopBausteine.cpp`.
  `LUMIVIZ_INSTRUMENT_CREATOR_BILD` legt ein zweites Bild ab (`_level`).
- **Gemessen:** acht teure Schichten kosten rund 16 ms je Sekunde Ton
  (Release-clang), die Zeit wächst gerade mit der Zahl.

## Notiert, nicht gebaut

- **Idee Patrik: Publish.** Ein Instrument, ein Loop oder anderes mit allen
  zugehörigen Dateien und einem Manifest als Paket `<name>.<type>.lvdata`
  weitergeben; beim Import eine Option Vergleichen, und beim Umbenennen
  ziehen die Nennungen im Paket mit. `Offene_Punkte.md` §7.

## Offen

- **Im Programm ungesehen und ungehört:** Pan, die acht Schichten, die Icons
  in der App und im Explorer — Liste 10, Abschnitt V.
- **Nicht gebaut:** der Knoten Shape (Stufe 19b, als Nächstes), die
  Schwingung Shape im Klangerzeuger, der Knoten XY Scope, die Shape-Schicht im
  Creator, das Fenster Edit shape.
- **Zu hören in Stufe 19c:** die Kante des Sprungs zwischen zwei Konturen in
  einem Umlauf.
- Aus den Sitzungen 118 und 119 weiter: Stufe 17 und der Signal path im
  Programm, Patriks Ohr an den Hörproben zu Kick und Clap, die Frage zu
  `asset/loop/effects_demo2.lvloop`.
