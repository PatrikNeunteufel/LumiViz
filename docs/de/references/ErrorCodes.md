# ErrorCodes — Referenz

> **Version:** 1.1.0  
> **Datum:** 2025-12-28  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [ErrorCodes.md](../../en/references/ErrorCodes.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Fatal Errors (E)](#3-fatal-errors-e)
4. [Warnings (W)](#4-warnings-w)
5. [Schnellreferenz](#5-schnellreferenz)
6. [Verwendung in Code](#6-verwendung-in-code)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert alle Fehlercodes des CMake Architecture Build-Systems mit Erklärungen und Lösungsvorschlägen.

### Fehlercode-Format

```
[E|W][K][NN]

E = Error (Build bricht ab)
W = Warning (Build läuft weiter)
K = Kategorie (1 Ziffer)
NN = Nummer (2 Ziffern)
```

### Zielgruppe

- Entwickler bei der Fehlersuche
- Build-System-Entwickler für konsistente Fehlermeldungen

---

## 2. Konventionen

### 2.1 Fehlercode-Bereiche

| Bereich | Codes | Beschreibung |
|---------|-------|--------------|
| JSON/Parsing | `E0xx` | Fehlende Pflichtfelder, ungültiges JSON |
| Target-Erstellung | `E1xx` | Target existiert bereits, Abhängigkeit fehlt |
| Externals | `E2xx` | Fetch fehlgeschlagen, Include.cmake fehlt |
| Tests | `E3xx` | Framework, Source-Fehler |
| App-Container | `E4xx` | App-Struktur, Verzeichnisse |
| System Externals | `E5xx` | find_package, Komponenten |
| Deprecation | `W0xx` | Veraltete Features/Syntax |
| Konfiguration | `W1xx` | Suboptimale Einstellungen |
| Tools/Setup | `W2xx` | Fehlende Tools |
| External-Caching | `W3xx` | Offline-Modus, Hook-Wiederverwendung |
| App-Container | `W4xx` | Optionale Verzeichnisse |
| System External-Warnungen | `W5xx` | Backup-Verwendung |

### 2.2 Schweregrade

| Symbol | Typ | Bedeutung |
|--------|-----|-----------|
| ⛔ | FATAL_ERROR | Build bricht ab |
| ⚠️ | WARNING | Build läuft weiter |

---

## 3. Fatal Errors (E)

### 3.1 E0xx — JSON/Parsing Errors

#### E001 — Pflichtfeld fehlt

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

#### E003 — Schema-Version inkompatibel

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.6.0 |

**Meldung:**
```
[E003] Solution.json: schemaVersion 1.0 incompatible (expected major: 0, current: 0.6)
```

**Ursache:** Die Major-Version des Schemas in Solution.json stimmt nicht mit der vom Build-System unterstützten Version überein.

**Lösung:**
- `schemaVersion` in Solution.json auf kompatible Version setzen (z.B. `"0.6"`)
- Oder Build-System aktualisieren, wenn neuere Schema-Version benötigt wird

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

#### E012 — External-Source-Feld Fehler

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

**Beschreibung:** Eine zirkuläre Abhängigkeit zwischen Targets wurde erkannt (A → B → C → A).

**Lösung:** Gemeinsamen Code in separate Library extrahieren.

---

#### E104 — Source.cmake nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⛔ FATAL |
| **Seit** | v0.1.0 |

**Beschreibung:** Der Source-Mode ist `explicit` und das Source-Verzeichnis enthält keine `Source.cmake` Datei.

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

#### E401 — App: name ist Pflichtfeld

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

#### W001 — Veraltetes Schema

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** Die Solution.json verwendet ein veraltetes Schema.

---

#### W002 — Veraltete Syntax/Feld

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** Ein Feld oder Syntax wird in Zukunft entfernt.

---

### 4.2 W1xx — Konfiguration/Validation Warnings

#### W101 — Schema-Version zu niedrig

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.6.0 |

**Meldung:**
```
[W101] Solution.json: schemaVersion 0.4 < 0.6, some features may not be available
```

**Beschreibung:** Die Solution.json verwendet eine ältere Schema-Version. Neuere Features des Build-Systems sind möglicherweise nicht verfügbar.

**Lösung:** `schemaVersion` in Solution.json auf aktuelle Version aktualisieren.

---

#### W102 — Schema-Version zu hoch

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.6.0 |

**Meldung:**
```
[W102] Solution.json: schemaVersion 0.7 > 0.6, some features may not work correctly
```

**Beschreibung:** Die Solution.json verwendet eine neuere Schema-Version als das Build-System unterstützt. Einige Features funktionieren möglicherweise nicht korrekt.

**Lösung:** Build-System aktualisieren oder `schemaVersion` reduzieren.

---

#### W103 — Include.cmake erstellt Executables

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** Eine Include.cmake eines Externals erstellt Executable-Targets (IDE Clutter).

---

#### W104 — Include.cmake bindet Beispiele ein

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** Eine Include.cmake bindet Beispiel- oder Test-Verzeichnisse ein.

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

**Beschreibung:** Das Target verwendet C++20 Module Interface Units, die noch experimentell sind.

---

#### W110 — GLOB-Fallback aktiv

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** Keine Source.cmake gefunden, GLOB wird als Fallback verwendet.

---

### 4.3 W2xx — Tools/Setup Warnings

#### W201 — Clang-Tidy nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.0 |

**Beschreibung:** `ENABLE_CLANG_TIDY=ON` ist gesetzt, aber clang-tidy wurde nicht gefunden.

---

### 4.4 W3xx — External-Caching Warnings

#### W301 — External aus Cache verwendet (Offline-Modus)

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.2 |

**Beschreibung:** Offline-Modus aktiv, gecachtes External wird verwendet.

---

#### W302 — Hook-Wiederverwendung aktiv

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.1.2 |

**Beschreibung:** Ein External verwendet einen Hook von einem anderen External.

---

### 4.5 W4xx — App-Container Warnings

#### W401 — App hat kein include/ Verzeichnis

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Beschreibung:** Das App-Container Verzeichnis hat kein `include/` für Public Headers.

---

#### W402 — PCH aktiviert aber Header nicht gefunden

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Beschreibung:** Precompiled Header ist aktiviert, aber die angegebene Header-Datei existiert nicht.

---

#### W403 — Tests-Verzeichnis existiert aber keine Sources

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 (Phase 8) |

**Beschreibung:** Ein `tests/` Verzeichnis existiert im App-Container, enthält aber keine Test-Dateien.

---

### 4.6 W5xx — System External Warnings

#### W501 — Backup-Pfad verwendet

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 |

**Beschreibung:** Primärer Pfad nicht gefunden, Backup-Pfad wird verwendet.

---

#### W502 — Version-Mismatch

| Aspekt | Wert |
|--------|------|
| **Schweregrad** | ⚠️ WARNING |
| **Seit** | v0.2.0 |

**Beschreibung:** Gefundene Version weicht von angeforderter Version ab.

---

## 5. Schnellreferenz

### 5.1 Fatal Errors (E)

| Code | Kategorie | Beschreibung |
|------|-----------|--------------|
| E001 | JSON | Pflichtfeld fehlt |
| E002 | JSON | Solution.json nicht gefunden |
| E003 | JSON | Schema-Version inkompatibel (Major) |
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
| E401 | AppContainer | name ist Pflichtfeld |
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

| Code | Kategorie | Beschreibung |
|------|-----------|--------------|
| W001 | Deprecation | Veraltetes Schema |
| W002 | Deprecation | Veraltete Syntax/Feld |
| W101 | Config | Schema-Version zu niedrig |
| W102 | Config | Schema-Version zu hoch |
| W103 | Config | Include.cmake erstellt Executables |
| W104 | Config | Include.cmake bindet Beispiel-Verzeichnisse ein |
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

## 6. Verwendung in Code

```cmake
include(cmake/core/Errors.cmake)

# Fehler auslösen (Build bricht ab)
cmake_fatal(E0101 "Solution.json not found at: ${path}")

# Warnung auslösen (Build läuft weiter)
cmake_warn(W0201 "Deprecated option '${opt}' used")

# Assertion (sollte nie auftreten)
cmake_assert(condition "Internal error: invalid state")
```

---

## 7. Siehe auch

- [Errors.cmake](../modules/core/Errors.md) — Fehler-Modul Dokumentation
- [Debug.cmake](../modules/core/Debug.md) — Debug-System
- [Solution_Schema.md](Solution_Schema.md) — JSON-Schema Referenz

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-28** | **E003 (Schema Major inkompatibel), W101/W102 (Schema Minor Warnungen) hinzugefügt** |
| 1.0.0 | 2025-12-14 | Blueprint v0.5.0 Format: Nummeriertes TOC, E3xx (Tests), E4xx/W4xx (AppContainer), E5xx/W5xx (System Externals), W3xx (Caching) |
| 0.1.1 | 2025-12-09 | E217 hinzugefügt (PostFetch Hook required for cmakeSupport=false) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Alle Codes aus v1.4 |
