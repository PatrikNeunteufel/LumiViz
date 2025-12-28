# Context.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Context.md](../../en/modules/core/Context.md)  
> **Modul:** [`cmake/core/Context.cmake`](../../../../cmake/core/Context.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Standard Context Keys](#6-standard-context-keys)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Best Practices](#8-best-practices)
9. [Debugging](#9-debugging)
10. [Bekannte Einschränkungen](#10-bekannte-einschränkungen)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Übersicht

Das `Context.cmake` Modul implementiert ein **Context-Objekt-Pattern** für isolierte Namensräume. Es ermöglicht die saubere Verwaltung von Target-Daten während der CMake-Konfiguration ohne globale Variablen.

### Kernidee

Statt globaler Variablen nutzt jede Executable/Library einen eigenen Namensraum mit eindeutigem Präfix.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Namensräume | Isolierte Daten pro Target |
| Propagation | Werte über alle Funktionsebenen verfügbar |
| Debugging | `ctx_dump()` für Diagnose |

### Verwendung durch

- ExecutableCollect.cmake
- LibraryCollect.cmake
- TestCollect.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| — | — | Keine (Basis-Modul) |

**Design-Entscheidung:** Context.cmake ist bewusst ohne Abhängigkeiten zu Debug.cmake oder Errors.cmake implementiert. Als fundamentales Basis-Modul, das von anderen Modulen (inkl. Debug.cmake indirekt) verwendet wird, vermeidet dies zirkuläre Abhängigkeiten. Fehlerhafte Verwendung führt zu leeren Rückgabewerten statt Exceptions.

→ Siehe [Offene Abklärungen](../../../Offene_Abklaerungen_Core_Module.md) für Details zu dieser Entscheidung.

---

## 3. Konzept

### 3.1 Warum Context-Objekte?

**Problem mit globalen Variablen:**
```cmake
# Schlecht: Globale Variablen
set(EXE_NAME "App1")
set(EXE_PATH "src/app1")
# ... später überschrieben ...
set(EXE_NAME "App2")  # App1-Daten verloren!
```

**Lösung mit Context:**
```cmake
# Gut: Isolierte Namensräume
ctx_create(EXE_0)
ctx_set(EXE_0 NAME "App1")
ctx_set(EXE_0 PATH "src/app1")

ctx_create(EXE_1)
ctx_set(EXE_1 NAME "App2")
ctx_set(EXE_1 PATH "src/app2")

# Beide existieren parallel
ctx_get(EXE_0 NAME _name)  # "App1"
ctx_get(EXE_1 NAME _name)  # "App2"
```

### 3.2 GLOBAL PROPERTY Implementierung

Der Context verwendet **GLOBAL PROPERTY** für die Speicherung, nicht `PARENT_SCOPE`. Dies hat wichtige Vorteile:

| Methode | Scope-Propagierung | Problem |
|---------|-------------------|---------|
| `PARENT_SCOPE` | Eine Ebene nach oben | Verschachtelte Funktionen verlieren Werte |
| `GLOBAL PROPERTY` | Überall verfügbar | ✅ Funktioniert über alle Scopes |

**Beispiel des Problems mit PARENT_SCOPE:**
```cmake
function(outer)
    function(inner)
        set(VAR "value" PARENT_SCOPE)  # Geht nur zu outer()
    endfunction()
    inner()
    # VAR ist hier gesetzt
endfunction()
outer()
# VAR ist hier NICHT gesetzt!
```

**Mit GLOBAL PROPERTY:**
```cmake
function(outer)
    function(inner)
        set_property(GLOBAL PROPERTY MY_VAR "value")
    endfunction()
    inner()
endfunction()
outer()
get_property(_val GLOBAL PROPERTY MY_VAR)  # "value" ✅
```

### 3.3 Context-Prefixe Konvention

| Prefix | Verwendung |
|--------|------------|
| `EXE_` | Executables (`EXE_0`, `EXE_1`, `EXE_MyApp`) |
| `LIB_` | Libraries (`LIB_0`, `LIB_CoreLib`) |
| `TEST_` | Tests (`TEST_0`, `TEST_UnitTests`) |

---

## 4. API-Referenz

### 4.1 ctx_create()

Erstellt einen neuen Context mit dem gegebenen Prefix.

```cmake
ctx_create(<PREFIX>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `PREFIX` | String | ✓ | Eindeutiger Namensraum-Identifier |

**Rückgabe:** Keine (initialisiert GLOBAL PROPERTY `${PREFIX}_KEYS`)

**Beispiel:**

```cmake
ctx_create(EXE_MyApp)
ctx_create(LIB_CoreLib)
ctx_create(TEST_UnitTests)
```

---

### 4.2 ctx_set()

Setzt einen Wert im Context.

```cmake
ctx_set(<PREFIX> <KEY> <VALUE>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `PREFIX` | String | ✓ | Context-Prefix |
| `KEY` | String | ✓ | Schlüssel (UPPER_SNAKE_CASE empfohlen) |
| `VALUE` | String/List | ✓ | Zu speichernder Wert |

**Rückgabe:** Keine (speichert als GLOBAL PROPERTY `${PREFIX}_${KEY}`)

**Beispiel:**

```cmake
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp VERSION "1.0.0")
ctx_set(EXE_MyApp EXTERNALS "bass;imgui;glfw")
```

---

### 4.3 ctx_get()

Liest einen Wert aus dem Context.

```cmake
ctx_get(<PREFIX> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `PREFIX` | String | ✓ | Context-Prefix |
| `KEY` | String | ✓ | Schlüssel |
| `OUT_VAR` | Variable | ✓ | Variable für den Rückgabewert |

**Rückgabe:** Wert in `OUT_VAR` (leerer String wenn Key nicht existiert)

**Beispiel:**

```cmake
ctx_get(EXE_MyApp NAME _name)
ctx_get(EXE_MyApp VERSION _version)
ctx_get(EXE_MyApp EXTERNALS _externals)

message(STATUS "Name: ${_name}")        # MyApp
message(STATUS "Version: ${_version}")  # 1.0.0

foreach(_ext IN LISTS _externals)
    message(STATUS "External: ${_ext}")
endforeach()
```

---

### 4.4 ctx_dump()

Debug-Ausgabe aller Keys im Context. Nur aktiv wenn `DEBUG_CONTEXT=ON`.

```cmake
ctx_dump(<PREFIX>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `PREFIX` | String | ✓ | Context-Prefix |

**Rückgabe:** Keine (gibt auf stdout aus wenn `DEBUG_CONTEXT=ON`)

**Aktivierung:**

```bash
cmake -B build -DDEBUG_CONTEXT=ON
```

**Beispiel:**

```cmake
ctx_dump(EXE_MyApp)
```

**Ausgabe:**

```
-- === Context Dump: EXE_MyApp ===
--   NAME = MyApp
--   VERSION = 1.0.0
--   TYPE = GUI
--   EXTERNALS = bass;imgui;glfw
-- === End Context: EXE_MyApp ===
```

---

## 5. Verwendungsbeispiele

### 5.1 Executable-Pipeline

```cmake
# In ExecutableCollect.cmake
function(_collect_executable EXE_JSON CTX)
    # JSON-Daten extrahieren
    _json_get_string("${EXE_JSON}" "name" _name)
    _json_get_string("${EXE_JSON}" "path" _path)
    _json_get_bool_from_key("${EXE_JSON}" "skip" _skip)
    
    # In Context speichern
    ctx_set(${CTX} NAME "${_name}")
    ctx_set(${CTX} PATH "${_path}")
    ctx_set(${CTX} SKIP "${_skip}")
endfunction()

# In Executables.cmake
foreach(_idx RANGE 0 ${_exe_count})
    ctx_create(EXE_${_idx})
    _collect_executable("${_exe_json}" EXE_${_idx})
    
    # Werte sind sofort verfügbar (dank GLOBAL PROPERTY)
    ctx_get(EXE_${_idx} NAME _name)
    ctx_get(EXE_${_idx} SKIP _skip)
    
    if(_skip)
        continue()
    endif()
    
    _create_executable_target(EXE_${_idx})
endforeach()
```

### 5.2 Mehrere Contexts parallel

```cmake
# Beide Contexts existieren unabhängig
ctx_create(EXE_Editor)
ctx_set(EXE_Editor NAME "Editor")
ctx_set(EXE_Editor TYPE "GUI")

ctx_create(EXE_CLI)
ctx_set(EXE_CLI NAME "CLI")
ctx_set(EXE_CLI TYPE "CONSOLE")

# Werte überschreiben sich nicht
ctx_get(EXE_Editor TYPE _type1)  # "GUI"
ctx_get(EXE_CLI TYPE _type2)     # "CONSOLE"
```

---

## 6. Standard Context Keys

### 6.1 Für Executables

| Key | Typ | Beschreibung |
|-----|-----|--------------|
| `NAME` | String | Target-Name |
| `DISPLAY_NAME` | String | Anzeigename |
| `DESCRIPTION` | String | Beschreibung |
| `VERSION` | String | Version (SemVer) |
| `PATH` | String | Source-Verzeichnis (relativ) |
| `TYPE` | String | GUI, CONSOLE, CLI, HEADLESS, WORKER |
| `SKIP` | Boolean | true = überspringen |
| `PCH_ENABLED` | Boolean | Precompiled Header aktiviert |
| `PCH_HEADER` | String | PCH Header-Datei |
| `DEPENDENCIES` | List | Interne Dependencies |
| `EXTERNALS` | List | Externe Dependencies |
| `DEFINES` | List | Preprocessor-Definitionen |
| `COMPILE_OPTIONS` | List | Compiler-Optionen |
| `LINK_OPTIONS` | List | Linker-Optionen |
| `PLATFORMS` | List | Zielplattformen |

### 6.2 Für Libraries

| Key | Typ | Beschreibung |
|-----|-----|--------------|
| `NAME` | String | Target-Name |
| `TYPE` | String | STATIC, SHARED, INTERFACE, OBJECT |
| `PUBLIC_HEADERS` | List | Öffentliche Header |
| `PRIVATE_HEADERS` | List | Private Header |
| `ALIAS` | String | Namespace-Alias (z.B. `MyProject::Core`) |
| ... | ... | (wie Executable) |

---

## 7. Fehlerbehandlung

Das Context-Modul selbst wirft keine spezifischen Error Codes. Fehlerhafte Verwendung führt zu:

| Situation | Verhalten |
|-----------|-----------|
| `ctx_get` ohne `ctx_create` | Leerer String |
| Key existiert nicht | Leerer String |
| Falscher PREFIX | Leerer String (stille Fehler) |

**Empfehlung:** Bei kritischen Keys explizit prüfen:

```cmake
ctx_get(${CTX} NAME _name)
if("${_name}" STREQUAL "")
    cmake_fatal("E001" "Context ${CTX}: NAME fehlt")
endif()
```

---

## 8. Best Practices

| Do | Don't |
|----|-------|
| `ctx_create()` vor erstem `ctx_set()` | Ohne Create direkt setzen |
| UPPER_SNAKE_CASE für Keys | lowercase-keys |
| Eindeutige Prefixes (EXE_, LIB_, TEST_) | Generische Prefixes |
| Listen als Semikolon-getrennt | Leerzeichen-getrennte Listen |
| `DEBUG_CONTEXT` für Fehlersuche | Manuelle Property-Abfragen |

### Design-Entscheidung: Minimale API

| Funktion | Status | Begründung |
|----------|--------|------------|
| `ctx_create` | ✅ | Grundfunktion |
| `ctx_set` | ✅ | Grundfunktion |
| `ctx_get` | ✅ | Grundfunktion |
| `ctx_dump` | ✅ | Debug-only, sehr nützlich |
| `ctx_has` | ❌ | `ctx_get` + `if(DEFINED)` reicht |
| `ctx_append` | ❌ | `ctx_get` + `list(APPEND)` + `ctx_set` explizit |

---

## 9. Debugging

### 9.1 Wert ist leer

**Problem:** `ctx_get()` gibt leeren String zurück.

**Mögliche Ursachen:**

1. `ctx_set()` wurde nie aufgerufen
2. Falscher PREFIX oder KEY
3. Tippfehler in Key-Name

**Lösung:**

```bash
cmake -B build -DDEBUG_CONTEXT=ON
```

```cmake
ctx_dump(EXE_MyApp)
```

### 9.2 Werte werden nicht propagiert

**Problem:** Werte sind in aufrufender Funktion nicht verfügbar.

**Ursache:** Altes Modul mit PARENT_SCOPE statt GLOBAL PROPERTY.

**Lösung:** Modul auf v0.1.0+ aktualisieren.

---

## 10. Bekannte Einschränkungen

| Einschränkung | Beschreibung |
|---------------|--------------|
| Keine Typisierung | Alle Werte sind Strings |
| Kein Löschen | Keys können nicht entfernt werden (nicht nötig) |
| Globaler Namespace | Prefixe müssen eindeutig sein |

---

## 11. Siehe auch

- [Errors.cmake](Errors.md) — Fehlerbehandlung
- [Debug.cmake](Debug.md) — Debug-System
- [ExecutableCollect.cmake](../project/ExecutableCollect.md) — Verwendet Context
- [LibraryCollect.cmake](../project/LibraryCollect.md) — Verwendet Context
- [guidelines](../../../concepts/guidelines.md) — Coding-Konventionen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung, Standard Context Keys Abschnitt** |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): GLOBAL PROPERTY statt PARENT_SCOPE, ctx_create/set/get/dump API |
