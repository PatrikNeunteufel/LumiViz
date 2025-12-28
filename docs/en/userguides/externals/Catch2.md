# Catch2 — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [catch2.md](../../../en/userguides/externals/Catch2.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Solution.json Configuration](#2-solutionjson-konfiguration)
3. [C++ Usage](#3-c-verwendung)
4. [BDD-Style Testing](#4-bdd-style-testing)
5. [Matchers](#5-matchers)
6. [Fortgeschrittene Techniken](#6-fortgeschrittene-techniken)
7. [Troubleshooting](#7-troubleshooting)
8. [Weiterführende Informationen](#8-weiterführende-informationen)
9. [Changelog](#9-changelog)

---

## 1. Overview

**Catch2** ist ein modernes C++ Testing Framework mit BDD-Style Syntax und Sections.

| Aspekt | Wert |
|--------|------|
| **Typ** | Git External |
| **Repository** | https://github.com/catchorg/Catch2 |
| **Empfohlener Tag** | v3.5.2 |
| **Lizenz** | BSL-1.0 |
| **Website** | [github.com/catchorg/Catch2](https://github.com/catchorg/Catch2) |

### Warum Catch2?

| Vorteil | Description |
|---------|--------------|
| 📝 **BDD-Style** | GIVEN/WHEN/THEN Syntax |
| 🔀 **Sections** | Verschachtelte Tests |
| 🏷️ **Tags** | Flexible Test-Filterung |
| 📊 **Benchmarks** | Integriertes Benchmarking |

### Vergleich

| Feature | Catch2 | googletest | doctest |
|---------|--------|------------|---------|
| **BDD Syntax** | ✅ | ❌ | ✅ |
| **Sections** | ✅ | ❌ | ✅ (Subcases) |
| **Mocking** | ❌ | ✅ GMock | ❌ |
| **Benchmarks** | ✅ | ❌ | ❌ |

---

## 2. Solution.json Configuration

### 2.1 Minimal

```json
{
    "externals": {
        "catch2": {
            "git": "https://github.com/catchorg/Catch2.git",
            "tag": "v3.5.2"
        }
    },
    "tests": [
        {
            "name": "UnitTests",
            "framework": "catch2",
            "externals": ["catch2"]
        }
    ]
}
```

### 2.2 Mit Library-Tests

```json
{
    "externals": {
        "catch2": {
            "git": "https://github.com/catchorg/Catch2.git",
            "tag": "v3.5.2"
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
            "framework": "catch2",
            "externals": ["catch2"],
            "libraries": ["CoreLib"]
        }
    ]
}
```

### 2.3 PreFetch Hook

```cmake
# cmake/externals/hooks/prefetch/catch2.cmake
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
```

### 2.4 Targets

| Target | Description |
|--------|--------------|
| `Catch2::Catch2` | Header-Only Interface |
| `Catch2::Catch2WithMain` | Mit main() |

---

## 3. C++ Usage

### 3.1 Einfacher Test

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Math operations", "[math]") {
    REQUIRE(1 + 1 == 2);
    REQUIRE(2 * 3 == 6);
    REQUIRE(10 / 2 == 5);
}
```

### 3.2 Mit eigenem main()

```cpp
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

// Tests hier...
```

### 3.3 Sections

Sections ermöglichen verschachtelte Tests mit gemeinsamem Setup:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("Vector operations", "[vector]") {
    std::vector<int> v;  // Gemeinsames Setup
    
    REQUIRE(v.empty());
    
    SECTION("pushing elements") {
        v.push_back(1);
        REQUIRE(v.size() == 1);
        
        SECTION("pushing more") {
            v.push_back(2);
            REQUIRE(v.size() == 2);
        }
        
        SECTION("popping") {
            v.pop_back();
            REQUIRE(v.empty());
        }
    }
    
    SECTION("reserving") {
        v.reserve(10);
        REQUIRE(v.capacity() >= 10);
    }
}
```

**Ausführungsreihenfolge:**
```
Vector operations → pushing elements → pushing more
Vector operations → pushing elements → popping
Vector operations → reserving
```

### 3.4 Tags

```cpp
TEST_CASE("Fast test", "[fast]") { /* ... */ }
TEST_CASE("Slow test", "[slow][integration]") { /* ... */ }
TEST_CASE("Math test", "[math][fast]") { /* ... */ }

// Ausführen:
// ./tests [fast]           - Nur [fast] Tests
// ./tests ~[slow]          - Ohne [slow] Tests
// ./tests [math][fast]     - Mit [math] UND [fast]
```

### 3.5 Assertions

| Assertion | Description | Bei Error |
|-----------|--------------|------------|
| `REQUIRE(expr)` | Prüft Expression | Abbruch |
| `CHECK(expr)` | Prüft Expression | Weiter |
| `REQUIRE_FALSE(expr)` | Prüft auf false | Abbruch |
| `CHECK_FALSE(expr)` | Prüft auf false | Weiter |

```cpp
TEST_CASE("Assertions") {
    int x = 5;
    
    REQUIRE(x == 5);
    CHECK(x > 0);
    REQUIRE_FALSE(x < 0);
    
    // Mit Nachricht
    INFO("Testing value: " << x);
    REQUIRE(x == 5);
}
```

---

## 4. BDD-Style Testing

### 4.1 SCENARIO / GIVEN / WHEN / THEN

```cpp
#include <catch2/catch_test_macros.hpp>

SCENARIO("Bank account operations", "[bank]") {
    GIVEN("A bank account with balance 100") {
        BankAccount account(100);
        
        WHEN("50 is deposited") {
            account.deposit(50);
            
            THEN("balance is 150") {
                REQUIRE(account.balance() == 150);
            }
        }
        
        WHEN("30 is withdrawn") {
            account.withdraw(30);
            
            THEN("balance is 70") {
                REQUIRE(account.balance() == 70);
            }
        }
        
        WHEN("200 is withdrawn") {
            THEN("exception is thrown") {
                REQUIRE_THROWS(account.withdraw(200));
            }
        }
    }
}
```

### 4.2 AND_GIVEN / AND_WHEN / AND_THEN

```cpp
SCENARIO("Shopping cart", "[cart]") {
    GIVEN("An empty cart") {
        Cart cart;
        
        AND_GIVEN("A product with price 10") {
            Product p{"Item", 10};
            
            WHEN("product is added") {
                cart.add(p);
                
                THEN("cart has 1 item") {
                    REQUIRE(cart.size() == 1);
                }
                
                AND_THEN("total is 10") {
                    REQUIRE(cart.total() == 10);
                }
            }
        }
    }
}
```

---

## 5. Matchers

### 5.1 String Matchers

```cpp
#include <catch2/matchers/catch_matchers_string.hpp>

using Catch::Matchers::StartsWith;
using Catch::Matchers::EndsWith;
using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::Equals;

TEST_CASE("String matchers") {
    std::string s = "Hello, World!";
    
    REQUIRE_THAT(s, StartsWith("Hello"));
    REQUIRE_THAT(s, EndsWith("!"));
    REQUIRE_THAT(s, ContainsSubstring("World"));
    REQUIRE_THAT(s, Equals("Hello, World!"));
}
```

### 5.2 Floating-Point Matchers

```cpp
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Float matchers") {
    double result = calculate();
    
    REQUIRE_THAT(result, WithinAbs(3.14159, 0.0001));
    REQUIRE_THAT(result, WithinRel(3.14159, 0.01));  // 1% Toleranz
}
```

### 5.3 Container Matchers

```cpp
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>

using Catch::Matchers::IsEmpty;
using Catch::Matchers::SizeIs;
using Catch::Matchers::Contains;

TEST_CASE("Container matchers") {
    std::vector<int> v = {1, 2, 3};
    
    REQUIRE_THAT(v, !IsEmpty());
    REQUIRE_THAT(v, SizeIs(3));
    REQUIRE_THAT(v, Contains(2));
}
```

### 5.4 Exception Matchers

```cpp
#include <catch2/matchers/catch_matchers_exception.hpp>

using Catch::Matchers::Message;

TEST_CASE("Exception matchers") {
    REQUIRE_THROWS_MATCHES(
        throwFunction(),
        std::runtime_error,
        Message("Expected error message")
    );
}
```

### 5.5 Custom Matchers

```cpp
#include <catch2/matchers/catch_matchers_templated.hpp>

struct IsEvenMatcher : Catch::Matchers::MatcherGenericBase {
    bool match(int n) const {
        return n % 2 == 0;
    }
    
    std::string describe() const override {
        return "is even";
    }
};

auto IsEven() { return IsEvenMatcher{}; }

TEST_CASE("Custom matcher") {
    REQUIRE_THAT(4, IsEven());
    REQUIRE_THAT(5, !IsEven());
}
```

---

## 6. Fortgeschrittene Techniken

### 6.1 Generators

```cpp
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("Generators") {
    auto i = GENERATE(1, 2, 3, 4, 5);
    
    CAPTURE(i);
    REQUIRE(i > 0);
    REQUIRE(i <= 5);
}

TEST_CASE("Range generator") {
    auto i = GENERATE(range(1, 10));
    
    REQUIRE(i >= 1);
    REQUIRE(i < 10);
}

TEST_CASE("Table generator") {
    auto [input, expected] = GENERATE(table<int, int>({
        {1, 1},
        {2, 4},
        {3, 9},
        {4, 16}
    }));
    
    REQUIRE(input * input == expected);
}
```

### 6.2 Benchmarks

```cpp
#include <catch2/benchmark/catch_benchmark.hpp>

TEST_CASE("Benchmarks") {
    BENCHMARK("Vector push_back") {
        std::vector<int> v;
        for (int i = 0; i < 1000; ++i) {
            v.push_back(i);
        }
        return v;
    };
    
    BENCHMARK_ADVANCED("Vector with reserve")(Catch::Benchmark::Chronometer meter) {
        std::vector<int> v;
        v.reserve(1000);
        
        meter.measure([&v] {
            for (int i = 0; i < 1000; ++i) {
                v.push_back(i);
            }
            return v.size();
        });
    };
}
```

### 6.3 Test Fixtures

```cpp
class DatabaseFixture {
protected:
    DatabaseFixture() {
        db.connect("test.db");
    }
    
    ~DatabaseFixture() {
        db.disconnect();
    }
    
    Database db;
};

TEST_CASE_METHOD(DatabaseFixture, "Database insert", "[db]") {
    db.insert("key", "value");
    REQUIRE(db.get("key") == "value");
}

TEST_CASE_METHOD(DatabaseFixture, "Database delete", "[db]") {
    db.insert("key", "value");
    db.remove("key");
    REQUIRE_FALSE(db.exists("key"));
}
```

### 6.4 Event Listeners

```cpp
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

class MyListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;
    
    void testCaseStarting(Catch::TestCaseInfo const& info) override {
        std::cout << "Starting: " << info.name << std::endl;
    }
    
    void testCaseEnded(Catch::TestCaseStats const& stats) override {
        std::cout << "Ended: " << stats.testInfo->name << std::endl;
    }
};

CATCH_REGISTER_LISTENER(MyListener)
```

### 6.5 Command Line

```bash
# Tests ausführen
./tests

# Mit Tags
./tests [fast]
./tests ~[slow]

# Test-Filter
./tests "Vector*"
./tests -c "Vector operations"

# Verbose
./tests -s

# XML Output
./tests -r xml -o results.xml

# Liste aller Tests
./tests --list-tests
./tests --list-tags
```

---

## 7. Troubleshooting

### 7.1 "catch2/catch_test_macros.hpp not found"

**Problem:** Header nicht gefunden

**Lösung:** Catch2 v3 verwendet neue Header-Struktur:
```cpp
// v2 (alt)
#include <catch2/catch.hpp>

// v3 (neu)
#include <catch2/catch_test_macros.hpp>
```

### 7.2 Link-Error

**Problem:** Undefined reference zu Catch2-Symbolen

**Lösung:** Gegen `Catch2::Catch2WithMain` linken.

### 7.3 Tests werden nicht gefunden

**Problem:** Keine Tests ausgeführt

**Lösung:** 
- Header `catch_test_macros.hpp` inkludieren
- Gegen `Catch2WithMain` linken (oder eigenes main)

### 7.4 Slow Compilation

**Problem:** Lange Kompilierzeit

**Lösung:** 
- Precompiled Headers verwenden
- Tests auf mehrere Dateien verteilen

---

## 8. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **GitHub** | [github.com/catchorg/Catch2](https://github.com/catchorg/Catch2) |
| **Dokumentation** | [github.com/catchorg/Catch2/blob/devel/docs/Readme.md](https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md) |
| **Tutorial** | [github.com/catchorg/Catch2/blob/devel/docs/tutorial.md](https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md) |
| **Reference** | [github.com/catchorg/Catch2/blob/devel/docs/assertions.md](https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md) |
| **Migration v2→v3** | [github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md](https://github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md) |

### See Also

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Git_Externals_Testing.md](../../references/externals/Git_Externals_Testing.md) — Reference
- [doctest.md](doctest.md) — Leichtgewichtige Alternative
- [googletest.md](googletest.md) — Alternative mit GMock

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für Catch2** |
