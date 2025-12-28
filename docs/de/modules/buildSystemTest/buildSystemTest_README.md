# Build-System-Tests — Phasen-Testdokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Sprache:** Deutsch  
> **English:** [README.md](../../../en/modules/buildSystemTest/README.md)

---

## Quick-Start

**Build-System testen?**
```bash
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=ON
cmake -B build -DTEST_PHASE=8  # Nur Phase 8
```

**Einzelne Phase verstehen?**
- [Phase1_doc.md](Phase1_doc.md) — Core-Module
- [Phase8_doc.md](Phase8_doc.md) — App-Container (neu)

---

## Übersicht

Die Build-System-Tests validieren die korrekte Funktion aller CMake-Module. Jede Phase testet einen spezifischen Teil der Pipeline.

### Test-Architektur

```
CMakeLists.txt
     │
     └── if(RUN_BUILD_SYSTEM_TESTS)
             │
             ├── Phase1.cmake  → Core-Module
             ├── Phase2.cmake  → Solution
             ├── Phase3.cmake  → Executables
             ├── Phase4.cmake  → Libraries
             ├── Phase5.cmake  → Local Externals
             ├── Phase6.cmake  → Git Externals
             ├── Phase7.cmake  → Tests
             └── Phase8.cmake  → App-Container (neu)
```

---

## Dateien

| Datei | Phase | Beschreibung |
|-------|-------|--------------|
| [Phase1_doc.md](Phase1_doc.md) | 1 | Core-Module (Errors, Debug, Context, Json, etc.) |
| [Phase2_doc.md](Phase2_doc.md) | 2 | Solution.cmake, JSON-Loading |
| [Phase3_doc.md](Phase3_doc.md) | 3 | Executable-Pipeline |
| [Phase4_doc.md](Phase4_doc.md) | 4 | Library-Pipeline |
| [Phase5_doc.md](Phase5_doc.md) | 5 | Local Externals |
| [Phase6_doc.md](Phase6_doc.md) | 6 | Git Externals + Hooks |
| [Phase7_doc.md](Phase7_doc.md) | 7 | Test-Pipeline |
| [Phase8_doc.md](Phase8_doc.md) | 8 | App-Container-Pipeline |

---

## Phasen-Übersicht

| Phase | Status | Module | Tests |
|-------|--------|--------|-------|
| **1** | ✅ Stabil | Errors, Debug, Context, Json, Validation, OutputDirs, Warnings, CompilerOptions, SourceCollect | Modul-Verfügbarkeit, Funktions-Tests |
| **2** | ✅ Stabil | Solution | JSON-Loading, Properties |
| **3** | ✅ Stabil | Executables, ExecutableCollect, ExecutableCreate | Target-Erstellung, Linking |
| **4** | ✅ Stabil | Libraries, LibraryCollect, LibraryCreate | STATIC/SHARED, Dependencies |
| **5** | ✅ Stabil | Externals (Local) | Local-Attach, Include-Files |
| **6** | ✅ Stabil | Externals (Git) | FetchContent, Hooks, Caching |
| **7** | ✅ Stabil | Tests, TestCollect, TestCreate | Framework-Integration, CTest |
| **8** | ✅ In Entwicklung | Apps, AppCollect, AppCreate | Core/Runner/Tests Targets |

---

## Test-Flags

| Flag | Default | Beschreibung |
|------|---------|--------------|
| `RUN_BUILD_SYSTEM_TESTS` | ON | Build-System-Tests aktivieren |
| `TEST_PHASE` | "" (alle) | Spezifische Phase(n) testen |
| `BUILD_TESTS` | OFF | Projekt-Tests aktivieren (für Phase 7/8) |

### Beispiele

```bash
# Alle Phasen testen
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=ON

# Nur Phase 8 testen
cmake -B build -DTEST_PHASE=8

# Phase 7+8 mit Projekt-Tests
cmake -B build -DBUILD_TESTS=ON -DTEST_PHASE="7;8"

# Tests deaktivieren
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=OFF
```

---

## Test-Output

Jede Phase gibt Debug-Output mit eigenem Tag:

```
-- [Phase8] === Phase 8 Test Start ===
-- [Phase8] Test 1: Apps array parsing...
-- [Phase8]   Found 1 app(s) in Solution.json
-- [Phase8] Test 3: App-Container targets...
-- [Phase8]   Core Library: DemoPlayer.Core
-- [Phase8]   Runner Executable: DemoPlayer
-- [Phase8] === Phase 8 Test PASSED ===
```

---

## Erfolgs-Flags

Jede Phase setzt ein Cache-Flag bei Erfolg:

| Phase | Flag |
|-------|------|
| 1 | `PHASE1_TEST_PASSED` |
| 2 | `PHASE2_TEST_PASSED` |
| 3 | `PHASE3_TEST_PASSED` |
| 4 | `PHASE4_TEST_PASSED` |
| 5 | `PHASE5_TEST_PASSED` |
| 6 | `PHASE6_TEST_PASSED` |
| 7 | `PHASE7_TEST_PASSED` |
| 8 | `PHASE8_TEST_PASSED` |

---

## Siehe auch

- [../README.md](../README.md) — Modul-Übersicht
- [../../projects/buildsystem/concepts/Implementation_Plan.md](../../projects/buildsystem/concepts/Implementation_Plan.md) — Phasen-Planung
- [../../references/ErrorCodes.md](../../references/ErrorCodes.md) — Fehlercodes
