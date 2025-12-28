# ExecutableCreate.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [ExecutableCreate.md](../../en/modules/project/ExecutableCreate.md)  
> **Modul:** [`cmake/project/ExecutableCreate.cmake`](../../../../cmake/project/ExecutableCreate.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Verarbeitung](#4-verarbeitung)
5. [Plattform-spezifisches Verhalten](#5-plattform-spezifisches-verhalten)
6. [Fehlerbehandlung](#6-fehlerbehandlung)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `ExecutableCreate.cmake` Modul **erstellt CMake-Targets** aus einem vorbereiteten Context. Es ist für die eigentliche Target-Erstellung mit allen Konfigurationen zuständig.

### Kernidee

Der Context enthält alle Daten — Create erstellt daraus ein vollständig konfiguriertes Target.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Target-Erstellung | add_executable() mit korrektem Typ |
| Source-Sammlung | GLOB oder Source.cmake |
| Abhängigkeiten | Interne Libraries, Externals |
| Optionen | Defines, Compile/Link Options |
| Standards | Warnings, CompilerOptions, OutputDirs |

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Context.cmake | 0.5.0 | `ctx_get` |
| Errors.cmake | 0.5.0 | `cmake_fatal`, `cmake_warn` |
| Debug.cmake | 0.5.0 | `dbg` |
| OutputDirs.cmake | 0.5.0 | `setup_output_dirs` |
| Warnings.cmake | 0.5.0 | `apply_warnings` |
| CompilerOptions.cmake | 0.5.0 | `apply_compiler_options` |
| Orchestrator.cmake | 0.5.0 | `apply_external_to_target` |
| Json.cmake | 0.5.0 | `_json_has_key`, `_json_get_object` |

---

## 3. API-Referenz

### 3.1 _create_executable_target()

Erstellt ein CMake-Executable-Target aus dem Context.

```cmake
_create_executable_target(<CTX>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CTX` | String | ✓ | Context-Prefix (z.B. `EXE_0`) |

**Erwartete Context-Keys:**

| Key | Verwendung |
|-----|------------|
| NAME | Target-Name |
| PATH | Source-Verzeichnis |
| TYPE | CONSOLE, GUI, CLI, etc. |
| VERSION | Target-Version |
| PCH_ENABLED | Precompiled Headers |
| PCH_HEADER | PCH-Dateiname |
| DEPENDENCIES | Interne Libraries |
| EXTERNALS | Externe Dependencies |
| EXTERNAL_OPTIONS | Optionen pro External |
| DEFINES | Präprozessor-Definitionen |
| COMPILE_OPTIONS | Compiler-Flags |
| LINK_OPTIONS | Linker-Flags |

**Beispiel:**

```cmake
ctx_create(EXE_0)
_collect_executable("${_exe_json}" EXE_0)
_create_executable_target(EXE_0)
```

---

## 4. Verarbeitung

### 4.1 Ablauf

```
_create_executable_target(CTX)
    │
    ├── 1. Context-Daten lesen
    │
    ├── 2. Source-Verzeichnis validieren
    │   └── E001 wenn nicht existiert
    │
    ├── 3. Target erstellen
    │   ├── GUI → WIN32/MACOSX_BUNDLE
    │   └── Andere → Standard executable
    │
    ├── 4. Sources sammeln (GLOB)
    │   └── W101 wenn keine Sources
    │
    ├── 5. Include-Directories setzen
    │   └── + pch/ wenn vorhanden
    │
    ├── 6. PCH aktivieren (wenn enabled)
    │   └── W101 wenn Header nicht gefunden
    │
    ├── 7. Interne Dependencies linken
    │   └── E101 wenn Target nicht existiert
    │
    ├── 8. Externals anwenden
    │   ├── E010 wenn nicht definiert
    │   └── apply_external_to_target()
    │
    ├── 9. Defines/Compile/Link Options setzen
    │
    ├── 10. Standard-Module anwenden
    │   ├── apply_warnings()
    │   ├── apply_compiler_options()
    │   └── setup_output_dirs()
    │
    └── 11. Plattform-spezifisches
        ├── Windows GUI: APP_WINDOWS_GUI
        └── macOS GUI: Bundle-Properties
```

### 4.2 Source-Sammlung

Aktuell via GLOB:
```cmake
file(GLOB_RECURSE _sources
    "${_src_dir}/*.cpp"
    "${_src_dir}/*.cxx"
    "${_src_dir}/*.cc"
    "${_src_dir}/*.c"
)
```

**Hinweis:** Für explizite Kontrolle kann Source.cmake verwendet werden (abhängig von SOLUTION_SOURCE_MODE).

---

## 5. Plattform-spezifisches Verhalten

### 5.1 Windows GUI

```cmake
if(WIN32 AND TYPE == "GUI")
    add_executable(${name} WIN32)
    set_target_properties(${name} PROPERTIES WIN32_EXECUTABLE TRUE)
    target_compile_definitions(${name} PRIVATE APP_WINDOWS_GUI)
endif()
```

Das Define `APP_WINDOWS_GUI` ermöglicht WinMain als Entry-Point:

```cpp
#ifdef APP_WINDOWS_GUI
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main(__argc, __argv);
}
#else
int main(int argc, char* argv[]) { ... }
#endif
```

### 5.2 macOS GUI

```cmake
if(APPLE AND TYPE == "GUI")
    add_executable(${name} MACOSX_BUNDLE)
    set_target_properties(${name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.project.${name}"
        MACOSX_BUNDLE_BUNDLE_NAME "${display_name}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${version}"
    )
endif()
```

### 5.3 Linux/Unix

Keine speziellen Anpassungen für GUI-Typ.

---

## 6. Fehlerbehandlung

### 6.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | Source-Pfad existiert nicht | Pfad prüfen/anlegen |
| E010 | External nicht in centraler Definition | External zu Solution.json hinzufügen |
| E101 | Interne Dependency existiert nicht | Library vor Executable definieren |

### 6.2 Warnings

| Code | Bedingung | Empfehlung |
|------|-----------|------------|
| W101 | Keine Sources gefunden | Source-Dateien hinzufügen |
| W101 | PCH-Header nicht gefunden | Header-Datei erstellen |

---

## 7. Siehe auch

- [Executables.cmake](Executables.md) — Ruft _create_executable_target auf
- [ExecutableCollect.cmake](ExecutableCollect.md) — Befüllt den Context
- [Orchestrator.cmake](../externals/Orchestrator.md) — apply_external_to_target
- [OutputDirs.cmake](../core/OutputDirs.md) — Output-Verzeichnisse
- [CompilerOptions.cmake](../core/CompilerOptions.md) — Compiler-Optionen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.1** | **2025-12-17** | **collect_sources() Integration, SourceCollect.cmake Dependency** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung |
| 0.1.2 | 2025-12-09 | APP_WINDOWS_GUI Define für Windows GUI |
| 0.1.1 | 2025-12-07 | Externals-Integration via apply_external_to_target() |
| 0.1.0 | 2025-12-05 | Initial (Clean Start): Target-Erstellung aus Context |
