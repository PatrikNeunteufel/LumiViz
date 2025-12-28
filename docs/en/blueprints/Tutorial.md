# Tutorial — Standard für Step-by-Step Anleitungen

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Documentation Authors  
> **Language:** English  
> **German:** [Tutorial.md](../../en/blueprints/Tutorial.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Abgrenzung zu Guide](#2-abgrenzung-zu-guide)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [Schreibstil](#5-schreibstil)
6. [Code-Examples](#6-code-beispiele)
7. [Struktur-Muster](#7-struktur-muster)
8. [Example-Tutorial](#8-beispiel-tutorial)
9. [Review Checklist](#9-review-checkliste)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Overview

This blueprint defines den Standard für **Tutorials** — praktische Step-by-Step Anleitungen, die den Leser durch ein konkretes Projekt oder eine Aufgabe führen.

### Zielgruppe

- Documentation Authors, die Tutorials schreiben
- Jeder, der lernen möchte, wie gute Tutorials aufgebaut sind

### Was ist ein Tutorial?

| Merkmal | Description |
|---------|--------------|
| **Ziel** | Konkretes Ergebnis am Ende |
| **Methode** | Learning by Doing |
| **Struktur** | Nummerierte Schritte |
| **Fokus** | "Folge mir und baue X" |

---

## 2. Abgrenzung zu Guide

| Aspekt | Tutorial | Guide |
|--------|----------|-------|
| **Struktur** | Linear, Schritt für Schritt | Thematisch, Nachschlagewerk |
| **Ziel** | Konkretes Projekt bauen | Concepte verstehen |
| **Leser** | Folgt Anweisungen | Sucht spezifische Info |
| **Example** | "Erstelle dein erstes Executable" | "Wie konfiguriere ich Qt6?" |
| **Ergebnis** | Funktionierendes Artefakt | Wissen/Verständnis |

**Faustregel:**
- **Tutorial:** "Am Ende hast du X gebaut"
- **Guide:** "So funktioniert X"

---

## 3. Header-Erweiterungen

### 3.1 Zusätzliche Felder

Tutorials erweitern den Standard-Header um:

```markdown
> **Prerequisites:** [Was muss der Leser vorher wissen/haben?]  
> **Zeitaufwand:** [Geschätzte Dauer, z.B. "~30 Minuten"]  
> **Schwierigkeit:** [Einsteiger | Fortgeschritten | Experte]  
> **Ergebnis:** [Was hat der Leser am Ende?]
```

### 3.2 Vollständiger Header

```markdown
# [Titel] — Tutorial

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** Tutorial  
> **Status:** [In Development | Stable | Deprecated]  
> **Target Audience:** [Wer soll dieses Tutorial machen?]  
> **Prerequisites:** [Guide X gelesen, Tool Y installiert]  
> **Zeitaufwand:** ~30 Minuten  
> **Schwierigkeit:** Einsteiger  
> **Ergebnis:** [Funktionierendes X]  
> **Language:** English  
> **German:** [Title_Tutorial.md](../../en/tutorials/Title_Tutorial.md)
```

### 3.3 Example

```markdown
# GoogleTest und Catch2 hinzufügen — Tutorial

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Tutorial  
> **Status:** Stable  
> **Target Audience:** C++ Developers  
> **Prerequisites:** [Adding_Externals_UserGuide.md](../guides/Adding_Externals_UserGuide.md)  
> **Zeitaufwand:** ~45 Minuten  
> **Schwierigkeit:** Fortgeschritten  
> **Ergebnis:** Funktionierende Test-Executables mit GoogleTest und Catch2  
> **Language:** English  
> **German:** [GoogleTest_Catch2_Tutorial.md](../../en/tutorials/GoogleTest_Catch2_Tutorial.md)
```

---

## 4. Requiredabschnitte

### 4.1 Struktur

```markdown
# [Titel] — Tutorial

> [Header]

---

## Table of Contents
[Nummeriert mit Ankern]

---

## 1. Overview
[Was wird gebaut? Warum? Endergebnis-Vorschau]

---

## 2. Prerequisites
[Checkliste: Tools, Wissen, Dateien]

---

## 3. Schritt 1: [Erste Aktion]
[Anweisungen + Code + Erklärung]

---

## 4. Schritt 2: [Zweite Aktion]
[...]

---

## N. Schritt N: [Letzte Aktion]
[...]

---

## N+1. Ergebnis testen
[Wie verifiziert man das Ergebnis?]

---

## N+2. Summary
[Was wurde gelernt? Nächste Schritte?]

---

## N+3. Troubleshooting
[Häufige Probleme und Lösungen]

---

## See Also
[Weiterführende Dokumentation]

---

## Changelog
[Versionshistorie]
```

### 4.2 Requiredabschnitte im Detail

| Abschnitt | Required | Description |
|-----------|---------|--------------|
| **Overview** | ✓ | Was wird gebaut, warum, Vorschau |
| **Prerequisites** | ✓ | Checkliste vor dem Start |
| **Schritte** | ✓ | Nummerierte Anweisungen |
| **Ergebnis testen** | ✓ | Verifizierung des Endergebnisses |
| **Summary** | ✓ | Was gelernt wurde |
| **Troubleshooting** | — | Häufige Probleme (empfohlen) |
| **Dateien-Overview** | — | Finale Struktur (empfohlen) |

---

## 5. Schreibstil

### 5.1 Grundregeln

| Regel | Richtig | Falsch |
|-------|---------|--------|
| Imperativ | "Erstelle die Datei" | "Die Datei sollte erstellt werden" |
| Direkt | "Öffne `Solution.json`" | "Man öffnet die Datei" |
| Konkret | "Füge folgende Zeile hinzu:" | "Configuration anpassen" |
| Kurz | "Speichere die Datei" | "Nun solltest du die Datei speichern" |

### 5.2 Schritt-Struktur

Jeder Schritt folgt dem Muster:

```markdown
## N. Schritt N: [Aktions-Titel]

[1-2 Sätze: Was wird gemacht und warum]

**Datei:** `pfad/zur/datei.ext`

```code
[Code oder Configuration]
```

**Erklärung:**
- `element1`: Description
- `element2`: Description

[Optional: Note-Box für wichtige Infos]
```

### 5.3 Note-Boxen

```markdown
> **Note:** Allgemeine Information

> **Important:** Kritische Information

> **Warning:** Potenzielle Probleme

> **Tip:** Optionale Verbesserung
```

### 5.4 Fortschritts-Indikatoren

Bei längeren Tutorials optional:

```markdown
---
**Fortschritt:** ████████░░ 80% — Schritt 4 von 5
---
```

---

## 6. Code-Examples

### 6.1 Regeln

| Regel | Description |
|-------|--------------|
| **Vollständig** | Kein Code ohne Kontext |
| **Kopierbar** | Direkt verwendbar |
| **Kommentiert** | Importante Zeilen erklärt |
| **Getestet** | Muss funktionieren! |

### 6.2 Code-Block-Format

```markdown
**Datei:** `cmake/externals/Hooks/PreFetch/googletest.cmake`

```cmake
# GoogleTest PreFetch Hook
# ========================

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch: Configuring GoogleTest")

# Windows CRT Fix (WICHTIG!)
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
```
```

### 6.3 Diff-Darstellung

Bei Changes an existierenden Dateien:

```markdown
**Änderung in** `Solution.json`:

```diff
{
    "externals": {
+       "googletest": {
+           "git": "https://github.com/google/googletest.git",
+           "tag": "v1.14.0"
+       }
    }
}
```
```

### 6.4 Erwartete Ausgabe

```markdown
**Erwartete Ausgabe:**

```
[==========] Running 2 tests from 1 test suite.
[----------] 2 tests from ExampleTest
[ RUN      ] ExampleTest.BasicAssertions
[       OK ] ExampleTest.BasicAssertions (0 ms)
[  PASSED  ] 2 tests.
```
```

---

## 7. Struktur-Muster

### 7.1 Einfaches Tutorial (1 Teil)

```markdown
# [Thema] — Tutorial

## 1. Overview
## 2. Prerequisites
## 3. Schritt 1: [...]
## 4. Schritt 2: [...]
## 5. Schritt 3: [...]
## 6. Ergebnis testen
## 7. Summary
```

### 7.2 Mehrteiliges Tutorial

Bei komplexen Themen in Teile aufteilen:

```markdown
# [Thema] — Tutorial

## Overview
## Prerequisites

---

# Teil 1: [Untertitel]

## 1.1 [Schritt]
## 1.2 [Schritt]
## 1.3 Zwischenergebnis testen

---

# Teil 2: [Untertitel]

## 2.1 [Schritt]
## 2.2 [Schritt]
## 2.3 Zwischenergebnis testen

---

# Summary

## Dateien-Overview
## Vergleich/Fazit
## See Also
```

### 7.3 Vergleichs-Tutorial

Wenn mehrere Alternativen gezeigt werden (z.B. GoogleTest vs Catch2):

```markdown
# [Alternative A] und [Alternative B] — Tutorial

## Overview

| Aspekt | Alternative A | Alternative B |
|--------|---------------|---------------|
| ... | ... | ... |

---

# Teil 1: Alternative A
[Vollständiges Tutorial für A]

---

# Teil 2: Alternative B
[Vollständiges Tutorial für B]

---

# Vergleich
[Entscheidungshilfe]
```

---

## 8. Example-Tutorial

### 8.1 Prerequisites-Abschnitt

```markdown
## 2. Prerequisites

Bevor du beginnst, stelle sicher:

**Tools:**
- [ ] CMake 3.19 oder höher installiert
- [ ] C++ Compiler (MSVC, GCC, oder Clang)
- [ ] Git installiert

**Wissen:**
- [ ] [Adding_Externals_UserGuide.md](../guides/Adding_Externals_UserGuide.md) gelesen
- [ ] Grundkenntnisse in CMake

**Dateien:**
- [ ] Funktionierendes CMake Architecture Projekt
- [ ] `Solution.json` vorhanden
```

### 8.2 Schritt-Abschnitt

```markdown
## 3. Schritt 1: External in Solution.json hinzufügen

Füge GoogleTest als External hinzu.

**Datei:** `Solution.json`

```json
{
    "externals": {
        "googletest": {
            "git": "https://github.com/google/googletest.git",
            "tag": "v1.14.0"
        }
    }
}
```

**Erklärung:**
- `git`: Repository-URL
- `tag`: Stablee Version (v1.14.0 ist die aktuelle Release)

> **Note:** Verwende immer Tags statt Branches für reproduzierbare Builds.
```

### 8.3 Dateien-Overview

```markdown
## Dateien-Overview

Nach Abschluss des Tutorials hast du:

```
cmake/externals/Hooks/PreFetch/
├── googletest.cmake        ← GoogleTest PreFetch Hook
└── catch2.cmake            ← Catch2 PreFetch Hook

projects/tests/MyTests/src/
└── main.cpp                ← Test-Datei

Solution.json               ← External + Test konfiguriert
```
```

### 8.4 Troubleshooting-Abschnitt

```markdown
## Troubleshooting

### Problem: Linker-Error auf Windows

**Symptom:**
```
LNK2038: mismatch detected for 'RuntimeLibrary'
```

**Ursache:** GoogleTest verwendet statische CRT, Projekt dynamische.

**Lösung:** `gtest_force_shared_crt` im PreFetch Hook setzen:
```cmake
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
```

---

### Problem: Tests werden nicht gefunden

**Symptom:** `ctest` findet keine Tests.

**Lösung:** 
1. Prüfe ob `enable_testing()` in CMakeLists.txt
2. Prüfe ob Test-Executable korrekt registriert
```
```

---

## 9. Review Checklist

Vor Veröffentlichung eines Tutorials prüfen:

**Header:**
- [ ] Alle Requiredfelder vorhanden
- [ ] Prerequisites vollständig
- [ ] Zeitaufwand realistisch
- [ ] Schwierigkeit korrekt

**Inhalt:**
- [ ] Overview erklärt Ziel und Ergebnis
- [ ] Prerequisites als Checkliste
- [ ] Schritte nummeriert und logisch
- [ ] Jeder Schritt hat: Aktion + Code + Erklärung
- [ ] Ergebnis-Test vorhanden
- [ ] Summary mit "Was gelernt"

**Code:**
- [ ] Alle Code-Examples getestet
- [ ] Dateipfade angegeben
- [ ] Erwartete Ausgaben gezeigt
- [ ] Keine Syntax-Error

**Qualität:**
- [ ] Tutorial von Anfang bis Ende durchgegangen
- [ ] Auf frischem System getestet (wenn möglich)
- [ ] Troubleshooting für bekannte Probleme

---

## 10. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Guide.md](Guide.md) — Für thematische Anleitungen
- [GoogleTest_Catch2_Tutorial.md](../tutorials/GoogleTest_Catch2_Tutorial.md) — Example-Tutorial

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Initial: Abgrenzung zu Guide, Header-Erweiterungen, Schritt-Struktur, Code-Examples, Mehrteilige Tutorials** |
