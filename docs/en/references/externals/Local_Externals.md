# Local Externals — Overview

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Local_Externals.md](../../en/references/externals/Local_Externals.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Kategorien](#3-kategorien)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Include.cmake System](#5-includecmake-system)
6. [Usage](#6-verwendung)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Diese Reference listet **lokale External Libraries**, die im `externals/` Ordner bereitgestellt werden. Im Gegensatz zu Git-Externals werden diese nicht gefetcht, sondern sind vorkompiliert, generiert oder als Quellen vorhanden.

### Umfang

- **4 Bibliotheken** in 4 Kategorien
- Vorkompilierte Libraries (BASS)
- Generierte Loader (glad)
- Header-Only Libraries (doctest)
- Source-basierte Libraries (lua54)

### Dokumentstruktur

Die Bibliotheken sind nach Kategorien in separate Dokumente aufgeteilt — **parallel zu den Git-Kategorien**:

| Dokument | Kategorie | Bibliotheken | Git-Pendant |
|----------|-----------|--------------|-------------|
| [Local_Externals_Testing.md](Local_Externals_Testing.md) | Testing | doctest | Git_Externals_Testing.md |
| [Local_Externals_Scripting.md](Local_Externals_Scripting.md) | Scripting | lua54 | Git_Externals_Scripting.md |
| [Local_Externals_GUI.md](Local_Externals_GUI.md) | GUI | glad | Git_Externals_GUI.md |
| [Local_Externals_Media.md](Local_Externals_Media.md) | Media | BASS | Git_Externals_Media.md |

---

## 2. Konventionen

### 2.1 Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ✅ | Verfügbar / Unterstützt |
| ❌ | Nicht verfügbar |
| 📦 | Vorkompiliert (Binaries) |
| 🔧 | Generiert (Konfigurator) |
| 📄 | Header-Only |
| 📝 | Aus Quellen kompiliert |

### 2.2 Library-Typen

| Typ | Description | Example |
|-----|--------------|----------|
| **Vorkompiliert** | DLLs/SOs mit Import-Libs | BASS |
| **Generiert** | Via Web-Konfigurator erstellt | glad |
| **Header-Only** | Nur Header, keine Kompilierung | doctest |
| **Source-basiert** | Wird aus Quellen kompiliert | lua54 |

### 2.3 Include.cmake Pfade

**Convention over Configuration:**

```
cmake/externals/includes/{name}/Include.cmake
```

| External | Include.cmake Pfad |
|----------|-------------------|
| doctest | `cmake/externals/includes/doctest/Include.cmake` |
| lua54 | `cmake/externals/includes/lua54/Include.cmake` |
| glad | `cmake/externals/includes/glad/Include.cmake` |
| bass | `cmake/externals/includes/bass/Include.cmake` |

---

## 3. Kategorien

### 3.1 Testing

Unit Testing und Benchmarking.

| Bibliothek | Typ | Description | Plattformen |
|------------|-----|--------------|-------------|
| **doctest** | 📄 Header-Only | Schnelles Testing Framework | Alle |

→ Details: [Local_Externals_Testing.md](Local_Externals_Testing.md)

### 3.2 Scripting

Embedded Scripting Engines.

| Bibliothek | Typ | Description | Plattformen |
|------------|-----|--------------|-------------|
| **lua54** | 📝 Source | Lua 5.4 Scripting Engine | Alle |

→ Details: [Local_Externals_Scripting.md](Local_Externals_Scripting.md)

### 3.3 GUI

OpenGL Loading und Rendering.

| Bibliothek | Typ | Description | Plattformen |
|------------|-----|--------------|-------------|
| **glad** | 🔧 Generiert | OpenGL Function Loader | Alle |

→ Details: [Local_Externals_GUI.md](Local_Externals_GUI.md)

### 3.4 Media

Audio-Verarbeitung und Multimedia.

| Bibliothek | Typ | Description | Plattformen |
|------------|-----|--------------|-------------|
| **BASS** | 📦 Vorkompiliert | Audio-Library mit Plugin-System | Win, Linux, macOS |

→ Details: [Local_Externals_Media.md](Local_Externals_Media.md)

---

## 4. Schnellreferenz

### 4.1 Alle Local Externals

| Library | Kategorie | Typ | Options | Detail-Dok |
|---------|-----------|-----|---------|------------|
| **doctest** | Testing | 📄 Header-Only | — | [doctest_Include.md](../../modules/externals/includes/doctest/Doctest_Include.md) |
| **lua54** | Scripting | 📝 Source | LUA_EMBEDDED | [lua54_Include.md](../../modules/externals/includes/lua54/Lua54_Include.md) |
| **glad** | GUI | 🔧 Generiert | — | [glad_Include.md](../../modules/externals/includes/glad/Glad_Include.md) |
| **BASS** | Media | 📦 Vorkompiliert | BASS_FLAC, BASS_FX, ... | [bass_Include.md](../../modules/externals/includes/bass/Bass_Include.md) |

### 4.2 Kategorie-Mapping (Local ↔ Git)

| Kategorie | Local Externals | Git Externals |
|-----------|-----------------|---------------|
| **Testing** | doctest | googletest, catch2, benchmark |
| **Scripting** | lua54 | sol2, pybind11, chaiscript |
| **GUI** | glad | glfw, imgui, SDL2, raylib |
| **Media** | BASS | miniaudio, openal-soft, stb, glm |

### 4.3 Verzeichnisstruktur

```
externals/
├── doctest/
│   └── include/
│       └── doctest/doctest.h
│
├── lua54/
│   └── src/
│       ├── lua.h
│       └── *.c
│
├── glad/
│   ├── include/
│   └── src/
│
└── bass/
    ├── include/
    └── lib/
```

### 4.4 Solution.json Overview

```json
{
    "externals": {
        "doctest": { "path": "externals/doctest" },
        "lua54": { "path": "externals/lua54" },
        "glad": { "path": "externals/glad" },
        "bass": { "path": "externals/bass" }
    }
}
```

---

## 5. Include.cmake System

### 5.1 Convention over Configuration

Local Externals benötigen eine `Include.cmake` die das Target erstellt:

```
cmake/externals/includes/{name}/Include.cmake
```

Das `include` Feld in Solution.json ist **optional** — die Convention wird automatisch verwendet.

### 5.2 Verfügbare Variablen

In jeder Include.cmake stehen diese Variablen zur Verfügung:

| Variable | Description |
|----------|--------------|
| `EXTERNAL_NAME` | Name des Externals (z.B. "bass") |
| `EXTERNAL_PATH` | Absoluter Pfad zum External |
| `EXTERNAL_JSON` | JSON-Element aus Solution.json |
| `EXTERNAL_OPTIONS` | Target-spezifische Options |

### 5.3 Include.cmake Templates

**Header-Only (doctest):**

```cmake
add_library(${EXTERNAL_NAME} INTERFACE)
target_include_directories(${EXTERNAL_NAME} INTERFACE
    "${EXTERNAL_PATH}/include"
)
_register_external_target("${EXTERNAL_NAME}" "${EXTERNAL_NAME}" PRIMARY)
```

**Aus Quellen (lua54):**

```cmake
file(GLOB _lua_sources "${EXTERNAL_PATH}/src/*.c")
add_library(lua54 STATIC ${_lua_sources})
target_include_directories(lua54 PUBLIC "${EXTERNAL_PATH}/src")
_register_external_target("lua54" "lua54" PRIMARY)
```

**Generiert (glad):**

```cmake
add_library(glad STATIC "${EXTERNAL_PATH}/src/glad.c")
target_include_directories(glad PUBLIC "${EXTERNAL_PATH}/include")
find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)
_register_external_target("glad" "glad" PRIMARY)
```

**Vorkompiliert (BASS):**

```cmake
add_library(bass SHARED IMPORTED GLOBAL)
set_target_properties(bass PROPERTIES
    IMPORTED_LOCATION "${EXTERNAL_PATH}/lib/x64/bass.dll"
    IMPORTED_IMPLIB "${EXTERNAL_PATH}/lib/x64/bass.lib"
)
target_include_directories(bass INTERFACE "${EXTERNAL_PATH}/include")
_register_external_target("bass" "bass" PRIMARY)
```

---

## 6. Usage

### 6.1 Einfaches Local External

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    },
    "tests": [
        {
            "name": "MyTests",
            "externals": ["doctest"]
        }
    ]
}
```

### 6.2 Mit Options (BASS Plugins)

```json
{
    "externals": {
        "bass": {
            "path": "externals/bass"
        }
    },
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"],
            "external_options": {
                "bass": {
                    "BASS_FLAC": true,
                    "BASS_FX": true
                }
            }
        }
    ]
}
```

### 6.3 Kombiniert mit Git Externals

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false }
    },
    "executables": [
        {
            "name": "MyGuiApp",
            "type": "GUI",
            "externals": ["glad", "glfw", "imgui"]
        }
    ]
}
```

> **Reihenfolge wichtig:** glad → glfw → imgui

---

## 7. See Also

- [Externals.md](Externals.md) — Hauptübersicht (Local + Git)
- [Git_Externals.md](Git_Externals.md) — Git Externals Overview
- [Solution_Schema.md](Solution_Schema.md) — External-Configuration
- [Attach_cmake.md](../modules/externals/Attach_cmake.md) — Local External Handler

### Kategorie-Dokumente

- [Local_Externals_Testing.md](Local_Externals_Testing.md) — doctest
- [Local_Externals_Scripting.md](Local_Externals_Scripting.md) — lua54
- [Local_Externals_GUI.md](Local_Externals_GUI.md) — glad
- [Local_Externals_Media.md](Local_Externals_Media.md) — BASS

### Detail-Dokumentationen (Include.cmake)

- [doctest_Include.md](../modules/externals/includes/doctest_Include.md)
- [lua54_Include.md](../modules/externals/includes/lua54_Include.md)
- [glad_Include.md](../modules/externals/includes/glad_Include.md)
- [bass_Include.md](../modules/externals/includes/bass_Include.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **4 Kategorien parallel zu Git: Testing, Scripting, GUI, Media** |
| 0.5.0 | 2025-12-15 | Initial: Overview für Local Externals |
