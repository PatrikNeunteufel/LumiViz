# OutputDirs.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [OutputDirs.md](../../en/modules/core/OutputDirs.md)  
> **Modul:** [`cmake/core/OutputDirs.cmake`](../../../../cmake/core/OutputDirs.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Output-Struktur](#5-output-struktur)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `OutputDirs.cmake` Modul konfiguriert **standardisierte Output-Verzeichnisse** für alle Targets. Es sorgt für eine einheitliche, isolierte Verzeichnisstruktur.

### Kernidee

Jedes Target bekommt sein eigenes Unterverzeichnis für bessere Isolation und einfachere Deployment-Vorbereitung.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Kategorisierung | Automatische Erkennung (EXECUTABLE vs LIBRARY) |
| Isolation | Eigenes Verzeichnis pro Target |
| Konfigurationen | Debug/Release/Testing Unterverzeichnisse |

### Verwendung durch

- ExecutableCreate.cmake
- LibraryCreate.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| — | — | Keine (Standalone-Modul) |

**Design-Entscheidung:** OutputDirs.cmake ist ein einfaches Standalone-Modul ohne Debug- oder Error-Integration. Fehler (z.B. Target existiert nicht) werden direkt von CMake gemeldet.

→ Siehe [Offene Abklärungen](../../../projects/buildsystem/concepts/Offene_Abklaerungen_Core_Module.md) für Details zu möglicher dbg()-Integration.

---

## 3. Konzept

### 3.1 Target-Kategorien

| Target-Typ | Kategorie | Pfad |
|------------|-----------|------|
| EXECUTABLE | exec | `exec/${TARGET}/bin/` |
| STATIC_LIBRARY | libs | `libs/${TARGET}/lib/` |
| SHARED_LIBRARY | libs | `libs/${TARGET}/lib/` |
| MODULE_LIBRARY | libs | `libs/${TARGET}/lib/` |
| OBJECT_LIBRARY | libs | `libs/${TARGET}/lib/` |
| INTERFACE_LIBRARY | libs | `libs/${TARGET}/lib/` |
| (Unbekannt) | other | `other/${TARGET}/` |

### 3.2 Gesetzte Properties

| Property | Verzeichnis | Beschreibung |
|----------|-------------|--------------|
| `RUNTIME_OUTPUT_DIRECTORY` | bin/ | Executables, DLLs |
| `LIBRARY_OUTPUT_DIRECTORY` | lib/ | Shared Libraries (.so) |
| `ARCHIVE_OUTPUT_DIRECTORY` | lib/ | Static Libraries (.a, .lib) |

Für jede Property werden zusätzlich konfigurationsspezifische Varianten gesetzt:
- `*_DEBUG`
- `*_RELEASE`
- `*_TESTING`

---

## 4. API-Referenz

### 4.1 setup_output_dirs()

Konfiguriert standardisierte Output-Verzeichnisse für ein Target.

```cmake
setup_output_dirs(<TARGET_NAME>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `TARGET_NAME` | String | ✓ | CMake Target (muss bereits existieren) |

**Voraussetzung:** Das Target muss bereits mit `add_executable()` oder `add_library()` erstellt worden sein.

**Rückgabe:** Keine (setzt Target-Properties)

**Beispiel:**

```cmake
add_executable(MyApp main.cpp)
setup_output_dirs(MyApp)
# → build/exec/MyApp/bin/Debug/MyApp.exe

add_library(CoreLib STATIC core.cpp)
setup_output_dirs(CoreLib)
# → build/libs/CoreLib/lib/Debug/CoreLib.lib
```

---

## 5. Output-Struktur

### 5.1 Vollständiges Beispiel

```
out/build/preset-name/
├── exec/
│   ├── MyApp/
│   │   └── bin/
│   │       ├── Debug/
│   │       │   ├── MyApp.exe
│   │       │   └── required.dll
│   │       └── Release/
│   │           └── MyApp.exe
│   └── OtherApp/
│       └── bin/
│           └── Debug/
│               └── OtherApp.exe
└── libs/
    ├── CoreLib/
    │   └── lib/
    │       ├── Debug/
    │       │   └── CoreLib.lib
    │       └── Release/
    │           └── CoreLib.lib
    └── PluginLib/
        └── lib/
            └── Debug/
                ├── PluginLib.lib    (Import-Library)
                └── PluginLib.dll    (via RUNTIME)
```

### 5.2 Plattform-spezifische Ausgaben

| Plattform | Executable | Static Lib | Shared Lib |
|-----------|------------|------------|------------|
| Windows | .exe → bin/ | .lib → lib/ | .dll → bin/, .lib → lib/ |
| Linux | (keine Ext.) → bin/ | .a → lib/ | .so → lib/ |
| macOS | (keine Ext.) → bin/ | .a → lib/ | .dylib → lib/ |

---

## 6. Verwendungsbeispiele

### 6.1 In ExecutableCreate.cmake

```cmake
function(_create_executable_target CTX)
    ctx_get(${CTX} NAME _name)
    
    add_executable(${_name} ${_sources})
    setup_output_dirs(${_name})    # ← Output-Verzeichnisse
    apply_warnings(${_name})
    apply_compiler_options(${_name})
endfunction()
```

### 6.2 In LibraryCreate.cmake

```cmake
function(_create_library_target CTX)
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} TYPE _type)
    
    add_library(${_name} ${_type} ${_sources})
    setup_output_dirs(${_name})    # ← Output-Verzeichnisse
endfunction()
```

### 6.3 Standalone-Verwendung

```cmake
add_executable(MyTool tool.cpp)
setup_output_dirs(MyTool)

add_library(HelperLib STATIC helper.cpp)
setup_output_dirs(HelperLib)
```

---

## 7. Siehe auch

- [ExecutableCreate.cmake](../project/ExecutableCreate.md) — Verwendet setup_output_dirs
- [LibraryCreate.cmake](../project/LibraryCreate.md) — Verwendet setup_output_dirs
- [CMake OUTPUT_DIRECTORY](https://cmake.org/cmake/help/latest/prop_tgt/RUNTIME_OUTPUT_DIRECTORY.html) — CMake-Dokumentation

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.3 | 2025-12-07 | Target-isolierte Verzeichnisse, exec/libs Trennung, automatische Typ-Erkennung |
| 0.1.2 | 2025-12-05 | TESTING Konfiguration hinzugefügt |
| 0.1.1 | 2025-12-04 | Debug/Release Unterverzeichnisse |
| 0.1.0 | 2025-12-03 | Initial (Clean Start) |
