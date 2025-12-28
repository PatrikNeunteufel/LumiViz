# Handler.cmake — System External Handler

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Handler_cmake.md](../../../../en/modules/externals/system/Handler_cmake.md)  
> **Modul:** [cmake/externals/system/Handler.cmake](../../../../../cmake/externals/system/Handler.cmake)  
> **Modul-Version:** 1.0.0  
> **Phase:** 9 (System Externals)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Verarbeitungsablauf](#4-verarbeitungsablauf)
5. [Package-Hooks](#5-package-hooks)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Debug-Ausgaben](#8-debug-ausgaben)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

`Handler.cmake` ist der zentrale Handler für **System Externals** — große, extern installierte Pakete wie Qt6 oder Boost, die über `find_package()` gefunden werden.

### Kernfunktionen

- **Pfad-Auflösung** — Mehrstufige Suche nach Installationspfaden
- **find_package()** — CMake-native Paketsuche mit Komponenten-Support
- **Package-Hooks** — Paketspezifische Konfiguration (Qt6, Boost)
- **Registrierung** — Speichert Ergebnis für spätere Verwendung

### Architektur-Position

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

## 2. Abhängigkeiten

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

## 3. API-Referenz

### 3.1 _handle_system_external()

Hauptfunktion zur Verarbeitung eines System Externals.

```cmake
_handle_system_external(EXT_NAME EXT_JSON)
```

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals (z.B. "qt6") |
| `EXT_JSON` | JSON | JSON-Definition aus Solution.json |

**Pflichtfelder in JSON:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `system` | bool | Muss `true` sein |
| `package` | string | Name für find_package() |

**Optionale Felder:**

| Feld | Typ | Default | Beschreibung |
|------|-----|---------|--------------|
| `version` | string | — | Versionsanforderung |
| `components` | array | `[]` | Paket-Komponenten |
| `hints` | array | `[]` | Zusätzliche Suchpfade |
| `backup` | string | — | Fallback-Pfad (mit Warnung) |
| `required` | bool | `true` | Optional wenn `false` |

---

### 3.2 _apply_system_external_to_target()

Verknüpft ein System External mit einem CMake-Target.

```cmake
_apply_system_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
```

| Parameter | Typ | Beschreibung |
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
    │   ├── package (Pflicht) → E502 wenn fehlt
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
    │   └── backup (mit W501 Warnung)
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

Package-Hooks ermöglichen paketspezifische Konfiguration.

### Hook-Pfad

```
cmake/externals/system/packages/{Package}.cmake
```

### Verfügbare Hook-Funktionen

| Funktion | Aufrufzeitpunkt | Zweck |
|----------|-----------------|-------|
| `_get_{Package}_standard_paths()` | Vor find_package | Standard-Installationspfade |
| `_{Package}_post_find()` | Nach find_package | Post-Processing |
| `_{Package}_configure_target()` | Bei apply_external | Target-Konfiguration |

### Beispiel: Qt6-Hook

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

## 6. Verwendungsbeispiele

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

## 7. Fehlerbehandlung

### Fehler-Codes

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E502 | Package-Feld fehlt | `package` ist Pflichtfeld |
| E503 | find_package fehlgeschlagen | Paket nicht gefunden (bei required=true) |

### Warnungen

| Code | Warnung | Beschreibung |
|------|---------|--------------|
| W501 | Backup verwendet | Fallback-Pfad statt ENV/hints |

### E503 Fehlermeldung

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

## 9. Siehe auch

- [PathResolver_cmake.md](PathResolver_cmake.md) — Pfad-Auflösung
- [packages/Qt6_cmake.md](packages/Qt6_cmake.md) — Qt6-spezifische Hooks
- [packages/Boost_cmake.md](packages/Boost_cmake.md) — Boost-spezifische Hooks
- [Orchestrator_cmake.md](../Orchestrator_cmake.md) — Dispatcht hierher
- [Solution_Schema.md](../../../references/Solution_Schema.md) — JSON-Schema (§5.4)

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.6.0** | **2025-12-18** | **Initial: Phase 9 System Externals, find_package Integration, Package-Hooks** |
