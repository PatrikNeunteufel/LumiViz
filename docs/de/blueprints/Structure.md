# Structure — Standard für Dokumentations-Organisation

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Blueprint v0.5  
> **Zielgruppe:** Dokumentations-Ersteller, Projekt-Maintainer  
> **Sprache:** Deutsch  
> **English:** [Structure.md](../../en/blueprints/Structure.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Haupt-Ordnerstruktur](#3-haupt-ordnerstruktur)
4. [Verwendung vs. Erstellung](#4-verwendung-vs-erstellung)
5. [Ordner-Definitionen](#5-ordner-definitionen)
6. [Projekt-Unterordner](#6-projekt-unterordner)
7. [Entscheidungsbaum](#7-entscheidungsbaum)
8. [Beispiele](#8-beispiele)
9. [Siehe auch](#9-siehe-auch)



---

## 1. Übersicht

Dieser Blueprint definiert **wo Dokumentationen abgelegt werden**. Er ergänzt [Doc.md](Doc.md), welcher definiert *wie* Dokumentationen geschrieben werden.

### Zielgruppe

- **Dokumentations-Ersteller:** Wo lege ich mein Dokument ab?
- **Projekt-Maintainer:** Wie organisiere ich die Dokumentation?
- **Neue Team-Mitglieder:** Wo finde ich welche Information?

### Kernprinzip

Die Struktur trennt zwei Perspektiven:

| Perspektive | Frage | Zielgruppe |
|-------------|-------|------------|
| **Verwendung** | "Wie nutze ich das Build-System?" | Build-System-Benutzer |
| **Erstellung** | "Wie entwickle ich das Build-System weiter?" | Build-System-Entwickler |

---

## 2. Geltungsbereich

Dieser Blueprint gilt für:

- Organisation der `docs/` Ordnerstruktur
- Zuordnung von Dokumentationstypen zu Ordnern
- Projektspezifische Dokumentation

**Nicht abgedeckt:**

- Dokumentations-Inhalt → siehe [Doc.md](Doc.md)
- Datei- und Ordnernamen-Konventionen → siehe [Doc.md](Doc.md)

> **Hinweis:** Ordnernamen werden immer kleingeschrieben.  
> Details siehe [Doc.md § 8.8](Doc.md#88-ordnernamen-konvention)

---

## 3. Haupt-Ordnerstruktur

```
docs/
├── de/                              # Deutsche Dokumentation
│   ├── blueprints/                  # Meta-Standards
│   ├── standards/                   # Allgemeine Coding Standards
│   │
│   │   ─────────────────────────────────────────────────
│   │   VERWENDUNG - Für Benutzer des Build-Systems
│   │   ─────────────────────────────────────────────────
│   │
│   ├── modules/                     # CMake-Modul API-Dokumentation
│   ├── reference/                   # Nachschlagewerke
│   ├── guides/                      # How-To Anleitungen
│   ├── tutorials/                   # Step-by-Step für Einsteiger
│   │
│   │   ─────────────────────────────────────────────────
│   │   ERSTELLUNG - Für Entwickler des Build-Systems
│   │   ─────────────────────────────────────────────────
│   │
│   └── projects/
│       └── buildsystem/             # Projektspezifische Dokumentation
│           ├── concepts/
│           ├── standards/
│           └── reports/
│
└── en/                              # Englische Dokumentation
    └── [gleiche Struktur]
```
> **Konvention:** Alle Ordnernamen sind kleingeschrieben.  
> Siehe [Doc.md § 8.8](Doc.md#88-ordnernamen-konvention)
---

## 4. Verwendung vs. Erstellung

### 4.1 Unterscheidung

| Aspekt | Verwendung | Erstellung |
|--------|------------|------------|
| **Zielgruppe** | C++ Entwickler, die das Build-System nutzen | Build-System-Entwickler |
| **Frage** | "Wie füge ich ein External hinzu?" | "Wie implementiere ich Phase 8?" |
| **Stabilität** | Stabil, öffentliche API | Kann sich ändern, intern |
| **Ordner** | `modules/`, `reference/`, `guides/` | `projects/buildsystem/` |

### 4.2 Szenarien

**Verwendungs-Szenarien:**

```
"Wie konfiguriere ich Qt6?"
→ guides/Qt6_Integration_UserGuide.md

"Was bedeutet Error E0501?"
→ reference/ErrorCodes.md

"Welche Felder hat Solution.json?"
→ reference/Solution_Schema.md

"Wie funktioniert Fetch.cmake?"
→ modules/externals/Fetch.md
```

**Erstellungs-Szenarien:**

```
"Wie ist das Build-System architektonisch aufgebaut?"
→ projects/buildsystem/concepts/master_concept.md

"Was sind die Konventionen für CMake-Module?"
→ projects/buildsystem/standards/guidelines.md

"Was haben wir bei Phase 6 gelernt?"
→ projects/buildsystem/reports/development_journal.md
```

---

## 5. Ordner-Definitionen

### 5.1 blueprints/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Meta-Standards für Dokumentationstypen |
| **Zielgruppe** | Dokumentations-Ersteller |
| **Beispiele** | Doc.md, CMake.md, Concept.md |

### 5.2 standards/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Allgemeine Coding/Project Standards |
| **Zielgruppe** | Alle Entwickler |
| **Beispiele** | Cpp_Coding_Standard.md, Git_Standard.md |

**Wichtig:** Hier liegen nur *allgemeine* Standards, die projektübergreifend gelten. Projektspezifische Standards gehören nach `projects/[projekt]/standards/`.

### 5.3 modules/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | API-Dokumentation der CMake-Module |
| **Zielgruppe** | Build-System-Benutzer und -Entwickler |
| **Typ** | ModuleDoc |

**Unterstruktur:**

```
modules/
├── core/                    # Kern-Module
│   ├── Errors.md
│   ├── Debug.md
│   ├── Context.md
│   └── ...
├── project/                 # Projekt-Module
│   ├── Solution.md
│   ├── Executables.md
│   └── Libraries.md
└── externals/               # External-Module
    ├── Orchestrator.md
    ├── Fetch.md
    ├── hooks/               # Hook-Dokumentation
    └── local/               # Lokale Externals
```

### 5.4 reference/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Nachschlagewerke, Schemas, Listen |
| **Zielgruppe** | Alle Entwickler |
| **Typ** | Reference |
| **Beispiele** | Solution_Schema.md, ErrorCodes.md, Glossar.md, Externals.md |

### 5.5 guides/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | How-To Anleitungen für konkrete Aufgaben |
| **Zielgruppe** | Build-System-Benutzer |
| **Typ** | Guide |
| **Beispiele** | Adding_Externals_UserGuide.md, Testing_UserGuide.md |

### 5.6 tutorials/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Step-by-Step Anleitungen für Einsteiger |
| **Zielgruppe** | Neue Benutzer, Einsteiger |
| **Typ** | Tutorial |
| **Beispiele** | Getting_Started.md, First_Executable.md |

---

## 6. Projekt-Unterordner

### 6.1 Struktur

```
projects/
└── buildsystem/                 # Projektname
    ├── README.md                # Projekt-Übersicht
    ├── concepts/                # Architektur-Konzepte
    │   ├── master_concept.md
    │   ├── implementation_plan.md
    │   ├── AppContainer.md
    │   ├── System_Externals.md
    │   └── future_enhancements.md
    ├── standards/               # Projekt-spezifische Standards
    │   └── guidelines.md
    └── reports/                 # Berichte, Journal
        ├── development_journal.md
        └── reviews/
```

### 6.2 concepts/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Architektur-Entscheidungen, Design-Konzepte |
| **Zielgruppe** | Build-System-Entwickler |
| **Typ** | Concept |
| **Lebenszyklus** | Entwurf → Review → Implementierung |

**Besondere Dokumente:**

| Dokument | Beschreibung |
|----------|--------------|
| `master_concept.md` | Gesamtarchitektur, alle Phasen |
| `implementation_plan.md` | Detaillierter Umsetzungsplan |
| `future_enhancements.md` | Sammlung geplanter Features |

### 6.3 standards/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Projektspezifische Konventionen |
| **Zielgruppe** | Build-System-Entwickler |
| **Typ** | Standard |

**Abgrenzung zu `docs/de/standards/`:**

| Ordner | Inhalt | Beispiel |
|--------|--------|----------|
| `docs/de/standards/` | Allgemeine Standards | "Wie schreibt man C++?" |
| `projects/buildsystem/standards/` | Projektspezifisch | "Namenskonventionen für CMake-Module im Build-System" |

### 6.4 reports/

| Eigenschaft | Wert |
|-------------|------|
| **Inhalt** | Berichte, Lessons Learned, Reviews |
| **Zielgruppe** | Build-System-Entwickler |
| **Typ** | Report |

**Typische Inhalte:**

- `development_journal.md` — Entwicklungstagebuch
- `phase_X_review.md` — Review nach Phasenabschluss
- `test_reports/` — Testergebnisse

---

## 7. Entscheidungsbaum

### 7.1 Wo gehört mein Dokument hin?

```
START: Was dokumentiere ich?
│
├─► "Wie schreibt man Dokumentation?"
│   └─► blueprints/
│
├─► "Wie schreibt man generell C++/CMake/Git?"
│   └─► standards/
│
├─► "Wie funktioniert ein CMake-Modul?"
│   └─► modules/
│
├─► "Was bedeutet X? Schema/Liste/API?"
│   └─► reference/
│
├─► "Wie mache ich X mit dem Build-System?"
│   └─► guides/
│
├─► "Erste Schritte für Einsteiger?"
│   └─► tutorials/
│
└─► "Entwicklung des Build-Systems selbst?"
    │
    ├─► "Architektur/Design-Entscheidung?"
    │   └─► projects/buildsystem/concepts/
    │
    ├─► "Projektspezifische Konvention?"
    │   └─► projects/buildsystem/standards/
    │
    └─► "Lessons Learned/Review/Journal?"
        └─► projects/buildsystem/reports/
```

### 7.2 Kurzreferenz

| Ich dokumentiere... | Ordner |
|---------------------|--------|
| Meta-Standard für Doku-Typen | `blueprints/` |
| Allgemeiner Coding Standard | `standards/` |
| CMake-Modul API | `modules/` |
| Schema, Fehlercodes, Glossar | `reference/` |
| How-To für Benutzer | `guides/` |
| Einsteiger-Tutorial | `tutorials/` |
| Architektur-Konzept | `projects/buildsystem/concepts/` |
| Build-System Konventionen | `projects/buildsystem/standards/` |
| Entwicklungs-Journal | `projects/buildsystem/reports/` |

---

## 8. Beispiele

### 8.1 Beispiel: Neue External-Integration dokumentieren

**Frage:** "Ich habe GLFW integriert. Wo dokumentiere ich das?"

| Aspekt | Ordner | Dokument |
|--------|--------|----------|
| Wie nutzt man GLFW? | `guides/` | `Adding_GLFW_UserGuide.md` |
| GLFW in Externals-Liste | `reference/` | `Externals.md` (erweitern) |
| Hook-Dokumentation | `modules/externals/hooks/` | `glfw.md` |

### 8.2 Beispiel: Neue Phase implementieren

**Frage:** "Ich plane Phase 8 (AppContainer). Wo dokumentiere ich das?"

| Aspekt | Ordner | Dokument |
|--------|--------|----------|
| Konzept/Design | `projects/buildsystem/concepts/` | `AppContainer.md` |
| Im Master Concept verlinken | `projects/buildsystem/concepts/` | `master_concept.md` (erweitern) |
| Nach Implementierung | `modules/project/` | `Apps.md` |
| Benutzer-Anleitung | `guides/` | `AppContainer_UserGuide.md` |

### 8.3 Beispiel: Fehlercode hinzufügen

**Frage:** "Ich habe neue Fehlercodes E250-E259 definiert. Wo dokumentiere ich das?"

| Aspekt | Ordner | Dokument |
|--------|--------|----------|
| Fehlercodes-Liste | `reference/` | `ErrorCodes.md` (erweitern) |

---

## 9. Siehe auch

- [Doc.md](Doc.md) — Wie schreibt man Dokumentationen?
- [Blueprint.md](Blueprint.md) — Meta-Blueprint
- [Concept.md](Concept.md) — Standard für Architektur-Konzepte

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.1** | **2025-12-15** | **Verweis auf Ordnernamen-Konvention (Doc.md § 8.8)** |
| **0.5.0** | **2025-12-13** | **Initial: Ordnerstruktur, Verwendung vs. Erstellung, Entscheidungsbaum** |
