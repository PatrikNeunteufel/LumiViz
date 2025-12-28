# phase3.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Pfad:** `cmake/buildSystemTest/phase3.cmake`  
> **Status:** Stabil  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Tests](#2-tests)
3. [Erfolgs-Flag](#3-erfolgs-flag)
4. [Siehe auch](#4-siehe-auch)
5. [Changelog](#5-changelog)

---

## 1. Übersicht

**Phase 3** testet die **Executable Pipeline** — das Erstellen von Executable-Targets.

| Aspekt | Beschreibung |
|--------|--------------|
| **Zweck** | Executable Pipeline Validation |
| **Debug-ID** | `PHASE3_TEST` |
| **Abhängigkeiten** | Phase 1-2, Executables.cmake |

---

## 2. Tests

### 2.1 Pipeline-Module geladen

| Funktion | Prüft |
|----------|-------|
| `_collect_executable` | ExecutableCollect.cmake geladen |
| `_create_executable_target` | ExecutableCreate.cmake geladen |

### 2.2 Executables aus Solution.json

| Prüfung | Beschreibung |
|---------|--------------|
| `executables` Array | Vorhanden in Solution.json |
| Anzahl | Wird gezählt und ausgegeben |
| Active vs Skipped | Skip-Flag wird geprüft |

### 2.3 Context mit Test-Daten

Testet `_collect_executable` mit synthetischem JSON:

```cmake
set(_test_exe_json "{
    \"name\": \"TestExe\",
    \"displayName\": \"Test Executable\",
    \"version\": \"1.0.0\",
    \"path\": \"test/path\",
    \"type\": \"CONSOLE\",
    \"skip\": false,
    \"pch\": { \"enabled\": true, \"header\": \"pch.h\" }
}")
```

| Feld | Erwartet |
|------|----------|
| `NAME` | TestExe |
| `DISPLAY_NAME` | Test Executable |
| `VERSION` | 1.0.0 |
| `TYPE` | CONSOLE |
| `SKIP` | FALSE |
| `PCH_ENABLED` | TRUE |

### 2.4 Default-Werte

Testet minimales JSON `{"name": "MinimalExe"}`:

| Feld | Default |
|------|---------|
| `PATH` | `projects/exec/MinimalExe/src` |
| `TYPE` | `SOLUTION_DEFAULT_EXECUTABLE_TYPE` oder CONSOLE |

### 2.5 Targets erstellt

Prüft, dass für jedes aktive Executable ein CMake-Target existiert:

```cmake
if(TARGET ${_exe_name})
    # OK
endif()
```

---

## 3. Erfolgs-Flag

```cmake
set(PHASE3_TEST_PASSED TRUE CACHE BOOL "Phase 3 Test passed" FORCE)
```

---

## 4. Siehe auch

- [Executables.md](../modules/project/Executables.md)
- [ExecutableCollect.md](../modules/project/ExecutableCollect.md)
- [ExecutableCreate.md](../modules/project/ExecutableCreate.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-05 | Initial |
