# ErrorCodes — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [ErrorCodes.md](../../en/references/ErrorCodes.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Fatal Errors (E)](#3-fatal-errors-e)
4. [Warnings (W)](#4-warnings-w)
5. [Schnellreferenz](#5-schnellreferenz)
6. [Usage in Code](#6-verwendung-in-code)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

This reference documents alle Errorcodes des CMake Architecture Build-Systems mit Erklärungen und Lösungsvorschlägen.

### Errorcode-Format

```
[E|W][K][NN]

E = Error (Build bricht ab)
W = Warning (Build läuft weiter)
K = Kategorie (1 Ziffer)
NN = Nummer (2 Ziffern)
```

### Zielgruppe

- Entwickler bei der Errorsuche
- Build System Developers für konsistente Errormeldungen

---

## 2. Konventionen

### 2.1 Errorcode-Bereiche

| Bereich | Codes | Description |
|---------|-------|--------------|
| JSON/Parsing | `E0xx` | Fehlende Requiredfelder, ungültiges JSON |
| Target-Erstellung | `E1xx` | Target existiert bereits, Abhängigkeit fehlt |
| Externals | `E2xx` | Fetch fehlgeschlagen, Include.cmake fehlt |
| Tests | `E3xx` | Framework, Source-Error |
| App-Container | `E4xx` | App-Struktur, Verzeichnisse |
| System Externals | `E5xx` | find_package, Komponenten |
| Deprecation | `W0xx` | Deprecatede Features/Syntax |
| Configuration | `W1xx` | Suboptimale Einstellungen |
| Tools/Setup | `W2xx` | Fehlende Tools |
| External-Caching | `W3xx` | Offline-Modus, Hook-Wiederverwendung |
| App-Container | `W4xx` | Optionale Verzeichnisse |
| System External-Warningen | `W5xx` | Backup-Usage |

### 2.2 Schweregrade

| Symbol | Typ | Bedeutung |
|--------|-----|-----------|
| ⛔ | FATAL_ERROR | Build bricht ab |
| ⚠️ | WARNING | Build läuft weiter |

---

## 3. Fatal Errors (E)

### 3.1 E0xx — JSON/Parsing Errors

#### E001 — Requiredfeld fehlt

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Meldung:**
```
[E001] Executable 'MyApp' hat kein 'name' Feld
```

**Lösung:**
```json
{
    "name": "MyApp",
    "path": "src/apps/myapp"
}
```

---

#### E002 — Solution.json nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:**
- Stelle sicher, dass `Solution.json` im Projekt-Root liegt
- Prüfe Schreibweise (Groß-/Kleinschreibung)

---

#### E010 — External nicht in externals-Block definiert

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Meldung:**
```
[E010] External 'imgui' nicht in externals-Block definiert
```

**Lösung:**
```json
{
    "externals": {
        "imgui": {
            "git": "https://github.com/ocornut/imgui.git",
            "tag": "v1.90.1"
        }
    }
}
```

---

#### E012 — External-Source-Feld Error

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Meldung:**
```
[E012] External 'mylib': Kein Source-Feld (path/git) angegeben
[E012] External 'mylib': Mehrere Source-Felder angegeben (nur eines erlaubt)
```

**Lösung:** Genau EIN Source-Feld pro External (path ODER git).

---

#### E013 — Geskipptes External verwendet

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.7.1 |

**Meldung:**
```
[E013] External 'future_lib' is skipped but used by target 'MyApp'. Remove from externals list or set skip: false
```

**Ursache:** Ein External mit `skip: true` wird von einem Target referenziert.

**Lösung:** 
- External aus der `externals`-Liste des Targets entfernen, ODER
- `skip: false` setzen (oder `skip` entfernen) im External-Block

---

### 3.2 E1xx — Target-Erstellung Errors

#### E101 — Abhängigkeit existiert nicht

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Library in Solution.json definieren oder Schreibfehler korrigieren.

---

#### E102 — Target existiert bereits

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Target-Namen müssen über Libraries, Executables UND Tests hinweg eindeutig sein.

---

#### E103 — Zirkuläre Abhängigkeit

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Description:** Eine zirkuläre Abhängigkeit zwischen Targets wurde erkannt (A → B → C → A).

**Lösung:** Gemeinsamen Code in separate Library extrahieren.

---

#### E104 — Source.cmake nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Description:** Der Source-Mode ist `explicit` und das Source-Verzeichnis enthält keine `Source.cmake` Datei.

**Lösung:** Source.cmake erstellen oder Mode auf `auto` ändern.

---

### 3.3 E2xx — Externals Errors

#### E201 — Fetched External: Kein Target in Registry

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Meldung:**
```
[E201] Fetched external 'mylib': Kein Target in Registry
```

**Lösung:**
1. Prüfe ob das External ein CMake-Projekt ist
2. Erstelle ggf. einen PostFetch-Hook
3. Oder verwende lokales External mit eigenem Include.cmake

---

#### E202 — External Fetch fehlgeschlagen

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:**
- Netzwerkverbindung prüfen
- URL in Solution.json prüfen
- Tag/Branch existiert?

---

#### E213 — Lokales External: Include.cmake nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Datei erstellen: `externals/${name}/Include.cmake`

---

#### E214 — Lokales External: Pfad existiert nicht

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Pfad in Solution.json prüfen.

---

#### E215 — Fetched External: Kein tag/branch/commit

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Eines von `tag`, `branch` oder `commit` angeben.

---

#### E216 — Explizit angegebener Hook nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Lösung:** Hook-Datei erstellen oder Hook-Angabe entfernen.

---

#### E217 — PostFetch Hook erforderlich aber nicht vorhanden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.1 |

**Meldung:**
```
[E217] External 'imgui': PostFetch hook required (cmakeSupport=false) but not found
```

**Lösung:**
1. PostFetch Hook erstellen: `cmake/externals/Hooks/PostFetch/${name}.cmake`
2. Oder `cmakeSupport: true` setzen (wenn External doch CMakeLists.txt hat)

---

### 3.4 E3xx — Test Errors

#### E301 — Unbekanntes Test-Framework

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.4 |

**Lösung:** Gültige Frameworks: `doctest`, `googletest`, `catch2`

---

#### E302 — source_from Executable existiert nicht

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.4 |

**Lösung:** Executable-Name in `source_from` prüfen.

---

#### E303 — Test-Source-Verzeichnis nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.4 |

**Lösung:** Pfad in Solution.json prüfen oder Verzeichnis erstellen.

---

### 3.5 E4xx — App-Container Errors

#### E401 — App: name ist Requiredfeld

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Meldung:**
```
[E401] App definition: 'name' is required
```

**Lösung:** `name` Feld in der App-Definition hinzufügen.

---

#### E402 — App-Pfad existiert nicht

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** Pfad prüfen oder Verzeichnis erstellen.

---

#### E403 — App hat kein src/ Verzeichnis

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** `src/` Verzeichnis im App-Container erstellen.

---

#### E404 — App hat keine Source-Dateien in src/

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** Source-Dateien (.cpp) in `src/` hinzufügen.

---

#### E405 — App-Dependency nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** Abhängigkeit in Solution.json definieren.

---

#### E406 — App hat kein main/ Verzeichnis

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** `main/` Verzeichnis für Entry-Point erstellen.

---

#### E407 — App hat keine Source-Dateien in main/

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 (Phase 8) |

**Lösung:** Entry-Point Datei (main.cpp) in `main/` hinzufügen.

---

### 3.6 E5xx — System External Errors

#### E501 — System External nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 |

**Lösung:** Hint-Pfad oder Umgebungsvariable prüfen.

---

#### E502 — package-Feld fehlt

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 |

**Lösung:** `package` Feld für System Externals angeben.

---

#### E503 — find_package fehlgeschlagen

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 |

**Lösung:** Installation prüfen, CMAKE_PREFIX_PATH setzen.

---

#### E504 — Komponente nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 |

**Lösung:** Komponenten-Name prüfen, Installation vervollständigen.

---

#### E505 — Version nicht erfüllt

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.2.0 |

**Lösung:** Neuere Version installieren oder Constraint anpassen.

---

## 4. Warnings (W)

### 4.1 W0xx — Deprecation Warnings

#### W001 — Deprecatedes Schema

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Die Solution.json verwendet ein veraltetes Schema.

---

#### W002 — Deprecatede Syntax/Feld

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Ein Feld oder Syntax wird in Zukunft entfernt.

---

### 4.2 W1xx — Configuration/Validation Warnings

#### W101 — Suboptimale Configuration

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Examples:**
- Keine Source-Dateien gefunden
- PCH aktiviert aber Header nicht gefunden

---

#### W103 — Include.cmake erstellt Executables

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Eine Include.cmake eines Externals erstellt Executable-Targets (IDE Clutter).

---

#### W104 — Include.cmake bindet Examples ein

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Eine Include.cmake bindet Example- oder Test-Verzeichnisse ein.

---

#### W105 — Version nicht SemVer-konform

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Korrektes Format:** `MAJOR.MINOR.PATCH` (z.B. `1.0.0`)

---

#### W109 — C++20 Module verwendet

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Das Target verwendet C++20 Module Interface Units, die noch experimentell sind.

---

#### W110 — GLOB-Fallback aktiv

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** Keine Source.cmake gefunden, GLOB wird als Fallback verwendet.

---

### 4.3 W2xx — Tools/Setup Warnings

#### W201 — Clang-Tidy nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Description:** `ENABLE_CLANG_TIDY=ON` ist gesetzt, aber clang-tidy wurde nicht gefunden.

---

### 4.4 W3xx — External-Caching Warnings

#### W301 — External aus Cache verwendet (Offline-Modus)

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.2 |

**Description:** Offline-Modus aktiv, gecachtes External wird verwendet.

---

#### W302 — Hook-Wiederverwendung aktiv

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.2 |

**Description:** Ein External verwendet einen Hook von einem anderen External.

---

### 4.5 W4xx — App-Container Warnings

#### W401 — App hat kein include/ Verzeichnis

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Description:** Das App-Container Verzeichnis hat kein `include/` für Public Headers.

---

#### W402 — PCH aktiviert aber Header nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Description:** Precompiled Header ist aktiviert, aber die angegebene Header-Datei existiert nicht.

---

#### W403 — Tests-Verzeichnis existiert aber keine Sources

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Description:** Ein `tests/` Verzeichnis existiert im App-Container, enthält aber keine Test-Dateien.

---

### 4.6 W5xx — System External Warnings

#### W501 — Backup-Pfad verwendet

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 |

**Description:** Primärer Pfad nicht gefunden, Backup-Pfad wird verwendet.

---

#### W502 — Version-Mismatch

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 |

**Description:** Gefundene Version weicht von angeforderter Version ab.

---

## 5. Schnellreferenz

### 5.1 Fatal Errors (E)

| Code | Kategorie | Description |
|------|-----------|--------------|
| E001 | JSON | Requiredfeld fehlt |
| E002 | JSON | Solution.json nicht gefunden |
| E010 | JSON | External nicht im externals-Block |
| E012 | JSON | Kein/mehrere Source-Felder |
| E013 | External | Geskipptes External verwendet |
| E101 | Target | Abhängigkeit existiert nicht |
| E102 | Target | Target existiert bereits |
| E103 | Target | Zirkuläre Abhängigkeit |
| E104 | Target | Source.cmake nicht gefunden (mode=explicit) |
| E201 | External | Fetched External: kein Target in Registry |
| E202 | External | External Fetch fehlgeschlagen |
| E213 | External | Lokales External: Include.cmake fehlt |
| E214 | External | Lokales External: Pfad existiert nicht |
| E215 | External | Fetched External: kein tag/branch/commit |
| E216 | External | Explizit angegebener Hook fehlt |
| E217 | External | PostFetch Hook erforderlich (cmakeSupport=false) |
| E301 | Test | Unbekanntes Test-Framework |
| E302 | Test | source_from Executable existiert nicht |
| E303 | Test | Test-Source-Verzeichnis nicht gefunden |
| E401 | AppContainer | name ist Requiredfeld |
| E402 | AppContainer | App-Pfad existiert nicht |
| E403 | AppContainer | Kein src/ Verzeichnis |
| E404 | AppContainer | Keine Sources in src/ |
| E405 | AppContainer | Dependency nicht gefunden |
| E406 | AppContainer | Kein main/ Verzeichnis |
| E407 | AppContainer | Keine Sources in main/ |
| E501 | System | System External nicht gefunden |
| E502 | System | package-Feld fehlt |
| E503 | System | find_package fehlgeschlagen |
| E504 | System | Komponente nicht gefunden |
| E505 | System | Version nicht erfüllt |

### 5.2 Warnings (W)

| Code | Kategorie | Description |
|------|-----------|--------------|
| W001 | Deprecation | Deprecatedes Schema |
| W002 | Deprecation | Deprecatede Syntax/Feld |
| W101 | Config | Suboptimale Configuration |
| W103 | Config | Include.cmake erstellt Executables |
| W104 | Config | Include.cmake bindet Example-Verzeichnisse ein |
| W105 | Config | Version nicht SemVer-konform |
| W109 | Config | C++20 Module verwendet (experimentell) |
| W110 | Config | GLOB-Fallback aktiv |
| W201 | Tools | Clang-Tidy nicht gefunden |
| W301 | Caching | External aus Cache (Offline-Modus) |
| W302 | Caching | Hook-Wiederverwendung aktiv |
| W401 | AppContainer | Kein include/ Verzeichnis |
| W402 | AppContainer | PCH Header nicht gefunden |
| W403 | AppContainer | Tests-Verzeichnis ohne Sources |
| W501 | System | Backup-Pfad verwendet |
| W502 | System | Version-Mismatch |

---

## 6. Usage in Code

```cmake
include(cmake/core/Errors.cmake)

# Error auslösen (Build bricht ab)
cmake_fatal(E0101 "Solution.json not found at: ${path}")

# Warning auslösen (Build läuft weiter)
cmake_warn(W0201 "Deprecated option '${opt}' used")

# Assertion (sollte nie auftreten)
cmake_assert(condition "Internal error: invalid state")
```

---

## 7. See Also

- [Errors.cmake](../modules/core/Errors.md) — Error-Modul Dokumentation
- [Debug.cmake](../modules/core/Debug.md) — Debug-System
- [Solution_Schema.md](Solution_Schema.md) — JSON-Schema Reference

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Format: Nummeriertes TOC, E3xx (Tests), E4xx/W4xx (AppContainer), E5xx/W5xx (System Externals), W3xx (Caching)** |
| 0.1.1 | 2025-12-09 | E217 hinzugefügt (PostFetch Hook required for cmakeSupport=false) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Alle Codes aus v1.4 |
