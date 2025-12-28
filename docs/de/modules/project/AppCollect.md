# AppCollect.cmake — Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung  
> **Zielgruppe:** Build-System-Entwickler  
> **Modul:** [cmake/project/AppCollect.cmake](../../../../cmake/project/AppCollect.cmake)  
> **Modul-Version:** 1.0.0  
> **Basiert auf:** ModuleDoc v0.5  
> **Sprache:** Deutsch  
> **English:** [AppCollect.md](../../../en/modules/project/AppCollect.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Context-Keys](#5-context-keys)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Best Practices](#8-best-practices)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Das `AppCollect`-Modul ist verantwortlich für das **Parsen von App-Container-Definitionen** aus der Solution.json und das Speichern der extrahierten Daten in einem Context-Objekt.

### Zweck

- Extraktion aller App-Container-Felder aus JSON
- Transformation in typisierte Context-Keys
- Anwendung von Default-Werten
- Validierung von Pflichtfeldern

### Features

- Vollständiges Parsing der `apps[]` JSON-Struktur
- Separate Behandlung von Core-, Runner- und Test-Konfiguration
- Intelligente Defaults (Convention over Configuration)
- Debug-Output für Diagnose

---

## 2. Abhängigkeiten

| Abhängigkeit | Typ | Beschreibung |
|--------------|-----|--------------|
| CMake 3.19+ | System | JSON-Funktionen, `include_guard(GLOBAL)` |
| `Json.cmake` | Modul | JSON-Parsing-Funktionen |
| `Context.cmake` | Modul | Context-Object-Pattern |
| `Debug.cmake` | Modul | Debug-Ausgabe |
| `Errors.cmake` | Modul | Fehlerbehandlung |

---

## 3. Konzept

### 3.1 App-Container-Architektur

Ein App-Container trennt Business-Logik (Core) vom Entry-Point (Runner):

```
┌─────────────────────────────────────────┐
│           App-Container                  │
│  ┌─────────────────────────────────┐    │
│  │      AppName.Core (STATIC)      │    │
│  │  ┌─────────┐ ┌─────────┐        │    │
│  │  │ Module A│ │ Module B│        │    │
│  │  └─────────┘ └─────────┘        │    │
│  └─────────────────────────────────┘    │
│                  │                       │
│         ┌───────┼───────┐               │
│         ▼       ▼       ▼               │
│  ┌──────────┐ ┌─────┐ ┌─────┐           │
│  │  Runner  │ │Unit │ │Int. │           │
│  │  (main)  │ │Tests│ │Tests│           │
│  └──────────┘ └─────┘ └─────┘           │
└─────────────────────────────────────────┘
```

### 3.2 JSON → Context Transformation

AppCollect transformiert die hierarchische JSON-Struktur in flache Context-Keys:

```
JSON:                          Context:
{                              
  "name": "AudioPlayer",   →   NAME = "AudioPlayer"
  "core": {                    
    "externals": ["bass"]  →   CORE_EXTERNALS = "bass"
  },                           
  "runner": {                  
    "type": "WINDOW"       →   RUNNER_TYPE = "WINDOW"
  }                            
}                              
```

### 3.3 Default-Werte

Das Modul wendet folgende Defaults an:

| Feld | Default | Begründung |
|------|---------|------------|
| `path` | `projects/apps/{name}` | Convention over Configuration |
| `runner.type` | `CONSOLE` | Häufigster Fall |
| `pch.header` | `pch.h` | Standard-Name, Pfad via Suchpriorität |
| `tests.framework` | `doctest` | Schnell, Header-only |
| `tests.unit.timeout` | `30` | Sekunden für Unit Tests |
| `tests.integration.timeout` | `120` | Sekunden für Integration |

---

## 4. API-Referenz

### 4.1 _collect_app()

```cmake
_collect_app(APP_JSON CTX)
```

**Beschreibung:**  
Parst eine App-Container-Definition aus JSON und speichert alle Felder im angegebenen Context.

**Parameter:**

| Parameter | Pflicht | Beschreibung |
|-----------|---------|--------------|
| `APP_JSON` | ✓ | JSON-String der App-Definition |
| `CTX` | ✓ | Context-Prefix (z.B. `APP_0`, `APP_1`) |

**Rückgabe:**  
Keine direkte Rückgabe. Alle Werte werden als Context-Keys gespeichert.

**Beispiel:**

```cmake
ctx_create(APP_0)
_collect_app("${_app_json}" APP_0)

ctx_get(APP_0 NAME _name)
ctx_get(APP_0 CORE_EXTERNALS _core_ext)
ctx_get(APP_0 RUNNER_TYPE _type)
```

**Fehler:**
- `E401` wenn `name` Feld fehlt

---

## 5. Context-Keys

### 5.1 Basis-Keys

| Key | Typ | JSON-Pfad | Default |
|-----|-----|-----------|---------|
| `NAME` | String | `name` | ⛔ Pflicht |
| `DISPLAY_NAME` | String | `displayName` | = NAME |
| `DESCRIPTION` | String | `description` | `""` |
| `VERSION` | String | `version` | Solution-Version |
| `PATH` | String | `path` | `projects/apps/{name}` |
| `SKIP` | Bool | `skip` | `FALSE` |
| `PLATFORMS` | List | `platforms[]` | `[]` (alle) |

### 5.2 Core-Keys

| Key | Typ | JSON-Pfad | Default |
|-----|-----|-----------|---------|
| `CORE_DEPENDENCIES` | List | `core.dependencies[]` | `[]` |
| `CORE_EXTERNALS` | List | `core.externals[]` | `[]` |
| `CORE_EXTERNAL_OPTIONS` | JSON | `core.external_options` | `{}` |

### 5.3 Runner-Keys

| Key | Typ | JSON-Pfad | Default |
|-----|-----|-----------|---------|
| `RUNNER_TYPE` | String | `runner.type` | `CONSOLE` |
| `RUNNER_EXTERNALS` | List | `runner.externals[]` | `[]` |
| `RUNNER_EXTERNAL_OPTIONS` | JSON | `runner.external_options` | `{}` |

### 5.4 PCH-Keys

| Key | Typ | JSON-Pfad | Default |
|-----|-----|-----------|---------|
| `PCH_ENABLED` | Bool | `pch.enabled` | `FALSE` |
| `PCH_HEADER` | String | `pch.header` | `pch.h` |
| `PCH_PATH` | String | `pch.path` | `""` (leer) |

**Hinweis:** PCH wird implizit aktiviert wenn `pch.header` oder `pch.path` angegeben ist und `pch.enabled` nicht explizit `false` ist.

### 5.5 Test-Keys

| Key | Typ | JSON-Pfad | Default |
|-----|-----|-----------|---------|
| `TESTS_FRAMEWORK` | String | `tests.framework` | `doctest` |
| `TESTS_UNIT_ENABLED` | Bool | (wenn `tests.unit` existiert) | `FALSE` |
| `TESTS_UNIT_TIMEOUT` | Number | `tests.unit.timeout` | `30` |
| `TESTS_UNIT_LABELS` | List | `tests.unit.labels[]` | `[]` |
| `TESTS_INTEGRATION_ENABLED` | Bool | (wenn `tests.integration` existiert) | `FALSE` |
| `TESTS_INTEGRATION_TIMEOUT` | Number | `tests.integration.timeout` | `120` |
| `TESTS_INTEGRATION_LABELS` | List | `tests.integration.labels[]` | `[]` |
| `TESTS_INTEGRATION_EXTERNALS` | List | `tests.integration.externals[]` | `[]` |

---

## 6. Verwendungsbeispiele

### 6.1 Minimale App-Definition

**JSON:**
```json
{
    "apps": [
        {
            "name": "SimpleApp"
        }
    ]
}
```

**Resultierende Context-Keys:**
```cmake
NAME = "SimpleApp"
DISPLAY_NAME = "SimpleApp"
PATH = "projects/apps/SimpleApp"
RUNNER_TYPE = "CONSOLE"
TESTS_FRAMEWORK = "doctest"
# ... (alle anderen mit Defaults)
```

### 6.2 Vollständige App-Definition

**JSON:**
```json
{
    "apps": [
        {
            "name": "AudioPlayer",
            "displayName": "Audio Player Application",
            "version": "2.0.0",
            
            "core": {
                "dependencies": ["BasicLogger"],
                "externals": ["bass", "spdlog"]
            },
            
            "runner": {
                "type": "WINDOW",
                "externals": ["imgui_docking", "glad", "glfw"]
            },
            
            "pch": {
                "enabled": true
            },
            
            "tests": {
                "framework": "doctest",
                "unit": {
                    "timeout": 30,
                    "labels": ["unit", "audio"]
                },
                "integration": {
                    "timeout": 120,
                    "labels": ["integration", "audio"],
                    "externals": ["bass"]
                }
            },
            
            "platforms": ["windows", "linux", "macos"]
        }
    ]
}
```

### 6.3 Verwendung in Apps.cmake

```cmake
# Iteration über apps Array
foreach(_idx RANGE 0 ${_last_idx})
    _json_array_get("${_solution_json}" "apps" ${_idx} _app_json)
    
    # Context erstellen und befüllen
    ctx_create(APP_${_idx})
    _collect_app("${_app_json}" APP_${_idx})
    
    # Werte nutzen
    ctx_get(APP_${_idx} NAME _name)
    ctx_get(APP_${_idx} SKIP _skip)
    
    if(_skip)
        continue()
    endif()
    
    # Weiter mit AppCreate...
endforeach()
```

---

## 7. Fehlerbehandlung

### 7.1 Ausgelöste Fehler

| Code | Bedingung | Meldung |
|------|-----------|---------|
| `E401` | `name` Feld fehlt | `App definition: 'name' is required` |

### 7.2 Fehler-Kontext

AppCollect löst nur Parsing-Fehler aus. Validierungsfehler (Pfad existiert nicht, etc.) werden von `AppCreate` behandelt.

---

## 8. Best Practices

### 8.1 Do's

| Empfehlung | Begründung |
|------------|------------|
| Context nach Collect sofort prüfen | Frühe Fehlererkennung |
| Debug-Level ULTRA_RARE für Details nutzen | Vollständige Diagnose |
| Defaults dokumentieren | Transparenz für Anwender |

### 8.2 Don'ts

| Vermeiden | Grund |
|-----------|-------|
| Validierung in Collect | Separation of Concerns |
| Direkte JSON-Zugriffe nach Collect | Context ist Single Source of Truth |
| Hardcoded Defaults ändern | Brechen bestehende Apps |

---

## 9. Siehe auch

- [Apps.cmake](Apps.md) — Orchestrator für App-Pipeline
- [AppCreate.cmake](AppCreate.md) — Target-Erstellung
- [Context.cmake](../core/Context.md) — Context-Object-Pattern
- [Json.cmake](../core/Json.md) — JSON-Parsing
- [AppContainer_Concept.md](../../projects/buildsystem/concepts/AppContainer_Concept.md) — Konzept-Dokument
- [ErrorCodes.md](../../references/ErrorCodes.md) — Fehlercode-Referenz (E4xx)

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.7.0** | **2025-12-20** | **CORE_EXTERNAL_OPTIONS und RUNNER_EXTERNAL_OPTIONS hinzugefügt** |
| 0.5.1 | 2025-12-18 | PCH-Defaults korrigiert: header auf pch.h, PCH_SOURCE entfernt, PCH_PATH hinzugefügt, implizite Aktivierung dokumentiert |
| 0.5.0 | 2025-12-17 | Initial: Phase 8 App-Container JSON-Parsing, Core/Runner/Tests-Trennung, vollständige Context-Keys |
