# Doc — Standard für alle Dokumentationen

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Blueprint v0.5  
> **Zielgruppe:** Dokumentations-Ersteller  
> **Sprache:** Deutsch  
> **English:** [Doc.md](../../en/blueprints/Doc.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Dokumentations-Typen](#3-dokumentations-typen)
4. [Header-Struktur](#4-header-struktur)
5. [Inhaltsverzeichnis](#5-inhaltsverzeichnis)
6. [Kapitel-Nummerierung](#6-kapitel-nummerierung)
7. [Anker-Konventionen](#7-anker-konventionen)
8. [Dateinamen-Konventionen](#8-dateinamen-konventionen)
9. [Sprache und Übersetzung](#9-sprache-und-übersetzung)
10. [Formatierungs-Regeln](#10-formatierungs-regeln)
11. [Changelog-Format](#11-changelog-format)
12. [Review-Checkliste](#12-review-checkliste)
13. [Siehe auch](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert **verbindliche Regeln für alle Dokumentationen** im CMake Architecture Projekt. Er dient als Basis für spezialisierte Blueprints (ModuleDoc, Guide, Reference, etc.).

### Zielgruppe

- Alle Personen, die Dokumentation erstellen oder pflegen
- Spezialisierte Blueprints, die von diesem erben

### Was dieser Blueprint regelt

| Bereich | Regeln |
|---------|--------|
| Header | Pflichtfelder, Format, Reihenfolge |
| TOC | Nummerierung, Anker, Platzierung |
| Sprache | Deutsch/Englisch, Ordnerstruktur |
| Format | Überschriften, Tabellen, Code-Blöcke |

---

## 2. Geltungsbereich

Dieser Blueprint gilt für **alle** Markdown-Dokumentationen:

| Typ | Beispiele | Spezialisierter Blueprint |
|-----|-----------|---------------------------|
| CMake-Module | Context.cmake Doku | ModuleDoc.md |
| Benutzerhandbücher | Qt6 Integration | Guide.md |
| Referenzen | Solution_Schema | Reference.md |
| Konzepte | AppContainer | Concept.md |
| Standards | Cpp_Coding_Standard | Standard.md |
| Tutorials | CreateFirstApp | Tutorial.md |

---

## 3. Dokumentations-Typen

### 3.1 Übersicht

| Typ | Zielgruppe | Fokus |
|-----|------------|-------|
| **ModuleDoc** | Build-System-Entwickler | Wie funktioniert das Modul? |
| **Guide** | C++ Entwickler (Endnutzer) | Wie nutze ich das System? |
| **Reference** | Alle Entwickler | Nachschlagewerk, API |
| **Concept** | Build-System-Entwickler | Architektur, Design |
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
> **Datum:** YYYY-MM-DD  
> **Typ:** [ModuleDoc | Guide | Reference | Concept | Standard | Tutorial]  
> **Status:** [In Entwicklung | Stabil | Deprecated]  
> **Zielgruppe:** [Wer soll dieses Dokument lesen?]  
> **Sprache:** Deutsch  
> **English:** [Dateiname.md](../../en/[ordner]/Dateiname.md)
```

### 4.2 Header-Felder

| Feld | Pflicht | Format | Beispiel |
|------|---------|--------|----------|
| **Version** | ✓ | SemVer | `1.2.3` |
| **Datum** | ✓ | ISO 8601 | `2025-12-13` |
| **Typ** | ✓ | Vordefiniert | `Guide` |
| **Status** | ✓ | Vordefiniert | `Stabil` |
| **Zielgruppe** | ✓ | Kurzbeschreibung | `C++ Entwickler` |
| **Sprache** | ✓ | Vordefiniert | `Deutsch` |
| **English** | ✓ | Relativer Pfad | `[...](.../.../en/...)` |
| **Basiert auf** | Optional | Name + Version | `Doc v0.5` |

### 4.3 Zielgruppe — Werte und Erläuterung

Die Zielgruppe im Header ist eine **Kurzform**. Im ersten Abschnitt (Übersicht) kann sie detaillierter beschrieben werden.

**Typische Zielgruppen:**

| Kurzform | Beschreibung |
|----------|--------------|
| `Build-System-Entwickler` | Entwickelt/erweitert das CMake-System selbst |
| `C++ Entwickler` | Nutzt das Build-System für eigene Projekte |
| `Alle Entwickler` | Sowohl Build-System- als auch C++-Entwickler |
| `Dokumentations-Ersteller` | Schreibt/pflegt Dokumentation |
| `Neue Team-Mitglieder` | Onboarding, Einstieg ins Projekt |
| `Architekten` | Technische Entscheidungsträger |

### 4.3 Erweiterte Header-Felder

Spezialisierte Blueprints definieren zusätzliche Felder:

| Blueprint | Zusätzliche Felder |
|-----------|-------------------|
| ModuleDoc | `Modul:`, `Modul-Version:` |
| Guide | `Modul:` (optional) |
| Reference | — |
| Concept | — |
| Standard | `Geltungsbereich:` |

### 4.4 English-Link Format

Der Link zur englischen Version folgt dem Muster:

```markdown
> **English:** [Dateiname.md](../../en/[ordner]/Dateiname.md)
```

| Ordner im deutschen | Ordner im englischen |
|---------------------|---------------------|
| `de/modules/core/` | `en/modules/core/` |
| `de/guides/` | `en/guides/` |
| `de/blueprints/` | `en/blueprints/` |

**Wichtig:** Dateinamen sind **immer auf Englisch** (auch im deutschen Ordner).

---

## 5. Inhaltsverzeichnis

### 5.1 Pflicht

**Jede Dokumentation MUSS ein Inhaltsverzeichnis haben.**

### 5.2 Platzierung

Das Inhaltsverzeichnis steht **direkt nach dem Header**, getrennt durch horizontalen Trenner:

```markdown
> **English:** [...]

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Installation](#2-installation)
...

---

## 1. Übersicht
```

### 5.3 Format

- Nummerierte Liste (1., 2., 3., ...)
- Jeder Eintrag ist ein Link zum Anker
- Überschrift ist immer `## Inhaltsverzeichnis` (nicht nummeriert)

### 5.4 Beispiel

```markdown
## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Installation](#2-installation)
   - [Windows](#21-windows)
   - [Linux](#22-linux)
3. [Konfiguration](#3-konfiguration)
4. [Troubleshooting](#4-troubleshooting)
5. [Changelog](#5-changelog)
```

---

## 6. Kapitel-Nummerierung

### 6.1 Schema

Alle H2-Überschriften (`##`) werden nummeriert:

```markdown
## 1. Übersicht
## 2. Installation  
## 3. Konfiguration
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
- `## Inhaltsverzeichnis`
- `## Changelog`
- `## Siehe auch`

---

## 7. Anker-Konventionen

### 7.1 Automatische Anker

GitHub/GitLab generieren Anker automatisch:

| Überschrift | Generierter Anker |
|-------------|-------------------|
| `## 1. Übersicht` | `#1-übersicht` |
| `### 2.1 Windows` | `#21-windows` |
| `## Troubleshooting` | `#troubleshooting` |

### 7.2 Regeln

1. **Lowercase:** Alles kleingeschrieben
2. **Bindestriche:** Leerzeichen werden zu `-`
3. **Umlaute:** Bleiben erhalten (ü, ä, ö)
4. **Sonderzeichen:** Punkte, Doppelpunkte werden entfernt
5. **Zahlen:** Bleiben erhalten

### 7.3 Beispiele

| Überschrift | Anker |
|-------------|-------|
| `## 1. Übersicht` | `#1-übersicht` |
| `### 2.1 Windows-Installation` | `#21-windows-installation` |
| `## Was ist Solution.json?` | `#was-ist-solutionjson` |
| `## API-Referenz` | `#api-referenz` |

### 7.4 Testen

Anker vor Commit testen:
1. Markdown lokal rendern (VS Code Preview)
2. Auf Links im Inhaltsverzeichnis klicken
3. Alle Links müssen funktionieren

---

## 8. Dateinamen-Konventionen

### 8.1 Grundprinzip: Version im Header, nicht im Dateinamen

> **Wichtig:** Aktuelle Dokumente haben **keine Version im Dateinamen**.  
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

**Zusammenfassung Dateinamen:**

- Erster Buchstabe **großgeschrieben**
- Wörter mit **Underscore** (`_`) getrennt
- Jedes Wort beginnt mit **Großbuchstaben** (Title_Case)
- Endung `.md` kleingeschrieben

**Beispiele:**

| Typ | Beispiel |
|-----|----------|
| Blueprint | `Doc.md`, `CMake.md` |
| ModuleDoc | `Context_cmake.md`, `Errors_cmake.md` |
| Guide | `Qt6_Integration_UserGuide.md` |
| Reference | `Solution_Schema.md`, `ErrorCodes.md` |
| Concept | `AppContainer_Concept.md` |

### 8.3 Aktuelle Dateien (ohne Version)

| Typ | Muster | Beispiel |
|-----|--------|----------|
| Blueprint | `[Name].md` | `Doc.md`, `CMake.md` |
| ModuleDoc | `[Modul].md` | `Context.md`, `Errors.md` |
| Guide | `[Name]_UserGuide.md` | `Qt6_Integration_UserGuide.md` |
| Reference | `[Name].md` | `Solution_Schema.md`, `ErrorCodes.md` |
| Concept | `[Name]_Concept.md` | `AppContainer_Concept.md` |
| Standard | `[Name]_Standard.md` | `Cpp_Coding_Standard.md` |

### 8.4 Archivierte Dateien (mit Version)

Bei Major- oder Minor-Updates wird die alte Version archiviert:

| Typ | Muster | Beispiel |
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
   - Bei signifikanten Änderungen (MINOR)
   - PATCH-Updates: Keine Archivierung nötig

### 8.7 Vorteile

| Vorteil | Beschreibung |
|---------|--------------|
| **Stabile Links** | URLs ändern sich nicht bei Updates |
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

| Grund | Beschreibung |
|-------|--------------|
| **Cross-Platform** | Linux ist case-sensitive, Windows nicht — Kleinschreibung vermeidet Probleme |
| **Konsistenz** | Einheitlich einfacher zu merken |
| **URLs** | Web-URLs sind typisch kleingeschrieben |
| **Git** | Keine Case-Konflikte bei Renames |

**Beispiele:**

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

| Regel | Beispiel | Begründung |
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
├── de/                    ← Deutsche Dokumentation
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

- **Deutsch** ist Primärsprache (Entwickler sind deutschsprachig)
- Englische Versionen sind Übersetzungen

### 9.3 Dateinamen

Dateinamen sind **immer auf Englisch**, auch im deutschen Ordner:

```
de/guides/Qt6_Integration_UserGuide.md   ← Inhalt auf Deutsch
en/guides/Qt6_Integration_UserGuide.md   ← Inhalt auf Englisch
```

**Hinweis:** Keine Version im Dateinamen (siehe Abschnitt 8).

### 9.4 Querverweise

- Innerhalb derselben Sprache: Relativer Pfad
- Zur anderen Sprache: Via `../../en/` oder `../../de/`

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
| Spalte 1 | Spalte 2 | Beschreibung |
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

| Element | Verwendung |
|---------|------------|
| **Fett** | Wichtige Begriffe |
| `Code` | Variablen, Funktionen, Pfade |
| *Kursiv* | Zitate, Fachbegriffe (selten) |
| ~~Durchgestrichen~~ | Deprecated |
| > Blockquote | Hinweise, Warnungen |

### 10.6 Hinweise

```markdown
> **Hinweis:** Allgemeine Information

> **Wichtig:** Kritische Information  

> **Warnung:** Potenzielle Probleme

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

| Version | Datum | Änderungen |
|---------|-------|------------|
| **X.Y.Z** | **YYYY-MM-DD** | **Aktuelle Änderung (fett)** |
| X.Y.Z-1 | YYYY-MM-DD | Vorherige Änderung |
| 0.1.0 | YYYY-MM-DD | Initial |
```

### 11.3 Regeln

- Neueste Version zuerst
- Aktuelle Version **fett**
- Kurze, prägnante Beschreibungen
- Breaking Changes mit ⚠️ kennzeichnen

---

## 12. Review-Checkliste

Vor Fertigstellung einer Dokumentation prüfen:

**Header:**
- [ ] Alle Pflichtfelder vorhanden
- [ ] Version und Datum aktuell
- [ ] Typ korrekt
- [ ] Zielgruppe angegeben
- [ ] English-Link vorhanden und korrekt

**Inhaltsverzeichnis:**
- [ ] Vorhanden und vollständig
- [ ] Nummerierung korrekt
- [ ] Alle Anker funktionieren

**Struktur:**
- [ ] Kapitel nummeriert (außer TOC, Changelog, Siehe auch)
- [ ] Horizontale Trenner vor H2
- [ ] Changelog am Ende

**Inhalt:**
- [ ] Zielgruppe in Übersicht detailliert (falls nötig)
- [ ] Code-Beispiele getestet
- [ ] Keine TODO/FIXME übrig
- [ ] Konsistente Terminologie
- [ ] Querverweise mit Versionen

**Datei:**
- [ ] Dateiname folgt Konvention
- [ ] Im richtigen Ordner

---

## 13. Siehe auch

- [Blueprint.md](Blueprint.md) — Meta-Blueprint
- [ModuleDoc.md](ModuleDoc.md) — CMake-Modul-Dokumentation
- [Guide.md](Guide.md) — Benutzerhandbücher
- [Reference.md](Reference.md) — API/Schema-Referenzen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.2** | **2025-12-17** | **Zusatzartikel zu Ordnernamen für Projekte unter § 8.9** |
| 0.5.1 | 2025-12-15 | Explizite Dateinamen-Konvention (§ 8.2): Title_Case. Neue Ordnernamen-Konvention (§ 8.8): immer kleingeschrieben |
| 0.5.0 | 2025-12-13 | Neu: Pflicht-Inhaltsverzeichnis mit Ankern, Zielgruppe-Pflichtfeld im Header, English-Link im Header, Kapitel-Nummerierung, Anker-Konventionen |
| 0.1.0 | 2025-12-03 | Initial (als Documentation_Blueprint) |
