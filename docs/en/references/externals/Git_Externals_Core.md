# Git Externals — Core & Utility

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Git_Externals_Core.md](../../en/references/externals/Git_Externals_Core.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Logging & Debugging](#2-logging--debugging)
3. [Utility / General](#3-utility--general)
4. [Schnellreferenz](#4-schnellreferenz)
5. [See Also](#5-siehe-auch)
6. [Changelog](#6-changelog)

---

## 1. Overview

This document describes grundlegende Bibliotheken für Logging und allgemeine Entwicklungsaufgaben.

> **Note:** Testing und Scripting sind in separate Kategorien ausgelagert:
> - [Git_Externals_Testing.md](Git_Externals_Testing.md) — googletest, catch2, benchmark
> - [Git_Externals_Scripting.md](Git_Externals_Scripting.md) — sol2, pybind11, chaiscript

### Kategorien

| Kategorie | Bibliotheken |
|-----------|--------------|
| Logging | spdlog, fmt |
| Utility | abseil, magic_enum, argparse, CLI11 |

---

## 2. Logging & Debugging

### 2.1 spdlog

> **Zweck:** Schnelle, header-only/compiled C++ Logging-Bibliothek mit Formatierung, Rotation und Sinks.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/gabime/spdlog.git` |
| **Aktueller Tag** | `v1.14.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Dependencies** | fmt (optional, bundled) |

```json
"spdlog": {
    "git": "https://github.com/gabime/spdlog.git",
    "tag": "v1.14.1"
}
```

**PreFetch Hook (spdlog.cmake):**
```cmake
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
# Optional: Externe fmt verwenden
# set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <spdlog/spdlog.h>

spdlog::info("Welcome to spdlog!");
spdlog::error("Error message: {}", error_code);
spdlog::warn("Warning at line {}", __LINE__);
```

**Mit verschiedenen Sinks:**
```cpp
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

// File Sink
auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/basic.txt");

// Rotating File Sink (5 MB, 3 Files)
auto rotating = spdlog::rotating_logger_mt("rotating", "logs/app.log", 5*1024*1024, 3);
```

---

### 2.2 fmt

> **Zweck:** Moderne Formatierungs-Bibliothek (Basis für C++20 std::format).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/fmtlib/fmt.git` |
| **Aktueller Tag** | `10.2.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"fmt": {
    "git": "https://github.com/fmtlib/fmt.git",
    "tag": "10.2.1"
}
```

**PreFetch Hook (fmt.cmake):**
```cmake
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>

std::string s = fmt::format("Hello, {}!", "world");
fmt::print("Time: {:%H:%M}\n", std::chrono::system_clock::now());

std::vector<int> v = {1, 2, 3};
fmt::print("Vector: {}\n", v);  // [1, 2, 3]
```

---

## 3. Utility / General

### 3.1 abseil-cpp

> **Zweck:** Google's C++ Common Libraries (Strings, Containers, Synchronization).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/abseil/abseil-cpp.git` |
| **Aktueller Tag** | `20240116.2` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch erforderlich |

```json
"abseil": {
    "git": "https://github.com/abseil/abseil-cpp.git",
    "tag": "20240116.2"
}
```

**PreFetch Hook (abseil.cmake):**
```cmake
set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ABSL_USE_GOOGLETEST_HEAD OFF CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

std::string result = absl::StrCat("Hello", " ", "World");
std::vector<std::string> parts = absl::StrSplit("a,b,c", ',');
```

---

### 3.2 magic_enum

> **Zweck:** Statische Enum-Reflection ohne Makros.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/Neargye/magic_enum.git` |
| **Aktueller Tag** | `v0.9.5` |
| **CMake Support** | ✅ |
| **Hook** | — |
| **Header-only** | ✅ |

```json
"magic_enum": {
    "git": "https://github.com/Neargye/magic_enum.git",
    "tag": "v0.9.5"
}
```

**Usage:**
```cpp
#include <magic_enum.hpp>

enum class Color { RED, GREEN, BLUE };

auto name = magic_enum::enum_name(Color::RED);  // "RED"
auto value = magic_enum::enum_cast<Color>("GREEN");  // Color::GREEN

// Iteration
for (auto color : magic_enum::enum_values<Color>()) {
    fmt::print("{}\n", magic_enum::enum_name(color));
}
```

---

### 3.3 argparse

> **Zweck:** Einfacher Argument-Parser im Python-Stil.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/p-ranav/argparse.git` |
| **Aktueller Tag** | `v3.0` |
| **CMake Support** | ✅ |
| **Hook** | — |
| **Header-only** | ✅ |

```json
"argparse": {
    "git": "https://github.com/p-ranav/argparse.git",
    "tag": "v3.0"
}
```

**Usage:**
```cpp
#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("myapp");
    
    program.add_argument("--verbose")
        .flag()
        .help("Enable verbose output");
        
    program.add_argument("-n", "--number")
        .default_value(1)
        .scan<'i', int>();
    
    program.parse_args(argc, argv);
    
    if (program["--verbose"] == true) {
        // verbose mode
    }
    
    int n = program.get<int>("--number");
}
```

---

### 3.4 CLI11

> **Zweck:** Feature-reicher Command-Line-Parser.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/CLIUtils/CLI11.git` |
| **Aktueller Tag** | `v2.4.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |

```json
"cli11": {
    "git": "https://github.com/CLIUtils/CLI11.git",
    "tag": "v2.4.1"
}
```

**PreFetch Hook (cli11.cmake):**
```cmake
set(CLI11_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <CLI/CLI.hpp>

int main(int argc, char* argv[]) {
    CLI::App app{"My App"};
    
    std::string filename;
    app.add_option("-f,--file", filename, "Input file")->required();
    
    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "Verbose output");
    
    CLI11_PARSE(app, argc, argv);
    
    // Use filename, verbose...
}
```

---

## 4. Schnellreferenz

| Bibliothek | Tag | CMake | Hook | Hauptverwendung |
|------------|-----|-------|------|-----------------|
| spdlog | v1.14.1 | ✅ | PreFetch | Fast logging |
| fmt | 10.2.1 | ✅ | PreFetch | String formatting |
| abseil | 20240116.2 | ✅ | PreFetch | Google's C++ libs |
| magic_enum | v0.9.5 | ✅ | — | Enum reflection |
| argparse | v3.0 | ✅ | — | Simple arg parsing |
| CLI11 | v2.4.1 | ✅ | PreFetch | Full-featured CLI |

---

## 5. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Git_Externals.md](Git_Externals.md) — Git Externals Overview
- [Git_Externals_Testing.md](Git_Externals_Testing.md) — Testing (googletest, catch2)
- [Git_Externals_Scripting.md](Git_Externals_Scripting.md) — Scripting (sol2, pybind11)
- [Git_Externals_GUI.md](Git_Externals_GUI.md) — GUI & Graphics
- [Git_Externals_Data.md](Git_Externals_Data.md) — Data & Serialization

---

## 6. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Testing → Git_Externals_Testing.md ausgelagert; Fokus auf Logging & Utility** |
| 0.5.0 | 2025-12-14 | Initial: Ausgelagert aus Git_Externals_Reference |
