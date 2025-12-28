# PreFetch/googletest.cmake — GoogleTest PreFetch Hook

> **Version:** 1.0.0  
> **Datum:** 2025-12-16  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [googletest_PreFetch.md](../../en/modules/externals/hooks/prefetch/googletest_PreFetch.md)  
> **Hook:** [cmake/externals/Hooks/PreFetch/googletest.cmake](../../../../../cmake/externals/Hooks/PreFetch/googletest.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PreFetch Hook?](#2-warum-ein-prefetch-hook)
3. [Target Mapping](#3-target-mapping)
4. [Gesetzte Optionen](#4-gesetzte-optionen)
5. [Windows CRT Fix](#5-windows-crt-fix)
6. [Solution.json Konfiguration](#6-solutionjson-konfiguration)
7. [Erstellte Targets](#7-erstellte-targets)
8. [Verwendung in Tests](#8-verwendung-in-tests)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Der `googletest.cmake` PreFetch Hook konfiguriert Google Test und Google Mock vor dem Build. Er definiert Target-Mappings und behebt den häufigen Windows CRT-Linking-Fehler.

---

## 2. Warum ein PreFetch Hook?

**Problem 1 — Target-Namen:**  
GoogleTest erstellt Targets mit anderen Namen als der External-Name (`gtest`, `gmock` statt `googletest`).

**Problem 2 — Windows CRT:**  
Ohne `gtest_force_shared_crt=ON` gibt es auf Windows Linker-Fehler:
```
LNK2038: mismatch detected for 'RuntimeLibrary'
```

**Problem 3 — Unnötige Installation:**  
Ohne Hook wird ein Install-Target erstellt, das bei FetchContent nicht benötigt wird.

---

## 3. Target Mapping

Der Hook registriert die GoogleTest-Targets über Global Properties:

```cmake
set_property(GLOBAL PROPERTY HOOK_KNOWN_TARGETS_${HOOK_EXTERNAL_NAME}
    "gtest;gtest_main;gmock;gmock_main"
)
set_property(GLOBAL PROPERTY HOOK_PRIMARY_TARGET_${HOOK_EXTERNAL_NAME}
    "gmock_main"
)
```

| Property | Wert | Beschreibung |
|----------|------|--------------|
| `HOOK_KNOWN_TARGETS` | `gtest;gtest_main;gmock;gmock_main` | Alle verfügbaren Targets |
| `HOOK_PRIMARY_TARGET` | `gmock_main` | Standard-Target (enthält gtest + gmock) |

---

## 4. Gesetzte Optionen

| Option | Wert | Beschreibung |
|--------|------|--------------|
| `BUILD_GMOCK` | `ON` | Google Mock mit bauen |
| `INSTALL_GTEST` | `OFF` | Kein Install-Target |
| `gtest_hide_internal_symbols` | `ON` | Interne Symbole verstecken |

---

## 5. Windows CRT Fix

Auf Windows wird automatisch gesetzt:

```cmake
set(gtest_force_shared_crt ON CACHE BOOL 
    "Use shared (DLL) run-time lib even when Google Test is built as static lib." 
    FORCE
)
```

### Hintergrund

GoogleTest kompiliert standardmäßig mit **statischer CRT** (`/MT`), aber die meisten Projekte verwenden **dynamische CRT** (`/MD`). Dies führt zu Linker-Fehlern.

Die Option `gtest_force_shared_crt=ON` sorgt dafür, dass GoogleTest die gleiche CRT wie das Hauptprojekt verwendet.

---

## 6. Solution.json Konfiguration

### External Definition

```json
{
    "googletest": {
        "git": "https://github.com/google/googletest.git",
        "tag": "v1.14.0"
    }
}
```

### In Tests verwenden

```json
{
    "tests": [{
        "name": "UnitTests",
        "framework": "googletest",
        "externals": ["googletest"]
    }]
}
```

**Hinweis:** Das Test-System fügt `googletest` automatisch zu den Externals hinzu, wenn `"framework": "googletest"` gesetzt ist.

---

## 7. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `gtest` | STATIC | Google Test ohne main() |
| `gtest_main` | STATIC | Google Test mit main() |
| `gmock` | STATIC | Google Mock ohne main() |
| `gmock_main` | STATIC | Google Mock mit main() (empfohlen) |

**Empfehlung:** Für die meisten Tests `gmock_main` verwenden (PRIMARY Target) — enthält sowohl Google Test als auch Google Mock.

---

## 8. Verwendung in Tests

### Einfacher Test mit gmock_main

```cpp
#include <gtest/gtest.h>

TEST(ExampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
}
```

Kein eigenes `main()` nötig — wird von `gmock_main` bereitgestellt.

### Test mit Mocking

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockClass {
public:
    MOCK_METHOD(int, getValue, (), (const));
};

TEST(MockTest, Example) {
    MockClass mock;
    EXPECT_CALL(mock, getValue()).WillOnce(testing::Return(42));
    EXPECT_EQ(mock.getValue(), 42);
}
```

### Test mit eigenem Main

```cpp
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    // Custom setup
    return RUN_ALL_TESTS();
}
```

Hierfür `gtest` statt `gtest_main` linken.

---

## 9. Siehe auch

- [HookLoader_cmake.md](../HookLoader_cmake.md) — Hook-System
- [Targets_cmake.md](../../registry/Targets_cmake.md) — Target-Registrierung
- [catch2_PreFetch.md](catch2_PreFetch.md) — Catch2 Alternative
- [Tests_cmake.md](../../../project/Tests_cmake.md) — Test-Pipeline

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-16** | **Initiale Dokumentation im Blueprint v0.5.0 Format** |
