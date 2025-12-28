# ModuleDoc — Standard für CMake-Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Documentation Authors, Build System Developers  
> **Language:** English  
> **German:** [ModuleDoc.md](../../en/blueprints/ModuleDoc.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [API-Reference-Format](#5-api-referenz-format)
6. [Versionskopplung](#6-versionskopplung)
7. [Example: Vollständige ModuleDoc](#7-beispiel-vollständige-moduledoc)
8. [Review Checklist](#8-review-checkliste)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

This blueprint defines die **Struktur für CMake-Modul-Dokumentationen**. Er erweitert den allgemeinen Doc-Blueprint um modulspezifische Anforderungen.

### Zielgruppe

- Build System Developers
- Personen, die CMake-Module verstehen oder erweitern wollen

### Was wird dokumentiert?

| Element | Example |
|---------|----------|
| Core-Module | `Errors.cmake`, `Context.cmake`, `Json.cmake` |
| Project-Module | `Executables.cmake`, `Libraries.cmake` |
| Externals-Module | `Orchestrator.cmake`, `Fetch.cmake` |
| Include-Files | `externals/*/Include.cmake` |

---

## 2. Scope

Dieser Blueprint gilt für alle Dokumentationen, die:
- Ein einzelnes CMake-Modul (`.cmake`-Datei) beschreiben
- Im Ordner `docs/[lang]/modules/` liegen

**Nicht betroffen:**
- CMakeLists.txt (→ Reference)
- CMakePresets.json (→ Reference)
- Conceptuelle Architecture-Dokumente (→ Concept)

---

## 3. Header-Erweiterungen

### 3.1 Zusätzliche Requiredfelder

Zusätzlich zu den Feldern aus Doc.md:

| Feld | Required | Description |
|------|---------|--------------|
| `Module:` | ✓ | Relativer Pfad zum Modul |
| `Module Version:` | ✓ | Version des dokumentierten Moduls |

### 3.2 Vollständiger Header

```markdown
# [ModulName].cmake — Dokumentation

> **Version:** X.Y.Z (doc vN)  
> **Date:** YYYY-MM-DD  
> **Type:** ModuleDoc  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** Build System Developers  
> **Module:** cmake/[pfad]/[ModulName].cmake  
> **Module Version:** X.Y.Z  
> **Based on:** ModuleDoc v0.5  
> **Language:** English  
> **German:** [ModulName.md](../../en/modules/[pfad]/ModulName.md)
```

### 3.3 Modul-Link

Das `Module:`-Feld kann auch als Link formatiert werden:

```markdown
> **Module:** [cmake/core/Context.cmake](../../../cmake/core/Context.cmake)
```

### 3.4 Versionierung erklärt

Die Dokumentations-Version folgt dem Schema:

```
X.Y.Z (doc vN)
│ │ │      │
│ │ │      └── Dokumentations-Revision (nur Doku-Changes)
│ │ └── PATCH der Modul-Version
│ └── MINOR der Modul-Version  
└── MAJOR der Modul-Version
```

**Example:**
- Modul `Context.cmake` ist Version 0.1.1
- Dokumentation wurde 2x überarbeitet ohne Modul-Änderung
- → Dokumentations-Version: `0.1.1 (doc v2)`

---

## 4. Requiredabschnitte

ModuleDoc verwendet diese Struktur:

```
## Table of Contents

## 1. Overview
## 2. Dependencies
## 3. Concept / Design
## 4. API-Reference
## 5. Usagesbeispiele
## 6. Errorbehandlung / Error Codes
## 7. Best Practices
## 8. Bekannte Einschränkungen (optional)
## 9. Migration (wenn Breaking Changes)
## 10. See Also
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Overview | Kurze Description, Zweck, was das Modul löst |
| 2 | Dependencies | `include()`-Dependencies, CMake-Version |
| 3 | Concept | Design-Entscheidungen, Architecture |
| 4 | API-Reference | Alle Functions/Macros mit Parametersn |
| 5 | Examples | Konkrete Anwendungsfälle |
| 6 | Error Codes | Alle Error, die das Modul auslösen kann |
| 7 | Best Practices | Empfehlungen, Dos and Don'ts |
| 8 | Einschränkungen | Bekannte Limitierungen (optional) |
| 9 | Migration | Bei Breaking Changes (optional) |
| 10 | See Also | Verwandte Module/Dokumente |

---

## 5. API-Reference-Format

### 5.1 Funktions-Dokumentation

Jede Funktion/Macro wird so dokumentiert:

```markdown
### 4.1 function_name()

```cmake
function_name(PARAM1 PARAM2 [OPTIONAL_PARAM])
```

**Description:**  
Was die Funktion tut.

**Parameters:**

| Parameters | Required | Description |
|-----------|---------|--------------|
| `PARAM1` | ✓ | Description |
| `PARAM2` | ✓ | Description |
| `OPTIONAL_PARAM` | — | Description (Default: `...`) |

**Rückgabe:**  
Was zurückgegeben wird (Variable, PARENT_SCOPE, etc.)

**Example:**

```cmake
function_name("wert1" "wert2")
```

**Error:**  
- `E0xxx` wenn Bedingung nicht erfüllt
```

### 5.2 Example: ctx_set()

```markdown
### 4.2 ctx_set()

```cmake
ctx_set(PREFIX KEY VALUE)
```

**Description:**  
Speichert einen Wert im Context unter dem angegebenen Schlüssel.
Verwendet GLOBAL PROPERTY für zuverlässige Propagation.

**Parameters:**

| Parameters | Required | Description |
|-----------|---------|--------------|
| `PREFIX` | ✓ | Context-Namespace (z.B. `EXE_MyApp`) |
| `KEY` | ✓ | Schlüssel (UPPER_SNAKE_CASE empfohlen) |
| `VALUE` | ✓ | Wert (String oder `;`-getrennte Liste) |

**Rückgabe:**  
Keine direkte Rückgabe. Wert wird als GLOBAL PROPERTY gespeichert.

**Example:**

```cmake
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp VERSION "1.0.0")
ctx_set(EXE_MyApp EXTERNALS "bass;imgui;glfw")
```

**Error:**  
Keine — leere Werte sind erlaubt.
```

---

## 6. Versionskopplung

### 6.1 Wann Dokumentation aktualisieren?

| Modul-Änderung | Dokumentations-Aktion |
|----------------|----------------------|
| Neue Funktion | Doku hinzufügen, doc vN+1 |
| Parameters geändert | Doku aktualisieren, doc vN+1 |
| Bugfix (API gleich) | Doku prüfen, optional doc vN+1 |
| Breaking Change | Migration-Abschnitt, doc vN+1 |

### 6.2 Dateinamen

Aktuelle Dokumentationen haben **keine Version im Dateinamen**:

```
cmake/core/Context.cmake          ← Modul (Version im Header: 0.1.1)
docs/de/modules/core/Context.md   ← Dokumentation (Version im Header)
```

### 6.3 Bei Modul-Version-Bump

1. Alte Dokumentation archivieren (falls gewünscht): `Context.md` → `Context_v0_1_0.md`
2. `Context.md` aktualisieren mit neuer Version
3. Links bleiben unverändert

---

## 7. Example: Vollständige ModuleDoc

```markdown
# Context.cmake — Dokumentation

> **Version:** 1.0.0 (doc v2)  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Stable  
> **Target Audience:** Build System Developers  
> **Module:** [cmake/core/Context.cmake](../../../cmake/core/Context.cmake)  
> **Module Version:** 1.0.0  
> **Based on:** ModuleDoc v0.5  
> **Language:** English  
> **German:** [Context.md](../../en/modules/core/Context.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Concept](#3-konzept)
4. [API-Reference](#4-api-referenz)
5. [Usagesbeispiele](#5-verwendungsbeispiele)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [Best Practices](#7-best-practices)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

Das Context-Modul implementiert ein **Context-Object-Pattern** für CMake.
Es ermöglicht isolierte Namespaces zum Sammeln von Daten während der
Pipeline-Verarbeitung.

### Features

- Isolierte Namespaces pro Executable/Library/Test
- Zuverlässige Propagation über Funktionsebenen
- Debug-Dump für Diagnose

---

## 2. Dependencies

| Abhängigkeit | Typ | Description |
|--------------|-----|--------------|
| CMake 3.19+ | System | Für `include_guard(GLOBAL)` |

**Keine** anderen Module als Abhängigkeit.

---

## 3. Concept

### 3.1 Warum Context?

CMake-Variablen haben Funktions-Scope. Bei verschachtelten Aufrufen
gehen Werte verloren. GLOBAL PROPERTY löst dieses Problem.

### 3.2 Architecture

```
ctx_create(EXE_MyApp)
    │
    ├── ctx_set(EXE_MyApp, NAME, "MyApp")
    │       └── GLOBAL PROPERTY: EXE_MyApp_NAME = "MyApp"
    │
    └── ctx_get(EXE_MyApp, NAME, _var)
            └── _var = "MyApp"
```

---

## 4. API-Reference

### 4.1 ctx_create()

[... wie in Abschnitt 5.1 beschrieben ...]

### 4.2 ctx_set()

[... wie in Abschnitt 5.2 beschrieben ...]

---

## 5. Usagesbeispiele

### 5.1 Executable-Daten sammeln

```cmake
ctx_create(EXE_MyApp)
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp VERSION "1.0.0")
ctx_set(EXE_MyApp TYPE "GUI")
ctx_dump(EXE_MyApp)  # Debug-Ausgabe
```

---

## 6. Errorbehandlung

Dieses Modul wirft keine eigenen Error.

---

## 7. Best Practices

| Do | Don't |
|----|-------|
| `ctx_create()` vor erstem `ctx_set()` | Ohne Create direkt setzen |
| UPPER_SNAKE_CASE für Keys | lowercase-keys |
| Eindeutige Prefixes (EXE_, LIB_, TEST_) | Generische Prefixes |

---

## 8. See Also

- [Errors.cmake](Errors_cmake_v0_1_1_doc_v1.md) — Errorbehandlung
- [ExecutableCollect.cmake](../project/ExecutableCollect_cmake_v0_1_0_doc_v1.md) — Verwendet Context

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.1 (doc v2)** | **2025-12-13** | **Format nach ModuleDoc v0.5 aktualisiert** |
| 0.1.1 (doc v1) | 2025-12-05 | Initial für Modul v0.1.1 |
```

---

## 8. Review Checklist

Zusätzlich zur Doc.md Checkliste:

**Header:**
- [ ] `Module:`-Feld vorhanden mit korrektem Pfad
- [ ] `Module Version:`-Feld vorhanden
- [ ] Link zum Modul funktioniert

**Inhalt:**
- [ ] Alle Functions des Moduls dokumentiert
- [ ] Alle Parameters jeder Funktion beschrieben
- [ ] Return Values dokumentiert
- [ ] Error/Error Codes aufgelistet
- [ ] Mindestens ein Example pro Funktion

**Kopplung:**
- [ ] Dateiname entspricht Modul-Version
- [ ] doc vN korrekt

---

## 9. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [CMake.md](CMake.md) — Standard für CMake-Scripts
- [Context.md](../modules/core/Context.md) — Example-Dokumentation

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Modul-Link im Header, API-Reference-Format, Versionskopplung, vollständiges Example** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
