# buildSystemTest — Selbsttests des Build-Systems (CMakeCraft)

> **In Konsumenten-Projekten (LumiViz & Co.) wird dieser Ordner NICHT benötigt.**
> Er ist Teil des CMakeCraft-Snapshots und wird beim Sync 1:1 mitgespiegelt —
> **bitte nicht löschen** (die Top-Level-CMakeLists.txt referenziert die Dateien,
> und der Spiegel-Sync `robocopy /MIR cmake/` würde ihn ohnehin wiederherstellen).

## Zweck

Phasentests der „CMake Architecture V2": `phase1.cmake` … `phase9.cmake` prüfen die
Build-System-Module (Core, Solution, Libraries, Externals, Tests, App-Container, …)
zur Configure-Zeit.

## Wann laufen sie?

- **Default: AUS** (`RUN_BUILD_SYSTEM_TESTS` = OFF) — in Projekten laufen sie also nie mit.
- **Im CMakeCraft-Repo** (Entwicklung am Build-System): Preset **`craft-selftest`**
  verwenden — oder jedem Preset `-DRUN_BUILD_SYSTEM_TESTS=ON` mitgeben.
- **In Projekten sinnvoll als Smoke-Check** direkt nach einem Snapshot-Sync
  (siehe Neues_Projekt_Guide §5.1):
  ```bash
  cmake --preset <euer-preset> -DRUN_BUILD_SYSTEM_TESTS=ON     # einmalig testen
  cmake --preset <euer-preset>                                  # danach wieder normal
  ```
- Einzelne Phase: zusätzlich `-DTEST_PHASE=6` (o. ä.).

Doku: `docs/en/projects/buildsystem/` (Konzepte) bzw. Modul-Doku der Phasen.
