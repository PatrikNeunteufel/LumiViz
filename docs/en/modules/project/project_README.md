# Project — Projekt-Pipeline-Module

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../../en/modules/project/README.md)

---

## Quick-Start

**Executable erstellen?**
1. [Executables.md](Executables.md) — Pipeline-Overview
2. [ExecutableCollect.md](ExecutableCollect.md) — JSON-Parsing
3. [ExecutableCreate.md](ExecutableCreate.md) — Target-Erstellung

**Library erstellen?**
1. [Libraries.md](Libraries.md) — Pipeline-Overview
2. [LibraryCollect.md](LibraryCollect.md) — JSON-Parsing
3. [LibraryCreate.md](LibraryCreate.md) — Target-Erstellung

**App-Container erstellen?** (Phase 8)
1. [Apps.md](Apps.md) — Pipeline-Overview
2. [AppCollect.md](AppCollect.md) — JSON-Parsing
3. [AppCreate.md](AppCreate.md) — Target-Erstellung (Core/Runner/Tests)

**Tests definieren?**
1. [Tests.md](Tests.md) — Test-Pipeline
2. [TestCollect.md](TestCollect.md) — JSON-Parsing
3. [TestCreate.md](TestCreate.md) — Test-Target-Erstellung

---

## Overview

Die Project-Module bilden die Hauptpipeline des Build-Systems. Sie verarbeiten die `Solution.json` und erstellen CMake-Targets für Executables, Libraries, Tests und App-Container.

### Pipeline-Architecture

```
Solution.json
     │
     ▼
Solution.cmake          ← Lädt und parst Solution.json
     │
     ├── Externals.cmake      ← External-Dependencies
     │
     ├── Libraries.cmake      ← Library-Pipeline
     │   ├── LibraryCollect   → JSON → Context
     │   └── LibraryCreate    → Context → Target
     │
     ├── Executables.cmake    ← Executable-Pipeline
     │   ├── ExecutableCollect → JSON → Context
     │   └── ExecutableCreate  → Context → Target
     │
     ├── Apps.cmake           ← App-Container-Pipeline (Phase 8)
     │   ├── AppCollect       → JSON → Context
     │   └── AppCreate        → Core/Runner/Tests Targets
     │
     └── Tests.cmake          ← Test-Pipeline
         ├── TestCollect      → JSON → Context
         └── TestCreate       → Context → Test Target
```

---

## Dateien

### Solution & Externals

| Datei | Phase | Description |
|-------|-------|--------------|
| [Solution.md](Solution.md) | 2 | Solution.json laden und parsen |
| [Externals.md](Externals.md) | 5-6 | External-Integration orchestrieren |

### Executables (Phase 3)

| Datei | Description |
|-------|--------------|
| [Executables.md](Executables.md) | Executable-Pipeline-Orchestrator |
| [ExecutableCollect.md](ExecutableCollect.md) | JSON-Parsing → Context |
| [ExecutableCreate.md](ExecutableCreate.md) | Context → CMake Target |

### Libraries (Phase 4)

| Datei | Description |
|-------|--------------|
| [Libraries.md](Libraries.md) | Library-Pipeline-Orchestrator |
| [LibraryCollect.md](LibraryCollect.md) | JSON-Parsing → Context |
| [LibraryCreate.md](LibraryCreate.md) | Context → CMake Target |

### Tests (Phase 7)

| Datei | Description |
|-------|--------------|
| [Tests.md](Tests.md) | Test-Pipeline-Orchestrator |
| [TestCollect.md](TestCollect.md) | JSON-Parsing → Context |
| [TestCreate.md](TestCreate.md) | Context → Test Target |

### App-Container (Phase 8)

| Datei | Description |
|-------|--------------|
| [Apps.md](Apps.md) | App-Container-Pipeline-Orchestrator |
| [AppCollect.md](AppCollect.md) | JSON-Parsing → Context |
| [AppCreate.md](AppCreate.md) | Context → Core/Runner/Tests Targets |

---

## Pipeline-Pattern

Alle Pipelines folgen dem gleichen Collect/Create-Pattern:

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   JSON      │ ──▶ │   Context   │ ──▶ │   Target    │
│ (Solution)  │     │  (Collect)  │     │  (Create)   │
└─────────────┘     └─────────────┘     └─────────────┘
```

| Phase | Orchestrator | Collect | Create |
|-------|--------------|---------|--------|
| 3 | Executables.cmake | ExecutableCollect | ExecutableCreate |
| 4 | Libraries.cmake | LibraryCollect | LibraryCreate |
| 7 | Tests.cmake | TestCollect | TestCreate |
| 8 | Apps.cmake | AppCollect | AppCreate |

---

## App-Container vs. Executable

| Aspekt | Executable | App-Container |
|--------|------------|---------------|
| **Targets** | 1 (Executable) | 3+ (Core, Runner, Tests) |
| **Testbarkeit** | Eingeschränkt | Vollständig (Core testbar) |
| **Struktur** | Flach | Hierarchisch (src/, main/, tests/) |
| **Use Case** | Einfache Tools | Komplexe Anwendungen |

---

## See Also

- [../README.md](../README.md) — Modul-Overview
- [../core/README.md](../core/README.md) — Core-Module
- [../../references/Solution_Schema.md](../../references/Solution_Schema.md) — JSON-Schema
- [../../projects/buildsystem/concepts/AppContainer_Concept.md](../../projects/buildsystem/concepts/AppContainer_Concept.md) — App-Container-Concept
