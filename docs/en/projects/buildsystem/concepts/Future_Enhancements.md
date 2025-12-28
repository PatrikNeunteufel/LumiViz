# Future Enhancements — Geplante Erweiterungen

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Concept  
> **Status:** Sammlung  
> **Target Audience:** Build System Developers, Architekten  
> **Language:** English  
> **German:** [Future_Enhancements.md](../../en/projects/buildsystem/concepts/Future_Enhancements.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [App-Container Erweiterungen](#2-app-container-erweiterungen)
3. [Externals-Erweiterungen](#3-externals-erweiterungen)
4. [Settings-Erweiterungen](#4-settings-erweiterungen)
5. [Executable-Erweiterungen](#5-executable-erweiterungen)
6. [Test-Erweiterungen](#6-test-erweiterungen)
7. [Build-System](#7-build-system)
8. [Tooling](#8-tooling)
9. [Projekt-Struktur](#9-projekt-struktur)
10. [CI/CD](#10-cicd)
11. [Offene Entscheidungen](#11-offene-entscheidungen)
12. [See Also](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Overview

Dieses Dokument sammelt mögliche **zukünftige Erweiterungen** für das CMake Architecture Build-System. Die Einträge sind nach Bereich gruppiert und mit Priorität/Komplexität bewertet.

> **Note:** Bereits beschlossene Features mit eigenem Concept-Dokument sind hier nicht aufgeführt. Siehe [Abschnitt 12 (See Also)](#12-siehe-auch) für fertige Concepte.

### 1.1 Legende

| Priorität | Bedeutung |
|-----------|-----------|
| 🔴 Hoch | Important für Produktivität/Usability |
| 🟡 Mittel | Nice-to-have, geplant |
| 🟢 Niedrig | Langfristig, bei Bedarf |

| Komplexität | Bedeutung |
|-------------|-----------|
| ⚪ Einfach | < 1 Tag Aufwand |
| 🔵 Mittel | 1-3 Tage Aufwand |
| 🟣 Komplex | > 3 Tage Aufwand |

---

## 2. App-Container Erweiterungen

> **Basis-Concept:** Siehe [AppContainer_Concept.md](AppContainer_Concept.md)

### 2.1 Mehrere Runner pro App

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel  
> **Wann relevant:** CLI + GUI für dieselbe App

**Description:**  
Ein App-Container mit mehreren Entry Points (z.B. GUI und CLI).

**Mögliche Struktur:**
```
projects/apps/AudioPlayer/
├── core/...
└── runners/
    ├── gui/main.cpp       → AudioPlayer.Gui
    └── cli/main.cpp       → AudioPlayer.Cli
```

---

### 2.2 Shared Core Library Option

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel  
> **Wann relevant:** Plugin-Architectureen, DLL-basierte Apps

**Description:**  
Option für `core.type: "SHARED"` statt nur STATIC.

---

## 3. Externals-Erweiterungen

> **Basis-Concept:** Siehe [System_Externals_Concept.md](System_Externals_Concept.md) für System-Pakete

### 3.1 externalsPolicy (JSON-Configuration)

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel  
> **Wann relevant:** Verschiedene Projekte mit unterschiedlichen Fetch-Strategien

**Description:**  
Optionale Configuration in Solution.json für explizite Kontrolle über das Fetch-Verhalten.

**Mögliche Felder:**
```json
{
    "externalsPolicy": {
        "fetchRoot": ".externals",
        "updatePolicy": "checkout",
        "lockfile": true
    }
}
```

| Feld | Default | Optionen |
|------|---------|----------|
| `fetchRoot` | `.externals` | Beliebiger Pfad |
| `updatePolicy` | `checkout` | `checkout`, `always`, `never`, `update`, `locked` |
| `lockfile` | `false` | `true`, `false` |

**updatePolicy Optionen:**

| Policy | Verhalten |
|--------|-----------|
| `checkout` | Fetch nur wenn nicht vorhanden (aktuelles Verhalten) |
| `always` | Immer neu fetchen |
| `never` | Nie fetchen, nur existierende nutzen |
| `update` | Fetch + git pull bei jedem Configure |
| `locked` | Exakte Commits aus Lockfile verwenden |

---

### 3.2 Lockfile-System

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🟣 Komplex  
> **Wann relevant:** Reproduzierbare Builds, Team-Entwicklung, CI/CD

**Description:**  
Automatisches Tracking welche Versionen (Commit-Hashes) tatsächlich verwendet werden.

**Format (externals.lock.json):**
```json
{
    "version": "1.0",
    "generated": "2025-12-10T10:30:00Z",
    "externals": {
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4",
            "resolved_commit": "7b6aead9fb88b3623e3b3725ebb42670cfe4c5f9",
            "fetched_at": "2025-12-10T10:30:00Z"
        }
    }
}
```

**Geplante CLI-Befehle:**
```bash
cmake --lockfile-update     # Lockfile aktualisieren
cmake --lockfile-verify     # Lockfile gegen Solution.json prüfen
cmake --lockfile-freeze     # Alle auf resolved_commit fixieren
```

**Vorteile:**
- Exakte Reproduzierbarkeit
- Audit-Trail für Dependency-Updates
- Erkennung von unbeabsichtigten Changes
- CI/CD kann identische Builds garantieren

---

### 3.3 vcpkg Integration

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🟣 Komplex  
> **Wann relevant:** Nutzung von vcpkg-Paketen

**Description:**  
Unterstützung für vcpkg als Alternative zu Git-Externals.

**Mögliche Syntax:**
```json
{
    "externals": {
        "fmt": {
            "vcpkg": "fmt",
            "version": "10.1.1"
        },
        "boost-asio": {
            "vcpkg": "boost-asio",
            "features": ["ssl"]
        }
    }
}
```

---

### 3.4 Conan Integration

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🟣 Komplex  
> **Wann relevant:** Conan als Package Manager bevorzugt

**Description:**  
Ähnlich wie vcpkg, aber für Conan Package Manager.

**Mögliche Syntax:**
```json
{
    "externals": {
        "spdlog": {
            "conan": "spdlog/1.12.0"
        }
    }
}
```

---

### 3.5 Submodule-Support

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** ⚪ Einfach  
> **Wann relevant:** Projekt verwendet Git Submodules

**Description:**  
Automatische Erkennung/Initialisierung von Git Submodules.

**Mögliche Syntax:**
```json
{
    "externals": {
        "mylib": {
            "submodule": "libs/mylib"
        }
    }
}
```

---

## 4. Settings-Erweiterungen

### 4.1 Output-Configuration

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Zentrale Configuration der Output-Verzeichnisse.

```json
{
    "settings": {
        "output": {
            "bin_dir": "bin",
            "lib_dir": "lib",
            "archive_dir": "lib"
        }
    }
}
```

---

### 4.2 Quality-Einstellungen

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Zentrale Code-Qualitäts-Steuerung.

```json
{
    "settings": {
        "quality": {
            "clang_tidy": true,
            "clang_format": true,
            "warnings_as_errors": false,
            "sanitizers": ["address", "undefined"]
        }
    }
}
```

---

### 4.3 Packaging-Metadaten

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** ⚪ Einfach

**Description:**  
CPack-Metadaten für Installer-Generierung.

```json
{
    "settings": {
        "packaging": {
            "vendor": "My Company",
            "contact": "support@example.com",
            "license": "MIT",
            "homepage": "https://example.com"
        }
    }
}
```

---

### 4.4 Platform-Requirements

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🟣 Komplex

**Description:**  
Minimum-Plattformversionen definieren.

```json
{
    "settings": {
        "platform": {
            "min_windows_version": "10.0.19041",
            "min_macos_version": "11.0",
            "min_glibc_version": "2.31"
        }
    }
}
```

---

## 5. Executable-Erweiterungen

### 5.1 Zusätzliche Executable-Types

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵-🟣 Variiert

| Type | Description | Komplexität |
|------|--------------|-------------|
| `TRAY` | System Tray App | 🔵 Mittel |
| `SERVICE` | OS Service/Daemon | 🟣 Komplex |
| `BENCHMARK` | Benchmark Runner | 🔵 Mittel |
| `BUNDLE` | macOS App Bundle | 🔵 Mittel |

---

### 5.2 Custom Entry Point

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** ⚪ Einfach

**Description:**  
Custom Entry-Point Funktion (WinMain, DllMain).

```json
{
    "executables": [{
        "name": "MyApp",
        "entrypoint": "WinMain"
    }]
}
```

---

### 5.3 Resources

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Win32 .rc Dateien, macOS .icns/Info.plist Integration.

```json
{
    "executables": [{
        "name": "MyApp",
        "resources": {
            "windows": "res/app.rc",
            "macos": {
                "icon": "res/app.icns",
                "plist": "res/Info.plist"
            }
        }
    }]
}
```

---

### 5.4 Visibility & Install

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🟣 Komplex

**Description:**  
Packaging/Export Steuerung für Targets.

```json
{
    "executables": [{
        "name": "MyApp",
        "visibility": "public",
        "install": {
            "destination": "bin",
            "component": "runtime"
        }
    }]
}
```

---

## 6. Test-Erweiterungen

### 6.1 Test Fixtures

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🟣 Komplex

**Description:**  
CTest Fixtures für Setup/Cleanup/Requires.

```json
{
    "tests": [{
        "name": "IntegrationTests",
        "fixtures": {
            "setup": "StartDatabase",
            "cleanup": "StopDatabase",
            "requires": ["DatabaseRunning"]
        }
    }]
}
```

---

### 6.2 Parametersized Tests

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel

**Description:**  
Tests mit verschiedenen Parametersn ausführen.

```json
{
    "tests": [{
        "name": "CrossPlatformTests",
        "parameters": {
            "backend": ["opengl", "vulkan", "d3d12"]
        }
    }]
}
```

---

## 7. Build-System

### 7.1 Unity Builds

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel  
> **Wann relevant:** Extreme Compile-Zeit-Optimierung

**Description:**  
CMake UNITY_BUILD Support für schnellere Builds.

```json
{
    "settings": {
        "unity_build": {
            "enabled": true,
            "batch_size": 16
        }
    }
}
```

---

### 7.2 C++20 Modules

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🟣 Komplex  
> **Wann relevant:** C++20 Module-Adoption

**Description:**  
Native Unterstützung für C++20 Modules (.ixx, .cppm).

---

### 7.3 Cross-Compilation Presets

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel  
> **Wann relevant:** Embedded, Mobile, WebAssembly

**Description:**  
Vordefinierte Presets für Cross-Compilation.

---

## 8. Tooling

### 8.1 clang-format Auto-Fix

> **Priorität:** 🔴 Hoch  
> **Komplexität:** ⚪ Einfach  
> **Wann relevant:** Automatische Code-Formatierung

**Description:**  
Build-Target für automatische Formatierung (nicht nur Check).

```bash
cmake --build . --target format-fix
```

---

### 8.2 clang-tidy Auto-Fix

> **Priorität:** 🟡 Mittel  
> **Komplexität:** ⚪ Einfach

**Description:**  
Build-Target für automatische clang-tidy Fixes.

```bash
cmake --build . --target tidy-fix
```

---

### 8.3 Documentation Generation (Doxygen)

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Automatische Doxygen-Configuration und Build-Target.

```json
{
    "documentation": {
        "doxygen": {
            "enabled": true,
            "output": "docs/api",
            "exclude": ["tests/", "externals/"]
        }
    }
}
```

---

### 8.4 Code Coverage

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Automatische Coverage-Instrumentierung und Report-Generierung.

```bash
cmake --build . --target coverage
```

---

## 9. Projekt-Struktur

### 9.1 Multi-Solution Support

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🟣 Komplex  
> **Wann relevant:** Monorepo mit mehreren unabhängigen Projekten

**Description:**  
Unterstützung für mehrere Solution.json in einem Repository.

---

### 9.2 Solution Templates

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel

**Description:**  
Vordefinierte Templates für verschiedene Projekt-Typen.

```bash
cmake --init-solution --template console-app
cmake --init-solution --template gui-app
cmake --init-solution --template library
```

---

### 9.3 Workspace-Support

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🟣 Komplex

**Description:**  
Übergeordnete Workspace-Datei die mehrere Solutions orchestriert.

---

## 10. CI/CD

### 10.1 GitHub Actions Generator

> **Priorität:** 🟡 Mittel  
> **Komplexität:** 🔵 Mittel

**Description:**  
Automatische Generierung von GitHub Actions Workflows basierend auf Presets.

```bash
cmake --generate-ci github
```

---

### 10.2 GitLab CI Generator

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel

---

### 10.3 Azure DevOps Generator

> **Priorität:** 🟢 Niedrig  
> **Komplexität:** 🔵 Mittel

---

## 11. Offene Entscheidungen

| Thema | Status | Optionen | Tendenz |
|-------|--------|----------|---------|
| Git-Strategie für Lockfile | Offen | Committen vs. .gitignore | Committen |
| Default Test Framework | Offen | doctest vs googletest | doctest |

---

## 12. See Also

**Fertige Concepte (nicht in diesem Dokument):**
- [AppContainer_Concept.md](AppContainer_Concept.md) — Phase 8: Testbare App-Architecture
- [System_Externals_Concept.md](System_Externals_Concept.md) — Phase 9: System-Pakete (Qt6, Boost, etc.)


**Architecture:**
- [master_concept.md](master_concept.md) — Architecture-Overview
- [implementation_plan.md](implementation_plan.md) — Phasen 1-7

---

## 13. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 (Concept), bereits beschlossene Concepte entfernt (App-Container Basis, System-Externals Basis), Verweis auf separate Concept-Dokumente** |
| 0.2.0 | 2025-12-12 | App-Container (Phase 8), Lockfile-Details, Settings-Erweiterungen |
| 0.1.0 | 2025-12-10 | Initial: externalsPolicy, Lockfile, vcpkg, Conan, Tooling |
