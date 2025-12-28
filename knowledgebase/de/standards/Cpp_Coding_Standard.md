# C++ Coding Standard

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Standard  
> **Status:** In Entwicklung  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Cpp_Coding_Standard.md](../../../en/standards/Cpp_Coding_Standard.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Namenskonventionen](#2-namenskonventionen)
3. [Dateien und Ordner](#3-dateien-und-ordner)
4. [Klassen-Struktur](#4-klassen-struktur)
5. [Präfixe](#5-präfixe)
6. [Code-Formatierung](#6-code-formatierung)
7. [Best Practices](#7-best-practices)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Dieser Standard definiert verbindliche Konventionen für C++-Code. Ziel ist konsistenter, lesbarer und wartbarer Code.

### Geltungsbereich

- Alle C++-Projekte
- Header-Dateien (`.hpp`, `.h`)
- Implementierungsdateien (`.cpp`, `.tpp`)
- Inline-Dateien (`.inl`)

---

## 2. Namenskonventionen

### 2.1 Allgemeine Regeln

| Regel | Beschreibung |
|-------|--------------|
| **Sprache** | Englisch |
| **Aussagekraft** | Beschreibend und klar |
| **Länge** | So kurz wie möglich, so lang wie nötig |
| **Konsistenz** | Einheitlich im gesamten Projekt |

### 2.2 Nach Elementtyp

| Element | Konvention | Beispiel |
|---------|------------|----------|
| **Klassen** | PascalCase | `FileManager`, `DataProcessor` |
| **Interfaces** | I + PascalCase | `ILogger`, `ISerializable` |
| **Funktionen** | camelCase | `processData()`, `getValue()` |
| **Variablen** | camelCase | `itemCount`, `isActive` |
| **Konstanten** | UPPER_SNAKE_CASE | `MAX_SIZE`, `DEFAULT_VALUE` |
| **Namespaces** | lowercase | `utils`, `network` |
| **Enums** | PascalCase | `enum class LogLevel` |
| **Enum-Werte** | PascalCase | `LogLevel::Error`, `LogLevel::Warning` |
| **Templates** | PascalCase (Typ) | `template<typename TValue>` |
| **Makros** | UPPER_SNAKE_CASE | `DEBUG_LOG(...)` |

### 2.3 Spezialfälle

| Element | Konvention | Beispiel |
|---------|------------|----------|
| **PIMPL-Klasse** | `Impl` | `class ExampleImpl;` |
| **Factory-Methoden** | `create...` | `createInstance()` |
| **Getter** | `get...` oder Substantiv | `getName()` oder `name()` |
| **Setter** | `set...` | `setName()` |
| **Boolean-Getter** | `is...`, `has...`, `can...` | `isActive()`, `hasData()` |

---

## 3. Dateien und Ordner

### 3.1 Dateinamen

| Regel | Beispiel |
|-------|----------|
| **PascalCase** | `FileManager.hpp`, `DataProcessor.cpp` |
| **Gleich wie Klasse** | Klasse `Logger` → `Logger.hpp` |
| **DLL-Ausnahme** | Suffix `Class`: `LoggerClass.hpp` |
| **PIMPL** | Suffix `Impl`: `LoggerImpl.hpp` |

### 3.2 Dateiendungen

| Endung | Verwendung |
|--------|------------|
| `.hpp` | C++ Header |
| `.cpp` | C++ Implementierung |
| `.tpp` | Template-Implementierung |
| `.inl` | Inline-Definitionen |
| `.h` | C Header (oder C-kompatible C++ Header) |
| `.c` | C Implementierung |

### 3.3 Ordnernamen

| Regel | Beispiel |
|-------|----------|
| **lowercase** | `source/`, `include/`, `tests/` |
| **snake_case bei Bedarf** | `user_data/` |

---

## 4. Klassen-Struktur

### 4.1 Reihenfolge der Sektionen

```cpp
class ClassName {
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

### 4.2 Beispiel

```cpp
class FileManager {
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

---

## 5. Präfixe

### 5.1 Member-Präfixe

| Präfix | Bedeutung | Beispiel |
|--------|-----------|----------|
| `m_` | Member-Variable | `m_name`, `m_count` |
| `s_` | Statische Variable | `s_instance` |
| `g_` | Globale Variable | `g_config` (vermeiden!) |
| — | Lokale Variable | `count`, `name` |

### 5.2 Pointer-Präfixe

| Präfix | Bedeutung | Beispiel |
|--------|-----------|----------|
| `p` | Pointer | `pBuffer` |
| `m_p` | Member-Pointer | `m_pImpl` |
| — | Smart Pointer | `m_data` (unique_ptr ist kein "roher" Pointer) |

### 5.3 Kombinationen

```cpp
class Example {
private:
    std::string m_name;              // Member
    int* m_pData;                    // Member-Pointer
    static Example* s_pInstance;     // Statischer Pointer
    std::unique_ptr<Impl> m_pImpl;   // PIMPL-Pointer
};
```

---

## 6. Code-Formatierung

### 6.1 Einrückung

- **4 Spaces** (keine Tabs)
- Konsistent im gesamten Projekt

### 6.2 Klammern

```cpp
// Funktionen: Klammer auf gleicher Zeile
void function() {
    // ...
}

// Klassen: Klammer auf neuer Zeile (optional)
class Example
{
    // ...
};

// If/For/While: Klammer auf gleicher Zeile
if (condition) {
    // ...
} else {
    // ...
}
```

### 6.3 Leerzeichen

```cpp
// Nach Kommas
function(a, b, c);

// Um Operatoren
x = a + b * c;

// Keine Leerzeichen in Klammern
function(a);      // ✅
function( a );    // ❌
```

### 6.4 Zeilenlänge

- **Maximum:** 120 Zeichen
- **Empfohlen:** 80-100 Zeichen

---

## 7. Best Practices

### 7.1 Header-Guards

```cpp
// Pragma once (modern)
#pragma once

// Oder klassisch
#ifndef PROJECT_MODULE_HEADER_HPP
#define PROJECT_MODULE_HEADER_HPP
// ...
#endif
```

### 7.2 Include-Reihenfolge

```cpp
// 1. Zugehöriger Header
#include "MyClass.hpp"

// 2. Projekt-Header
#include "utils/Logger.hpp"
#include "core/Config.hpp"

// 3. Drittanbieter-Header
#include <boost/algorithm/string.hpp>

// 4. Standard-Header
#include <string>
#include <vector>
#include <memory>
```

### 7.3 Const-Correctness

```cpp
// Const für unveränderliche Parameter
void process(const std::string& input);

// Const für unveränderliche Methoden
class Example {
public:
    [[nodiscard]] int getValue() const;
    [[nodiscard]] const std::string& getName() const;
};
```

### 7.4 Explizite Konstruktoren

```cpp
class Value {
public:
    // Verhindert implizite Konvertierung
    explicit Value(int v);
};
```

### 7.5 Nodiscard

```cpp
// Für wichtige Rückgabewerte
[[nodiscard]] bool validate();
[[nodiscard]] Error process();
```

---

## 8. Siehe auch

- [Cpp_Attributes_Reference.md](../languages/cpp/Cpp_Attributes_Reference.md) — Compiler-Attribute
- [Rule_of_Five_Concept.md](../patterns/design/Rule_of_Five_Concept.md) — Konstruktor-Regeln
- [Doxygen_Reference.md](../tooling/documentation/Doxygen_Reference.md) — Dokumentation

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus Conventions.md, namespace.md** |
