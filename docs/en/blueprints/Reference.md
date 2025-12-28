# Reference — Standard für API/Schema-Referenceen

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Reference.md](../../en/blueprints/Reference.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Header-Struktur](#3-header-struktur)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [Eintragsformat](#5-eintragsformat)
6. [Schnellreferenz-Tabellen](#6-schnellreferenz-tabellen)
7. [Example: Vollständige Reference](#7-beispiel-vollständige-reference)
8. [Review Checklist](#8-review-checkliste)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

This blueprint defines die **Struktur für Reference-Dokumentationen**. Referenceen sind Nachschlagewerke, die vollständige, strukturierte Informationen zu einem Thema liefern.

### Zielgruppe

- Alle Entwickler, die schnell etwas nachschlagen wollen
- Fokus auf Vollständigkeit und schnelles Finden

### Abgrenzung

| Dokumentations-Typ | Fragestellung |
|-------------------|---------------|
| **Reference** | "Was bedeutet X?" "Welche Optionen gibt es?" |
| **Guide** | "Wie mache ich X?" |
| **ModuleDoc** | "Wie funktioniert Modul X intern?" |

---

## 2. Scope

Dieser Blueprint gilt für:
- Schema-Dokumentationen (`Solution_Schema`, `CMakePresets`)
- Error-Code-Referenceen
- Glossare
- API-Overviewen (nicht Module — dafür ModuleDoc)

**Typischer Ordner:** `docs/[lang]/reference/`

---

## 3. Header-Struktur

### 3.1 Standard-Header

```markdown
# [Name] — Reference

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Reference  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Name.md](../../en/reference/Name.md)
```

### 3.2 Keine zusätzlichen Requiredfelder

Reference verwendet den Standard-Header aus Doc.md.

---

## 4. Requiredabschnitte

Referenceen verwenden diese Struktur:

```
## Table of Contents

## 1. Overview
## 2. Konventionen
## 3. [Kategorisierte Einträge]
## 4. Schnellreferenz
## 5. Usage in Code
## 6. See Also
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Overview | Was wird dokumentiert? Zweck |
| 2 | Konventionen | Notation, Legende, Abkürzungen |
| 3 | Einträge | Kategorisierte, vollständige Auflistung |
| 4 | Schnellreferenz | Kompakte Tabelle aller Einträge |
| 5 | Usage | Code-Examples für typische Anwendung |
| 6 | See Also | Verwandte Dokumente |

---

## 5. Eintragsformat

### 5.1 Für Schema-Felder

```markdown
### 3.1 `feldname`

| Aspekt | Wert |
|--------|------|
| **Typ** | `string` / `number` / `boolean` / `object` / `array` |
| **Required** | ✓ / — |
| **Default** | `"wert"` / — |
| **Seit** | v0.1.0 |

**Description:**  
Was dieses Feld macht.

**Gültige Werte:**
- `"option1"` — Description
- `"option2"` — Description

**Example:**
```json
"feldname": "option1"
```

**Notee:**
- Zusätzliche Informationen
```

### 5.2 Für Error Codes

```markdown
### E0501 — Executable nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Kategorie** | Executable Pipeline |
| **Schweregrad** | FATAL_ERROR |
| **Seit** | v0.1.0 |

**Meldung:**
```
[Executables] ERROR E0501: Executable 'MyApp' not found
```

**Ursache:**  
Das in Solution.json deklarierte Executable existiert nicht im angegebenen Pfad.

**Lösung:**
1. Pfad in Solution.json prüfen
2. Dateistruktur prüfen
3. `path`-Feld ggf. anpassen

**Verwandte Error:**
- [E0502](#e0502) — Executable-Typ ungültig
```

### 5.3 Für Glossar-Einträge

```markdown
### Context

**Definition:**  
Ein isolierter Namespace zum Sammeln von Daten während der CMake-Verarbeitung.

**Usage:**
```cmake
ctx_create(EXE_MyApp)
ctx_set(EXE_MyApp NAME "MyApp")
```

**See Also:** [Context.cmake](../modules/core/Context_cmake_v0_1_1_doc_v1.md)
```

---

## 6. Schnellreferenz-Tabellen

### 6.1 Zweck

Am Ende des Hauptteils eine kompakte Overview aller Einträge.

### 6.2 Format für Schema

```markdown
## 4. Schnellreferenz

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `name` | string | ✓ | — | Eindeutiger Name |
| `version` | string | — | `"0.1.0"` | SemVer-Version |
| `type` | string | — | `"CONSOLE"` | CONSOLE / GUI |
```

### 6.3 Format für Error Codes

```markdown
## 4. Schnellreferenz

| Code | Kategorie | Description |
|------|-----------|--------------|
| E0101 | Core | Json-Datei nicht gefunden |
| E0102 | Core | Json-Parse-Error |
| E0501 | Executable | Executable nicht gefunden |
| E0502 | Executable | Ungültiger Typ |
```

---

## 7. Example: Vollständige Reference

```markdown
# ErrorCodes — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [ErrorCodes.md](../../en/reference/ErrorCodes.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Error Codes nach Kategorie](#3-error-codes-nach-kategorie)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Usage in Code](#5-verwendung-in-code)
6. [See Also](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Overview

This reference documents alle Error Codes des CMake Architecture 
Build-Systems.

### Error-Code-Format

```
E[KK][NN]

K = Kategorie (2 Ziffern)
N = Nummer (2 Ziffern)
```

---

## 2. Konventionen

### Kategorien

| Präfix | Kategorie | Description |
|--------|-----------|--------------|
| E01xx | Core | Grundlegende Error |
| E05xx | Executable | Executable-Pipeline |
| E06xx | Library | Library-Pipeline |
| E20xx | External | Externals-Verarbeitung |

### Schweregrade

| Symbol | Bedeutung |
|--------|-----------|
| ⛔ FATAL | Build bricht ab |
| ⚠️ WARNING | Build läuft weiter |
| ℹ️ INFO | Nur Information |

---

## 3. Error Codes nach Kategorie

### 3.1 Core (E01xx)

#### E0101 — JSON-Datei nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Meldung:**
```
[Core] ERROR E0101: Solution.json not found at: ${path}
```

**Lösung:** Pfad prüfen, Datei erstellen.

---

### 3.2 Executable (E05xx)

#### E0501 — Executable nicht gefunden

[...]

---

## 4. Schnellreferenz

| Code | Schwere | Kategorie | Description |
|------|---------|-----------|--------------|
| E0101 | ⛔ | Core | JSON nicht gefunden |
| E0102 | ⛔ | Core | JSON-Parse-Error |
| E0501 | ⛔ | Executable | Executable nicht gefunden |

---

## 5. Usage in Code

```cmake
include(cmake/core/Errors.cmake)

# Error auslösen
cmake_fatal(E0101 "Solution.json not found")

# Warning auslösen  
cmake_warn(W0201 "Deprecated option used")
```

---

## 6. See Also

- [Errors.cmake](../modules/core/Errors_cmake_v0_1_1_doc_v1.md)
- [Debug.cmake](../modules/core/Debug_cmake_v0_1_1_doc_v1.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.1** | **2025-12-13** | **E20xx Kategorie hinzugefügt** |
| 0.1.0 | 2025-12-03 | Initial |
```

---

## 8. Review Checklist

Zusätzlich zur Doc.md Checkliste:

**Vollständigkeit:**
- [ ] Alle Einträge dokumentiert
- [ ] Keine fehlenden Kategorien
- [ ] Schnellreferenz vollständig

**Format:**
- [ ] Einheitliches Eintragsformat
- [ ] Konventionen/Legende vorhanden
- [ ] Usagesbeispiele

**Navigation:**
- [ ] Kategorisierung sinnvoll
- [ ] Schnellreferenz-Tabelle vorhanden
- [ ] Querverweise funktionieren

---

## 9. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [ModuleDoc.md](ModuleDoc.md) — Für Modul-Dokumentation
- [Guide.md](Guide.md) — Für Anleitungen

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Eintragsformate für Schema/ErrorCodes/Glossar, Schnellreferenz-Tabellen** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
