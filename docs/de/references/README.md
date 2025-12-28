# References — Nachschlagewerke

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Sprache:** Deutsch  
> **English:** [README.md](../../en/references/README.md)

---

## Quick-Start

**Fehlercode nachschlagen?**
1. [ErrorCodes.md](ErrorCodes.md) — Alle Fehlercodes E0xx-E9xx

**JSON-Konfiguration verstehen?**
1. [Solution_Schema.md](Solution_Schema.md) — Solution.json Schema
2. [CMakePresets.md](CMakePresets.md) — CMakePresets.json
3. [CMakeUserPresets.md](CMakeUserPresets.md) — Eigene Presets

**Externals-Katalog durchsuchen?**
1. [Externals.md](Externals.md) — Übersicht Externals-System
2. [externals/Git_Externals.md](externals/Git_Externals.md) — Git-basierte Libraries
3. [externals/Local_Externals.md](externals/Local_Externals.md) — Lokale Libraries

**Begriff nachschlagen?**
1. [Glossar.md](Glossar.md) — Alle Definitionen

---

## Übersicht

Referenzdokumente sind Nachschlagewerke ohne Tutorial-Charakter. Sie dokumentieren APIs, Schemas, Konfigurationsoptionen und Definitionen.

---

## Dateien

| Datei | Beschreibung |
|-------|--------------|
| [ErrorCodes.md](ErrorCodes.md) | Vollständige Fehlercode-Referenz (E001-E999) |
| [Solution_Schema.md](Solution_Schema.md) | JSON-Schema für Solution.json |
| [CMakePresets.md](CMakePresets.md) | CMakePresets.json Referenz |
| [CMakeUserPresets.md](CMakeUserPresets.md) | CMakeUserPresets.json Referenz |
| [Externals.md](Externals.md) | Externals-System Übersicht |
| [Glossar.md](Glossar.md) | Begriffsdefinitionen A-Z |

---

## Unterordner

| Ordner | Dateien | Beschreibung |
|--------|---------|--------------|
| [externals/](externals/README.md) | 14 | Externals-Definitionen (Git + Local) |

---

## Direktzugriff: Externals-Katalog

### Git-basierte Externals

| Datei | Kategorie | Libraries |
|-------|-----------|-----------|
| [externals/Git_Externals.md](externals/Git_Externals.md) | Übersicht | Alle Git Externals |
| [externals/Git_Externals_Core.md](externals/Git_Externals_Core.md) | Core | spdlog, fmt, nlohmann_json |
| [externals/Git_Externals_GUI.md](externals/Git_Externals_GUI.md) | GUI | GLFW, ImGui |
| [externals/Git_Externals_Testing.md](externals/Git_Externals_Testing.md) | Testing | doctest, Catch2, GoogleTest |
| [externals/Git_Externals_Media.md](externals/Git_Externals_Media.md) | Media | stb |
| [externals/Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) | Scripting | sol2 |
| [externals/Git_Externals_AI.md](externals/Git_Externals_AI.md) | AI/ML | onnxruntime |
| [externals/Git_Externals_Data.md](externals/Git_Externals_Data.md) | Data | sqlite3, rapidjson |
| [externals/Git_Externals_Network.md](externals/Git_Externals_Network.md) | Network | cpr, asio |

### Lokale Externals

| Datei | Kategorie | Libraries |
|-------|-----------|-----------|
| [externals/Local_Externals.md](externals/Local_Externals.md) | Übersicht | Alle Local Externals |
| [externals/Local_Externals_GUI.md](externals/Local_Externals_GUI.md) | GUI | Qt6 |
| [externals/Local_Externals_Media.md](externals/Local_Externals_Media.md) | Media | BASS |
| [externals/Local_Externals_Scripting.md](externals/Local_Externals_Scripting.md) | Scripting | Lua |
| [externals/Local_Externals_Testing.md](externals/Local_Externals_Testing.md) | Testing | doctest (local) |

---

## Siehe auch

- [../README.md](../README.md) — Deutsche Dokumentation Übersicht
- [../blueprints/Reference.md](../blueprints/Reference.md) — Reference Blueprint
- [../userguides/](../userguides/README.md) — Praktische Anleitungen
