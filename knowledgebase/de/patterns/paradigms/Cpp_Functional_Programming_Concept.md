# Functional Programming in C++ — Konzept

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Concept  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Cpp_Functional_Programming_Concept.md](../../../en/patterns/paradigms/Cpp_Functional_Programming_Concept.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Kernkonzepte](#2-kernkonzepte)
3. [Lambdas](#3-lambdas)
4. [Higher-Order Functions](#4-higher-order-functions)
5. [Currying und Partial Application](#5-currying-und-partial-application)
6. [Optionale Werte](#6-optionale-werte)
7. [Pipelines](#7-pipelines)
8. [Vergleich mit anderen Sprachen](#8-vergleich-mit-anderen-sprachen)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

**Functional Programming (FP)** basiert darauf, Berechnungen als reine Funktionen zu modellieren. C++ unterstützt FP-Konzepte durch Lambdas, `std::function`, `std::bind`, und moderne Features wie `std::optional` und `std::expected`.

### Kernprinzipien

| Prinzip | Beschreibung |
|---------|--------------|
| **Pure Functions** | Keine Seiteneffekte, gleiche Eingabe → gleiche Ausgabe |
| **Immutability** | Daten werden nicht verändert, sondern kopiert |
| **First-Class Functions** | Funktionen als Werte (Parameter, Rückgabe) |
| **Higher-Order Functions** | Funktionen die Funktionen verarbeiten |

---

## 2. Kernkonzepte

### 2.1 Pure Functions

```cpp
// ✅ Pure — keine Seiteneffekte
int add(int a, int b) {
    return a + b;
}

// ❌ Nicht pure — modifiziert globalen Zustand
int counter = 0;
int increment() {
    return ++counter;
}
```

### 2.2 Immutability

```cpp
// ❌ Mutierend
void addToVector(std::vector<int>& v, int x) {
    v.push_back(x);
}

// ✅ Immutable
std::vector<int> withElement(std::vector<int> v, int x) {
    v.push_back(x);
    return v;  // Neue Kopie
}
```

---

## 3. Lambdas

### 3.1 Syntax

```cpp
// Minimal
auto add = [](int a, int b) { return a + b; };

// Mit explizitem Rückgabetyp
auto divide = [](double a, double b) -> double {
    return a / b;
};

// Mit Capture
int factor = 10;
auto multiply = [factor](int x) { return x * factor; };

// Mutable Capture
auto counter = [count = 0]() mutable { return ++count; };
```

### 3.2 Capture-Modi

| Capture | Bedeutung |
|---------|-----------|
| `[]` | Keine Captures |
| `[=]` | Alle by-value |
| `[&]` | Alle by-reference |
| `[x]` | `x` by-value |
| `[&x]` | `x` by-reference |
| `[=, &x]` | Alle by-value, `x` by-reference |
| `[this]` | `this`-Pointer |
| `[*this]` | Kopie von `*this` (C++17) |

---

## 4. Higher-Order Functions

### 4.1 Standard-Library Algorithmen

```cpp
#include <algorithm>
#include <vector>

std::vector<int> numbers = {1, 2, 3, 4, 5};

// Transform (map)
std::vector<int> doubled;
std::transform(numbers.begin(), numbers.end(),
               std::back_inserter(doubled),
               [](int x) { return x * 2; });

// Filter
std::vector<int> evens;
std::copy_if(numbers.begin(), numbers.end(),
             std::back_inserter(evens),
             [](int x) { return x % 2 == 0; });

// Reduce (fold)
int sum = std::accumulate(numbers.begin(), numbers.end(), 0,
                          [](int acc, int x) { return acc + x; });
```

### 4.2 Ranges (C++20)

```cpp
#include <ranges>

auto result = numbers
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::views::transform([](int x) { return x * 2; });
    
for (int x : result) {
    // Lazy evaluation
}
```

---

## 5. Currying und Partial Application

### 5.1 Mit Lambdas

```cpp
// Currying: Eine Funktion, die eine Funktion zurückgibt
auto add = [](int a) {
    return [a](int b) {
        return a + b;
    };
};

auto add5 = add(5);
int result = add5(3);  // 8
```

### 5.2 Mit std::bind

```cpp
#include <functional>

int multiply(int a, int b, int c) {
    return a * b * c;
}

// Partial Application
using namespace std::placeholders;
auto multiplyBy2 = std::bind(multiply, 2, _1, _2);
int result = multiplyBy2(3, 4);  // 24
```

### 5.3 Vergleich zu Haskell/JavaScript

```haskell
-- Haskell (automatisch curried)
add :: Int -> Int -> Int
add a b = a + b
add5 = add 5
```

```javascript
// JavaScript
const add = a => b => a + b;
const add5 = add(5);
```

```cpp
// C++ (manuell)
auto add = [](int a) {
    return [a](int b) { return a + b; };
};
```

---

## 6. Optionale Werte

### 6.1 std::optional (C++17)

```cpp
#include <optional>

std::optional<int> findValue(const std::vector<int>& v, int target) {
    auto it = std::find(v.begin(), v.end(), target);
    if (it != v.end()) {
        return *it;
    }
    return std::nullopt;
}

// Verwendung
auto result = findValue(numbers, 3);
if (result) {
    std::cout << *result << '\n';
}

// Mit value_or
int value = result.value_or(-1);
```

### 6.2 std::expected (C++23)

```cpp
#include <expected>

std::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

// Verwendung
auto result = divide(10, 2);
if (result) {
    std::cout << *result << '\n';
} else {
    std::cout << "Error: " << result.error() << '\n';
}
```

---

## 7. Pipelines

### 7.1 Einfache Pipeline

```cpp
template<typename T, typename F>
auto operator|(T&& value, F&& func) {
    return func(std::forward<T>(value));
}

auto result = 5
    | [](int x) { return x * 2; }
    | [](int x) { return x + 3; }
    | [](int x) { return std::to_string(x); };
// result = "13"
```

### 7.2 Flexible Typed Pipeline

```cpp
#include <any>
#include <optional>

class FlexibleValue {
    std::any m_value;
    
public:
    template<typename T>
    void set(const T& value) {
        m_value = value;
    }
    
    template<typename T>
    std::optional<T> get() const {
        if (m_value.type() == typeid(T)) {
            return std::any_cast<T>(m_value);
        }
        return std::nullopt;
    }
};
```

---

## 8. Vergleich mit anderen Sprachen

| Konzept | C++ | Haskell | JavaScript |
|---------|-----|---------|------------|
| **Lambda** | `[](x){ }` | `\x -> ` | `x => ` |
| **Currying** | Manuell | Automatisch | Manuell |
| **Optional** | `std::optional` | `Maybe` | `null`/`undefined` |
| **Result** | `std::expected` | `Either` | Promise |
| **Map** | `std::transform` | `map` | `array.map` |
| **Filter** | `std::copy_if` | `filter` | `array.filter` |
| **Reduce** | `std::accumulate` | `foldl` | `array.reduce` |
| **Lazy** | `std::views` (C++20) | Default | Manuell |

---

## 9. Siehe auch

- [Cpp_Attributes_Reference.md](../../languages/cpp/Cpp_Attributes_Reference.md) — C++ Attribute
- [Rule_of_Five_Concept.md](../design/Rule_of_Five_Concept.md) — Wertesemantik

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus cpp functional programming.md, functional.md, Flexible Typisierte Funktions-Pipeline.md** |
