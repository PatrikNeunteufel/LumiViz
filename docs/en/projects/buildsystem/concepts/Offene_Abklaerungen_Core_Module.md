# Offene Abklärungen — Build-System Module

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Abklärung  
> **Status:** In Bearbeitung  
> **Target Audience:** Build System Developers  
> **Language:** English  

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dokumentations-Status](#2-dokumentations-status)
3. [Core-Module Abklärungen](#3-core-module-abklärungen)
4. [Project-Module Abklärungen](#4-project-module-abklärungen)
5. [Offene Architecture-Fragen](#5-offene-architektur-fragen)
6. [Empfehlungen](#6-empfehlungen)
7. [Entscheidungsmatrix](#7-entscheidungsmatrix)
8. [Changelog](#8-changelog)

---

## 1. Overview

Dieses Dokument trackt **offene Abklärungspunkte** und den **Dokumentations-Status** aller Build-System Module nach der Migration auf Blueprint v0.5.0.

### Hintergrund

Am 2025-12-15 wurden alle Modul-Dokumentationen auf Blueprint v0.5.0 migriert:
- Neuer Header mit Zielgruppe, Sprache, English-Link, Modul-Link
- Nummeriertes Table of Contents mit funktionierenden Ankern
- Kapitel-Nummerierung
- Einheitliche Modul-Version 0.5.0

---

## 2. Dokumentations-Status

### 2.1 Core-Module (9/9 dokumentiert) ✅

| Modul | Doku-Version | CMake-Version | Debug | Errors | Status |
|-------|--------------|---------------|-------|--------|--------|
| CompilerOptions.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ✅ | ✅ W201 | ✅ Fertig |
| Context.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ❌ | ❌ | ⚪ Abklären |
| Debug.cmake | 0.5.0 | 0.1.1 → 0.5.0 | — | — | ✅ Fertig |
| Errors.cmake | 0.5.0 | 0.1.2 → 0.5.0 | — | — | ✅ Fertig |
| Json.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ❌ | ❌ | ⚪ Abklären |
| OutputDirs.cmake | 0.5.0 | 0.1.3 → 0.5.0 | ❌ | ❌ | ⚪ Abklären |
| SourceCollect.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| Validation.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ❌ | ✅ | ✅ Fertig |
| Warnings.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ❌ | ❌ | ⚪ Abklären |

### 2.2 Project-Module (11/11 dokumentiert) ✅

| Modul | Doku-Version | CMake-Version | Debug | Errors | Status |
|-------|--------------|---------------|-------|--------|--------|
| Solution.cmake | 0.5.0 | 0.1.1 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| Executables.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| ExecutableCollect.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| ExecutableCreate.cmake | 0.5.0 | 0.1.2 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| Libraries.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| LibraryCollect.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ❌ | ✅ Fertig |
| LibraryCreate.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ✅ | ✅ Fertig |
| Externals.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ❌ | ✅ Fertig |
| Tests.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ❌¹ | ❌ | ✅ Fertig |
| TestCollect.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ❌ | ✅ Fertig |
| TestCreate.cmake | 0.5.0 | 0.1.0 → 0.5.0 | ✅ | ✅ | ✅ Fertig |

**Anmerkung:**
- ¹ Tests.cmake verwendet `message(STATUS ...)` statt `dbg()` — Konsistenz-Abklärung nötig

### 2.3 Externals-Module (8/8 dokumentiert) ✅

| Modul | Dateiname | Modul-Version | Status |
|-------|-----------|---------------|--------|
| Orchestrator.cmake | Orchestrator_cmake.md | 0.5.0 | ✅ Fertig |
| Core/Fetch.cmake | Fetch_cmake.md | 0.5.0 | ✅ Fertig |
| Fetched/Handler.cmake | Handler_cmake.md | 0.5.0 | ✅ Fertig |
| Local/Attach.cmake | Attach_cmake.md | 0.5.0 | ✅ Fertig |
| Hooks/HookLoader.cmake | HookLoader_cmake.md | 0.5.0 | ✅ Fertig |
| Registry/Targets.cmake | Targets_cmake.md | 0.5.0 | ✅ Fertig |
| hooks/prefetch/glfw.cmake | glfw_PreFetch.md | 0.5.0 | ✅ Fertig |
| hooks/postfetch/imgui.cmake | imgui_PostFetch.md | 0.5.0 | ✅ Fertig |

### 2.4 Include.cmake Dokumentationen (4/4 dokumentiert) ✅

| External | Dateiname | Modul-Version | Status |
|----------|-----------|---------------|--------|
| includes/bass/Include.cmake | bass_Include.md | 0.5.0 | ✅ Fertig |
| includes/lua54/Include.cmake | lua54_Include.md | 0.5.0 | ✅ Fertig |
| includes/doctest/Include.cmake | doctest_Include.md | 0.5.0 | ✅ Fertig |
| includes/glad/Include.cmake | glad_Include.md | 0.5.0 | ✅ Fertig |

### 2.5 Externals-Reference (v0.5.2 - 8 Kategorien parallel) ✅

**Hauptübersicht:**

| Dokument | Dateiname | Inhalt | Status |
|----------|-----------|--------|--------|
| Externals | Externals.md | Alle Externals nach Kategorie | ✅ v0.5.2 |

**Overviews-Dokumente:**

| Typ | Dateiname | Inhalt | Status |
|-----|-----------|--------|--------|
| Local | Local_Externals.md | Overview Local Externals | ✅ v0.5.2 |
| Git | Git_Externals.md | Overview Git Externals | ✅ v0.5.2 |

**Kategorie-Dokumente (8 Kategorien, parallel):**

| Kategorie | Local | Git | Status |
|-----------|-------|-----|--------|
| Testing | Local_Externals_Testing.md (doctest) | Git_Externals_Testing.md (googletest, catch2) | ✅ v0.5.2 |
| Scripting | Local_Externals_Scripting.md (lua54) | Git_Externals_Scripting.md (sol2, pybind11) | ✅ v0.5.2 |
| GUI | Local_Externals_GUI.md (glad) | Git_Externals_GUI.md (glfw, imgui) | ✅ v0.5.2 |
| Media | Local_Externals_Media.md (BASS) | Git_Externals_Media.md (miniaudio, stb) | ✅ v0.5.2 |
| Core | — | Git_Externals_Core.md (spdlog, fmt, abseil) | ✅ v0.5.2 |
| Data | — | Git_Externals_Data.md | ✅ (Project Knowledge) |
| Network | — | Git_Externals_Network.md | ✅ (Project Knowledge) |
| AI | — | Git_Externals_AI.md | ✅ (Project Knowledge) |

### 2.6 Fortschritt

```
Core-Module:      [█████████████████████] 9/9   (100%)
Project-Module:   [█████████████████████] 11/11 (100%)
Externals-Module: [█████████████████████] 8/8   (100%)
Include.cmake:    [█████████████████████] 4/4   (100%)
Externals-Ref:    [█████████████████████] 13/13 (100%)
─────────────────────────────────────────────────────────
Gesamt:           [█████████████████████] 45/45 (100%)
```

---

## 3. Core-Module Abklärungen

### 3.1 Context.cmake — Basis-Modul ohne Debug/Errors

**Aktueller Zustand:**
- Nur `DEBUG_CONTEXT` Cache-Variable für `ctx_dump()`
- Keine `dbg()` Ausgaben
- Keine Parameters-Validierung

**Design-Entscheidung:** Context.cmake bleibt ohne Debug/Errors-Abhängigkeit, da:
- Es von Debug.cmake verwendet wird (indirekte Abhängigkeit)
- Echtes Basis-Modul ohne zirkuläre Dependencies
- Einfacher zu testen

**Potenzielle Verbesserungen (optional):**

| Situation | Aktuelle Behandlung | Alternative |
|-----------|---------------------|-------------|
| `ctx_create("")` | Stille Ausführung | cmake_assert? |
| `ctx_get()` nicht-existent | Leerer String | Warning? |

---

### 3.2 Json.cmake — Errortolerantes Design

**Aktueller Zustand:**
- Keine Debug-Ausgaben
- Keine expliziten Error-Codes
- Error werden durch leere Return Values signalisiert

**Design-Entscheidung:** Json.cmake bleibt ohne Debug/Errors, da:
- Validierung erfolgt durch aufrufende Module (Validation.cmake)
- Errortolerantes Design ist gewollt (Defaults statt Exceptions)
- CMake 3.19+ JSON-Error werden intern behandelt

---

### 3.3 OutputDirs.cmake — Minimalistisches Modul

**Aktueller Zustand:**
- Keine Debug-Ausgaben
- Keine Errorbehandlung
- CMake-Error werden durchgereicht

**Empfehlung:** Optional `dbg()` für Tracing hinzufügen:
```cmake
dbg(${DBG_ULTRA_RARE} "  Output: ${_target_base}/bin" ID OUTPUT_DIRS)
```

---

### 3.4 Warnings.cmake — Sehr einfaches Modul

**Aktueller Zustand:**
- Keine Debug-Ausgaben
- Keine Errorbehandlung
- 72 Zeilen Code

**Empfehlung:** Bleibt ohne Debug/Errors — zu simpel für Overhead.

---

### 3.5 Importante Klarstellung: Errors.cmake vs Warnings.cmake

| Modul | Zweck | Functions |
|-------|-------|------------|
| **Errors.cmake** | Build-System Error/Warningen | `cmake_fatal()`, `cmake_warn()`, `cmake_assert()` |
| **Warnings.cmake** | Compiler-Warningen | `apply_warnings()` → `-Wall`, `/W4` |

⚠️ **Namensverwirrung:** `cmake_warn()` ist in **Errors.cmake** definiert, nicht in Warnings.cmake!

---

## 4. Project-Module Abklärungen

### 4.1 Tests.cmake — Inkonsistente Debug-Usage

**Problem:** Tests.cmake verwendet `message(STATUS "[Tests] ...")` statt `dbg()`:

```cmake
message(STATUS "[Tests] === Test Pipeline Start ===")
message(STATUS "[Tests] Processing ${_tests_count} test(s)...")
```

**Empfehlung:** Migration zu `dbg()` für Konsistenz mit Executables.cmake und Libraries.cmake:

```cmake
dbg_init(ID TESTS LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Tests")
dbg(${DBG_OFTEN} "=== Test Pipeline Start ===" ID TESTS)
```

---

### 4.2 LibraryCollect.cmake — Keine Errors-Integration

**Aktueller Zustand:** Nur `cmake_fatal("E001" ...)` für fehlendes name-Feld.

**Status:** ✅ Ausreichend — Weitere Validierung erfolgt in LibraryCreate.

---

### 4.3 Externals.cmake — Minimale Errorbehandlung

**Aktueller Zustand:** Delegiert an Orchestrator.cmake.

**Status:** ✅ Ausreichend — Orchestrator übernimmt Validierung.

---

## 5. Offene Architecture-Fragen

### A1: CMake-Module auf v0.5.0 aktualisieren

**Frage:** Sollen die CMake-Module selbst auf Version 0.5.0 aktualisiert werden?

**Status:** Die Dokumentation ist bereits auf v0.5.0, die CMake-Dateien noch auf 0.1.x.

**Empfehlung:** Yes, mit folgenden Changes:
- Header aktualisieren (Version, Datum, "Based on: v0.5")
- Keine funktionalen Changes nötig

---

### A2: Externals-Module dokumentieren

**Frage:** Wann sollen die Externals-Module (Orchestrator, Fetch, etc.) dokumentiert werden?

**Ausstehend:**
- Orchestrator.cmake
- Fetch.cmake
- Fetched_Handler.cmake
- Local_Attach.cmake
- Registry_Targets.cmake
- HookLoader.cmake
- Pre/PostFetch Hooks

---

### A3: Tests.cmake Debug-Migration

**Frage:** Soll Tests.cmake von `message()` auf `dbg()` migriert werden?

**Pro:**
- Konsistenz mit Executables.cmake, Libraries.cmake
- Kontrollierbare Verbosity

**Contra:**
- Funktioniert aktuell
- Niedriger Aufwand

**Empfehlung:** Migration in v0.5.0 oder v0.6.0

---

## 6. Empfehlungen

### Sofort (v0.5.0)

| Aktion | Priorität | Status |
|--------|-----------|--------|
| Core-Module Dokumentation | Hoch | ✅ Fertig |
| Project-Module Dokumentation | Hoch | ✅ Fertig |
| CMake-Module Header auf v0.5.0 | Mittel | 🔲 Ausstehend |

### Kurzfristig (v0.5.x)

| Aktion | Priorität | Status |
|--------|-----------|--------|
| Externals-Module dokumentieren | Hoch | 🔲 Ausstehend |
| Tests.cmake → dbg() Migration | Niedrig | 🔲 Optional |
| OutputDirs.cmake dbg() hinzufügen | Niedrig | 🔲 Optional |

### Mittelfristig (v0.6.x)

| Aktion | Priorität |
|--------|-----------|
| Guidelines-Update für Basis-Module | Mittel |
| Error-Code-Dokumentation vervollständigen | Mittel |
| English Translations | Niedrig |

---

## 7. Entscheidungsmatrix

### Core-Module

| Modul | Doku v0.5.0 | Debug hinzufügen? | Errors hinzufügen? |
|-------|-------------|-------------------|-------------------|
| CompilerOptions.cmake | ✅ | 🟢 Vorhanden | 🟢 Vorhanden |
| Context.cmake | ✅ | 🔴 No (Basis) | 🔴 No (Basis) |
| Debug.cmake | ✅ | — | — |
| Errors.cmake | ✅ | — | — |
| Json.cmake | ✅ | 🔴 No (Design) | 🔴 No (Design) |
| OutputDirs.cmake | ✅ | 🟡 Optional | ⚪ Optional |
| SourceCollect.cmake | ✅ | 🟢 Vorhanden | 🟢 Vorhanden |
| Validation.cmake | ✅ | ⚪ Optional | 🟢 Vorhanden |
| Warnings.cmake | ✅ | ⚪ Optional | ⚪ Optional |

### Project-Module

| Modul | Doku v0.5.0 | Debug | Errors | Aktion nötig? |
|-------|-------------|-------|--------|---------------|
| Solution.cmake | ✅ | ✅ | ✅ | ❌ |
| Executables.cmake | ✅ | ✅ | ✅ | ❌ |
| ExecutableCollect.cmake | ✅ | ✅ | ✅ | ❌ |
| ExecutableCreate.cmake | ✅ | ✅ | ✅ | ❌ |
| Libraries.cmake | ✅ | ✅ | ✅ | ❌ |
| LibraryCollect.cmake | ✅ | ✅ | ❌ | ⚪ Optional |
| LibraryCreate.cmake | ✅ | ✅ | ✅ | ❌ |
| Externals.cmake | ✅ | ✅ | ❌ | ❌ |
| Tests.cmake | ✅ | ❌¹ | ❌ | 🟡 Migration |
| TestCollect.cmake | ✅ | ✅ | ❌ | ❌ |
| TestCreate.cmake | ✅ | ✅ | ✅ | ❌ |

**Legende:**
- ✅ Fertig / Vorhanden
- 🟢 Bereits vorhanden
- 🟡 Empfohlen
- ⚪ Optional
- 🔴 Nicht empfohlen
- ❌ Keine Aktion nötig
- ¹ Verwendet message() statt dbg()

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Erweitert auf alle Module (Core + Project), Dokumentations-Status-Tracking, Fortschrittsanzeige, aktualisierte Entscheidungsmatrix** |
| 0.1.0 | 2025-12-15 | Initial: Analyse der Core-Module auf Debug/Error-Konsistenz |
