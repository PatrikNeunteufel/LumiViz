# Errors.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5, ErrorCodes v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Errors.md](../../en/modules/core/Errors.md)  
> **Module:** [`cmake/core/Errors.cmake`](../../../../cmake/core/Errors.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Concept](#3-konzept)
4. [API-Reference](#4-api-referenz)
5. [Usagesbeispiele](#5-verwendungsbeispiele)
6. [Errorcode-Kategorien](#6-fehlercode-kategorien)
7. [Best Practices](#7-best-practices)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

Das `Errors.cmake` Modul stellt ein **einheitliches Error-Handling** für das gesamte Build-System bereit. Es definiert standardisierte Functions für fatale Error, Warningen und Assertions.

### Kernidee

Alle Error verwenden standardisierte Codes (E0xx, W0xx, etc.) für konsistente, nachvollziehbare Errormeldungen.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Fatale Error | Build-Abbruch mit Code |
| Warningen | Notee ohne Abbruch |
| Assertions | Interne Konsistenz-Checks |
| Feld-Validierung | Context-Requiredfelder prüfen |

### Usage durch

- Alle Module für Error-Handling
- CompilerOptions.cmake (W201)
- SourceCollect.cmake (E104, W110)
- Validation.cmake (E001, E002)
- Und viele weitere...

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| — | — | Keine (Basis-Modul, muss als erstes geladen werden) |

**Note:** `cmake_require_field()` benötigt `Context.cmake` (ctx_get), aber Context.cmake muss nach Errors.cmake geladen werden.

---

## 3. Concept

### 3.1 Error vs. Warning vs. Assertion

| Funktion | Verhalten | Usage |
|----------|-----------|------------|
| `cmake_fatal()` | Bricht Build ab | User-Error, fehlende Requiredfelder |
| `cmake_warn()` | Build läuft weiter | Deprecated, suboptimale Config |
| `cmake_assert()` | Bricht ab wenn falsch | Interne Konsistenz-Checks |

### 3.2 Errorcode-Format

```
[E|W][K][NN]

E = Error (Build bricht ab)
W = Warning (Build läuft weiter)
K = Kategorie (1 Ziffer)
NN = Nummer (2 Ziffern)
```

---

## 4. API-Reference

### 4.1 cmake_fatal()

Bricht den Build mit einem Errorcode ab.

```cmake
cmake_fatal(<CODE> <MESSAGE>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `CODE` | String | ✓ | Errorcode (z.B. `E001`, `E012`) |
| `MESSAGE` | String | ✓ | Errorbeschreibung |

**Rückgabe:** Keine (bricht Build ab)

**Ausgabe:**
```
CMake Error at cmake/core/Errors.cmake:XX (message):
  [E001] Executable 'MyApp': Requiredfeld 'name' fehlt
```

**Example:**

```cmake
cmake_fatal("E001" "Executable 'MyApp': Requiredfeld 'name' fehlt")
cmake_fatal("E002" "Solution.json nicht gefunden: ${CMAKE_SOURCE_DIR}/Solution.json")
cmake_fatal("E012" "External 'imgui': Kein Source-Feld (path/git) angegeben")
```

---

### 4.2 cmake_warn()

Gibt eine Warning aus, Build läuft weiter.

```cmake
cmake_warn(<CODE> <MESSAGE>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `CODE` | String | ✓ | Warningscode (z.B. `W001`, `W201`) |
| `MESSAGE` | String | ✓ | Warningsbeschreibung |

**Rückgabe:** Keine (gibt Warning aus)

**Ausgabe:**
```
CMake Warning at cmake/core/Errors.cmake:XX (message):
  [W001] Schema-Version < 0.1, Features eingeschränkt
```

**Example:**

```cmake
cmake_warn("W001" "Schema version < 0.1, features limited")
cmake_warn("W201" "ENABLE_CLANG_TIDY=ON but clang-tidy not found")
cmake_warn("W302" "External 'glfw': Version mismatch but offline mode - using cached")
```

---

### 4.3 cmake_assert()

Prüft eine Bedingung und bricht ab wenn falsch.

```cmake
cmake_assert(<CONDITION> <MESSAGE>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `CONDITION` | CMake-Bedingung | ✓ | Wird mit `if(NOT ...)` geprüft |
| `MESSAGE` | String | ✓ | Errorbeschreibung bei Error |

**Rückgabe:** Keine (bricht ab wenn Bedingung falsch)

**Note:** Dies ist ein `macro()`, nicht `function()`, damit CONDITION korrekt ausgewertet wird.

**Example:**

```cmake
cmake_assert(DEFINED _var "Variable muss definiert sein")
cmake_assert(_count GREATER 0 "Count muss > 0 sein")
cmake_assert("${_type}" STREQUAL "GUI" "Unerwarteter Typ")
```

---

### 4.4 cmake_require_field()

Validiert dass ein Feld in einem Context gesetzt ist.

```cmake
cmake_require_field(<CTX> <FIELD_NAME> <ENTITY_TYPE>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `CTX` | String | ✓ | Context-Prefix (z.B. `EXE_0`) |
| `FIELD_NAME` | String | ✓ | Name des zu prüfenden Felds |
| `ENTITY_TYPE` | String | ✓ | Typ für Errormeldung (z.B. "Executable") |

**Rückgabe:** Keine (bricht ab wenn Feld fehlt)

**Abhängigkeit:** Benötigt `Context.cmake` (ctx_get).

**Error:**

| Code | Bedingung |
|------|-----------|
| E001 | Feld fehlt oder ist leer |

**Example:**

```cmake
ctx_create(EXE_0)
ctx_set(EXE_0 NAME "MyApp")
# ctx_set(EXE_0 PATH "...")  # Fehlt!

cmake_require_field(EXE_0 PATH "Executable")
# → [E001] Executable 'MyApp' hat kein 'PATH' Feld
```

---

## 5. Usagesbeispiele

### 5.1 Requiredfeld-Validierung

```cmake
function(_collect_executable EXE_JSON CTX)
    _json_get_string("${EXE_JSON}" "name" _name)
    
    if("${_name}" STREQUAL "")
        cmake_fatal("E001" "Executable: Requiredfeld 'name' fehlt")
    endif()
    
    ctx_set(${CTX} NAME "${_name}")
endfunction()
```

### 5.2 External-Validierung

```cmake
function(validate_external EXT_NAME EXT_JSON)
    _json_has_key("${EXT_JSON}" "path" _has_path)
    _json_has_key("${EXT_JSON}" "git" _has_git)
    
    if(NOT _has_path AND NOT _has_git)
        cmake_fatal("E012" "External '${EXT_NAME}': Kein Source-Feld")
    endif()
endfunction()
```

### 5.3 Schema-Version prüfen

```cmake
_json_get_string("${_json}" "schemaVersion" _version)

if(NOT "${_version}" VERSION_GREATER_EQUAL "0.1")
    cmake_warn("W001" "Schema-Version ${_version} < 0.1")
endif()
```

### 5.4 Interne Assertions

```cmake
function(_internal_process DATA)
    # Interne Konsistenz prüfen
    cmake_assert(DEFINED DATA "DATA muss übergeben werden")
    cmake_assert(NOT "${DATA}" STREQUAL "" "DATA darf nicht leer sein")
    
    # ... Verarbeitung ...
endfunction()
```

---

## 6. Errorcode-Kategorien

### 6.1 Fatal Errors (E)

| Bereich | Codes | Description |
|---------|-------|--------------|
| JSON/Parsing | `E0xx` | Fehlende Requiredfelder, ungültiges JSON |
| Target-Erstellung | `E1xx` | Target existiert, Abhängigkeit fehlt |
| Externals | `E2xx` | Fetch fehlgeschlagen, Include.cmake fehlt |
| Tests | `E3xx` | Framework, Source-Error |
| AppContainer | `E4xx` | App-Struktur, Verzeichnisse |
| System Externals | `E5xx` | find_package, Komponenten |

### 6.2 Warnings (W)

| Bereich | Codes | Description |
|---------|-------|--------------|
| Deprecation | `W0xx` | Deprecatede Features/Syntax |
| Config/Validation | `W1xx` | Suboptimale Einstellungen |
| Tools/Setup | `W2xx` | Fehlende Tools |
| External-Caching | `W3xx` | Offline-Modus, Hook-Wiederverwendung |
| AppContainer | `W4xx` | Optionale Verzeichnisse |
| System Externals | `W5xx` | Backup-Usage |

→ Vollständige Reference: [ErrorCodes.md](../../../reference/ErrorCodes.md)

---

## 7. Best Practices

### 7.1 Immer Errorcode verwenden

```cmake
# ✅ Gut
cmake_fatal("E012" "External 'foo': Kein Source-Feld")

# ❌ Schlecht
message(FATAL_ERROR "External 'foo': Kein Source-Feld")
```

### 7.2 Aussagekräftige Errormeldungen

```cmake
# ✅ Gut - enthält Kontext
cmake_fatal("E001" "Executable 'MyApp': Requiredfeld 'path' fehlt")

# ❌ Schlecht - kein Kontext
cmake_fatal("E001" "Requiredfeld fehlt")
```

### 7.3 Assertions nur für interne Checks

```cmake
# ✅ Für interne Konsistenz
cmake_assert(DEFINED _internal "Interner Error")

# ❌ Nicht für User-Error
cmake_assert(_user_input "...")  # → cmake_fatal verwenden!
```

### 7.4 Früh validieren

```cmake
function(process_target NAME PATH)
    # ✅ Validierung am Anfang
    if("${NAME}" STREQUAL "")
        cmake_fatal("E001" "NAME ist Required")
    endif()
    
    # Dann erst Verarbeitung...
endfunction()
```

---

## 8. See Also

- [ErrorCodes.md](../../../reference/ErrorCodes.md) — Vollständige Errorcode-Reference
- [Debug.cmake](Debug.md) — Für Debug-Ausgaben (nicht Error)
- [Context.cmake](Context.md) — Für cmake_require_field()
- [guidelines](../../../standards/guidelines.md) — Error-Handling Konventionen

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung** |
| 0.1.2 | 2025-12-09 | W3xx Range für Externals, E218, W302 dokumentiert |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): cmake_fatal/cmake_warn/cmake_assert/cmake_require_field API |
