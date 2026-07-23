# PresetTypeIcons

> **Header-only:** `UI/widgets/PresetTypeIcons.hpp` · **Seit:** Session 43
> (Sichttest-Befund: Import-Browser zeigte generische Datei-Icons)

Gemeinsame Format-Icons für Preset-Dateien und -Nodes: **AVS** (`avs.ico`),
**MilkDrop** (`milkdrop.ico`), **LumiViz nativ** (`lumiviz.ico`) aus
`asset/img/logo/icons` — SSOT für Effect-Chain-Panel (Origin-Icon der
Type-Spalte, Palette) und Import-Browser (Datei-Einträge).

## Funktion

- `presetIconDir()` — löst das Icon-Verzeichnis zur Laufzeit auf
  (Aufwärtssuche vom Anwendungsverzeichnis, weil die Exe aus `out/build/…`
  läuft); leer, wenn nicht gefunden (statisch gecacht).
- `presetTypeIcon(PresetIconKind)` — Icon je Format (statisch gecacht;
  Null-Icon ohne Icon-Verzeichnis).
- `presetTypeIconForSuffix(suffix)` — Endungs-Mapping `.milk` → MilkDrop,
  `.lvfx`/`.lvfx2` → Nativ, sonst AVS (gleiche Zuordnung wie der
  Import-Dispatch des Import-Browsers).

## Verträge

- Aufrufer prüfen `presetIconDir().isEmpty()` und fallen selbst auf Text
  (Chain-Panel-Palette) bzw. `QStyle::SP_FileIcon` (Import-Browser) zurück.
- Nur Qt-GUI-Thread (QIcon).

## Absicherung

Qt-UI-Helfer → kein Unit-Test; Sichttest über Effect-Chain-Panel und
Import-Browser (Icons je Dateityp sichtbar).
