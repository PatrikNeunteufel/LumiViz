# Guide — Standard für Benutzerhandbücher

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Guide.md](../../en/blueprints/Guide.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [Schreibstil](#5-schreibstil)
6. [Example: Vollständiger Guide](#6-beispiel-vollständiger-guide)
7. [Review Checklist](#7-review-checkliste)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

This blueprint defines die **Struktur für Benutzerhandbücher** (User Guides). Guides sind aufgabenorientierte Dokumentationen, die erklären, **wie** etwas gemacht wird.

### Zielgruppe

- C++ Developers, die das Build-System **nutzen** (nicht entwickeln)
- Fokus auf praktische Anwendung, nicht interne Details

### Abgrenzung

| Dokumentations-Typ | Fragestellung | Example |
|-------------------|---------------|----------|
| **Guide** | "Wie konfiguriere ich X?" | Qt6 Integration |
| **Tutorial** | "Zeig mir Schritt für Schritt" | Erstes Projekt erstellen |
| **Reference** | "Was bedeutet Y?" | Solution_Schema |
| **ModuleDoc** | "Wie funktioniert Z intern?" | Context.cmake |

---

## 2. Scope

Dieser Blueprint gilt für alle Dokumentationen, die:
- Praktische Anleitungen für End Users sind
- Im Ordner `docs/[lang]/guides/` liegen
- "How-To"-Charakter haben

**Examples:**
- `Qt6_Integration_UserGuide_v0_2_0.md`
- `Testing_UserGuide_v0_1_0.md`
- `Adding_Externals_UserGuide_v0_1_0.md`

---

## 3. Header-Erweiterungen

### 3.1 Optionale Zusatzfelder

| Feld | Required | Description |
|------|---------|--------------|
| `Module:` | Optional | Wenn Guide ein spezifisches Modul betrifft |

### 3.2 Vollständiger Header

```markdown
# [Thema] — Benutzerhandbuch

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Guide  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** C++ Developers  
> **Module:** externals/qt6/Include.cmake v0.6.0 (optional)  
> **Language:** English  
> **German:** [Thema_UserGuide.md](../../en/guides/Thema_UserGuide.md)
```

---

## 4. Requiredabschnitte

Guides verwenden diese Struktur:

```
## Table of Contents

## 1. Überblick
## 2. Prerequisites
## 3. Schnellstart
## 4. [Aufgabenorientierte Abschnitte]
## 5. Stolpersteine und Lösungen
## 6. Troubleshooting
## 7. See Also
## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt |
|---|-----------|--------|
| 1 | Überblick | Was ermöglicht dieser Guide? Features-Liste |
| 2 | Prerequisites | Was muss installiert/konfiguriert sein? |
| 3 | Schnellstart | Minimales, funktionierendes Example |
| 4 | Aufgaben | "Wie füge ich X hinzu?", "Wie konfiguriere ich Y?" |
| 5 | Stolpersteine | Häufige Error mit Erklärung und Lösung |
| 6 | Troubleshooting | Checkliste, häufige Error-Tabelle |
| 7 | See Also | Verwandte Guides, Referenceen |

### 4.2 Aufgabenorientierte Abschnitte

Der Hauptteil besteht aus aufgabenorientierten Abschnitten:

```markdown
## 4. Configuration

### 4.1 Minimale Configuration

[Wie macht man das Minimum?]

### 4.2 Mit erweiterten Optionen

[Wie aktiviert man Feature X?]

### 4.3 Plattform-spezifisch

[Unterschiede Windows/Linux/macOS]
```

---

## 5. Schreibstil

### 5.1 Aktiv und direkt

| ❌ Passiv | ✅ Aktiv |
|----------|---------|
| "Es wird empfohlen..." | "Wir empfehlen..." |
| "Die Datei sollte erstellt werden" | "Erstelle die Datei" |
| "Es kann konfiguriert werden" | "Konfiguriere X so:" |

### 5.2 Aufgabenorientiert

Überschriften als Fragen oder Aufgaben:

| ❌ Abstrakt | ✅ Aufgabenorientiert |
|------------|----------------------|
| "Configuration" | "Wie konfiguriere ich Qt6?" |
| "Externe Dependencies" | "Wie füge ich ein External hinzu?" |
| "Build-Prozess" | "Wie baue ich das Projekt?" |

### 5.3 Konkret mit Examplesn

Jede Anleitung enthält:
1. **Was:** Kurze Erklärung
2. **Wie:** Code-Example oder Befehl
3. **Warum:** Optionale Erklärung bei nicht-offensichtlichen Schritten

```markdown
### 4.2 Mit Pfad-Hint

Wenn `QT_ROOT` nicht gesetzt ist, gib den Pfad direkt an:

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "C:/Qt/6.10.1/msvc2022_64"
    }
}
```

Der `hint` wird nur verwendet, wenn keine Umgebungsvariable gefunden wurde.
```

### 5.4 Visuelle Strukturierung

- **Checklisten** für Prerequisites und Troubleshooting
- **Tabellen** für Vergleiche und Optionen
- **Code-Blöcke** für alle Befehle und Configurationen
- **Note-Boxen** für Warningen und Tips

---

## 6. Example: Vollständiger Guide

```markdown
# Qt6 Integration — Benutzerhandbuch

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** C++ Developers  
> **Module:** externals/qt6/Include.cmake v0.6.0  
> **Language:** English  
> **German:** [Qt6_Integration_UserGuide.md](../../en/guides/Qt6_Integration_UserGuide.md)

---

## Table of Contents

1. [Überblick](#1-überblick)
2. [Prerequisites](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Configuration](#4-konfiguration)
5. [Stolpersteine und Lösungen](#5-stolpersteine-und-lösungen)
6. [Troubleshooting](#6-troubleshooting)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Überblick

Die Qt6-Integration ermöglicht die Usage von Qt6 in CMake Architecture Projekten.

### Features

- Flexible Pfad-Erkennung
- Automatisches DLL-Deployment (Windows)
- RPATH-Configuration (Linux/macOS)
- Komponentenauswahl

---

## 2. Prerequisites

- [ ] Qt6 installiert (6.5+)
- [ ] CMake 3.24+
- [ ] Compiler: MSVC 2022 / GCC / Clang

---

## 3. Schnellstart

**1. Solution.json:**
```json
"externals": {
    "qt6": {
        "path": "externals/qt6",
        "options": { "hint": "${QT_ROOT}" }
    }
}
```

**2. Build:**
```bash
cmake --preset windows-ninja-debug
cmake --build out/build/windows-ninja-debug
```

---

## 4. Configuration

### 4.1 Minimale Configuration

[...]

### 4.2 Mit Komponenten

[...]

---

## 5. Stolpersteine und Lösungen

### 5.1 Qt-Header werden nicht gefunden

**Problem:** `fatal error: 'QApplication' file not found`

**Ursache:** Include.cmake Version < 0.3.0

**Lösung:** Include.cmake aktualisieren.

---

## 6. Troubleshooting

### Checkliste

- [ ] Qt installiert?
- [ ] Umgebungsvariable gesetzt?
- [ ] CMake-Cache gelöscht?

### Häufige Error

| Error | Lösung |
|--------|--------|
| `Qt6 not found` | Umgebungsvariable prüfen |
| DLLs fehlen | Include.cmake v0.5.0+ |

---

## 7. See Also

- [Solution_Schema](../reference/Solution_Schema_v0_1_4.md)
- [Adding_Externals](Adding_Externals_UserGuide_v0_1_0.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.2.0** | **2025-12-13** | **Stolpersteine, Troubleshooting erweitert** |
| 0.1.0 | 2025-12-10 | Initial |
```

---

## 7. Review Checklist

Zusätzlich zur Doc.md Checkliste:

**Struktur:**
- [ ] Schnellstart vorhanden (minimal funktionierendes Example)
- [ ] Prerequisites als Checkliste
- [ ] Troubleshooting-Abschnitt vorhanden

**Inhalt:**
- [ ] Aufgabenorientierte Überschriften
- [ ] Jede Anleitung hat Code-Example
- [ ] Stolpersteine dokumentiert
- [ ] Alle Plattformen berücksichtigt (wenn relevant)

**Stil:**
- [ ] Aktiver Schreibstil
- [ ] Direkte Anweisungen
- [ ] Keine unnötigen Interna

---

## 8. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Tutorial.md](Tutorial.md) — Für Schritt-für-Schritt Anleitungen
- [Reference.md](Reference.md) — Für Nachschlagewerke

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Schreibstil-Regeln, aufgabenorientierte Struktur, vollständiges Example** |
| 0.1.0 | 2025-12-03 | Initial (Teil von Documentation_Blueprint) |
