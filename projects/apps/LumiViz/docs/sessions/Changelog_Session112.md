# Changelog Session 112 (2026-10-11)

> **Am Programm hat sich nichts geändert.** Die Session hob den Bezug des
> Build-Systems auf CMakeCraft v0.11.1 und behob ein Bau-Werkzeug.

## Geändert

- **Build-System:** `cmakecraft.pin` steht auf v0.11.1 (vorher v0.11.0).

## Behoben

- **`tools\bau_clang.bat`** gab beim Start zehn Zeilen „Der Befehl … ist
  entweder falsch geschrieben oder konnte nicht gefunden werden" aus und
  baute trotzdem. Die Datei hatte Zeilenenden, die die Eingabeaufforderung
  von Windows nicht richtig liest; sie steht jetzt auf CRLF.

## Für Entwickler

- CMakeCraft v0.11.1 (Paketdatei 1.1.1): das Ziel, das ein Paket neben die
  Exe kopiert (`LumiViz_deploy_sichttest`), schreibt unter den
  Visual-Studio-Generatoren keine Recipe-Datei mehr. Anlass war eine Warnung
  in einem anderen Projekt; ob sie bei LumiViz je auftrat, ist nicht gemessen.
- `.gitattributes`: `*.bat` und `*.cmd` tragen `eol=crlf` — die Arbeitskopie
  bekommt CRLF, was immer `core.autocrlf` sagt.
- Eine Zeile zu `vswhere.exe` beim Start von `bau_clang.bat` bleibt; sie kommt
  aus `VsDevCmd.bat` von Visual Studio 18.

## Offen

- Der Fall „PaletteUi Panel A10/A11" fiel in einem von zwei vollen Läufen um
  (`test_PaletteUi.cpp:1609`, `3 == 2`); einzeln fünfmal grün. Ursache nicht
  untersucht.
- Alles aus Session 111 bleibt offen wie dort beschrieben: der Blick ins
  Programm und das Ohr auf Instrumente und Loops, Hall und Echo am Muster,
  ein größeres Drumkit, `asset\loop` im Release-Paket.

## Tests

1044 Cases, zwei volle Läufe (Testing-MSVC): 1043 / 1044, dann 1044 / 1044,
0 Skips. Unter Release-clang 72 Fälle zu Loop, Editoren und
Sichttest-Aktionen. Nicht gemacht: Debug-MSVC, der volle Lauf unter clang.
