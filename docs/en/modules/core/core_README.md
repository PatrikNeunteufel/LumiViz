# Core — Kern-Module

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../../en/modules/core/README.md)

---

## Quick-Start

**Error ausgeben?**
1. [Errors.md](Errors.md) — `cmake_fatal()`, `cmake_warn()`

**Debug-Output?**
1. [Debug.md](Debug.md) — `dbg()`, Debug-Level

**JSON parsen?**
1. [Json.md](Json.md) — `_json_get_string()`, `_json_get_number()`, etc.

**Context-Pattern?**
1. [Context.md](Context.md) — `ctx_create()`, `ctx_set()`, `ctx_get()`

---

## Overview

Die Core-Module bilden das Fundament des Build-Systems. Sie sind unabhängig von der Projekt-Pipeline und werden von allen anderen Modulen verwendet.

### Abhängigkeits-Hierarchie

```
Errors.cmake          ← Keine Dependencies (Basis)
     │
     ▼
Debug.cmake           ← Verwendet Errors
     │
     ▼
Context.cmake         ← Verwendet Debug
Json.cmake            ← Keine Dependencies
Validation.cmake      ← Verwendet Json, Errors
     │
     ▼
OutputDirs.cmake      ← Verwendet Debug
Warnings.cmake        ← Verwendet Debug
CompilerOptions.cmake ← Verwendet Debug, Json
SourceCollect.cmake   ← Verwendet Errors, Debug
```

---

## Dateien

| Datei | Description |
|-------|--------------|
| [Errors.md](Errors.md) | Zentralisierte Errormeldungen (`cmake_fatal`, `cmake_warn`) |
| [Debug.md](Debug.md) | Debug-Output-System mit Level und Context |
| [Context.md](Context.md) | Globaler Build-Context (GLOBAL PROPERTY Pattern) |
| [Json.md](Json.md) | JSON-Parsing-Functions |
| [Validation.md](Validation.md) | Schema-Validierung für Solution.json |
| [OutputDirs.md](OutputDirs.md) | Output-Verzeichnisse konfigurieren |
| [Warnings.md](Warnings.md) | Compiler-Warningen konfigurieren |
| [CompilerOptions.md](CompilerOptions.md) | Compiler-Optionen (Standard, Optimierung) |
| [SourceCollect.md](SourceCollect.md) | Source-Datei-Sammlung (GLOB/explicit) |

---

## Modul-Kategorien

### Errorbehandlung & Debugging

| Modul | Functions |
|-------|------------|
| **Errors** | `cmake_fatal()`, `cmake_warn()` |
| **Debug** | `dbg()`, `dbg_init()`, `enddbgblock()`, `dbgspace()` |

### Datenstrukturen

| Modul | Functions |
|-------|------------|
| **Context** | `ctx_create()`, `ctx_set()`, `ctx_get()`, `ctx_dump()` |
| **Json** | `_json_get_string()`, `_json_get_number()`, `_json_get_bool_from_key()`, etc. |
| **Validation** | Schema-Prüfung, Requiredfeld-Validierung |

### Build-Configuration

| Modul | Functions |
|-------|------------|
| **OutputDirs** | `setup_output_dirs()` |
| **Warnings** | `apply_warnings()` |
| **CompilerOptions** | `apply_compiler_options()` |
| **SourceCollect** | `collect_sources()`, `collect_files()` |

---

## Debug-Level

Das Debug-System verwendet 5 Level:

| Level | Konstante | Usage |
|-------|-----------|------------|
| 1 | `DBG_OFTEN` | Phasen-Start/Ende |
| 2 | `DBG_COMMON` | Importante Schritte |
| 3 | `DBG_NORMAL` | Details |
| 4 | `DBG_RARE` | Selten benötigt |
| 5 | `DBG_ULTRA_RARE` | Loop-Iterationen |

```cmake
dbg(${DBG_COMMON} "Processing ${_name}" ID MY_MODULE)
```

---

## Error Codes

Core-Module verwenden die Bereiche:

| Bereich | Codes | Modul |
|---------|-------|-------|
| E0xx | E001-E099 | Allgemein (Errors, Validation) |
| E1xx | E100-E199 | Context, SourceCollect |
| W0xx | W001-W099 | Warningen (Errors) |
| W1xx | W100-W199 | Warningen (SourceCollect, Warnings) |

→ Vollständige Liste: [../../references/ErrorCodes.md](../../references/ErrorCodes.md)

---

## See Also

- [../README.md](../README.md) — Modul-Overview
- [../project/README.md](../project/README.md) — Projekt-Module
- [../../references/ErrorCodes.md](../../references/ErrorCodes.md) — Errorcode-Reference
- [../../projects/buildsystem/standards/Guidelines.md](../../projects/buildsystem/standards/Guidelines.md) — CMake-Coding-Standards
