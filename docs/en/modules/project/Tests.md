# Tests.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Tests.md](../../en/modules/project/Tests.md)  
> **Module:** [`cmake/project/Tests.cmake`](../../../../cmake/project/Tests.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Aktivierung](#3-aktivierung)
4. [Verarbeitung](#4-verarbeitung)
5. [CTest-Integration](#5-ctest-integration)
6. [Debug-Ausgaben](#6-debug-ausgaben)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `Tests.cmake` Modul ist der **Orchestrator der Test-Pipeline**. Es iteriert über alle Tests in Solution.json und erstellt Test-Targets mit CTest-Integration.

### Kernidee

Tests werden nur bei Bedarf gebaut (BUILD_TESTS=ON) und sind vollständig in CTest integriert.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Aktivierung | BUILD_TESTS Flag prüfen |
| Iteration | Über tests-Array |
| Filter | skip, platforms |
| CTest | enable_testing(), add_test() |

### Lädt automatisch

- TestCollect.cmake
- TestCreate.cmake

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Context.cmake | 0.5.0 | `ctx_create`, `ctx_get` |
| Errors.cmake | 0.5.0 | (indirekt) |
| Debug.cmake | 0.5.0 | (indirekt) |
| Json.cmake | 0.5.0 | `_json_has_key`, JSON-Parsing |
| Solution.cmake | 0.5.0 | `SOLUTION_JSON` Property |

---

## 3. Aktivierung

### 3.1 CMake-Aufruf

```bash
# Tests aktivieren
cmake -B build -DBUILD_TESTS=ON

# Tests deaktivieren (Default)
cmake -B build -DBUILD_TESTS=OFF
```

### 3.2 Preset-Configuration

```json
{
    "configurePresets": [
        {
            "name": "dev",
            "cacheVariables": {
                "BUILD_TESTS": "ON"
            }
        }
    ]
}
```

---

## 4. Verarbeitung

### 4.1 Pipeline-Ablauf

```
Tests.cmake
    │
    ├── 1. BUILD_TESTS == OFF?
    │   └── Return early
    │
    ├── 2. Load Sub-Modules
    │   ├── TestCollect.cmake
    │   └── TestCreate.cmake
    │
    ├── 3. Get tests array from SOLUTION_JSON
    │   └── Return early if empty
    │
    ├── 4. Enable CTest
    │   ├── include(CTest)
    │   └── enable_testing()
    │
    └── 5. For each test:
        │
        ├── 5.1 Create Context (TEST_0, TEST_1, ...)
        │
        ├── 5.2 _collect_test() → Fill Context
        │
        ├── 5.3 Check Filters:
        │   ├── skip=true → Continue
        │   └── platforms → Not matching → Continue
        │
        └── 5.4 _create_test_target()
```

---

## 5. CTest-Integration

### 5.1 Tests ausführen

```bash
# Alle Tests
ctest --test-dir build

# Mit Output bei Errorn
ctest --test-dir build --output-on-failure

# Verbose
ctest --test-dir build -V
```

### 5.2 Tests filtern

```bash
# Nach Label
ctest -L unit
ctest -L integration

# Nach Name (Regex)
ctest -R "MyTest"
ctest -R ".*Core.*"
```

---

## 6. Debug-Ausgaben

```
-- [Tests] === Test Pipeline Start ===
-- [Tests] Processing 3 test(s)...
-- [Tests] --- Processing: CoreLibTests ---
-- [Tests]   Created: CoreLibTests
-- [Tests] --- Processing: UtilTests ---
-- [Tests]   Created: UtilTests
-- [Tests] --- Processing: WinOnlyTests ---
-- [Tests]   Skipped (platform filter)
-- 
-- [Tests] === Test Pipeline Complete ===
-- [Tests] Run tests with: ctest --test-dir <build-dir>
-- [Tests] Filter by label: ctest -L unit
-- [Tests] Filter by name:  ctest -R <pattern>
```

---

## 7. See Also

- [TestCollect.cmake](TestCollect.md) — JSON zu Context
- [TestCreate.cmake](TestCreate.md) — Context zu Target
- [Executables.cmake](Executables.md) — Analoges Modul
- [Testing_UserGuide.md](../../../guides/Testing_UserGuide.md) — Benutzerhandbuch

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0** |
| 0.1.0 | 2025-12-11 | Initial (Clean Start): Test-Pipeline mit CTest |
