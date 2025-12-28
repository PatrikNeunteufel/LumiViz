# Registry/Targets.cmake — External Target Registry

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Targets_cmake.md](../../en/modules/externals/Targets_cmake.md)  
> **Modul:** [cmake/externals/Registry/Targets.cmake](../../../cmake/externals/Registry/Targets.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Auto-Detection](#4-auto-detection)
5. [Target Linking](#5-target-linking)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

`Registry/Targets.cmake` verwaltet die Target-Registrierung für Fetched Externals.

### Kernfunktionen

- **Registriert Targets** — Ordnet Targets einem External zu
- **Auto-Detection** — Erkennt Standard-Target-Namen automatisch
- **Primary Target** — Bestimmt das Haupt-Target für Linking
- **Linking** — Verbindet Externals mit Consumer-Targets

### Design-Prinzip

**Keine hardcodierten Mappings** — Target-Zuordnungen werden dynamisch ermittelt.

---

## 2. Abhängigkeiten

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Fehlerbehandlung |
| `Debug.cmake` | Debug-Ausgaben |

---

## 3. API-Referenz

### 3.1 _register_external_target()

```cmake
_register_external_target(EXT_NAME TARGET_NAME [PRIMARY])
```

| Parameter | Typ | Beschreibung |
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

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `CONSUMER_TARGET` | String | Target das External nutzt |
| `EXT_NAME` | String | Name des Externals |
| `SCOPE` | Keyword | PRIVATE/PUBLIC/INTERFACE (default: PRIVATE) |

---

## 4. Auto-Detection

### Erkannte Patterns

| Pattern | Beispiel |
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

**Wichtig:** Immer Keyword-Signatur verwenden!

---

## 6. Verwendungsbeispiele

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

## 7. Fehlerbehandlung

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E201 | Keine Targets | External hat keine registrierten Targets |
| E203 | Target nicht gefunden | Primary Target existiert nicht |

---

## 8. Siehe auch

- [Handler_cmake.md](Handler_cmake.md) — Ruft Auto-Register auf
- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Ruft Linking auf

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
