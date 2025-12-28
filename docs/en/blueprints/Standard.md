# Standard — Standard für Coding/Project Standards

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Standard.md](../../en/blueprints/Standard.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [Regel-Format](#5-regel-format)
6. [Example: Vollständiger Standard](#6-beispiel-vollständiger-standard)
7. [Review Checklist](#7-review-checkliste)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

This blueprint defines die **Struktur für Coding- und Projekt-Standards**. Standards legen verbindliche Regeln für Code-Stil, Namenskonventionen und Workflows fest.

### Zielgruppe

- Alle Entwickler im Team
- CI/CD-Systeme (automatisierte Prüfung)
- Code-Review-Prozesse

### Abgrenzung

| Dokumentations-Typ | Fokus |
|-------------------|-------|
| **Standard** | "So MUSS Code geschrieben werden" |
| **Guide** | "So KANN man X erreichen" |
| **Concept** | "Warum wurde X so entschieden" |
| **Blueprint** | "So MUSS Dokumentation geschrieben werden" |

---

## 2. Scope

Dieser Blueprint gilt für:
- Coding Standards (C++, C, CMake)
- Git Workflows
- Review-Richtlinien
- Projekt-Konventionen

**Typischer Ordner:** `docs/[lang]/standards/`

**Examples:**
- `Cpp_Coding_Standard_v0_1_0.md`
- `CMake_Standard_v0_1_0.md`
- `Git_Standard_v0_1_0.md`

---

## 3. Header-Erweiterungen

### 3.1 Zusätzliche Requiredfelder

| Feld | Required | Description |
|------|---------|--------------|
| `Scope:` | ✓ | Welcher Code/Workflow betroffen |
| `Durchsetzung:` | Optional | Wie wird der Standard erzwungen? |

### 3.2 Vollständiger Header

```markdown
# [Name] — Standard

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Standard  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** Alle Entwickler  
> **Scope:** [C++ Code | CMake Scripts | Git Workflow | ...]  
> **Durchsetzung:** [clang-format | clang-tidy | CI | Review | ...]  
> **Language:** English  
> **German:** [Name_Standard.md](../../en/standards/Name_Standard.md)
```

---

## 4. Requiredabschnitte

Standards verwenden diese Struktur:

```
## Table of Contents

## 1. Overview
## 2. Scope
## 3. [Thematische Regel-Abschnitte]
## 4. Ausnahmen
## 5. Tool-Configuration
## 6. Migration
## 7. Checkliste
## 8. See Also
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Overview | Warum dieser Standard? Ziele |
| 2 | Scope | Welcher Code, welche Dateien |
| 3 | Regeln | Die eigentlichen Vorschriften |
| 4 | Ausnahmen | Wann darf abgewichen werden? |
| 5 | Tool-Configuration | clang-format, clang-tidy, etc. |
| 6 | Migration | Umstellung von altem Code |
| 7 | Checkliste | Schnellprüfung für Reviews |
| 8 | See Also | Verwandte Standards |

---

## 5. Regel-Format

### 5.1 Regel-Dokumentation

Jede Regel wird so dokumentiert:

```markdown
### 3.2 Namenskonvention für Functions

**Regel:** Functions verwenden `snake_case`.

**Verbindlichkeit:** MUSS

**Richtig:**
```cpp
void calculate_checksum();
void parse_json_file();
```

**Falsch:**
```cpp
void CalculateChecksum();   // PascalCase
void calculateChecksum();   // camelCase
void CALCULATE_CHECKSUM();  // UPPER_CASE
```

**Begründung:**  
Konsistenz mit CMake-API und C-Stil-Code.

**Ausnahmen:**  
Keine.

**Prüfung:**  
clang-tidy Check `readability-identifier-naming`
```

### 5.2 Verbindlichkeits-Stufen

| Stufe | Bedeutung | Keyword |
|-------|-----------|---------|
| **MUSS** | Keine Ausnahmen | Required, Mandatory |
| **SOLL** | Empfohlen, Ausnahmen möglich | Should, Recommended |
| **KANN** | Optional, bei Bedarf | May, Optional |
| **DARF NICHT** | Verboten | Must Not, Forbidden |

### 5.3 Regel-Tabellen

Für Overviewen kompakte Tabellenform:

```markdown
| Regel | Richtig | Falsch | Verbindlichkeit |
|-------|---------|--------|-----------------|
| Functions | `snake_case` | `camelCase` | MUSS |
| Klassen | `PascalCase` | `snake_case` | MUSS |
| Konstanten | `kPascalCase` | `UPPER_CASE` | SOLL |
```

---

## 6. Example: Vollständiger Standard

```markdown
# C++ Coding Standard

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Scope:** Alle C++ Dateien (*.cpp, *.hpp, *.tpp)  
> **Durchsetzung:** clang-format, clang-tidy, Code Review  
> **Language:** English  
> **German:** [Cpp_Coding_Standard.md](../../en/standards/Cpp_Coding_Standard.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Namenskonventionen](#3-namenskonventionen)
4. [Formatierung](#4-formatierung)
5. [Sprachfeatures](#5-sprachfeatures)
6. [Ausnahmen](#6-ausnahmen)
7. [Tool-Configuration](#7-tool-konfiguration)
8. [Checkliste](#8-checkliste)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

Dieser Standard definiert verbindliche Regeln für C++ Code im Projekt.

### Ziele

- Einheitlicher Code-Stil
- Lesbarkeit und Wartbarkeit
- Automatische Prüfbarkeit

---

## 2. Scope

| Dateityp | Betroffen |
|----------|-----------|
| `*.cpp` | ✓ |
| `*.hpp` | ✓ |
| `*.tpp` | ✓ |
| `*.h` (C-Header) | Nur wenn C++-Code |

**Nicht betroffen:** Third-Party-Code in `externals/`

---

## 3. Namenskonventionen

### 3.1 Overview

| Element | Stil | Example |
|---------|------|----------|
| Namespaces | `snake_case` | `audio_engine` |
| Klassen | `PascalCase` | `AudioPlayer` |
| Functions | `snake_case` | `play_sound()` |
| Variablen | `snake_case` | `sample_rate` |
| Konstanten | `kPascalCase` | `kMaxVolume` |
| Macros | `UPPER_CASE` | `AUDIO_API` |
| Member | `m_snake_case` | `m_buffer_size` |

### 3.2 Klassen und Structs

**Regel:** `PascalCase`

**Verbindlichkeit:** MUSS

**Richtig:**
```cpp
class AudioPlayer {};
struct SoundBuffer {};
```

**Falsch:**
```cpp
class audio_player {};  // snake_case
class audioPlayer {};   // camelCase
```

---

## 4. Formatierung

### 4.1 Einrückung

**Regel:** 4 Spaces, keine Tabs

**Verbindlichkeit:** MUSS

**Prüfung:** clang-format `IndentWidth: 4`

### 4.2 Klammern (Allman-Stil)

**Regel:** Öffnende Klammer auf neuer Zeile

**Verbindlichkeit:** MUSS

**Richtig:**
```cpp
if (condition)
{
    doSomething();
}
```

**Falsch:**
```cpp
if (condition) {
    doSomething();
}
```

---

## 5. Sprachfeatures

### 5.1 auto-Usage

**Regel:** `auto` nur bei komplexen Typen

**Verbindlichkeit:** SOLL

**Richtig:**
```cpp
auto it = container.begin();
auto ptr = std::make_unique<Widget>();
```

**Falsch:**
```cpp
auto i = 0;        // int ist klar
auto name = "str"; // const char* ist klar
```

---

## 6. Ausnahmen

| Situation | Erlaubte Abweichung |
|-----------|---------------------|
| Third-Party-Integration | Deren Stil beibehalten |
| Performance-kritischer Code | Dokumentierte Optimierungen |
| Legacy-Migration | Temporär mit TODO |

---

## 7. Tool-Configuration

### clang-format

Siehe: [.clang-format](../../.clang-format)

### clang-tidy

```yaml
Checks: >
  readability-identifier-naming,
  modernize-*,
  bugprone-*
```

---

## 8. Checkliste

Vor Code Review prüfen:

- [ ] Namenskonventionen eingehalten
- [ ] clang-format ausgeführt
- [ ] Keine Compiler-Warnings
- [ ] Keine clang-tidy Error
- [ ] Kommentare aktuell

---

## 9. See Also

- [ClangFormat_Blueprint](../blueprints/ClangFormat_Blueprint_v0_1_0.md)
- [ClangTidy_Blueprint](../blueprints/ClangTidy_Blueprint_v0_1_0.md)
- [CMake_Standard](CMake_Standard_v0_1_0.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-13** | **Initial** |
```

---

## 7. Review Checklist

Zusätzlich zur Doc.md Checkliste:

**Regeln:**
- [ ] Alle Regeln mit Verbindlichkeit versehen
- [ ] Richtig/Falsch-Examples für jede Regel
- [ ] Begründungen angegeben
- [ ] Ausnahmen dokumentiert

**Durchsetzung:**
- [ ] Tool-Configuration vorhanden oder verlinkt
- [ ] Automatische Prüfung möglich (wo sinnvoll)

**Praxis:**
- [ ] Checkliste für Reviews vorhanden
- [ ] Migrationshinweise (falls Legacy-Code existiert)

---

## 8. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [ClangFormat_Blueprint.md](ClangFormat_Blueprint.md) — Formatierungs-Standard
- [ClangTidy_Blueprint.md](ClangTidy_Blueprint.md) — Statische Analyse

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Regel-Format mit Verbindlichkeitsstufen, Tool-Configuration, Checkliste** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
