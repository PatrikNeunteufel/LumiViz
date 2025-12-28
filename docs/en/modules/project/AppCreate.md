# AppCreate.cmake — Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development  
> **Target Audience:** Build System Developers  
> **Module:** [cmake/project/AppCreate.cmake](../../../../cmake/project/AppCreate.cmake)  
> **Module Version:** 1.0.0  
> **Based on:** ModuleDoc v0.5  
> **Language:** English  
> **German:** [AppCreate.md](../../../en/modules/project/AppCreate.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Concept](#3-konzept)
4. [API-Reference](#4-api-referenz)
   - 4.1 [_create_app_core()](#41-_create_app_core)
   - 4.2 [_create_app_runner()](#42-_create_app_runner)
   - 4.3 [_create_app_tests()](#43-_create_app_tests)
   - 4.4 [_create_app_test_target()](#44-_create_app_test_target)
5. [Verzeichnisstruktur](#5-verzeichnisstruktur)
6. [PCH-Behandlung](#6-pch-behandlung)
7. [Usagesbeispiele](#7-verwendungsbeispiele)
8. [Errorbehandlung](#8-fehlerbehandlung)
9. [Best Practices](#9-best-practices)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Overview

Das `AppCreate`-Modul ist verantwortlich für die **Erstellung aller CMake-Targets** eines App-Containers. Es enthält drei Hauptfunktionen, die jeweils einen Teil des App-Containers erstellen.

### Zweck

- Erstellung der Core Library (`{AppName}.Core`)
- Erstellung des Runner Executable (`{AppName}`)
- Erstellung der Test Executables (`{AppName}.*Tests`)

### Generierte Targets

| Funktion | Target | Typ | Description |
|----------|--------|-----|--------------|
| `_create_app_core()` | `{AppName}.Core` | STATIC Library | Business-Logik |
| `_create_app_runner()` | `{AppName}` | Executable | Entry-Point mit main() |
| `_create_app_tests()` | `{AppName}.UnitTests` | Executable | Unit Tests |
| `_create_app_tests()` | `{AppName}.IntegrationTests` | Executable | Integration Tests |

---

## 2. Dependencies

| Abhängigkeit | Typ | Description |
|--------------|-----|--------------|
| CMake 3.19+ | System | JSON-Functions, FILE_SET |
| `Context.cmake` | Modul | Context-Daten lesen |
| `Errors.cmake` | Modul | Errorbehandlung |
| `Debug.cmake` | Modul | Debug-Ausgabe |
| `OutputDirs.cmake` | Modul | Ausgabeverzeichnisse |
| `Warnings.cmake` | Modul | Warning-Configuration |
| `CompilerOptions.cmake` | Modul | Compiler-Einstellungen |
| `Orchestrator.cmake` | Modul | External-Integration |

---

## 3. Concept

### 3.1 Target-Hierarchie

```
{AppName}.Core (STATIC Library)
       │
       │ PUBLIC link
       │
       ├──────────────────┬────────────────────┐
       │                  │                    │
       ▼                  ▼                    ▼
{AppName}          {AppName}.UnitTests  {AppName}.IntegrationTests
(Executable)       (Test Executable)    (Test Executable)
```

### 3.2 Dependency-Propagation

| Link-Typ | Von → Zu | Bedeutung |
|----------|----------|-----------|
| PUBLIC | Core.Dependencies → Core | Transitiv zu Runner/Tests |
| PUBLIC | Core.Externals → Core | Transitiv zu Runner/Tests |
| PUBLIC | Core.pch/ Include → Core | Transitiv zu Runner/Tests |
| PRIVATE | Runner.Externals → Runner | Nur für Runner |
| PRIVATE | Integration.Externals → Integration | Nur für Integration Tests |

### 3.3 Warum STATIC Library?

- **Testbarkeit:** Tests können gegen Core linken ohne main()-Konflikt
- **Compilation:** Einmalige Kompilierung, mehrfache Nutzung
- **Isolation:** Core enthält keine Entry-Point-Logik
- **PCH-Sharing:** Runner und Tests können PCH von Core wiederverwenden

---

## 4. API-Reference

### 4.1 _create_app_core()

```cmake
_create_app_core(CTX)
```

**Description:**  
Erstellt die `{AppName}.Core` STATIC Library mit allen Business-Logik-Sources.

**Parameters:**

| Parameters | Required | Description |
|-----------|---------|--------------|
| `CTX` | ✓ | Context-Prefix (z.B. `APP_0`) |

**Erwartete Context-Keys:**

| Key | Usage |
|-----|------------|
| `NAME` | Target-Name Basis |
| `PATH` | Basis-Verzeichnis |
| `VERSION` | Target-Version |
| `PCH_ENABLED` | Precompiled Headers aktivieren |
| `PCH_HEADER` | PCH Header-Dateiname |
| `PCH_PATH` | Optionaler Custom-Pfad für PCH |
| `CORE_DEPENDENCIES` | Interne Libraries |
| `CORE_EXTERNALS` | Externe Dependencies |
| `CORE_EXTERNAL_OPTIONS` | Per-External Optionen (JSON) |

**Erwartete Verzeichnisse:**

```
{PATH}/
├── include/    ← PUBLIC Headers (optional, W401 wenn fehlt)
├── src/        ← Implementation (Required, E403 wenn fehlt)
└── pch/        ← Precompiled Header (optional)
```

**Generiertes Target:**
- `{AppName}.Core` — STATIC Library

**Target Properties (gesetzt):**

| Property | Description |
|----------|--------------|
| `APP_PCH_ENABLED` | Boolean: PCH aktiv |
| `APP_PCH_PATH` | Pfad zum PCH-Header |

**Error:**
- `E402` — Pfad existiert nicht
- `E403` — Kein src/ Verzeichnis
- `E404` — Keine Sources in src/
- `E405` — Dependency nicht gefunden
- `E010` — External nicht definiert
- `W401` — Kein include/ Verzeichnis
- `W402` — PCH Header nicht gefunden

---

### 4.2 _create_app_runner()

```cmake
_create_app_runner(CTX)
```

**Description:**  
Erstellt das `{AppName}` Executable mit dem Entry-Point (main()). Verwendet **kein PCH** — main.cpp ist typisch minimal und profitiert kaum davon.

**Parameters:**

| Parameters | Required | Description |
|-----------|---------|--------------|
| `CTX` | ✓ | Context-Prefix (z.B. `APP_0`) |

**Erwartete Context-Keys:**

| Key | Usage |
|-----|------------|
| `NAME` | Target-Name |
| `PATH` | Basis-Verzeichnis |
| `VERSION` | Target-Version |
| `DISPLAY_NAME` | Anzeigename (macOS Bundle) |
| `RUNNER_TYPE` | CONSOLE oder WINDOW/GUI |
| `RUNNER_EXTERNALS` | Runner-spezifische Externals |
| `RUNNER_EXTERNAL_OPTIONS` | Per-External Optionen (JSON) |

**Erwartete Verzeichnisse:**

```
{PATH}/
└── main/    ← Entry-Point (Required, E406 wenn fehlt)
```

**Generiertes Target:**
- `{AppName}` — Executable (WIN32/MACOSX_BUNDLE für GUI)

**PCH-Verhalten:**
- Erbt `pch/` Include-Directory von Core (PUBLIC)
- Verwendet **kein** `target_precompile_headers()`
- `#include "pch.h"` ist in main.cpp **nicht erforderlich**

**Error:**
- `E406` — Kein main/ Verzeichnis
- `E407` — Keine Sources in main/
- `E010` — External nicht definiert

---

### 4.3 _create_app_tests()

```cmake
_create_app_tests(CTX)
```

**Description:**  
Erstellt Test Executables basierend auf der Tests-Configuration im Context.

**Parameters:**

| Parameters | Required | Description |
|-----------|---------|--------------|
| `CTX` | ✓ | Context-Prefix (z.B. `APP_0`) |

**Erwartete Context-Keys:**

| Key | Usage |
|-----|------------|
| `NAME` | App-Name für Target-Prefix |
| `PATH` | Basis-Verzeichnis |
| `TESTS_FRAMEWORK` | doctest, googletest, catch2 |
| `TESTS_UNIT_ENABLED` | Unit Tests erstellen |
| `TESTS_UNIT_TIMEOUT` | CTest Timeout |
| `TESTS_UNIT_LABELS` | CTest Labels |
| `TESTS_INTEGRATION_ENABLED` | Integration Tests erstellen |
| `TESTS_INTEGRATION_TIMEOUT` | CTest Timeout |
| `TESTS_INTEGRATION_LABELS` | CTest Labels |
| `TESTS_INTEGRATION_EXTERNALS` | Zusätzliche Externals |

**Erwartete Verzeichnisse:**

```
{PATH}/
└── tests/
    ├── unit/        ← Unit Test Sources
    └── integration/ ← Integration Test Sources
```

**Generierte Targets:**
- `{AppName}.UnitTests` — wenn `TESTS_UNIT_ENABLED`
- `{AppName}.IntegrationTests` — wenn `TESTS_INTEGRATION_ENABLED`

**Error:**
- `E301` — Unbekanntes Framework
- `E010` — Framework/External nicht definiert
- `W403` — Tests aktiviert aber Verzeichnis fehlt/leer

---

### 4.4 _create_app_test_target()

```cmake
_create_app_test_target(TARGET_NAME SRC_DIR CORE_TARGET FRAMEWORK TIMEOUT LABELS EXTRA_EXTERNALS APP_NAME)
```

**Description:**  
Interne Hilfsfunktion zur Erstellung eines einzelnen Test-Targets.

**Parameters:**

| Parameters | Description |
|-----------|--------------|
| `TARGET_NAME` | Name des Test-Targets |
| `SRC_DIR` | Verzeichnis mit Test-Sources |
| `CORE_TARGET` | Core Library zum Linken |
| `FRAMEWORK` | Test-Framework |
| `TIMEOUT` | CTest Timeout in Sekunden |
| `LABELS` | CTest Labels (Liste) |
| `EXTRA_EXTERNALS` | Zusätzliche Externals |
| `APP_NAME` | App-Name für IDE-Folder |

---

## 5. Verzeichnisstruktur

### 5.1 Vollständige App-Struktur

```
projects/apps/{AppName}/
├── include/                 ← PUBLIC Headers (Core)
│   └── Application.hpp
├── src/                     ← Implementation (Core)
│   └── Application.cpp
├── main/                    ← Entry Point (Runner)
│   └── main.cpp
├── pch/                     ← Precompiled Headers (optional)
│   └── pch.h
└── tests/                   ← Tests
    ├── unit/
    │   └── Application_Tests.cpp
    └── integration/
        └── Application_Integration_Tests.cpp
```

### 5.2 Include-Verzeichnisse

| Verzeichnis | Visibility | Zweck |
|-------------|------------|-------|
| `include/` | PUBLIC | Öffentliche Header für Runner/Tests |
| `pch/` | PUBLIC | PCH-Header für Runner/Tests |
| `src/` | PRIVATE | Nur wenn private Headers existieren |

### 5.3 Minimale App-Struktur

```
projects/apps/{AppName}/
├── src/
│   └── Logic.cpp
└── main/
    └── main.cpp
```

---

## 6. PCH-Behandlung

### 6.1 Aktivierung

PCH wird in Solution.json aktiviert:

```json
"apps": [{
    "name": "MyApp",
    "pch": {
        "enabled": true,
        "header": "pch.h"    // Optional, Default: "pch.h"
    }
}]
```

### 6.2 Suchpfad-Priorität

| Priorität | Pfad | Example |
|-----------|------|----------|
| 1 | `{PATH}/pch/{header}` | `projects/apps/MyApp/pch/pch.h` |
| 2 | `{PATH}/src/{header}` | `projects/apps/MyApp/src/pch.h` |
| 3 | `{PATH}/{header}` | `projects/apps/MyApp/pch.h` |

### 6.3 PCH nur für Core

**Importante Vereinfachung:** PCH wird **nur für Core** aktiviert, nicht für Runner oder Tests.

```
Core Target:
  ├── target_include_directories(PUBLIC ${pch_dir})   ← pch/ ist sichtbar
  └── target_precompile_headers(PRIVATE ${pch_path})  ← Kompiliert PCH

Runner:
  ├── Erbt pch/ include dir via PUBLIC link           ← Könnte pch.h finden
  └── KEIN target_precompile_headers()                ← Verwendet PCH nicht

Tests:
  ├── Erbt pch/ include dir via PUBLIC link           ← Könnte pch.h finden
  └── KEIN target_precompile_headers()                ← Verwendet PCH nicht
```

### 6.4 Warum kein PCH für Runner/Tests?

| Aspekt | Begründung |
|--------|------------|
| **Core wird nicht neu kompiliert** | Runner/Tests linken nur gegen die fertige `.lib` |
| **main.cpp ist minimal** | Typisch 20-30 Zeilen, PCH-Gewinn vernachlässigbar |
| **Tests haben Framework-Includes** | doctest/googletest dominieren die Kompilierzeit |
| **Einfachere Templates** | Kein `#include "pch.h"` in main.cpp und Tests nötig |
| **Weniger Errorquellen** | Kein "PCH not found" bei deaktiviertem PCH |

### 6.5 Warum pch/ trotzdem PUBLIC?

Das `pch/` Verzeichnis ist als **PUBLIC Include** konfiguriert, damit:
- `src/*.cpp` mit `#include "pch.h"` funktioniert
- Bei Bedarf Runner/Tests manuell `#include "pch.h"` nutzen könnten

### 6.6 Warum manuelles #include in Core nötig?

MSVC erfordert `#include "pch.h"` als erste Zeile in jeder `.cpp` die PCH nutzt. GCC/Clang können PCH via `-include` injizieren, aber für **Cross-Platform-Kompatibilität** ist das explizite Include erforderlich.

**Bei deaktiviertem PCH:** `#include "pch.h"` muss nur aus `src/*.cpp` entfernt werden (nicht aus main.cpp oder Tests, da diese es ohnehin nicht verwenden).

---

## 7. Usagesbeispiele

### 7.1 Core Library verwenden

```cpp
// main/main.cpp (kein PCH erforderlich)
#include "Application.hpp" // Aus include/ der Core Library

int main(int argc, char* argv[]) {
    MyApp::Application app;
    return app.run(argc, argv);
}
```

### 7.2 Unit Test schreiben

```cpp
// tests/unit/Application_Tests.cpp (kein PCH erforderlich)
#include <doctest/doctest.h>
#include "Application.hpp"        // Aus include/ der Core Library

TEST_CASE("Application functionality") {
    MyApp::Application app;
    CHECK(app.initialize() == true);
}
```

### 7.3 CTest ausführen

```bash
# Alle App-Tests
ctest -L AudioPlayer

# Nur Unit Tests
ctest -R "UnitTests"

# Mit Timeout-Override
ctest --timeout 60
```

---

## 8. Errorbehandlung

### 8.1 Core-Error (E4xx)

| Code | Funktion | Bedingung |
|------|----------|-----------|
| `E402` | `_create_app_core` | Pfad existiert nicht |
| `E403` | `_create_app_core` | Kein src/ Verzeichnis |
| `E404` | `_create_app_core` | Keine Sources in src/ |
| `E405` | `_create_app_core` | Dependency nicht gefunden |
| `E406` | `_create_app_runner` | Kein main/ Verzeichnis |
| `E407` | `_create_app_runner` | Keine Sources in main/ |

### 8.2 Allgemeine Error

| Code | Bedingung |
|------|-----------|
| `E010` | External nicht in externals-Block definiert |
| `E301` | Unbekanntes Test-Framework |

### 8.3 Warningen (W4xx)

| Code | Funktion | Bedingung |
|------|----------|-----------|
| `W401` | `_create_app_core` | Kein include/ Verzeichnis |
| `W402` | `_create_app_core` | PCH Header nicht gefunden |
| `W403` | `_create_app_tests` | Tests aktiviert aber keine Sources |

---

## 9. Best Practices

### 9.1 Do's

| Empfehlung | Begründung |
|------------|------------|
| Headers in include/, Implementation in src/ | Klare Trennung |
| main.cpp minimal halten | Logik gehört in Core |
| PCH für Standard-Includes | Reduziert Build-Zeit |
| Framework in externals definieren | Zentrale Verwaltung |
| Labels für Tests setzen | Einfaches Filtern |

### 9.2 Don'ts

| Vermeiden | Grund |
|-----------|-------|
| Business-Logik in main/ | Nicht testbar |
| Tests ohne Framework-External | E010 Error |
| Große main.cpp | Verletzt App-Container-Prinzip |
| PCH in Core ohne #include | MSVC-Error in src/*.cpp |

### 9.3 Example: Gute vs. Schlechte Struktur

**Gut:**
```cpp
// main/main.cpp (minimal, kein PCH nötig)
#include "Application.hpp"
int main() { return MyApp::run(); }

// src/Application.cpp (testbar, mit PCH)
#include "pch.h"
#include "Application.hpp"
namespace MyApp {
    int run() { /* Business Logic */ }
}
```

**Schlecht:**
```cpp
// main/main.cpp (zu viel Logik)
int main() {
    // 500 Zeilen Business-Logik...
}
```

---

## 10. See Also

- [Apps.cmake](Apps.md) — Orchestrator
- [AppCollect.cmake](AppCollect.md) — JSON-Parsing
- [ExecutableCreate.cmake](ExecutableCreate.md) — Ähnliches Pattern
- [TestCreate.cmake](TestCreate.md) — Test-Pattern
- [AppContainer_Concept.md](../../projects/buildsystem/concepts/AppContainer_Concept.md) — Concept
- [ErrorCodes.md](../../references/ErrorCodes.md) — E4xx, W4xx Codes

---

## 11. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.7.0** | **2025-12-20** | **CORE_EXTERNAL_OPTIONS und RUNNER_EXTERNAL_OPTIONS: apply_external_to_target() mit Optionen** |
| 0.5.4 | 2025-12-18 | PCH vereinfacht: Nur für Core, nicht für Runner/Tests (kein REUSE_FROM mehr) |
| 0.5.3 | 2025-12-18 | PCH-Include-Directory als PUBLIC hinzugefügt, APP_PCH_* Target Properties |
| 0.5.2 | 2025-12-18 | PCH 3-tier search, implicit activation |
| 0.5.1 | 2025-12-17 | collect_sources() Integration, SourceCollect.cmake Dependency |
| 0.5.0 | 2025-12-17 | Initial: Phase 8 App-Container Target-Erstellung |
