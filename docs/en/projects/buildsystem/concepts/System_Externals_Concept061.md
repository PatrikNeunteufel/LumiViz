# System Externals — Concept

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Concept  
> **Status:** Entwurf  
> **Phase:** 9 (geplant)  
> **Target Audience:** Build System Developers, Architekten  
> **Language:** English  
> **German:** [System_Externals_Concept.md](../../en/projects/buildsystem/concepts/System_Externals_Concept.md)

---

## Table of Contents

1. [Problemstellung](#1-problemstellung)
2. [Lösung: System External Type](#2-lösung-system-external-type)
3. [Typ-Erkennung](#3-typ-erkennung)
4. [Pfad-Auflösung](#4-pfad-auflösung)
5. [Implementation](#5-implementierung)
6. [Examples](#6-beispiele)
7. [Error Codes](#7-error-codes)
8. [Migration](#8-migration)
9. [Offene Punkte](#9-offene-punkte)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Problemstellung

### 1.1 Aktuelle External-Typen

Das Build-System unterstützt zwei External-Typen:

| Typ | Erkennungsfeld | Speicherort | Example |
|-----|----------------|-------------|----------|
| **Local** | `path` | `externals/` im Repo | bass, glad, doctest |
| **Fetched** | `git` | `.externals/` (gecached) | glfw, imgui, spdlog |

### 1.2 Das Problem mit großen Libraries

Bibliotheken wie **Qt6**, **Boost**, **OpenCV**, **CUDA** passen nicht in dieses Schema:

| Problem | Description |
|---------|--------------|
| **Zu groß** | Qt6 > 5 GB — nicht praktikabel für `externals/` |
| **Bereits installiert** | Oft schon auf dem System vorhanden |
| **Verschiedene Pfade** | Jeder Entwickler hat Qt woanders installiert |
| **Externe Laufwerke** | Manchmal auf D:/, E:/, USB, NAS |

### 1.3 Aktueller Workaround

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}"
    }
}
```

**Nachteile:**

- ❌ `path` zeigt nicht auf echte Dateien (semantisch falsch)
- ❌ Ordner `externals/qt6/` muss existieren (nur für Include.cmake)
- ❌ Pfad-Suche muss manuell in Include.cmake implementiert werden

---

## 2. Lösung: System External Type

### 2.1 Neuer External-Typ

Ein **System External** ist eine Bibliothek, die:
- Bereits auf dem System installiert ist
- Über CMake's `find_package()` gefunden wird
- Nicht heruntergeladen oder ins Repo kopiert wird

### 2.2 JSON-Syntax

```json
"externals": {
    "qt6": {
        "system": true,
        "package": "Qt6",
        "components": ["Core", "Widgets", "Gui"]
    }
}
```

### 2.3 Alle Felder

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `system` | bool | ✅ | — | Muss `true` sein |
| `package` | string | ✅ | — | Name für `find_package()` |
| `version` | string | — | — | Version-Constraint (z.B. `">=6.5.0"`) |
| `components` | string[] | — | `[]` | Package-Komponenten |
| `hints` | string[] | — | `[]` | Zusätzliche Suchpfade |
| `backup` | string | — | — | Notfall-Pfad (mit Warning) |
| `required` | bool | — | `true` | Error wenn nicht gefunden |

**Important:** `system` und `package` sind **nur zusammen** Required. Wenn `system: true` gesetzt ist, muss auch `package` angegeben werden.

---

## 3. Typ-Erkennung

### 3.1 Die drei External-Typen

Das Build-System erkennt den Typ anhand des **ersten vorhandenen Erkennungsfelds**:

```
┌─────────────────────────────────────────────────────────────────┐
│                    External-Definition                          │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────────┐
              │   Hat "system": true ?        │
              └───────────────────────────────┘
                     │                │
                    Yes               No
                     │                │
                     ▼                ▼
            ┌─────────────┐  ┌───────────────────────────┐
            │   SYSTEM    │  │   Hat "git" Feld?         │
            │   External  │  └───────────────────────────┘
            └─────────────┘         │                │
                                   Yes               No
                                    │                │
                                    ▼                ▼
                           ┌─────────────┐  ┌───────────────────────────┐
                           │   FETCHED   │  │   Hat "path" Feld?        │
                           │   External  │  └───────────────────────────┘
                           └─────────────┘         │                │
                                                  Yes               No
                                                   │                │
                                                   ▼                ▼
                                          ┌─────────────┐  ┌─────────────┐
                                          │    LOCAL    │  │   ERROR     │
                                          │   External  │  │   E012      │
                                          └─────────────┘  └─────────────┘
```

### 3.2 Requiredfelder pro Typ

| Typ | Requiredfelder | Example |
|-----|---------------|----------|
| **Local** | `path` | `"bass": { "path": "externals/bass" }` |
| **Fetched** | `git` + Version* | `"glfw": { "git": "...", "tag": "3.4" }` |
| **System** | `system` + `package` | `"qt6": { "system": true, "package": "Qt6" }` |

*Version = `tag`, `branch` oder `commit`

### 3.3 Gegenseitiger Ausschluss

Ein External kann **nur einen Typ** haben:

```json
// ❌ FALSCH: Mehrere Typ-Felder
"bad": {
    "system": true,
    "path": "externals/bad"    // Konflikt!
}

// ✅ RICHTIG: Genau ein Typ
"qt6": {
    "system": true,
    "package": "Qt6"
}
```

---

## 4. Pfad-Auflösung

### 4.1 Das Problem: Wo ist Qt installiert?

Verschiedene Entwickler haben Qt an verschiedenen Orten:

| Entwickler | Qt-Installation |
|------------|-----------------|
| Alice | `C:/Qt/6.7.0/msvc2022_64` |
| Bob | `D:/Tools/Qt/6.7.0/msvc2022_64` |
| Charlie | `E:/Libs/Qt/6.7.0/msvc2022_64` |
| CI-Server | `/opt/Qt/6.7.0/gcc_64` |

CMake's `find_package(Qt6)` sucht nur an Standard-Orten. Wenn Qt woanders installiert ist, findet es nichts.

### 4.2 Lösung: Mehrstufige Suche

Das System sucht in dieser Reihenfolge:

```
┌─────────────────────────────────────────────────────────────────┐
│ STUFE 1: Environment-Variablen (vom Entwickler gesetzt)        │
│          $QT_ROOT, $QT_DIR, $QT_HOME                           │
│          → Jeder Entwickler setzt seine eigene Variable        │
├─────────────────────────────────────────────────────────────────┤
│ STUFE 2: CMAKE_PREFIX_PATH                                      │
│          → Falls in CMake-Presets oder Command-Line gesetzt    │
├─────────────────────────────────────────────────────────────────┤
│ STUFE 3: hints[] aus Solution.json                              │
│          → Projekt gibt bekannte Pfade vor                      │
│          → Hilfreich für Team mit ähnlichen Setups             │
├─────────────────────────────────────────────────────────────────┤
│ STUFE 4: Standard-Pfade (plattformspezifisch)                   │
│          → C:/Qt/..., /opt/Qt/..., ~/Qt/...                    │
│          → Typische Installationsorte                          │
├─────────────────────────────────────────────────────────────────┤
│ STUFE 5: backup Pfad (mit Warning)                              │
│          → Notfall-Lösung, z.B. externes Laufwerk              │
│          → Zeigt Warning: "Bitte richtig installieren"         │
├─────────────────────────────────────────────────────────────────┤
│ STUFE 6: Error                                                 │
│          → Klare Errormeldung mit Lösungsvorschlägen          │
└─────────────────────────────────────────────────────────────────┘
```

### 4.3 Wann brauche ich was?

| Situation | Empfohlene Lösung |
|-----------|-------------------|
| **Ich alleine** | Environment-Variable setzen (`QT_ROOT`) |
| **Team mit ähnlichen Setups** | `hints[]` in Solution.json |
| **CI/CD** | Environment-Variable im CI-System |
| **Notfall (externes Laufwerk)** | `backup` Pfad |

### 4.4 Example: Vollständige Configuration

```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "components": ["Core", "Widgets"],
    "hints": [
        "${QT_ROOT}",                        // 1. Environment-Variable
        "C:/Qt/6.7.0/msvc2022_64",          // 2. Windows Standard
        "D:/Tools/Qt/6.7.0/msvc2022_64"     // 3. Alternative
    ],
    "backup": "E:/Backup/Qt/6.7.0/msvc2022_64"  // 4. Notfall
}
```

**Was passiert:**

1. **Alice** hat `QT_ROOT=C:/Qt/6.7.0/msvc2022_64` gesetzt → Gefunden ✅
2. **Bob** hat kein QT_ROOT, aber Qt in `D:/Tools/...` → Gefunden via hint ✅
3. **Charlie** hat Qt nur auf E:/ → Gefunden via backup ⚠️ (mit Warning)
4. **Dave** hat kein Qt → Klarer Error mit Hilfetext ❌

### 4.5 Minimale Configuration

Für einfache Fälle reicht:

```json
"boost": {
    "system": true,
    "package": "Boost",
    "components": ["filesystem", "system"]
}
```

→ Sucht nur via Environment-Variable und Standard-Pfade

---

## 5. Implementation

### 5.1 Architecture-Overview

#### Vorher (Workaround mit Local External)

```
Solution.json                    cmake/externals/
     │                                │
     ▼                                ▼
"qt6": {                         includes/qt6/Include.cmake
    "path": "externals/qt6"  ───────────────────────────────┐
}                                                            │
     │                                                       │
     ▼                                                       ▼
Orchestrator.cmake              Macht ALLES:
     │                          - Path-Auflösung (ENV, hints, backup)
     ▼                          - find_package(Qt6)
local/Attach.cmake              - Standard-Pfade (C:/Qt/...)
     │                          - AUTOMOC/AUTOUIC/AUTORCC
     ▼                          - windeployqt/macdeployqt
❌ Prüft ob externals/qt6/      - Target-Erstellung
   existiert (sinnlos!)
```

**Probleme:**
- Leerer Ordner `externals/qt6/` muss existieren
- Jede Include.cmake implementiert Path-Auflösung neu
- Keine einheitliche Errorbehandlung
- Semantisch falsch (`path` zeigt auf nichts)

#### Nachher (System External)

```
Solution.json                    cmake/externals/
     │                                │
     ▼                                ▼
"qt6": {                         system/
    "system": true,              ├── Handler.cmake ◄────────────────┐
    "package": "Qt6",            │   (generisch)                    │
    "components": [...]          │   - JSON parsen                  │
}                                │   - PathResolver aufrufen        │
     │                           │   - find_package()               │
     ▼                           │   - Package-Hook laden           │
Orchestrator.cmake               │                                  │
     │                           ├── PathResolver.cmake             │
     ▼                           │   (generisch)                    │
system/Handler.cmake ────────────┤   - ENV-Variablen prüfen         │
                                 │   - hints[] durchsuchen          │
                                 │   - Standard-Pfade               │
                                 │   - backup mit Warning           │
                                 │                                  │
                                 └── packages/                      │
                                     └── Qt6.cmake ◄────────────────┘
                                         (Qt-spezifisch)
                                         - Standard-Pfade für Qt
                                         - AUTOMOC/AUTOUIC/AUTORCC
                                         - windeployqt/macdeployqt
```

**Vorteile:**
- Kein leerer Ordner nötig
- Path-Auflösung einmal implementiert, überall genutzt
- Einheitliche Error-Codes (E5xx, W5xx)
- Semantisch korrekt (`system: true`)
- Package-spezifische Logik optional und isoliert

### 5.2 Neue Dateien

```
cmake/externals/
├── system/
│   ├── Handler.cmake       # Generischer System External Handler
│   ├── PathResolver.cmake  # Generische Pfad-Auflösung
│   └── packages/           # Package-spezifische Hooks (optional)
│       ├── Qt6.cmake       # AUTOMOC, windeployqt, Qt-Pfade
│       ├── Boost.cmake     # Boost-spezifische Logik
│       └── OpenCV.cmake    # OpenCV-spezifische Logik
├── includes/               # BLEIBT für Local Externals
│   ├── bass/Include.cmake  # Unverändert
│   ├── glad/Include.cmake  # Unverändert
│   └── qt6/Include.cmake   # → DEPRECATED (Migration zu system)
└── Orchestrator.cmake      # Erweitert um system Type
```

### 5.3 Was wird generisch vs. package-spezifisch?

| Funktion | Generisch (Handler/PathResolver) | Package-Hook |
|----------|----------------------------------|--------------|
| JSON parsen | ✅ | — |
| Environment-Variablen prüfen | ✅ `${PACKAGE}_ROOT` | — |
| hints[] durchsuchen | ✅ | — |
| Standard-Pfade | ⚠️ Basis-Liste | ✅ Erweiterte Liste |
| backup mit Warning | ✅ | — |
| find_package() aufrufen | ✅ | — |
| Error-Handling (E5xx) | ✅ | — |
| AUTOMOC/AUTOUIC/AUTORCC | — | ✅ Qt6.cmake |
| windeployqt/macdeployqt | — | ✅ Qt6.cmake |
| Boost-Namespace-Handling | — | ✅ Boost.cmake |
| OpenCV CUDA Config | — | ✅ OpenCV.cmake |

### 5.4 Package-Hook Mechanismus

Package-Hooks sind **optional**. Sie werden automatisch geladen, wenn vorhanden:

```cmake
# In Handler.cmake
function(_handle_system_external EXT_NAME EXT_JSON)
    # ... generische Logik ...
    
    # Package-Hook laden (falls vorhanden)
    set(_package_hook "${CMAKE_SOURCE_DIR}/cmake/externals/system/packages/${_package}.cmake")
    if(EXISTS "${_package_hook}")
        dbg(${DBG_RARE} "[${EXT_NAME}] Loading package hook: ${_package}.cmake" ID EXTERNALS)
        include("${_package_hook}")
    endif()
endfunction()
```

#### Example: Qt6.cmake Package-Hook

```cmake
# cmake/externals/system/packages/Qt6.cmake
# Wird nach find_package(Qt6) aufgerufen

# Qt-spezifische Standard-Pfade (erweitert PathResolver)
function(_get_qt6_standard_paths OUT_VAR)
    set(_paths "")
    if(WIN32)
        list(APPEND _paths
            "C:/Qt/6.8.0/msvc2022_64"
            "C:/Qt/6.7.0/msvc2022_64"
            "D:/Qt/6.8.0/msvc2022_64"
        )
    elseif(APPLE)
        list(APPEND _paths
            "$ENV{HOME}/Qt/6.8.0/macos"
            "/opt/homebrew/opt/qt@6"
        )
    else()
        list(APPEND _paths
            "$ENV{HOME}/Qt/6.8.0/gcc_64"
            "/opt/Qt/6.8.0/gcc_64"
        )
    endif()
    set(${OUT_VAR} "${_paths}" PARENT_SCOPE)
endfunction()

# AUTOMOC/AUTOUIC/AUTORCC für Target aktivieren
function(_qt6_configure_target TARGET_NAME)
    if(TARGET ${TARGET_NAME})
        set_target_properties(${TARGET_NAME} PROPERTIES
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
        )
    endif()
endfunction()

# Deployment konfigurieren
function(_qt6_configure_deployment TARGET_NAME)
    if(WIN32)
        find_program(_WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
        if(_WINDEPLOYQT)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND "${_WINDEPLOYQT}" --no-translations "$<TARGET_FILE:${TARGET_NAME}>"
                COMMENT "[qt6] Running windeployqt..."
            )
        endif()
    endif()
endfunction()
```

### 5.5 Orchestrator.cmake Erweiterung

```cmake
# cmake/externals/Orchestrator.cmake (erweitert)

include_guard(GLOBAL)

include(cmake/externals/local/Attach.cmake)
include(cmake/externals/fetched/Handler.cmake)
include(cmake/externals/system/Handler.cmake)  # NEU

function(_orchestrate_external EXT_NAME EXT_JSON)
    
    # Validate: Exactly one source field
    validate_external_source("${EXT_NAME}" "${EXT_JSON}")
    
    # Detect Type (Reihenfolge: system → git → path)
    _json_get_bool_or_default("${EXT_JSON}" "system" FALSE _is_system)
    _json_has_key("${EXT_JSON}" "git" _is_fetched)
    _json_has_key("${EXT_JSON}" "path" _is_local)
    
    # Dispatch
    if(_is_system)
        dbg(${DBG_RARE} "  Type: SYSTEM" ID EXTERNALS)
        _handle_system_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(_is_fetched)
        dbg(${DBG_RARE} "  Type: FETCHED (git)" ID EXTERNALS)
        _handle_fetched_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(_is_local)
        dbg(${DBG_RARE} "  Type: LOCAL" ID EXTERNALS)
        _attach_local_external("${EXT_NAME}" "${EXT_JSON}")
        
    else()
        cmake_fatal("E012" 
            "External '${EXT_NAME}': Kein gültiger Typ.\n"
            "  Benötigt eines von:\n"
            "    - 'system: true' + 'package' (System External)\n"
            "    - 'git' + 'tag/branch/commit' (Fetched External)\n"
            "    - 'path' (Local External)"
        )
    endif()
    
endfunction()
```

### 5.6 system/Handler.cmake

```cmake
# cmake/externals/system/Handler.cmake
# =====================================
# System External Handler - finds system-installed packages

include_guard(GLOBAL)
include(cmake/externals/system/PathResolver.cmake)

function(_handle_system_external EXT_NAME EXT_JSON)
    dbg(${DBG_COMMON} "[${EXT_NAME}] Processing system external" ID EXTERNALS)
    
    # =========================================================================
    # Requiredfeld: package
    # =========================================================================
    
    _json_get_string("${EXT_JSON}" "package" _package)
    if("${_package}" STREQUAL "")
        cmake_fatal("E502" "System external '${EXT_NAME}': 'package' field is required")
    endif()
    
    # =========================================================================
    # Optionale Felder
    # =========================================================================
    
    _json_get_string("${EXT_JSON}" "version" _version)
    _json_get_array_as_list("${EXT_JSON}" "components" _components)
    _json_get_array_as_list("${EXT_JSON}" "hints" _hints)
    _json_get_string("${EXT_JSON}" "backup" _backup)
    _json_get_bool_or_default("${EXT_JSON}" "required" TRUE _required)
    
    # =========================================================================
    # Package-Hook für Standard-Pfade laden (falls vorhanden)
    # =========================================================================
    
    set(_package_hook "${CMAKE_SOURCE_DIR}/cmake/externals/system/packages/${_package}.cmake")
    set(_additional_paths "")
    
    if(EXISTS "${_package_hook}")
        include("${_package_hook}")
        # Hook kann _get_${_package}_standard_paths() definieren
        if(COMMAND _get_${_package}_standard_paths)
            cmake_language(CALL _get_${_package}_standard_paths _additional_paths)
        endif()
    endif()
    
    # =========================================================================
    # Pfad auflösen
    # =========================================================================
    
    _resolve_system_path(
        "${EXT_NAME}" 
        "${_package}" 
        "${_hints}" 
        "${_additional_paths}"
        "${_backup}" 
        _resolved_path 
        _used_backup
    )
    
    # Warning bei Backup-Usage
    if(_used_backup)
        cmake_warn("W501" 
            "[${EXT_NAME}] Using backup location: ${_resolved_path}\n"
            "  Consider setting ${_package}_ROOT environment variable."
        )
    endif()
    
    # =========================================================================
    # CMAKE_PREFIX_PATH erweitern
    # =========================================================================
    
    if(NOT "${_resolved_path}" STREQUAL "")
        list(PREPEND CMAKE_PREFIX_PATH "${_resolved_path}")
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
    endif()
    
    # =========================================================================
    # find_package aufrufen
    # =========================================================================
    
    if(_required)
        set(_req_flag REQUIRED)
    else()
        set(_req_flag "")
    endif()
    
    if(_components)
        find_package(${_package} ${_version} ${_req_flag} COMPONENTS ${_components})
    else()
        find_package(${_package} ${_version} ${_req_flag})
    endif()
    
    # Success prüfen
    if(${_package}_FOUND)
        dbg(${DBG_COMMON} "[${EXT_NAME}] Found ${_package} ${${_package}_VERSION}" ID EXTERNALS)
        
        # Package-Hook für Post-Setup aufrufen (AUTOMOC etc.)
        if(COMMAND _${_package}_post_find)
            cmake_language(CALL _${_package}_post_find)
        endif()
        
    elseif(_required)
        cmake_fatal("E503" "System external '${EXT_NAME}': find_package(${_package}) failed")
    else()
        dbg(${DBG_COMMON} "[${EXT_NAME}] ${_package} not found (optional)" ID EXTERNALS)
    endif()
    
    # =========================================================================
    # Als System External registrieren
    # =========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TYPE "SYSTEM")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PACKAGE "${_package}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_REGISTERED TRUE)
    
endfunction()
```

### 5.7 system/PathResolver.cmake

```cmake
# cmake/externals/system/PathResolver.cmake
# ==========================================
# Generic path resolution for system externals

include_guard(GLOBAL)

function(_resolve_system_path EXT_NAME PACKAGE HINTS ADDITIONAL_PATHS BACKUP OUT_PATH OUT_IS_BACKUP)
    set(_found FALSE)
    set(_result_path "")
    set(_is_backup FALSE)
    
    # =========================================================================
    # Stufe 1: Environment-Variablen
    # =========================================================================
    
    foreach(_var_suffix ROOT DIR HOME)
        set(_var_name "${PACKAGE}_${_var_suffix}")
        if(DEFINED ENV{${_var_name}})
            set(_candidate "$ENV{${_var_name}}")
            if(EXISTS "${_candidate}")
                set(_found TRUE)
                set(_result_path "${_candidate}")
                dbg(${DBG_RARE} "[${EXT_NAME}] Found via ${_var_name}: ${_candidate}" ID EXTERNALS)
                break()
            endif()
        endif()
    endforeach()
    
    # =========================================================================
    # Stufe 2: hints[] aus Solution.json
    # =========================================================================
    
    if(NOT _found AND HINTS)
        foreach(_hint IN LISTS HINTS)
            # Expand environment variables
            string(CONFIGURE "${_hint}" _hint_expanded @ONLY)
            if(EXISTS "${_hint_expanded}")
                set(_found TRUE)
                set(_result_path "${_hint_expanded}")
                dbg(${DBG_RARE} "[${EXT_NAME}] Found via hint: ${_hint_expanded}" ID EXTERNALS)
                break()
            endif()
        endforeach()
    endif()
    
    # =========================================================================
    # Stufe 3: Package-spezifische Standard-Pfade
    # =========================================================================
    
    if(NOT _found AND ADDITIONAL_PATHS)
        foreach(_path IN LISTS ADDITIONAL_PATHS)
            if(EXISTS "${_path}")
                set(_found TRUE)
                set(_result_path "${_path}")
                dbg(${DBG_RARE} "[${EXT_NAME}] Found at standard path: ${_path}" ID EXTERNALS)
                break()
            endif()
        endforeach()
    endif()
    
    # =========================================================================
    # Stufe 4: Backup (mit Warning)
    # =========================================================================
    
    if(NOT _found AND NOT "${BACKUP}" STREQUAL "")
        string(CONFIGURE "${BACKUP}" _backup_expanded @ONLY)
        if(EXISTS "${_backup_expanded}")
            set(_found TRUE)
            set(_result_path "${_backup_expanded}")
            set(_is_backup TRUE)
        endif()
    endif()
    
    # =========================================================================
    # Ergebnis
    # =========================================================================
    
    if(_found)
        set(${OUT_PATH} "${_result_path}" PARENT_SCOPE)
        set(${OUT_IS_BACKUP} ${_is_backup} PARENT_SCOPE)
    else()
        # Kein fataler Error hier - find_package() wird es später melden
        # Das erlaubt find_package() eigene Suchlogik zu nutzen
        set(${OUT_PATH} "" PARENT_SCOPE)
        set(${OUT_IS_BACKUP} FALSE PARENT_SCOPE)
        dbg(${DBG_RARE} "[${EXT_NAME}] No path found, relying on find_package()" ID EXTERNALS)
    endif()
    
endfunction()
```

### 5.8 Validation.cmake Erweiterung

Die bestehende Validation muss erweitert werden:

```cmake
# cmake/core/Validation.cmake

# Source-Felder Liste erweitern
function(validate_external_source EXT_NAME EXT_JSON)
    # ERWEITERT: "system" hinzugefügt
    set(_source_fields "path;git;system;vcpkg;conan;find_package")
    set(_found_count 0)
    set(_found_fields "")
    
    foreach(_field IN LISTS _source_fields)
        _json_has_key("${EXT_JSON}" "${_field}" _has)
        if(_has)
            math(EXPR _found_count "${_found_count} + 1")
            list(APPEND _found_fields "${_field}")
        endif()
    endforeach()
    
    if(_found_count EQUAL 0)
        cmake_fatal("E012" 
            "External '${EXT_NAME}': No source field specified.\n"
            "  Required: one of 'system', 'git', or 'path'"
        )
    elseif(_found_count GREATER 1)
        cmake_fatal("E012" 
            "External '${EXT_NAME}': Multiple source fields specified: ${_found_fields}\n"
            "  Only one source field is allowed."
        )
    endif()
    
    # System External: Zusätzliche Validierung
    _json_has_key("${EXT_JSON}" "system" _has_system)
    if(_has_system)
        _json_get_bool_or_default("${EXT_JSON}" "system" FALSE _is_system)
        if(_is_system)
            _json_has_key("${EXT_JSON}" "package" _has_package)
            if(NOT _has_package)
                cmake_fatal("E502" 
                    "System external '${EXT_NAME}': 'package' field is required.\n"
                    "  Example: { \"system\": true, \"package\": \"Qt6\" }"
                )
            endif()
        endif()
    endif()
endfunction()
```

---

## 6. Examples

### 6.1 Qt6 (vollständig)

```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "version": ">=6.5.0",
    "components": ["Core", "Widgets", "Gui", "OpenGL"],
    "hints": [
        "${QT_ROOT}",
        "C:/Qt/6.7.0/msvc2022_64"
    ],
    "backup": "E:/Backup/Qt/6.7.0/msvc2022_64"
}
```

### 6.2 Boost (minimal)

```json
"boost": {
    "system": true,
    "package": "Boost",
    "components": ["filesystem", "system", "thread"]
}
```

### 6.3 OpenCV

```json
"opencv": {
    "system": true,
    "package": "OpenCV",
    "version": ">=4.5.0",
    "hints": ["${OPENCV_DIR}"]
}
```

### 6.4 Optional (nicht required)

```json
"cuda": {
    "system": true,
    "package": "CUDAToolkit",
    "required": false
}
```

→ Kein Error wenn nicht gefunden, Code kann mit `if(CUDAToolkit_FOUND)` prüfen

---

## 7. Error Codes

### 7.1 System External Errors (E5xx)

| Code | Description |
|------|--------------|
| E501 | System external not found (no valid path) |
| E502 | System external: 'package' field is required |
| E503 | System external: find_package() failed |
| E504 | System external: required component not found |
| E505 | System external: version constraint not satisfied |

### 7.2 System External Warnings (W5xx)

| Code | Description |
|------|--------------|
| W501 | Using backup location for system external |
| W502 | System external version mismatch (using found version) |

---

## 8. Migration

### 8.1 Overview: Was passiert mit bestehenden Dateien?

| Datei | Status nach Phase 9 | Aktion |
|-------|---------------------|--------|
| `cmake/externals/includes/qt6/Include.cmake` | ⚠️ DEPRECATED | Migrieren zu `system` |
| `cmake/externals/includes/bass/Include.cmake` | ✅ Unverändert | Bleibt (echtes Local External) |
| `cmake/externals/includes/glad/Include.cmake` | ✅ Unverändert | Bleibt (echtes Local External) |
| `externals/qt6/` (leerer Ordner) | ❌ Löschen | Nicht mehr nötig |
| `externals/bass/` (mit Dateien) | ✅ Unverändert | Bleibt (echtes Local External) |

### 8.2 Entscheidungshilfe: Local vs. System External

```
                        Ist die Library im Repository?
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
                   Yes                             No
                    │                               │
                    ▼                               ▼
            ┌───────────────┐               Wird sie mit find_package() gefunden?
            │    LOCAL      │                       │
            │   External    │           ┌───────────┴───────────────┐
            │               │           │                           │
            │ "path": "..." │          Yes                         No
            └───────────────┘           │                           │
                                        ▼                           ▼
                                ┌───────────────┐           ┌───────────────┐
                                │    SYSTEM     │           │    FETCHED    │
                                │   External    │           │   External    │
                                │               │           │               │
                                │ "system":true │           │ "git": "..."  │
                                └───────────────┘           └───────────────┘
```

### 8.3 Migration: Qt6 (Workaround → System External)

#### Schritt 1: Solution.json ändern

**Vorher (Workaround):**
```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}",
        "components": ["Core", "Widgets"]
    }
}
```

**Nachher (System External):**
```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "components": ["Core", "Widgets"],
    "hints": ["${QT_ROOT}"]
}
```

#### Schritt 2: Leeren Ordner löschen

```bash
# Ordner war nur für Workaround nötig
rm -rf externals/qt6/
```

#### Schritt 3: Include.cmake (optional behalten)

Die alte `cmake/externals/includes/qt6/Include.cmake` wird **nicht mehr aufgerufen**.

Stattdessen wird (falls vorhanden) der Package-Hook geladen:
`cmake/externals/system/packages/Qt6.cmake`

**Option A:** Include.cmake löschen, Package-Hook nutzen (empfohlen)
**Option B:** Include.cmake zu Package-Hook umbenennen/verschieben

### 8.4 Vergleich: Vorher vs. Nachher

| Aspekt | Workaround (path) | System External |
|--------|-------------------|-----------------|
| **Semantik** | ❌ `path` zeigt auf leeren Ordner | ✅ `system: true` ist klar |
| **Ordner nötig** | ❌ `externals/qt6/` (leer) | ✅ No |
| **Path-Auflösung** | ❌ In jeder Include.cmake | ✅ Einmal in PathResolver.cmake |
| **Error-Handling** | ❌ Individuell | ✅ Einheitlich (E5xx) |
| **Backup-Support** | ❌ Manuell implementiert | ✅ Integriert mit W501 |
| **Version-Check** | ❌ Manuell implementiert | ✅ Integriert |
| **Package-Hook** | ❌ Alles in Include.cmake | ✅ Nur Qt-spezifisches |

### 8.5 Abwärtskompatibilität

Die **Local External** Funktionalität bleibt vollständig erhalten:

```json
// Funktioniert weiterhin für echte Local Externals
"bass": {
    "path": "externals/bass"
}
```

Nur der **Workaround** (path auf leeren/nicht genutzten Ordner) wird obsolet.

### 8.6 Deprecation-Warning (optional)

In Phase 9 könnte eine Warning hinzugefügt werden:

```cmake
# In local/Attach.cmake
function(_attach_local_external EXT_NAME EXT_JSON)
    # ... bestehender Code ...
    
    # Deprecation-Warning für bekannte System-Libraries
    set(_known_system_libs "qt6;boost;opencv;cuda")
    if("${EXT_NAME}" IN_LIST _known_system_libs)
        cmake_warn("W105" 
            "External '${EXT_NAME}' is defined as local but is typically a system library.\n"
            "  Consider migrating to: \"${EXT_NAME}\": { \"system\": true, \"package\": \"...\" }"
        )
    endif()
endfunction()
```

---

## 9. Offene Punkte

### 9.1 Entschieden

| Frage | Entscheidung |
|-------|--------------|
| Typ-Erkennung? | `system` → `git` → `path` → Error |
| Requiredfelder? | `system: true` erfordert `package` |
| Rückwärtskompatibel? | ✅ Yes, alte Externals funktionieren |

### 9.2 Nicht im Scope

- vcpkg/Conan Integration (separates Feature)
- Automatischer Download von System Externals
- Hybrid-Externals (System mit Fetched-Fallback) — evtl. später

---

## 10. See Also

- [master_concept.md](master_concept.md) — Architecture-Overview
- [implementation_plan.md](implementation_plan.md) — Phasen-Plan
- [Solution_Schema.md](../../../references/Solution_Schema.md) — JSON-Schema

---

## 11. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.1** | **2025-12-18** | **Umfassende Überarbeitung:** Section 5 komplett neu (Architecture Vorher/Nachher, Package-Hook Mechanismus, vollständiger Handler/PathResolver Code), Section 8 Migration erweitert (Include.cmake Deprecation, Entscheidungshilfe-Diagramm), Typ-Erkennung klargestellt (Section 3), Requiredfelder-Logik korrigiert |
| 0.6.0 | 2025-12-18 | Klarere Erklärung: Typ-Erkennung (Section 3), Pfad-Auflösung mit Examplesn (Section 4), Requiredfelder-Logik korrigiert (system+package zusammen), Diagramme hinzugefügt |
| 0.5.0 | 2025-12-14 | Blueprint v0.5.0 Format, Error Codes E5xx/W5xx |
| 0.1.0 | 2025-12-10 | Initial: Concept für System Externals |
