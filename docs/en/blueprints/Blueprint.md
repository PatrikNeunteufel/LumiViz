# Blueprint — Meta-Standard für Blueprints

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Blueprint.md](../../en/blueprints/Blueprint.md)

---

## Table of Contents

1. [Overview](#1-uebersicht)
2. [Blueprint-Hierarchie](#2-blueprint-hierarchie)
3. [Kernprinzipien](#3-kernprinzipien)
4. [Blueprint-Struktur](#4-blueprint-struktur)
5. [Vererbung](#5-vererbung)
6. [Versionierung](#6-versionierung)
7. [Example: Neuen Blueprint erstellen](#7-beispiel-neuen-blueprint-erstellen)
8. [Review Checklist](#8-review-checkliste)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelogs)

---

## 1. Overview

Dieses Dokument ist der **Meta-Blueprint** — es definiert, wie Blueprints selbst aufgebaut sein müssen. 

Blueprints sind Standards, die:
- Struktur und Format für andere Dokumente oder Code definieren
- Konsistenz über das gesamte Projekt sicherstellen
- Als "Basisklassen" für spezialisierte Standards fungieren

### Was ist ein Blueprint?

| Aspekt | Description |
|--------|--------------|
| **Zweck** | Definiert verbindliche Regeln für Dokumente oder Code |
| **Zielgruppe** | Documentation Authors, Code-Autoren |
| **Verbindlichkeit** | Normativ — Abweichungen müssen begründet werden |
| **Selbstreferenz** | Dieser Blueprint beschreibt auch sich selbst |

---

## 2. Blueprint-Hierarchie

Die Blueprints folgen einer Vererbungshierarchie:

```
Blueprint.md                    ← Meta-Blueprint (dieses Dokument)
    │
    ├── Doc.md                  ← Allgemeine Dokumentations-Regeln
    │       │
    │       ├── ModuleDoc.md    ← CMake-Modul-Dokumentation
    │       ├── Guide.md        ← Benutzerhandbücher
    │       ├── Reference.md    ← API/Schema-Referenceen
    │       ├── Concept.md      ← Architecture-Concepte
    │       ├── Standard.md     ← Coding/Project Standards
    │       └── Tutorial.md     ← Step-by-Step Tutorials
    
    ├── CMake.md                ← CMake-Scripts (.cmake)
    │
    └── Cpp.md                  ← C++/C Code-Dateien (.cpp, .hpp, .tpp)
```

### Vererbungsregeln

| Regel | Description |
|-------|--------------|
| **Erben** | Spezialisierte Blueprints übernehmen Regeln vom Parent |
| **Erweitern** | Zusätzliche Regeln können hinzugefügt werden |
| **Überschreiben** | Parent-Regeln können explizit überschrieben werden |
| **Dokumentieren** | Abweichungen müssen im "Basiert auf"-Header sichtbar sein |

---

## 3. Kernprinzipien

Alle Blueprints folgen diesen Grundsätzen:

### 3.1 Konsistenz vor Individualität

> Einheitliche Struktur ermöglicht schnelles Erfassen von Dokumenten.

### 3.2 Convention over Configuration

> Standardwerte und implizite Konventionen reduzieren Boilerplate.

### 3.3 Explizit vor Implizit

> Bei Mehrdeutigkeiten explizite Angabe bevorzugen.

### 3.4 Maschinenlesbar wenn möglich

> Strukturen so wählen, dass Tools sie parsen können (YAML-Header, konsistente Überschriften).

---

## 4. Blueprint-Struktur

Jeder Blueprint **MUSS** diese Elemente enthalten:

### 4.1 Header (Required)

```markdown
# [Name] — [Kurzbeschreibung]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Blueprint  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** [Wer soll diesen Blueprint lesen?]  
> **Language:** English  
> **German:** [Dateiname.md](../../en/blueprints/Dateiname.md)
```

### 4.2 Table of Contents (Required)

Nach dem Header folgt immer ein nummeriertes Table of Contents mit Anker-Links:

```markdown
---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Thema A](#2-thema-a)
3. [Thema B](#3-thema-b)
...
```

### 4.3 Requiredabschnitte für Blueprints

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Overview | Zweck, Scope |
| 2-n | Thematische Abschnitte | Die eigentlichen Regeln |
| n+1 | Examples/Templates | Konkrete Anwendung |
| n+2 | Review Checklist | Prüfpunkte vor Fertigstellung |
| n+3 | See Also | Verwandte Dokumente |
| n+4 | Changelog | Versionshistorie (immer am Ende) |

### 4.4 Horizontale Trenner

Vor jedem H2-Abschnitt (`##`) steht ein horizontaler Trenner:

```markdown
---

## Neuer Abschnitt
```

---

## 5. Vererbung

### 5.1 "Basiert auf"-Angabe

Spezialisierte Blueprints müssen ihren Parent referenzieren:

```markdown
> **Based on:** Doc v0.5, Blueprint v0.5
```

### 5.2 Überschreiben von Regeln

Wenn ein Blueprint eine Parent-Regel überschreibt:

```markdown
### Überschreibungen von Doc.md

| Regel | Parent-Wert | Dieser Blueprint |
|-------|-------------|------------------|
| Header-Feld X | Optional | Required |
```

### 5.3 Erweitern von Strukturen

Zusätzliche Requiredfelder werden dokumentiert:

```markdown
### Zusätzliche Header-Felder

Zusätzlich zu den Feldern aus Doc.md:

| Feld | Required | Description |
|------|---------|--------------|
| `Module:` | ✓ | Pfad zum dokumentierten Modul |
```

---

## 6. Versionierung

### 6.1 Semantic Versioning

Blueprints verwenden SemVer:

| Teil | Bedeutung | Wann erhöhen? |
|------|-----------|---------------|
| **MAJOR** | Inkompatible Changes | Bestehende Dokumente werden ungültig |
| **MINOR** | Neue Regeln | Rückwärtskompatibel |
| **PATCH** | Korrekturen | Typos, Klarstellungen |

### 6.2 Pre-Release Phase

Während v0.x.x:
- Breaking Changes mit MINOR-Bump erlaubt
- Schnelle Iteration möglich
- Ab v1.0.0: Strikte SemVer

### 6.3 Dateinamenskonvention

```
[Name]_v[MAJOR]_[MINOR]_[PATCH].md

Examples:
  Blueprint_v0_5_0.md
  Doc_v0_5_0.md
  ModuleDoc_v0_5_0.md
```

---

## 7. Example: Neuen Blueprint erstellen

### 7.1 Schritt-für-Schritt

1. **Parent wählen:** Welcher Blueprint wird spezialisiert?
2. **Header erstellen:** Mit korrektem "Basiert auf"
3. **Table of Contents:** Nummeriert mit Ankern
4. **Regeln definieren:** Was ist gleich, was ist neu, was wird überschrieben?
5. **Examples:** Mindestens ein vollständiges Example
6. **Checkliste:** Prüfpunkte für Anwender
7. **Review:** Gegen Parent-Blueprint validieren

### 7.2 Template für neue Blueprints

```markdown
# [Name] — [Kurzbeschreibung]

> **Version:** 1.0.0  
> **Date:** YYYY-MM-DD  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** [Parent] vX.Y  
> **Target Audience:** [Wer soll diesen Blueprint lesen?]  
> **Language:** English  
> **German:** [Name.md](../../en/blueprints/Name.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Überschreibungen von Parent.md](#3-überschreibungen-von-parentmd)
4. [Zusätzliche Regeln](#4-zusätzliche-regeln)
5. [Examples](#5-beispiele)
6. [Review Checklist](#6-review-checkliste)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

[Zweck und Zielgruppe — hier detaillierter beschreiben]

---

## 2. Scope

[Welche Dokumente/Code betrifft dieser Blueprint?]

---

## 3. Überschreibungen von Parent.md

| Regel | Parent-Wert | Dieser Blueprint |
|-------|-------------|------------------|
| ... | ... | ... |

---

## 4. Zusätzliche Regeln

[Neue Regeln hier]

---

## 5. Examples

[Vollständiges Example]

---

## 6. Review Checklist

- [ ] Header vollständig
- [ ] Table of Contents aktuell
- [ ] Überschreibungen dokumentiert
- [ ] Examples vorhanden
- [ ] Changelog aktuell

---

## 7. See Also

- [Parent-Blueprint](Parent_vX_Y_Z.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **YYYY-MM-DD** | **Initial** |
```

---

## 8. Review Checklist

Vor Freigabe eines Blueprints prüfen:

- [ ] Header vollständig (Version, Datum, Typ, Status, Zielgruppe, Sprache, English-Link)
- [ ] Zielgruppe im Header UND in Overview beschrieben
- [ ] Table of Contents nummeriert und verlinkt
- [ ] Anker funktionieren (lowercase, bindestriche statt leerzeichen)
- [ ] Alle Requiredabschnitte vorhanden
- [ ] Examples/Templates vollständig und getestet
- [ ] Review Checklist vorhanden
- [ ] "See Also" mit relevanten Querverweisen
- [ ] Changelog am Ende, aktuelle Version fett
- [ ] Dateiname folgt Konvention
- [ ] Bei Vererbung: "Basiert auf" korrekt

---

## 9. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [CMake.md](CMake.md) — CMake-Dateistruktur
- [Cpp.md](Cpp.md) — C++/C Dateistruktur
- [Strukture.md](Structure.md) — Ordnerstruktur

---

<a id="10-changelogs"></a>

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Vererbungshierarchie, Required-Table of Contents mit Ankern, Zielgruppe-Requiredfeld, English-Link im Header, Template für neue Blueprints** |
| 0.1.0 | 2025-12-03 | Initial (als Documentation_Blueprint) |
