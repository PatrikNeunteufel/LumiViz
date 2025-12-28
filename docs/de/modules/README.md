# Module — CMake-Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Sprache:** Deutsch  
> **English:** [README.md](../../en/modules/README.md)

---

## Quick-Start

**Das Build-System verstehen?** Hier sind alle CMake-Module dokumentiert.

Starte mit:
1. [core/](core/README.md) — Kern-Module (Errors, Context, Debug)
2. [project/](project/README.md) — Projekt-Pipeline (Executables, Libraries, Tests)

**Externes verwalten?**
1. [externals/](externals/README.md) — External-Management-System

---

## Übersicht

Die Modul-Dokumentation beschreibt alle CMake-Module des Build-Systems. Jedes Modul hat eine standardisierte Dokumentation nach dem [ModuleDoc Blueprint](../blueprints/ModuleDoc.md).

### Modul-Kategorien

| Kategorie | Beschreibung | Module |
|-----------|--------------|--------|
| **Core** | Grundlegende Funktionen | 9 Module |
| **Project** | Projekt-Erstellung | 14 Module |
| **Externals** | Dependency-Management | 8+ Module |

---

## Dateien

| Datei | Beschreibung |
|-------|--------------|
| [CMakeLists.md](CMakeLists.md) | Root-CMakeLists.txt Dokumentation |

---

## Unterordner

| Ordner | Beschreibung |
|--------|--------------|
| [buildSystemTest/](buildSystemTest/README.md) | Phasen-Testdokumentation (Phase 1-8) |
| [core/](core/README.md) | Kern-Module (Errors, Debug, Context, etc.) |
| [externals/](externals/README.md) | External-Management-System |
| [project/](project/README.md) | Projekt-Pipeline (Executables, Libraries, Apps, Tests) |

---

## Modul-Übersicht

### Core-Module (Phase 1)

```
core/
├── Errors.cmake        → Zentralisierte Fehlermeldungen
├── Debug.cmake         → Debug-Output-System
├── Context.cmake       → Globaler Build-Context
├── Json.cmake          → JSON-Parsing
├── Validation.cmake    → Schema-Validierung
├── OutputDirs.cmake    → Output-Verzeichnisse
├── Warnings.cmake      → Compiler-Warnungen
├── CompilerOptions.cmake → Compiler-Optionen
└── SourceCollect.cmake → Source-Sammlung
```

### Project-Module (Phase 2-8)

```
project/
├── Solution.cmake           → Projekt-Initialisierung
├── Executables.cmake        → Executable-Pipeline
├── ExecutableCollect.cmake  → Executable-Discovery
├── ExecutableCreate.cmake   → Executable-Erstellung
├── Libraries.cmake          → Library-Pipeline
├── LibraryCollect.cmake     → Library-Discovery
├── LibraryCreate.cmake      → Library-Erstellung
├── Tests.cmake              → Test-Pipeline
├── TestCollect.cmake        → Test-Discovery
├── TestCreate.cmake         → Test-Erstellung
├── Apps.cmake               → App-Container-Pipeline (Phase 8)
├── AppCollect.cmake         → App-Discovery
├── AppCreate.cmake          → App-Erstellung (Core/Runner/Tests)
└── Externals.cmake          → Externals-Integration
```

### Externals-Module (Phase 4-5)

```
externals/
├── Orchestrator.cmake       → Haupt-Orchestrierung
├── core/Fetch.cmake         → Git-Fetch
├── fetched/Handler.cmake    → Fetched-Library-Handler
├── locals/Attach.cmake      → Local-Library-Attach
├── registry/Targets.cmake   → Target-Registry
├── hooks/HookLoader.cmake   → Hook-System
└── includes/                → Library-Include-Files
```

---

## Siehe auch

- [../blueprints/ModuleDoc.md](../blueprints/ModuleDoc.md) — ModuleDoc Blueprint
- [../references/ErrorCodes.md](../references/ErrorCodes.md) — Fehlercodes
