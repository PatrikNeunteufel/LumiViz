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

### 5.1 Neue Dateien

```
cmake/externals/
├── system/
│   ├── Handler.cmake       # System External Handler
│   └── PathResolver.cmake  # Pfad-Auflösung
└── Orchestrator.cmake      # Erweitert um system Type
```

### 5.2 Orchestrator.cmake Erweiterung

```cmake
function(_process_external EXT_NAME EXT_JSON)
    # Typ-Erkennung (Reihenfolge wichtig!)
    _json_get_bool_or_default("${EXT_JSON}" "system" FALSE _is_system)
    string(JSON _git ERROR_VARIABLE _err_git GET "${EXT_JSON}" "git")
    string(JSON _path ERROR_VARIABLE _err_path GET "${EXT_JSON}" "path")
    
    if(_is_system)
        # System External → find_package
        include(cmake/externals/system/Handler.cmake)
        _handle_system_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(NOT "${_err_git}" STREQUAL "NOTFOUND")
        # Fetched External → FetchContent
        _handle_fetched_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(NOT "${_err_path}" STREQUAL "NOTFOUND")
        # Local External → Include.cmake
        _handle_local_external("${EXT_NAME}" "${EXT_JSON}")
        
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

### 5.3 system/Handler.cmake

```cmake
# ==============================================================================
# system/Handler.cmake — System External Handler
# ==============================================================================

include_guard(GLOBAL)
include(cmake/externals/system/PathResolver.cmake)

function(_handle_system_external EXT_NAME EXT_JSON)
    dbg(${DBG_COMMON} "[${EXT_NAME}] Processing system external" ID EXTERNALS)
    
    # Requiredfeld: package
    _json_get_string("${EXT_JSON}" "package" _package)
    if("${_package}" STREQUAL "")
        cmake_fatal("E502" "System external '${EXT_NAME}': 'package' field is required")
    endif()
    
    # Optionale Felder
    _json_get_string("${EXT_JSON}" "version" _version)
    _json_get_array_as_list("${EXT_JSON}" "components" _components)
    _json_get_array_as_list("${EXT_JSON}" "hints" _hints)
    _json_get_string("${EXT_JSON}" "backup" _backup)
    _json_get_bool_or_default("${EXT_JSON}" "required" TRUE _required)
    
    # Pfad auflösen
    _resolve_system_path("${EXT_NAME}" "${_package}" "${_hints}" "${_backup}" 
                         _resolved_path _used_backup)
    
    # Warning bei Backup-Usage
    if(_used_backup)
        cmake_warn("W501" 
            "[${EXT_NAME}] Using backup location: ${_resolved_path}\n"
            "  Consider setting ${_package}_ROOT environment variable."
        )
    endif()
    
    # CMAKE_PREFIX_PATH erweitern (wenn Pfad gefunden)
    if(NOT "${_resolved_path}" STREQUAL "")
        list(PREPEND CMAKE_PREFIX_PATH "${_resolved_path}")
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
    endif()
    
    # find_package aufrufen
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
    elseif(_required)
        cmake_fatal("E503" "System external '${EXT_NAME}': find_package(${_package}) failed")
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

### 8.1 Von Workaround zu system

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

### 8.2 Vergleich

| Aspekt | Workaround | System External |
|--------|------------|-----------------|
| Semantik | ❌ Unklar | ✅ Klar |
| Ordner nötig | ❌ `externals/qt6/` | ✅ No |
| Include.cmake | ❌ Manuell | ✅ Optional |
| Backup-Support | ❌ Manuell | ✅ Integriert |
| Version-Check | ❌ Manuell | ✅ Integriert |

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
| **0.6.0** | **2025-12-18** | **Klarere Erklärung: Typ-Erkennung (Section 3), Pfad-Auflösung mit Examplesn (Section 4), Requiredfelder-Logik korrigiert (system+package zusammen), Diagramme hinzugefügt** |
| 0.5.0 | 2025-12-14 | Blueprint v0.5.0 Format, Error Codes E5xx/W5xx |
| 0.1.0 | 2025-12-10 | Initial: Concept für System Externals |
