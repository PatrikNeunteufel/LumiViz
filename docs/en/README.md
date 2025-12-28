# Germane Dokumentation — CMake Architecture

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../en/README.md)

---

## Quick-Start

**Neu im Projekt?** Hier ist dein Einstieg:

1. [userguides/Getting_Started.md](userguides/Getting_Started.md) — Erste Schritte, Projekt einrichten
2. [references/Glossar.md](references/Glossar.md) — Begriffe klären
3. [references/Solution_Schema.md](references/Solution_Schema.md) — JSON-Configuration verstehen

**Build-System entwickeln?**
1. [modules/core/Errors.md](modules/core/Errors.md) — Errorbehandlung (Basis für alles)
2. [modules/core/Context.md](modules/core/Context.md) — Globaler State
3. [projects/buildsystem/concepts/Master_Concept.md](projects/buildsystem/concepts/Master_Concept.md) — Gesamtarchitektur

**Dokumentation schreiben?**
1. [blueprints/Doc.md](blueprints/Doc.md) — Grundregeln für alle Dokumente
2. [blueprints/README_Blueprint.md](blueprints/README_Blueprint.md) — README-Dateien erstellen

**Externe Libraries nutzen?**
1. [userguides/Externals.md](userguides/Externals.md) — Externals-Overview
2. [userguides/Adding_Externals.md](userguides/Adding_Externals.md) — Neue Externals hinzufügen

---

## Overview

Die deutsche Dokumentation ist die Primärsprache für das CMake Architecture Projekt. Sie umfasst alle technischen Spezifikationen, Benutzerhandbücher und Referencematerialien.

| Bereich | Anzahl Dokumente | Description |
|---------|------------------|--------------|
| Blueprints | 12 | Vorlagen und Standards |
| Modules | 30+ | CMake-Modul-Dokumentation |
| Projects | 7 | Projektspezifische Docs |
| References | 20+ | Nachschlagewerke |
| Standards | 5 | Coding-Standards |
| User Guides | 15+ | Anleitungen |

---

## Unterordner

| Ordner | Description |
|--------|--------------|
| [blueprints/](blueprints/README.md) | Vorlagen für Dokumentation und Code (12 Blueprints) |
| [modules/](modules/README.md) | CMake-Modul-Dokumentation (Core, Project, Externals) |
| [projects/](projects/README.md) | Projektspezifische Dokumentation |
| [references/](references/README.md) | API-Referenceen, Schemas, Errorcodes |
| [standards/](standards/README.md) | Coding- und Projekt-Standards (5 Standards) |
| [userguides/](userguides/README.md) | Benutzerhandbücher und How-Tos |

---

## Importante Einstiegspunkte

### Für C++ Developers (End Users)

| Dokument | Description |
|----------|--------------|
| [userguides/Getting_Started.md](userguides/Getting_Started.md) | Projekt einrichten |
| [userguides/Testing.md](userguides/Testing.md) | Tests schreiben |
| [references/ErrorCodes.md](references/ErrorCodes.md) | Errormeldungen verstehen |

### Für Build System Developers

| Dokument | Description |
|----------|--------------|
| [projects/buildsystem/concepts/Master_Concept.md](projects/buildsystem/concepts/Master_Concept.md) | Gesamtarchitektur |
| [projects/buildsystem/concepts/Implementation_Plan.md](projects/buildsystem/concepts/Implementation_Plan.md) | Phasen-Planung |
| [modules/CMakeLists.md](modules/CMakeLists.md) | Root CMakeLists.txt |

### Für Documentation Authors

| Dokument | Description |
|----------|--------------|
| [blueprints/Doc.md](blueprints/Doc.md) | Basis-Regeln |
| [blueprints/Structure.md](blueprints/Structure.md) | Ordnerstruktur |
| [standards/Language_Standard.md](standards/Language_Standard.md) | Sprachkonventionen |

---

## See Also

- [../README.md](../README.md) — Dokumentations-Root (EN)
- [blueprints/Blueprint.md](blueprints/Blueprint.md) — Wie Dokumentationen aufgebaut sind
