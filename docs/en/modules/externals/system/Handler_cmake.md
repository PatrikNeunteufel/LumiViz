# Handler.cmake — System External Handler

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Handler_cmake.md](../../../../en/modules/externals/system/Handler_cmake.md)  
> **Module:** [cmake/externals/system/Handler.cmake](../../../../../cmake/externals/system/Handler.cmake)  
> **Module Version:** 1.0.0  
> **Phase:** 9 (System Externals)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Verarbeitungsablauf](#4-verarbeitungsablauf)
5. [Package-Hooks](#5-package-hooks)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Errorbehandlung](#7-fehlerbehandlung)
8. [Debug-Ausgaben](#8-debug-ausgaben)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

`Handler.cmake` ist der zentrale Handler für **System Externals** — große, extern installierte Pakete wie Qt6 oder Boost, die über `find_package()` gefunden werden.

### Kernfunktionen

- **Pfad-Auflösung** — Mehrstufige Suche nach Installationspfaden
- **find_package()** — CMake-native Paketsuche mit Komponenten-Support
- **Package-Hooks** — Paketspezifische Configuration (Qt6, Boost)
- **Registrierung** — Speichert Ergebnis für spätere Usage

### Architecture-Position

```
Orchestrator.cmake
       │
       ├── system: true
       ▼
┌─────────────────────┐
│  system/Handler     │  ← Dieser Handler
└─────────┬───────────┘
          │
    ┌─────┴─────┐
    ▼           ▼
PathResolver  Package-Hooks
              (Qt6, Boost)
```

---

## 2. Dependencies

### Benötigte Module

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | `cmake_fatal`, `cmake_warn` |
| `Debug.cmake` | `dbg` |
| `Json.cmake` | JSON-Parsing |
| `PathResolver.cmake` | Pfad-Auflösung |

### Auto-geladene Module

| Modul | Bedingung |
|-------|-----------|
| `packages/Qt6.cmake` | Wenn `package="Qt6"` |
| `packages/Boost.cmake` | Wenn `package="Boost"` |

---

## 3. API-Reference

### 3.1 _handle_system_external()

Hauptfunktion zur Verarbeitung eines System Externals.

```cmake
_handle_system_external(EXT_NAME EXT_JSON)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals (z.B. "qt6") |
| `EXT_JSON` | JSON | JSON-Definition aus Solution.json |

**Requiredfelder in JSON:**

| Feld | Typ | Description |
|------|-----|--------------|
| `system` | bool | Muss `true` sein |
| `package` | string | Name für find_package() |

**Optionale Felder:**

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `version` | string | — | Versionsanforderung |
| `components` | array | `[]` | Paket-Komponenten |
| `hints` | array | `[]` | Zusätzliche Suchpfade |
| `backup` | string | — | Fallback-Pfad (mit Warning) |
| `required` | bool | `true` | Optional wenn `false` |

---

### 3.2 _apply_system_external_to_target()

Verknüpft ein System External mit einem CMake-Target.

```cmake
_apply_system_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `TARGET_NAME` | String | CMake-Target |
| `EXT_NAME` | String | Name des Externals |
| `EXT_OPTIONS` | JSON | Options (aktuell ungenutzt) |

**Verhalten:**
1. Holt registrierten Package-Namen und Komponenten
2. Linkt `Package::Component` Targets
3. Ruft `_Package_configure_target()` Hook auf (falls definiert)

---

## 4. Verarbeitungsablauf

```
_handle_system_external(EXT_NAME, EXT_JSON)
    │
    ├── 1. JSON parsen
    │   ├── package (Required) → E502 wenn fehlt
    │   ├── version, components, hints, backup
    │   └── required (default: true)
    │
    ├── 2. Package-Hook laden (falls vorhanden)
    │   └── packages/{package}.cmake
    │       └── _get_{package}_standard_paths() aufrufen
    │
    ├── 3. Pfad auflösen via PathResolver
    │   ├── ENV-Variablen
    │   ├── hints[]
    │   ├── Standard-Pfade
    │   └── backup (mit W501 Warning)
    │
    ├── 4. CMAKE_PREFIX_PATH erweitern
    │
    ├── 5. find_package() aufrufen
    │   ├── Mit COMPONENTS wenn angegeben
    │   └── Mit REQUIRED wenn required=true
    │
    ├── 6. Ergebnis prüfen
    │   ├── Gefunden → Post-Find Hook aufrufen
    │   └── Nicht gefunden → E503 (wenn required)
    │
    └── 7. Registrieren
        ├── EXTERNAL_{name}_TYPE = "SYSTEM"
        ├── EXTERNAL_{name}_PACKAGE
        ├── EXTERNAL_{name}_COMPONENTS
        └── EXTERNAL_{name}_REGISTERED = TRUE
```

---

## 5. Package-Hooks

Package-Hooks ermöglichen paketspezifische Configuration.

### Hook-Pfad

```
cmake/externals/system/packages/{Package}.cmake
```

### Verfügbare Hook-Functions

| Funktion | Aufrufzeitpunkt | Zweck |
|----------|-----------------|-------|
| `_get_{Package}_standard_paths()` | Vor find_package | Standard-Installationspfade |
| `_{Package}_post_find()` | Nach find_package | Post-Processing |
| `_{Package}_configure_target()` | Bei apply_external | Target-Configuration |

### Example: Qt6-Hook

```cmake
# packages/Qt6.cmake

function(_get_Qt6_standard_paths OUT_VAR)
    set(_paths "C:/Qt/6.8.0/msvc2022_64" ...)
    set(${OUT_VAR} "${_paths}" PARENT_SCOPE)
endfunction()

function(_Qt6_configure_target TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
endfunction()
```

---

## 6. Usagesbeispiele

### Solution.json

```json
{
    "externals": {
        "qt6": {
            "system": true,
            "package": "Qt6",
            "components": ["Core", "Widgets", "Gui"],
            "hints": ["${QT_ROOT}"],
            "backup": "C:/Qt/6.8.0/msvc2022_64"
        },
        "boost": {
            "system": true,
            "package": "Boost",
            "components": ["filesystem", "system"],
            "required": false
        }
    }
}
```

### Automatischer Ablauf

```cmake
# In Orchestrator.cmake - automatisch wenn system: true
_handle_system_external("qt6" "${_ext_json}")

# In ExecutableCreate.cmake - bei externals: ["qt6"]
_apply_system_external_to_target("MyApp" "qt6" "{}")
```

---

## 7. Errorbehandlung

### Error-Codes

| Code | Error | Description |
|------|--------|--------------|
| E502 | Package-Feld fehlt | `package` ist Requiredfeld |
| E503 | find_package fehlgeschlagen | Paket nicht gefunden (bei required=true) |

### Warningen

| Code | Warning | Description |
|------|---------|--------------|
| W501 | Backup verwendet | Fallback-Pfad statt ENV/hints |

### E503 Errormeldung

```
[E503] System external 'qt6': find_package(Qt6) failed.

  Possible solutions:
    1. Set Qt6_ROOT environment variable
    2. Add 'hints' in Solution.json
    3. Install Qt6 to a standard location

  Example Solution.json:
    "qt6": {
      "system": true,
      "package": "Qt6",
      "hints": ["C:/Path/To/Qt6"]
    }
```

---

## 8. Debug-Ausgaben

### Debug-ID: `EXTERNALS`

| Level | Ausgabe |
|-------|---------|
| `DBG_COMMON` | Processing system external |
| `DBG_RARE` | Package, Components, Hints |
| `DBG_RARE` | find_package() Aufruf |
| `DBG_ULTRA_RARE` | CMAKE_PREFIX_PATH |

### Aktivierung

```bash
cmake -DDEBUG_CATEGORIES="EXTERNALS" ..
```

---

## 9. See Also

- [PathResolver_cmake.md](PathResolver_cmake.md) — Pfad-Auflösung
- [packages/Qt6_cmake.md](packages/Qt6_cmake.md) — Qt6-spezifische Hooks
- [packages/Boost_cmake.md](packages/Boost_cmake.md) — Boost-spezifische Hooks
- [Orchestrator_cmake.md](../Orchestrator_cmake.md) — Dispatcht hierher
- [Solution_Schema.md](../../../references/Solution_Schema.md) — JSON-Schema (§5.4)

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-18** | **Initial: Phase 9 System Externals, find_package Integration, Package-Hooks** |
