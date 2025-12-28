# README — Standard für Ordner-Navigations-Dokumente

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5.1, Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [README_Blueprint.md](../../en/blueprints/README_Blueprint.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Dateinamen-Konventionen](#3-dateinamen-konventionen)
4. [Header-Struktur](#4-header-struktur)
5. [Inhaltliche Struktur](#5-inhaltliche-struktur)
6. [Quick-Start Abschnitt](#6-quick-start-abschnitt)
7. [Datei- und Ordner-Descriptionen](#7-datei--und-ordner-beschreibungen)
8. [Examples](#8-beispiele)
9. [Review Checklist](#9-review-checkliste)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Overview

This blueprint defines **verbindliche Regeln für README.md Dateien**, die als Navigations- und Orientierungshilfe in jedem Ordner dienen.

### Zielgruppe

- **Documentation Authors**, die README-Dateien verfassen
- **Neue Team-Mitglieder**, die sich im Projekt orientieren

### Was dieser Blueprint regelt

| Bereich | Regeln |
|---------|--------|
| Dateinamen | Sprach-Suffixe, Verzeichnis-abhängige Konventionen |
| Header | Requiredfelder, Sprach-Links |
| Struktur | Quick-Start, Dateibeschreibungen, Ordner-Links |
| Zielgruppe | Navigation für Neulinge und erfahrene Nutzer |

### Zweck von README-Dateien

| Aspekt | Description |
|--------|--------------|
| **Primär** | Schnelle Orientierung im Ordner |
| **Sekundär** | Einstiegspunkt für Neulinge (Quick-Start) |
| **Tertiär** | Navigation zu Unterordnern und verwandten Dokumenten |

---

## 2. Scope

### 2.1 Wo README-Dateien benötigt werden

Jeder Ordner mit Dokumentation oder Code **SOLLTE** eine README.md enthalten:

| Ordner-Typ | README erforderlich |
|------------|---------------------|
| `docs/` | ✓ Required |
| `docs/de/`, `docs/en/` | ✓ Required |
| `docs/de/blueprints/` | ✓ Required |
| `docs/de/modules/core/` | ✓ Required |
| `cmake/` | ✓ Empfohlen |
| Projekt-Root | ✓ Required (GitHub-Standard) |

### 2.2 Ausnahmen

Keine README erforderlich für:
- Sehr kleine Ordner mit nur 1-2 selbsterklärenden Dateien
- Temporäre oder generierte Ordner

Bei Ordnern mit nur einer Datei kann diese vom **übergeordneten README** verlinkt werden.

---

## 3. Dateinamen-Konventionen

### 3.1 Grundprinzip

| Kontext | Dateiname | Sprache |
|---------|-----------|---------|
| **Standard** | `README.md` | Englisch |
| **Andere Sprache** | `README_de.md`, `README_fr.md` | Entsprechend Suffix |
| **In Sprach-Verzeichnis** (`/de/`, `/en/`) | `README.md` | Sprache des Verzeichnisses |

### 3.2 Examples

```
project/
├── README.md                    ← Englisch (Standard)
├── README_de.md                 ← German (optional)
│
├── cmake/
│   ├── README.md                ← Englisch
│   └── README_de.md             ← German (optional)
│
└── docs/
    ├── README.md                ← Englisch (Overview)
    │
    ├── de/
    │   ├── README.md            ← German (im /de/ Ordner)
    │   ├── blueprints/
    │   │   └── README.md        ← German (erbt von /de/)
    │   └── modules/
    │       └── README.md        ← German (erbt von /de/)
    │
    └── en/
        ├── README.md            ← Englisch (im /en/ Ordner)
        └── blueprints/
            └── README.md        ← Englisch (erbt von /en/)
```

### 3.3 Sprach-Vererbung

READMEs in Unterordnern von `/de/` oder `/en/` erben die Language:

| Pfad | Sprache | Begründung |
|------|---------|------------|
| `docs/de/blueprints/README.md` | German | Unterordner von `/de/` |
| `docs/en/modules/README.md` | Englisch | Unterordner von `/en/` |
| `cmake/README.md` | Englisch | Kein Sprach-Verzeichnis |
| `cmake/README_de.md` | German | Expliziter Suffix |

---

## 4. Header-Struktur

### 4.1 Vereinfachter Header für READMEs

READMEs verwenden einen **vereinfachten Header** ohne Typ/Status/Target Audience:

```markdown
# [Ordnername] — [Kurzbeschreibung]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Language:** English  
> **German:** [README.md](pfad/zur/englischen/version)
```

### 4.2 Header-Felder

| Feld | Required | Description |
|------|---------|--------------|
| **Version** | ✓ | SemVer, synchron mit Ordnerinhalt |
| **Datum** | ✓ | Letzte Aktualisierung |
| **Sprache** | ✓ | `German` oder `English` |
| **English** | ✓ (nur nicht-EN) | Link zur englischen Version |

### 4.3 Sprach-Links

**Für nicht-englische READMEs** (außerhalb von `/en/`):

```markdown
> **German:** [README.md](../../en/ordner/README.md)
```

**Für englische READMEs** (außerhalb von Sprach-Verzeichnissen):

```markdown
> **German:** [README_de.md](./README_de.md)
```

---

## 5. Inhaltliche Struktur

### 5.1 Required-Abschnitte

Jede README **MUSS** diese Abschnitte enthalten:

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Quick-Start | Einstieg für Neulinge (mehrere Pfade möglich) |
| 2 | Overview | Zweck des Ordners |
| 3 | Dateien | **Alle** Dateien beschrieben + verlinkt |
| 4 | Unterordner | Falls vorhanden, mit Links zu deren READMEs |

### 5.2 Optionale Abschnitte

| Abschnitt | Wann sinnvoll |
|-----------|---------------|
| See Also | Verwandte Ordner/Dokumente |
| Changelog | Bei häufigen Struktur-Changes |

### 5.3 Struktur-Template

```markdown
# [Ordnername] — [Kurzbeschreibung]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Language:** English  
> **German:** [README.md](pfad/zur/en/version)

---

## Quick-Start

**[Zielgruppe 1]?**
1. [Dokument1.md](Dokument1.md) — Description
2. [Dokument2.md](Dokument2.md) — Description

**[Zielgruppe 2]?**
1. [Dokument3.md](Dokument3.md) — Description

---

## Overview

[Detailliertere Description des Ordnerzwecks]

---

## Dateien

| Datei | Description |
|-------|--------------|
| [Datei1.md](Datei1.md) | Kurzbeschreibung |
| [Datei2.md](Datei2.md) | Kurzbeschreibung |
| ... | ... |

---

## Unterordner

| Ordner | Description |
|--------|--------------|
| [ordner1/](ordner1/README.md) | Kurzbeschreibung |
| [ordner2/](ordner2/README.md) | Kurzbeschreibung |
```

---

## 6. Quick-Start Abschnitt

### 6.1 Zweck

Der Quick-Start ist der **wichtigste Teil** für neue Nutzer. Er beantwortet:
- Was finde ich hier?
- Wo fange ich an?
- Was sind die wichtigsten Dateien für **meine** Situation?

### 6.2 Mehrere Einstiegspfade

Quick-Start sollte **verschiedene Zielgruppen** ansprechen:

```markdown
## Quick-Start

**Neu im Projekt?**
1. [Getting_Started.md](Getting_Started.md) — Erste Schritte
2. [Glossar.md](../references/Glossar.md) — Begriffe klären

**Build-System entwickeln?**
1. [Master_Concept.md](concepts/Master_Concept.md) — Architecture verstehen
2. [Errors.md](../modules/core/Errors.md) — Errorbehandlung

**Externe Library einbinden?**
1. [Externals.md](Externals.md) — Overview
2. [Adding_Externals.md](Adding_Externals.md) — Schritt-für-Schritt
```

### 6.3 Regeln

| Regel | Description |
|-------|--------------|
| **Mehrere Pfade** | Verschiedene Zielgruppen ansprechen |
| **Priorisiert** | Importantstes zuerst pro Pfad |
| **Verlinkt** | Direkte Links zu Dokumenten |
| **Kurze Description** | Was lernt man dort? |

### 6.4 Verlinkung über Ordnergrenzen

Quick-Start darf und sollte auch Dokumente aus **anderen Ordnern** verlinken, wenn sie für den Einstieg relevant sind:

```markdown
## Quick-Start

**Testing Framework nutzen?**
1. [Doctest.md](Doctest.md) — Dieses Verzeichnis
2. [../references/externals/Git_Externals_Testing.md](../references/externals/Git_Externals_Testing.md) — Alle Test-Frameworks
3. [../../userguides/Testing.md](../../userguides/Testing.md) — Tests schreiben
```

---

## 7. Datei- und Ordner-Descriptionen

### 7.1 Vollständige Dateiliste

**Alle** Dateien im Ordner müssen aufgelistet werden:

```markdown
## Dateien

| Datei | Description |
|-------|--------------|
| [Blueprint.md](Blueprint.md) | Meta-Blueprint — wie man Blueprints schreibt |
| [Doc.md](Doc.md) | Allgemeine Regeln für alle Dokumentationen |
| [CMake.md](CMake.md) | Struktur für CMake-Scripts |
| [Concept.md](Concept.md) | Architecture-Concepte (ADR) |
| [Cpp.md](Cpp.md) | C++/C Code-Struktur |
| [Guide.md](Guide.md) | Benutzerhandbücher (How-To) |
| [ModuleDoc.md](ModuleDoc.md) | CMake-Modul-Dokumentation |
| [README_Blueprint.md](README_Blueprint.md) | Dieses Dokument |
| [Reference.md](Reference.md) | Nachschlagewerke |
| [Standard.md](Standard.md) | Coding/Project Standards |
| [Structure.md](Structure.md) | Dokumentations-Struktur |
| [Tutorial.md](Tutorial.md) | Step-by-Step Anleitungen |
```

### 7.2 Unterordner mit einzelnen Dateien

Bei Unterordnern mit nur **einer Datei** kann diese direkt vom übergeordneten README verlinkt werden:

```markdown
## Unterordner

| Ordner | Inhalt |
|--------|--------|
| [core/](core/README.md) | [Fetch_cmake.md](core/Fetch_cmake.md) — Git FetchContent |
| [fetched/](fetched/README.md) | [Handler_cmake.md](fetched/Handler_cmake.md) — Post-Fetch Handler |
```

Oder als erweiterte Tabelle:

```markdown
## Unterordner und deren Dateien

### core/
| Datei | Description |
|-------|--------------|
| [Fetch_cmake.md](core/Fetch_cmake.md) | Git FetchContent Wrapper |

### fetched/
| Datei | Description |
|-------|--------------|
| [Handler_cmake.md](fetched/Handler_cmake.md) | Post-Fetch Handler |
```

### 7.3 Gruppierung bei vielen Dateien

Bei > 10 Dateien sollten diese thematisch gruppiert werden:

```markdown
## Dateien

### Kern-Blueprints

| Datei | Description |
|-------|--------------|
| [Blueprint.md](Blueprint.md) | Meta-Blueprint |
| [Doc.md](Doc.md) | Dokumentations-Grundlagen |

### Dokumentations-Typen

| Datei | Description |
|-------|--------------|
| [Guide.md](Guide.md) | Benutzerhandbücher |
| [Reference.md](Reference.md) | Nachschlagewerke |
| [Tutorial.md](Tutorial.md) | Anleitungen |
| ... | ... |
```

---

## 8. Examples

### 8.1 Vollständiges Example: modules/externals/README.md

```markdown
# Externals — External-Management-System

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../../en/modules/externals/README.md)

---

## Quick-Start

**Externe Libraries einbinden?**
1. [Orchestrator_cmake.md](Orchestrator_cmake.md) — Haupt-Entry verstehen
2. [../../userguides/Externals.md](../../userguides/Externals.md) — Praktische Anleitung

**Git-basierte Library fetchen?**
1. [core/Fetch_cmake.md](core/Fetch_cmake.md) — FetchContent-Wrapper
2. [hooks/HookLoader_cmake.md](hooks/HookLoader_cmake.md) — Pre/Post-Fetch Hooks

**Lokale Library einbinden?**
1. [locals/Attach_cmake.md](locals/Attach_cmake.md) — Local-Attach-System
2. [includes/](includes/README.md) — Include-Definitionen

---

## Overview

Das External-Management-System handhabt alle externen Dependencies.

---

## Dateien

| Datei | Description |
|-------|--------------|
| [Orchestrator_cmake.md](Orchestrator_cmake.md) | Haupt-Orchestrierung des External-Systems |

---

## Unterordner

| Ordner | Dateien | Description |
|--------|---------|--------------|
| [core/](core/README.md) | 1 | [Fetch_cmake.md](core/Fetch_cmake.md) — Git FetchContent |
| [fetched/](fetched/README.md) | 1 | [Handler_cmake.md](fetched/Handler_cmake.md) — Post-Fetch Handler |
| [hooks/](hooks/README.md) | 3+ | Hook-System (Pre/Post-Fetch) |
| [includes/](includes/README.md) | 4+ | Library-spezifische Includes |
| [locals/](locals/README.md) | 1 | [Attach_cmake.md](locals/Attach_cmake.md) — Local Libraries |
| [registry/](registry/README.md) | 1 | [Targets_cmake.md](registry/Targets_cmake.md) — Target-Registry |
```

---

## 9. Review Checklist

Vor Fertigstellung einer README prüfen:

**Header:**
- [ ] Version und Datum aktuell
- [ ] Sprache korrekt angegeben
- [ ] English-Link vorhanden (bei nicht-EN)

**Quick-Start:**
- [ ] Mehrere Einstiegspfade für verschiedene Zielgruppen
- [ ] Links zu wichtigsten Dokumenten
- [ ] Auch relevante Dokumente aus anderen Ordnern verlinkt

**Dateien:**
- [ ] **Alle** Dateien im Ordner aufgelistet
- [ ] Jede Datei hat eine aussagekräftige Description
- [ ] Links funktionieren

**Unterordner:**
- [ ] Alle Unterordner aufgelistet
- [ ] Bei Ordnern mit einzelnen Dateien: Dateien direkt verlinkt
- [ ] Link zur README des Unterordners

**Struktur:**
- [ ] Horizontale Trenner vor H2
- [ ] Keine Nummerierung der Abschnitte

---

## 10. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Blueprint.md](Blueprint.md) — Meta-Blueprint
- [Structure.md](Structure.md) — Dokumentations-Struktur

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-15** | **Initial: Dateinamen-Konventionen, Quick-Start mit mehreren Pfaden, vollständige Dateilisten, Unterordner-Integration** |
