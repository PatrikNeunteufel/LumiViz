# Solution Schema — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Solution_Schema.md](../../en/references/Solution_Schema.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [solution Block](#3-solution-block)
4. [settings Block](#4-settings-block)
5. [externals Block](#5-externals-block)
6. [executables Array](#6-executables-array)
7. [libraries Array](#7-libraries-array)
8. [tests Array](#8-tests-array)
9. [apps Array](#9-apps-array)
10. [Schnellreferenz](#10-schnellreferenz)
11. [Vollständiges Example](#11-vollständiges-beispiel)
12. [Error-Codes](#12-fehler-codes)
13. [See Also](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Overview

Diese Reference beschreibt das vollständige Schema der Solution.json für das CMake Architecture Build-System. Die Solution.json ist das Herzstück der deklarativen Configuration.

### Root-Level Struktur

```json
{
    "schemaVersion": "0.1",
    "solution": { },
    "settings": { },
    "externalsPolicy": { },
    "externals": { },
    "libraries": [ ],
    "executables": [ ],
    "tests": [ ],
    "apps": [ ]           
}
```

### Block-Overview

| Block | Required | Description |
|-------|---------|--------------|
| `schemaVersion` | ✅ | Version des JSON-Schemas (nur MAJOR.MINOR) |
| `solution` | ✅ | Metadaten (Name, Version, Autoren) |
| `settings` | – | Globale Build-Einstellungen |
| `externalsPolicy` | – | External-Verhalten |
| `externals` | – | Zentrale External-Definitionen |
| `libraries` | – | Interne Libraries |
| `executables` | – | Ausführbare Programme |
| `tests` | – | Test-Targets |
| `apps` | – | App-Container (Core/Runner Separation) |

---

## 2. Konventionen

### Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ✅ | Requiredfeld |
| – | Optional / Standard |

### Pfad-Konventionen

Alle Pfade sind relativ zu `CMAKE_SOURCE_DIR` (Projekt-Root).

### Umgebungsvariablen

Syntax: `${VAR_NAME}` wird zu `$ENV{VAR_NAME}` expandiert.

```json
"hint": "${QT_ROOT}"        // → C:/Qt/6.10.1/msvc2022_64
"hint": "${HOME}/Qt/6.10.1" // → /home/user/Qt/6.10.1
```

---

## 3. solution Block

```json
"solution": {
    "name": "MySolution",
    "version": "1.0.0",
    "description": "Description",
    "authors": ["Name"]
}
```

| Feld | Required | Typ | Description |
|------|---------|-----|--------------|
| `name` | ✅ | string | Projektname (→ PROJECT_NAME) |
| `version` | ✅ | string | Semantische Version (MAJOR.MINOR.PATCH) |
| `description` | – | string | Description für IDEs/Dokumentation |
| `authors` | – | string[] | Autoren-Liste |

---

## 4. settings Block

```json
"settings": {
    "standards": {
        "cxx_standard": 20,
        "cxx_standard_required": true,
        "cxx_extensions": false
    },
    "defaults": {
        "library_type": "STATIC",
        "executable_type": "CONSOLE"
    },
    "sources": {
        "mode": "auto"
    }
}
```

### 4.1 standards

| Feld | Default | Description |
|------|---------|--------------|
| `cxx_standard` | 20 | C++ Standard (11, 14, 17, 20, 23) |
| `cxx_standard_required` | true | Standard erzwingen |
| `cxx_extensions` | false | Compiler-Erweiterungen |

### 4.2 defaults

| Feld | Default | Description |
|------|---------|--------------|
| `library_type` | `"STATIC"` | Standard Library-Typ |
| `executable_type` | `"CONSOLE"` | Standard Executable-Typ |

### 4.3 sources

| Feld | Default | Description |
|------|---------|--------------|
| `mode` | `"auto"` | Source-Collection (explicit, glob, auto) |

---

## 5. externals Block

Der zentrale Ort für alle External-Definitionen.

### 5.1 External-Typen

| Typ | Erkennungsmerkmal | Description |
|-----|-------------------|--------------|
| **Local** | `path` Feld | Vorkompilierte Bibliotheken in `externals/` |
| **Fetched** | `git` Feld | Via Git geklont |
| **System** | `system: true` | Große externe Installationen (Qt6, Boost) via find_package() |

> **Typ-Priorität:** `system` → `git` → `path`

### 5.2 Local Externals
```json
"externals": {
    "bass": {
        "path": "externals/bass"
    }
}
```

| Feld | Required | Description |
|------|---------|--------------|
| `path` | ✅ | Pfad relativ zu CMAKE_SOURCE_DIR |
| `include` | — | Pfad zur Include.cmake (optional) |

#### Include.cmake Convention

**Default-Pfad (Convention over Configuration):**

```markdown
cmake/externals/includes/{name}/Include.cmake
```
Wobei `{name}` der External-Schlüssel ist (z.B. `bass`, `lua54`).

**Examples:**

| External | Convention-Pfad |
|----------|-----------------|
| `"bass": {...}` | `cmake/externals/includes/bass/Include.cmake` |
| `"lua54": {...}` | `cmake/externals/includes/lua54/Include.cmake` |
| `"doctest": {...}` | `cmake/externals/includes/doctest/Include.cmake` |

**Custom Include (optional):**

Nur verwenden wenn vom Default abgewichen werden muss:
```json
"externals": {
    "mylib": {
        "path": "externals/mylib",
        "include": "cmake/custom/mylib_special.cmake"
    }
}
```

> **Empfehlung:** Immer die Convention verwenden. Das `include` Feld nur für Sonderfälle.

### 5.3 Fetched Externals

```json
"externals": {
    "spdlog": {
        "git": "https://github.com/gabime/spdlog.git",
        "tag": "v1.12.0",
        "shallow": true,
        "cmakeSupport": true
    }
}
```

| Feld | Required | Description |
|------|---------|--------------|
| `git` | ✅ | Repository-URL |
| `tag` | – | Git Tag oder Branch |
| `commit` | – | Spezifischer Commit-Hash |
| `shallow` | – | Shallow Clone (Default: true) |
| `cmakeSupport` | – | Hat CMakeLists.txt (Default: true) |
| `preFetchHook` | – | Pfad zu PreFetch Hook |
| `postFetchHook` | – | Pfad zu PostFetch Hook |
| `hook` | – | Hook-Wiederverwendung |

### 5.4 System Externals (Phase 9)

Für große, extern installierte Bibliotheken wie Qt6, Boost, OpenCV.

**Keine lokale Kopie nötig** – nutzt `find_package()` direkt.

```json
"externals": {
    "qt6": {
        "system": true,
        "package": "Qt6",
        "version": ">=6.5.0",
        "components": ["Core", "Widgets", "Gui", "OpenGL"],
        "hints": ["${QT_ROOT}", "C:/Qt/6.10.1/msvc2022_64"],
        "backup": "E:/Backup/Qt/6.10.1/msvc2022_64",
        "required": true
    }
}
```

| Feld | Required | Typ | Description |
|------|---------|-----|--------------|
| `system` | ✅ | boolean | Muss `true` sein |
| `package` | ✅ | string | Name für find_package() (z.B. "Qt6", "Boost") |
| `version` | – | string | Versionsanforderung (z.B. ">=6.5.0") |
| `components` | – | string[] | find_package() COMPONENTS |
| `hints` | – | string[] | Zusätzliche Suchpfade |
| `backup` | – | string | Fallback-Pfad (löst W501 aus) |
| `required` | – | boolean | find_package() REQUIRED (Default: true) |

#### Pfad-Auflösung (Reihenfolge)

1. **Umgebungsvariablen:** `Qt6_ROOT`, `QT_ROOT`, `Qt6_DIR`, etc.
2. **hints[]** aus Solution.json (mit `${VAR}` Expansion)
3. **Standard-Pfade** aus Package-Hook (plattformspezifisch)
4. **backup** Pfad (mit W501 Warning)
5. **find_package() Default** (CMake-eigene Suche)

#### Package-Hooks

Optionale plattformspezifische Configuration in:
```
cmake/externals/system/packages/{Package}.cmake
```

Vordefinierte Hooks:
- `Qt6.cmake` – Standard-Installationspfade, AUTOMOC/AUTOUIC/AUTORCC, windeployqt
- `Boost.cmake` – Standard-Pfade, MSVC auto-linking Deaktivierung

#### Migration von Local zu System

**Vorher (Workaround mit leerem Ordner):**
```json
"qt6": {
    "path": "externals/qt6",
    "options": { "hint": "${QT_ROOT}" }
}
```

**Nachher (kein lokaler Ordner nötig):**
```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "hints": ["${QT_ROOT}"]
}
```

### 5.5 cmakeSupport Flag

| Wert | Bedeutung | Hook-Anforderung |
|------|-----------|------------------|
| `true` (default) | External hat CMakeLists.txt | PostFetch optional |
| `false` | Kein CMakeLists.txt | PostFetch **PFLICHT** |

### 5.6 hook Feld (Hook-Wiederverwendung)

```json
"externals": {
    "imgui": {
        "git": "https://github.com/ocornut/imgui.git",
        "tag": "v1.91.6",
        "cmakeSupport": false
    },
    "imgui_docking": {
        "git": "https://github.com/ocornut/imgui.git",
        "tag": "v1.91.6-docking",
        "cmakeSupport": false,
        "hook": "imgui"
    }
}
```

| External | Hook-Datei | `HOOK_EXTERNAL_NAME` | Target |
|----------|------------|----------------------|--------|
| `imgui` | `imgui.cmake` | `"imgui"` | `imgui` |
| `imgui_docking` | `imgui.cmake` | `"imgui_docking"` | `imgui_docking` |

---

### 5.7 Gemeinsame Felder (alle Typen)

Diese Felder funktionieren für Local, Fetched und System Externals:

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `skip` | bool | `false` | External überspringen (nicht laden) |

#### skip (External deaktivieren)

Mit `skip: true` kann ein External vorbereitet werden, ohne es zu laden:

```json
"externals": {
    "bass": {
        "path": "externals/bass"
    },
    "future_feature": {
        "git": "https://github.com/example/future.git",
        "tag": "v1.0.0",
        "skip": true
    },
    "optional_qt": {
        "system": true,
        "package": "Qt6",
        "components": ["Core", "Widgets"],
        "skip": true
    }
}
```

**Verhalten:**
- External wird nicht geladen (kein FetchContent, kein find_package, kein Include)
- Wird trotzdem im externals-Block validiert
- **FATAL ERROR (E013)** wenn ein geskipptes External von einem Target verwendet wird

**Anwendungsfälle:**
- Externals für spätere Features vorbereiten
- Optionale Dependencies temporär deaktivieren
- Platform-spezifische Externals (manuell oder via Preset)

---

## 6. executables Array

```json
"executables": [
    {
        "name": "MyApp",
        "displayName": "My Application",
        "version": "1.0.0",
        "type": "GUI",
        "path": "projects/apps/MyApp/src",
        "skip": false,
        "dependencies": ["CoreLib"],
        "externals": ["qt6", "spdlog"],
        "external_options": {
            "bass": { "BASS_FLAC": true }
        }
    }
]
```

### 6.1 Requiredfelder

| Feld | Description |
|------|--------------|
| `name` | Eindeutiger Target-Name |

### 6.2 Optionale Felder

| Feld | Default | Description |
|------|---------|--------------|
| `displayName` | `name` | Anzeigename |
| `version` | Solution-Version | Executable-Version |
| `type` | `CONSOLE` | `CONSOLE`, `GUI`, `WORKER` |
| `path` | `projects/exec/{name}/src` | Source-Verzeichnis |
| `skip` | `false` | Build überspringen |
| `pch` | – | Precompiled Headers Config (siehe [§ 6.5](#65-pch-object-precompiled-headers)) |
| `dependencies` | `[]` | Interne Libraries |
| `externals` | `[]` | External-Referenceen |
| `external_options` | `{}` | Per-External Options |
| `platforms` | `[]` (alle) | Plattform-Filter |
| `defines` | `[]` | Preprocessor-Definitionen |
| `compile_options` | `[]` | Compiler-Flags |
| `link_options` | `[]` | Linker-Flags |

### 6.3 type Werte

| Typ | Windows | macOS | Linux |
|-----|---------|-------|-------|
| `CONSOLE` | Normal | Normal | Normal |
| `GUI` | WIN32 | MACOSX_BUNDLE | Normal |
| `WORKER` | Normal | Normal | Normal |

### 6.4 external_options

Per-Target Options für Externals:

```json
"external_options": {
    "bass": {
        "BASS_FLAC": true,
        "BASS_FX": true
    }
}
```

### 6.5 pch Object (Precompiled Headers)

```json
"pch": {
    "enabled": true,
    "header": "stdafx.h",
    "path": "common/pch"
}
```

| Feld | Default | Description |
|------|---------|--------------|
| `enabled` | `false` | PCH aktivieren |
| `header` | `pch.h` | Name der PCH-Datei |
| `path` | – | Custom-Pfad (relativ zu `projects/`) |

**Aktivierung (implizit):**

PCH wird automatisch aktiviert wenn:
- `enabled: true` explizit gesetzt ist, ODER
- `header` angegeben ist und `enabled` nicht `false`, ODER
- `path` angegeben ist und `enabled` nicht `false`

**Suchpfad-Priorität** (wenn `path` nicht angegeben):

| Priorität | Pfad |
|-----------|------|
| 1 | `{target-path}/pch/{header}` |
| 2 | `{target-path}/src/{header}` |
| 3 | `{target-path}/{header}` |

Wenn `path` angegeben: `projects/{path}/{header}`

---

## 7. libraries Array

```json
"libraries": [
    {
        "name": "CoreLib",
        "version": "1.0.0",
        "type": "STATIC",
        "path": "projects/libs/CoreLib/src",
        "public_headers": "projects/libs/CoreLib/include",
        "dependencies": ["UtilsLib"],
        "externals": ["bass"],
        "external_options": {
            "bass": {
                "BASS_FLAC": true
            }
        }
    }
]
```

| Feld | Default | Description |
|------|---------|--------------|
| `name` | ✅ Required | Target-Name |
| `version` | Solution-Version | Library-Version |
| `type` | Settings-Default | `STATIC`, `SHARED`, `INTERFACE` |
| `path` | Convention | Source-Verzeichnis |
| `public_headers` | – | Public Include-Verzeichnis |
| `dependencies` | `[]` | Andere Libraries (Dependencies) |
| `externals` | `[]` | Externe Dependencies |
| `external_options` | `{}` | Per-External Options |
| `pch` | – | Precompiled Headers Config (siehe [§ 6.5](#65-pch-object-precompiled-headers)) |

---

## 8. tests Array

### 8.1 Grundstruktur

```json
"tests": [
    {
        "name": "CoreLib_UnitTests",
        "displayName": "CoreLib Unit Tests",
        "type": "unit",
        "framework": "doctest",
        "dependencies": ["CoreLib"],
        "externals": ["doctest"],
        "timeout": 30,
        "labels": ["unit", "core", "fast"],
        "parallel": true
    }
]
```

### 8.2 Requiredfelder

| Feld | Typ | Description |
|------|-----|--------------|
| `name` | string | Eindeutiger Test-Target-Name |

### 8.3 Optionale Felder

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `displayName` | string | `name` | Anzeigename |
| `version` | string | Solution-Version | Test-Version |
| `type` | string | `"unit"` | Test-Typ |
| `framework` | string | `"doctest"` | Test-Framework |
| `path` | string | Convention | Source-Verzeichnis |
| `target` | string | – | Zu testendes Target (für Coverage) |
| `dependencies` | string[] | `[]` | Interne Libraries |
| `externals` | string[] | `[]` | Externe Libraries |
| `external_options` | object | `{}` | Per-External Options |
| `timeout` | int | 60 | Timeout in Sekunden |
| `labels` | string[] | `[type]` | CTest Labels |
| `parallel` | bool | true | Parallel ausführbar |
| `skip` | bool | false | Test überspringen |
| `source_from` | string | – | Sources von Executable übernehmen |
| `exclude_sources` | string[] | `[]` | Auszuschließende Sources |

### 8.4 Test-Typen

| Typ | Description | Empfohlene Labels |
|-----|--------------|-------------------|
| `unit` | Einzelne Functions/Klassen testen | `fast`, `isolated` |
| `integration` | Komponenten-Zusammenspiel | `slow`, `database` |
| `system` | Gesamtsystem (End-to-End) | `e2e`, `slow` |
| `performance` | Benchmarks, Performance-Tests | `benchmark`, `slow` |
| `smoke` | Schnelle Basis-Tests | `fast`, `critical` |

### 8.5 Unterstützte Frameworks

| Framework | External-Name | Description |
|-----------|---------------|--------------|
| `doctest` | `doctest` | Schnell, Header-only, ideal für Unit Tests |
| `googletest` | `googletest` | Feature-reich, Mocking (GMock) |
| `catch2` | `catch2` | BDD-Style, Sections, Benchmarks |

### 8.6 source_from (Executable-Sources testen)

Für Tests die Module aus einem Executable testen (ohne dessen main):

```json
"tests": [
    {
        "name": "MyApp_ModuleY_Tests",
        "type": "unit",
        "framework": "doctest",
        "source_from": "MyApp",
        "exclude_sources": ["main.cpp"],
        "externals": ["doctest"]
    }
]
```

### 8.7 CTest-Integration

```bash
# Alle Tests ausführen
ctest --test-dir build

# Nach Label filtern
ctest -L unit        # Nur Unit Tests
ctest -L fast        # Nur schnelle Tests
ctest -LE slow       # Keine langsamen Tests

# Parallel ausführen
ctest -j8            # 8 parallele Jobs
```

---
## 9. apps Array

Das `apps` Array definiert App-Container mit Core/Runner Separation für maximale Testbarkeit.

### 9.1 Grundstruktur

```json
"apps": [
    {
        "name": "MyVisualizer",
        "displayName": "My Visualizer",
        "version": "1.0.0",
        "description": "Audio visualization application",
        "skip": false,
        
        "core": {
            "dependencies": ["CoreLib"],
            "externals": ["bass"],
            "external_options": {
                "bass": { "BASS_FLAC": true }
            }
        },
        
        "runner": {
            "type": "GUI",
            "externals": ["glad", "glfw"],
            "external_options": {
                "glad": { "GLAD_DEBUG": true }
            }
        },
        
        "pch": {
            "enabled": true,
            "header": "pch.h"
        },
        
        "tests": {
            "skip": false,
            "framework": "doctest",
            "targets": [
                {
                    "name": "UnitTests",
                    "type": "unit",
                    "skip": false,
                    "path": "tests/unit/UnitTests",
                    "timeout": 30,
                    "labels": ["unit", "fast"],
                    "parallel": true
                }
            ]
        },
        
        "platforms": ["windows", "linux", "macos"]
    }
]
```

### 9.2 Requiredfelder

| Feld | Typ | Description |
|------|-----|--------------|
| `name` | string | Eindeutiger App-Name (Präfix für alle Targets) |

### 9.3 Optionale Felder (Root)

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `displayName` | string | `name` | Anzeigename |
| `version` | string | Solution-Version | App-Version |
| `description` | string | `""` | Description |
| `skip` | bool | `false` | Gesamte App überspringen |
| `path` | string | `projects/apps/{name}` | Pfad zum App-Verzeichnis |
| `core` | object | `{}` | Core-Library Configuration |
| `runner` | object | `{}` | Runner-Executable Configuration |
| `pch` | object | `{}` | Precompiled Header Configuration |
| `tests` | object | `{}` | Test-Configuration |
| `platforms` | string[] | `[]` | Plattform-Filter |

### 9.4 core Object

Konfiguriert die Core-Library (`{AppName}.Core`).

```json
"core": {
    "dependencies": ["CoreLib", "UtilsLib"],
    "externals": ["bass", "lua54"],
    "external_options": {
        "bass": {
            "BASS_FLAC": true
        }
    }
}
```

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `dependencies` | string[] | `[]` | Interne Libraries (aus `libraries[]`) |
| `externals` | string[] | `[]` | Externe Dependencies |
| `external_options` | object | `{}` | Per-External Options |

### 9.5 runner Object

Konfiguriert das Runner-Executable (`{AppName}`).

```json
"runner": {
    "type": "GUI",
    "externals": ["glad", "glfw"],
    "external_options": {
        "glad": {
            "GLAD_DEBUG": true
        }
    }
}
```

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `type` | string | `"CONSOLE"` | Executable-Typ (`GUI` / `CONSOLE`) |
| `externals` | string[] | `[]` | Runner-spezifische Externals |
| `external_options` | object | `{}` | Per-External Options |

**runner.type:**

| Typ | Windows | Linux/macOS |
|-----|---------|-------------|
| `GUI` | `WinMain` (kein Konsolenfenster) | `main` |
| `CONSOLE` | `main` (mit Konsole) | `main` |

### 9.6 pch Object

Konfiguriert Precompiled Header.

```json
"pch": {
    "enabled": true,
    "header": "pch.h"
}
```

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `enabled` | bool | `false` | PCH aktivieren |
| `header` | string | `"pch.h"` | PCH-Header Dateiname |

**PCH-Scope:**
- Core (`src/`): Verwendet PCH wenn aktiviert
- Runner (`main/`): Verwendet PCH NICHT
- Tests: Verwenden PCH NICHT

### 9.7 tests Object

Konfiguriert App-Tests mit flexiblem targets[] Array.

```json
"tests": {
    "skip": false,
    "framework": "doctest",
    "targets": [ ... ]
}
```

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `skip` | bool | `false` | **Alle Tests dieser App überspringen** |
| `framework` | string | — | Default-Framework für alle Tests |
| `targets` | array | `[]` | Test-Target Definitionen |

### 9.8 tests.targets[] Object

Jedes Element definiert ein Test-Target.

```json
{
    "name": "UnitTests",
    "type": "unit",
    "skip": false,
    "path": "tests/unit/UnitTests",
    "framework": "doctest",
    "timeout": 30,
    "labels": ["unit", "fast"],
    "externals": ["bass"],
    "parallel": true
}
```

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `name` | string | ✅ | — | Target-Name (→ `{AppName}.{name}`) |
| `type` | string | ✅ | — | Test-Typ |
| `skip` | bool | — | `false` | **Diesen Test überspringen** |
| `path` | string | — | `tests/{type}/{name}` | Pfad relativ zum App-Verzeichnis |
| `framework` | string | — | `tests.framework` | Test-Framework |
| `timeout` | number | — | Typ-abhängig | CTest Timeout (Sekunden) |
| `labels` | string[] | — | Typ-abhängig | CTest Labels |
| `externals` | string[] | — | `[]` | Zusätzliche Externals |
| `parallel` | bool | — | Typ-abhängig | Parallele Ausführung |

### 9.9 Test-Typen und Defaults

| Typ | Default Timeout | Default Parallel | Default Labels |
|-----|-----------------|------------------|----------------|
| `unit` | 30s | true | `["unit", "fast"]` |
| `integration` | 120s | true | `["integration"]` |
| `performance` | 300s | false | `["performance", "benchmark"]` |
| `system` | 180s | false | `["system", "e2e", "slow"]` |
| `smoke` | 10s | true | `["smoke", "critical", "fast"]` |
| `fuzz` | 60s | false | `["fuzz", "security"]` |
| `security` | 120s | false | `["security"]` |
| `ui` | 180s | false | `["ui", "slow"]` |
| `api` | 60s | true | `["api", "integration"]` |
| *(unbekannt)* | 60s | true | `["{type}"]` |

### 9.10 Skip-Logik

| `tests.skip` | `targets[].skip` | Ergebnis |
|--------------|------------------|----------|
| `true` | egal | Test übersprungen |
| `false`/fehlt | `true` | Test übersprungen |
| `false`/fehlt | `false`/fehlt | Test wird erstellt |

**Globales Skip hat Vorrang** — wenn `tests.skip: true`, werden alle Tests übersprungen.

### 9.11 Generierte Targets

Für jede App werden folgende CMake Targets erstellt:

| Target | Typ | Description |
|--------|-----|--------------|
| `{AppName}.Core` | STATIC Library | Business-Logik (include/ + src/) |
| `{AppName}` | Executable | Entry Point (main/) |
| `{AppName}.{TestName}` | Executable | Test (für jedes Target in tests.targets[]) |

**Example:**

```
MyVisualizer.Core              ← STATIC Library
MyVisualizer                   ← GUI Executable
MyVisualizer.UnitTests         ← Test Executable
MyVisualizer.IntegrationTests  ← Test Executable
```

### 9.12 Ordnerstruktur

```
projects/apps/{AppName}/
├── include/                    # Öffentliche Header → {AppName}.Core
│   ├── Source.cmake
│   └── *.hpp
├── src/                        # Implementation → {AppName}.Core
│   ├── Source.cmake
│   └── *.cpp
├── main/                       # Entry Point → {AppName}
│   ├── Source.cmake
│   └── main.cpp
├── pch/                        # Precompiled Header
│   └── pch.h
└── tests/
    └── {type}/
        └── {TestName}/         # → {AppName}.{TestName}
            ├── Source.cmake
            ├── test_main.cpp
            └── test_*.cpp
```

### 9.13 Vollständiges Example

```json
{
    "apps": [
        {
            "name": "MyVisualizer",
            "displayName": "My Visualizer",
            "version": "0.1.0",
            
            "core": {
                "externals": ["bass"]
            },
            
            "runner": {
                "type": "GUI",
                "externals": ["glad", "glfw"]
            },
            
            "pch": {
                "enabled": true
            },
            
            "tests": {
                "framework": "doctest",
                "targets": [
                    {
                        "name": "UnitTests",
                        "type": "unit",
                        "path": "tests/unit/UnitTests"
                    },
                    {
                        "name": "IntegrationTests",
                        "type": "integration",
                        "path": "tests/integration/IntegrationTests",
                        "externals": ["bass"]
                    },
                    {
                        "name": "Benchmarks",
                        "type": "performance",
                        "skip": true
                    }
                ]
            }
        },
        {
            "name": "DemoPlayer",
            "tests": {
                "skip": true,
                "framework": "doctest",
                "targets": [
                    { "name": "UnitTests", "type": "unit" }
                ]
            }
        }
    ]
}
```

---

## 10. Schnellreferenz

### 10.1 Requiredfelder

| Block | Feld |
|-------|------|
| Root | `schemaVersion` |
| solution | `name`, `version` |
| executables[] | `name` |
| libraries[] | `name` |
| tests[] | `name` |
| apps[] | `name` |
| apps[].tests.targets[] | `name`, `type` |
| externals (local) | `path` (include optional) |
| externals (fetched) | `git`, (tag\|branch\|commit) |
| externals (system) | `system: true`, `package` |


### 10.2 Defaults

| Einstellung | Default-Wert |
|-------------|--------------|
| C++ Standard | 20 |
| Library-Typ | STATIC |
| Executable-Typ | CONSOLE |
| Source-Mode | auto |
| Test-Framework | doctest |
| Test-Timeout | 60s |
| App runner.type | CONSOLE |
| App PCH | disabled |
| App tests.skip | false |

---

## 11. Vollständiges Example

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "MyProject",
        "version": "1.0.0",
        "description": "Example project with Qt6 and Tests"
    },
    "settings": {
        "standards": { "cxx_standard": 20 },
        "defaults": {
            "library_type": "STATIC",
            "executable_type": "CONSOLE"
        },
        "sources": { "mode": "auto" }
    },
    "externals": {
        "bass": { "path": "externals/bass" },
        "doctest": { "path": "externals/doctest" },
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        },
        "imgui": {
            "git": "https://github.com/ocornut/imgui.git",
            "tag": "v1.91.6",
            "cmakeSupport": false
        },
        "qt6": {
            "system": true,
            "package": "Qt6",
            "components": ["Core", "Widgets", "Gui"],
            "hints": ["${QT_ROOT}"]
        }
    },
    "libraries": [
        {
            "name": "CoreLib",
            "version": "1.0.0",
            "type": "STATIC",
            "path": "projects/libs/CoreLib/src",
            "public_headers": "projects/libs/CoreLib/include"
        }
    ],
    "executables": [
        {
            "name": "ConsoleApp",
            "type": "CONSOLE",
            "dependencies": ["CoreLib"],
            "externals": ["bass"],
            "external_options": {
                "bass": { "BASS_FLAC": true }
            }
        },
        {
            "name": "QtApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ],
    "tests": [
        {
            "name": "CoreLib_UnitTests",
            "type": "unit",
            "framework": "doctest",
            "dependencies": ["CoreLib"],
            "externals": ["doctest"],
            "labels": ["unit", "core", "fast"],
            "timeout": 30
        }
    ]
}
```
> **Note:** Für lokale externals ist **kein** `include` Feld nötig — die Convention `cmake/externals/includes/{name}/Include.cmake` wird automatisch verwendet.

---


## 12. Error-Codes

### 12.1 External-bezogene Error

| Code | Description |
|------|--------------|
| E010 | External nicht in externals Block definiert |
| E012 | External hat weder path, git noch system Feld |
| E213 | Include.cmake für lokales External nicht gefunden |
| E214 | Local external path existiert nicht |
| E216 | PostFetch Hook fehlt (cmakeSupport: false) |
| E218 | Hook-Datei nicht gefunden |
| E220 | Target nach Hook nicht registriert |
| **E502** | **System external: `package` Feld fehlt** |
| **E503** | **System external: find_package() fehlgeschlagen** |

### 12.2 Test-bezogene Error (standalone tests[])

| Code | Description |
|------|--------------|
| E301 | Unbekanntes Test-Framework |
| E302 | source_from Executable existiert nicht |
| E303 | Test-Source-Verzeichnis nicht gefunden |

### 12.3 App-Container Error (apps[])

| Code | Description |
|------|--------------|
| E401 | App: `name` ist Requiredfeld |
| E402 | App-Pfad existiert nicht |
| E403 | App hat kein `src/` Verzeichnis |
| E404 | App hat keine Source-Dateien in `src/` |
| E405 | App-Dependency nicht gefunden |
| E406 | App hat kein `main/` Verzeichnis |
| E407 | App hat keine Source-Dateien in `main/` |

### 12.4 App-Tests Error (apps[].tests.targets[])

| Code | Description |
|------|--------------|
| E301 | Kein Framework angegeben (weder global noch per-target) |
| E302 | Unbekanntes Test-Framework |
| E303 | Test-Target: `name` fehlt |
| E304 | Test-Target: `type` fehlt |
| E305 | Test-Pfad existiert nicht |
| E306 | Keine Source-Dateien im Test-Pfad |

### 12.5 Warningen

| Code | Bereich | Description |
|------|---------|--------------|
| W302 | Externals | Hook-Wiederverwendung aktiv |
| W401 | App-Container | App hat kein `include/` Verzeichnis |
| W402 | App-Container | PCH aktiviert aber Header nicht gefunden |
| W402 | App-Tests | Test mit seriellem Typ hat `parallel: true` gesetzt |
| W403 | App-Container | Tests-Verzeichnis existiert aber keine Sources |
| **W501** | **System Externals** | **Backup-Pfad verwendet (nicht empfohlen)** |

**Note zu W402:** Der Code W402 wird in zwei Kontexten verwendet:
- Bei PCH: Warning wenn `pch.enabled: true` aber `pch/{header}` nicht gefunden
- Bei Tests: Warning wenn `parallel: true` für Typen wie `performance`, `system`, `fuzz`, `security`, `ui` gesetzt wird

---

## 13. See Also

- [Externals.md](Externals.md) — External Libraries Reference
- [ErrorCodes.md](ErrorCodes.md) — Vollständige Errorcode-Reference
- [CMakePresets Reference](CMakePresets.md) — Build-Presets
- [App Tests Targets Concept](../projects/buildsystem/concepts/App_Tests_Targets_Concept.md) — Concept für flexible App-Tests

---

## 14. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.7.2** | **2025-12-20** | **Neu: `skip` für externals (§5.7) - Externals vorbereiten ohne zu laden. E013 bei Usage geskippter Externals** |
| 0.7.1 | 2025-12-20 | Konsistente external_options: Für libraries[], apps.core, apps.runner hinzugefügt (fehlte bisher). Nun einheitlich wie executables[] und tests[] |
| 0.7.0 | 2025-12-18 | Phase 9: System Externals mit `system: true` Syntax, find_package() Integration, Package-Hooks (Qt6, Boost), Error-Codes E502/E503/W501, Pfad-Auflösung mit ENV-Variablen |
| 0.6.0 | 2025-12-18 | Neu: apps[] Array mit Core/Runner Separation, tests.targets[] flexible Test-Configuration, Skip-Feature (tests.skip, targets[].skip), App-Container Error-Codes (E4xx, W4xx) |
| 0.5.2 | 2025-12-18 | PCH-Objekt vollständig dokumentiert (§ 6.5): implizite Aktivierung, Suchpfad-Priorität, pch für libraries hinzugefügt |
| 0.5.1 | 2025-12-15 | Include.cmake Convention dokumentiert (§ 5.2), include Feld als optional, Hook-Pfade kleingeschrieben |
| 0.5.0 | 2025-12-14 | Blueprint v0.5.0 Format: Nummeriertes TOC, Reference-Header, Schnellreferenz, Änderungsblöcke ins Changelog integriert |
| 0.1.4 | 2025-12-12 | tests Array dokumentiert: Typen, Frameworks, source_from, CTest |
| 0.1.3 | 2025-12-11 | System Externals: options Feld (hint, backup, components) |
| 0.1.2 | 2025-12-10 | Hook-Wiederverwendung (hook Feld) |
| 0.1.1 | 2025-12-09 | Git Externals (git, tag, cmakeSupport) |
| 0.1.0 | 2025-12-05 | Initial Schema |
