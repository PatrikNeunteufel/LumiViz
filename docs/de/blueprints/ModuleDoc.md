# ModuleDoc — Standard für CMake-Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Doc v0.5, Blueprint v0.5  
> **Zielgruppe:** Dokumentations-Ersteller, Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [ModuleDoc.md](../../en/blueprints/ModuleDoc.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Pflichtabschnitte](#4-pflichtabschnitte)
5. [API-Referenz-Format](#5-api-referenz-format)
6. [Versionskopplung](#6-versionskopplung)
7. [Beispiel: Vollständige ModuleDoc](#7-beispiel-vollständige-moduledoc)
8. [Review-Checkliste](#8-review-checkliste)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert die **Struktur für CMake-Modul-Dokumentationen**. Er erweitert den allgemeinen Doc-Blueprint um modulspezifische Anforderungen.

### Zielgruppe

- Build-System-Entwickler
- Personen, die CMake-Module verstehen oder erweitern wollen

### Was wird dokumentiert?

| Element | Beispiel |
|---------|----------|
| Core-Module | `Errors.cmake`, `Context.cmake`, `Json.cmake` |
| Project-Module | `Executables.cmake`, `Libraries.cmake` |
| Externals-Module | `Orchestrator.cmake`, `Fetch.cmake` |
| Include-Files | `externals/*/Include.cmake` |

---

## 2. Geltungsbereich

Dieser Blueprint gilt für alle Dokumentationen, die:
- Ein einzelnes CMake-Modul (`.cmake`-Datei) beschreiben
- Im Ordner `docs/[lang]/modules/` liegen

**Nicht betroffen:**
- CMakeLists.txt (→ Reference)
- CMakePresets.json (→ Reference)
- Konzeptuelle Architektur-Dokumente (→ Concept)

---

## 3. Header-Erweiterungen

### 3.1 Zusätzliche Pflichtfelder

Zusätzlich zu den Feldern aus Doc.md:

| Feld | Pflicht | Beschreibung |
|------|---------|--------------|
| `Modul:` | ✓ | Relativer Pfad zum Modul |
| `Modul-Version:` | ✓ | Version des dokumentierten Moduls |

### 3.2 Vollständiger Header

```markdown
# [ModulName].cmake — Dokumentation

> **Version:** X.Y.Z (doc vN)  
> **Datum:** YYYY-MM-DD  
> **Typ:** ModuleDoc  
> **Status:** [In Entwicklung | Stabil | Deprecated]  
> **Zielgruppe:** Build-System-Entwickler  
> **Modul:** cmake/[pfad]/[ModulName].cmake  
> **Modul-Version:** X.Y.Z  
> **Basiert auf:** ModuleDoc v0.5  
> **Sprache:** Deutsch  
> **English:** [ModulName.md](../../en/modules/[pfad]/ModulName.md)
```

### 3.3 Modul-Link

Das `Modul:`-Feld kann auch als Link formatiert werden:

```markdown
> **Modul:** [cmake/core/Context.cmake](../../../cmake/core/Context.cmake)
```

### 3.4 Versionierung erklärt

Die Dokumentations-Version folgt dem Schema:

```
X.Y.Z (doc vN)
│ │ │      │
│ │ │      └── Dokumentations-Revision (nur Doku-Änderungen)
│ │ └── PATCH der Modul-Version
│ └── MINOR der Modul-Version  
└── MAJOR der Modul-Version
```

**Beispiel:**
- Modul `Context.cmake` ist Version 0.1.1
- Dokumentation wurde 2x überarbeitet ohne Modul-Änderung
- → Dokumentations-Version: `0.1.1 (doc v2)`

---

## 4. Pflichtabschnitte

ModuleDoc verwendet diese Struktur:

```
## Inhaltsverzeichnis

## 1. Übersicht
## 2. Abhängigkeiten
## 3. Konzept / Design
## 4. API-Referenz
## 5. Verwendungsbeispiele
## 6. Fehlerbehandlung / Error Codes
## 7. Best Practices
## 8. Bekannte Einschränkungen (optional)
## 9. Migration (wenn Breaking Changes)
## 10. Siehe auch
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Übersicht | Kurze Beschreibung, Zweck, was das Modul löst |
| 2 | Abhängigkeiten | `include()`-Abhängigkeiten, CMake-Version |
| 3 | Konzept | Design-Entscheidungen, Architektur |
| 4 | API-Referenz | Alle Funktionen/Macros mit Parametern |
| 5 | Beispiele | Konkrete Anwendungsfälle |
| 6 | Error Codes | Alle Fehler, die das Modul auslösen kann |
| 7 | Best Practices | Empfehlungen, Dos and Don'ts |
| 8 | Einschränkungen | Bekannte Limitierungen (optional) |
| 9 | Migration | Bei Breaking Changes (optional) |
| 10 | Siehe auch | Verwandte Module/Dokumente |

---

## 5. API-Referenz-Format

### 5.1 Funktions-Dokumentation

Jede Funktion/Macro wird so dokumentiert:

```markdown
### 4.1 function_name()

```cmake
function_name(PARAM1 PARAM2 [OPTIONAL_PARAM])
```

**Beschreibung:**  
Was die Funktion tut.

**Parameter:**

| Parameter | Pflicht | Beschreibung |
|-----------|---------|--------------|
| `PARAM1` | ✓ | Beschreibung |
| `PARAM2` | ✓ | Beschreibung |
| `OPTIONAL_PARAM` | — | Beschreibung (Default: `...`) |

**Rückgabe:**  
Was zurückgegeben wird (Variable, PARENT_SCOPE, etc.)

**Beispiel:**

```cmake
function_name("wert1" "wert2")
```

**Fehler:**  
- `E0xxx` wenn Bedingung nicht erfüllt
```

### 5.2 Beispiel: ctx_set()

```markdown
### 4.2 ctx_set()

```cmake
ctx_set(PREFIX KEY VALUE)
```

**Beschreibung:**  
Speichert einen Wert im Context unter dem angegebenen Schlüssel.
Verwendet GLOBAL PROPERTY für zuverlässige Propagation.

**Parameter:**

| Parameter | Pflicht | Beschreibung |
|-----------|---------|--------------|
| `PREFIX` | ✓ | Context-Namespace (z.B. `EXE_MyApp`) |
| `KEY` | ✓ | Schlüssel (UPPER_SNAKE_CASE empfohlen) |
| `VALUE` | ✓ | Wert (String oder `;`-getrennte Liste) |

**Rückgabe:**  
Keine direkte Rückgabe. Wert wird als GLOBAL PROPERTY gespeichert.

**Beispiel:**

```cmake
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp VERSION "1.0.0")
ctx_set(EXE_MyApp EXTERNALS "bass;imgui;glfw")
```

**Fehler:**  
Keine — leere Werte sind erlaubt.
```

---

## 6. Versionskopplung

### 6.1 Wann Dokumentation aktualisieren?

| Modul-Änderung | Dokumentations-Aktion |
|----------------|----------------------|
| Neue Funktion | Doku hinzufügen, doc vN+1 |
| Parameter geändert | Doku aktualisieren, doc vN+1 |
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

## 7. Beispiel: Vollständige ModuleDoc

```markdown
# Context.cmake — Dokumentation

> **Version:** 1.0.0 (doc v2)  
> **Datum:** 2025-12-13  
> **Typ:** ModuleDoc  
> **Status:** Stabil  
> **Zielgruppe:** Build-System-Entwickler  
> **Modul:** [cmake/core/Context.cmake](../../../cmake/core/Context.cmake)  
> **Modul-Version:** 1.0.0  
> **Basiert auf:** ModuleDoc v0.5  
> **Sprache:** Deutsch  
> **English:** [Context.md](../../en/modules/core/Context.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Fehlerbehandlung](#6-fehlerbehandlung)
7. [Best Practices](#7-best-practices)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Das Context-Modul implementiert ein **Context-Object-Pattern** für CMake.
Es ermöglicht isolierte Namespaces zum Sammeln von Daten während der
Pipeline-Verarbeitung.

### Features

- Isolierte Namespaces pro Executable/Library/Test
- Zuverlässige Propagation über Funktionsebenen
- Debug-Dump für Diagnose

---

## 2. Abhängigkeiten

| Abhängigkeit | Typ | Beschreibung |
|--------------|-----|--------------|
| CMake 3.19+ | System | Für `include_guard(GLOBAL)` |

**Keine** anderen Module als Abhängigkeit.

---

## 3. Konzept

### 3.1 Warum Context?

CMake-Variablen haben Funktions-Scope. Bei verschachtelten Aufrufen
gehen Werte verloren. GLOBAL PROPERTY löst dieses Problem.

### 3.2 Architektur

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

## 4. API-Referenz

### 4.1 ctx_create()

[... wie in Abschnitt 5.1 beschrieben ...]

### 4.2 ctx_set()

[... wie in Abschnitt 5.2 beschrieben ...]

---

## 5. Verwendungsbeispiele

### 5.1 Executable-Daten sammeln

```cmake
ctx_create(EXE_MyApp)
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp VERSION "1.0.0")
ctx_set(EXE_MyApp TYPE "GUI")
ctx_dump(EXE_MyApp)  # Debug-Ausgabe
```

---

## 6. Fehlerbehandlung

Dieses Modul wirft keine eigenen Fehler.

---

## 7. Best Practices

| Do | Don't |
|----|-------|
| `ctx_create()` vor erstem `ctx_set()` | Ohne Create direkt setzen |
| UPPER_SNAKE_CASE für Keys | lowercase-keys |
| Eindeutige Prefixes (EXE_, LIB_, TEST_) | Generische Prefixes |

---

## 8. Siehe auch

- [Errors.cmake](Errors_cmake_v0_1_1_doc_v1.md) — Fehlerbehandlung
- [ExecutableCollect.cmake](../project/ExecutableCollect_cmake_v0_1_0_doc_v1.md) — Verwendet Context

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.1 (doc v2)** | **2025-12-13** | **Format nach ModuleDoc v0.5 aktualisiert** |
| 0.1.1 (doc v1) | 2025-12-05 | Initial für Modul v0.1.1 |
```

---

## 8. Review-Checkliste

Zusätzlich zur Doc.md Checkliste:

**Header:**
- [ ] `Modul:`-Feld vorhanden mit korrektem Pfad
- [ ] `Modul-Version:`-Feld vorhanden
- [ ] Link zum Modul funktioniert

**Inhalt:**
- [ ] Alle Funktionen des Moduls dokumentiert
- [ ] Alle Parameter jeder Funktion beschrieben
- [ ] Rückgabewerte dokumentiert
- [ ] Fehler/Error Codes aufgelistet
- [ ] Mindestens ein Beispiel pro Funktion

**Kopplung:**
- [ ] Dateiname entspricht Modul-Version
- [ ] doc vN korrekt

---

## 9. Siehe auch

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [CMake.md](CMake.md) — Standard für CMake-Scripts
- [Context.md](../modules/core/Context.md) — Beispiel-Dokumentation

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Modul-Link im Header, API-Referenz-Format, Versionskopplung, vollständiges Beispiel** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
