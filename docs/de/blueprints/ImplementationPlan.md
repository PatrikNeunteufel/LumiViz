# Implementierungsplan — Standard für Umsetzungspläne

> **Version:** 1.2.0  
> **Datum:** 2025-12-26  
> **Typ:** Blueprint  
> **Status:** Stabil  
> **Basiert auf:** Doc v1.0, Blueprint v1.0  
> **Zielgruppe:** Dokumentations-Ersteller, Projektleiter  
> **Sprache:** Deutsch  
> **English:** [ImplementationPlan.md](../../en/blueprints/ImplementationPlan.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Pflichtabschnitte](#4-pflichtabschnitte)
5. [Checklisten-Format](#5-checklisten-format)
6. [Schreibstil](#6-schreibstil)
7. [Beispiel: Vollständiger Implementierungsplan](#7-beispiel-vollständiger-implementierungsplan)
8. [Review-Checkliste](#8-review-checkliste)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert die **Struktur für Implementierungspläne**. Ein Implementierungsplan beschreibt, **wie** eine Phase oder ein Feature systematisch umgesetzt wird.

### Zielgruppe

- Entwickler, die eine Phase oder ein Feature implementieren
- Projektleiter, die den Fortschritt verfolgen

### Abgrenzung

| Dokumentations-Typ | Fragestellung | Beispiel |
|-------------------|---------------|----------|
| **Implementierungsplan** | "Was muss ich in welcher Reihenfolge tun?" | Phase 1 Umsetzung |
| **Concept** | "Wie soll die Architektur aussehen?" | MyVisualizer Konzept |
| **Guide** | "Wie konfiguriere ich X?" | Qt6 Integration |
| **Tutorial** | "Zeig mir Schritt für Schritt" | Erstes Projekt erstellen |

### Kernmerkmale

Ein Implementierungsplan:

- Zerlegt eine große Aufgabe in konkrete, abhakbare Schritte
- Definiert Akzeptanzkriterien für "fertig"
- Ermöglicht Fortschrittsverfolgung
- Ist praxisorientiert, nicht konzeptionell

---

## 2. Geltungsbereich

Dieser Blueprint gilt für Dokumente, die:

- Konkrete Umsetzungsschritte für Phasen oder Features beschreiben
- Checklisten für Aufgaben enthalten
- Im Ordner `docs/[lang]/projects/` oder projektspezifisch liegen

**Beispiele:**

- `MyVisualizer_Phase1_Implementierungsplan.md`
- `LumiPulse_Phase1_Foundation.md`
- `CMake_Phase8_Implementierungsplan.md`
- `Feature_Docking_Implementierungsplan.md`

---

## 3. Header-Erweiterungen

### 3.1 Pflichtfelder

| Feld | Beschreibung |
|------|--------------|
| `Bezug:` | Referenziertes Konzept oder Spezifikation |

### 3.2 Optionale Zusatzfelder

| Feld | Beschreibung | Wann verwenden |
|------|--------------|----------------|
| `Phase:` | Wenn Teil einer größeren Phasenplanung | Phasen-basierte Projekte |
| `Geschätzte Dauer:` | Zeitschätzung vor Beginn | Immer empfohlen |
| `Tatsächliche Dauer:` | Reale Dauer nach Abschluss | Für abgeschlossene Phasen |
| `Abhängigkeiten:` | Voraussetzungen (vorherige Phasen) | Bei Phasen > 1 |
| `Methodik:` | Entwicklungsmethodik (z.B. TDD) | Bei TDD-Projekten |

### 3.3 Vollständiger Header

```markdown
# [Projekt] — Implementierungsplan [Phase/Feature]

> **Version:** X.Y.Z  
> **Datum:** YYYY-MM-DD  
> **Typ:** Implementierungsplan  
> **Status:** [In Entwicklung | In Umsetzung | ✅ Abgeschlossen (X%)]  
> **Zielgruppe:** Entwickler  
> **Bezug:** [Konzept-Dokument]  
> **Phase:** X (optional)  
> **Abhängigkeiten:** Phase X (abgeschlossen) (optional)  
> **Geschätzte Dauer:** ~X Wochen (optional)  
> **Tatsächliche Dauer:** ~X Tage (nach Abschluss)  
> **Sprache:** Deutsch  
> **Methodik:** Test-Driven Development (TDD) (optional)  
```

### 3.4 Status-Varianten

| Status | Bedeutung |
|--------|-----------|
| `In Entwicklung` | Plan wird noch erstellt |
| `In Umsetzung` | Implementierung läuft |
| `✅ Abgeschlossen (100%)` | Alle Aufgaben erledigt |
| `⏸️ Pausiert` | Vorübergehend gestoppt |

---

## 4. Pflichtabschnitte

Implementierungspläne verwenden diese Struktur:

```
## Inhaltsverzeichnis

## Übersichts-Checkliste
### Schritt N: [Titel]
### Fortschritt (Tabelle)

## 1. Übersicht
### 1.1 Phasenziel
### 1.2 Lieferumfang
### 1.3 TDD-Kernprinzip (bei TDD-Projekten)

## 2. TDD-Workflow (bei TDD-Projekten)

## 3. Voraussetzungen

## 4. Projektstruktur (optional)

## 5. Konfiguration (optional)

## 6. Implementierungsschritte
### Schritt N: [Titel]
#### N.X Aufgabe

## 7. Dokumentation (PFLICHT - immer letzter Implementierungsschritt)
### 7.1 Modul-Dokumentation
### 7.2 API-Referenz
### 7.3 Beispiel-Code
### 7.4 Changelog

## 8. Akzeptanzkriterien
### 8.1 Funktionale Anforderungen
### 8.2 Nicht-funktionale Anforderungen
### 8.3 TDD-Anforderungen (bei TDD-Projekten)

## 9. Nächste Schritte
### 9.1 Nach Phase X
### 9.2 Offene Entscheidungen

## Changelog
```

### 4.1 Abschnitts-Details

| # | Abschnitt | Inhalt | Pflicht |
|---|-----------|--------|---------|
| — | Inhaltsverzeichnis | Nummerierte Kapitel | ✅ |
| — | Übersichts-Checkliste | Kompakter Überblick + Fortschritt | ✅ |
| 1 | Übersicht | Ziel, Lieferumfang, TDD-Prinzip | ✅ |
| 2 | TDD-Workflow | Test-First-Regeln, Struktur | Optional (bei TDD) |
| 3 | Voraussetzungen | Externe Abhängigkeiten, Tools | ✅ |
| 4 | Projektstruktur | Verzeichnisbaum, Namespaces | Optional |
| 5 | Konfiguration | Solution.json, CMake, etc. | Optional |
| 6 | Implementierungsschritte | Schritte mit Detail-Checklisten | ✅ |
| 7 | **Dokumentation** | **Modul-Doku, API, Beispiele** | **✅ PFLICHT** |
| 8 | Akzeptanzkriterien | Wann ist die Phase "fertig"? | ✅ |
| 9 | Nächste Schritte | Was kommt danach? Offene Punkte | ✅ |

### 4.2 Dokumentations-Schritt (PFLICHT)

> **Dokumentation ist IMMER der letzte Implementierungsschritt einer Phase.**

Die Phase ist erst abgeschlossen, wenn die Dokumentation aktualisiert wurde.

**Dokumentations-Checkliste (in jeder Phase):**

```markdown
### Schritt N: Dokumentation (LETZTER SCHRITT)

- [ ] N.1 Modul-Dokumentation erstellen/aktualisieren
- [ ] N.2 API-Referenz dokumentieren (Doxygen-Kommentare)
- [ ] N.3 Beispiel-Code in Dokumentation
- [ ] N.4 Changelog aktualisieren
```

**Dokumentations-Inhalt:**

| Element | Beschreibung | Pflicht |
|---------|--------------|---------|
| **Modul-Übersicht** | Was macht das Modul? | ✅ |
| **API-Referenz** | Alle öffentlichen Klassen/Methoden | ✅ |
| **Beispiel-Code** | Typische Verwendung | ✅ |
| **Fehlerbehandlung** | ErrorCodes und deren Bedeutung | Optional |
| **Thread-Safety** | Welche Methoden sind thread-safe? | Optional |
| **Changelog** | Änderungen dieser Phase | ✅ |

---

## 5. Checklisten-Format

### 5.1 Markdown-Checkboxen

Verwende **immer** echte Markdown-Checkboxen:

```markdown
- [ ] Aufgabe offen
- [x] Aufgabe erledigt
```

**Niemals** Checkboxen in Code-Blöcken — diese werden nicht interaktiv gerendert.

### 5.2 Übersichts-Checkliste

Die Übersichts-Checkliste steht direkt nach dem Inhaltsverzeichnis und bietet einen kompakten Überblick:

```markdown
## Übersichts-Checkliste

### Schritt 1: [Titel] ✅

- [x] 1.1 Aufgabe A
- [x] 1.2 Aufgabe B
- [x] 1.3 Aufgabe C

### Schritt 2: [Titel] 🔄

- [x] 2.1 Aufgabe A
- [ ] 2.2 Aufgabe B

### Fortschritt

| Schritt | Beschreibung | Aufgaben | Erledigt | Status |
|---------|--------------|----------|----------|--------|
| 1 | Projektgerüst | 6 | 6 | ✅ |
| 2 | Implementation | 5 | 1 | 🔄 |
| **Σ** | **Gesamt** | **11** | **7** | **64%** |
```

### 5.3 Detail-Checklisten

Jeder Implementierungsschritt hat eine eigene Detail-Checkliste:

```markdown
### Schritt 1: Projektgerüst aufsetzen

**Ziel:** Build-System funktioniert, leeres Fenster erscheint

**Erwartetes Ergebnis:** [Konkrete Beschreibung]

---

#### 1.1 Solution.json

- [ ] schemaVersion prüfen
- [ ] Externals konfigurieren
- [ ] Build testen

---

#### 1.2 Header erweitern

- [ ] Qt-Header hinzufügen
- [ ] STL-Header hinzufügen
- [ ] Kompilierung testen
```

### 5.4 Status-Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ⬜ | Offen |
| 🔄 | In Arbeit |
| ✅ | Erledigt |

### 5.5 Nummerierung

Aufgaben werden durchnummeriert im Format `Schritt.Unteraufgabe`:

- `1.1`, `1.2`, `1.3` für Schritt 1
- `2.1`, `2.2` für Schritt 2
- etc.

Dies ermöglicht eindeutige Referenzierung in Commit-Messages oder Diskussionen.

---

## 6. Schreibstil

### 6.1 Imperativ für Aufgaben

Checklisten-Einträge im Imperativ:

| ❌ Passiv/Beschreibend | ✅ Imperativ |
|----------------------|-------------|
| "Header sollte erstellt werden" | "Header erstellen" |
| "Tests werden geschrieben" | "Tests schreiben" |
| "Konfiguration ist anzupassen" | "Konfiguration anpassen" |

### 6.2 Konkret und messbar

Aufgaben müssen eindeutig abschließbar sein:

| ❌ Vage | ✅ Konkret |
|--------|-----------|
| "Code verbessern" | "Error Handling implementieren" |
| "Tests hinzufügen" | "Unit Test für loadFile() schreiben" |
| "Dokumentation" | "README.md mit Build-Anleitung aktualisieren" |

### 6.3 Abhängigkeiten explizit

Wenn Aufgaben aufeinander aufbauen, dies kennzeichnen:

```markdown
**3.4 BassAudioSource Implementation**

- [ ] loadFile(): BASS_StreamCreateFile()
- [ ] play(): BASS_ChannelPlay() *(benötigt loadFile)*
- [ ] getFFT(): BASS_ChannelGetData() *(benötigt play)*
```

### 6.4 TDD-Semantik dokumentieren

Bei TDD-Projekten: **Semantik-Tabellen VOR Tests**:

```markdown
#### 2.1 Result_Tests.cpp schreiben (RED)

**Semantik-Entscheidungen:**

| Methode | Erwartetes Verhalten |
|---------|---------------------|
| `value()` auf Err | Wirft Exception |
| `valueOr(default)` | Gibt value oder default zurück |
| Doppeltes `init()` | Gibt false zurück |
```

---

## 7. Beispiel: Vollständiger Implementierungsplan

```markdown
# LumiPulse — Implementierungsplan Phase 1

> **Version:** 1.0.0  
> **Datum:** 2025-12-26  
> **Typ:** Implementierungsplan  
> **Status:** ✅ Abgeschlossen (100%)  
> **Zielgruppe:** Entwickler  
> **Bezug:** LumiPulse Konzept v0.1.0  
> **Phase:** 1  
> **Tatsächliche Dauer:** ~1 Tag  
> **Sprache:** Deutsch  
> **Methodik:** Test-Driven Development (TDD)  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [TDD-Workflow](#2-tdd-workflow)
3. [Voraussetzungen](#3-voraussetzungen)
4. [Projektstruktur](#4-projektstruktur)
5. [Implementierungsschritte](#5-implementierungsschritte)
6. [Dokumentation](#6-dokumentation)
7. [Akzeptanzkriterien](#7-akzeptanzkriterien)
8. [Nächste Schritte](#8-nächste-schritte)

---

## Übersichts-Checkliste

### Schritt 1: Projektgerüst ✅

- [x] 1.1 Solution.json konfigurieren
- [x] 1.2 Verzeichnisstruktur anlegen
- [x] 1.3 Build testen

### Schritt 2: Core/Types (TDD) ✅

- [x] 2.1 Types_Tests.cpp schreiben (RED)
- [x] 2.2 Types.hpp implementieren (GREEN)
- [x] 2.3 Refactoring (REFACTOR)

### Schritt 3: Dokumentation ✅

- [x] 3.1 Modul-Dokumentation aktualisieren
- [x] 3.2 Doxygen-Kommentare in Headers
- [x] 3.3 Beispiel-Code dokumentieren
- [x] 3.4 Changelog aktualisieren

### Fortschritt

| Schritt | Beschreibung | Aufgaben | Erledigt | Status |
|---------|--------------|----------|----------|--------|
| 1 | Projektgerüst | 3 | 3 | ✅ |
| 2 | Core/Types (TDD) | 3 | 3 | ✅ |
| 3 | Dokumentation | 4 | 4 | ✅ |
| **Σ** | **Gesamt** | **10** | **10** | **✅ 100%** |

---

## 1. Übersicht

### 1.1 Phasenziel

**Phase 1: Foundation**

Ziel ist ein kompilierbares Projektgerüst mit grundlegenden Typen.

### 1.2 Lieferumfang

| Komponente | Beschreibung | Priorität |
|------------|--------------|-----------|
| Types.hpp | ParamValue, Color4f, Vec2f, Rect | P1 |

### 1.3 TDD-Kernprinzip

> **Tests definieren das Verhalten — der Code folgt.**

---

## 2. TDD-Workflow

### 2.1 Test-First-Regel

**NIEMALS Produktionscode ohne fehlschlagenden Test schreiben.**

### 2.2 Semantik VOR Implementation klären

| Frage | Test-Case |
|-------|-----------|
| Was passiert bei `contains(-1, 50)`? | "returns false" |

---

## 3. Voraussetzungen

| Voraussetzung | Version | Status |
|---------------|---------|--------|
| CMake Architecture V2 | 1.0+ | ✅ |
| C++ Standard | C++20 | ✅ |

---

## 4. Projektstruktur

```
projects/apps/LumiPulse/
├── include/Core/
│   └── Types.hpp
└── tests/unit/Core_UnitTests/
    └── Types_Tests.cpp
```

---

## 5. Implementierungsschritte

### Schritt 2: Core/Types (TDD)

**Ziel:** Grundlegende Datentypen für LumiPulse

---

#### 2.1 Types_Tests.cpp schreiben (RED)

**Semantik-Entscheidungen:**

| Typ | Erwartetes Verhalten |
|-----|---------------------|
| Color4f | RGBA mit operator[], equality |
| Vec2f | 2D-Vektor mit +, -, *, / |

**Checkliste:**

- [x] Tests schreiben
- [x] Tests ausführen → ROT

---

## 6. Dokumentation

**Ziel:** Modul-Dokumentation erstellen/aktualisieren

**Checkliste:**

- [x] Types.hpp Doxygen-Kommentare vollständig
- [x] Beispiel-Code für Color4f, Vec2f dokumentiert
- [x] README.md mit Core-Modul-Info aktualisiert
- [x] Changelog aktualisiert

---

## 7. Akzeptanzkriterien

### 7.1 Funktionale Anforderungen

| # | Kriterium | Testmethode | Status |
|---|-----------|-------------|--------|
| A1 | Types kompilieren | Unit Test | ✅ |

### 7.2 Nicht-funktionale Anforderungen

| # | Kriterium | Testmethode | Status |
|---|-----------|-------------|--------|
| N1 | Build ohne Warnings | CI | ✅ |

### 7.3 TDD-Anforderungen

| # | Kriterium | Status |
|---|-----------|--------|
| T1 | Tests VOR Implementation | ✅ |

---

## 8. Nächste Schritte

### 8.1 Nach Phase 1

**Phase 2: Audio & Rendering**

| Komponente | Abhängigkeit von Phase 1 |
|------------|-------------------------|
| PlayerEngine | Application |

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-26** | **Phase 1 ABGESCHLOSSEN** |
```

---

## 8. Review-Checkliste

Zusätzlich zur Doc.md Checkliste:

**Struktur:**

- [ ] Inhaltsverzeichnis vorhanden
- [ ] Übersichts-Checkliste direkt nach Inhaltsverzeichnis
- [ ] Fortschritts-Tabelle in Übersichts-Checkliste
- [ ] Jeder Schritt hat Ziel und erwartetes Ergebnis
- [ ] Akzeptanzkriterien definiert
- [ ] **Dokumentation ist letzter Implementierungsschritt**

**Checklisten:**

- [ ] Markdown-Checkboxen (nicht in Code-Blöcken)
- [ ] Nummerierung im Format `Schritt.Unteraufgabe`
- [ ] Aufgaben sind konkret und abschließbar
- [ ] Imperativ-Form verwendet

**Dokumentations-Schritt (PFLICHT):**

- [ ] Modul-Dokumentation als Aufgabe enthalten
- [ ] API-Referenz (Doxygen) als Aufgabe enthalten
- [ ] Beispiel-Code als Aufgabe enthalten
- [ ] Changelog-Aktualisierung als Aufgabe enthalten

**Bei TDD-Projekten:**

- [ ] TDD-Workflow Abschnitt vorhanden
- [ ] Semantik-Tabellen vor Implementierung
- [ ] TDD-Anforderungen in Akzeptanzkriterien
- [ ] RED → GREEN → REFACTOR dokumentiert

**Inhalt:**

- [ ] Bezug auf Konzept-Dokument vorhanden
- [ ] Voraussetzungen gelistet
- [ ] Nächste Schritte definiert
- [ ] Offene Punkte dokumentiert

**Praktikabilität:**

- [ ] Schritte sind in sinnvoller Reihenfolge
- [ ] Abhängigkeiten zwischen Aufgaben klar
- [ ] Geschätzte Dauer realistisch

---

## 9. Siehe auch

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Concept.md](Concept.md) — Für Architektur-Konzepte
- [Guide.md](Guide.md) — Für Benutzerhandbücher

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.2.0** | **2025-12-26** | **Dokumentation als PFLICHT-Schritt am Ende jeder Phase. Review-Checkliste erweitert.** |
| 1.1.0 | 2025-12-26 | Neue Felder: Methodik, Tatsächliche Dauer, Status-Varianten. TDD-spezifische Abschnitte dokumentiert. |
| 1.0.0 | 2025-12-26 | Release-Version: Struktur, Checklisten-Format, Beispiel |
| 0.1.0 | 2025-12-21 | Initial | |
