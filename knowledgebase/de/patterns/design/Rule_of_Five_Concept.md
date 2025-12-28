# Rule of Five — Konzept

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Concept  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Rule_of_Five_Concept.md](../../../en/patterns/design/Rule_of_Five_Concept.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Die fünf Spezialmember](#2-die-fünf-spezialmember)
3. [Rule of Zero](#3-rule-of-zero)
4. [Rule of Five](#4-rule-of-five)
5. [Entscheidungsmatrix](#5-entscheidungsmatrix)
6. [Implementierungsbeispiele](#6-implementierungsbeispiele)
7. [Abstrakte Klassen](#7-abstrakte-klassen)
8. [Best Practices](#8-best-practices)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Die **Rule of Five** (früher Rule of Three) besagt: Wenn eine Klasse einen der fünf speziellen Member-Funktionen explizit definiert, sollte sie alle fünf explizit behandeln.

### Warum?

```cpp
// ❌ Gefährlich — nur Destruktor definiert
class ResourceHolder {
    int* m_data;
public:
    ResourceHolder() : m_data(new int[100]) {}
    ~ResourceHolder() { delete[] m_data; }
    // Copy-Konstruktor: Compiler generiert shallow copy!
    // → Double-Free bei Kopie
};
```

---

## 2. Die fünf Spezialmember

| Member | Signatur | Zweck |
|--------|----------|-------|
| **Destruktor** | `~T()` | Ressourcen freigeben |
| **Copy-Konstruktor** | `T(const T&)` | Kopie erstellen |
| **Copy-Zuweisung** | `T& operator=(const T&)` | Kopie zuweisen |
| **Move-Konstruktor** | `T(T&&) noexcept` | Ressourcen übernehmen |
| **Move-Zuweisung** | `T& operator=(T&&) noexcept` | Ressourcen übernehmen |

### Compiler-generierte Versionen

Der Compiler generiert diese Funktionen automatisch, **außer**:

| Wenn definiert... | ...wird NICHT generiert |
|-------------------|-------------------------|
| Irgendein Konstruktor | Default-Konstruktor |
| Destruktor | Move-Konstruktor, Move-Zuweisung |
| Copy-Konstruktor | Move-Konstruktor, Move-Zuweisung |
| Move-Konstruktor | Copy-Konstruktor, Copy-Zuweisung |
| Move-Zuweisung | Copy-Konstruktor, Copy-Zuweisung |

---

## 3. Rule of Zero

> **Bevorzuge die Rule of Zero:** Vermeide manuelle Ressourcenverwaltung.

```cpp
// ✅ Rule of Zero — keine Spezialmember nötig
class Person {
    std::string m_name;
    std::vector<std::string> m_hobbies;
    std::unique_ptr<Address> m_address;
    
    // Compiler generiert korrekte Copy/Move/Destruktor
};
```

**Wann möglich:** Nutze RAII-Wrapper (`std::unique_ptr`, `std::shared_ptr`, `std::vector`, etc.) für alle Ressourcen.

---

## 4. Rule of Five

Wenn Rule of Zero nicht möglich ist (z.B. bei C-API-Wrappern), definiere alle fünf:

```cpp
class FileHandle {
    FILE* m_file;
    
public:
    // Konstruktor
    explicit FileHandle(const char* path) 
        : m_file(std::fopen(path, "r")) {}
    
    // 1. Destruktor
    ~FileHandle() {
        if (m_file) std::fclose(m_file);
    }
    
    // 2. Copy-Konstruktor
    FileHandle(const FileHandle& other) 
        : m_file(nullptr) {
        // Nicht kopierbar oder: Deep Copy implementieren
        throw std::runtime_error("FileHandle cannot be copied");
    }
    
    // 3. Copy-Zuweisung
    FileHandle& operator=(const FileHandle& other) {
        throw std::runtime_error("FileHandle cannot be copied");
    }
    
    // 4. Move-Konstruktor
    FileHandle(FileHandle&& other) noexcept 
        : m_file(std::exchange(other.m_file, nullptr)) {}
    
    // 5. Move-Zuweisung
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (m_file) std::fclose(m_file);
            m_file = std::exchange(other.m_file, nullptr);
        }
        return *this;
    }
};
```

---

## 5. Entscheidungsmatrix

### 5.1 Nach Ressourcentyp

| Ressource | Copy | Move | Destruktor |
|-----------|------|------|------------|
| **Keine (nur Werte)** | `= default` | `= default` | `= default` |
| **RAII-Wrapper (unique_ptr)** | `= delete` | `= default` | `= default` |
| **RAII-Wrapper (shared_ptr)** | `= default` | `= default` | `= default` |
| **Raw Pointer (besitzend)** | Deep Copy oder `= delete` | Transfer | Freigeben |
| **Handle/Socket** | `= delete` oder Duplicate | Transfer | Schließen |
| **Singleton** | `= delete` | `= delete` | `= default` |
| **Interface (abstrakt)** | Meist `= delete` | Meist `= delete` | `virtual = default` |

### 5.2 Nach Klassentyp

| Klassentyp | Konstruktor | Copy | Move | Destruktor |
|------------|-------------|------|------|------------|
| **Daten-Klasse (POD)** | `= default` | `= default` | `= default` | `= default` |
| **Resource Wrapper** | Explizit | `= delete` | Explizit | Explizit |
| **Abstrakte Basis** | `protected` | `= delete` | `= delete` | `virtual = default` |
| **Factory-Klasse** | `= delete` | `= delete` | `= delete` | `= delete` |
| **PIMPL-Klasse** | Explizit | Explizit oder `= delete` | `= default` (in .cpp) | `= default` (in .cpp) |

---

## 6. Implementierungsbeispiele

### 6.1 Nicht kopierbar, nur verschiebbar

```cpp
class UniqueResource {
    int* m_data;
    
public:
    explicit UniqueResource(size_t size) 
        : m_data(new int[size]) {}
    
    ~UniqueResource() { delete[] m_data; }
    
    // Copy verbieten
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource& operator=(const UniqueResource&) = delete;
    
    // Move erlauben
    UniqueResource(UniqueResource&& other) noexcept 
        : m_data(std::exchange(other.m_data, nullptr)) {}
    
    UniqueResource& operator=(UniqueResource&& other) noexcept {
        if (this != &other) {
            delete[] m_data;
            m_data = std::exchange(other.m_data, nullptr);
        }
        return *this;
    }
};
```

### 6.2 Voll kopierbar (Deep Copy)

```cpp
class DeepCopyable {
    int* m_data;
    size_t m_size;
    
public:
    explicit DeepCopyable(size_t size) 
        : m_data(new int[size]), m_size(size) {}
    
    ~DeepCopyable() { delete[] m_data; }
    
    // Deep Copy
    DeepCopyable(const DeepCopyable& other) 
        : m_data(new int[other.m_size]), m_size(other.m_size) {
        std::copy(other.m_data, other.m_data + m_size, m_data);
    }
    
    DeepCopyable& operator=(const DeepCopyable& other) {
        if (this != &other) {
            delete[] m_data;
            m_size = other.m_size;
            m_data = new int[m_size];
            std::copy(other.m_data, other.m_data + m_size, m_data);
        }
        return *this;
    }
    
    // Move
    DeepCopyable(DeepCopyable&& other) noexcept 
        : m_data(std::exchange(other.m_data, nullptr))
        , m_size(std::exchange(other.m_size, 0)) {}
    
    DeepCopyable& operator=(DeepCopyable&& other) noexcept {
        if (this != &other) {
            delete[] m_data;
            m_data = std::exchange(other.m_data, nullptr);
            m_size = std::exchange(other.m_size, 0);
        }
        return *this;
    }
};
```

### 6.3 Copy-and-Swap Idiom

```cpp
class SwapBased {
    int* m_data;
    size_t m_size;
    
public:
    explicit SwapBased(size_t size) 
        : m_data(new int[size]), m_size(size) {}
    
    ~SwapBased() { delete[] m_data; }
    
    // Copy-Konstruktor
    SwapBased(const SwapBased& other) 
        : m_data(new int[other.m_size]), m_size(other.m_size) {
        std::copy(other.m_data, other.m_data + m_size, m_data);
    }
    
    // Unified Assignment (Copy und Move)
    SwapBased& operator=(SwapBased other) noexcept {
        swap(*this, other);
        return *this;
    }
    
    // Move-Konstruktor
    SwapBased(SwapBased&& other) noexcept 
        : m_data(nullptr), m_size(0) {
        swap(*this, other);
    }
    
    friend void swap(SwapBased& a, SwapBased& b) noexcept {
        using std::swap;
        swap(a.m_data, b.m_data);
        swap(a.m_size, b.m_size);
    }
};
```

---

## 7. Abstrakte Klassen

### 7.1 Virtueller Destruktor

Abstrakte Basisklassen benötigen einen **virtuellen Destruktor**:

```cpp
class IInterface {
public:
    virtual ~IInterface() = default;  // Immer virtual!
    virtual void doSomething() = 0;
    
    // Copy/Move verbieten (Interface sollte nicht kopiert werden)
    IInterface(const IInterface&) = delete;
    IInterface& operator=(const IInterface&) = delete;

protected:
    IInterface() = default;
};
```

### 7.2 Vererbungshierarchie

| Klasse | Destruktor | Copy/Move |
|--------|------------|-----------|
| **Interface (abstrakt)** | `virtual = default` | `= delete` |
| **Abstrakte Basisklasse** | `virtual = default` | Abhängig von Semantik |
| **Konkrete Klasse** | `override = default` oder explizit | Abhängig von Ressourcen |

---

## 8. Best Practices

### 8.1 Checkliste

- [ ] Ressourcen identifizieren (Pointer, Handles, etc.)
- [ ] Rule of Zero prüfen (RAII-Wrapper möglich?)
- [ ] Wenn nein: Alle fünf Spezialmember explizit behandeln
- [ ] `noexcept` für Move-Operationen
- [ ] Selbstzuweisung prüfen bei Assignment-Operatoren
- [ ] Bei Vererbung: Virtueller Destruktor

### 8.2 Modern C++ Empfehlungen

1. **Bevorzuge `= default`** für triviale Implementierungen
2. **Bevorzuge `= delete`** über private/nicht-deklariert
3. **Nutze `std::exchange`** für Move-Implementierungen
4. **Nutze Copy-and-Swap** für exception-sichere Zuweisung

### 8.3 noexcept Regel

Move-Operationen sollten `noexcept` sein:

```cpp
// ✅ Standard-Container nutzen noexcept-Move
MyClass(MyClass&&) noexcept;
MyClass& operator=(MyClass&&) noexcept;
```

Grund: `std::vector` nutzt Move nur, wenn es `noexcept` ist (sonst Copy für Sicherheit).

---

## 9. Siehe auch

- [PIMPL_Pattern_Concept.md](PIMPL_Pattern_Concept.md) — Spezielle Copy/Move-Semantik bei PIMPL
- [Cpp_Traits_Guide.md](../../languages/cpp/Cpp_Traits_Guide.md) — Type Traits für SFINAE

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus Rule_of_Five_Design_Guide.md, Konstruktor_Operator-Regel.md** |
