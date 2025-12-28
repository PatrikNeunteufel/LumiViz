# Local Externals — Testing

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Local_Externals_Testing.md](../../en/references/externals/Local_Externals_Testing.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [doctest Testing Framework](#2-doctest-testing-framework)
3. [See Also](#3-siehe-auch)
4. [Changelog](#4-changelog)

---

## 1. Overview

This document describes lokale Testing-Externals für das CMake Architecture Build-System.

| Library | Description | Lizenz |
|---------|--------------|--------|
| **doctest** | Schnelles Header-Only Testing Framework | MIT |

---

## 2. doctest Testing Framework

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Typ** | 📄 Header-Only |
| **Pfad** | `externals/doctest` |
| **Include.cmake** | `cmake/externals/includes/doctest/Include.cmake` |
| **Plattformen** | Alle |
| **GitHub** | [doctest/doctest](https://github.com/doctest/doctest) |

### Warum doctest?

| Vorteil | Description |
|---------|--------------|
| **Schnell** | Extrem schnelle Kompilierung |
| **Header-Only** | Nur ein Header, keine Library |
| **Lightweight** | Minimaler Overhead |
| **BDD-Style** | Subcases für verschachtelte Tests |

### Solution.json

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    },
    "tests": [
        {
            "name": "MyTests",
            "framework": "doctest",
            "externals": ["doctest"]
        }
    ]
}
```

### Verzeichnisstruktur

```
externals/doctest/
└── include/
    └── doctest/
        └── doctest.h
```

> **Note:** doctest besteht nur aus einem einzigen Header!

### Usagesbeispiel

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Math operations") {
    CHECK(1 + 1 == 2);
    CHECK(2 * 3 == 6);
    CHECK(10 / 2 == 5);
}
```

### Subcases (BDD-Style)

```cpp
#include <doctest/doctest.h>
#include <string>

TEST_CASE("String operations") {
    std::string s = "Hello";
    
    SUBCASE("append") {
        s += " World";
        CHECK(s == "Hello World");
    }
    
    SUBCASE("length") {
        CHECK(s.length() == 5);
    }
    
    SUBCASE("clear") {
        s.clear();
        CHECK(s.empty());
    }
}
```

### Assertions

| Assertion | Description |
|-----------|--------------|
| `CHECK(expr)` | Prüft, fährt bei Error fort |
| `REQUIRE(expr)` | Prüft, bricht bei Error ab |
| `CHECK_EQ(a, b)` | Gleichheit mit Details |
| `CHECK_NE(a, b)` | Ungleichheit |
| `CHECK_THROWS(expr)` | Exception erwartet |
| `CHECK_NOTHROW(expr)` | Keine Exception erwartet |

### Test-Suites

```cpp
TEST_SUITE("Math") {
    TEST_CASE("Addition") {
        CHECK(1 + 1 == 2);
    }
    
    TEST_CASE("Subtraction") {
        CHECK(5 - 3 == 2);
    }
}
```

### Compile-Time Configuration

| Define | Description |
|--------|--------------|
| `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` | main() automatisch generieren |
| `DOCTEST_CONFIG_IMPLEMENT` | Nur Implementation, kein main() |
| `DOCTEST_CONFIG_DISABLE` | Tests komplett deaktivieren |

### Vergleich mit Git-Alternativen

| Feature | doctest (Local) | googletest (Git) | catch2 (Git) |
|---------|-----------------|------------------|--------------|
| **Header-Only** | ✅ | ❌ | ✅ |
| **Kompilierzeit** | ⚡ Sehr schnell | 🐢 Langsam | 🐌 Langsamer |
| **Mocking** | ❌ | ✅ GMock | ❌ |
| **BDD Subcases** | ✅ | ❌ | ✅ |
| **Footprint** | Minimal | Groß | Mittel |

### Detail-Dokumentation

→ [doctest_Include.md](../../modules/externals/includes/doctest/Doctest_Include.md)

---

## 3. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Local_Externals.md](Local_Externals.md) — Local Externals Overview
- [Git_Externals_Testing.md](Git_Externals_Testing.md) — Git Testing-Externals (googletest, catch2)
- [Testing_UserGuide.md](../../userguides/Testing.md) — Test-Anleitung

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Eigene Kategorie Testing (parallel zu Git)** |
