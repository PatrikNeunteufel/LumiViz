# CMake Standard — Übergeordnete Build-System-Richtlinien

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Scope:** Alle CMake-basierten Projekte  
> **Durchsetzung:** Code Review, CI  
> **Language:** English  
> **German:** [CMake_Standard.md](../../en/standards/CMake_Standard.md)

---

## Table of Contents

1. [Zweck und Scope](#1-zweck-und-geltungsbereich)
2. [CMake-Version und Policies](#2-cmake-version-und-policies)
3. [Projektstruktur](#3-projektstruktur)
4. [Namenskonventionen](#4-namenskonventionen)
5. [Externe Dependencies](#5-externe-abhängigkeiten)
6. [Error- und Logging-Verhalten](#6-fehler--und-logging-verhalten)
7. [Presets](#7-presets)
8. [Tests und Tools](#8-tests-und-tools)
9. [Compiler-Optionen](#9-compiler-optionen)
10. [Legacy-Projekte](#10-legacy-projekte)
11. [Verhältnis zu anderen Standards](#11-verhältnis-zu-anderen-standards)
12. [Checkliste für neue Projekte](#12-checkliste-für-neue-projekte)
13. [See Also](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Zweck und Scope

Dieser Standard definiert **Mindestanforderungen und Konventionen** für alle CMake-basierten Projekte.

### Zielgruppe

Alle Entwickler, die CMake für Build-Configuration verwenden. Dieser Standard ist verbindlich für neue Projekte und empfohlen bei Modernisierung bestehender Projekte.

### Anwendungsbereich

| Projekt-Typ | Gilt für |
|-------------|----------|
| Modulares Build-System | Solution.json, CMake-Module |
| Einfache Projekte | Standalone CMakeLists.txt |
| Prototypen | Minimal-Setup |

### Verhältnis zu CMake Architecture

Dieser Standard ist **übergeordnet** zum modularen Build-System:
- CMake Architecture (Solution.json) folgt diesem Standard
- Eigenständige Projekte folgen ebenfalls diesem Standard
- Der Standard definiert das "Was", Module das "Wie"

---

## 2. CMake-Version und Policies

### 2.1 Minimum-Version

```cmake
cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(ProjectName LANGUAGES C CXX)
```

| Anforderung | Wert |
|-------------|------|
| Minimum-Version | **3.24** (oder aktueller) |
| Abweichungen | Im README begründen |

### 2.2 Policies

| Regel | Description |
|-------|--------------|
| Implizit setzen | `cmake_policy(VERSION <min-version>)` |
| Explizites Override | Nur mit Begründungskommentar |

---

## 3. Projektstruktur

### 3.1 Mindeststruktur

```
<ProjectRoot>/
├── CMakeLists.txt          # Root: project(), options, add_subdirectory()
├── src/                    # Source-Dateien
├── include/                # (Optional) Public Headers
├── tests/                  # (Optional) Tests
├── cmake/                  # (Optional) CMake-Module
└── docs/                   # (Optional) Dokumentation
```

### 3.2 Root-CMakeLists.txt

Enthält **nur**:
- `cmake_minimum_required()`
- `project()`
- Globale Optionen (`option()`)
- `add_subdirectory()` Aufrufe

**Keine** Detail-Configuration in Root.

### 3.3 Modern CMake (Target-basiert)

```cmake
# ✅ Modern CMake
add_library(MyLib)

target_sources(MyLib PRIVATE
    src/MyLib.cpp
)

target_include_directories(MyLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(MyLib PUBLIC
    cxx_std_20
)
```

### 3.4 Verboten (außer Legacy)

| Befehl | Problem | Alternative |
|--------|---------|-------------|
| `include_directories()` | Global | `target_include_directories()` |
| `add_definitions()` | Global | `target_compile_definitions()` |
| `link_libraries()` | Global | `target_link_libraries()` |

---

## 4. Namenskonventionen

### 4.1 Targets

| Target-Typ | Konvention | Example |
|------------|------------|----------|
| Library | Klar erkennbar | `CoreLib`, `AudioEngine` |
| Executable | Klar erkennbar | `MyApp`, `AudioPlayer` |
| Test | Suffix/Prefix | `CoreLibTests`, `test_Audio` |

### 4.2 Optionen und Cache-Variablen

| Typ | Konvention | Example |
|-----|------------|----------|
| Projekt-Option | `PROJEKT_FEATURE` | `MYAPP_ENABLE_TESTS` |
| Cache-Variable | `UPPER_CASE` | `BUILD_SHARED_LIBS` |
| Interne Variable | `_lower_case` | `_source_files` |

```cmake
option(MYAPP_ENABLE_TESTS "Enable test targets" ON)
set(MYAPP_EXTERNALS_DIR "${CMAKE_SOURCE_DIR}/externals" 
    CACHE PATH "Externals root directory")
```

### 4.3 Functions und Makros

| Typ | Konvention | Example |
|-----|------------|----------|
| Projekt-Funktion | `projekt_snake_case` | `myapp_add_executable()` |
| Generische Funktion | `modul_snake_case` | `ctx_create()`, `json_get()` |

---

## 5. Externe Dependencies

### 5.1 Empfohlener Weg

```cmake
# ✅ Modern CMake mit Imported Targets
find_package(ZLIB REQUIRED)
target_link_libraries(MyApp PRIVATE ZLIB::ZLIB)

find_package(SDL2 REQUIRED)
target_link_libraries(MyApp PRIVATE SDL2::SDL2)
```

### 5.2 Paketmanager

| Manager | Integration |
|---------|-------------|
| vcpkg | Via Toolchain-File |
| Conan | Via Toolchain-File |

**Nicht** hart im CMake-Code verankern.

### 5.3 Verbotene harte Pfade

```cmake
# ❌ Verboten
set(VCPKG_ROOT "H:/Dev/vcpkg")

# ✅ Erlaubt (in CMakeUserPresets.json)
# "CMAKE_TOOLCHAIN_FILE": "H:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

---

## 6. Error- und Logging-Verhalten

### 6.1 Errorbehandlung

| Funktion | Usage |
|----------|------------|
| `message(FATAL_ERROR ...)` | Build abbrechen |
| `message(WARNING ...)` | Verdächtige Configuration |
| `message(STATUS ...)` | Informativ |

Empfohlen: Projekt-Wrapper für konsistente Errorcodes:

```cmake
function(proj_fatal CODE MESSAGE)
    message(FATAL_ERROR "[${CODE}] ${MESSAGE}")
endfunction()
```

### 6.2 Errorformat

```
[E101] Dependency 'CoreLib' für 'MyApp' existiert nicht
 ^      ^                                ^
 Code   Description                     Kontext
```

### 6.3 TODO-Marker

Für unfertige Aufgaben oder zu behebende Probleme: `TODO` im Kommentar verwenden.

```cmake
# TODO: Add support for cross-compilation
# TODO: Optimize find_package calls
# TODO: Remove deprecated function after migration
```

---

## 7. Presets

### 7.1 CMakePresets.json

**Empfohlen** für neue Projekte:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "debug",
            "displayName": "Debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        }
    ]
}
```

### 7.2 Mindest-Presets

| Preset | Zweck |
|--------|-------|
| `debug` | Lokale Entwicklung |
| `release` | Release-Build |
| (Optional) CI-Presets | Automatisierung |

### 7.3 User-Presets

| Datei | Inhalt | Git |
|-------|--------|-----|
| `CMakePresets.json` | Team-Presets | ✅ Committen |
| `CMakeUserPresets.json` | Lokale Pfade | ❌ Gitignore |
| `CMakeUserPresets.example.json` | Template | ✅ Committen |

---

## 8. Tests und Tools

### 8.1 CTest-Integration

```cmake
include(CTest)
enable_testing()

add_executable(MyTests test_main.cpp)
add_test(NAME MyTests COMMAND MyTests)
```

### 8.2 Statische Analyse

| Sprache | Tool | Integration |
|---------|------|-------------|
| C++ | clang-format | IDE, Pre-Commit |
| C++ | clang-tidy | `CMAKE_CXX_CLANG_TIDY` |
| C | clang-tidy | PC-Build, Cross-Check |

```cmake
# In Preset oder Toolchain
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
```

---

## 9. Compiler-Optionen

### 9.1 Per-Target (nicht global)

```cmake
# ✅ Per-Target
target_compile_options(MyApp PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# ❌ Global
add_compile_options(/W4)
```

### 9.2 Generator Expressions

Für plattformspezifische Optionen:

```cmake
target_compile_definitions(MyApp PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
)
```

---

## 10. Legacy-Projekte

### 10.1 Migration

- Schrittweise modernisieren
- Abweichungen im README dokumentieren
- Ziel: Modern CMake

### 10.2 Dokumentation

Bei dauerhaften Abweichungen:
- Grund dokumentieren
- Scope minimieren

---

## 11. Verhältnis zu anderen Standards

| Standard | Regelt |
|----------|--------|
| **CMake_Standard** (dieses Dokument) | Build-System-Konventionen |
| Cpp_Coding_Standard | C++ Code-Stil |
| C_Coding_Standard | C Code-Stil (Embedded) |
| ClangFormat_Blueprint | Formatierung |
| ClangTidy_Blueprint | Statische Analyse |

---

## 12. Checkliste für neue Projekte

- [ ] `cmake_minimum_required(VERSION 3.24)`
- [ ] `project()` mit korrekten `LANGUAGES`
- [ ] Target-basierte Configuration
- [ ] Keine globalen `include_directories()`
- [ ] Keine harten Pfade
- [ ] CMakePresets.json vorhanden
- [ ] CMakeUserPresets.example.json vorhanden
- [ ] CTest aktiviert (falls Tests)

---

## 13. See Also

- [CMake.md](../blueprints/CMake.md) — CMake-Modul-Struktur
- [Cpp_Coding_Standard.md](Cpp_Coding_Standard.md) — C++ Stil
- [C_Coding_Standard.md](C_Coding_Standard.md) — C Stil
- [CMakePresets References](../references/CMakePresets.md) — Presets-Dokumentation

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-19** | **Neu: TODO-Marker (6.3)** |
| 0.5.0 | 2025-12-13 | Migration auf Blueprint v0.5: Neuer Header, Table of Contents, Encoding-Fix |
| 0.1.0 | 2025-12-05 | Initial: Modern CMake, Presets, Target-basiert, Namenskonventionen |
