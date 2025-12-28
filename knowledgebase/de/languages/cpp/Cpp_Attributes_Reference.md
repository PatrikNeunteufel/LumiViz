# C++ Attribute — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Cpp_Attributes_Reference.md](../../../en/languages/cpp/Cpp_Attributes_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Standard-Attribute](#3-standard-attribute)
4. [C++23 Features](#4-c23-features)
5. [Compiler-spezifische Attribute](#5-compiler-spezifische-attribute)
6. [Schnellreferenz](#6-schnellreferenz)
7. [Verwendung in Code](#7-verwendung-in-code)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

**Attribute** sind standardisierte Compiler-Hinweise, die Code-Verhalten beeinflussen ohne die Semantik zu ändern. Sie helfen bei Warnungen, Optimierungen und Dokumentation.

### Syntax

```cpp
[[attribut]]
[[attribut(parameter)]]
[[namespace::attribut]]
[[attribut1, attribut2]]
```

### Platzierung

```cpp
[[nodiscard]] int getValue();              // Vor Deklaration
[[deprecated("Use newFunc")]] void old();  // Mit Parameter
[[maybe_unused]] int x = 42;               // Bei Variablen
```

---

## 2. Konventionen

### Notation

| Symbol | Bedeutung |
|--------|-----------|
| C++11/14/17/20/23 | Verfügbar ab dieser Version |
| ⚠️ | Erzeugt Compiler-Warnung |
| 🔧 | Optimierungspotential |

---

## 3. Standard-Attribute

### 3.1 `[[nodiscard]]` (C++17)

Warnt, wenn Rückgabewert ignoriert wird.

```cpp
[[nodiscard]] int computeResult();

computeResult();        // ⚠️ Warnung: Rückgabewert ignoriert
int x = computeResult(); // ✅ OK
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++17 |
| **Zweck** | Verhindert vergessene Fehlerprüfung |
| **Anwendung** | Fehler-Codes, Ressourcen, wichtige Ergebnisse |

**Mit Nachricht (C++20):**

```cpp
[[nodiscard("Must check for errors")]] bool validate();
```

---

### 3.2 `[[maybe_unused]]` (C++17)

Unterdrückt Warnungen bei unbenutzten Entities.

```cpp
[[maybe_unused]] int debugCounter = 0;  // Keine Warnung

void process([[maybe_unused]] int reserved) {
    // reserved nur für zukünftige Erweiterung
}
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++17 |
| **Zweck** | Sauberer Code ohne Warnung-Suppression |
| **Anwendung** | Debug-Variablen, reservierte Parameter |

---

### 3.3 `[[deprecated]]` (C++14)

Markiert Code als veraltet.

```cpp
[[deprecated]]
void oldFunction();

[[deprecated("Use newFunction() instead")]]
void legacyFunction();
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++14 |
| **Zweck** | API-Evolution ohne Breaking Changes |
| **Anwendung** | Veraltete Funktionen, Typen, Enums |

**Verwendung für Migration:**

```cpp
class [[deprecated("Migrate to NewWidget by v2.0")]] OldWidget {
    // ...
};
```

---

### 3.4 `[[fallthrough]]` (C++17)

Erlaubt explizites Durchfallen in `switch-case`.

```cpp
switch (value) {
    case 1:
        prepare();
        [[fallthrough]];  // Explizit: kein break gewünscht
    case 2:
        execute();
        break;
    case 3:
        // ⚠️ Warnung: implizites Fallthrough
    case 4:
        cleanup();
        break;
}
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++17 |
| **Zweck** | Dokumentiert bewusstes Fallthrough |
| **Anwendung** | Switch-Statements mit gemeinsamer Logik |

---

### 3.5 `[[noreturn]]` (C++11)

Funktion kehrt niemals zurück.

```cpp
[[noreturn]] void terminate() {
    std::abort();
}

[[noreturn]] void throwError(const char* msg) {
    throw std::runtime_error(msg);
}
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++11 |
| **Zweck** | Optimierung, Kontrollfluss-Analyse |
| **Anwendung** | Exit-Funktionen, Endlos-Loops, throw-Funktionen |

**Warnung:** UB wenn Funktion doch zurückkehrt!

---

### 3.6 `[[likely]]` / `[[unlikely]]` (C++20)

Branch-Prediction-Hints für Optimierung.

```cpp
if (condition) [[likely]] {
    // Häufiger Pfad — CPU-Branch-Predictor wird optimiert
    fastPath();
} else [[unlikely]] {
    // Seltener Pfad
    handleError();
}
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++20 |
| **Zweck** | 🔧 Performance-Optimierung |
| **Anwendung** | Hot Paths, Error Handling |

**In Switch:**

```cpp
switch (state) {
    case Running: [[likely]]
        processNormal();
        break;
    case Error: [[unlikely]]
        handleError();
        break;
}
```

---

### 3.7 `[[no_unique_address]]` (C++20)

Optimiert Speicher bei leeren Membern (EBO für Member).

```cpp
struct Tag {};  // Leere Klasse

struct WithoutAttribute {
    Tag tag;        // Belegt mindestens 1 Byte
    int data;       // Alignment-Padding möglich
};

struct WithAttribute {
    [[no_unique_address]] Tag tag;  // Kann 0 Bytes belegen
    int data;
};

// sizeof(WithoutAttribute) >= sizeof(int) + 1
// sizeof(WithAttribute) == sizeof(int)
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++20 |
| **Zweck** | 🔧 Speicheroptimierung |
| **Anwendung** | Stateless Allocators, Policy-Klassen, Tags |

---

### 3.8 `[[assume(expr)]]` (C++23)

Compiler darf annehmen, dass `expr` wahr ist.

```cpp
void process(int* ptr, size_t len) {
    [[assume(ptr != nullptr)]];
    [[assume(len > 0)]];
    
    // Compiler kann nullptr-Checks entfernen
    // und Loop-Unrolling aggressiver durchführen
}
```

| Aspekt | Wert |
|--------|------|
| **Seit** | C++23 |
| **Zweck** | 🔧 Aggressive Optimierung |
| **Anwendung** | Performance-kritischer Code, Invarianten |

**Warnung:** UB wenn Annahme falsch!

---

## 4. C++23 Features

### 4.1 Übersicht neuer Features

| Feature | Beschreibung |
|---------|--------------|
| `std::expected<T, E>` | Alternative zu Exceptions für Fehlerbehandlung |
| `std::print` / `std::println` | Direkter Print ohne `cout` |
| `std::format` Verbesserungen | Typsicheres String-Formatting |
| `views::chunk`, `slide`, `adjacent` | Neue Range-Views |
| `mdspan` | Multidimensionale Arrays |
| `[[assume(expr)]]` | Compiler-Optimierungshinweis |

### 4.2 std::expected

```cpp
#include <expected>

std::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

auto result = divide(10, 2);
if (result) {
    std::println("Result: {}", *result);
} else {
    std::println("Error: {}", result.error());
}
```

---

## 5. Compiler-spezifische Attribute

### 5.1 GCC/Clang

```cpp
[[gnu::always_inline]] void criticalPath();
[[gnu::pure]] int computePure(int x);  // Keine Seiteneffekte
[[gnu::const]] int computeConst(int x); // Noch strenger
[[gnu::hot]] void hotFunction();        // Häufig aufgerufen
[[gnu::cold]] void errorHandler();      // Selten aufgerufen
```

### 5.2 MSVC

```cpp
[[msvc::intrinsic]] void builtinFunc();
[[msvc::noinline]] void preventInline();
```

### 5.3 Portable Wrapper

```cpp
#if defined(__GNUC__) || defined(__clang__)
    #define ALWAYS_INLINE [[gnu::always_inline]] inline
    #define PURE [[gnu::pure]]
#elif defined(_MSC_VER)
    #define ALWAYS_INLINE __forceinline
    #define PURE
#else
    #define ALWAYS_INLINE inline
    #define PURE
#endif
```

---

## 6. Schnellreferenz

### 6.1 Nach C++-Version

| Attribut | Seit | Zweck |
|----------|------|-------|
| `[[noreturn]]` | C++11 | Funktion kehrt nie zurück |
| `[[deprecated]]` | C++14 | Markiert als veraltet |
| `[[deprecated("msg")]]` | C++14 | Mit Begründung |
| `[[fallthrough]]` | C++17 | Erlaubtes Switch-Fallthrough |
| `[[nodiscard]]` | C++17 | Rückgabewert prüfen |
| `[[maybe_unused]]` | C++17 | Unbenutzt erlaubt |
| `[[nodiscard("msg")]]` | C++20 | Mit Begründung |
| `[[likely]]` | C++20 | Wahrscheinlicher Branch |
| `[[unlikely]]` | C++20 | Unwahrscheinlicher Branch |
| `[[no_unique_address]]` | C++20 | Speicheroptimierung |
| `[[assume(expr)]]` | C++23 | Compiler-Annahme |

### 6.2 Nach Anwendungsfall

| Anwendung | Empfohlenes Attribut |
|-----------|---------------------|
| Fehler-Rückgabe | `[[nodiscard]]` |
| API-Deprecation | `[[deprecated("...")]]` |
| Debug-Variablen | `[[maybe_unused]]` |
| Error-Handler | `[[unlikely]]`, `[[noreturn]]` |
| Hot-Path | `[[likely]]` |
| Stateless Member | `[[no_unique_address]]` |

---

## 7. Verwendung in Code

### 7.1 Typisches API-Design

```cpp
class [[nodiscard]] Result {
    int m_value;
    bool m_success;
public:
    [[nodiscard]] bool isSuccess() const;
    [[nodiscard]] int getValue() const;
};

[[nodiscard]] Result performOperation();

// Nutzer muss Ergebnis prüfen
auto result = performOperation();  // ✅ OK
performOperation();                // ⚠️ Warnung
```

### 7.2 Performance-kritischer Code

```cpp
void processPackets(Packet* packets, size_t count) {
    [[assume(packets != nullptr)]];
    [[assume(count > 0)]];
    
    for (size_t i = 0; i < count; ++i) {
        if (packets[i].isValid()) [[likely]] {
            handleNormal(packets[i]);
        } else [[unlikely]] {
            handleError(packets[i]);
        }
    }
}
```

---

## 8. Siehe auch

- [Cpp_Coding_Standard.md](../../standards/Cpp_Coding_Standard.md) — Coding-Konventionen

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus C++ Attribute für Kompiler.md** |
