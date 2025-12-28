# PreFetch/catch2.cmake — Catch2 PreFetch Hook

> **Version:** 1.0.0  
> **Datum:** 2025-12-16  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [catch2_PreFetch.md](../../en/modules/externals/hooks/prefetch/catch2_PreFetch.md)  
> **Hook:** [cmake/externals/Hooks/PreFetch/catch2.cmake](../../../../../cmake/externals/Hooks/PreFetch/catch2.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PreFetch Hook?](#2-warum-ein-prefetch-hook)
3. [Target Mapping](#3-target-mapping)
4. [Gesetzte Optionen](#4-gesetzte-optionen)
5. [Solution.json Konfiguration](#5-solutionjson-konfiguration)
6. [Erstellte Targets](#6-erstellte-targets)
7. [Verwendung in Tests](#7-verwendung-in-tests)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Der `catch2.cmake` PreFetch Hook konfiguriert Catch2 v3 vor dem Build. Er definiert Target-Mappings für die Auto-Registrierung und deaktiviert unnötige Komponenten.

**Wichtig:** Catch2 v3 ist **nicht mehr header-only**! Es muss als Library gelinkt werden.

---

## 2. Warum ein PreFetch Hook?

**Problem 1 — Target-Namen:**  
Catch2 erstellt Targets mit anderen Namen als der External-Name (`Catch2`, `Catch2WithMain` statt `catch2`).

**Problem 2 — Unnötige Targets:**  
Ohne Hook werden Tests, Examples und Extras mit gebaut.

---

## 3. Target Mapping

Der Hook registriert die Catch2-Targets über Global Properties:

```cmake
set_property(GLOBAL PROPERTY HOOK_KNOWN_TARGETS_${HOOK_EXTERNAL_NAME}
    "Catch2;Catch2WithMain"
)
set_property(GLOBAL PROPERTY HOOK_PRIMARY_TARGET_${HOOK_EXTERNAL_NAME}
    "Catch2WithMain"
)
```

| Property | Wert | Beschreibung |
|----------|------|--------------|
| `HOOK_KNOWN_TARGETS` | `Catch2;Catch2WithMain` | Alle verfügbaren Targets |
| `HOOK_PRIMARY_TARGET` | `Catch2WithMain` | Standard-Target zum Linken |

---

## 4. Gesetzte Optionen

| Option | Wert | Beschreibung |
|--------|------|--------------|
| `CATCH_BUILD_TESTING` | `OFF` | Keine eigenen Tests bauen |
| `CATCH_BUILD_EXAMPLES` | `OFF` | Keine Examples bauen |
| `CATCH_INSTALL_DOCS` | `OFF` | Keine Docs installieren |
| `CATCH_INSTALL_EXTRAS` | `OFF` | Keine CMake-Extras |
| `BUILD_SHARED_LIBS` | `OFF` | Als Static Library bauen |

---

## 5. Solution.json Konfiguration

### External Definition

```json
{
    "catch2": {
        "git": "https://github.com/catchorg/Catch2.git",
        "tag": "v3.5.2"
    }
}
```

### In Tests verwenden

```json
{
    "tests": [{
        "name": "CoreTests",
        "framework": "catch2",
        "externals": ["catch2"]
    }]
}
```

**Hinweis:** Das Test-System fügt `catch2` automatisch zu den Externals hinzu, wenn `"framework": "catch2"` gesetzt ist.

---

## 6. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `Catch2` | STATIC | Catch2 ohne main() |
| `Catch2WithMain` | STATIC | Catch2 mit main() (empfohlen) |

**Empfehlung:** Für die meisten Tests `Catch2WithMain` verwenden (PRIMARY Target).

---

## 7. Verwendung in Tests

### Einfacher Test mit Catch2WithMain

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Example test", "[example]") {
    REQUIRE(1 + 1 == 2);
}
```

Kein eigenes `main()` nötig — wird von `Catch2WithMain` bereitgestellt.

### Test mit eigenem Main

```cpp
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>

int main(int argc, char* argv[]) {
    // Custom setup
    return Catch::Session().run(argc, argv);
}
```

Hierfür `Catch2` statt `Catch2WithMain` linken.

---

## 8. Siehe auch

- [HookLoader_cmake.md](../HookLoader_cmake.md) — Hook-System
- [Targets_cmake.md](../../registry/Targets_cmake.md) — Target-Registrierung
- [googletest_PreFetch.md](googletest_PreFetch.md) — GoogleTest Alternative
- [Tests_cmake.md](../../../project/Tests_cmake.md) — Test-Pipeline

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-16** | **Initiale Dokumentation im Blueprint v0.5.0 Format** |
