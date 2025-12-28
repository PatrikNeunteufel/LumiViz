# Knowledge Base

> **Stand:** 2025-12-19  
> **Status:** In Entwicklung  
> **Sprache:** Deutsch (Primär)

---

## Übersicht

Diese Knowledge Base enthält strukturierte Dokumentation zu:

- **Programmiersprachen** (C, C++, gemeinsame Konzepte)
- **Design Patterns & Paradigmen** (PIMPL, Singleton, Functional Programming)
- **Build-Systeme** (CMake, Make/Makefile)
- **Embedded Systems** (TI C2000, Microchip dsPIC/PIC, Arduino)
- **Tooling** (IDEs, Analyse-Tools, Git, Doxygen)
- **Standards** (Coding Conventions, Namenskonventionen)

---

## Struktur

```
knowledge/
├── de/                              ← Deutsche Dokumentation (Primär)
│   ├── languages/
│   │   ├── c/                       C-spezifische Dokumentation
│   │   ├── cpp/                     C++-spezifische Dokumentation
│   │   └── shared/                  Gemeinsame Konzepte (mit Querverweisen)
│   │
│   ├── patterns/
│   │   ├── design/                  Design Patterns (PIMPL, Singleton, ...)
│   │   └── paradigms/               Paradigmen (FP, OOP, ...)
│   │
│   ├── build-systems/
│   │   ├── cmake/                   CMake-Dokumentation
│   │   └── make/                    Make/Makefile-Dokumentation
│   │
│   ├── embedded/
│   │   ├── ti-c2000/                TI C2000 (TMS320F28P65x, etc.)
│   │   ├── microchip/               Microchip (dsPIC33, PIC18F, etc.)
│   │   └── arduino/                 Arduino-Plattform
│   │
│   ├── tooling/
│   │   ├── ide/                     IDEs (Visual Studio, CCS, etc.)
│   │   ├── analysis/                Analyse-Tools (clang-tidy, clang-format)
│   │   ├── vcs/                     Versionskontrolle (Git)
│   │   └── documentation/           Dokumentations-Tools (Doxygen)
│   │
│   ├── reference/                   Schnellreferenzen & Cheatsheets
│   │
│   └── standards/                   Coding Standards & Konventionen
│
└── en/                              ← English (geplant)
```

---

## Dokumenttypen

Diese Knowledge Base verwendet die Blueprints aus dem Projekt-Standard:

| Typ | Dateiname-Suffix | Zweck |
|-----|------------------|-------|
| **Reference** | `_Reference.md` | Nachschlagewerk, vollständige Auflistung |
| **Guide** | `_Guide.md` | Benutzerhandbuch, How-To |
| **Concept** | `_Concept.md` | Architektur, Design-Entscheidungen |
| **Standard** | `_Standard.md` | Verbindliche Konventionen |
| **Cheatsheet** | `_Cheatsheet.md` | Schnellreferenz, Kurzübersicht |
| **Tutorial** | `_Tutorial.md` | Schritt-für-Schritt Anleitung |

---

## Querverweise C ↔ C++

Dokumente, die für beide Sprachen relevant sind, befinden sich in `languages/shared/` und enthalten Abschnitte wie:

```markdown
### C-Variante
...

### C++-Variante
...

### Unterschiede
| Aspekt | C | C++ |
|--------|---|-----|
| ... | ... | ... |
```

Sprach-spezifische Dokumente verweisen aufeinander:

```markdown
> **Siehe auch:** [C++-Variante](../cpp/Streams_Reference.md)
```

---

## Konventionen

### Dateinamen

- Englisch, Title_Case mit Underscores
- Suffix entsprechend Dokumenttyp
- Beispiel: `PIMPL_Pattern_Concept.md`

### Header

Jedes Dokument beginnt mit:

```markdown
# [Titel]

> **Version:** X.Y.Z  
> **Datum:** YYYY-MM-DD  
> **Typ:** [Reference | Guide | Concept | Standard | Cheatsheet | Tutorial]  
> **Status:** [In Entwicklung | Stabil | Deprecated]  
> **Zielgruppe:** [Beschreibung]  
> **Sprache:** Deutsch  
> **English:** [Datei.md](pfad/zur/englischen/version)
```

---

## Navigation

### Nach Sprache

- [C](de/languages/c/) — C-spezifische Dokumentation
- [C++](de/languages/cpp/) — C++-spezifische Dokumentation
- [Gemeinsam](de/languages/shared/) — Konzepte für beide Sprachen

### Nach Thema

- [Design Patterns](de/patterns/design/) — PIMPL, Singleton, Factory, etc.
- [Paradigmen](de/patterns/paradigms/) — Functional, OOP, etc.
- [Build-Systeme](de/build-systems/) — CMake, Make
- [Embedded](de/embedded/) — TI, Microchip, Arduino

### Schnellzugriff

- [Referenzen](de/reference/) — Cheatsheets und Schnellübersichten
- [Standards](de/standards/) — Coding Conventions

---

## Changelog

| Datum | Änderungen |
|-------|------------|
| **2025-12-19** | **TMS320_Silent_Failure_Reference.md und TMS320_Silent_Failure_Cheatsheet.md hinzugefügt** |
| 2025-12-19 | Initiale Struktur mit Blueprint-konformer Organisation |
