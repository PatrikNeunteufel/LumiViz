# C++ Coding Standard — Stil-Richtlinien

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Standard  
> **Status:** Stabil  
> **Zielgruppe:** Alle C++ Entwickler  
> **Geltungsbereich:** Alle C++-Projekte (PC-Applikationen, Tools, Treiber)  
> **Durchsetzung:** clang-format, clang-tidy, Code Review  
> **Sprache:** Deutsch  
> **English:** [Cpp_Coding_Standard.md](../../en/standards/Cpp_Coding_Standard.md)

---

## Inhaltsverzeichnis

1. [Zweck und Geltungsbereich](#1-zweck-und-geltungsbereich)
2. [Sprachversion und Features](#2-sprachversion-und-features)
3. [Grundprinzipien](#3-grundprinzipien)
4. [Datei-Header](#4-datei-header)
5. [Datei- und Modul-Organisation](#5-datei--und-modul-organisation)
6. [Formatierung](#6-formatierung)
7. [Namenskonventionen](#7-namenskonventionen)
8. [Klassen und Strukturen](#8-klassen-und-strukturen)
9. [Typen, Ownership und Lifetime](#9-typen-ownership-und-lifetime)
10. [Fehlerbehandlung](#10-fehlerbehandlung)
11. [Concurrency](#11-concurrency)
12. [Hardware-Zugriff (Optional)](#12-hardware-zugriff-optional)
13. [Statische Analyse](#13-statische-analyse)
14. [Test-Code](#14-test-code)
15. [Dokumentation](#15-dokumentation)
16. [Verhältnis zu C (Embedded)](#16-verhältnis-zu-c-embedded)
17. [Legacy-Code und Ausnahmen](#17-legacy-code-und-ausnahmen)
18. [MISRA/CERT-Alignment](#18-misracert-alignment)
19. [Siehe auch](#19-siehe-auch)
20. [Changelog](#20-changelog)

---

## 1. Zweck und Geltungsbereich

Dieser Standard definiert **Coding-Konventionen für C++** im Unternehmen.

### Zielgruppe

Dieser Standard richtet sich an alle Entwickler, die C++ Code für PC-Applikationen schreiben. Er ist verbindlich für neue Projekte und empfohlen für bestehenden Code bei Refactoring.

### Anwendungsbereich

| Sprache | Fokus | Typische Projekte |
|---------|-------|-------------------|
| **C++** | PC-Applikationen | Tools, GUIs, Services, Test-Programme, Libraries |
| C | Embedded | Firmware, MCUs, sicherheitskritische Teile |

Dieser Standard wird ergänzt durch:
- **C_Coding_Standard** — Embedded-spezifisch
- **CMake_Standard** — Build-System
- **ClangFormat_Blueprint** — Formatierung
- **ClangTidy_Blueprint** — Statische Analyse

### Tool-Autorität

> Bei Konflikten zwischen Dokumentation und Tool-Konfiguration gelten `.clang-format` und `.clang-tidy` als **verbindliche Umsetzung**.

---

## 2. Sprachversion und Features

### 2.1 Standard-Version

| Projekt-Typ | C++ Standard |
|-------------|--------------|
| Neue Projekte | **C++20** (Default) |
| Legacy-Projekte | Dokumentiert im README |

### 2.2 Empfohlene Features

| Feature | Verwendung |
|---------|------------|
| `enum class` | Stark typisierte Enums |
| RAII | Alle Ressourcen |
| Smart Pointer | Ownership-Management |
| `std::string_view` | Nicht-besitzende String-Referenz |
| `std::optional` | Optionale Werte |
| `constexpr` | Compile-Zeit-Berechnungen |
| Concepts (C++20) | Template-Constraints |

### 2.3 Zu vermeidende Features

| Feature | Alternative |
|---------|-------------|
| `new`/`delete` direkt | `std::make_unique`, `std::make_shared` |
| C-Style Casts | `static_cast`, `dynamic_cast`, etc. |
| `#define` für Konstanten | `constexpr`, `const` |
| Raw Arrays | `std::array`, `std::vector` |

---

## 3. Grundprinzipien

1. **Lesbarkeit über Cleverness**
2. **Sicherheit und Korrektheit über vorzeitige Optimierung**
3. **Konsistenz über persönliche Präferenz**
4. **Automatisierte Tools über manuelle Stil-Diskussionen**

---

## 4. Datei-Header

### 4.1 Standard-Header für C++ Dateien

Jede `.hpp` und `.cpp` Datei **muss** mit folgendem Doxygen-kompatiblen Header beginnen:

```cpp
/**
 ****************************************************************************************
 * @file   Filename.hpp
 * @brief  Short description
 *         Optional second line for context
 *
 * @author Author Name
 * @date   Month YYYY
 ****************************************************************************************
 */
```

### 4.2 Pflichtfelder

| Feld | Beschreibung |
|------|--------------|
| `@file` | Exakter Dateiname |
| `@brief` | Kurzbeschreibung (1-2 Zeilen) |
| `@author` | Hauptautor |
| `@date` | Erstellungsdatum (Monat Jahr) |

### 4.3 Optionale Felder

| Feld | Verwendung |
|------|------------|
| `@version` | Bei versionierten Komponenten |
| `@copyright` | Bei speziellen Lizenzen |
| `@see` | Verweise auf verwandte Dateien |

### 4.4 Beispiel

```cpp
/**
 ****************************************************************************************
 * @file   AudioEngine.hpp
 * @brief  Audio Engine Interface
 *         Provides high-level audio playback and management
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

#pragma once

#include <memory>
// ...
```

### 4.5 Sprache

- **Englisch** ist Pflicht für alle öffentlichen APIs und Templates
- Interne/projektspezifische Dateien können Deutsch verwenden, Englisch wird empfohlen

---

## 5. Datei- und Modul-Organisation

### 5.1 Dateinamen

| Regel | Beispiel |
|-------|----------|
| **PascalCase** | `FileManager.hpp`, `DataProcessor.cpp` |
| **Gleich wie Klasse** | Klasse `Logger` → `Logger.hpp` |
| **DLL-Ausnahme** | Suffix `Class`: `LoggerClass.hpp` |
| **PIMPL** | Suffix `Impl`: `LoggerImpl.hpp` |
| **Interface** | Präfix `I`: `ILogger.hpp` |

### 5.2 Dateiendungen

| Endung | Verwendung |
|--------|------------|
| `.hpp` | C++ Header |
| `.cpp` | C++ Implementierung |
| `.tpp` | Template-Implementierung |
| `.inl` | Inline-Definitionen |
| `.h` | C Header (oder C-kompatible C++ Header) |
| `.c` | C Implementierung |

### 5.3 Ordnernamen

| Regel | Beispiel |
|-------|----------|
| **lowercase** | `source/`, `include/`, `tests/` |
| **snake_case bei Bedarf** | `user_data/` |

### 5.4 Header-Guards

```cpp
// Pragma once (bevorzugt)
#pragma once

// Oder klassisch
#ifndef PROJECT_MODULE_HEADER_HPP
#define PROJECT_MODULE_HEADER_HPP
// ...
#endif
```

### 5.5 Include-Reihenfolge

```cpp
// 1. Precompiled Header (falls vorhanden)
#include "pch/pch.h"

// 2. Zugehöriger Header
#include "MyClass.hpp"

// 3. System-Header
#include <string>
#include <vector>
#include <memory>

// 4. Projekt-Header
#include "utils/Logger.hpp"
#include "core/Config.hpp"
```

> **Hinweis:** clang-format sortiert Includes automatisch gemäß `.clang-format`.

---

## 6. Formatierung

### 6.1 Autorität

- Alle Formatierung via `clang-format`
- Manuelle Abweichungen nicht erlaubt
- Bei Problemen: `.clang-format` anpassen, nicht umgehen

### 6.2 Übersicht (Details in `.clang-format`)

| Aspekt | Regel |
|--------|-------|
| Basis-Stil | LLVM mit Anpassungen |
| Einrückung | 4 Spaces |
| Tabs | Nie verwenden |
| Klammern | **Allman-Stil** (neue Zeile) |
| Zeilenlänge | 80 Zeichen |
| Pointer | Links ausgerichtet (`int* ptr`) |

### 6.3 Klammern-Stil (Allman)

```cpp
// Funktionen
void function()
{
    // ...
}

// Klassen
class Example
{
public:
    void method();
};

// Kontrollstrukturen
if (condition)
{
    // ...
}
else
{
    // ...
}

// Schleifen
for (int i = 0; i < count; ++i)
{
    // ...
}
```

### 6.4 Leerzeichen

```cpp
// Nach Kommas
function(a, b, c);

// Um Operatoren
x = a + b * c;

// Vor Klammern
if (condition)
while (running)
for (auto& item : items)

// Keine Leerzeichen in Klammern
function(a);      // ✅
function( a );    // ❌
```

---

## 7. Namenskonventionen

### 7.1 Autorität

- Namensregeln via `clang-tidy` (`readability-identifier-naming`)
- Verstöße beheben, nicht unterdrücken

### 7.2 Übersicht

| Entität | Konvention | Beispiel |
|---------|------------|----------|
| Namespace | `lower_case` | `audio`, `core_utils` |
| Klasse/Struct | `PascalCase` | `LogManager`, `AudioBuffer` |
| Interface | `I` + `PascalCase` | `ILogger`, `ISerializable` |
| Enum | `PascalCase` | `LogLevel` |
| Enum-Konstante | `PascalCase` | `LogLevel::Info`, `LogLevel::Error` |
| Funktion/Methode | `camelCase` | `writeLog()`, `processData()` |
| Parameter | `camelCase` | `filePath`, `bufferSize` |
| Lokale Variable | `camelCase` | `currentIndex`, `tempValue` |
| Member-Variable | `m_` Präfix | `m_buffer`, `m_logger` |
| Globale Konstante | `UPPER_CASE` | `MAX_BUFFER_SIZE` |
| Globale Variable | `g_` Präfix | `g_logger` |
| Statische Variable | `s_` Präfix | `s_cache` |
| Template-Parameter | `T` + `PascalCase` | `TValue`, `TContainer` |
| Makros | `UPPER_CASE` | `DEBUG_LOG(...)` |

### 7.3 Präfixe

| Präfix | Bedeutung | Beispiel |
|--------|-----------|----------|
| `m_` | Member-Variable | `m_name`, `m_count` |
| `s_` | Statische Variable | `s_instance` |
| `g_` | Globale Variable | `g_config` (vermeiden!) |
| `p` | Pointer (lokal oder Parameter) | `pBuffer`, `pConfig` |
| `m_p` | Member-Pointer | `m_pImpl`, `m_pData` |
| `s_p` | Statischer Pointer | `s_pInstance` |
| — | Lokale Variable / Parameter | `count`, `name`, `value` |
| — | Smart Pointer Member | `m_data` (unique_ptr ist kein "roher" Pointer) |

### 7.4 Spezialfälle

| Element | Konvention | Beispiel |
|---------|------------|----------|
| **PIMPL-Klasse** | `Impl` | `class ExampleImpl;` |
| **Factory-Methoden** | `create...` | `createInstance()` |
| **Getter** | `get...` oder Substantiv | `getName()` oder `name()` |
| **Setter** | `set...` | `setName()` |
| **Boolean-Getter** | `is...`, `has...`, `can...` | `isActive()`, `hasData()` |

---

## 8. Klassen und Strukturen

### 8.1 Wann struct vs class

| Verwende | Für | Beispiel |
|----------|-----|----------|
| `struct` | POD-Typen, DTOs, Aggregate ohne Invarianten | `struct Point { int x; int y; };` |
| `class` | Typen mit Invarianten, Kapselung, Methoden | `class FileManager { ... };` |

**Faustregel:** Wenn alle Member öffentlich sind und keine Invarianten zu schützen sind → `struct`.

```cpp
// ✅ struct: Reiner Datencontainer
struct ConfigData
{
    std::string name;
    int timeout;
    bool enabled;
};

// ✅ class: Kapselung und Invarianten
class Connection
{
public:
    explicit Connection(const std::string& host);
    void send(std::span<const std::byte> data);
private:
    Socket m_socket;
    bool m_connected;
};
```


### 8.2 Klassen-Layout

#### Reihenfolge der Sektionen

```cpp
class ClassName
{
/* ═══════════════════════════════════════════════════════════════ */
/* Members                                                          */
/* ═══════════════════════════════════════════════════════════════ */
public:     /* Öffentliche Member (selten) */
protected:  /* Geschützte Member */
private:    /* Private Member */

/* ═══════════════════════════════════════════════════════════════ */
/* Constructors / Destructors                                       */
/* ═══════════════════════════════════════════════════════════════ */
public:     /* Öffentliche Konstruktoren */
protected:  /* Geschützte Konstruktoren (für Basisklassen) */
private:    /* Private Konstruktoren (Singleton, Factory) */

/* ═══════════════════════════════════════════════════════════════ */
/* Methods                                                          */
/* ═══════════════════════════════════════════════════════════════ */
public:     /* Öffentliche Methoden */
protected:  /* Geschützte Methoden */
private:    /* Private Methoden */
};
```

#### Beispiel

```cpp
class FileManager
{
/* Members */
private:
    std::string m_basePath;
    std::vector<FileHandle> m_openFiles;
    static size_t s_instanceCount;

/* Constructors / Destructors */
public:
    explicit FileManager(const std::string& basePath);
    ~FileManager();
    
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    
    FileManager(FileManager&&) noexcept;
    FileManager& operator=(FileManager&&) noexcept;

/* Methods */
public:
    bool openFile(const std::string& name);
    void closeAll();
    
    [[nodiscard]] size_t openFileCount() const;
    [[nodiscard]] bool isFileOpen(const std::string& name) const;

private:
    void validatePath(const std::string& path);
};
```

### 8.3 Struct-Layout

Structs folgen denselben Layout-Regeln wie in C:

```cpp
// ✅ Gut: Größere Typen zuerst (minimiert Padding)
struct Message
{
    std::uint64_t timestamp;    // 8 bytes
    std::uint32_t id;           // 4 bytes
    std::uint16_t flags;        // 2 bytes
    std::uint8_t  priority;     // 1 byte
    std::uint8_t  reserved;     // 1 byte (explizites Padding)
};  // Total: 16 bytes, kein verstecktes Padding
```

**C++20:** `[[no_unique_address]]` für leere Member (z.B. Allocators).

### 8.4 Union und std::variant

| Verwende | Für | Hinweis |
|----------|-----|---------|
| `std::variant` | Typsichere Alternative | **Bevorzugt** |
| `union` | Low-Level, Hardware-Interop, C-Kompatibilität | Mit Vorsicht |

```cpp
// ✅ Bevorzugt: std::variant (typsicher)
using Value = std::variant<int, double, std::string>;

Value v = 42;
if (auto* pInt = std::get_if<int>(&v))
{
    // Sichere Verwendung
}

// ⚠️ Nur wenn nötig: union (für Hardware-Interop)
union RegisterValue
{
    struct
    {
        std::uint16_t low;
        std::uint16_t high;
    } parts;
    std::uint32_t full;
};
```

### 8.5 Bitfields (für Hardware-nahe Tools)

Für Protokoll-Parser, Hardware-Interop oder C-API-Kompatibilität:

| Kürzel | Typ | Bits |
|--------|-----|------|
| `b` | bit (struct) | variabel |
| `c` | char | 8 |
| `s` | short | 16 |
| `l` | long | 32 |
| `ll` | long long | 64 |

```cpp
// Register-Definition für Hardware-Tool
union ControlRegister
{
    struct
    {
        std::uint16_t enable   : 1;   // Bit 0
        std::uint16_t mode     : 2;   // Bit 1-2
        std::uint16_t reserved : 13;  // Bit 3-15
    } b;                              // Bitfield access
    std::uint16_t s;                  // 16-bit access
};
```

> **Hinweis:** Bitfield-Layout ist implementation-defined. Für portable Protokolle besser Bit-Masken und Shifts verwenden.

### 8.6 Best Practices

```cpp
// [[nodiscard]] für wichtige Rückgabewerte
[[nodiscard]] bool validate();
[[nodiscard]] std::optional<r> process();

// explicit für Ein-Parameter-Konstruktoren
class Value
{
public:
    explicit Value(int v);  // Verhindert implizite Konvertierung
};

// const-correctness
class Example
{
public:
    [[nodiscard]] int getValue() const;
    [[nodiscard]] const std::string& getName() const;
};
```

---

## 9. Typen, Ownership und Lifetime

### 9.1 Fundamentale Typen

| Anforderung | Typ |
|-------------|-----|
| Größe wichtig | `std::int32_t`, `std::uint64_t` |
| Größen/Indizes | `std::size_t` |

### 9.2 Ownership-Modell

| Ownership | Mechanismus | Verwendung |
|-----------|-------------|------------|
| Exklusiv | `std::unique_ptr` | **Default für Heap-Objekte** |
| Geteilt | `std::shared_ptr` | Nur wenn echtes Shared Ownership nötig |
| Nicht-besitzend | Raw Pointer, Reference, `std::span` | Temporäre Referenzen |

### 9.3 Smart Pointer vs Raw Pointer

**Smart Pointer (bevorzugt):**

```cpp
// ✅ Ownership klar, automatisches Cleanup
auto pResource = std::make_unique<Resource>();
auto pShared = std::make_shared<Config>();
```

**Raw Pointer (erlaubt für non-owning):**

```cpp
// ✅ Non-owning Referenz (Ownership liegt woanders)
void processData(const Data* pData);

// ✅ Optionaler Parameter (nullptr = nicht gesetzt)
void configure(const Config* pOptionalConfig = nullptr);

// ✅ C-API Interop
extern "C" void legacyFunction(void* pUserData);

// ❌ Verboten: Owning Raw Pointer
Resource* pRes = new Resource();  // Wer löscht das?
```

**Präfix-Regel:** Raw Pointer mit `p` Präfix kennzeichnen.

| Typ | Präfix | Beispiel |
|-----|--------|----------|
| Raw Pointer (lokal) | `p` | `pBuffer`, `pConfig` |
| Raw Pointer (member) | `m_p` | `m_pImpl`, `m_pData` |
| Smart Pointer (member) | `m_` | `m_resource` (kein `p`, da nicht "raw") |

### 9.4 RAII

Alle Ressourcen werden durch RAII verwaltet:
- Dateien, Sockets, Handles
- Speicher
- Locks

```cpp
// ✅ RAII
{
    std::unique_ptr<Resource> res = createResource();
    // Automatic cleanup at scope end
}

// ❌ Manual
Resource* res = createResource();
// ... forgotten delete = leak
delete res;
```
---

## 10. Fehlerbehandlung

### 10.1 Exceptions (C++ PC)

Exceptions sind **erlaubt und erwartet**:

| Regel | Beschreibung |
|-------|--------------|
| Werfen | By Value |
| Fangen | By (const) Reference |
| Verwendung | Echte Ausnahmesituationen |
| Nicht verwenden für | Normalen Kontrollfluss |

### 10.2 Alternative Fehlerbehandlung

Für Low-Level-Code (I/O, OS-Interfaces):

| Mechanismus | Verwendung |
|-------------|------------|
| `std::error_code` | Erwartete Fehler |
| `std::optional<T>` | Optionale Rückgabe |
| `std::expected` (C++23) | Fehler oder Wert |

### 10.3 Logging

- Zentrales Logging-System verwenden (z.B. `LogManager`)
- Keine `std::cout` / `printf` in Produktionscode
- Exceptions an Grenzen loggen oder propagieren

### 10.4 Multiple Conditions

Bei mehreren Bedingungen: **jede Bedingung in Klammern** für Klarheit.

```cpp
// ✅ Richtig: Jede Bedingung in Klammern
if ((value > minValue) && (value < maxValue))
{
    // ...
}

if ((condition1) && (condition2) || (condition3))
{
    // ...
}

// ❌ Falsch: Keine Klammern
if (value > minValue && value < maxValue)
{
    // ...
}
```

### 10.5 Division — Divisor prüfen

Division durch 0 führt zu undefiniertem Verhalten. **Divisor validieren** oder Exception werfen.

```cpp
// ✅ Mit Exception
if (divisor == 0)
{
    throw std::invalid_argument("Division by zero");
}
double result = value / divisor;

// ✅ Mit optional
std::optional<double> safeDivide(double value, double divisor)
{
    if (divisor == 0.0)
    {
        return std::nullopt;
    }
    return value / divisor;
}
```

---

## 11. Concurrency

### 11.1 Empfohlene Mechanismen

| Mechanismus | Verwendung |
|-------------|------------|
| `std::thread` / `std::jthread` | Thread-Erzeugung |
| `std::mutex` / `std::shared_mutex` | Synchronisation |
| `std::lock_guard` / `std::unique_lock` | RAII-Locking |
| `std::atomic<T>` | Atomare Operationen |

### 11.2 Richtlinien

- **Keine Data Races** — Shared Data immer schützen
- **Kurze kritische Sektionen** — Locks minimal halten
- **Thread-Safe Design bevorzugen** — Immutable Data, Message Passing

---

## 12. Hardware-Zugriff (Optional)

> **Geltungsbereich:** Treiber-Entwicklung, Hardware-nahe PC-Tools, System-Programmierung

### 12.1 Anwendungsfälle

| Kontext | Beispiele |
|---------|-----------|
| Windows-Treiber | WDK (Windows Driver Kit) |
| Linux-Kernel | Kernel-Module |
| Hardware-Tools | Protokoll-Analyzer, Debug-Tools |
| Memory-Mapped I/O | Über OS-spezifische APIs |

### 12.2 Register-Zugriff

Für Hardware-Tools, die Register-Definitionen benötigen:

```cpp
// Memory-Mapped I/O (über OS-API)
volatile std::uint32_t* pRegister = 
    reinterpret_cast<volatile std::uint32_t*>(mappedAddress);

// Lesen/Schreiben
std::uint32_t value = *pRegister;
*pRegister = newValue;
```

> **Wichtig:** `volatile` für Hardware-Register verwenden. Zugriff nur über OS-APIs (keine direkten Adressen in User-Mode).

### 12.3 Interrupt-Handling

Auf PC-Systemen werden Interrupts typischerweise nicht direkt behandelt:

| Plattform | Mechanismus |
|-----------|-------------|
| Windows User-Mode | Events, Callbacks |
| Windows Kernel-Mode | ISR via WDK |
| Linux User-Mode | Signals, epoll |
| Linux Kernel-Mode | IRQ Handler |

Für die meisten C++ PC-Applikationen sind Events/Callbacks das richtige Pattern.

---

## 13. Statische Analyse

### 13.1 Default-Profil: Dev-Gentle

Aktivierte Check-Kategorien:
- `clang-analyzer-*` — Kritische Bugs
- `bugprone-*` — Logik-Fehler
- `performance-*` — Ineffizienzen
- `readability-*` — Lesbarkeit
- `modernize-*` — C++-Modernisierung

### 13.2 Umgang mit Warnungen

| Warnung | Anforderung |
|---------|-------------|
| `clang-analyzer-*`, `bugprone-*` | **Beheben** oder dokumentiert unterdrücken |
| Stil-Warnungen | Zeitnah beheben, nicht ignorieren |

### 13.3 Profile

| Profil | Kontext |
|--------|---------|
| Dev-Gentle | Tägliche Entwicklung |
| CI-Strict | Pull Requests |
| API-Gate | Öffentliche APIs |

---

## 14. Test-Code

- Test-Code folgt **demselben Standard**
- Test-spezifische Abkürzungen bleiben in Tests
- Verwendete Frameworks (GoogleTest, Catch2) respektieren Naming-Regeln

---

## 15. Dokumentation

### 15.1 TODO-Marker

Für unfertige Aufgaben, Ideen oder zu behebende Probleme: `TODO` im Kommentar verwenden.

```cpp
// TODO: Implement timeout handling
// TODO: Optimize buffer allocation
// TODO: Add error recovery for communication failure
```

IDEs können TODO-Marker automatisch erkennen und auflisten (z.B. VS Code: Todo Tree Extension, Visual Studio: Task List).

### 15.2 Code-Kommentare

| Kommentar-Typ | Verwendung |
|---------------|------------|
| `//` | Kurze Inline-Kommentare, temporäre Notizen |
| `/* */` | Mehrzeilige Kommentare, Sektions-Header |
| `///` oder `/** */` | Doxygen-Dokumentation |

### 15.3 Funktions- und Klassen-Dokumentation

Öffentliche APIs mit Doxygen dokumentieren:

```cpp
/**
 * @brief  Process incoming data packets
 * @param  data Span of bytes to process
 * @param  config Optional processing configuration
 * @return Number of processed bytes, or error code
 * @throws std::invalid_argument if data is empty
 * @note   Thread-safe
 */
[[nodiscard]] std::expected<size_t, Error> 
processPackets(std::span<const std::byte> data,
               const Config* pConfig = nullptr);
```

| Tag | Verwendung |
|-----|------------|
| `@brief` | Kurzbeschreibung (Pflicht für public API) |
| `@param` | Parameter-Beschreibung |
| `@return` | Rückgabewert |
| `@throws` | Geworfene Exceptions |
| `@note` | Wichtige Hinweise |
| `@warning` | Warnungen, Einschränkungen |
| `@see` | Verwandte Funktionen/Klassen |

### 15.4 Sprache

| Kontext | Sprache |
|---------|---------|
| Öffentliche APIs | **Englisch** (Pflicht) |
| Libraries | **Englisch** (Pflicht) |
| Interne Tools | Deutsch erlaubt |
| Commit-Messages | Englisch empfohlen |

---

## 16. Verhältnis zu C (Embedded)

| Aspekt | C++ (PC) | C (Embedded) |
|--------|----------|--------------|
| Exceptions | Ja | Nein |
| Dynamic Allocation | Erlaubt | Vermeiden |
| Standard Library | Voll | Eingeschränkt |
| MISRA/CERT | Alignment | Strikte Einhaltung |

### 16.1 Shared Components

Libraries für PC und Embedded müssen dokumentieren:
- Verwendete C++-Subset
- Embedded-Einschränkungen

---

## 17. Legacy-Code und Ausnahmen

### 17.1 Legacy-Code

- Bleibt temporär, wenn nicht compliant
- Neuer Code **immer** nach Standard
- Refactoring-Chancen nutzen

### 17.2 Intentionale Abweichungen

- Kommentar im Code
- Begründung dokumentieren
- Scope minimieren

---

## 18. MISRA/CERT-Alignment

Dieser Standard orientiert sich an:

| Richtlinie | Relevanz |
|------------|----------|
| MISRA C++ | Sicherheitskritischer Code |
| SEI CERT C++ | Defensive Programmierung |

### Umgesetzte Prinzipien

- Keine gefährlichen Casts
- Keine implizite Truncation
- Rückgabewerte prüfen
- Variablen initialisieren

---

## 19. Siehe auch

- [C_Coding_Standard.md](C_Coding_Standard.md) — Embedded C
- [CMake_Standard.md](CMake_Standard.md) — Build-System
- [Git_Standard.md](Git_Standard.md) — Versionskontrolle
- [ClangFormat_Blueprint.md](../blueprints/ClangFormat_Blueprint.md) — Formatierung
- [ClangTidy_Blueprint.md](../blueprints/ClangTidy_Blueprint.md) — Statische Analyse

---

## 20. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.9.0** | **2025-12-19** | **Neu: §8 erweitert (Struct, Union, Bitfields), §9 Raw Pointer Regeln, §12 Hardware-Zugriff, §15 Dokumentation; §2↔§3 getauscht** |
| 0.8.0 | 2025-12-19 | Neu: TODO-Marker (2.1), Multiple Conditions in Klammern (10.4), Division durch 0 prüfen (10.5) |
| 0.7.0 | 2025-12-19 | Vereinheitlichung: Präfix-Tabelle klargestellt (p für alle Pointer) |
| 0.6.0 | 2025-12-19 | Konsolidierung: Neuer Abschnitt 8 Klassen-Struktur, erweiterte Präfix-Tabelle (7.3), Dateinamen-Konventionen (5.1), Interface-Präfix I, Include-Reihenfolge (5.5), nodiscard/explicit Best Practices (8.3) |
| 0.5.1 | 2025-12-18 | Neuer Abschnitt 4: Datei-Header mit Doxygen-Format, Pflichtfelder, Sprachregelung |
| 0.5.0 | 2025-12-13 | Migration auf Blueprint v0.5: Neuer Header, Inhaltsverzeichnis, Encoding-Fix |
| 0.1.0 | 2025-12-05 | Initial: Namenskonventionen, Ownership, Exceptions, MISRA-Alignment |
