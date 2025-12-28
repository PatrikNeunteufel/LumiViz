# Git Externals — Scripting

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Git_Externals_Scripting.md](../../en/references/externals/Git_Externals_Scripting.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [sol2](#2-sol2)
3. [pybind11](#3-pybind11)
4. [chaiscript](#4-chaiscript)
5. [Schnellreferenz](#5-schnellreferenz)
6. [See Also](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Overview

This document describes Git Scripting-Externals für das CMake Architecture Build-System.

| Library | Description | CMake | Hook |
|---------|--------------|-------|------|
| **sol2** | C++ Lua Bindings | ✅ | — |
| **pybind11** | C++ Python Bindings | ✅ | — |
| **chaiscript** | Embedded Scripting Language | ✅ | — |

---

## 2. sol2

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Repository** | https://github.com/ThePhD/sol2 |
| **Empfohlener Tag** | v3.3.1 |
| **CMake-Support** | ✅ |
| **Hook** | — (nicht nötig) |
| **Typ** | Header-Only |

### Solution.json

```json
{
    "externals": {
        "lua54": { "path": "externals/lua54" },
        "sol2": {
            "git": "https://github.com/ThePhD/sol2.git",
            "tag": "v3.3.1"
        }
    }
}
```

> **Important:** sol2 benötigt eine Lua-Implementation (lua54 als Local External empfohlen)

### Usagesbeispiel

```cpp
#include <sol/sol.hpp>

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    
    // Variablen
    lua["x"] = 10;
    int x = lua["x"];
    
    // Functions
    lua["add"] = [](int a, int b) { return a + b; };
    int result = lua["add"](2, 3);  // 5
    
    // Skript ausführen
    lua.script("print('Hello from Lua!')");
    
    return 0;
}
```

### Klassen exportieren

```cpp
#include <sol/sol.hpp>

class Player {
public:
    std::string name;
    int health = 100;
    
    void damage(int amount) { health -= amount; }
    bool isAlive() const { return health > 0; }
};

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    
    // Klasse registrieren
    lua.new_usertype<Player>("Player",
        "name", &Player::name,
        "health", &Player::health,
        "damage", &Player::damage,
        "isAlive", &Player::isAlive
    );
    
    // In Lua verwenden
    lua.script(R"(
        local p = Player.new()
        p.name = "Hero"
        p:damage(30)
        print(p.name .. " has " .. p.health .. " HP")
    )");
    
    return 0;
}
```

### Targets

| Target | Description |
|--------|--------------|
| `sol2::sol2` | Header-Only Interface |

---

## 3. pybind11

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Repository** | https://github.com/pybind/pybind11 |
| **Empfohlener Tag** | v2.12.0 |
| **CMake-Support** | ✅ |
| **Hook** | — (nicht nötig) |
| **Typ** | Header-Only |

### Solution.json

```json
{
    "externals": {
        "pybind11": {
            "git": "https://github.com/pybind/pybind11.git",
            "tag": "v2.12.0"
        }
    }
}
```

> **Important:** Python Development Headers müssen installiert sein

### Usagesbeispiel (Python Module erstellen)

```cpp
#include <pybind11/pybind11.h>

int add(int a, int b) {
    return a + b;
}

PYBIND11_MODULE(example, m) {
    m.doc() = "Example module";
    m.def("add", &add, "Add two numbers");
}
```

### Klassen exportieren

```cpp
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Calculator {
public:
    int add(int a, int b) { return a + b; }
    int multiply(int a, int b) { return a * b; }
};

PYBIND11_MODULE(calculator, m) {
    py::class_<Calculator>(m, "Calculator")
        .def(py::init<>())
        .def("add", &Calculator::add)
        .def("multiply", &Calculator::multiply);
}
```

**Python:**

```python
import calculator
calc = calculator.Calculator()
print(calc.add(2, 3))       # 5
print(calc.multiply(4, 5))  # 20
```

### Targets

| Target | Description |
|--------|--------------|
| `pybind11::module` | Für Python Module |
| `pybind11::embed` | Für Embedded Python |

---

## 4. chaiscript

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Repository** | https://github.com/ChaiScript/ChaiScript |
| **Empfohlener Tag** | v6.1.0 |
| **CMake-Support** | ✅ |
| **Hook** | — (nicht nötig) |
| **Typ** | Header-Only |

### Solution.json

```json
{
    "externals": {
        "chaiscript": {
            "git": "https://github.com/ChaiScript/ChaiScript.git",
            "tag": "v6.1.0"
        }
    }
}
```

### Usagesbeispiel

```cpp
#include <chaiscript/chaiscript.hpp>

int main() {
    chaiscript::ChaiScript chai;
    
    // Variablen
    chai.add(chaiscript::var(42), "x");
    
    // Functions
    chai.add(chaiscript::fun([](int a, int b) { return a + b; }), "add");
    
    // Skript ausführen
    chai.eval("print(add(2, 3))");
    
    // Wert abrufen
    int result = chai.eval<int>("add(10, 20)");
    
    return 0;
}
```

### Klassen exportieren

```cpp
#include <chaiscript/chaiscript.hpp>

class Player {
public:
    std::string name;
    int health = 100;
    
    void damage(int amount) { health -= amount; }
};

int main() {
    chaiscript::ChaiScript chai;
    
    // Klasse registrieren
    chai.add(chaiscript::user_type<Player>(), "Player");
    chai.add(chaiscript::constructor<Player()>(), "Player");
    chai.add(chaiscript::fun(&Player::name), "name");
    chai.add(chaiscript::fun(&Player::health), "health");
    chai.add(chaiscript::fun(&Player::damage), "damage");
    
    // In ChaiScript verwenden
    chai.eval(R"(
        var p = Player()
        p.name = "Hero"
        p.damage(30)
        print("${p.name} has ${p.health} HP")
    )");
    
    return 0;
}
```

### Targets

| Target | Description |
|--------|--------------|
| `chaiscript` | Main Library |

---

## 5. Schnellreferenz

| Library | Tag | CMake | Hook | Typ | Sprache |
|---------|-----|-------|------|-----|---------|
| sol2 | v3.3.1 | ✅ | — | Header-Only | Lua |
| pybind11 | v2.12.0 | ✅ | — | Header-Only | Python |
| chaiscript | v6.1.0 | ✅ | — | Header-Only | ChaiScript |

### Vergleich

| Feature | sol2 | pybind11 | chaiscript |
|---------|------|----------|------------|
| **Zielsprache** | Lua | Python | ChaiScript |
| **Runtime** | lua54 (Local) | Python Interpreter | Eingebaut |
| **Syntax** | Lua | Python | C++-ähnlich |
| **Footprint** | Klein | Mittel | Mittel |
| **Anwendung** | Game Scripting | Python Integration | Embedded DSL |

### Vergleich mit Local-Alternative

| Feature | lua54 (Local) | sol2 (Git) |
|---------|---------------|------------|
| **Typ** | Core Engine | C++ Bindings |
| **API** | C-Style | Modern C++ |
| **Kombination** | ✅ Zusammen verwenden | ✅ Zusammen verwenden |

---

## 6. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Git_Externals.md](Git_Externals.md) — Git Externals Overview
- [Local_Externals_Scripting.md](Local_Externals_Scripting.md) — Local Scripting (lua54)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Eigene Kategorie Scripting (aus Git_Externals_Core ausgelagert)** |
