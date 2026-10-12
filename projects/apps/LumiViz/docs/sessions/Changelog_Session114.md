# Changelog Session 114 (2026-10-12)

> **Datenablage und das Fenster Instruments.** Was LumiViz mitbringt, liegt
> jetzt nach jedem Bau neben der Exe; die eigenen Daten lassen sich in eine
> Datei exportieren und wieder einlesen; und Instrumente wählt man im Loop in
> einem eigenen Fenster mit Suche und Genre statt in einer Dropdown-Liste.
> **Im Programm geprüft ist davon nichts** — alles ist mit Unit-Tests und
> Bildern aus den Tests geprüft. Die Zuordnung der Instrumente nach Art, ihre
> Genres und Beschreibungen sind gesetzt, nicht abgehört.

## Neu

- **Der Ordner `asset` neben der Exe:** mitgelieferte Voreinstellungen der
  Effekt-Module, Shader-Vorlagen, Trails, Blenden und Mutes, das Kit, die
  Instrumente und die Beispiel-Loops, die Datei-Icons. Eine weitergegebene
  Exe hatte davon bisher nichts.
- **Export und Import der eigenen Daten** (Settings → Panels → **User Data:
  Export...**, **Import...**): eigene Presets, Vorlagen, Trails, Blenden und
  Instrumente — alle oder einzelne Bereiche — in eine Datei `.lvdata`.
  Wählbar dazu: Einstellungen (Tastenkürzel, Favoriten, Optionen) und die
  geladenen Songtexte mit Versätzen. Die Datei ist ein Zip.
- **Import mit vier Regeln** für Namen, die es schon gibt: **Keep mine**,
  **Replace mine**, **Keep both** (die importierte Datei bekommt eine Zahl an
  den Namen), **Let me decide for each file** — eine Liste mit beiden Dateien
  je Name, Datum und Größe. Eine Datei, die Byte für Byte gleich ist, zählt
  nicht.
- **Das Fenster Instruments** im Loop (**Browse...** im Instrument-Editor):
  die Instrumente nach Art (Keys, Bass, Lead, Pads and strings, Plucked,
  Wind, Percussion, Unusual), **Search**, Filter nach **Genre**,
  **Favorites**, **Recently used**, **My files**. Ein Klick spielt einen Ton,
  Doppelklick oder **Use** setzt das Instrument ins Muster. **Preview note**
  wählt die Tonhöhe des Anspielens.
- **Eigene Instrumente haben einen Ort:** **Save** im Instrument-Creator legt
  sie unter einem Namen zu den eigenen Dateien; sie stehen im Fenster unter
  My files und gehen mit dem Export mit. Ein eigenes verdrängt kein
  mitgeliefertes.
- **Type, Genres, Description** im Instrument-Creator: wo ein Instrument
  einsortiert ist, zu welchen Genres es gehört, ein Satz dazu.
- **Benutzerhandbuch:** Settings (Export und Import), §16 Loop (das Fenster
  Instruments, der Creator).

## Geändert

- **Im Instrument-Editor gibt es keine Dropdown-Liste der Instrumente mehr.**
  Der Name steht als Text, **Browse...** öffnet das Fenster.
- **Save im Instrument-Creator** ist auch bei einem mitgelieferten Instrument
  wählbar, sobald etwas geändert ist; es fragt nach einem Namen für die
  eigene Kopie. Vorher war es grau, und nur **Save as...** ging.
- **Das Format `.lvinst` trägt Fassung 3** (Art, Genres, Beschreibung).
  Fassung 1 und 2 liest LumiViz weiter; gespeichert wird Fassung 3. Die 49
  mitgelieferten Instrumente stehen in Fassung 3 — ein älteres LumiViz lehnt
  sie ab.
- **LumiViz liest Mitgeliefertes aus der Kopie neben der Exe**, nicht mehr aus
  dem Projektordner. Im Entwicklungsbaum lässt sich das in den Settings
  umstellen (**Bundled Data: Read from the project folder first**).
- **`presets` neben der Exe** wird bei jedem Bau abgeglichen, auch wenn die
  Exe nicht neu gelinkt wird; was im Repo gelöscht ist, verschwindet dort.

## Für Entwickler

- [Datenablage_Konzept.md](../core-services/Datenablage_Konzept.md) 0.4.0
  (neu) — Bestandsaufnahme, Entscheide **D1 b** bis **D10 c**, **E1 a**,
  **E2 b**, die gebilligten Skizzen; Stufen A1 bis A4b und A3 gebaut.
- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.38.0 — §4h: die Skizze des
  Fensters, Entscheide **F3 b** (Felder in der `.lvinst`, Fassung 3) und
  **P2 a** (eigene Instrumente als `user:<name>`, nichts wird verdrängt);
  Stufe 14 gebaut, 11 und 15 offen.
- **`asset/Asset.cmake`** listet, was neben die Exe kommt
  (`lumi_asset(<Bereich> [NACH …] [NUR …] [OHNE …])`);
  `asset/AssetAbgleich.cmake` führt die Liste aus; das Ziel `LumiViz_Assets`,
  von dem `LumiViz` abhängt, läuft bei jedem Bau. Nur eigene Werke.
- `core/Mitgeliefert.hpp` (neu): `suche`, `ordner` — die eine Stelle für den
  Ordner eines mitgelieferten Bereichs; sieben Sucher rufen sie.
- **miniz 3.1.2** unter `externals/miniz` als lokales External mit eigener
  `Include.cmake` **im Projekt** (`Solution.json`, Feld `include`) — ohne
  Änderung an CMakeCraft. Vermerk in `THIRD_PARTY_NOTICES.md`.
- `core/ZipArchiv.hpp` (neu): die einzige Stelle, die miniz kennt, nur über
  Speicher. `core/Datensicherung.hpp` (neu): Bereiche, `exportiere`, `oeffne`,
  `importiere`, die feste Liste der Einstellungen (`einstellungGehtMit`) — ein
  Archiv schreibt nur in die Ordner der Bereiche; `shadertoy/apiKey` geht nie
  hinaus und nie herein. `UI/dialogs/DatensicherungDialoge.hpp` (neu).
- `loop/Instrument.hpp`: `art`, `genres`, `beschreibung`; `instrumentArten`;
  `kEigenVorsatz`, `eigenerName`, `eigeneInstrumente`, `speichereEigenes`,
  `loescheEigenes`, `bekannteGenres`. `loop/InstrumentPalette.hpp` (neu).
- `asset/loop/instrument_katalog.py` (neu): Art und Beschreibung der
  mitgelieferten Instrumente in einer Tabelle, die Genres aus den Loops
  gezählt. Wer ein Instrument dazulegt, trägt es dort ein.
- Bilder aus den Tests: `LUMIVIZ_EXPORT_DIALOG_BILD`,
  `LUMIVIZ_IMPORT_DIALOG_BILD`, `LUMIVIZ_IMPORT_WAHL_BILD`,
  `LUMIVIZ_INSTRUMENT_PALETTE_BILD` (je `=<datei.png>`);
  `LUMIVIZ_ZIP_PROBE=<datei.zip>` legt ein Archiv zum Entpacken ab.
- Gemessen: der Abgleich von 421 Dateien dauert ohne Änderung rund 1,1 s
  (einmal).

## Offen

- Der Blick ins Programm und das Ohr: alles aus dieser Sitzung.
- Offene Listen lesen nach einem Import nicht von selbst neu.
- Beispiele aus `examples`, `composits`, `shadertoys`, `effectchain` liegen
  noch nicht neben der Exe; `.lvloop`, `.lvinst`, `.lvdata` haben kein Icon.
- Ein Schalter, mit dem ein eigenes Instrument das mitgelieferte für alle
  Loops ersetzt, ist vorgemerkt.
- In der Konfiguration Testing unter MSVC fehlt `/MD` bei allen Zielen; der
  Linker meldet es seit miniz (LNK4098). Nicht behoben.
- Instrumente aus Samples, Wave-Clips und Autotune.

## Tests

1105 Cases (37 neu), voller Lauf 1105 / 1105, 0 Skips (Testing-MSVC). Unter
Release-clang 144 Fälle zu Loop, Instrumenten, Editoren, Sichttest-Aktionen
und Datenablage. Nicht gemacht: Debug-MSVC, der volle Lauf unter clang.
