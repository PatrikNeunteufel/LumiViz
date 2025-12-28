# Google Test — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [googletest.md](../../../en/userguides/externals/Googletest.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Solution.json Configuration](#2-solutionjson-konfiguration)
3. [C++ Usage](#3-c-verwendung)
4. [Assertions](#4-assertions)
5. [GMock (Mocking)](#5-gmock-mocking)
6. [Fortgeschrittene Techniken](#6-fortgeschrittene-techniken)
7. [Troubleshooting](#7-troubleshooting)
8. [Weiterführende Informationen](#8-weiterführende-informationen)
9. [Changelog](#9-changelog)

---

## 1. Overview

**Google Test** ist das C++ Testing Framework von Google, inklusive GMock für Mocking.

| Aspekt | Wert |
|--------|------|
| **Typ** | Git External |
| **Repository** | https://github.com/google/googletest |
| **Empfohlener Tag** | v1.14.0 |
| **Lizenz** | BSD-3-Clause |
| **Website** | [google.github.io/googletest](https://google.github.io/googletest/) |

### Warum Google Test?

| Vorteil | Description |
|---------|--------------|
| 🎭 **GMock** | Integriertes Mocking Framework |
| 🏢 **Industrie-Standard** | Weit verbreitet |
| 📊 **XML Reports** | CI/CD Integration |
| 🔧 **Feature-reich** | Fixtures, Parametrisierung |

### Vergleich mit doctest

| Feature | googletest | doctest |
|---------|------------|---------|
| **Mocking** | ✅ GMock | ❌ |
| **Kompilierzeit** | 🐢 Langsam | ⚡ Schnell |
| **Header-Only** | ❌ | ✅ |
| **Footprint** | Groß | Klein |

---

## 2. Solution.json Configuration

### 2.1 Minimal

```json
{
    "externals": {
        "googletest": {
            "git": "https://github.com/google/googletest.git",
            "tag": "v1.14.0"
        }
    },
    "tests": [
        {
            "name": "UnitTests",
            "framework": "googletest",
            "externals": ["googletest"]
        }
    ]
}
```

### 2.2 Mit Library-Tests

```json
{
    "externals": {
        "googletest": {
            "git": "https://github.com/google/googletest.git",
            "tag": "v1.14.0"
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
            "framework": "googletest",
            "externals": ["googletest"],
            "libraries": ["CoreLib"]
        }
    ]
}
```

### 2.3 PreFetch Hook

```cmake
# cmake/externals/hooks/prefetch/googletest.cmake
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Windows: Prevent CRT mismatch
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
```

### 2.4 Targets

| Target | Description |
|--------|--------------|
| `gtest` | Google Test Core |
| `gtest_main` | Mit main() |
| `gmock` | Google Mock Core |
| `gmock_main` | Mock mit main() |

---

## 3. C++ Usage

### 3.1 Einfacher Test

```cpp
#include <gtest/gtest.h>

TEST(MathTest, Addition) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_EQ(2 + 3, 5);
}

TEST(MathTest, Subtraction) {
    EXPECT_EQ(5 - 3, 2);
    EXPECT_EQ(10 - 7, 3);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

### 3.2 Mit gtest_main

```cpp
// Kein main() nötig - von gtest_main bereitgestellt
#include <gtest/gtest.h>

TEST(MyTest, Works) {
    EXPECT_TRUE(true);
}
```

### 3.3 Test Fixtures

```cpp
#include <gtest/gtest.h>
#include <vector>

class VectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Wird vor jedem TEST_F aufgerufen
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
    }
    
    void TearDown() override {
        // Wird nach jedem TEST_F aufgerufen
        vec.clear();
    }
    
    std::vector<int> vec;
};

TEST_F(VectorTest, InitialSize) {
    EXPECT_EQ(vec.size(), 3);
}

TEST_F(VectorTest, PushBack) {
    vec.push_back(4);
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec.back(), 4);
}

TEST_F(VectorTest, PopBack) {
    vec.pop_back();
    EXPECT_EQ(vec.size(), 2);
}
```

### 3.4 Test Suites

```cpp
// Alle Tests mit gleichem Prefix gehören zur gleichen Suite
TEST(StringTest, Empty) {
    std::string s;
    EXPECT_TRUE(s.empty());
}

TEST(StringTest, Length) {
    std::string s = "hello";
    EXPECT_EQ(s.length(), 5);
}
```

---

## 4. Assertions

### 4.1 EXPECT vs ASSERT

| Typ | Bei Error | Usage |
|-----|------------|------------|
| `EXPECT_*` | Test läuft weiter | Standard |
| `ASSERT_*` | Test bricht ab | Wenn Fortsetzung sinnlos |

### 4.2 Boolean Assertions

```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Mit Nachricht
EXPECT_TRUE(result) << "Expected result to be true";
```

### 4.3 Vergleiche

```cpp
EXPECT_EQ(a, b);   // a == b
EXPECT_NE(a, b);   // a != b
EXPECT_LT(a, b);   // a < b
EXPECT_LE(a, b);   // a <= b
EXPECT_GT(a, b);   // a > b
EXPECT_GE(a, b);   // a >= b
```

### 4.4 String Assertions

```cpp
EXPECT_STREQ(str1, str2);     // C-Strings gleich
EXPECT_STRNE(str1, str2);     // C-Strings ungleich
EXPECT_STRCASEEQ(str1, str2); // Case-insensitive gleich
EXPECT_STRCASENE(str1, str2); // Case-insensitive ungleich
```

### 4.5 Floating-Point

```cpp
EXPECT_FLOAT_EQ(a, b);     // float mit ULP-Toleranz
EXPECT_DOUBLE_EQ(a, b);    // double mit ULP-Toleranz
EXPECT_NEAR(a, b, 0.001);  // Absolute Toleranz
```

### 4.6 Exception Assertions

```cpp
EXPECT_THROW(statement, exception_type);
EXPECT_ANY_THROW(statement);
EXPECT_NO_THROW(statement);

// Example
EXPECT_THROW(vec.at(100), std::out_of_range);
EXPECT_NO_THROW(vec.at(0));
```

### 4.7 Death Tests

```cpp
// Testet ob Programm terminiert
EXPECT_DEATH(abort(), "");
EXPECT_EXIT(exit(1), testing::ExitedWithCode(1), "");
```

---

## 5. GMock (Mocking)

### 5.1 Mock-Klasse erstellen

```cpp
#include <gmock/gmock.h>

// Interface
class Database {
public:
    virtual ~Database() = default;
    virtual bool connect(const std::string& host) = 0;
    virtual std::string query(const std::string& sql) = 0;
    virtual void disconnect() = 0;
};

// Mock
class MockDatabase : public Database {
public:
    MOCK_METHOD(bool, connect, (const std::string& host), (override));
    MOCK_METHOD(std::string, query, (const std::string& sql), (override));
    MOCK_METHOD(void, disconnect, (), (override));
};
```

### 5.2 Expectations

```cpp
TEST(DatabaseTest, ConnectCalled) {
    MockDatabase db;
    
    // Erwartung: connect wird mit "localhost" aufgerufen
    EXPECT_CALL(db, connect("localhost"))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    // Code der getestet wird
    bool result = db.connect("localhost");
    
    EXPECT_TRUE(result);
}
```

### 5.3 Actions

```cpp
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Throw;
using ::testing::DoAll;
using ::testing::SetArgReferee;

EXPECT_CALL(mock, method())
    .WillOnce(Return(42))              // Wert zurückgeben
    .WillOnce(Throw(std::runtime_error("error")))  // Exception
    .WillRepeatedly(Return(0));        // Für alle weiteren Aufrufe
```

### 5.4 Matchers

```cpp
using ::testing::_;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Lt;
using ::testing::HasSubstr;
using ::testing::StartsWith;

EXPECT_CALL(mock, method(_))           // Beliebiger Parameters
    .With(Eq(5))                       // Gleich 5
    .With(HasSubstr("test"))           // Enthält "test"
    .With(StartsWith("hello"));        // Beginnt mit "hello"
```

### 5.5 Vollständiges Example

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class UserService {
public:
    UserService(Database* db) : db_(db) {}
    
    bool login(const std::string& user, const std::string& pass) {
        if (!db_->connect("auth-server")) {
            return false;
        }
        
        std::string result = db_->query(
            "SELECT * FROM users WHERE name='" + user + "'"
        );
        
        db_->disconnect();
        return result.find(pass) != std::string::npos;
    }
    
private:
    Database* db_;
};

TEST(UserServiceTest, LoginSuccess) {
    MockDatabase mockDb;
    UserService service(&mockDb);
    
    EXPECT_CALL(mockDb, connect("auth-server"))
        .WillOnce(testing::Return(true));
    
    EXPECT_CALL(mockDb, query(testing::HasSubstr("john")))
        .WillOnce(testing::Return("john:secret123"));
    
    EXPECT_CALL(mockDb, disconnect())
        .Times(1);
    
    EXPECT_TRUE(service.login("john", "secret123"));
}

TEST(UserServiceTest, LoginConnectionFailed) {
    MockDatabase mockDb;
    UserService service(&mockDb);
    
    EXPECT_CALL(mockDb, connect(testing::_))
        .WillOnce(testing::Return(false));
    
    EXPECT_FALSE(service.login("john", "secret"));
}
```

---

## 6. Fortgeschrittene Techniken

### 6.1 Parametrisierte Tests

```cpp
class FibonacciTest : public ::testing::TestWithParam<std::pair<int, int>> {};

TEST_P(FibonacciTest, Compute) {
    auto [input, expected] = GetParam();
    EXPECT_EQ(fibonacci(input), expected);
}

INSTANTIATE_TEST_SUITE_P(
    FibonacciValues,
    FibonacciTest,
    ::testing::Values(
        std::make_pair(0, 0),
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 2),
        std::make_pair(10, 55)
    )
);
```

### 6.2 Type-Parametrisierte Tests

```cpp
template <typename T>
class TypedTest : public ::testing::Test {};

using MyTypes = ::testing::Types<int, float, double>;
TYPED_TEST_SUITE(TypedTest, MyTypes);

TYPED_TEST(TypedTest, DefaultIsZero) {
    TypeParam value{};
    EXPECT_EQ(value, TypeParam{});
}
```

### 6.3 Test Filters

```bash
# Nur bestimmte Tests ausführen
./tests --gtest_filter=MathTest.*
./tests --gtest_filter=*Addition*
./tests --gtest_filter=-*Slow*  # Ausschließen

# Wiederholen
./tests --gtest_repeat=10

# Shuffle
./tests --gtest_shuffle
```

### 6.4 XML Output

```bash
./tests --gtest_output=xml:results.xml
```

### 6.5 Custom Main

```cpp
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    
    // Eigene Initialisierung
    MyApp::init();
    
    int result = RUN_ALL_TESTS();
    
    // Eigenes Cleanup
    MyApp::shutdown();
    
    return result;
}
```

---

## 7. Troubleshooting

### 7.1 Link-Error unter Windows

**Problem:** `LNK2038: mismatch detected for 'RuntimeLibrary'`

**Lösung:** PreFetch Hook setzen:
```cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
```

### 7.2 "undefined reference to testing::..."

**Problem:** Google Test nicht gelinkt

**Lösung:** Target gegen `gtest` oder `gtest_main` linken.

### 7.3 Tests werden nicht gefunden

**Problem:** Keine Tests ausgeführt

**Lösung:** 
- `RUN_ALL_TESTS()` aufrufen
- Gegen `gtest_main` linken (statt eigenem main)

### 7.4 GMock Warnings

**Problem:** "Uninteresting mock function call"

**Lösung:** `NiceMock` verwenden:
```cpp
testing::NiceMock<MockDatabase> db;
```

---

## 8. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **GitHub** | [github.com/google/googletest](https://github.com/google/googletest) |
| **Dokumentation** | [google.github.io/googletest](https://google.github.io/googletest/) |
| **Primer** | [google.github.io/googletest/primer.html](https://google.github.io/googletest/primer.html) |
| **GMock** | [google.github.io/googletest/gmock_cook_book.html](https://google.github.io/googletest/gmock_cook_book.html) |

### See Also

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Git_Externals_Testing.md](../../references/externals/Git_Externals_Testing.md) — Reference
- [doctest.md](doctest.md) — Leichtgewichtige Alternative
- [catch2.md](catch2.md) — BDD-Style Alternative

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für Google Test** |
