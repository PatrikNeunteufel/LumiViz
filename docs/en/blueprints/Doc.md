# Doc — Standard für alle Dokumentationen

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Doc.md](../../en/blueprints/Doc.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Dokumentations-Typen](#3-dokumentations-typen)
4. [Header-Struktur](#4-header-struktur)
5. [Table of Contents](#5-inhaltsverzeichnis)
6. [Kapitel-Nummerierung](#6-kapitel-nummerierung)
7. [Anker-Konventionen](#7-anker-konventionen)
8. [Dateinamen-Konventionen](#8-dateinamen-konventionen)
9. [Sprache und Übersetzung](#9-sprache-und-übersetzung)
10. [Formatierungs-Regeln](#10-formatierungs-regeln)
11. [Changelog-Format](#11-changelog-format)
12. [Review Checklist](#12-review-checkliste)
13. [See Also](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Overview

This blueprint defines **verbindliche Regeln für alle Dokumentationen** im CMake Architecture Projekt. Er dient als Basis für spezialisierte Blueprints (ModuleDoc, Guide, Reference, etc.).

### Zielgruppe

- Alle Personen, die Dokumentation erstellen oder pflegen
- Spezialisierte Blueprints, die von diesem erben

### Was dieser Blueprint regelt

| Bereich | Regeln |
|---------|--------|
| Header | Requiredfelder, Format, Reihenfolge |
| TOC | Nummerierung, Anker, Platzierung |
| Sprache | German/Englisch, Ordnerstruktur |
| Format | Überschriften, Tabellen, Code-Blöcke |

---

## 2. Scope

Dieser Blueprint gilt für **alle** Markdown-Dokumentationen:

| Typ | Examples | Spezialisierter Blueprint |
|-----|-----------|---------------------------|
| CMake-Module | Context.cmake Doku | ModuleDoc.md |
| Benutzerhandbücher | Qt6 Integration | Guide.md |
| Referenceen | Solution_Schema | Reference.md |
| Concepte | AppContainer | Concept.md |
| Standards | Cpp_Coding_Standard | Standard.md |
| Tutorials | CreateFirstApp | Tutorial.md |

---

## 3. Dokumentations-Typen

### 3.1 Overview

| Typ | Zielgruppe | Fokus |
|-----|------------|-------|
| **ModuleDoc** | Build System Developers | Wie funktioniert das Modul? |
| **Guide** | C++ Developers (End Users) | Wie nutze ich das System? |
| **Reference** | Alle Entwickler | Nachschlagewerk, API |
| **Concept** | Build System Developers | Architecture, Design |
| **Standard** | Alle Entwickler | Coding-Konventionen |
| **Tutorial** | Einsteiger | Schritt-für-Schritt Anleitung |

### 3.2 Wann welcher Typ?

```
"Wie konfiguriere ich Qt6?"          → Guide
"Was bedeutet Error E0501?"          → Reference
"Wie funktioniert Context.cmake?"    → ModuleDoc
"Warum nutzen wir GLOBAL PROPERTY?"  → Concept
"Wie benenne ich Variablen?"         → Standard
"Erstelle dein erstes Executable"    → Tutorial
```

---

## 4. Header-Struktur

### 4.1 Basis-Header (alle Dokumentationen)

Jede Dokumentation **MUSS** mit diesem Header beginnen:

```markdown
# [Titel]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** [ModuleDoc | Guide | Reference | Concept | Standard | Tutorial]  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** [Wer soll dieses Dokument lesen?]  
> **Language:** English  
> **German:** [Dateiname.md](../../en/[ordner]/Dateiname.md)
```

### 4.2 Header-Felder

| Feld | Required | Format | Example |
|------|---------|--------|----------|
| **Version** | ✓ | SemVer | `1.2.3` |
| **Datum** | ✓ | ISO 8601 | `2025-12-13` |
| **Typ** | ✓ | Vordefiniert | `Guide` |
| **Status** | ✓ | Vordefiniert | `Stable` |
| **Zielgruppe** | ✓ | Kurzbeschreibung | `C++ Developers` |
| **Sprache** | ✓ | Vordefiniert | `German` |
| **English** | ✓ | Relativer Pfad | `[...](.../.../en/...)` |
| **Basiert auf** | Optional | Name + Version | `Doc v0.5` |

### 4.3 Zielgruppe — Werte und Erläuterung

Die Zielgruppe im Header ist eine **Kurzform**. Im ersten Abschnitt (Overview) kann sie detaillierter beschrieben werden.

**Typische Zielgruppen:**

| Kurzform | Description |
|----------|--------------|
| `Build System Developers` | Entwickelt/erweitert das CMake-System selbst |
| `C++ Developers` | Nutzt das Build-System für eigene Projekte |
| `Alle Entwickler` | Sowohl Build-System- als auch C++-Entwickler |
| `Documentation Authors` | Schreibt/pflegt Dokumentation |
| `Neue Team-Mitglieder` | Onboarding, Einstieg ins Projekt |
| `Architekten` | Technische Entscheidungsträger |

### 4.3 Erweiterte Header-Felder

Spezialisierte Blueprints definieren zusätzliche Felder:

| Blueprint | Zusätzliche Felder |
|-----------|-------------------|
| ModuleDoc | `Module:`, `Module Version:` |
| Guide | `Module:` (optional) |
| Reference | — |
| Concept | — |
| Standard | `Scope:` |

### 4.4 English-Link Format

Der Link zur englischen Version folgt dem Muster:

```markdown
> **German:** [Dateiname.md](../../en/[ordner]/Dateiname.md)
```

| Ordner im deutschen | Ordner im englischen |
|---------------------|---------------------|
| `de/modules/core/` | `en/modules/core/` |
| `de/guides/` | `en/guides/` |
| `de/blueprints/` | `en/blueprints/` |

**Important:** Dateinamen sind **immer auf Englisch** (auch im deutschen Ordner).

---

## 5. Table of Contents

### 5.1 Required

**Jede Dokumentation MUSS ein Table of Contents haben.**

### 5.2 Platzierung

Das Table of Contents steht **direkt nach dem Header**, getrennt durch horizontalen Trenner:

```markdown
> **German:** [...]

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Installation](#2-installation)
...

---

## 1. Overview
```

### 5.3 Format

- Nummerierte Liste (1., 2., 3., ...)
- Jeder Eintrag ist ein Link zum Anker
- Überschrift ist immer `## Table of Contents` (nicht nummeriert)

### 5.4 Example

```markdown
## Table of Contents

1. [Overview](#1-übersicht)
2. [Installation](#2-installation)
   - [Windows](#21-windows)
   - [Linux](#22-linux)
3. [Configuration](#3-konfiguration)
4. [Troubleshooting](#4-troubleshooting)
5. [Changelog](#5-changelog)
```

---

## 6. Kapitel-Nummerierung

### 6.1 Schema

Alle H2-Überschriften (`##`) werden nummeriert:

```markdown
## 1. Overview
## 2. Installation  
## 3. Configuration
```

### 6.2 Unterabschnitte

H3-Überschriften (`###`) erhalten verschachtelte Nummerierung:

```markdown
## 2. Installation

### 2.1 Windows

### 2.2 Linux

#### 2.2.1 Ubuntu
```

### 6.3 Ausnahmen

**Nicht nummeriert** werden:
- `## Table of Contents`
- `## Changelog`
- `## See Also`

---

## 7. Anker-Konventionen

### 7.1 Automatische Anker

GitHub/GitLab generieren Anker automatisch:

| Überschrift | Generierter Anker |
|-------------|-------------------|
| `## 1. Overview` | `#1-übersicht` |
| `### 2.1 Windows` | `#21-windows` |
| `## Troubleshooting` | `#troubleshooting` |

### 7.2 Regeln

1. **Lowercase:** Alles kleingeschrieben
2. **Bindestriche:** Leerzeichen werden zu `-`
3. **Umlaute:** Bleiben erhalten (ü, ä, ö)
4. **Sonderzeichen:** Punkte, Doppelpunkte werden entfernt
5. **Zahlen:** Bleiben erhalten

### 7.3 Examples

| Überschrift | Anker |
|-------------|-------|
| `## 1. Overview` | `#1-übersicht` |
| `### 2.1 Windows-Installation` | `#21-windows-installation` |
| `## Was ist Solution.json?` | `#was-ist-solutionjson` |
| `## API-Reference` | `#api-referenz` |

### 7.4 Testen

Anker vor Commit testen:
1. Markdown lokal rendern (VS Code Preview)
2. Auf Links im Table of Contents klicken
3. Alle Links müssen funktionieren

---

## 8. Dateinamen-Konventionen

### 8.1 Grundprinzip: Version im Header, nicht im Dateinamen

> **Important:** Aktuelle Dokumente haben **keine Version im Dateinamen**.  
> Die Version steht nur im Header. Dadurch müssen Links bei Version-Updates nicht angepasst werden.

| Datei-Art | Dateiname | Version |
|-----------|-----------|---------|
| **Aktuell** | `Doc.md` | Im Header: `> **Version:** 1.0.0` |
| **Archiviert** | `Doc_v0_4_0.md` | Im Dateinamen + Header |

### 8.2 Allgemeine Regeln

| Regel | Richtig | Falsch |
|-------|---------|--------|
| Englisch | `Getting_Started.md` | `Erste_Schritte.md` |
| Keine Version (aktuell) | `Doc.md` | `Doc_v0_5_0.md` |
| Underscores als Trenner | `Error_Codes.md` | `ErrorCodes.md` |
| Keine Leerzeichen | `Error_Codes.md` | `Error Codes.md` |
| **Großbuchstabe am Anfang** | `Context.md` | `context.md` |
| **CamelCase bei Komposita** | `Solution_Schema.md` | `solution_schema.md` |
| **Wörter mit Underscore trennen** | `Getting_Started.md` | `GettingStarted.md` |

**Summary Dateinamen:**

- Erster Buchstabe **großgeschrieben**
- Wörter mit **Underscore** (`_`) getrennt
- Jedes Wort beginnt mit **Großbuchstaben** (Title_Case)
- Endung `.md` kleingeschrieben

**Examples:**

| Typ | Example |
|-----|----------|
| Blueprint | `Doc.md`, `CMake.md` |
| ModuleDoc | `Context_cmake.md`, `Errors_cmake.md` |
| Guide | `Qt6_Integration_UserGuide.md` |
| Reference | `Solution_Schema.md`, `ErrorCodes.md` |
| Concept | `AppContainer_Concept.md` |

### 8.3 Aktuelle Dateien (ohne Version)

| Typ | Muster | Example |
|-----|--------|----------|
| Blueprint | `[Name].md` | `Doc.md`, `CMake.md` |
| ModuleDoc | `[Modul].md` | `Context.md`, `Errors.md` |
| Guide | `[Name]_UserGuide.md` | `Qt6_Integration_UserGuide.md` |
| Reference | `[Name].md` | `Solution_Schema.md`, `ErrorCodes.md` |
| Concept | `[Name]_Concept.md` | `AppContainer_Concept.md` |
| Standard | `[Name]_Standard.md` | `Cpp_Coding_Standard.md` |

### 8.4 Archivierte Dateien (mit Version)

Bei Major- oder Minor-Updates wird die alte Version archiviert:

| Typ | Muster | Example |
|-----|--------|----------|
| Blueprint | `[Name]_v[X]_[Y]_[Z].md` | `Doc_v0_4_0.md` |
| ModuleDoc | `[Modul]_v[X]_[Y]_[Z].md` | `Context_v0_1_0.md` |
| Guide | `[Name]_UserGuide_v[X]_[Y]_[Z].md` | `Qt6_Integration_UserGuide_v0_1_0.md` |

### 8.5 Versionierung im Dateinamen (nur Archiv)

| Regel | Richtig | Falsch |
|-------|---------|--------|
| Lowercase `v` | `_v0_5_0` | `_V0_5_0` |
| Underscores | `_v0_5_0` | `_v0.5.0` |
| Am Ende vor `.md` | `Doc_v0_5_0.md` | `v0_5_0_Doc.md` |

### 8.6 Archivierungs-Workflow

1. **Vor Major/Minor-Update:**
   - Aktuelle Datei kopieren: `Doc.md` → `Doc_v0_4_0.md`
   - In Archiv-Ordner verschieben (optional)

2. **Nach Update:**
   - `Doc.md` enthält neue Version
   - Links bleiben unverändert

3. **Wann archivieren?**
   - Bei Breaking Changes (MAJOR)
   - Bei signifikanten Changes (MINOR)
   - PATCH-Updates: Keine Archivierung nötig

### 8.7 Vorteile

| Vorteil | Description |
|---------|--------------|
| **Stablee Links** | URLs ändern sich nicht bei Updates |
| **Einfachere Wartung** | Keine Link-Updates in anderen Dokumenten |
| **Klare Aktualität** | Datei ohne Version = aktuelle Version |
| **Historische Versionen** | Bei Bedarf im Archiv verfügbar |

### 8.8 Ordnernamen-Konvention

Ordnernamen werden **immer kleingeschrieben**.

| Regel | Richtig | Falsch |
|-------|---------|--------|
| Kleingeschrieben | `modules/` | `Modules/` |
| Kleingeschrieben | `blueprints/` | `Blueprints/` |
| Kleingeschrieben | `hooks/prefetch/` | `Hooks/PreFetch/` |
| snake_case bei Bedarf | `user_guides/` | `UserGuides/` |

**Begründung:**

| Grund | Description |
|-------|--------------|
| **Cross-Platform** | Linux ist case-sensitive, Windows nicht — Kleinschreibung vermeidet Probleme |
| **Konsistenz** | Einheitlich einfacher zu merken |
| **URLs** | Web-URLs sind typisch kleingeschrieben |
| **Git** | Keine Case-Konflikte bei Renames |

**Examples:**

```markdown
docs/
├── de/
│   ├── blueprints/        ✓ (nicht: Blueprints/)
│   ├── modules/
│   │   ├── core/          ✓ (nicht: Core/)
│   │   └── externals/
│   └── reference/
cmake/
└── externals/
├── includes/          ✓ (nicht: Includes/)
└── hooks/
├── prefetch/      ✓ (nicht: PreFetch/)
└── postfetch/     ✓ (nicht: PostFetch/)
```

### 8.9 Ausnahme: Projekt-Ordner

**Projekt-Ordner** folgen dem **Target-Namen**, nicht der Kleinschreibung:

| Regel | Example | Begründung |
|-------|----------|------------|
| Ordnername = Target-Name | `projects/apps/DemoPlayer/` | Konsistenz zwischen Ordner und CMake-Target |
| Ordnername = Target-Name | `projects/exec/MinimalConsole/` | IDE-Navigation, Zuordnung |
| Ordnername = Target-Name | `projects/libs/BasicLogger/` | Klare 1:1-Beziehung |

**Betroffene Pfade:**
```
projects/
├── apps/
│   └── DemoPlayer/        ← Target: DemoPlayer, DemoPlayer.Core
├── exec/
│   └── MinimalConsole/    ← Target: MinimalConsole
├── libs/
│   └── BasicLogger/       ← Target: BasicLogger
└── tests/
    └── unit/
        └── CoreLib_Tests/ ← Target: CoreLib_Tests
```

**Nicht betroffen** (weiterhin lowercase):

- `projects/` selbst
- Typ-Unterordner: `apps/`, `exec/`, `libs/`, `tests/`
- Alle System-Ordner: `cmake/`, `docs/`, `externals/`

---

## 9. Sprache und Übersetzung

### 9.1 Ordnerstruktur

```
docs/
├── de/                    ← Germane Dokumentation
│   ├── blueprints/
│   ├── modules/
│   └── guides/
│
└── en/                    ← Englische Dokumentation
    ├── blueprints/
    ├── modules/
    └── guides/
```

### 9.2 Primärsprache

- **German** ist Primärsprache (Entwickler sind deutschsprachig)
- Englische Versionen sind Übersetzungen

### 9.3 Dateinamen

Dateinamen sind **immer auf Englisch**, auch im deutschen Ordner:

```
de/guides/Qt6_Integration_UserGuide.md   ← Inhalt auf German
en/guides/Qt6_Integration_UserGuide.md   ← Inhalt auf Englisch
```

**Note:** Keine Version im Dateinamen (siehe Abschnitt 8).

### 9.4 Querverweise

- Innerhalb derselben Language: Relativer Pfad
- Zur anderen Language: Via `../../en/` oder `../../de/`

---

## 10. Formatierungs-Regeln

### 10.1 Überschriften

```markdown
# Haupttitel (H1) — nur einmal pro Dokument

## Nummerierter Abschnitt (H2)

### Unterabschnitt (H3)

#### Detail (H4) — sparsam verwenden
```

### 10.2 Horizontale Trenner

**Vor jedem H2-Abschnitt** (`##`):

```markdown
---

## 2. Installation
```

### 10.3 Tabellen

Mit Header-Zeile und Ausrichtung:

```markdown
| Spalte 1 | Spalte 2 | Description |
|----------|----------|--------------|
| Wert 1   | Wert 2   | Text         |
```

### 10.4 Code-Blöcke

Mit Sprach-Annotation:

````markdown
```cmake
function(my_function)
endfunction()
```

```json
{ "key": "value" }
```
````

### 10.5 Hervorhebungen

| Element | Usage |
|---------|------------|
| **Fett** | Importante Begriffe |
| `Code` | Variablen, Functions, Pfade |
| *Kursiv* | Zitate, Fachbegriffe (selten) |
| ~~Durchgestrichen~~ | Deprecated |
| > Blockquote | Notee, Warningen |

### 10.6 Notee

```markdown
> **Note:** Allgemeine Information

> **Important:** Kritische Information  

> **Warning:** Potenzielle Probleme

> **DEPRECATED seit vX.Y:** Mit Migrationshinweis
```

---

## 11. Changelog-Format

### 11.1 Position

Der Changelog steht **immer am Ende** des Dokuments.

### 11.2 Format

```markdown
---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **X.Y.Z** | **YYYY-MM-DD** | **Aktuelle Änderung (fett)** |
| X.Y.Z-1 | YYYY-MM-DD | Vorherige Änderung |
| 0.1.0 | YYYY-MM-DD | Initial |
```

### 11.3 Regeln

- Neueste Version zuerst
- Aktuelle Version **fett**
- Kurze, prägnante Descriptionen
- Breaking Changes mit ⚠️ kennzeichnen

---

## 12. Review Checklist

Vor Fertigstellung einer Dokumentation prüfen:

**Header:**
- [ ] Alle Requiredfelder vorhanden
- [ ] Version und Datum aktuell
- [ ] Typ korrekt
- [ ] Zielgruppe angegeben
- [ ] English-Link vorhanden und korrekt

**Table of Contents:**
- [ ] Vorhanden und vollständig
- [ ] Nummerierung korrekt
- [ ] Alle Anker funktionieren

**Struktur:**
- [ ] Kapitel nummeriert (außer TOC, Changelog, See Also)
- [ ] Horizontale Trenner vor H2
- [ ] Changelog am Ende

**Inhalt:**
- [ ] Zielgruppe in Overview detailliert (falls nötig)
- [ ] Code-Examples getestet
- [ ] Keine TODO/FIXME übrig
- [ ] Konsistente Terminologie
- [ ] Querverweise mit Versionen

**Datei:**
- [ ] Dateiname folgt Konvention
- [ ] Im richtigen Ordner

---

## 13. See Also

- [Blueprint.md](Blueprint.md) — Meta-Blueprint
- [ModuleDoc.md](ModuleDoc.md) — CMake-Modul-Dokumentation
- [Guide.md](Guide.md) — Benutzerhandbücher
- [Reference.md](Reference.md) — API/Schema-Referenceen

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-17** | **Zusatzartikel zu Ordnernamen für Projekte unter § 8.9** |
| 0.5.1 | 2025-12-15 | Explizite Dateinamen-Konvention (§ 8.2): Title_Case. Neue Ordnernamen-Konvention (§ 8.8): immer kleingeschrieben |
| 0.5.0 | 2025-12-13 | Neu: Required-Table of Contents mit Ankern, Zielgruppe-Requiredfeld im Header, English-Link im Header, Kapitel-Nummerierung, Anker-Konventionen |
| 0.1.0 | 2025-12-03 | Initial (als Documentation_Blueprint) |
