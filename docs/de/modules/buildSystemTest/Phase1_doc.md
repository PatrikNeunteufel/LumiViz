# phase1.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Pfad:** `cmake/buildSystemTest/phase1.cmake`  
> **Status:** Stabil  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Tests](#2-tests)
3. [Erfolgs-Flag](#3-erfolgs-flag)
4. [Siehe auch](#4-siehe-auch)
5. [Changelog](#5-changelog)

---

## 1. Übersicht

**Phase 1** testet die **Core-Module** — die fundamentalen Bausteine des Build-Systems.

| Aspekt | Beschreibung |
|--------|--------------|
| **Zweck** | Core Module Validation |
| **Debug-ID** | `PHASE1_TEST` |
| **Abhängigkeiten** | Alle Core-Module müssen geladen sein |

---

## 2. Tests

### 2.1 Context API

```cmake
ctx_create(TEST_CTX)
ctx_set(TEST_CTX NAME "TestTarget")
ctx_get(TEST_CTX NAME _test_name)
```

| Test | Prüft |
|------|-------|
| `ctx_create` | Context erstellen |
| `ctx_set` | Werte setzen |
| `ctx_get` | Werte lesen |

### 2.2 JSON Helpers

| Funktion | Prüft |
|----------|-------|
| `_json_has_key` | Key-Existenz |
| `_json_get_string` | String-Werte |
| `_json_get_string_or_default` | Default-Werte |
| `_json_get_bool_from_key` | Boolean-Werte |
| `_json_get_type` | Typ-Erkennung |

### 2.3 Debug System

| Funktion | Prüft |
|----------|-------|
| `dbg_init` | Debug-Initialisierung |
| `dbg` | Debug-Ausgaben (alle Level) |
| `dbgspace` | Leerzeilen |
| `enddbgblock` | Block-Ende |

### 2.4 Warning System

| Funktion | Prüft |
|----------|-------|
| `cmake_warn` | Nicht-fatale Warnungen |

---

## 3. Erfolgs-Flag

```cmake
set(PHASE1_TEST_PASSED TRUE CACHE BOOL "Phase 1 Test passed" FORCE)
```

---

## 4. Siehe auch

- [Errors.md](../modules/core/Errors.md)
- [Debug.md](../modules/core/Debug.md)
- [Json.md](../modules/core/Json.md)
- [Context.md](../modules/core/Context.md)
- [Validation.md](../modules/core/Validation.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-05 | Initial |
