# Externe Bibliotheken hinzufügen – Benutzerhandbuch

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Build System Developers, C++ Developers  
> **Language:** English  
> **German:** [Adding_Externals_UserGuide.md](../../en/userguides/Adding_Externals.md)

---

## Table of Contents

1. [Überblick](#1-überblick)
2. [Prerequisites](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [External analysieren](#4-external-analysieren)
5. [Solution.json konfigurieren](#5-solutionjson-konfigurieren)
6. [PreFetch Hook erstellen](#6-prefetch-hook-erstellen)
7. [PostFetch Hook erstellen](#7-postfetch-hook-erstellen)
8. [Executable konfigurieren](#8-executable-konfigurieren)
9. [Testen](#9-testen)
10. [Stolpersteine und Lösungen](#10-stolpersteine-und-lösungen)
11. [Troubleshooting](#11-troubleshooting)
12. [See Also](#12-siehe-auch)

---

## 1. Überblick

This guide explains Schritt für Schritt, wie neue externe Bibliotheken zum CMake Architecture Build-System hinzugefügt werden.

### Zielgruppe

- **Primär:** Build System Developers, die das External-System erweitern
- **Sekundär:** C++ Developers, die neue Bibliotheken integrieren möchten

### Ablauf

```
┌─────────────────────────────────────────────────────────────┐
│ 1. External analysieren                                      │
│    └─ CMake Support? Tests/Examples? Dependencies?        │
├─────────────────────────────────────────────────────────────┤
│ 2. Solution.json konfigurieren                               │
│    └─ externals Block erweitern                             │
├─────────────────────────────────────────────────────────────┤
│ 3. PreFetch Hook erstellen (falls nötig)                    │
│    └─ Tests/Examples deaktivieren                           │
├─────────────────────────────────────────────────────────────┤
│ 4. PostFetch Hook erstellen (falls nötig)                   │
│    └─ Target manuell erstellen                              │
├─────────────────────────────────────────────────────────────┤
│ 5. Executable konfigurieren                                  │
│    └─ External in externals-Liste aufnehmen                 │
├─────────────────────────────────────────────────────────────┤
│ 6. Testen                                                    │
│    └─ Configure, Build, Link                                │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Prerequisites

### Checkliste

- [ ] **CMake Architecture** eingerichtet und funktionsfähig
- [ ] **Git** installiert (für Git Externals)
- [ ] **Kenntnisse:** CMake-Grundlagen (add_library, target_link_libraries)
- [ ] **Zugriff:** Repository des Externals (für Analyse)

### Empfohlene Kenntnisse

| Thema | Warum benötigt |
|-------|----------------|
| CMake Targets | Für PostFetch-Hooks |
| FetchContent | Verständnis des Download-Mechanismus |
| CMake Cache | Für PreFetch-Hooks |
| Solution.json Schema | Korrekte Configuration |

---

## 3. Schnellstart

### Einfachstes Szenario: Git External mit CMake Support

```json
{
    "externals": {
        "spdlog": {
            "git": "https://github.com/gabime/spdlog.git",
            "tag": "v1.14.1"
        }
    },
    "executables": [
        {
            "name": "MyApp",
            "externals": ["spdlog"]
        }
    ]
}
```

Fertig! Kein Hook nötig, wenn das External:
- Eine `CMakeLists.txt` hat
- Standard-Targets erstellt
- Keine störenden Tests/Examples baut

---

## 4. External analysieren

Bevor du ein External hinzufügst, beantworte folgende Fragen:

### 4.1 CMake Support prüfen

**Prüfe im Repository:**
- Gibt es eine `CMakeLists.txt` im Root?
- Wird `add_library()` oder `add_executable()` verwendet?

| Situation | cmakeSupport | Hook |
|-----------|--------------|------|
| CMakeLists.txt vorhanden | `true` (default) | PreFetch optional |
| Kein CMakeLists.txt | `false` | PostFetch **erforderlich** |
| Header-only Library | `true` oder `false` | Je nach CMake-Integration |

### 4.2 Build-Optionen finden

**Suche in CMakeLists.txt nach:**
```cmake
option(BUILD_TESTS ...)
option(BUILD_EXAMPLES ...)
option(BUILD_DOCS ...)
option(XXX_INSTALL ...)
```

Diese sollten im PreFetch Hook deaktiviert werden.

### 4.3 Target-Namen identifizieren

**Suche nach:**
```cmake
add_library(target_name ...)
```

Du brauchst den genauen Target-Namen für `target_link_libraries()`.

### 4.4 Dependencies prüfen

- Braucht das External andere Libraries?
- Sind diese bundled oder extern?

---

## 5. Solution.json konfigurieren

### 5.1 Basis-Configuration

**Füge zum `externals` Block hinzu:**

```json
{
    "externals": {
        "mein_external": {
            "git": "https://github.com/user/repo.git",
            "tag": "v1.0.0"
        }
    }
}
```

### 5.2 Felder-Reference

| Feld | Required | Description |
|------|---------|--------------|
| `git` | ✅ | Repository URL |
| `tag` | ✅* | Git Tag (empfohlen) |
| `branch` | ❌* | Git Branch (für bleeding edge) |
| `commit` | ❌* | Commit Hash (für exakte Version) |
| `cmakeSupport` | ❌ | `false` wenn kein CMakeLists.txt |
| `hook` | ❌ | Hook-Wiederverwendung |

*Genau eines von `tag`, `branch`, `commit` erforderlich.

### 5.3 Examples

**Standard (mit CMake Support):**
```json
"spdlog": {
    "git": "https://github.com/gabime/spdlog.git",
    "tag": "v1.14.1"
}
```

**Ohne CMake Support:**
```json
"imgui": {
    "git": "https://github.com/ocornut/imgui.git",
    "tag": "v1.91.6",
    "cmakeSupport": false
}
```

**Mit Hook-Wiederverwendung:**
```json
"imgui_docking": {
    "git": "https://github.com/ocornut/imgui.git",
    "tag": "v1.91.6-docking",
    "cmakeSupport": false,
    "hook": "imgui"
}
```

---

## 6. PreFetch Hook erstellen

### 6.1 Wann benötigt?

Ein PreFetch Hook ist sinnvoll wenn das External:
- Tests/Examples/Benchmarks baut (Build-Zeit!)
- Install-Targets erstellt (nicht nötig mit FetchContent)
- Dokumentation generiert
- Spezielle Optionen braucht

### 6.2 Hook-Datei erstellen

**Path:** `cmake/externals/Hooks/PreFetch/{name}.cmake`

**Template:**
```cmake
# ==============================================================================
# PreFetch/{name}.cmake – {Name} PreFetch Hook
# ==============================================================================
#
# Hook:         {name}.cmake
# Version:      0.1.0
# Date:         {DATUM}
# Part of:      CMake Architecture
#
# Description:
#   PreFetch hook for {Name}.
#   Disables tests, examples, and installation.
#
# ==============================================================================

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch: Setting options")

# ==============================================================================
# Disable Tests
# ==============================================================================

set({PREFIX}_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set({PREFIX}_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# ==============================================================================
# Disable Examples
# ==============================================================================

set({PREFIX}_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# ==============================================================================
# Disable Install
# ==============================================================================

set({PREFIX}_INSTALL OFF CACHE BOOL "" FORCE)

# ==============================================================================
# Disable Documentation
# ==============================================================================

set({PREFIX}_BUILD_DOCS OFF CACHE BOOL "" FORCE)

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch complete")
```

### 6.3 Importante Notee

**CACHE BOOL "" FORCE:**
```cmake
set(OPTION OFF CACHE BOOL "" FORCE)
#                          ^^^^^ Important! Überschreibt existierende Werte
```

**Dynamischer Name:**
```cmake
message(STATUS "[${HOOK_EXTERNAL_NAME}] ...")
#                ^^^^^^^^^^^^^^^^^^^^^ Verwende immer diese Variable
```

---

## 7. PostFetch Hook erstellen

### 7.1 Wann benötigt?

Ein PostFetch Hook ist **erforderlich** wenn:
- `cmakeSupport: false` (kein CMakeLists.txt)
- Zusätzliche Targets erstellt werden müssen
- Spezielle Configuration nach dem Fetch nötig ist

### 7.2 Hook-Datei erstellen

**Path:** `cmake/externals/Hooks/PostFetch/{name}.cmake`

**Template (für Library ohne CMake):**
```cmake
# ==============================================================================
# PostFetch/{name}.cmake – {Name} PostFetch Hook
# ==============================================================================
#
# Hook:         {name}.cmake
# Version:      0.1.0
# Date:         {DATUM}
# Part of:      CMake Architecture
#
# Description:
#   PostFetch hook for {Name}.
#   Creates the library target manually.
#
# Provided Variables:
#   HOOK_EXTERNAL_NAME - Name of the external (use for target names!)
#   HOOK_SOURCE_DIR    - Path to source directory
#
# ==============================================================================

message(STATUS "[${HOOK_EXTERNAL_NAME}] Creating target from: ${HOOK_SOURCE_DIR}")

# ==============================================================================
# Collect Source Files
# ==============================================================================

set(_sources
    "${HOOK_SOURCE_DIR}/src/file1.cpp"
    "${HOOK_SOURCE_DIR}/src/file2.cpp"
)

set(_includes
    "${HOOK_SOURCE_DIR}/include"
)

# ==============================================================================
# Create Library Target
# ==============================================================================

add_library(${HOOK_EXTERNAL_NAME} STATIC ${_sources})

target_include_directories(${HOOK_EXTERNAL_NAME} PUBLIC ${_includes})

# C++ Standard (falls nötig)
target_compile_features(${HOOK_EXTERNAL_NAME} PUBLIC cxx_std_17)

# Suppress warnings in external code
if(MSVC)
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE /W0)
else()
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE -w)
endif()

# ==============================================================================
# Register Target
# ==============================================================================

_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)

message(STATUS "[${HOOK_EXTERNAL_NAME}] Target created")
message(STATUS "[${HOOK_EXTERNAL_NAME}] PostFetch complete")
```

**Template (für Header-only Library):**
```cmake
# ==============================================================================
# PostFetch/{name}.cmake – {Name} PostFetch Hook (Header-only)
# ==============================================================================

message(STATUS "[${HOOK_EXTERNAL_NAME}] Creating INTERFACE target")

# ==============================================================================
# Create Interface Library
# ==============================================================================

add_library(${HOOK_EXTERNAL_NAME} INTERFACE)

target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE 
    "${HOOK_SOURCE_DIR}/include"
)

# ==============================================================================
# Register Target
# ==============================================================================

_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)

message(STATUS "[${HOOK_EXTERNAL_NAME}] INTERFACE target created")
```

### 7.3 Importante Regeln

**Immer `${HOOK_EXTERNAL_NAME}` verwenden:**
```cmake
# ✅ Richtig
add_library(${HOOK_EXTERNAL_NAME} STATIC ${sources})

# ❌ Falsch
add_library(mylib STATIC ${sources})
```

**Immer Target registrieren:**
```cmake
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

**Warnings unterdrücken:**
```cmake
if(MSVC)
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE /W0)
else()
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE -w)
endif()
```

---

## 8. Executable konfigurieren

**Füge das External zur Executable hinzu:**

```json
{
    "executables": [
        {
            "name": "MyApp",
            "externals": ["mein_external"]
        }
    ]
}
```

**Reihenfolge beachten:**

Wenn Externals voneinander abhängen, müssen sie in der richtigen Reihenfolge stehen:

```json
"externals": ["glad", "glfw", "imgui"]
#              ^^^^   ^^^^   ^^^^^
#              1.     2.     3. (braucht 1 und 2)
```

---

## 9. Testen

### 9.1 Konfigurieren

```bash
cmake --preset msvc-debug
```

**Erwartete Ausgabe:**
```
[mein_external] PreFetch: Setting options
[mein_external] PreFetch complete
...
[mein_external] Fetching from https://github.com/...
[mein_external] Fetched and configured successfully
```

### 9.2 Bauen

```bash
cmake --build build/msvc-debug --target MyApp
```

### 9.3 Verifikation

**Checkliste nach erfolgreichem Build:**

- [ ] Configure ohne Error
- [ ] Build ohne Error
- [ ] Executable startet
- [ ] External-Funktionalität verfügbar

---

## 10. Stolpersteine und Lösungen

### 10.1 "Target not found"

**Problem:** CMake kann das External-Target nicht finden.

**Ursache:** PostFetch-Hook fehlt oder Target nicht registriert.

**Lösung:**
1. Bei `cmakeSupport: false` → PostFetch-Hook erstellen
2. `_register_external_target()` nicht vergessen
3. Target-Namen prüfen

### 10.2 "Undefined reference"

**Problem:** Linker findet Symbole nicht.

**Ursache:** Falscher Target-Name oder fehlende Source-Dateien.

**Lösung:**
1. Target-Namen in External's CMakeLists.txt prüfen
2. Alle Source-Dateien in PostFetch-Hook auflisten
3. Library-Typ prüfen (STATIC vs SHARED vs INTERFACE)

### 10.3 Tests werden trotzdem gebaut

**Problem:** External baut Tests obwohl deaktiviert.

**Ursache:** Falsche Cache-Variable oder FORCE fehlt.

**Lösung:**
```cmake
# ❌ Ohne FORCE - wird ignoriert wenn bereits gesetzt
set(BUILD_TESTS OFF CACHE BOOL "")

# ✅ Mit FORCE - überschreibt immer
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
```

### 10.4 CMake Error in External

**Problem:** Error in der CMakeLists.txt des Externals.

**Ursache:** Fehlende Abhängigkeit oder inkompatible CMake-Version.

**Lösung:**
1. CMakeLists.txt des Externals analysieren
2. Fehlende find_package() Aufrufe prüfen
3. Minimum-CMake-Version prüfen

### 10.5 Hook wird nicht ausgeführt

**Problem:** Hook-Datei existiert aber wird nicht geladen.

**Ursache:** Falscher Dateiname oder Pfad.

**Lösung:**
1. Dateiname muss exakt `{external_name}.cmake` sein
2. Path: `cmake/externals/Hooks/PreFetch/` oder `PostFetch/`
3. Groß-/Kleinschreibung beachten

---

## 11. Troubleshooting

### Checkliste neues External

- [ ] Repository URL und Tag identifiziert
- [ ] CMake Support geprüft
- [ ] Build-Optionen identifiziert
- [ ] Target-Name identifiziert
- [ ] Solution.json erweitert
- [ ] PreFetch Hook erstellt (falls nötig)
- [ ] PostFetch Hook erstellt (falls nötig)
- [ ] Executable konfiguriert
- [ ] Configure erfolgreich
- [ ] Build erfolgreich
- [ ] Linking erfolgreich

### Häufige Error

| Problem | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| "Target not found" | PostFetch Hook fehlt | Hook erstellen mit `cmakeSupport: false` |
| "Undefined reference" | Falscher Target-Name | Target-Namen in Library prüfen |
| Tests werden gebaut | PreFetch Hook fehlt/falsch | Options mit FORCE setzen |
| CMake Error in External | Falsche Options | CMakeLists.txt des Externals prüfen |
| Hook nicht ausgeführt | Falscher Pfad/Name | Dateiname und Pfad prüfen |

### Debug-Tips

```bash
# Verbose CMake-Output
cmake --preset ... --debug-output

# Hook-Ausführung prüfen
cmake --preset ... -DHOOKS_DEBUG=ON

# FetchContent Details
cmake --preset ... --log-level=DEBUG
```

---

## 12. See Also

- [Git_Externals_Reference.md](../references/externals/Git_Externals.md) – Externe Bibliotheken Overview
- [Solution_Schema.md](../references/Solution_Schema.md) – JSON Schema
- [Externals.md](../references/Externals.md) – Externals Reference
- [Externals_UserGuide.md](Externals.md) – Externals verwenden

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Konformität: Nummerierte TOC, Prerequisites, Stolpersteine/Troubleshooting getrennt, See Also** |
| 0.1.0 | 2025-12-10 | Initial: Komplette Anleitung mit Templates |
