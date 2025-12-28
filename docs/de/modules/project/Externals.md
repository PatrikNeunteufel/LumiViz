# Externals.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Externals.md](../../en/modules/project/Externals.md)  
> **Modul:** [`cmake/project/Externals.cmake`](../../../../cmake/project/Externals.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Verarbeitung](#3-verarbeitung)
4. [Ladereihenfolge](#4-ladereihenfolge)
5. [Debug-Ausgaben](#5-debug-ausgaben)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Das `Externals.cmake` Modul ist der **Einstiegspunkt für externe Abhängigkeiten**. Es verarbeitet alle Externals aus Solution.json und dispatcht an den Orchestrator.

### Kernidee

Zentrale Verarbeitung aller Externals VOR Libraries und Executables, damit alle Targets verfügbar sind.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Iteration | Über externals-Objekt (Key-Value) |
| Dispatch | An Orchestrator für Typ-spezifische Verarbeitung |
| Timing | Läuft VOR Libraries/Executables |

### Lädt automatisch

- cmake/externals/Orchestrator.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Errors.cmake | 0.5.0 | (indirekt via Orchestrator) |
| Debug.cmake | 0.5.0 | `dbg`, `dbg_init`, `dbgspace`, `enddbgblock` |
| Json.cmake | 0.5.0 | `_json_has_key`, `_json_get_object` |
| Solution.cmake | 0.5.0 | `SOLUTION_JSON` Property |

---

## 3. Verarbeitung

### 3.1 Pipeline-Ablauf

```
Externals.cmake
    │
    ├── 1. Load Orchestrator.cmake
    │
    ├── 2. Get externals block from SOLUTION_JSON
    │   └── Return early if empty
    │
    ├── 3. Store as SOLUTION_EXTERNALS_JSON
    │
    └── 4. For each external (key-value):
        │
        ├── 4.1 Get name (key)
        │
        ├── 4.2 Get definition (value)
        │
        ├── 4.3 Check skip flag
        │   ├── skip: true  → Mark as SKIPPED, continue to next
        │   └── skip: false → Proceed with orchestration
        │
        └── 4.4 _orchestrate_external(name, definition)
```

### 3.2 Skip-Feature

Mit `skip: true` können Externals vorbereitet, aber noch nicht geladen werden:

```json
{
    "externals": {
        "bass": { "path": "externals/bass" },
        "future_lib": { 
            "git": "https://...", 
            "tag": "v1.0.0",
            "skip": true
        }
    }
}
```

**Verhalten:**
- Geskippte Externals werden in `SKIPPED_EXTERNALS` Property gespeichert
- `apply_external_to_target()` prüft diese Liste → **E013 FATAL** bei Verwendung
- Funktioniert für Local, Fetched und System Externals

### 3.3 Externals-Struktur

```json
{
    "externals": {
        "bass": { "path": "externals/bass" },
        "imgui": { "git": "https://...", "tag": "v1.90" },
        "glfw": { "git": "https://...", "tag": "3.3.8" }
    }
}
```

Jeder Key wird einzeln an den Orchestrator übergeben.

---

## 4. Ladereihenfolge

```
CMakeLists.txt
    │
    ├── Core-Module laden
    │
    ├── Solution.cmake           ← JSON laden
    │
    ├── Externals.cmake          ← HIER (vor Libraries!)
    │   └── _orchestrate_external() für jedes External
    │
    ├── Libraries.cmake          ← Kann Externals verwenden
    │
    └── Executables.cmake        ← Kann Externals verwenden
```

**Wichtig:** Externals muss VOR Libraries und Executables geladen werden!

---

## 5. Debug-Ausgaben

```
-- [Externals] === Externals Pipeline Start ===
-- [Externals] Processing 3 external(s)...
-- [Externals] --- Processing: bass ---
-- [Externals] --- Processing: imgui ---
-- [Externals] --- Processing: glfw ---
-- [Externals] 
-- [Externals] === Externals Pipeline Complete ===
-- -------------------------------------------
```

---

## 6. Siehe auch

- [Orchestrator.cmake](../externals/Orchestrator.md) — Typ-spezifisches Handling
- [Solution.cmake](Solution.md) — Liefert SOLUTION_JSON
- [Fetch.cmake](../externals/Fetch.md) — Git-Fetching
- [Local_Attach.cmake](../externals/Local_Attach.md) — Lokale Externals

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.6.0** | **2025-12-20** | **Skip-Feature: Externals mit `skip: true` werden nicht geladen, SKIPPED_EXTERNALS Property** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0 |
| 0.1.0 | 2025-12-08 | Initial (Clean Start): Externals-Pipeline |
