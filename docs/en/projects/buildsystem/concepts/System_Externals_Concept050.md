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
3. [Pfad-Auflösung](#3-pfad-auflösung)
4. [Implementation](#4-implementierung)
5. [Examples](#5-beispiele)
6. [Error Codes](#6-error-codes)
7. [Migration](#7-migration)
8. [Offene Punkte](#8-offene-punkte)
9. [Roadmap](#9-roadmap)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Problemstellung

### 1.1 Aktuelle Situation

Das Build-System unterstützt zwei External-Typen:

| Typ | Erkennungsmerkmal | Speicherort |
|-----|-------------------|-------------|
| **Local** | `path` Feld | `externals/` im Projekt |
| **Fetched** | `git` Feld | `.externals/` (gecached) |

### 1.2 Problem

Große Bibliotheken wie **Qt6**, **Boost**, **OpenCV**, **CUDA** sind:

- Zu groß für `externals/` (Qt6 > 5 GB)
- Oft bereits installiert (System, Installer)
- Auf verschiedenen Pfaden je nach Installationsart
- Manchmal auf externen Laufwerken (Backup, USB)

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

| Problem | Description |
|---------|--------------|
| Semantisch unklar | `path` zeigt nicht auf echte Dateien |
| Ordner nötig | `externals/qt6/` muss existieren (nur für Include.cmake) |
| Keine Standard-Suche | Pfad-Suche muss manuell implementiert werden |

---

## 2. Lösung: System External Type

### 2.1 Neues `system` Feld

```json
"externals": {
    "qt6": {
        "system": true,
        "package": "Qt6",
        "version": ">=6.5.0",
        "components": ["Core", "Widgets", "Gui"],
        "hints": [
            "${QT_ROOT}",
            "C:/Qt/6.7.0/msvc2022_64"
        ],
        "backup": "E:/Backup/Libs/Qt/6.7.0/msvc2022_64"
    }
}
```

### 2.2 Feld-Definitionen

| Feld | Typ | Required | Description |
|------|-----|---------|--------------|
| `system` | `bool` | ✅ | Kennzeichnet System External |
| `package` | `string` | ✅ | find_package Name |
| `version` | `string` | ❌ | Version Constraint |
| `components` | `string[]` | ❌ | Package-Komponenten |
| `hints` | `string[]` | ❌ | Suchpfade (Priorität) |
| `backup` | `string` | ❌ | Backup-Pfad (Warning) |
| `required` | `bool` | ❌ | Default: true |
| `config` | `object` | ❌ | Package-spezifische Config |

### 2.3 Typ-Erkennung

```
if "system" == true  → System External
else if "git" exists → Fetched External  
else if "path" exists → Local External
else → Error E012
```

---

## 3. Pfad-Auflösung

### 3.1 Suchreihenfolge

```
┌─────────────────────────────────────────────────────────────────┐
│ 1. Umgebungsvariablen                                           │
│    └─ ${PACKAGE}_ROOT, ${PACKAGE}_DIR, ${PACKAGE}_HOME          │
├─────────────────────────────────────────────────────────────────┤
│ 2. CMAKE_PREFIX_PATH                                            │
├─────────────────────────────────────────────────────────────────┤
│ 3. hints[] aus Solution.json (in Reihenfolge)                   │
├─────────────────────────────────────────────────────────────────┤
│ 4. Standard-Pfade (plattformspezifisch)                         │
├─────────────────────────────────────────────────────────────────┤
│ 5. backup Pfad                                                  │
│    └─ ⚠️ WARNING: "Using backup location"                       │
├─────────────────────────────────────────────────────────────────┤
│ 6. Error wenn nichts gefunden                                  │
│    └─ ❌ FATAL_ERROR mit Hilfetext                              │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 Plattform-spezifische Standard-Pfade

**Qt6:**

| Plattform | Pfade |
|-----------|-------|
| Windows | `C:/Qt/{VERSION}/msvc2022_64`, `D:/Qt/{VERSION}/msvc2022_64` |
| Linux | `~/Qt/{VERSION}/gcc_64`, `/opt/Qt/{VERSION}/gcc_64`, `/usr/lib/qt6` |
| macOS | `~/Qt/{VERSION}/macos`, `/opt/homebrew/opt/qt@6` |

**Boost:**

| Plattform | Pfade |
|-----------|-------|
| Windows | `C:/local/boost_{VERSION}`, `C:/Boost` |
| Linux | `/usr/include/boost`, `/usr/local/include/boost` |
| macOS | `/opt/homebrew/include/boost` |

### 3.3 Backup-Verhalten

Wenn der `backup` Pfad verwendet wird:

```cmake
message(WARNING 
    "[${name}] Primary installation not found!\n"
    "  Using backup location: ${backup_path}\n"
    "  Consider setting ${NAME}_ROOT environment variable."
)
```

---

## 4. Implementation

### 4.1 Neue Dateien

```
cmake/externals/
├── System/
│   ├── Handler.cmake       # System External Handler
│   ├── PathResolver.cmake  # Pfad-Auflösung
│   └── Packages/
│       ├── Qt6.cmake       # Qt6-spezifische Logik
│       ├── Boost.cmake     # Boost-spezifische Logik
│       └── OpenCV.cmake    # OpenCV-spezifische Logik
└── Orchestrator.cmake      # Erweitert um system Type
```

### 4.2 Orchestrator.cmake Erweiterung

```cmake
function(_process_external EXT_NAME EXT_JSON)
    # Typ-Erkennung
    string(JSON _system ERROR_VARIABLE _err GET "${EXT_JSON}" "system")
    string(JSON _git ERROR_VARIABLE _err2 GET "${EXT_JSON}" "git")
    string(JSON _path ERROR_VARIABLE _err3 GET "${EXT_JSON}" "path")
    
    if(_system)
        # System External
        include(cmake/externals/System/Handler.cmake)
        _handle_system_external("${EXT_NAME}" "${EXT_JSON}")
    elseif(NOT _err2)
        # Fetched External
        _handle_fetched_external("${EXT_NAME}" "${EXT_JSON}")
    elseif(NOT _err3)
        # Local External
        _handle_local_external("${EXT_NAME}" "${EXT_JSON}")
    else()
        message(FATAL_ERROR "[E012] External '${EXT_NAME}': Invalid definition")
    endif()
endfunction()
```

### 4.3 System/Handler.cmake

```cmake
# ==============================================================================
# System/Handler.cmake — System External Handler
# ==============================================================================

include_guard(GLOBAL)
include(cmake/externals/System/PathResolver.cmake)

function(_handle_system_external EXT_NAME EXT_JSON)
    message(STATUS "[${EXT_NAME}] Processing system external")
    
    # JSON parsen
    string(JSON _package GET "${EXT_JSON}" "package")
    string(JSON _version ERROR_VARIABLE _err GET "${EXT_JSON}" "version")
    string(JSON _components ERROR_VARIABLE _err GET "${EXT_JSON}" "components")
    string(JSON _hints ERROR_VARIABLE _err GET "${EXT_JSON}" "hints")
    string(JSON _backup ERROR_VARIABLE _err GET "${EXT_JSON}" "backup")
    
    # Pfad auflösen
    _resolve_system_path("${EXT_NAME}" "${_package}" "${_hints}" "${_backup}" 
                         _resolved_path _is_backup)
    
    if(_is_backup)
        message(WARNING 
            "[${EXT_NAME}] Using backup location: ${_resolved_path}\n"
            "  Consider setting ${_package}_ROOT or installing properly."
        )
    endif()
    
    # CMAKE_PREFIX_PATH erweitern
    list(PREPEND CMAKE_PREFIX_PATH "${_resolved_path}")
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
    
    # find_package aufrufen
    if(_components)
        find_package(${_package} ${_version} REQUIRED COMPONENTS ${_components})
    else()
        find_package(${_package} ${_version} REQUIRED)
    endif()
    
    # Package-spezifische Configuration
    set(_package_config "${CMAKE_CURRENT_LIST_DIR}/Packages/${_package}.cmake")
    if(EXISTS "${_package_config}")
        include("${_package_config}")
    endif()
    
    message(STATUS "[${EXT_NAME}] Found ${_package} ${${_package}_VERSION}")
endfunction()
```

### 4.4 System/PathResolver.cmake

```cmake
# ==============================================================================
# System/PathResolver.cmake — Path Resolution for System Externals
# ==============================================================================

include_guard(GLOBAL)

function(_resolve_system_path EXT_NAME PACKAGE HINTS BACKUP OUT_PATH OUT_IS_BACKUP)
    set(_found FALSE)
    set(_result_path "")
    set(_is_backup FALSE)
    
    # 1. Umgebungsvariablen
    foreach(_var ${PACKAGE}_ROOT ${PACKAGE}_DIR ${PACKAGE}_HOME)
        if(DEFINED ENV{${_var}})
            set(_candidate "$ENV{${_var}}")
            if(_is_valid_package_path("${_candidate}" "${PACKAGE}"))
                set(_found TRUE)
                set(_result_path "${_candidate}")
                message(STATUS "[${EXT_NAME}]   Found via ${_var}: ${_candidate}")
                break()
            endif()
        endif()
    endforeach()
    
    # 2. hints aus Solution.json
    if(NOT _found AND HINTS)
        string(JSON _hints_count LENGTH "${HINTS}")
        if(_hints_count GREATER 0)
            math(EXPR _last "${_hints_count} - 1")
            foreach(_idx RANGE 0 ${_last})
                string(JSON _hint GET "${HINTS}" ${_idx})
                string(CONFIGURE "${_hint}" _hint_expanded)
                if(_is_valid_package_path("${_hint_expanded}" "${PACKAGE}"))
                    set(_found TRUE)
                    set(_result_path "${_hint_expanded}")
                    message(STATUS "[${EXT_NAME}]   Found via hint: ${_hint_expanded}")
                    break()
                endif()
            endforeach()
        endif()
    endif()
    
    # 3. Standard-Pfade
    if(NOT _found)
        _get_standard_paths("${PACKAGE}" _std_paths)
        foreach(_path IN LISTS _std_paths)
            if(_is_valid_package_path("${_path}" "${PACKAGE}"))
                set(_found TRUE)
                set(_result_path "${_path}")
                message(STATUS "[${EXT_NAME}]   Found at standard path: ${_path}")
                break()
            endif()
        endforeach()
    endif()
    
    # 4. Backup
    if(NOT _found AND BACKUP)
        string(CONFIGURE "${BACKUP}" _backup_expanded)
        if(_is_valid_package_path("${_backup_expanded}" "${PACKAGE}"))
            set(_found TRUE)
            set(_result_path "${_backup_expanded}")
            set(_is_backup TRUE)
        endif()
    endif()
    
    # Ergebnis
    if(_found)
        set(${OUT_PATH} "${_result_path}" PARENT_SCOPE)
        set(${OUT_IS_BACKUP} ${_is_backup} PARENT_SCOPE)
    else()
        message(FATAL_ERROR
            "[E501] [${EXT_NAME}] ${PACKAGE} not found!\n"
            "  \n"
            "  Set one of these environment variables:\n"
            "    ${PACKAGE}_ROOT\n"
            "    ${PACKAGE}_DIR\n"
            "  \n"
            "  Or add 'hints' in Solution.json:\n"
            "    \"hints\": [\"C:/Path/To/${PACKAGE}\"]\n"
        )
    endif()
endfunction()
```

---

## 5. Examples

### 5.1 Qt6 (vollständig)

```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "version": ">=6.5.0",
    "components": ["Core", "Widgets", "Gui", "OpenGL"],
    "hints": [
        "${QT_ROOT}",
        "C:/Qt/6.7.0/msvc2022_64",
        "D:/Development/Qt/6.7.0"
    ],
    "backup": "E:/Backup/Libs/Qt/6.7.0/msvc2022_64",
    "config": {
        "automoc": true,
        "autouic": true,
        "autorcc": true
    }
}
```

### 5.2 Boost (minimal)

```json
"boost": {
    "system": true,
    "package": "Boost",
    "components": ["filesystem", "system", "thread"]
}
```

### 5.3 OpenCV mit CUDA

```json
"opencv": {
    "system": true,
    "package": "OpenCV",
    "version": ">=4.5.0",
    "hints": ["${OPENCV_DIR}"],
    "config": {
        "with_cuda": true
    }
}
```

### 5.4 CUDA Toolkit

```json
"cuda": {
    "system": true,
    "package": "CUDAToolkit",
    "version": ">=11.0",
    "components": ["cudart", "cublas", "curand"]
}
```

---

## 6. Error Codes

### 6.1 System External Errors (E5xx)

| Code | Description |
|------|--------------|
| E501 | System external not found (no valid path) |
| E502 | System external: 'package' field is required |
| E503 | System external: find_package failed |
| E504 | System external: required component not found |
| E505 | System external: version constraint not satisfied |

### 6.2 System External Warnings (W5xx)

| Code | Description |
|------|--------------|
| W501 | Using backup location for system external |
| W502 | System external version mismatch (using found version) |

---

## 7. Migration

### 7.1 Von Workaround zu system

**Vorher (aktuell):**
```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}",
        "components": ["Core", "Widgets"]
    }
}
```

**Nachher (Ziel):**
```json
"qt6": {
    "system": true,
    "package": "Qt6",
    "components": ["Core", "Widgets"],
    "hints": ["${QT_ROOT}"]
}
```

### 7.2 Vorteile

| Aspekt | Workaround | system |
|--------|------------|--------|
| Semantik | Unklar | Klar |
| Ordner nötig | `externals/qt6/` | No |
| Include.cmake | Manuell | Optional/Standard |
| Backup-Support | Manuell | Integriert |
| Version-Check | Manuell | Integriert |
| Standard-Pfade | In Include.cmake | Zentral |

---

## 8. Offene Punkte

### 8.1 Zu klären

| Frage | Optionen | Status |
|-------|----------|--------|
| Schema-Version? | Erfordert schemaVersion Bump (0.2)? | Offen |
| Rückwärtskompatibilität? | path + options weiter unterstützen? | Yes |
| Package-spezifische Configs? | Wie strukturieren? | Packages/*.cmake |
| Validation? | JSON Schema für system Externals? | Phase 9 |

### 8.2 Nicht im Scope

- vcpkg/Conan Integration (separates Feature)
- Automatischer Download von System Externals
- Version-Locking für System Externals

---

## 9. Roadmap

| Phase | Description | Status |
|-------|--------------|--------|
| Aktuell | Workaround mit path + options + backup | ✅ Verfügbar |
| Phase 9 | system Feld implementieren | 🔄 Geplant |
| Post-Release | Package-spezifische Configs, vcpkg Integration | ⬜ Später |

---

## 10. See Also

- [master_concept.md](master_concept.md) — Architecture-Overview
- [implementation_plan.md](implementation_plan.md) — Phasen-Plan
- [Future_Enhancements.md](Future_Enhancements.md) — vcpkg/Conan Integration

---

## 11. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Format, nummeriertes Table of Contents, UTF-8 korrigiert, Phase 9 Reference, Error Codes E5xx/W5xx hinzugefügt** |
| 0.1.0 | 2025-12-10 | Initial: Concept für System Externals |
