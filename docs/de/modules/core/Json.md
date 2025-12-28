# Json.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-18  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Json.md](../../en/modules/core/Json.md)  
> **Modul:** [`cmake/core/Json.cmake`](../../../../cmake/core/Json.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Verwendungsbeispiele](#4-verwendungsbeispiele)
5. [Rückgabewerte und Fehlerbehandlung](#5-rückgabewerte-und-fehlerbehandlung)
6. [Best Practices](#6-best-practices)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `Json.cmake` Modul stellt **JSON-Hilfsfunktionen** für das CMake Build-System bereit. Es kapselt die CMake 3.19+ `string(JSON ...)` Befehle in benutzerfreundliche Funktionen.

### Kernidee

Einheitliche, fehlertolerante JSON-Operationen für das gesamte Build-System.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Lesen | Strings, Numbers, Booleans, Objekte, Arrays |
| Prüfen | Key-Existenz, Typ-Erkennung |
| Fallbacks | Default-Werte bei fehlenden Keys |
| Konvertierung | Arrays zu CMake-Listen |

### Verwendung durch

- Solution.cmake
- ExecutableCollect.cmake
- LibraryCollect.cmake
- TestCollect.cmake
- AppCollect.cmake
- Validation.cmake

### Hinweis

Alle Funktionen sind mit `_`-Prefix markiert (private), da sie nur für interne Build-System-Verwendung gedacht sind.

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| CMake | 3.19+ | `string(JSON ...)` Befehl |

**Design-Entscheidung:** Json.cmake ist ein Basis-Modul ohne Abhängigkeiten zu anderen CMake-Modulen. Es verwendet keine Debug- oder Error-Funktionen — Fehler werden durch leere Rückgabewerte signalisiert und von aufrufenden Modulen behandelt.

→ Siehe [Offene Abklärungen](../../../projects/buildsystem/concepts/Offene_Abklaerungen_Core_Module.md) für Details zu dieser Entscheidung.

---

## 3. API-Referenz

### Funktionsübersicht

| # | Funktion | Beschreibung |
|---|----------|--------------|
| 3.1 | `_json_has_key` | Prüft Key-Existenz |
| 3.2 | `_json_get_string` | Liest String |
| 3.3 | `_json_get_string_or_default` | String mit Default |
| 3.4 | `_json_get_bool_from_key` | Liest Boolean |
| 3.5 | `_json_get_bool_or_default` | Boolean mit Default |
| 3.6 | `_json_array_length` | Array-Länge |
| 3.7 | `_json_array_get` | Array-Element |
| 3.8 | `_json_get_array_as_list` | Array als CMake-Liste |
| 3.9 | `_json_get_object` | Extrahiert Objekt |
| 3.10 | `_json_get_object_or_empty` | Objekt mit Fallback |
| 3.11 | `_json_get_type` | Typ eines Keys |
| 3.12 | `_json_get_number` | Liest Zahl |
| 3.13 | `_json_get_number_or_default` | Zahl mit Default |

---

### 3.1 _json_has_key()

Prüft ob ein Key im JSON-Objekt existiert.

```cmake
_json_has_key(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Zu prüfender Key |
| `OUT_VAR` | Variable | ✓ | Output: TRUE oder FALSE |

**Rückgabe:** TRUE wenn Key existiert, FALSE sonst

**Beispiel:**

```cmake
_json_has_key("${_json}" "name" _has_name)
if(_has_name)
    # Key existiert
endif()
```

---

### 3.2 _json_get_string()

Liest einen String-Wert aus JSON.

```cmake
_json_get_string(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `OUT_VAR` | Variable | ✓ | Output: String-Wert oder "" |

**Rückgabe:** String-Wert bei Erfolg, leerer String wenn Key fehlt

**Beispiel:**

```cmake
_json_get_string("${_json}" "name" _name)
```

---

### 3.3 _json_get_string_or_default()

Liest String-Wert mit Fallback zu Default.

```cmake
_json_get_string_or_default(<JSON_STRING> <KEY> <DEFAULT> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `DEFAULT` | String | ✓ | Fallback-Wert |
| `OUT_VAR` | Variable | ✓ | Output: String-Wert oder DEFAULT |

**Rückgabe:** String-Wert oder DEFAULT wenn Key fehlt/leer

**Beispiel:**

```cmake
_json_get_string_or_default("${_json}" "type" "CONSOLE" _type)
# _type ist "CONSOLE" wenn "type" fehlt
```

---

### 3.4 _json_get_bool_from_key()

Liest einen Boolean-Wert mit robuster Erkennung.

```cmake
_json_get_bool_from_key(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `OUT_VAR` | Variable | ✓ | Output: TRUE oder FALSE |

**Erkannte Werte:**

| TRUE | FALSE |
|------|-------|
| true, TRUE, 1, ON, YES | false, FALSE, 0, OFF, NO, leer, Key fehlt |

**Hinweis:** CMakes `string(JSON GET)` gibt für JSON `true`/`false` die Strings "true"/"false" (lowercase) zurück. Diese Funktion erkennt alle gängigen Varianten.

**Beispiel:**

```cmake
_json_get_bool_from_key("${_json}" "skip" _skip)
if(_skip)
    return()  # Überspringe
endif()
```

---

### 3.5 _json_get_bool_or_default()

Liest Boolean-Wert mit Fallback-Default wenn Key fehlt.

```cmake
_json_get_bool_or_default(<JSON_STRING> <KEY> <DEFAULT> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `DEFAULT` | Bool | ✓ | Fallback-Wert wenn Key fehlt (TRUE/FALSE) |
| `OUT_VAR` | Variable | ✓ | Output: TRUE oder FALSE |

**Erkannte Werte:**

| TRUE | FALSE |
|------|-------|
| true, TRUE, 1, ON, YES | false, FALSE, 0, OFF, NO |

**Unterschied zu `_json_get_bool_from_key`:**

| Funktion | Wenn Key fehlt |
|----------|----------------|
| `_json_get_bool_from_key` | Gibt FALSE zurück |
| `_json_get_bool_or_default` | Gibt übergebenen DEFAULT zurück |

**Beispiel:**

```cmake
# Mit Default TRUE wenn "enabled" fehlt
_json_get_bool_or_default("${_json}" "enabled" TRUE _enabled)

# Mit Default FALSE wenn "skip" fehlt  
_json_get_bool_or_default("${_json}" "skip" FALSE _skip)
if(_skip)
    message(STATUS "Übersprungen")
endif()

# Parallele Ausführung: Default TRUE für unit-Tests
_json_get_bool_or_default("${_test_json}" "parallel" TRUE _parallel)
```

---

### 3.6 _json_array_length()

Ermittelt die Länge eines JSON-Arrays.

```cmake
_json_array_length(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key des Arrays |
| `OUT_VAR` | Variable | ✓ | Output: Array-Länge (0 wenn fehlt) |

**Rückgabe:** Array-Länge oder 0 wenn Key fehlt/kein Array

**Beispiel:**

```cmake
_json_array_length("${_json}" "externals" _count)
if(_count GREATER 0)
    # Array verarbeiten
endif()
```

---

### 3.7 _json_array_get()

Liest ein Element aus einem JSON-Array.

```cmake
_json_array_get(<JSON_STRING> <KEY> <INDEX> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key des Arrays |
| `INDEX` | Int | ✓ | 0-basierter Index |
| `OUT_VAR` | Variable | ✓ | Output: Element-Wert oder "" |

**Rückgabe:** Element-Wert oder leerer String bei Fehler

**Beispiel:**

```cmake
_json_array_length("${_json}" "items" _count)
math(EXPR _last "${_count} - 1")
foreach(_idx RANGE 0 ${_last})
    _json_array_get("${_json}" "items" ${_idx} _item)
    message(STATUS "Item: ${_item}")
endforeach()
```

---

### 3.8 _json_get_array_as_list()

Liest JSON-Array und gibt es als CMake-Liste zurück.

```cmake
_json_get_array_as_list(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key des Arrays |
| `OUT_VAR` | Variable | ✓ | Output: CMake-Liste (semicolon-separated) oder "" |

**Rückgabe:** CMake-Liste oder leerer String wenn Key fehlt oder Array leer

**Beispiel:**

```cmake
# JSON: { "dependencies": ["LibA", "LibB", "LibC"] }
_json_get_array_as_list("${_json}" "dependencies" _deps)
# _deps = "LibA;LibB;LibC"

foreach(_dep IN LISTS _deps)
    message(STATUS "Dependency: ${_dep}")
endforeach()
```

**Vergleich zu manueller Iteration:**

```cmake
# Alt (manuell):
_json_array_length("${_json}" "items" _count)
set(_items "")
if(_count GREATER 0)
    math(EXPR _last "${_count} - 1")
    foreach(_i RANGE 0 ${_last})
        _json_array_get("${_json}" "items" ${_i} _item)
        list(APPEND _items "${_item}")
    endforeach()
endif()

# Neu (mit _json_get_array_as_list):
_json_get_array_as_list("${_json}" "items" _items)
```

---

### 3.9 _json_get_object()

Extrahiert ein verschachteltes JSON-Objekt als String.

```cmake
_json_get_object(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key des Objekts |
| `OUT_VAR` | Variable | ✓ | Output: JSON-String des Objekts oder "" |

**Rückgabe:** JSON-String des verschachtelten Objekts oder leerer String

**Beispiel:**

```cmake
_json_get_object("${_json}" "settings" _settings)
_json_get_string("${_settings}" "cxx_standard" _std)
```

---

### 3.10 _json_get_object_or_empty()

Wie `_json_get_object`, aber gibt "{}" zurück wenn Key fehlt.

```cmake
_json_get_object_or_empty(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key des Objekts |
| `OUT_VAR` | Variable | ✓ | Output: JSON-String oder "{}" |

**Rückgabe:** JSON-String oder "{}" — ermöglicht sichere Weiterverarbeitung

**Beispiel:**

```cmake
_json_get_object_or_empty("${_json}" "options" _options)
# _options ist mindestens "{}", nie leer
_json_get_string_or_default("${_options}" "key" "default" _val)
```

---

### 3.11 _json_get_type()

Ermittelt den JSON-Typ eines Werts.

```cmake
_json_get_type(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `OUT_VAR` | Variable | ✓ | Output: Typ-String |

**Mögliche Rückgabewerte:**

| Wert | Bedeutung |
|------|-----------|
| `NULL` | null |
| `BOOLEAN` | true/false |
| `NUMBER` | Numerisch |
| `STRING` | String |
| `ARRAY` | Array [...] |
| `OBJECT` | Objekt {...} |
| `""` | Key existiert nicht |

**Beispiel:**

```cmake
_json_get_type("${_json}" "version" _vtype)
if("${_vtype}" STREQUAL "STRING")
    _json_get_string("${_json}" "version" _ver)
elseif("${_vtype}" STREQUAL "OBJECT")
    _json_get_object("${_json}" "version" _vobj)
endif()
```

---

### 3.12 _json_get_number()

Liest einen numerischen Wert.

```cmake
_json_get_number(<JSON_STRING> <KEY> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `OUT_VAR` | Variable | ✓ | Output: Numerischer Wert oder "" |

**Beispiel:**

```cmake
_json_get_number("${_json}" "timeout" _timeout)
if(NOT "${_timeout}" STREQUAL "")
    message("Timeout: ${_timeout}s")
endif()
```

---

### 3.13 _json_get_number_or_default()

Liest einen numerischen Wert mit Fallback.

```cmake
_json_get_number_or_default(<JSON_STRING> <KEY> <DEFAULT> <OUT_VAR>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `KEY` | String | ✓ | Key |
| `DEFAULT` | Number | ✓ | Fallback-Wert |
| `OUT_VAR` | Variable | ✓ | Output: Numerischer Wert oder Default |

**Beispiel:**

```cmake
# Test-Timeout mit Default 30 Sekunden
_json_get_number_or_default("${_json}" "timeout" 30 _timeout)
message("Timeout: ${_timeout}s")
```

---

## 4. Verwendungsbeispiele

### 4.1 Solution.json parsen

```cmake
# JSON laden
file(READ "${CMAKE_SOURCE_DIR}/Solution.json" _json)

# Pflichtfelder lesen
_json_get_string("${_json}" "schemaVersion" _schema)
_json_get_object("${_json}" "solution" _solution)
_json_get_string("${_solution}" "name" _name)

# Optionale Felder mit Defaults
_json_get_string_or_default("${_solution}" "version" "0.0.0" _version)
```

### 4.2 Executables-Array iterieren

```cmake
_json_get_object("${_json}" "solution" _solution)
_json_array_length("${_solution}" "executables" _exe_count)

if(_exe_count GREATER 0)
    math(EXPR _last "${_exe_count} - 1")
    foreach(_idx RANGE 0 ${_last})
        _json_array_get("${_solution}" "executables" ${_idx} _exe_json)
        _json_get_string("${_exe_json}" "name" _name)
        _json_get_bool_from_key("${_exe_json}" "skip" _skip)
        
        if(NOT _skip)
            # Executable verarbeiten
        endif()
    endforeach()
endif()
```

### 4.3 Test-Konfiguration mit Defaults

```cmake
# Test-JSON: { "name": "UnitTests", "type": "unit", "timeout": 60 }
_json_get_string("${_test_json}" "name" _name)
_json_get_string("${_test_json}" "type" _type)

# Optional mit Default
_json_get_number_or_default("${_test_json}" "timeout" 30 _timeout)
_json_get_bool_or_default("${_test_json}" "skip" FALSE _skip)
_json_get_bool_or_default("${_test_json}" "parallel" TRUE _parallel)

# Labels als Liste
_json_get_array_as_list("${_test_json}" "labels" _labels)
if("${_labels}" STREQUAL "")
    set(_labels "${_type}")  # Default: Typ als Label
endif()
```

---

## 5. Rückgabewerte und Fehlerbehandlung

### 5.1 Fehlertolerantes Design

Alle Funktionen geben bei Fehlern sinnvolle Defaults zurück:

| Funktion | Bei Fehler |
|----------|------------|
| `_json_has_key` | FALSE |
| `_json_get_string` | "" |
| `_json_get_string_or_default` | DEFAULT |
| `_json_get_bool_from_key` | FALSE |
| `_json_get_bool_or_default` | DEFAULT |
| `_json_get_number` | "" |
| `_json_get_number_or_default` | DEFAULT |
| `_json_array_length` | 0 |
| `_json_array_get` | "" |
| `_json_get_array_as_list` | "" |
| `_json_get_object` | "" |
| `_json_get_object_or_empty` | "{}" |
| `_json_get_type` | "" |

### 5.2 Keine Exceptions

Das Modul wirft keine Fehler. Die Validierung von Pflichtfeldern erfolgt durch die aufrufenden Module (z.B. Validation.cmake mit cmake_fatal).

---

## 6. Best Practices

### 6.1 Immer Key-Existenz prüfen bei kritischen Feldern

```cmake
# ✅ Gut - explizite Prüfung
_json_has_key("${_json}" "name" _has_name)
if(NOT _has_name)
    cmake_fatal("E001" "Pflichtfeld 'name' fehlt")
endif()
_json_get_string("${_json}" "name" _name)

# ❌ Schlecht - keine Prüfung
_json_get_string("${_json}" "name" _name)
# _name könnte leer sein!
```

### 6.2 _or_default für optionale Felder

```cmake
# ✅ Gut - Default-Wert
_json_get_string_or_default("${_json}" "type" "CONSOLE" _type)
_json_get_bool_or_default("${_json}" "skip" FALSE _skip)

# ❌ Schlecht - manuelle Prüfung
_json_get_string("${_json}" "type" _type)
if("${_type}" STREQUAL "")
    set(_type "CONSOLE")
endif()
```

### 6.3 Typ prüfen bei polymorphen Feldern

```cmake
_json_get_type("${_json}" "version" _type)
if("${_type}" STREQUAL "STRING")
    # Einfache Version: "1.0.0"
elseif("${_type}" STREQUAL "OBJECT")
    # Komplexe Version: { "major": 1, "minor": 0 }
endif()
```

### 6.4 _json_get_array_as_list für Listen

```cmake
# ✅ Gut - kompakt mit _json_get_array_as_list
_json_get_array_as_list("${_json}" "dependencies" _deps)
foreach(_dep IN LISTS _deps)
    target_link_libraries(${_target} PRIVATE ${_dep})
endforeach()

# ❌ Umständlich - manuelle Iteration
_json_array_length("${_json}" "dependencies" _count)
if(_count GREATER 0)
    math(EXPR _last "${_count} - 1")
    foreach(_i RANGE 0 ${_last})
        _json_array_get("${_json}" "dependencies" ${_i} _dep)
        target_link_libraries(${_target} PRIVATE ${_dep})
    endforeach()
endif()
```

---

## 7. Siehe auch

- [Validation.cmake](Validation.md) — Schema-Validierung mit Json.cmake
- [Solution.cmake](../project/Solution.md) — Hauptnutzer von Json.cmake
- [TestCollect.cmake](../project/TestCollect.md) — Verwendet _json_get_bool_or_default, _json_get_array_as_list
- [CMake string(JSON)](https://cmake.org/cmake/help/latest/command/string.html#json) — CMake-Dokumentation

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.6.0** | **2025-12-18** | **Neu: _json_get_bool_or_default() für Boolean mit Default, _json_get_array_as_list() für Array→Liste Konvertierung. Zentralisierung aller JSON-Helper (zuvor teilweise in TestCollect.cmake dupliziert)** |
| 0.5.1 | 2025-12-17 | Neu: _json_get_number(), _json_get_number_or_default() für numerische Werte |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): _json_has_key, _json_get_string, _json_get_string_or_default, _json_get_bool_from_key, _json_array_length, _json_array_get, _json_get_object, _json_get_object_or_empty, _json_get_type |
