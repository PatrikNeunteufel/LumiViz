# References — Nachschlagewerke

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../en/references/README.md)

---

## Quick-Start

**Errorcode nachschlagen?**
1. [ErrorCodes.md](ErrorCodes.md) — Alle Errorcodes E0xx-E9xx

**JSON-Configuration verstehen?**
1. [Solution_Schema.md](Solution_Schema.md) — Solution.json Schema
2. [CMakePresets.md](CMakePresets.md) — CMakePresets.json
3. [CMakeUserPresets.md](CMakeUserPresets.md) — Eigene Presets

**Externals-Katalog durchsuchen?**
1. [Externals.md](Externals.md) — Overview Externals-System
2. [externals/Git_Externals.md](externals/Git_Externals.md) — Git-basierte Libraries
3. [externals/Local_Externals.md](externals/Local_Externals.md) — Lokale Libraries

**Begriff nachschlagen?**
1. [Glossar.md](Glossar.md) — Alle Definitionen

---

## Overview

Referencedokumente sind Nachschlagewerke ohne Tutorial-Charakter. Sie dokumentieren APIs, Schemas, Configurationsoptionen und Definitionen.

---

## Dateien

| Datei | Description |
|-------|--------------|
| [ErrorCodes.md](ErrorCodes.md) | Vollständige Errorcode-Reference (E001-E999) |
| [Solution_Schema.md](Solution_Schema.md) | JSON-Schema für Solution.json |
| [CMakePresets.md](CMakePresets.md) | CMakePresets.json Reference |
| [CMakeUserPresets.md](CMakeUserPresets.md) | CMakeUserPresets.json Reference |
| [Externals.md](Externals.md) | Externals-System Overview |
| [Glossar.md](Glossar.md) | Begriffsdefinitionen A-Z |

---

## Unterordner

| Ordner | Dateien | Description |
|--------|---------|--------------|
| [externals/](externals/README.md) | 14 | Externals-Definitionen (Git + Local) |

---

## Direktzugriff: Externals-Katalog

### Git-basierte Externals

| Datei | Kategorie | Libraries |
|-------|-----------|-----------|
| [externals/Git_Externals.md](externals/Git_Externals.md) | Overview | Alle Git Externals |
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
| [externals/Local_Externals.md](externals/Local_Externals.md) | Overview | Alle Local Externals |
| [externals/Local_Externals_GUI.md](externals/Local_Externals_GUI.md) | GUI | Qt6 |
| [externals/Local_Externals_Media.md](externals/Local_Externals_Media.md) | Media | BASS |
| [externals/Local_Externals_Scripting.md](externals/Local_Externals_Scripting.md) | Scripting | Lua |
| [externals/Local_Externals_Testing.md](externals/Local_Externals_Testing.md) | Testing | doctest (local) |

---

## See Also

- [../README.md](../README.md) — Germane Dokumentation Overview
- [../blueprints/Reference.md](../blueprints/Reference.md) — Reference Blueprint
- [../userguides/](../userguides/README.md) — Praktische Anleitungen
