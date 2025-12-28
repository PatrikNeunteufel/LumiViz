# TestCollect.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [TestCollect.md](../../en/modules/project/TestCollect.md)  
> **Module:** [`cmake/project/TestCollect.cmake`](../../../../cmake/project/TestCollect.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Context-Keys](#4-context-keys)
5. [Default-Werte](#5-default-werte)
6. [JSON-Mapping](#6-json-mapping)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `TestCollect.cmake` Modul **sammelt Test-Daten** aus JSON und speichert sie in einem Context. Es behandelt Default-Werte und Pfad-Konventionen.

### Kernidee

Analog zu ExecutableCollect/LibraryCollect — Trennung von Collect und Create.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| JSON-Parsing | Alle Test-Felder extrahieren |
| Defaults | Framework, Timeout, Labels |
| Konventionen | Pfad-Defaults |

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Context.cmake | 0.5.0 | `ctx_set` |
| Json.cmake | 0.5.0 | Alle `_json_*` Functions |
| Debug.cmake | 0.5.0 | `dbg` |

---

## 3. API-Reference

### 3.1 _collect_test()

Sammelt alle Felder eines Tests aus JSON in einen Context.

```cmake
_collect_test(<TEST_JSON> <CTX>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `TEST_JSON` | String | ✓ | JSON-String des Tests |
| `CTX` | String | ✓ | Context-Prefix (z.B. `TEST_0`) |

---

## 4. Context-Keys

| Key | Typ | Default | Description |
|-----|-----|---------|--------------|
| `NAME` | String | — | Test-Name (Required) |
| `DISPLAY_NAME` | String | NAME | Anzeigename |
| `VERSION` | String | Solution-Version | Version |
| `TYPE` | String | "unit" | unit, integration, e2e |
| `FRAMEWORK` | String | "doctest" | Test-Framework |
| `PATH` | Path | `projects/tests/{name}/src` | Source-Pfad |
| `TARGET` | String | "" | Zu testendes Target |
| `DEPENDENCIES` | List | "" | Interne Dependencies |
| `EXTERNALS` | List | "" | Externe Dependencies |
| `EXTERNAL_OPTIONS` | JSON | "{}" | External-Optionen |
| `TIMEOUT` | Int | 60 | Timeout in Sekunden |
| `LABELS` | List | [TYPE] | CTest Labels |
| `PARALLEL` | Bool | TRUE | Parallel ausführbar |
| `SKIP` | Bool | FALSE | Überspringen |
| `PLATFORMS` | List | "" | Plattform-Filter |
| `DEFINES` | List | "" | Präprozessor-Definitionen |
| `COMPILE_OPTIONS` | List | "" | Compiler-Optionen |

---

## 5. Default-Werte

| Konstante | Wert | Description |
|-----------|------|--------------|
| `_TEST_DEFAULT_TYPE` | "unit" | Standard Test-Typ |
| `_TEST_DEFAULT_FRAMEWORK` | "doctest" | Standard Framework |
| `_TEST_DEFAULT_TIMEOUT` | 60 | Sekunden |
| `_TEST_DEFAULT_PARALLEL` | TRUE | Parallel erlaubt |

---

## 6. JSON-Mapping

### 6.1 Minimale JSON

```json
{
    "name": "CoreLibTests"
}
```

### 6.2 Vollständige JSON

```json
{
    "name": "CoreLibTests",
    "displayName": "Core Library Tests",
    "type": "unit",
    "framework": "doctest",
    "path": "tests/core",
    "target": "CoreLib",
    "dependencies": ["UtilLib"],
    "externals": ["doctest"],
    "timeout": 120,
    "labels": ["unit", "core", "fast"],
    "parallel": true,
    "skip": false,
    "platforms": ["windows", "linux"],
    "defines": ["TESTING=1"],
    "compile_options": ["-O0"]
}
```

---

## 7. See Also

- [Tests.cmake](Tests.md) — Ruft _collect_test auf
- [TestCreate.cmake](TestCreate.md) — Verwendet den Context
- [ExecutableCollect.cmake](ExecutableCollect.md) — Analoges Modul

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0** |
| 0.1.0 | 2025-12-11 | Initial (Clean Start): Test-Datensammlung |
