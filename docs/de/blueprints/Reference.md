# Reference — Standard für API/Schema-Referenzen

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Doc v0.5, Blueprint v0.5  
> **Zielgruppe:** Dokumentations-Ersteller  
> **Sprache:** Deutsch  
> **English:** [Reference.md](../../en/blueprints/Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Header-Struktur](#3-header-struktur)
4. [Pflichtabschnitte](#4-pflichtabschnitte)
5. [Eintragsformat](#5-eintragsformat)
6. [Schnellreferenz-Tabellen](#6-schnellreferenz-tabellen)
7. [Beispiel: Vollständige Reference](#7-beispiel-vollständige-reference)
8. [Review-Checkliste](#8-review-checkliste)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert die **Struktur für Referenz-Dokumentationen**. Referenzen sind Nachschlagewerke, die vollständige, strukturierte Informationen zu einem Thema liefern.

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

## 2. Geltungsbereich

Dieser Blueprint gilt für:
- Schema-Dokumentationen (`Solution_Schema`, `CMakePresets`)
- Error-Code-Referenzen
- Glossare
- API-Übersichten (nicht Module — dafür ModuleDoc)

**Typischer Ordner:** `docs/[lang]/reference/`

---

## 3. Header-Struktur

### 3.1 Standard-Header

```markdown
# [Name] — Referenz

> **Version:** X.Y.Z  
> **Datum:** YYYY-MM-DD  
> **Typ:** Reference  
> **Status:** [In Entwicklung | Stabil | Deprecated]  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [Name.md](../../en/reference/Name.md)
```

### 3.2 Keine zusätzlichen Pflichtfelder

Reference verwendet den Standard-Header aus Doc.md.

---

## 4. Pflichtabschnitte

Referenzen verwenden diese Struktur:

```
## Inhaltsverzeichnis

## 1. Übersicht
## 2. Konventionen
## 3. [Kategorisierte Einträge]
## 4. Schnellreferenz
## 5. Verwendung in Code
## 6. Siehe auch
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Übersicht | Was wird dokumentiert? Zweck |
| 2 | Konventionen | Notation, Legende, Abkürzungen |
| 3 | Einträge | Kategorisierte, vollständige Auflistung |
| 4 | Schnellreferenz | Kompakte Tabelle aller Einträge |
| 5 | Verwendung | Code-Beispiele für typische Anwendung |
| 6 | Siehe auch | Verwandte Dokumente |

---

## 5. Eintragsformat

### 5.1 Für Schema-Felder

```markdown
### 3.1 `feldname`

| Aspekt | Wert |
|--------|------|
| **Typ** | `string` / `number` / `boolean` / `object` / `array` |
| **Pflicht** | ✓ / — |
| **Default** | `"wert"` / — |
| **Seit** | v0.1.0 |

**Beschreibung:**  
Was dieses Feld macht.

**Gültige Werte:**
- `"option1"` — Beschreibung
- `"option2"` — Beschreibung

**Beispiel:**
```json
"feldname": "option1"
```

**Hinweise:**
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

**Verwandte Fehler:**
- [E0502](#e0502) — Executable-Typ ungültig
```

### 5.3 Für Glossar-Einträge

```markdown
### Context

**Definition:**  
Ein isolierter Namespace zum Sammeln von Daten während der CMake-Verarbeitung.

**Verwendung:**
```cmake
ctx_create(EXE_MyApp)
ctx_set(EXE_MyApp NAME "MyApp")
```

**Siehe auch:** [Context.cmake](../modules/core/Context_cmake_v0_1_1_doc_v1.md)
```

---

## 6. Schnellreferenz-Tabellen

### 6.1 Zweck

Am Ende des Hauptteils eine kompakte Übersicht aller Einträge.

### 6.2 Format für Schema

```markdown
## 4. Schnellreferenz

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | ✓ | — | Eindeutiger Name |
| `version` | string | — | `"0.1.0"` | SemVer-Version |
| `type` | string | — | `"CONSOLE"` | CONSOLE / GUI |
```

### 6.3 Format für Error Codes

```markdown
## 4. Schnellreferenz

| Code | Kategorie | Beschreibung |
|------|-----------|--------------|
| E0101 | Core | Json-Datei nicht gefunden |
| E0102 | Core | Json-Parse-Fehler |
| E0501 | Executable | Executable nicht gefunden |
| E0502 | Executable | Ungültiger Typ |
```

---

## 7. Beispiel: Vollständige Reference

```markdown
# ErrorCodes — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [ErrorCodes.md](../../en/reference/ErrorCodes.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Error Codes nach Kategorie](#3-error-codes-nach-kategorie)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert alle Error Codes des CMake Architecture 
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

| Präfix | Kategorie | Beschreibung |
|--------|-----------|--------------|
| E01xx | Core | Grundlegende Fehler |
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

| Code | Schwere | Kategorie | Beschreibung |
|------|---------|-----------|--------------|
| E0101 | ⛔ | Core | JSON nicht gefunden |
| E0102 | ⛔ | Core | JSON-Parse-Fehler |
| E0501 | ⛔ | Executable | Executable nicht gefunden |

---

## 5. Verwendung in Code

```cmake
include(cmake/core/Errors.cmake)

# Fehler auslösen
cmake_fatal(E0101 "Solution.json not found")

# Warnung auslösen  
cmake_warn(W0201 "Deprecated option used")
```

---

## 6. Siehe auch

- [Errors.cmake](../modules/core/Errors_cmake_v0_1_1_doc_v1.md)
- [Debug.cmake](../modules/core/Debug_cmake_v0_1_1_doc_v1.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.1** | **2025-12-13** | **E20xx Kategorie hinzugefügt** |
| 0.1.0 | 2025-12-03 | Initial |
```

---

## 8. Review-Checkliste

Zusätzlich zur Doc.md Checkliste:

**Vollständigkeit:**
- [ ] Alle Einträge dokumentiert
- [ ] Keine fehlenden Kategorien
- [ ] Schnellreferenz vollständig

**Format:**
- [ ] Einheitliches Eintragsformat
- [ ] Konventionen/Legende vorhanden
- [ ] Verwendungsbeispiele

**Navigation:**
- [ ] Kategorisierung sinnvoll
- [ ] Schnellreferenz-Tabelle vorhanden
- [ ] Querverweise funktionieren

---

## 9. Siehe auch

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [ModuleDoc.md](ModuleDoc.md) — Für Modul-Dokumentation
- [Guide.md](Guide.md) — Für Anleitungen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Eintragsformate für Schema/ErrorCodes/Glossar, Schnellreferenz-Tabellen** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
