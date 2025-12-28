# phase7.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Path:** `cmake/buildSystemTest/phase7.cmake`  
> **Status:** Stable  
> **Language:** English  

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Tests](#2-tests)
3. [Successs-Flag](#3-erfolgs-flag)
4. [See Also](#4-siehe-auch)
5. [Changelog](#5-changelog)

---

## 1. Overview

**Phase 7** testet die **Test Pipeline** — das Erstellen von Test-Targets und CTest-Integration.

| Aspekt | Description |
|--------|--------------|
| **Zweck** | Test Pipeline Validation |
| **Debug-ID** | `PHASE7_TEST` |
| **Dependencies** | Phase 1-6, Tests.cmake |
| **Bedingung** | Läuft vollständig nur wenn `BUILD_TESTS=ON` |

---

## 2. Tests

### 2.1 BUILD_TESTS Option

```cmake
if(BUILD_TESTS)
    # Vollständige Tests
else()
    # Nur JSON-Parsing Tests
endif()
```

### 2.2 Tests Array Parsing

Prüft `tests` Array in Solution.json:

```cmake
string(JSON _tests_json GET "${_solution_json}" "tests")
string(JSON _tests_count LENGTH "${_tests_json}")
```

| Feld | Description |
|------|--------------|
| `name` | Test-Name |
| `type` | unit, integration, system, etc. |
| `framework` | doctest, googletest, catch2 |

### 2.3 Framework Externals

Prüft, dass Test-Framework-Externals definiert sind:

| Framework | Typ |
|-----------|-----|
| `doctest` | Local |
| `googletest` | Git (Fetched) |
| `catch2` | Git (Fetched) |

### 2.4 Test Targets

Prüft Target-Erstellung (nur wenn `BUILD_TESTS=ON`):

| Target | Framework |
|--------|-----------|
| `BasicLogger_UnitTests` | doctest |
| `AudioEngine_UnitTests` | googletest |
| `Integration_Tests` | catch2 |

### 2.5 CTest Integration

```cmake
if(CMAKE_TESTING_ENABLED)
    # CTest ist aktiviert
endif()
```

Prüft Test-Properties:

| Property | Description |
|----------|--------------|
| `TIMEOUT` | Test-Timeout in Sekunden |
| `LABELS` | Test-Labels für Filterung |

### 2.6 Test Types

Unterstützte Test-Typen:

| Type | Description |
|------|--------------|
| `unit` | Unit Tests |
| `integration` | Integration Tests |
| `system` | System Tests |
| `performance` | Performance Tests |
| `smoke` | Smoke Tests |

### 2.7 Framework Targets

Prüft Framework-Library-Targets (wenn `BUILD_TESTS=ON`):

| Target | Framework |
|--------|-----------|
| `doctest` | doctest |
| `gtest_main` | GoogleTest |
| `gmock_main` | GoogleMock |
| `Catch2WithMain` | Catch2 |

---

## 3. Successs-Flag

```cmake
set(PHASE7_TEST_PASSED TRUE CACHE BOOL "Phase 7 Test passed" FORCE)
```

---

## 4. See Also

- [Tests.md](../modules/project/Tests.md)
- [TestCollect.md](../modules/project/TestCollect.md)
- [TestCreate.md](../modules/project/TestCreate.md)
- [Git_Externals_Testing.md](../reference/Git_Externals_Testing.md)
- [Local_Externals_Testing.md](../reference/Local_Externals_Testing.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-12 | Initial |
