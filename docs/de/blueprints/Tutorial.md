# Tutorial — Standard für Step-by-Step Anleitungen

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Doc v0.5, Blueprint v0.5  
> **Zielgruppe:** Dokumentations-Ersteller  
> **Sprache:** Deutsch  
> **English:** [Tutorial.md](../../en/blueprints/Tutorial.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abgrenzung zu Guide](#2-abgrenzung-zu-guide)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Pflichtabschnitte](#4-pflichtabschnitte)
5. [Schreibstil](#5-schreibstil)
6. [Code-Beispiele](#6-code-beispiele)
7. [Struktur-Muster](#7-struktur-muster)
8. [Beispiel-Tutorial](#8-beispiel-tutorial)
9. [Review-Checkliste](#9-review-checkliste)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert den Standard für **Tutorials** — praktische Step-by-Step Anleitungen, die den Leser durch ein konkretes Projekt oder eine Aufgabe führen.

### Zielgruppe

- Dokumentations-Ersteller, die Tutorials schreiben
- Jeder, der lernen möchte, wie gute Tutorials aufgebaut sind

### Was ist ein Tutorial?

| Merkmal | Beschreibung |
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
| **Ziel** | Konkretes Projekt bauen | Konzepte verstehen |
| **Leser** | Folgt Anweisungen | Sucht spezifische Info |
| **Beispiel** | "Erstelle dein erstes Executable" | "Wie konfiguriere ich Qt6?" |
| **Ergebnis** | Funktionierendes Artefakt | Wissen/Verständnis |

**Faustregel:**
- **Tutorial:** "Am Ende hast du X gebaut"
- **Guide:** "So funktioniert X"

---

## 3. Header-Erweiterungen

### 3.1 Zusätzliche Felder

Tutorials erweitern den Standard-Header um:

```markdown
> **Voraussetzungen:** [Was muss der Leser vorher wissen/haben?]  
> **Zeitaufwand:** [Geschätzte Dauer, z.B. "~30 Minuten"]  
> **Schwierigkeit:** [Einsteiger | Fortgeschritten | Experte]  
> **Ergebnis:** [Was hat der Leser am Ende?]
```

### 3.2 Vollständiger Header

```markdown
# [Titel] — Tutorial

> **Version:** X.Y.Z  
> **Datum:** YYYY-MM-DD  
> **Typ:** Tutorial  
> **Status:** [In Entwicklung | Stabil | Deprecated]  
> **Zielgruppe:** [Wer soll dieses Tutorial machen?]  
> **Voraussetzungen:** [Guide X gelesen, Tool Y installiert]  
> **Zeitaufwand:** ~30 Minuten  
> **Schwierigkeit:** Einsteiger  
> **Ergebnis:** [Funktionierendes X]  
> **Sprache:** Deutsch  
> **English:** [Title_Tutorial.md](../../en/tutorials/Title_Tutorial.md)
```

### 3.3 Beispiel

```markdown
# GoogleTest und Catch2 hinzufügen — Tutorial

> **Version:** 1.0.0  
> **Datum:** 2025-12-10  
> **Typ:** Tutorial  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Voraussetzungen:** [Adding_Externals_UserGuide.md](../guides/Adding_Externals_UserGuide.md)  
> **Zeitaufwand:** ~45 Minuten  
> **Schwierigkeit:** Fortgeschritten  
> **Ergebnis:** Funktionierende Test-Executables mit GoogleTest und Catch2  
> **Sprache:** Deutsch  
> **English:** [GoogleTest_Catch2_Tutorial.md](../../en/tutorials/GoogleTest_Catch2_Tutorial.md)
```

---

## 4. Pflichtabschnitte

### 4.1 Struktur

```markdown
# [Titel] — Tutorial

> [Header]

---

## Inhaltsverzeichnis
[Nummeriert mit Ankern]

---

## 1. Übersicht
[Was wird gebaut? Warum? Endergebnis-Vorschau]

---

## 2. Voraussetzungen
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

## N+2. Zusammenfassung
[Was wurde gelernt? Nächste Schritte?]

---

## N+3. Troubleshooting
[Häufige Probleme und Lösungen]

---

## Siehe auch
[Weiterführende Dokumentation]

---

## Changelog
[Versionshistorie]
```

### 4.2 Pflichtabschnitte im Detail

| Abschnitt | Pflicht | Beschreibung |
|-----------|---------|--------------|
| **Übersicht** | ✓ | Was wird gebaut, warum, Vorschau |
| **Voraussetzungen** | ✓ | Checkliste vor dem Start |
| **Schritte** | ✓ | Nummerierte Anweisungen |
| **Ergebnis testen** | ✓ | Verifizierung des Endergebnisses |
| **Zusammenfassung** | ✓ | Was gelernt wurde |
| **Troubleshooting** | — | Häufige Probleme (empfohlen) |
| **Dateien-Übersicht** | — | Finale Struktur (empfohlen) |

---

## 5. Schreibstil

### 5.1 Grundregeln

| Regel | Richtig | Falsch |
|-------|---------|--------|
| Imperativ | "Erstelle die Datei" | "Die Datei sollte erstellt werden" |
| Direkt | "Öffne `Solution.json`" | "Man öffnet die Datei" |
| Konkret | "Füge folgende Zeile hinzu:" | "Konfiguration anpassen" |
| Kurz | "Speichere die Datei" | "Nun solltest du die Datei speichern" |

### 5.2 Schritt-Struktur

Jeder Schritt folgt dem Muster:

```markdown
## N. Schritt N: [Aktions-Titel]

[1-2 Sätze: Was wird gemacht und warum]

**Datei:** `pfad/zur/datei.ext`

```code
[Code oder Konfiguration]
```

**Erklärung:**
- `element1`: Beschreibung
- `element2`: Beschreibung

[Optional: Hinweis-Box für wichtige Infos]
```

### 5.3 Hinweis-Boxen

```markdown
> **Hinweis:** Allgemeine Information

> **Wichtig:** Kritische Information

> **Warnung:** Potenzielle Probleme

> **Tipp:** Optionale Verbesserung
```

### 5.4 Fortschritts-Indikatoren

Bei längeren Tutorials optional:

```markdown
---
**Fortschritt:** ████████░░ 80% — Schritt 4 von 5
---
```

---

## 6. Code-Beispiele

### 6.1 Regeln

| Regel | Beschreibung |
|-------|--------------|
| **Vollständig** | Kein Code ohne Kontext |
| **Kopierbar** | Direkt verwendbar |
| **Kommentiert** | Wichtige Zeilen erklärt |
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

Bei Änderungen an existierenden Dateien:

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

## 1. Übersicht
## 2. Voraussetzungen
## 3. Schritt 1: [...]
## 4. Schritt 2: [...]
## 5. Schritt 3: [...]
## 6. Ergebnis testen
## 7. Zusammenfassung
```

### 7.2 Mehrteiliges Tutorial

Bei komplexen Themen in Teile aufteilen:

```markdown
# [Thema] — Tutorial

## Übersicht
## Voraussetzungen

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

# Zusammenfassung

## Dateien-Übersicht
## Vergleich/Fazit
## Siehe auch
```

### 7.3 Vergleichs-Tutorial

Wenn mehrere Alternativen gezeigt werden (z.B. GoogleTest vs Catch2):

```markdown
# [Alternative A] und [Alternative B] — Tutorial

## Übersicht

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

## 8. Beispiel-Tutorial

### 8.1 Voraussetzungen-Abschnitt

```markdown
## 2. Voraussetzungen

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
- `tag`: Stabile Version (v1.14.0 ist die aktuelle Release)

> **Hinweis:** Verwende immer Tags statt Branches für reproduzierbare Builds.
```

### 8.3 Dateien-Übersicht

```markdown
## Dateien-Übersicht

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

### Problem: Linker-Fehler auf Windows

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

## 9. Review-Checkliste

Vor Veröffentlichung eines Tutorials prüfen:

**Header:**
- [ ] Alle Pflichtfelder vorhanden
- [ ] Voraussetzungen vollständig
- [ ] Zeitaufwand realistisch
- [ ] Schwierigkeit korrekt

**Inhalt:**
- [ ] Übersicht erklärt Ziel und Ergebnis
- [ ] Voraussetzungen als Checkliste
- [ ] Schritte nummeriert und logisch
- [ ] Jeder Schritt hat: Aktion + Code + Erklärung
- [ ] Ergebnis-Test vorhanden
- [ ] Zusammenfassung mit "Was gelernt"

**Code:**
- [ ] Alle Code-Beispiele getestet
- [ ] Dateipfade angegeben
- [ ] Erwartete Ausgaben gezeigt
- [ ] Keine Syntax-Fehler

**Qualität:**
- [ ] Tutorial von Anfang bis Ende durchgegangen
- [ ] Auf frischem System getestet (wenn möglich)
- [ ] Troubleshooting für bekannte Probleme

---

## 10. Siehe auch

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Guide.md](Guide.md) — Für thematische Anleitungen
- [GoogleTest_Catch2_Tutorial.md](../tutorials/GoogleTest_Catch2_Tutorial.md) — Beispiel-Tutorial

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Initial: Abgrenzung zu Guide, Header-Erweiterungen, Schritt-Struktur, Code-Beispiele, Mehrteilige Tutorials** |
