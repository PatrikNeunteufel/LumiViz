# TestCreate.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [TestCreate.md](../../en/modules/project/TestCreate.md)  
> **Modul:** [`cmake/project/TestCreate.cmake`](../../../../cmake/project/TestCreate.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Verarbeitung](#4-verarbeitung)
5. [Framework-Integration](#5-framework-integration)
6. [CTest-Registrierung](#6-ctest-registrierung)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Das `TestCreate.cmake` Modul **erstellt Test-Targets** aus einem vorbereiteten Context. Es behandelt Framework-Integration, CTest-Registrierung und Dependencies.

### Kernidee

Analog zu ExecutableCreate — aber mit Test-spezifischer Logik (Framework, CTest).

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Target-Erstellung | add_executable() für Test |
| Framework | doctest, catch2, gtest Integration |
| CTest | add_test(), Labels, Timeout |
| Dependencies | Target, Libraries, Externals |

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Context.cmake | 0.5.0 | `ctx_get` |
| Errors.cmake | 0.5.0 | `cmake_fatal`, `cmake_warn` |
| Debug.cmake | 0.5.0 | `dbg` |
| Warnings.cmake | 0.5.0 | `apply_warnings` |
| CompilerOptions.cmake | 0.5.0 | `apply_compiler_options` |
| OutputDirs.cmake | 0.5.0 | `setup_output_dirs` |
| Orchestrator.cmake | 0.5.0 | `apply_external_to_target` |

---

## 3. API-Referenz

### 3.1 _create_test_target()

Erstellt ein CMake-Test-Target aus dem Context.

```cmake
_create_test_target(<CTX>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `CTX` | String | ✓ | Context-Prefix (z.B. `TEST_0`) |

**Erwartete Context-Keys:**

| Key | Verwendung |
|-----|------------|
| NAME | Test-Name |
| PATH | Source-Verzeichnis |
| FRAMEWORK | doctest, catch2, gtest |
| TARGET | Zu testendes Target |
| DEPENDENCIES | Interne Libraries |
| EXTERNALS | Externe Dependencies |
| TIMEOUT | CTest Timeout |
| LABELS | CTest Labels |
| PARALLEL | Parallel-Flag |

---

## 4. Verarbeitung

### 4.1 Ablauf

```
_create_test_target(CTX)
    │
    ├── 1. Context-Daten lesen
    │
    ├── 2. Source-Verzeichnis validieren
    │
    ├── 3. Test-Executable erstellen
    │   └── add_executable(${name} ${sources})
    │
    ├── 4. Framework-External linken
    │   └── apply_external_to_target(name, framework)
    │
    ├── 5. Target-Under-Test linken
    │   └── target_link_libraries(name PRIVATE target)
    │
    ├── 6. Dependencies linken
    │
    ├── 7. Andere Externals linken
    │
    ├── 8. Standard-Module anwenden
    │   ├── apply_warnings()
    │   ├── apply_compiler_options()
    │   └── setup_output_dirs()
    │
    └── 9. CTest registrieren
        ├── add_test(NAME name COMMAND name)
        ├── set_tests_properties(TIMEOUT)
        ├── set_tests_properties(LABELS)
        └── set_tests_properties(RUN_SERIAL) wenn !parallel
```

---

## 5. Framework-Integration

### 5.1 doctest (Default)

```json
{
    "name": "MyTests",
    "framework": "doctest"
}
```

doctest muss als External definiert sein.

### 5.2 Catch2

```json
{
    "framework": "catch2"
}
```

### 5.3 Google Test

```json
{
    "framework": "gtest"
}
```

---

## 6. CTest-Registrierung

### 6.1 Basis-Registrierung

```cmake
add_test(
    NAME ${_name}
    COMMAND ${_name}
)
```

### 6.2 Properties

```cmake
set_tests_properties(${_name} PROPERTIES
    TIMEOUT ${_timeout}
    LABELS "${_labels}"
)

# Wenn nicht parallel
if(NOT _parallel)
    set_tests_properties(${_name} PROPERTIES
        RUN_SERIAL TRUE
    )
endif()
```

### 6.3 Tests ausführen

```bash
ctest --test-dir build              # Alle Tests
ctest -L unit                        # Nur Unit-Tests
ctest -R "Core"                      # Nach Name filtern
ctest --output-on-failure           # Output bei Fehlern
```

---

## 7. Fehlerbehandlung

### 7.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | Source-Pfad existiert nicht | Pfad prüfen/anlegen |
| E010 | Framework-External nicht definiert | External hinzufügen |
| E101 | Dependency existiert nicht | Target-Reihenfolge prüfen |

### 7.2 Warnings

| Code | Bedingung | Empfehlung |
|------|-----------|------------|
| W101 | Keine Sources gefunden | Test-Dateien hinzufügen |

---

## 8. Siehe auch

- [Tests.cmake](Tests.md) — Ruft _create_test_target auf
- [TestCollect.cmake](TestCollect.md) — Befüllt den Context
- [ExecutableCreate.cmake](ExecutableCreate.md) — Analoges Modul
- [Testing_UserGuide.md](../../../guides/Testing_UserGuide.md) — Benutzerhandbuch

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.1** | **2025-12-17** | **collect_sources() Integration, SourceCollect.cmake Dependency** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0 |
| 0.1.0 | 2025-12-11 | Initial (Clean Start): Test-Target-Erstellung mit CTest |
