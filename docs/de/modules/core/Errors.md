# Errors.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5, ErrorCodes v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Errors.md](../../en/modules/core/Errors.md)  
> **Modul:** [`cmake/core/Errors.cmake`](../../../../cmake/core/Errors.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Fehlercode-Kategorien](#6-fehlercode-kategorien)
7. [Best Practices](#7-best-practices)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Das `Errors.cmake` Modul stellt ein **einheitliches Error-Handling** für das gesamte Build-System bereit. Es definiert standardisierte Funktionen für fatale Fehler, Warnungen und Assertions.

### Kernidee

Alle Fehler verwenden standardisierte Codes (E0xx, W0xx, etc.) für konsistente, nachvollziehbare Fehlermeldungen.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Fatale Fehler | Build-Abbruch mit Code |
| Warnungen | Hinweise ohne Abbruch |
| Assertions | Interne Konsistenz-Checks |
| Feld-Validierung | Context-Pflichtfelder prüfen |

### Verwendung durch

- Alle Module für Error-Handling
- CompilerOptions.cmake (W201)
- SourceCollect.cmake (E104, W110)
- Validation.cmake (E001, E002)
- Und viele weitere...

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| — | — | Keine (Basis-Modul, muss als erstes geladen werden) |

**Hinweis:** `cmake_require_field()` benötigt `Context.cmake` (ctx_get), aber Context.cmake muss nach Errors.cmake geladen werden.

---

## 3. Konzept

### 3.1 Fehler vs. Warnung vs. Assertion

| Funktion | Verhalten | Verwendung |
|----------|-----------|------------|
| `cmake_fatal()` | Bricht Build ab | User-Fehler, fehlende Pflichtfelder |
| `cmake_warn()` | Build läuft weiter | Deprecated, suboptimale Config |
| `cmake_assert()` | Bricht ab wenn falsch | Interne Konsistenz-Checks |

### 3.2 Fehlercode-Format

```
[E|W][K][NN]

E = Error (Build bricht ab)
W = Warning (Build läuft weiter)
K = Kategorie (1 Ziffer)
NN = Nummer (2 Ziffern)
```

---

## 4. API-Referenz

### 4.1 cmake_fatal()

Bricht den Build mit einem Fehlercode ab.

```cmake
cmake_fatal(<CODE> <MESSAGE>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CODE` | String | ✓ | Fehlercode (z.B. `E001`, `E012`) |
| `MESSAGE` | String | ✓ | Fehlerbeschreibung |

**Rückgabe:** Keine (bricht Build ab)

**Ausgabe:**
```
CMake Error at cmake/core/Errors.cmake:XX (message):
  [E001] Executable 'MyApp': Pflichtfeld 'name' fehlt
```

**Beispiel:**

```cmake
cmake_fatal("E001" "Executable 'MyApp': Pflichtfeld 'name' fehlt")
cmake_fatal("E002" "Solution.json nicht gefunden: ${CMAKE_SOURCE_DIR}/Solution.json")
cmake_fatal("E012" "External 'imgui': Kein Source-Feld (path/git) angegeben")
```

---

### 4.2 cmake_warn()

Gibt eine Warnung aus, Build läuft weiter.

```cmake
cmake_warn(<CODE> <MESSAGE>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CODE` | String | ✓ | Warnungscode (z.B. `W001`, `W201`) |
| `MESSAGE` | String | ✓ | Warnungsbeschreibung |

**Rückgabe:** Keine (gibt Warnung aus)

**Ausgabe:**
```
CMake Warning at cmake/core/Errors.cmake:XX (message):
  [W001] Schema-Version < 0.1, Features eingeschränkt
```

**Beispiel:**

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

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CONDITION` | CMake-Bedingung | ✓ | Wird mit `if(NOT ...)` geprüft |
| `MESSAGE` | String | ✓ | Fehlerbeschreibung bei Fehler |

**Rückgabe:** Keine (bricht ab wenn Bedingung falsch)

**Hinweis:** Dies ist ein `macro()`, nicht `function()`, damit CONDITION korrekt ausgewertet wird.

**Beispiel:**

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

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CTX` | String | ✓ | Context-Prefix (z.B. `EXE_0`) |
| `FIELD_NAME` | String | ✓ | Name des zu prüfenden Felds |
| `ENTITY_TYPE` | String | ✓ | Typ für Fehlermeldung (z.B. "Executable") |

**Rückgabe:** Keine (bricht ab wenn Feld fehlt)

**Abhängigkeit:** Benötigt `Context.cmake` (ctx_get).

**Fehler:**

| Code | Bedingung |
|------|-----------|
| E001 | Feld fehlt oder ist leer |

**Beispiel:**

```cmake
ctx_create(EXE_0)
ctx_set(EXE_0 NAME "MyApp")
# ctx_set(EXE_0 PATH "...")  # Fehlt!

cmake_require_field(EXE_0 PATH "Executable")
# → [E001] Executable 'MyApp' hat kein 'PATH' Feld
```

---

## 5. Verwendungsbeispiele

### 5.1 Pflichtfeld-Validierung

```cmake
function(_collect_executable EXE_JSON CTX)
    _json_get_string("${EXE_JSON}" "name" _name)
    
    if("${_name}" STREQUAL "")
        cmake_fatal("E001" "Executable: Pflichtfeld 'name' fehlt")
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

## 6. Fehlercode-Kategorien

### 6.1 Fatal Errors (E)

| Bereich | Codes | Beschreibung |
|---------|-------|--------------|
| JSON/Parsing | `E0xx` | Fehlende Pflichtfelder, ungültiges JSON |
| Target-Erstellung | `E1xx` | Target existiert, Abhängigkeit fehlt |
| Externals | `E2xx` | Fetch fehlgeschlagen, Include.cmake fehlt |
| Tests | `E3xx` | Framework, Source-Fehler |
| AppContainer | `E4xx` | App-Struktur, Verzeichnisse |
| System Externals | `E5xx` | find_package, Komponenten |

### 6.2 Warnings (W)

| Bereich | Codes | Beschreibung |
|---------|-------|--------------|
| Deprecation | `W0xx` | Veraltete Features/Syntax |
| Config/Validation | `W1xx` | Suboptimale Einstellungen |
| Tools/Setup | `W2xx` | Fehlende Tools |
| External-Caching | `W3xx` | Offline-Modus, Hook-Wiederverwendung |
| AppContainer | `W4xx` | Optionale Verzeichnisse |
| System Externals | `W5xx` | Backup-Verwendung |

→ Vollständige Referenz: [ErrorCodes.md](../../../reference/ErrorCodes.md)

---

## 7. Best Practices

### 7.1 Immer Fehlercode verwenden

```cmake
# ✅ Gut
cmake_fatal("E012" "External 'foo': Kein Source-Feld")

# ❌ Schlecht
message(FATAL_ERROR "External 'foo': Kein Source-Feld")
```

### 7.2 Aussagekräftige Fehlermeldungen

```cmake
# ✅ Gut - enthält Kontext
cmake_fatal("E001" "Executable 'MyApp': Pflichtfeld 'path' fehlt")

# ❌ Schlecht - kein Kontext
cmake_fatal("E001" "Pflichtfeld fehlt")
```

### 7.3 Assertions nur für interne Checks

```cmake
# ✅ Für interne Konsistenz
cmake_assert(DEFINED _internal "Interner Fehler")

# ❌ Nicht für User-Fehler
cmake_assert(_user_input "...")  # → cmake_fatal verwenden!
```

### 7.4 Früh validieren

```cmake
function(process_target NAME PATH)
    # ✅ Validierung am Anfang
    if("${NAME}" STREQUAL "")
        cmake_fatal("E001" "NAME ist Pflicht")
    endif()
    
    # Dann erst Verarbeitung...
endfunction()
```

---

## 8. Siehe auch

- [ErrorCodes.md](../../../reference/ErrorCodes.md) — Vollständige Fehlercode-Referenz
- [Debug.cmake](Debug.md) — Für Debug-Ausgaben (nicht Fehler)
- [Context.cmake](Context.md) — Für cmake_require_field()
- [guidelines](../../../standards/guidelines.md) — Error-Handling Konventionen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.2 | 2025-12-09 | W3xx Range für Externals, E218, W302 dokumentiert |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): cmake_fatal/cmake_warn/cmake_assert/cmake_require_field API |
