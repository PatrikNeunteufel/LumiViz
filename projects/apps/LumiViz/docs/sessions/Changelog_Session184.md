# Changelog Session 184 (2026-11-09)

> **CMakeCraft v0.14.1.** Das Build-System steht auf der neuen Fassung; Qt-Programme bekommen unter clang einen
> ausdrücklichen Einstiegspunkt. Dazu zwei Befunde ohne Codeänderung.

## Geändert

- **Build-System:** `cmakecraft.pin` auf **v0.14.1** (`0c027f0`) — ausdrücklicher Einstiegspunkt für Qt-GUI-Programme
  (lld-link warnt nicht mehr vor WinMain), vollständige Fehlermeldungen, Prüfungen E503–E505 für System-Externals. Alle
  fünf Build-Ordner neu konfiguriert (Release-clang, Testing-MSVC, Ninja Multi, Debug-MSVC, Release-MSVC).

## Befunde (ohne Code)

- **„Instrument not found" für Upright, Honky-tonk, Electric grand, Clavinet:** gestartet war die Exe von Visual Studio
  (`out/build/windows-ninja-multi`) vom 08.11. — ihre Kopie der mitgelieferten Instrumente war älter als S181. Neu gebaut.
- **Composer-Clip eines Loops stumm:** der Clip trug den Grundpegel 0 (`"base": 0`) ohne Pegelform — er stand die ganze
  Zeit auf null. Die vier Titanium-Vergleiche unter `asset/` zeigen jetzt auf den neuen Ort des Loops
  (`…\cmake\samples\Hoerproben\LumiViz\titanium\`), `fullCompare2` mit Form und Gain wie `fullCompare`.

## Bekannt

- Der volle Lauf nach dem Pin-Wechsel wurde abgebrochen (Patrik); bis dahin rot: `test_LoopInstrumentPalette` „Suche und
  Genre" — 31 bekannte Genres statt 30.
