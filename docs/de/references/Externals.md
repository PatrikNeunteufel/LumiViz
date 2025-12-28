# Externals — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [Externals.md](../../en/references/Externals.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Kategorien](#2-kategorien)
3. [Testing](#3-testing)
4. [Scripting](#4-scripting)
5. [GUI](#5-gui)
6. [Media](#6-media)
7. [Core](#7-core)
8. [Data](#8-data)
9. [Network](#9-network)
10. [AI](#10-ai)
11. [Schnellreferenz](#11-schnellreferenz)
12. [Siehe auch](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Übersicht

Diese Referenz bietet eine **kategorisierte Übersicht** aller verfügbaren External Libraries für das CMake Architecture Build-System.

### External-Typen

| Typ | Quelle | Beschreibung |
|-----|--------|--------------|
| **Local** | `path` | Vorkompilierte/generierte Libraries im `externals/` Ordner |
| **Git** | `git` | Via Git geklonte Libraries |

### Grundlegende Verwendung

```json
{
    "externals": {
        "bass": { "path": "externals/bass" },
        "glad": { "path": "externals/glad" },
        "glfw": { 
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        }
    },
    "executables": [
        {
            "name": "MyApp",
            "externals": ["bass", "glad", "glfw"]
        }
    ]
}
```

---

## 2. Kategorien

### Übersicht

Die Kategorien sind für **Local und Git parallel** strukturiert:

| Kategorie | Local | Git | Hauptverwendung |
|-----------|-------|-----|-----------------|
| [Testing](#3-testing) | doctest | googletest, catch2, benchmark | Unit Tests, Benchmarks |
| [Scripting](#4-scripting) | lua54 | sol2, pybind11, chaiscript | Embedded Scripting |
| [GUI](#5-gui) | glad | glfw, imgui, SDL2/3, raylib | Graphics, Window, UI |
| [Media](#6-media) | BASS | miniaudio, openal-soft, stb, glm | Audio, Image, Math |
| [Core](#7-core) | — | spdlog, fmt, abseil, magic_enum | Logging, Utility |
| [Data](#8-data) | — | nlohmann_json, yaml-cpp, SQLiteCpp | JSON, DB, Kompression |
| [Network](#9-network) | — | cpp-httplib, asio, taskflow | HTTP, Threading |
| [AI](#10-ai) | — | llama.cpp, whisper.cpp, onnxruntime | ML, LLM, CUDA |

### Detail-Dokumente

| Kategorie | Local | Git |
|-----------|-------|-----|
| Testing | [Local_Externals_Testing.md](externals/Local_Externals_Testing.md) | [Git_Externals_Testing.md](externals/Git_Externals_Testing.md) |
| Scripting | [Local_Externals_Scripting.md](externals/Local_Externals_Scripting.md) | [Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) |
| GUI | [Local_Externals_GUI.md](externals/Local_Externals_GUI.md) | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| Media | [Local_Externals_Media.md](externals/Local_Externals_Media.md) | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| Core | — | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| Data | — | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| Network | — | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| AI | — | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |

---

## 3. Testing

### Local

| Library | Typ | Beschreibung | Details |
|---------|-----|--------------|---------|
| **doctest** | 📄 Header-Only | Schnelles Testing Framework | [Local_Externals_Testing.md](externals/Local_Externals_Testing.md) |

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **googletest** | Google Test + GMock | ✅ | PreFetch | [Git_Externals_Testing.md](externals/Git_Externals_Testing.md) |
| **catch2** | BDD-Style Testing | ✅ | PreFetch | [Git_Externals_Testing.md](externals/Git_Externals_Testing.md) |
| **benchmark** | Google Benchmark | ✅ | PreFetch | [Git_Externals_Testing.md](externals/Git_Externals_Testing.md) |

### Empfehlung

| Anwendungsfall | Empfehlung |
|----------------|------------|
| Schnelle Unit Tests | **doctest** (Local) |
| Mocking benötigt | googletest (Git) |
| BDD-Style | catch2 (Git) |
| Performance-Tests | benchmark (Git) |

---

## 4. Scripting

### Local

| Library | Typ | Beschreibung | Details |
|---------|-----|--------------|---------|
| **lua54** | 📝 Source | Lua 5.4 Scripting Engine | [Local_Externals_Scripting.md](externals/Local_Externals_Scripting.md) |

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **sol2** | C++ Lua Bindings | ✅ | — | [Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) |
| **pybind11** | Python Bindings | ✅ | — | [Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) |
| **chaiscript** | Embedded Scripting | ✅ | — | [Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) |

### Empfehlung

| Anwendungsfall | Empfehlung |
|----------------|------------|
| Konfiguration, Modding | **lua54** (Local) + sol2 (Git) |
| Python-Integration | pybind11 (Git) |
| C++-ähnliche Syntax | chaiscript (Git) |

---

## 5. GUI

### Local

| Library | Typ | Beschreibung | Details |
|---------|-----|--------------|---------|
| **glad** | 🔧 Generiert | OpenGL Function Loader | [Local_Externals_GUI.md](externals/Local_Externals_GUI.md) |

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **glfw** | Window/Input Library | ✅ | PreFetch | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| **imgui** | Immediate Mode GUI | ❌ | PostFetch 🔧 | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| **SDL2** | Multimedia Library | ✅ | PreFetch | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| **SDL3** | Multimedia (Modern) | ✅ | PreFetch | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| **raylib** | Game Development | ✅ | PreFetch | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |

### Typische Kombination (OpenGL + ImGui)

```json
"externals": {
    "glad": { "path": "externals/glad" },
    "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
    "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false }
}
```

> **Reihenfolge wichtig:** glad → glfw → imgui

---

## 6. Media

### Local

| Library | Typ | Beschreibung | Details |
|---------|-----|--------------|---------|
| **BASS** | 📦 Vorkompiliert | Audio-Library mit Plugins | [Local_Externals_Media.md](externals/Local_Externals_Media.md) |

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **miniaudio** | Single-Header Audio | ❌ | PostFetch 🔧 | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| **openal-soft** | 3D Audio | ✅ | PreFetch | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| **stb** | Image Loading | ❌ | PostFetch 🔧 | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| **glm** | OpenGL Mathematics | ✅ | PreFetch | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| **Eigen** | Linear Algebra | ✅ | PreFetch | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |

### Empfehlung

| Anwendungsfall | Empfehlung |
|----------------|------------|
| Musik-Player, Plugins | **BASS** (Local) |
| Einfache Sounds | miniaudio (Git) |
| 3D-Spiele Audio | openal-soft (Git) |

---

## 7. Core

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **spdlog** | Fast Logging | ✅ | PreFetch | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| **fmt** | String Formatting | ✅ | PreFetch | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| **abseil** | Google's C++ Library | ✅ | PreFetch | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| **magic_enum** | Enum Reflection | ✅ | — | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| **argparse** | Argument Parsing | ✅ | — | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| **CLI11** | CLI Parsing | ✅ | PreFetch | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |

---

## 8. Data

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **nlohmann_json** | JSON for Modern C++ | ✅ | PreFetch | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| **rapidjson** | Fast JSON | ✅ | — | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| **yaml-cpp** | YAML Parser | ✅ | PreFetch | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| **toml++** | TOML Parser | ✅ | — | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| **SQLiteCpp** | SQLite Wrapper | ✅ | PreFetch | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| **zstd** | Fast Compression | ✅ | PreFetch | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |

---

## 9. Network

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **cpp-httplib** | HTTP Client/Server | ✅ | — | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| **cpr** | C++ Requests | ✅ | PreFetch | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| **asio** | Async I/O | ⚠️ | PostFetch 🔧 | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| **taskflow** | Parallel Programming | ✅ | PreFetch | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| **concurrentqueue** | Lock-free Queue | ✅ | — | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |

---

## 10. AI

### Git

| Library | Beschreibung | CMake | Hook | Details |
|---------|--------------|-------|------|---------|
| **llama.cpp** | LLM Inference | ✅ | PreFetch | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |
| **whisper.cpp** | Speech-to-Text | ✅ | PreFetch | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |
| **onnxruntime** | ML Inference | ⚠️ | PreFetch | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |
| **ncnn** | Neural Network | ✅ | PreFetch | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |

> **Hinweis:** AI/ML Libraries sind oft komplex zu integrieren. Siehe Detail-Dokumentation.

---

## 11. Schnellreferenz

### Alle Local Externals

| Library | Kategorie | Typ | Include.cmake |
|---------|-----------|-----|---------------|
| **doctest** | Testing | 📄 Header-Only | `cmake/externals/includes/doctest/` |
| **lua54** | Scripting | 📝 Source | `cmake/externals/includes/lua54/` |
| **glad** | GUI | 🔧 Generiert | `cmake/externals/includes/glad/` |
| **BASS** | Media | 📦 Vorkompiliert | `cmake/externals/includes/bass/` |

### Beliebte Kombinationen

| Anwendung | Externals |
|-----------|-----------|
| OpenGL GUI App | glad (L), glfw (G), imgui (G) |
| Audio Player | BASS (L) + BASS_FLAC, BASS_FX |
| Scripted Game | lua54 (L), sol2 (G), raylib (G) |
| REST API Client | cpp-httplib (G), nlohmann_json (G) |
| Unit Test Suite | doctest (L) oder googletest (G) |

*(L) = Local, (G) = Git*

---

## 12. Siehe auch

### Übersichts-Dokumente

- [Local_Externals.md](externals/Local_Externals.md) — Übersicht Local Externals
- [Git_Externals.md](externals/Git_Externals.md) — Übersicht Git Externals

### Kategorie-Detail-Dokumente

| Kategorie | Local | Git |
|-----------|-------|-----|
| Testing | [Local_Externals_Testing.md](externals/Local_Externals_Testing.md) | [Git_Externals_Testing.md](externals/Git_Externals_Testing.md) |
| Scripting | [Local_Externals_Scripting.md](externals/Local_Externals_Scripting.md) | [Git_Externals_Scripting.md](externals/Git_Externals_Scripting.md) |
| GUI | [Local_Externals_GUI.md](externals/Local_Externals_GUI.md) | [Git_Externals_GUI.md](externals/Git_Externals_GUI.md) |
| Media | [Local_Externals_Media.md](externals/Local_Externals_Media.md) | [Git_Externals_Media.md](externals/Git_Externals_Media.md) |
| Core | — | [Git_Externals_Core.md](externals/Git_Externals_Core.md) |
| Data | — | [Git_Externals_Data.md](externals/Git_Externals_Data.md) |
| Network | — | [Git_Externals_Network.md](externals/Git_Externals_Network.md) |
| AI | — | [Git_Externals_AI.md](externals/Git_Externals_AI.md) |

### Weitere Referenzen

- [Solution_Schema.md](Solution_Schema.md) — JSON-Schema für externals Block
- [Externals_UserGuide.md](../userguides/Externals.md) — Verwendungsanleitung

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **8 Kategorien: Testing, Scripting, GUI, Media, Core, Data, Network, AI — parallel für Local + Git** |
| 0.5.0 | 2025-12-15 | Kategorisierte Übersicht für Local + Git Externals |
