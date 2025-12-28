# LibraryCollect.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [LibraryCollect.md](../../en/modules/project/LibraryCollect.md)  
> **Module:** [`cmake/project/LibraryCollect.cmake`](../../../../cmake/project/LibraryCollect.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Context-Keys](#4-context-keys)
5. [JSON-Mapping](#5-json-mapping)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `LibraryCollect.cmake` Modul **sammelt Daten** aus dem JSON einer Library und speichert sie in einem Context. Es ist für das Parsing und die Normalisierung der Library-Definitionen zuständig.

### Kernidee

Analog zu ExecutableCollect — Trennung von Concerns: Collect sammelt, Create verwendet.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| JSON-Parsing | Alle Felder extrahieren |
| Normalisierung | Defaults setzen, Typen vereinheitlichen |
| Context-Befüllung | Alle Werte als Context-Keys speichern |

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Json.cmake | 0.5.0 | Alle `_json_*` Functions |
| Context.cmake | 0.5.0 | `ctx_set` |
| Debug.cmake | 0.5.0 | `dbg` |

---

## 3. API-Reference

### 3.1 _collect_library()

Sammelt alle Felder einer Library aus JSON in einen Context.

```cmake
_collect_library(<LIB_JSON> <CTX>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `LIB_JSON` | String | ✓ | JSON-String der Library |
| `CTX` | String | ✓ | Context-Prefix (z.B. `LIB_0`) |

**Rückgabe:** Keine (befüllt Context)

---

## 4. Context-Keys

### 4.1 Requiredfelder

| Key | Typ | Description |
|-----|-----|--------------|
| `NAME` | String | Target-Name (Required in JSON) |

### 4.2 Optionale Felder

| Key | Typ | Default | Description |
|-----|-----|---------|--------------|
| `VERSION` | String | Solution-Version | Version |
| `PATH` | Path | `projects/libs/{name}/src` | Source-Pfad |
| `TYPE` | Enum | Aus settings | STATIC, SHARED, INTERFACE |
| `PUBLIC_HEADERS` | Path | "" | Öffentliche Header-Verzeichnis |
| `SKIP` | Bool | FALSE | Überspringen |
| `PCH_ENABLED` | Bool | FALSE | PCH aktivieren |
| `PCH_HEADER` | String | `pch.h` | PCH-Header-Name |
| `PCH_PATH` | String | "" | Custom PCH-Pfad (relativ zu projects/) |
| `DEPENDENCIES` | List | "" | Interne Dependencies |
| `EXTERNALS` | List | "" | Externe Dependencies |
| `EXTERNAL_OPTIONS` | JSON | "{}" | Per-External Optionen |
| `PLATFORM` | String | "" (= alle) | Ziel-Plattform |

**Note:** PCH wird implizit aktiviert wenn `pch.header` oder `pch.path` angegeben ist und `pch.enabled` nicht explizit `false` ist.

---

## 5. JSON-Mapping

### 5.1 Minimale JSON

```json
{
    "name": "CoreLib"
}
```

### 5.2 Vollständige JSON

```json
{
    "name": "CoreLib",
    "version": "1.0.0",
    "path": "src/libs/core",
    "type": "STATIC",
    "public_headers": "include/core",
    "skip": false,
    "pch": {
        "enabled": true,
        "header": "pch.h"
    },
    "dependencies": ["UtilLib"],
    "externals": ["bass"],
    "external_options": {
        "bass": { "BASS_FLAC": true }
    },
    "platform": "windows"
}
```

---

## 6. Errorbehandlung

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | `name`-Feld fehlt | Name hinzufügen |

---

## 7. See Also

- [Libraries.cmake](Libraries.md) — Ruft _collect_library auf
- [LibraryCreate.cmake](LibraryCreate.md) — Verwendet den befüllten Context
- [ExecutableCollect.cmake](ExecutableCollect.md) — Analoges Modul für Executables

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-20** | **EXTERNAL_OPTIONS hinzugefügt: Per-External Optionen analog zu ExecutableCollect** |
| 0.5.1 | 2025-12-18 | PCH-Support hinzugefügt: PCH_ENABLED, PCH_HEADER, PCH_PATH |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0 |
| 0.1.0 | 2025-12-07 | Initial (Clean Start): JSON zu Context Mapping |
