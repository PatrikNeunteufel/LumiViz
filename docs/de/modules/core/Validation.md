# Validation.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Validation.md](../../en/modules/core/Validation.md)  
> **Modul:** [`cmake/core/Validation.cmake`](../../../../cmake/core/Validation.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Fehlerbehandlung](#4-fehlerbehandlung)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Das `Validation.cmake` Modul bietet **JSON-Schema-Validierung** für Solution.json und Externals. Es stellt sicher, dass alle Konfigurationen korrekt und vollständig sind.

### Kernidee

Frühe Validierung nach dem Fail-Fast-Prinzip — Fehler werden erkannt bevor sie zu kryptischen CMake-Fehlern führen.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Pflichtfelder | Prüfung auf Existenz |
| Source-Felder | Genau ein Source-Typ pro External |
| Schema-Version | Kompatibilitätsprüfung |
| Best Practices | Include.cmake Qualitätschecks |

### Verwendung durch

- Solution.cmake
- Orchestrator.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Errors.cmake | 0.1.2 | `cmake_fatal`, `cmake_warn` |
| Json.cmake | 0.1.1 | `_json_has_key`, `_json_get_string`, `_json_get_string_or_default` |

---

## 3. API-Referenz

### 3.1 validate_external_source()

Prüft dass ein External genau ein Source-Feld hat.

```cmake
validate_external_source(<EXT_NAME> <EXT_JSON>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `EXT_NAME` | String | ✓ | Name des Externals |
| `EXT_JSON` | String | ✓ | JSON-String des External-Objekts |

**Erlaubte Source-Felder:**

- `path` — Lokales External
- `git` — Fetched via Git
- `vcpkg` — Via vcpkg (geplant)
- `conan` — Via Conan (geplant)
- `find_package` — System External (Phase 9)

**Fehler:**

| Code | Bedingung |
|------|-----------|
| E012 | Kein Source-Feld vorhanden |
| E012 | Mehrere Source-Felder vorhanden |

**Beispiel:**

```cmake
validate_external_source("bass" "${_ext_json}")
```

---

### 3.2 validate_required_fields()

Prüft dass alle angegebenen Pflichtfelder existieren.

```cmake
validate_required_fields(<JSON_STRING> <ENTITY_TYPE> <ENTITY_NAME> FIELDS <field1> [field2...])
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `JSON_STRING` | String | ✓ | JSON-Objekt als String |
| `ENTITY_TYPE` | String | ✓ | Typ für Fehlermeldung |
| `ENTITY_NAME` | String | ✓ | Name für Fehlermeldung |
| `FIELDS` | List | ✓ | Liste der Pflichtfelder |

**Fehler:**

| Code | Bedingung |
|------|-----------|
| E001 | Ein Pflichtfeld fehlt |

**Beispiel:**

```cmake
validate_required_fields("${_json}" "Executable" "MyApp" FIELDS name path)
```

---

### 3.3 validate_fetched_external()

Prüft dass ein Fetched External einen Versions-Specifier hat.

```cmake
validate_fetched_external(<EXT_NAME> <EXT_JSON>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `EXT_NAME` | String | ✓ | Name des Externals |
| `EXT_JSON` | String | ✓ | JSON-String des External-Objekts |

**Erforderlich (mindestens eines):**

- `tag` — Git Tag (z.B. "v1.0.0")
- `branch` — Git Branch (z.B. "main")
- `commit` — Git Commit SHA

**Fehler:**

| Code | Bedingung |
|------|-----------|
| E215 | Weder tag, branch noch commit angegeben |

**Beispiel:**

```cmake
validate_fetched_external("glfw" "${_ext_json}")
```

---

### 3.4 validate_local_external()

Prüft dass Include.cmake für ein lokales External existiert.

```cmake
validate_local_external(<EXT_NAME> <EXT_JSON>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `EXT_NAME` | String | ✓ | Name des Externals |
| `EXT_JSON` | String | ✓ | JSON-String des External-Objekts |

**Prüft:**

1. Custom "include" Feld oder
2. Convention: `${path}/Include.cmake`

**Fehler:**

| Code | Bedingung |
|------|-----------|
| E213 | Include.cmake nicht gefunden |

**Beispiel:**

```cmake
validate_local_external("bass" "${_ext_json}")
```

---

### 3.5 validate_solution_schema()

Prüft die Schema-Version der Solution.json.

```cmake
validate_solution_schema(<SOLUTION_JSON>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `SOLUTION_JSON` | String | ✓ | Kompletter JSON-String der Solution.json |

**Warnungen:**

| Code | Bedingung |
|------|-----------|
| W001 | schemaVersion fehlt |
| W001 | schemaVersion < 0.1 |

**Beispiel:**

```cmake
file(READ "${CMAKE_SOURCE_DIR}/Solution.json" _json)
validate_solution_schema("${_json}")
```

---

### 3.6 validate_local_external_include()

Best-Practice-Checks für Include.cmake.

```cmake
validate_local_external_include(<INCLUDE_FILE> <EXT_NAME>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `INCLUDE_FILE` | Path | ✓ | Voller Pfad zur Include.cmake |
| `EXT_NAME` | String | ✓ | Name des Externals |

**Warnungen:**

| Code | Bedingung |
|------|-----------|
| W103 | `add_executable()` gefunden (IDE Clutter) |
| W104 | `add_subdirectory(examples|tests|...)` gefunden |

**Beispiel:**

```cmake
validate_local_external_include("${_include_path}" "imgui")
```

---

## 4. Fehlerbehandlung

### 4.1 Fatal Errors

| Code | Funktion | Beschreibung |
|------|----------|--------------|
| E001 | validate_required_fields | Pflichtfeld fehlt |
| E012 | validate_external_source | Kein/mehrere Source-Felder |
| E213 | validate_local_external | Include.cmake nicht gefunden |
| E215 | validate_fetched_external | Kein tag/branch/commit |

### 4.2 Warnings

| Code | Funktion | Beschreibung |
|------|----------|--------------|
| W001 | validate_solution_schema | Schema-Version fehlt/veraltet |
| W103 | validate_local_external_include | add_executable() gefunden |
| W104 | validate_local_external_include | Example-Verzeichnisse eingebunden |

---

## 5. Verwendungsbeispiele

### 5.1 In Solution.cmake

```cmake
file(READ "${CMAKE_SOURCE_DIR}/Solution.json" _json)

# Schema prüfen
validate_solution_schema("${_json}")

# solution.name ist Pflicht
_json_get_object("${_json}" "solution" _solution)
validate_required_fields("${_solution}" "Solution" "" FIELDS name)
```

### 5.2 In Orchestrator.cmake

```cmake
function(_orchestrate_external EXT_NAME EXT_JSON)
    # Source-Feld prüfen
    validate_external_source("${EXT_NAME}" "${EXT_JSON}")
    
    _json_has_key("${EXT_JSON}" "path" _is_local)
    _json_has_key("${EXT_JSON}" "git" _is_git)
    
    if(_is_local)
        validate_local_external("${EXT_NAME}" "${EXT_JSON}")
        # ...
    elseif(_is_git)
        validate_fetched_external("${EXT_NAME}" "${EXT_JSON}")
        # ...
    endif()
endfunction()
```

### 5.3 Executable-Validierung

```cmake
function(_collect_executable EXE_JSON CTX)
    _json_get_string("${EXE_JSON}" "name" _name)
    
    # Pflichtfelder validieren
    validate_required_fields("${EXE_JSON}" "Executable" "${_name}" 
        FIELDS name path)
    
    # ...
endfunction()
```

---

## 6. Siehe auch

- [Errors.cmake](Errors.md) — cmake_fatal, cmake_warn
- [Json.cmake](Json.md) — JSON-Hilfsfunktionen
- [Solution.cmake](../project/Solution.md) — Verwendet Validation
- [Orchestrator.cmake](../externals/Orchestrator.md) — Verwendet Validation
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E001, E012, E213, E215, W001, W103, W104

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): validate_external_source, validate_required_fields, validate_fetched_external, validate_local_external, validate_solution_schema, validate_local_external_include |
