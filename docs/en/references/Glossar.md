# Glossar — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Glossar.md](../../en/references/Glossar.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Abkürzungen](#3-abkürzungen)
4. [CMake-Begriffe](#4-cmake-begriffe)
5. [Projekt-Begriffe](#5-projekt-begriffe)
6. [Build-System Concepte](#6-build-system-konzepte)
7. [Architecture-Patterns](#7-architektur-patterns) ← **NEU**
8. [C++-Idiome](#8-c-idiome) ← **NEU**
9. [Typen und Modi](#9-typen-und-modi)
10. [Compiler und Flags](#10-compiler-und-flags)
11. [Datei-Extensions](#11-datei-extensions)
12. [Errorcode-Bereiche](#12-fehlercode-bereiche)
13. [Debug-Level](#13-debug-level)
14. [Übersetzungs-Reference](#14-übersetzungs-referenz)
15. [See Also](#15-siehe-auch)
16. [Changelog](#16-changelog)

---

## 1. Overview

Dieses Glossar definiert alle Fachbegriffe, die in der Dokumentation des CMake Architecture Build-Systems verwendet werden. Es dient als zentrale Nachschlagereferenz für konsistente Terminologie.

### Zielgruppe

- Alle Entwickler im Projekt
- Neue Teammitglieder zur Einarbeitung
- Dokumentations-Autoren für einheitliche Begriffe

---

## 2. Konventionen

### Notation

| Symbol | Bedeutung |
|--------|-----------|
| **Fett** | Primärer Begriff |
| `Code` | Technischer Bezeichner |
| → | Verweis auf anderen Eintrag |

### Sprache

Begriffe werden auf German erklärt. Englische Fachbegriffe werden beibehalten, wenn sie in der Praxis üblich sind (z.B. "Target", "External", "Hook", "EventBus").

---

## 3. Abkürzungen

| Abkürzung | Ausgeschrieben | Erklärung |
|-----------|----------------|-----------|
| **API** | Application Programming Interface | Programmierschnittstelle |
| **CI/CD** | Continuous Integration/Continuous Deployment | Automatisierte Build- und Deployment-Pipelines |
| **CLI** | Command Line Interface | Kommandozeilen-Interface |
| **CRT** | C Runtime Library | Windows C-Laufzeitbibliothek |
| **DI** | Dependency Injection | Entwurfsmuster zur Entkopplung |
| **DLL** | Dynamic Link Library | Dynamisch geladene Bibliothek (Windows) |
| **DSP** | Digital Signal Processing | Digitale Signalverarbeitung |
| **EMA** | Exponential Moving Average | Glättungsalgorithmus für Zeitreihen |
| **FFT** | Fast Fourier Transform | Schnelle Fourier-Transformation |
| **FPS** | Frames Per Second | Bilder pro Sekunde |
| **GUI** | Graphical User Interface | Grafische Benutzeroberfläche |
| **i18n** | Internationalization | Internationalisierung (i + 18 Buchstaben + n) |
| **IDE** | Integrated Development Environment | Integrierte Entwicklungsumgebung |
| **JSON** | YesvaScript Object Notation | Datenaustauschformat |
| **LOC** | Lines of Code | Codezeilen (Komplexitätsmetrik) |
| **PCH** | Precompiled Header | Vorkompilierte Header-Datei zur Build-Beschleunigung |
| **PIMPL** | Pointer to Implementation | Entwurfsmuster zur Kapselung |
| **PR** | Pull Request | Anfrage zur Code-Integration |
| **RAII** | Resource Acquisition Is Initialization | C++ Idiom für automatische Ressourcenverwaltung |
| **RTTI** | Run-Time Type Information | Laufzeit-Typinformationen in C++ |
| **SemVer** | Semantic Versioning | Semantische Versionierung (MAJOR.MINOR.PATCH) |
| **SO** | Shared Object | Dynamisch geladene Bibliothek (Linux) |
| **TDD** | Test-Driven Development | Testgetriebene Entwicklung |

---

## 4. CMake-Begriffe

| Begriff | Erklärung |
|---------|-----------|
| **Cache-Variable** | Persistente CMake-Variable, gespeichert in CMakeCache.txt |
| **Configure** | Erste Phase des CMake-Builds: Projektkonfiguration |
| **FetchContent** | CMake-Modul zum Herunterladen von Dependencies |
| **Generator** | Backend für Build-System (Visual Studio, Ninja, Make) |
| **include_guard** | Verhindert mehrfaches Laden einer CMake-Datei |
| **Preset** | Vordefinierte CMake-Configuration in CMakePresets.json |
| **Property** | CMake-Eigenschaft (Target, Directory, Global) |
| **Target** | Build-Einheit in CMake (Executable, Library, Custom) |
| **Toolchain** | Compiler + Linker + Tools für eine Plattform |

---

## 5. Projekt-Begriffe

Diese Begriffe werden in unserem Projekt **nicht übersetzt**:

| Begriff | Erklärung |
|---------|-----------|
| **Context** | Isolierter Namensraum für Build-Daten (ctx_create, ctx_set, ctx_get) |
| **External** | Externe Abhängigkeit/Bibliothek im Build-System |
| **Hook** | Callback-Mechanismus für External-Verarbeitung (PreFetch, PostFetch) |
| **Pipeline** | Mehrstufiger Verarbeitungsprozess (Collect → Create → Configure) |
| **Solution** | Zentrale Configurationsdatei (Solution.json) |

---

## 6. Build-System Concepte

| Begriff | Erklärung |
|---------|-----------|
| **Convention over Configuration** | Standard-Verhalten ohne explizite Configuration |
| **Deklarativ** | Beschreibend statt imperativ (Was statt Wie) |
| **Fail-fast** | Frühzeitiges Abbrechen bei Errorn |
| **Fetched External** | External, das aus Git heruntergeladen wird |
| **Local External** | External, das im Repository liegt |
| **Single Source of Truth** | Eine zentrale Stelle für Informationen |

---

## 7. Architecture-Patterns

> **Neu in v0.6.0** — Allgemeine Software-Architecture-Begriffe

### 7.1 Architecture-Stile

| Begriff | Erklärung |
|---------|-----------|
| **Service-Architecture** | Architecturestil mit passiven Services, die von außen gesteuert werden. Services wrappen Domain-Logik und kommunizieren über → EventBus. |
| **Agent-Architecture** | Architecturestil mit autonomen Agents, die eigenes Verhalten und → State-Machine haben. Agents reagieren selbstständig auf Events. |
| **Microservices** | Architecturestil mit unabhängig deploybare Services, die über Netzwerk kommunizieren. |
| **Monolith** | Architecturestil mit einer einzigen, zusammenhängenden Anwendung. |

### 7.2 Kommunikations-Patterns

| Begriff | Erklärung |
|---------|-----------|
| **EventBus** | Zentraler Message-Broker für type-safe → Publish/Subscribe. Ermöglicht → Lose Kopplung zwischen Modulen. |
| **CommandBus** | Dispatcher für benannte Aktionen (Commands). Ermöglicht UI-Aktionen über String-IDs. |
| **MessageQueue** | Asynchrone Nachrichtenwarteschlange zwischen Komponenten. |
| **Publish/Subscribe** | Entkopplungs-Pattern: Publisher kennt Subscriber nicht, beide kennen nur den → EventBus. |
| **Observer** | Pattern bei dem Objekte (Observer) auf Changes eines Subjects reagieren. Enger gekoppelt als → Publish/Subscribe. |
| **Request/Response** | Synchrones Kommunikationsmuster: Anfrage → Antwort. |

### 7.3 Event-Concepte

| Begriff | Erklärung |
|---------|-----------|
| **Event** | Immutables Daten-Struct, das etwas beschreibt, das passiert ist. |
| **Command** | Benannte Aktion, die ausgeführt werden kann. Imperativ (tue etwas). |
| **Publisher** | Komponente, die Events veröffentlicht. |
| **Subscriber** | Komponente, die Events empfängt. |
| **Subscription** | Registrierung eines → Subscribers für einen Event-Typ. |
| **Handler** | Funktion, die auf ein Event oder Command reagiert. |

### 7.4 Dependency Injection

| Begriff | Erklärung |
|---------|-----------|
| **Dependency Injection** | Entwurfsmuster: Dependencies werden von außen übergeben statt selbst erzeugt. |
| **ServiceContainer** | Container der alle Services hält und bei Bedarf injiziert. Auch: IoC Container. |
| **IoC** | Inversion of Control. Prinzip hinter → Dependency Injection. |
| **Singleton** | Lebenszyklus: Genau eine Instanz für gesamte Anwendung. |
| **Scoped** | Lebenszyklus: Eine Instanz pro Scope (z.B. Request). |
| **Transient** | Lebenszyklus: Neue Instanz bei jedem Abruf. |

### 7.5 Design-Prinzipien

| Begriff | Erklärung |
|---------|-----------|
| **Lose Kopplung** | Module kennen sich nicht direkt, kommunizieren nur über Abstraktionen. Gegenteil: → Enge Kopplung. |
| **Enge Kopplung** | Module rufen sich direkt auf. Schwer testbar, fragil. Zu vermeiden. |
| **Interface-First Design** | Zuerst Interface definieren, dann Implementation. Ermöglicht späteren Austausch. |
| **Separation of Concerns** | Jedes Modul hat genau eine Verantwortung. |
| **Single Responsibility** | Eine Klasse sollte nur einen Grund zur Änderung haben. |
| **Open/Closed Principle** | Offen für Erweiterung, geschlossen für Modifikation. |

### 7.6 Zustandsverwaltung

| Begriff | Erklärung |
|---------|-----------|
| **State-Machine** | Zustandsautomat mit definierten Zuständen und Übergängen. |
| **Immutable** | Unveränderlich nach Erstellung. Events sollten immutable sein. |
| **Mutable** | Veränderbar. Gegenteil von → Immutable. |

---

## 8. C++-Idiome

> **Neu in v0.6.0** — Allgemeine C++-Begriffe und Idiome

| Begriff | Erklärung |
|---------|-----------|
| **RAII** | Resource Acquisition Is Initialization. Ressourcen werden im Konstruktor erworben, im Destruktor freigegeben. |
| **PIMPL** | Pointer to Implementation. Kapselung der Implementation hinter Pointer. |
| **CRTP** | Curiously Recurring Template Pattern. Klasse erbt von Template mit sich selbst als Parameters. |
| **Type-Erasure** | Technik um Template-Typen hinter nicht-template Interface zu verbergen. |
| **StrongId** | Type-safe ID-Wrapper: `StrongId<T, Tag>`. Verhindert Verwechslung verschiedener ID-Typen zur Compile-Zeit. |
| **Tag Dispatch** | Überladungsauswahl über leere Tag-Typen. |
| **SFINAE** | Substitution Failure Is Not An Error. Template-Metaprogrammierung. |
| **Copy-and-Swap** | Idiom für exception-safe Assignment-Operator. |
| **Rule of Zero** | Keine eigenen Destruktor/Copy/Move wenn möglich. |
| **Rule of Five** | Wenn einer von Destruktor/Copy-Ctor/Copy-Assign/Move-Ctor/Move-Assign nötig, dann alle fünf. |

---

## 9. Typen und Modi

### 9.1 Executable-Typen

| Typ | Usage |
|-----|------------|
| **CONSOLE** | Kommandozeilen-Anwendung mit stdout/stderr |
| **GUI** | Grafische Anwendung (Windows: WinMain) |
| **CLI** | Kommandozeilen-Tool mit Argument-Parsing |
| **HEADLESS** | Server/Dienst ohne Benutzeroberfläche |
| **WORKER** | Hintergrund-Prozess |

### 9.2 Library-Typen

| Typ | Erklärung |
|-----|-----------|
| **STATIC** | Statisch gelinkte Bibliothek (.a, .lib) |
| **SHARED** | Dynamisch gelinkte Bibliothek (.so, .dll) |
| **INTERFACE** | Header-only Bibliothek (keine Kompilierung) |

### 9.3 Test-Typen

| Typ | Erklärung | Typischer Timeout |
|-----|-----------|-------------------|
| **UNIT** | Isolierte Funktions-/Klassen-Tests | 30s |
| **INTEGRATION** | Zusammenspiel mehrerer Komponenten | 120s |
| **SYSTEM** | Ende-zu-Ende Tests | 300s |
| **PERFORMANCE** | Leistungs- und Benchmark-Tests | 600s |

### 9.4 Source-Modi

| Modus | Erklärung |
|-------|-----------|
| **explicit** | Source.cmake erforderlich (empfohlen) |
| **glob** | Automatisches Sammeln per Wildcard |
| **auto** | Source.cmake wenn vorhanden, sonst GLOB |

---

## 10. Compiler und Flags

### 10.1 Compiler

| Begriff | Erklärung |
|---------|-----------|
| **Clang** | LLVM-basierter C/C++ Compiler |
| **Clang-CL** | Clang mit MSVC-kompatiblem Frontend (Windows) |
| **Clang-Format** | Code-Formatierungs-Tool |
| **Clang-Tidy** | Statisches Analyse-Tool |
| **GCC** | GNU Compiler Collection |
| **MinGW** | Minimalist GNU for Windows |
| **MSVC** | Microsoft Visual C++ Compiler |
| **Apple Clang** | Apples Variante des Clang-Compilers |

### 10.2 MSVC-Flags

| Flag | Erklärung |
|------|-----------|
| `/EHsc` | Exception Handling aktiviert |
| `/EHs-c-` | Exception Handling deaktiviert |
| `/GR-` | RTTI deaktiviert |
| `/permissive-` | Strikte Standard-Konformität |
| `/W4` | Hohe Warnstufe |
| `/Zc:__cplusplus` | Korrekter __cplusplus Makro-Wert |
| `/Zc:preprocessor` | Standard-konformer Präprozessor |

### 10.3 GCC/Clang-Flags

| Flag | Erklärung |
|------|-----------|
| `-fno-exceptions` | Exception Handling deaktiviert |
| `-fno-rtti` | RTTI deaktiviert |
| `-Wall` | Alle wichtigen Warningen |
| `-Wextra` | Zusätzliche Warningen |
| `-Wpedantic` | Strikte Standard-Konformität |

---

## 11. Datei-Extensions

| Extension | Usage |
|-----------|------------|
| `.cmake` | CMake-Modul |
| `.cpp`, `.cxx`, `.cc`, `.c` | Kompilierbare Source-Dateien |
| `.h`, `.hpp`, `.hxx`, `.hh` | Header-Dateien |
| `.tpp`, `.txx`, `.ipp` | Template-Implementationen |
| `.inl` | Inline-Implementationen |
| `.impl` | PIMPL-Details |
| `.ixx`, `.cppm`, `.mpp` | C++20 Module Interface Units |
| `.a`, `.lib` | Statische Bibliotheken |
| `.so`, `.dll` | Dynamische Bibliotheken |
| `.vert`, `.frag`, `.glsl` | GLSL Shader-Dateien |

---

## 12. Errorcode-Bereiche

| Bereich | Description |
|---------|--------------|
| **E0xx** | JSON/Parsing-Error |
| **E1xx** | Target-Erstellung |
| **E2xx** | External-Verarbeitung |
| **E3xx** | Test-Pipeline |
| **E4xx** | App-Container-Pipeline |
| **E5xx** | System Externals |
| **W0xx** | Deprecation-Warningen |
| **W1xx** | Configurations-Warningen |
| **W2xx** | Tool/Setup-Warningen |
| **W3xx** | External-Caching-Warningen |
| **W4xx** | App-Container-Warningen |
| **W5xx** | System External-Warningen |

---

## 13. Debug-Level

### 13.1 Anzeige-Level (was sehen wir)

| Level | Konstante | Usage |
|-------|-----------|------------|
| 1 | `DBG_SHOW_LITTLE` | Nur das Importantste |
| 2 | `DBG_SHOW_SOME` | Standard (Default) |
| 3 | `DBG_SHOW_MUCH` | Mehr Details |
| 4 | `DBG_SHOW_LOTS` | Viele Details |
| 5 | `DBG_SHOW_ALL` | Alles |

### 13.2 Message-Level (wie wichtig ist die Nachricht)

| Level | Konstante | Message-Importantkeit |
|-------|-----------|---------------------|
| 1 | `DBG_OFTEN` | Häufig, wichtig (Phasen-Start) |
| 2 | `DBG_COMMON` | Übliche Info (Features) |
| 3 | `DBG_NORMAL` | Standard (Zwischenschritte) |
| 4 | `DBG_RARE` | Selten (Pfade, Details) |
| 5 | `DBG_ULTRA_RARE` | Tiefes Debugging |

---

## 14. Übersetzungs-Reference

Konsistente Übersetzungen zwischen German und Englisch:

| German | Englisch |
|---------|----------|
| Abhängigkeit | Dependency |
| Agent | Agent |
| Ausführbare Datei | Executable |
| Bibliothek | Library |
| Dienst / Service | Service |
| Enge Kopplung | Tight Coupling |
| Ereignis | Event |
| Configuration | Configuration |
| Lose Kopplung | Loose Coupling |
| Requiredfeld | Required field |
| Verzeichnis | Directory |
| Warning | Warning |
| Error | Error |
| Einstellung | Setting |
| Ziel | Target |
| Zustandsautomat | State Machine |

---

## 15. See Also

- [ErrorCodes.md](ErrorCodes.md) — Vollständige Errorcode-Reference
- [Solution_Schema.md](Solution_Schema.md) — JSON-Schema Reference
- [Cpp_Coding_Standard.md](../standards/Cpp_Coding_Standard.md) — Coding-Konventionen

---

## 16. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-24** | **Neue Abschnitte: 7. Architecture-Patterns (Service/Agent-Architecture, EventBus, CommandBus, Publish/Subscribe, Dependency Injection, Design-Prinzipien), 8. C++-Idiome (RAII, PIMPL, CRTP, Type-Erasure, StrongId, Rule of Zero/Five). Neue Abkürzungen: DI, EMA, FFT, FPS, LOC, RAII, TDD. Neue Datei-Extensions: .vert, .frag, .glsl** |
| 0.5.0 | 2025-12-14 | Blueprint v0.5.0 Format: Nummeriertes TOC, Reference-Header, vollständige Errorcode-Bereiche |
| 0.1.0 | 2025-12-05 | Initial: Begriffe aus allen Dokumentationen gesammelt |
