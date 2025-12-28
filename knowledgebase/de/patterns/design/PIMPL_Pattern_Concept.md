# PIMPL Pattern — Konzept

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Concept  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [PIMPL_Pattern_Concept.md](../../../en/patterns/design/PIMPL_Pattern_Concept.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum PIMPL?](#2-warum-pimpl)
3. [Kern-Architektur](#3-kern-architektur)
4. [Implementierung](#4-implementierung)
5. [ABI-Sicherheit](#5-abi-sicherheit)
6. [Rule of Five bei PIMPL](#6-rule-of-five-bei-pimpl)
7. [Performance-Überlegungen](#7-performance-überlegungen)
8. [Best Practices](#8-best-practices)
9. [Häufige Fehler](#9-häufige-fehler)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

**PIMPL** (Pointer to IMPLementation) ist ein C++-Idiom zur **Entkopplung von Interface und Implementierung**. Die öffentliche Klasse enthält nur einen Pointer auf eine private Implementierungsklasse.

### Andere Namen

- **Opaque Pointer**
- **Compiler Firewall**
- **Cheshire Cat** (nach Lewis Carroll)
- **d-pointer** (Qt-Terminologie)

### Grundstruktur

```
┌─────────────────────────────┐     ┌─────────────────────────────┐
│     Public Header           │     │     Private Implementation   │
│     (Example.hpp)           │     │     (ExampleImpl.hpp/.cpp)   │
├─────────────────────────────┤     ├─────────────────────────────┤
│ class Example {             │     │ class ExampleImpl {          │
│   std::unique_ptr<Impl> m;  │────►│   // Alle Member             │
│ public:                     │     │   // Alle privaten Methoden  │
│   void doSomething();       │     │ };                           │
│ };                          │     │                              │
└─────────────────────────────┘     └─────────────────────────────┘
        │                                      │
        │ Kompiliert zu DLL                    │ Nur in DLL sichtbar
        ▼                                      ▼
    Stabile ABI                         Änderbar ohne Rekompilierung
```

---

## 2. Warum PIMPL?

### 2.1 Compile-Time Entkopplung

Änderungen an privaten Membern erfordern **keine Rekompilierung** der nutzenden Projekte.

| Ohne PIMPL | Mit PIMPL |
|------------|-----------|
| Jede Header-Änderung → alle Abhängigen neu kompilieren | Nur `.cpp` ändert sich → nur Library neu kompilieren |
| Lange Build-Zeiten | Schnelle inkrementelle Builds |

### 2.2 ABI-Stabilität für DLLs

Die **Application Binary Interface** bleibt stabil:

```cpp
// Header bleibt identisch über Versionen
class EXPORT Example {
    std::unique_ptr<Impl> m_pImpl;
public:
    Example();
    ~Example();
    void doSomething();
};
```

Die `sizeof(Example)` ist immer `sizeof(std::unique_ptr)` — unabhängig von der Implementierung.

### 2.3 Verbergen von Abhängigkeiten

Private Abhängigkeiten erscheinen nicht im öffentlichen Header:

```cpp
// OHNE PIMPL — Nutzer muss <windows.h> haben
class Example {
    HANDLE m_handle;  // Erfordert <windows.h>
};

// MIT PIMPL — Nutzer braucht kein <windows.h>
class Example {
    std::unique_ptr<Impl> m_pImpl;  // Impl kennt HANDLE
};
```

---

## 3. Kern-Architektur

### 3.1 Datei-Struktur

```
project/
├── include/                    ← Öffentliche Header
│   └── Example.hpp             ← Forward Declaration + Interface
│
└── src/
    ├── ExampleImpl.hpp         ← Private Impl-Klasse (optional)
    └── Example.cpp             ← Impl-Definition + Wrapper
```

### 3.2 Forward Declaration

Der öffentliche Header enthält nur eine **Forward Declaration**:

```cpp
// Example.hpp
#pragma once
#include <memory>

class Example {
    class Impl;                           // Forward Declaration
    std::unique_ptr<Impl> m_pImpl;        // Opaque Pointer

public:
    Example();
    ~Example();                           // Muss im .cpp definiert werden!
    
    void doSomething();
};
```

> **Wichtig:** Der Destruktor **muss** in der `.cpp`-Datei definiert werden, da dort `Impl` vollständig bekannt ist.

### 3.3 Implementierungs-Klasse

```cpp
// Example.cpp
#include "Example.hpp"
#include <string>
#include <vector>

// Private Implementierung — hier dürfen beliebige Header inkludiert werden
class Example::Impl {
public:
    std::string m_name;
    std::vector<int> m_data;
    
    void internalMethod() {
        // Komplexe Logik
    }
};

// Konstruktor
Example::Example() : m_pImpl(std::make_unique<Impl>()) {}

// Destruktor — MUSS hier stehen
Example::~Example() = default;

// Wrapper-Methoden
void Example::doSomething() {
    m_pImpl->internalMethod();
}
```

---

## 4. Implementierung

### 4.1 Minimales Beispiel

```cpp
// ═══════════════════════════════════════════════════════════════════════════
// Logger.hpp — Öffentliches Interface
// ═══════════════════════════════════════════════════════════════════════════
#pragma once
#include <memory>
#include <string_view>

#ifdef LOGGER_EXPORTS
    #define LOGGER_API __declspec(dllexport)
#else
    #define LOGGER_API __declspec(dllimport)
#endif

class LOGGER_API Logger {
    class Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    Logger();
    ~Logger();
    
    // Keine Copy/Move — Singleton-artig
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(std::string_view message);
    void setLevel(int level);
};
```

```cpp
// ═══════════════════════════════════════════════════════════════════════════
// Logger.cpp — Private Implementierung
// ═══════════════════════════════════════════════════════════════════════════
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <mutex>

class Logger::Impl {
public:
    int m_level = 0;
    std::ofstream m_file;
    std::mutex m_mutex;
    
    void log(std::string_view message) {
        std::lock_guard lock(m_mutex);
        if (m_file.is_open()) {
            m_file << message << '\n';
        }
        std::cout << message << '\n';
    }
};

Logger::Logger() : m_pImpl(std::make_unique<Impl>()) {}
Logger::~Logger() = default;

void Logger::log(std::string_view message) {
    m_pImpl->log(message);
}

void Logger::setLevel(int level) {
    m_pImpl->m_level = level;
}
```

### 4.2 Mit Copy/Move Support

Wenn Kopieren/Verschieben erlaubt sein soll:

```cpp
// Header
class Example {
    class Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    Example();
    ~Example();
    
    // Copy
    Example(const Example& other);
    Example& operator=(const Example& other);
    
    // Move
    Example(Example&& other) noexcept;
    Example& operator=(Example&& other) noexcept;
};
```

```cpp
// Implementation
Example::Example(const Example& other)
    : m_pImpl(std::make_unique<Impl>(*other.m_pImpl)) {}

Example& Example::operator=(const Example& other) {
    if (this != &other) {
        *m_pImpl = *other.m_pImpl;
    }
    return *this;
}

Example::Example(Example&& other) noexcept = default;
Example& Example::operator=(Example&& other) noexcept = default;
```

---

## 5. ABI-Sicherheit

### 5.1 Was darf geändert werden?

| Änderung | ABI-sicher? | Begründung |
|----------|-------------|------------|
| Neue Member in `Impl` | ✅ Ja | Größe der öffentlichen Klasse ändert sich nicht |
| Neue private Methoden in `Impl` | ✅ Ja | Nicht Teil der ABI |
| Neue öffentliche Methode | ⚠️ Teilweise | Symbol muss neu gelinkt werden |
| Signatur ändern | ❌ Nein | Name Mangling ändert sich |
| Virtuelle Methode hinzufügen | ❌ Nein | VTable-Layout ändert sich |
| Member in öffentlicher Klasse ändern | ❌ Nein | Größe/Offset ändert sich |

### 5.2 Goldene Regel

> **Die öffentliche Klasse darf nur diese Member haben:**
> 1. `std::unique_ptr<Impl>` (oder raw pointer)
> 2. Statische Member (optional)

Jedes weitere Member bricht die ABI-Stabilität.

---

## 6. Rule of Five bei PIMPL

### 6.1 Entscheidungsmatrix

| Szenario | Copy | Move | Destruktor |
|----------|------|------|------------|
| **Nicht kopierbar** (z.B. Singleton) | `= delete` | `= delete` | `= default` in .cpp |
| **Nur verschiebbar** (z.B. Resource Handle) | `= delete` | `= default` | `= default` in .cpp |
| **Voll kopierbar** | Manuell | `= default` | `= default` in .cpp |

### 6.2 Warum Destruktor in .cpp?

```cpp
// FEHLER — Kompiliert nicht!
class Example {
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
public:
    // ~Example() = default; // ← unique_ptr braucht vollständigen Typ
};
```

Der Destruktor von `std::unique_ptr` muss `delete` aufrufen können, was einen vollständigen Typ erfordert. Im Header ist `Impl` nur forward-deklariert.

---

## 7. Performance-Überlegungen

### 7.1 Overhead

| Aspekt | Overhead | Mitigation |
|--------|----------|------------|
| Heap-Allokation | 1× bei Konstruktion | Meist vernachlässigbar |
| Indirection | 1 Pointer-Dereference pro Aufruf | CPU-Cache-freundlich halten |
| Cache-Miss | Möglich bei großem Impl | Kritische Daten nahe beieinander |

### 7.2 Wann PIMPL vermeiden?

- **Tight Loops** mit vielen Aufrufen pro Frame
- **Kleine, häufig kopierte Objekte** (z.B. Vektoren, Punkte)
- **Header-only Libraries** (PIMPL erfordert Linkage)

### 7.3 Optimierung: Fast PIMPL

Für performance-kritische Fälle:

```cpp
class FastExample {
    static constexpr size_t ImplSize = 128;  // Genug Platz für Impl
    static constexpr size_t ImplAlign = 8;
    
    alignas(ImplAlign) std::byte m_storage[ImplSize];
    
public:
    FastExample();   // Placement new in m_storage
    ~FastExample();  // Explicit destructor call
};
```

> **Warnung:** Dies erfordert manuelle Größen-/Alignment-Verwaltung und ist fehleranfällig.

---

## 8. Best Practices

### 8.1 Namenskonventionen

| Element | Konvention | Beispiel |
|---------|------------|----------|
| Impl-Klasse | `Impl` oder `<Klasse>Impl` | `Logger::Impl` |
| Pointer-Member | `m_pImpl` | Zeigt Pointer-Semantik |
| Impl-Header (optional) | `<Klasse>Impl.hpp` | `LoggerImpl.hpp` |

### 8.2 Checkliste

- [ ] Destruktor in `.cpp` definieren (nicht im Header)
- [ ] Copy/Move explizit behandeln
- [ ] `Impl` als `class Impl;` forward-deklarieren
- [ ] Keine `inline`-Methoden, die auf Impl zugreifen
- [ ] DLL-Export-Makro korrekt setzen

### 8.3 DLL-Export Pattern

```cpp
#ifdef _WIN32
    #ifdef MYLIB_EXPORTS
        #define MYLIB_API __declspec(dllexport)
    #else
        #define MYLIB_API __declspec(dllimport)
    #endif
#else
    #define MYLIB_API __attribute__((visibility("default")))
#endif
```

---

## 9. Häufige Fehler

### 9.1 Destruktor im Header

```cpp
// ❌ FEHLER
class Example {
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
public:
    ~Example() = default;  // error: incomplete type 'Impl'
};
```

**Lösung:** Destruktor in `.cpp` definieren.

### 9.2 Inline-Methoden mit Impl-Zugriff

```cpp
// ❌ FEHLER — im Header
inline void Example::doSomething() {
    m_pImpl->work();  // error: incomplete type
}
```

**Lösung:** Alle Methoden, die `Impl` nutzen, in `.cpp`.

### 9.3 Vergessene Copy-Semantik

```cpp
// ❌ Problem — Default Copy kopiert nur den Pointer
Example a;
Example b = a;  // b.m_pImpl zeigt auf gleiche Impl wie a!
```

**Lösung:** Copy explizit implementieren oder löschen.

---

## 10. Siehe auch

- [Rule_of_Five_Concept.md](Rule_of_Five_Concept.md) — Konstruktor-/Operator-Regeln
- [Singleton_Patterns_Reference.md](Singleton_Patterns_Reference.md) — Singleton-Varianten
- [DLL_Building_Guide.md](../../tooling/ide/DLL_Building_Guide.md) — DLL erstellen in Visual Studio

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus pimpl.md, pimpl_guide.md, pimpl_factory-Texten** |
