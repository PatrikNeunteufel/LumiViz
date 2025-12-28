# Registry/Targets.cmake — External Target Registry

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Targets_cmake.md](../../en/modules/externals/Targets_cmake.md)  
> **Module:** [cmake/externals/Registry/Targets.cmake](../../../cmake/externals/Registry/Targets.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Auto-Detection](#4-auto-detection)
5. [Target Linking](#5-target-linking)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Errorbehandlung](#7-fehlerbehandlung)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

`Registry/Targets.cmake` verwaltet die Target-Registrierung für Fetched Externals.

### Kernfunktionen

- **Registriert Targets** — Ordnet Targets einem External zu
- **Auto-Detection** — Erkennt Standard-Target-Namen automatisch
- **Primary Target** — Bestimmt das Haupt-Target für Linking
- **Linking** — Verbindet Externals mit Consumer-Targets

### Design-Prinzip

**Keine hardcodierten Mappings** — Target-Zuordnungen werden dynamisch ermittelt.

---

## 2. Dependencies

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Errorbehandlung |
| `Debug.cmake` | Debug-Ausgaben |

---

## 3. API-Reference

### 3.1 _register_external_target()

```cmake
_register_external_target(EXT_NAME TARGET_NAME [PRIMARY])
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals |
| `TARGET_NAME` | String | CMake Target-Name |
| `PRIMARY` | Flag | Markiert als Primary Target |

---

### 3.2 _auto_register_external_targets()

```cmake
_auto_register_external_targets(EXT_NAME)
```

Erkennt und registriert Targets automatisch.

---

### 3.3 _get_external_primary_target()

```cmake
_get_external_primary_target(EXT_NAME OUT_VAR)
```

Gibt das Primary Target zurück.

---

### 3.4 _link_external_to_target()

```cmake
_link_external_to_target(CONSUMER_TARGET EXT_NAME [SCOPE <scope>])
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `CONSUMER_TARGET` | String | Target das External nutzt |
| `EXT_NAME` | String | Name des Externals |
| `SCOPE` | Keyword | PRIVATE/PUBLIC/INTERFACE (default: PRIVATE) |

---

## 4. Auto-Detection

### Erkannte Patterns

| Pattern | Example |
|---------|----------|
| `${EXT_NAME}` | `spdlog` |
| `${EXT_NAME}::${EXT_NAME}` | `spdlog::spdlog` |
| `${ext_lower}` | `spdlog` |
| `${EXT_UPPER}` | `SPDLOG` |

### Hook-Hints für spezielle Namen

```cmake
# cmake/externals/hooks/prefetch/catch2.cmake
set_property(GLOBAL PROPERTY HOOK_KNOWN_TARGETS_catch2 
    "Catch2::Catch2WithMain" "Catch2::Catch2")
set_property(GLOBAL PROPERTY HOOK_PRIMARY_TARGET_catch2 
    "Catch2::Catch2WithMain")
```

---

## 5. Target Linking

### Keyword-Signatur

```cmake
target_link_libraries(${CONSUMER_TARGET} ${SCOPE} ${_primary})
```

**Important:** Immer Keyword-Signatur verwenden!

---

## 6. Usagesbeispiele

### PostFetch: Target registrieren

```cmake
add_library(${HOOK_EXTERNAL_NAME} STATIC ...)
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

### Manuelles Linking

```cmake
_link_external_to_target(MyLibrary spdlog SCOPE PUBLIC)
```

---

## 7. Errorbehandlung

| Code | Error | Description |
|------|--------|--------------|
| E201 | Keine Targets | External hat keine registrierten Targets |
| E203 | Target nicht gefunden | Primary Target existiert nicht |

---

## 8. See Also

- [Handler_cmake.md](Handler_cmake.md) — Ruft Auto-Register auf
- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Ruft Linking auf

---

## 9. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
