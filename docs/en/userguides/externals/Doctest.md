# doctest — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [doctest.md](../../../en/userguides/externals/Doctest.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Installation](#2-installation)
3. [Solution.json Configuration](#3-solutionjson-konfiguration)
4. [C++ Usage](#4-c-verwendung)
5. [Fortgeschrittene Techniken](#5-fortgeschrittene-techniken)
6. [Best Practices](#6-best-practices)
7. [Troubleshooting](#7-troubleshooting)
8. [Weiterführende Informationen](#8-weiterführende-informationen)
9. [Changelog](#9-changelog)

---

## 1. Overview

**doctest** ist ein schnelles, Header-Only C++ Testing Framework mit minimaler Kompilierzeit.

| Aspekt | Wert |
|--------|------|
| **Typ** | Local External (Header-Only) |
| **Pfad** | `externals/doctest` |
| **Lizenz** | MIT |
| **Website** | [github.com/doctest/doctest](https://github.com/doctest/doctest) |

### Warum doctest?

| Vorteil | Description |
|---------|--------------|
| ⚡ **Schnell** | Extrem schnelle Kompilierung |
| 📄 **Header-Only** | Nur ein Header, keine Library |
| 🪶 **Lightweight** | Minimaler Overhead (~17KB) |
| 🔀 **Subcases** | BDD-Style verschachtelte Tests |

---

## 2. Installation

### 2.1 Download

```bash
# Von GitHub
git clone https://github.com/doctest/doctest.git externals/doctest

# Oder nur den Header
mkdir -p externals/doctest/include/doctest
curl -o externals/doctest/include/doctest/doctest.h \
  https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h
```

### 2.2 Verzeichnisstruktur

```
externals/doctest/
└── include/
    └── doctest/
        └── doctest.h
```

---

## 3. Solution.json Configuration

### 3.1 Minimal

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    },
    "tests": [
        {
            "name": "UnitTests",
            "framework": "doctest",
            "externals": ["doctest"]
        }
    ]
}
```

### 3.2 Mit Library-Tests

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    },
    "libraries": [
        {
            "name": "CoreLib",
            "type": "static"
        }
    ],
    "tests": [
        {
            "name": "CoreTests",
            "framework": "doctest",
            "externals": ["doctest"],
            "libraries": ["CoreLib"]
        }
    ]
}
```

### 3.3 Mehrere Test-Suites

```json
{
    "tests": [
        {
            "name": "UnitTests",
            "framework": "doctest",
            "externals": ["doctest"],
            "sources": ["tests/unit"]
        },
        {
            "name": "IntegrationTests",
            "framework": "doctest",
            "externals": ["doctest"],
            "sources": ["tests/integration"]
        }
    ]
}
```

---

## 4. C++ Usage

### 4.1 Einfacher Test

```cpp
// tests/test_math.cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Addition") {
    CHECK(1 + 1 == 2);
    CHECK(2 + 3 == 5);
    CHECK(-1 + 1 == 0);
}

TEST_CASE("Subtraction") {
    CHECK(5 - 3 == 2);
    CHECK(0 - 1 == -1);
}
```

### 4.2 Assertions

| Assertion | Description | Bei Error |
|-----------|--------------|------------|
| `CHECK(expr)` | Prüft Expression | Weiter |
| `REQUIRE(expr)` | Prüft Expression | Abbruch |
| `CHECK_EQ(a, b)` | Gleichheit | Weiter |
| `CHECK_NE(a, b)` | Ungleichheit | Weiter |
| `CHECK_LT(a, b)` | a < b | Weiter |
| `CHECK_GT(a, b)` | a > b | Weiter |
| `CHECK_LE(a, b)` | a <= b | Weiter |
| `CHECK_GE(a, b)` | a >= b | Weiter |
| `CHECK_THROWS(expr)` | Exception erwartet | Weiter |
| `CHECK_NOTHROW(expr)` | Keine Exception | Weiter |

```cpp
TEST_CASE("Various assertions") {
    int x = 42;
    
    CHECK(x == 42);
    CHECK_EQ(x, 42);
    CHECK_NE(x, 0);
    CHECK_GT(x, 0);
    CHECK_LE(x, 100);
    
    CHECK_THROWS(throw std::runtime_error("error"));
    CHECK_NOTHROW(x = 1);
}
```

### 4.3 Subcases (BDD-Style)

Subcases ermöglichen verschachtelte Tests mit gemeinsamen Setup:

```cpp
TEST_CASE("String operations") {
    std::string s = "Hello";  // Gemeinsames Setup
    
    SUBCASE("append") {
        s += " World";
        CHECK(s == "Hello World");
        CHECK(s.length() == 11);
    }
    
    SUBCASE("clear") {
        s.clear();
        CHECK(s.empty());
        CHECK(s.length() == 0);
    }
    
    SUBCASE("substring") {
        SUBCASE("from start") {
            CHECK(s.substr(0, 2) == "He");
        }
        SUBCASE("from middle") {
            CHECK(s.substr(2, 2) == "ll");
        }
    }
}
```

**Ausführungsreihenfolge:**
```
String operations → append
String operations → clear
String operations → substring → from start
String operations → substring → from middle
```

### 4.4 Test Suites

```cpp
TEST_SUITE("Math") {
    TEST_CASE("Addition") {
        CHECK(1 + 1 == 2);
    }
    
    TEST_CASE("Multiplication") {
        CHECK(2 * 3 == 6);
    }
}

TEST_SUITE("Strings") {
    TEST_CASE("Concatenation") {
        CHECK(std::string("a") + "b" == "ab");
    }
}
```

### 4.5 Fixtures

```cpp
struct DatabaseFixture {
    DatabaseFixture() {
        // Setup: Wird vor jedem TEST_CASE_FIXTURE aufgerufen
        db.connect("test.db");
    }
    
    ~DatabaseFixture() {
        // Teardown: Wird nach jedem TEST_CASE_FIXTURE aufgerufen
        db.disconnect();
    }
    
    Database db;
};

TEST_CASE_FIXTURE(DatabaseFixture, "Insert record") {
    db.insert("key", "value");
    CHECK(db.get("key") == "value");
}

TEST_CASE_FIXTURE(DatabaseFixture, "Delete record") {
    db.insert("key", "value");
    db.remove("key");
    CHECK_FALSE(db.exists("key"));
}
```

### 4.6 Parametrisierte Tests

```cpp
TEST_CASE("Parametersized test") {
    std::vector<std::pair<int, int>> test_data = {
        {1, 1}, {2, 4}, {3, 9}, {4, 16}
    };
    
    for (const auto& [input, expected] : test_data) {
        CAPTURE(input);  // Zeigt Wert bei Error
        CHECK(input * input == expected);
    }
}
```

### 4.7 Exception Testing

```cpp
void might_throw(int x) {
    if (x < 0) throw std::invalid_argument("negative");
    if (x > 100) throw std::out_of_range("too large");
}

TEST_CASE("Exception handling") {
    CHECK_THROWS(might_throw(-1));
    CHECK_THROWS_AS(might_throw(-1), std::invalid_argument);
    CHECK_THROWS_WITH(might_throw(-1), "negative");
    
    CHECK_NOTHROW(might_throw(50));
}
```

---

## 5. Fortgeschrittene Techniken

### 5.1 Eigene main()-Funktion

```cpp
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char** argv) {
    doctest::Context context;
    
    // Defaults setzen
    context.setOption("abort-after", 5);
    context.setOption("order-by", "name");
    
    // Command-Line parsen
    context.applyCommandLine(argc, argv);
    
    // Tests ausführen
    int res = context.run();
    
    // Cleanup
    if (context.shouldExit()) {
        return res;
    }
    
    // Eigener Code nach Tests
    return res;
}
```

### 5.2 Tests in Produktionscode deaktivieren

```cpp
// In Release-Builds keine Tests kompilieren
#ifdef NDEBUG
#define DOCTEST_CONFIG_DISABLE
#endif

#include <doctest/doctest.h>

// Diese Tests werden in Release-Builds ignoriert
TEST_CASE("Debug only test") {
    CHECK(1 == 1);
}
```

### 5.3 Test-Filter (Command Line)

```bash
# Nur Tests mit "Math" im Namen
./tests --test-case="*Math*"

# Tests ausschließen
./tests --test-case-exclude="*Slow*"

# Bestimmte Suite
./tests --test-suite="Unit"

# Verbose Output
./tests --success

# Liste aller Tests
./tests --list-test-cases
```

### 5.4 Reporter

```bash
# Standard (Console)
./tests

# Compact
./tests --reporters=compact

# XML (für CI)
./tests --reporters=xml --out=results.xml
```

---

## 6. Best Practices

### 6.1 Dateiorganisation

```
project/
├── src/
│   └── math.cpp
├── include/
│   └── math.h
└── tests/
    ├── test_main.cpp      # Nur DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    ├── test_math.cpp      # Tests ohne main()
    └── test_strings.cpp   # Tests ohne main()
```

**test_main.cpp:**
```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
```

**test_math.cpp:**
```cpp
#include <doctest/doctest.h>
#include "math.h"

TEST_CASE("Math functions") {
    // Tests...
}
```

### 6.2 Aussagekräftige Errormeldungen

```cpp
TEST_CASE("User validation") {
    User user{"John", -5};  // Ungültiges Alter
    
    // Schlecht: Keine Info bei Error
    CHECK(user.isValid());
    
    // Besser: Mit INFO
    INFO("User: ", user.name, ", Age: ", user.age);
    CHECK(user.isValid());
    
    // Noch besser: Mit CAPTURE
    CAPTURE(user.name);
    CAPTURE(user.age);
    CHECK(user.isValid());
}
```

### 6.3 AAA-Pattern

```cpp
TEST_CASE("Order total calculation") {
    // Arrange
    Order order;
    order.addItem(Item{"Book", 10.0});
    order.addItem(Item{"Pen", 2.50});
    
    // Act
    double total = order.calculateTotal();
    
    // Assert
    CHECK(total == doctest::Approx(12.50));
}
```

---

## 7. Troubleshooting

### 7.1 "doctest/doctest.h not found"

**Problem:** Header nicht gefunden

**Lösung:** Pfad in Solution.json prüfen:
```json
"doctest": {
    "path": "externals/doctest"  // Muss include/doctest/doctest.h enthalten
}
```

### 7.2 "multiple definition of main"

**Problem:** main() mehrfach definiert

**Lösung:** `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` nur in EINER Datei:
```cpp
// test_main.cpp - NUR HIER
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// andere_tests.cpp - OHNE main
#include <doctest/doctest.h>
```

### 7.3 Floating-Point Vergleiche

**Problem:** Float-Vergleiche schlagen fehl

**Lösung:** `doctest::Approx` verwenden:
```cpp
CHECK(result == doctest::Approx(3.14159).epsilon(0.001));
```

### 7.4 Tests werden nicht ausgeführt

**Problem:** Tests erscheinen nicht

**Lösung:** Prüfen ob `DOCTEST_CONFIG_DISABLE` gesetzt ist.

---

## 8. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **GitHub** | [github.com/doctest/doctest](https://github.com/doctest/doctest) |
| **Dokumentation** | [github.com/doctest/doctest/blob/master/doc/markdown/readme.md](https://github.com/doctest/doctest/blob/master/doc/markdown/readme.md) |
| **Tutorial** | [github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md](https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md) |
| **Assertion Reference** | [github.com/doctest/doctest/blob/master/doc/markdown/assertions.md](https://github.com/doctest/doctest/blob/master/doc/markdown/assertions.md) |

### See Also

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Local_Externals_Testing.md](../../references/externals/Local_Externals_Testing.md) — Reference
- [googletest.md](googletest.md) — Alternative mit Mocking
- [catch2.md](catch2.md) — Alternative mit BDD

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für doctest** |
