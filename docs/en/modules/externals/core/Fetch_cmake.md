# Core/Fetch.cmake — FetchContent Wrapper mit Caching

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Fetch_cmake.md](../../en/modules/externals/Fetch_cmake.md)  
> **Module:** [cmake/externals/Core/Fetch.cmake](../../../cmake/externals/Core/Fetch.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Configuration](#3-konfiguration)
4. [API-Reference](#4-api-referenz)
5. [Caching-Logik](#5-caching-logik)
6. [Git-Referenceen](#6-git-referenzen)
7. [Usagesbeispiele](#7-verwendungsbeispiele)
8. [Errorbehandlung](#8-fehlerbehandlung)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

`Core/Fetch.cmake` ist ein Wrapper um CMakes `FetchContent`, der intelligentes Caching in einem zentralen `.externals/` Verzeichnis implementiert.

### Kernfunktionen

- **Preset-übergreifendes Caching** — Alle Build-Presets teilen dieselben Downloads
- **Offline-Modus** — Arbeiten ohne Netzwerkzugriff
- **Force-Fetch** — Erzwingen eines erneuten Downloads
- **Version-Checking** — Automatische Erkennung veralteter Caches

### Architecture

```
.externals/                     ← Zentrales Cache-Verzeichnis
├── spdlog/
├── glfw/
└── imgui/

build-windows-debug/            ← Build-Verzeichnis (Preset)
build-linux-release/            ← Anderes Preset, GLEICHER Cache
```

---

## 2. Dependencies

| Modul | Zweck |
|-------|-------|
| `FetchContent` | CMake Built-in für Downloads |
| `Errors.cmake` | Errorbehandlung |
| `Debug.cmake` | Debug-Ausgaben |
| `Json.cmake` | JSON-Parsing |

---

## 3. Configuration

### Cache-Variablen

| Variable | Default | Description |
|----------|---------|--------------|
| `EXTERNALS_FETCH_ROOT` | `${CMAKE_SOURCE_DIR}/.externals` | Cache-Verzeichnis |

### Optionen

| Option | Default | Description |
|--------|---------|--------------|
| `EXTERNALS_OFFLINE` | `OFF` | Nur Cache verwenden, kein Netzwerk |
| `EXTERNALS_FORCE_FETCH` | `OFF` | Alle Externals neu herunterladen |

---

## 4. API-Reference

### 4.1 _fetch_git_external()

```cmake
_fetch_git_external(EXT_NAME EXT_JSON)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals |
| `EXT_JSON` | JSON | JSON-Definition mit `git`, `tag/branch/commit` |

**JSON-Felder:**

| Feld | Required | Description |
|------|---------|--------------|
| `git` | ✓ | Repository-URL |
| `tag` | ¹ | Git-Tag (z.B. "v1.12.0") |
| `branch` | ¹ | Git-Branch (z.B. "main") |
| `commit` | ¹ | Git-Commit-Hash |
| `shallow` | ✗ | Shallow Clone (default: true) |

¹ Genau eines von `tag`, `branch`, `commit` erforderlich.

---

### 4.2 _make_external_available()

```cmake
_make_external_available(EXT_NAME)
```

Macht ein deklariertes External verfügbar (Download falls nötig).

---

### 4.3 _is_external_populated()

```cmake
_is_external_populated(EXT_NAME OUT_VAR)
```

Prüft ob ein External verfügbar ist.

---

### 4.4 _get_external_source_dir()

```cmake
_get_external_source_dir(EXT_NAME OUT_VAR)
```

Gibt den Source-Pfad eines Externals zurück.

---

## 5. Caching-Logik

### Entscheidungsbaum

```
FORCE_FETCH=ON?  ──YES──► FETCH
       │
       NO
       ▼
Cache exists?  ──NO──► OFFLINE? ──YES──► E218 ERROR
       │                   │
      YES                  NO
       │                   ▼
       ▼               FETCH
Version match? ──YES──► USE CACHE
       │
       NO
       ▼
OFFLINE? ──YES──► W302 + USE CACHE
    │
    NO
    ▼
  FETCH
```

---

## 6. Git-Referenceen

### Tag (empfohlen)

```json
{
    "spdlog": {
        "git": "https://github.com/gabime/spdlog.git",
        "tag": "v1.12.0"
    }
}
```

### Commit

```json
{
    "imgui": {
        "git": "https://github.com/ocornut/imgui.git",
        "commit": "a1234567890abcdef"
    }
}
```

### Branch (nicht empfohlen)

```json
{
    "experimental": {
        "git": "https://github.com/example/lib.git",
        "branch": "develop"
    }
}
```

---

## 7. Usagesbeispiele

### Standard-Usage (via Handler)

```cmake
_fetch_git_external("spdlog" "${_ext_json}")
_make_external_available("spdlog")
```

### CI/CD Offline-Build

```bash
cmake -B build-ci -DEXTERNALS_OFFLINE=ON
```

---

## 8. Errorbehandlung

| Code | Error | Description |
|------|--------|--------------|
| E012 | Keine git URL | `git` Feld fehlt oder leer |
| E202 | Fetch fehlgeschlagen | FetchContent konnte External nicht laden |
| E215 | Keine Version | Kein tag/branch/commit angegeben |
| E218 | Offline ohne Cache | External nicht gecacht, OFFLINE=ON |

---

## 9. See Also

- [Handler_cmake.md](Handler_cmake.md) — Fetched External Pipeline
- [HookLoader_cmake.md](HookLoader_cmake.md) — Pre/PostFetch Hooks
- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Type Dispatcher

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
