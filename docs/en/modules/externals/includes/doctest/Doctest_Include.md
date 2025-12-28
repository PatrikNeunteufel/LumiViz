# doctest/Include.cmake — doctest Testing Framework Integration

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers, C++ Developers  
> **Language:** English  
> **German:** [doctest_Include.md](../../en/modules/externals/includes/doctest_Include.md)  
> **Module:** [cmake/externals/includes/doctest/Include.cmake](../../../../cmake/externals/includes/doctest/Include.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Verfügbare Variablen](#2-verfügbare-variablen)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Options](#4-options)
5. [Usagesbeispiele](#5-verwendungsbeispiele)
6. [Test-Patterns](#6-test-patterns)
7. [Verzeichnisstruktur](#7-verzeichnisstruktur)
8. [Errorbehandlung](#8-fehlerbehandlung)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

Die `doctest/Include.cmake` integriert das **doctest Testing Framework** als lokales External in das Build-System.

### Kernfunktionen

| Funktion | Description |
|----------|--------------|
| Header-Only | Nur ein Header, keine Library nötig |
| Schnell | Extrem schnelle Kompilierung |
| Lightweight | Minimaler Overhead |
| BDD-Style | Subcases für verschachtelte Tests |

### Target-Erstellung

Erstellt ein `INTERFACE` Target `doctest` das nur Include-Directories bereitstellt.

---

## 2. Verfügbare Variablen

Diese Variablen werden vom Orchestrator bereitgestellt:

| Variable | Description |
|----------|--------------|
| `EXTERNAL_NAME` | `"doctest"` |
| `EXTERNAL_PATH` | Absoluter Pfad zu `externals/doctest` |
| `EXTERNAL_JSON` | JSON-Element aus Solution.json |
| `EXTERNAL_OPTIONS` | Target-spezifische Options (JSON) |

---

## 3. Erstellte Targets

| Target | Typ | Description |
|--------|-----|--------------|
| `doctest` | INTERFACE | Header-only Include-Directory |

### Include-Directories

```cmake
target_include_directories(doctest INTERFACE
    "${EXTERNAL_PATH}/include"
)
```

---

## 4. Options

doctest ist Header-only und benötigt keine spezifischen CMake-Options.

### Compile-Time Configuration

Diese Defines können in C++ vor `#include <doctest.h>` gesetzt werden:

| Define | Description |
|--------|--------------|
| `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` | main() automatisch generieren |
| `DOCTEST_CONFIG_IMPLEMENT` | Nur Implementation, kein main() |
| `DOCTEST_CONFIG_DISABLE` | Tests komplett deaktivieren |
| `DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES` | Nur `DOCTEST_*` Makros |

---

## 5. Usagesbeispiele

### 5.1 Einfacher Unit Test

**Solution.json:**

```json
{
    "externals": {
        "doctest": { "path": "externals/doctest" }
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

**test_main.cpp:**

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Math operations") {
    CHECK(1 + 1 == 2);
    CHECK(2 * 3 == 6);
    CHECK(10 / 2 == 5);
}
```

### 5.2 Subcases (BDD-Style)

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

### 5.3 Test mit Fixtures

```cpp
#include <doctest/doctest.h>
#include <vector>

TEST_CASE("Vector tests") {
    std::vector<int> vec;
    
    REQUIRE(vec.empty());
    
    SUBCASE("push_back") {
        vec.push_back(1);
        vec.push_back(2);
        
        CHECK(vec.size() == 2);
        CHECK(vec[0] == 1);
        CHECK(vec[1] == 2);
    }
    
    SUBCASE("reserve") {
        vec.reserve(100);
        
        CHECK(vec.capacity() >= 100);
        CHECK(vec.empty());  // Size ist noch 0
    }
}
```

### 5.4 Eigene main() Funktion

```cpp
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char** argv) {
    doctest::Context context;
    
    // Defaults
    context.setOption("abort-after", 5);
    context.setOption("order-by", "name");
    
    // Kommandozeilen-Argumente
    context.applyCommandLine(argc, argv);
    
    int result = context.run();
    
    if (context.shouldExit()) {
        return result;
    }
    
    // Eigene Logik hier...
    
    return result;
}
```

### 5.5 Tests in Produktionscode deaktivieren

```cpp
// In Release-Builds: Tests komplett entfernen
#ifdef NDEBUG
    #define DOCTEST_CONFIG_DISABLE
#endif

#include <doctest/doctest.h>

// Diese Tests existieren nur in Debug-Builds
TEST_CASE("Debug-only test") {
    CHECK(1 == 1);
}
```

---

## 6. Test-Patterns

### 6.1 Assertions

| Assertion | Description |
|-----------|--------------|
| `CHECK(expr)` | Prüft, fährt bei Error fort |
| `REQUIRE(expr)` | Prüft, bricht bei Error ab |
| `CHECK_EQ(a, b)` | Gleichheit mit Details |
| `CHECK_NE(a, b)` | Ungleichheit |
| `CHECK_LT(a, b)` | Kleiner als |
| `CHECK_GT(a, b)` | Größer als |
| `CHECK_THROWS(expr)` | Exception erwartet |
| `CHECK_NOTHROW(expr)` | Keine Exception erwartet |

### 6.2 Test-Organisation

```cpp
TEST_SUITE("Math") {
    TEST_CASE("Addition") {
        CHECK(1 + 1 == 2);
    }
    
    TEST_CASE("Subtraction") {
        CHECK(5 - 3 == 2);
    }
}

TEST_SUITE("Strings") {
    TEST_CASE("Concatenation") {
        CHECK(std::string("a") + "b" == "ab");
    }
}
```

### 6.3 Parametrisierte Tests

```cpp
TEST_CASE_TEMPLATE("Type traits", T, int, float, double) {
    T value = T(42);
    CHECK(value == T(42));
}
```

---

## 7. Verzeichnisstruktur

```
externals/doctest/
└── include/
    └── doctest/
        └── doctest.h
```

> **Note:** doctest besteht nur aus einem einzigen Header!

---

## 8. Errorbehandlung

| Code | Description |
|------|--------------|
| E213 | Include.cmake nicht gefunden |
| E214 | External-Pfad existiert nicht |

### Häufige Error

**"doctest.h not found":**
```cpp
// Falsch:
#include <doctest.h>

// Richtig:
#include <doctest/doctest.h>
```

**"Undefined reference to main":**
```cpp
// Fehlt:
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
```

---

## 9. See Also

- [Testing_UserGuide.md](../../../guides/Testing_UserGuide.md) — Test-Anleitung
- [Solution_Schema.md](../../../reference/Solution_Schema.md) — tests Array
- [Attach_cmake.md](../Attach_cmake.md) — Local External Handler
- [doctest GitHub](https://github.com/doctest/doctest) — Offizielle Dokumentation

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0, Convention-Pfad `cmake/externals/includes/doctest/`** |
| 0.1.0 | 2025-12-05 | Initial: Header-only INTERFACE Target |
