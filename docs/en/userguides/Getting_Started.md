# Erste Schritte – Benutzerhandbuch

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** C++ Developers (Einsteiger und Fortgeschrittene)  
> **Language:** English  
> **German:** [Getting_Started_UserGuide.md](../../en/userguides/Getting_Started.md)

---

## Table of Contents

1. [Überblick](#1-überblick)
2. [Prerequisites](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Projektstruktur verstehen](#4-projektstruktur-verstehen)
5. [Solution.json – Das Herzstück](#5-solutionjson--das-herzstück)
6. [Erstes Executable erstellen](#6-erstes-executable-erstellen)
7. [Erste Library erstellen](#7-erste-library-erstellen)
8. [Build-Befehle](#8-build-befehle)
9. [Externals verwenden](#9-externals-verwenden)
10. [Tests hinzufügen](#10-tests-hinzufügen)
11. [Workflow-Summary](#11-workflow-zusammenfassung)
12. [Stolpersteine und Lösungen](#12-stolpersteine-und-lösungen)
13. [Troubleshooting](#13-troubleshooting)
14. [Nächste Schritte](#14-nächste-schritte)
15. [See Also](#15-siehe-auch)

---

## 1. Überblick

Willkommen zum **CMake Architecture** Build-System! Dieses Guide führt dich durch alle Schritte, um dein erstes Projekt erfolgreich zu erstellen und zu bauen.

### Was ist CMake Architecture?

Ein **JSON-gesteuertes Build-System** für C++ Projekte, das:

- **Deklarativ** arbeitet – beschreibe WAS, nicht WIE
- **Multi-Projekt** unterstützt – mehrere Executables und Libraries in einer Solution
- **Externe Bibliotheken** automatisch verwaltet
- **Plattformübergreifend** funktioniert (Windows, Linux, macOS)
- **IDE-Integration** bietet (Visual Studio, CLion, VS Code)

### Das wichtigste Prinzip

> **Wenn es nicht in der Solution.json steht, existiert es nicht.**

Alles – Executables, Libraries, Tests, Externals – wird in `Solution.json` definiert. Das Build-System generiert daraus automatisch die CMake-Configuration.

### Was du am Ende dieses Guides haben wirst

- Ein funktionierendes Projekt mit Executable und Library
- Verständnis der Projektstruktur
- Wissen, wie du neue Module hinzufügst
- Grundlagen für Tests und Externals

---

## 2. Prerequisites

### Checkliste

- [ ] **CMake 3.24+** installiert
- [ ] **C++ Compiler:** MSVC 2022, GCC 11+, oder Clang 14+
- [ ] **Git** installiert
- [ ] **IDE** (empfohlen): Visual Studio 2022, CLion, oder VS Code mit CMake-Extension
- [ ] **Ninja** (empfohlen für schnellere Builds)

### Installation prüfen

```bash
# CMake Version
cmake --version
# Sollte 3.24 oder höher sein

# Compiler
cl /?          # Windows (MSVC)
g++ --version  # Linux (GCC)
clang++ --version  # macOS/Linux (Clang)

# Git
git --version

# Ninja (optional aber empfohlen)
ninja --version
```

### Empfohlene Versionen

| Tool | Minimum | Empfohlen |
|------|---------|-----------|
| CMake | 3.24 | 3.28+ |
| MSVC | 2019 | 2022 |
| GCC | 11 | 13+ |
| Clang | 14 | 17+ |
| Ninja | 1.10 | 1.11+ |

---

## 3. Schnellstart

### 3.1 Projekt klonen/erstellen

```bash
# Neues Projekt aus Template
git clone https://github.com/your-org/cmake-v2-template.git MyProject
cd MyProject

# Oder bestehendes Projekt
cd /path/to/existing/project
```

### 3.2 Solution.json erstellen

Erstelle `Solution.json` im Projekt-Root:

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "MyProject",
        "version": "1.0.0"
    },
    "settings": {
        "standards": {
            "cxx_standard": 20
        }
    },
    "executables": [
        {
            "name": "HelloWorld",
            "type": "CONSOLE"
        }
    ]
}
```

### 3.3 Source-Datei erstellen

Erstelle `projects/demos/exec/HelloWorld/src/main.cpp`:

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, CMake Architecture!" << std::endl;
    return 0;
}
```

### 3.4 Bauen und Ausführen

```bash
# Konfigurieren
cmake --preset windows-ninja-debug    # Windows
cmake --preset linux-ninja-debug      # Linux
cmake --preset macos-ninja-debug      # macOS

# Bauen
cmake --build out/build/windows-ninja-debug

# Ausführen
./out/build/windows-ninja-debug/bin/Debug/HelloWorld.exe    # Windows
./out/build/linux-ninja-debug/bin/Debug/HelloWorld          # Linux
```

**Geschafft!** Du hast dein erstes Projekt gebaut. 🎉

---

## 4. Projektstruktur verstehen

### 4.1 Standard-Verzeichnisstruktur

```
MyProject/
├── CMakeLists.txt              ← Haupt-CMake-Datei (generiert vom System)
├── CMakePresets.json           ← Team-weite Build-Presets
├── CMakeUserPresets.json       ← Persönliche Presets (nicht committen!)
├── Solution.json               ← ZENTRALE KONFIGURATION
│
├── cmake/                      ← Build-System Module
│   ├── core/                   ← Kern-Module
│   ├── externals/              ← External-Verwaltung
│   └── project/                ← Projekt-Erstellung
│
├── externals/                  ← Lokale externe Bibliotheken
│   ├── bass/
│   ├── doctest/
│   └── lua54/
│
├── projects/                   ← DEIN CODE
│   ├── demos/                  ← Demo-Projekte
│   │   ├── exec/               ← Executables
│   │   │   └── HelloWorld/
│   │   │       └── src/
│   │   │           └── main.cpp
│   │   └── libs/               ← Libraries
│   │       └── CoreLib/
│   │           ├── include/
│   │           │   └── CoreLib/
│   │           │       └── Calculator.h
│   │           └── src/
│   │               └── Calculator.cpp
│   └── tests/                  ← Test-Projekte
│       └── unit/
│           └── CoreLib_Tests/
│
└── out/                        ← Build-Ausgabe (generiert)
    └── build/
        └── windows-ninja-debug/
            └── bin/
                └── Debug/
                    └── HelloWorld.exe
```

### 4.2 Importante Dateien

| Datei | Zweck | Bearbeiten? |
|-------|-------|-------------|
| `Solution.json` | Projekt-Definition | ✅ JA |
| `CMakeLists.txt` | CMake-Einstiegspunkt | ❌ No (generiert) |
| `CMakePresets.json` | Team-Presets | ⚠️ Selten |
| `CMakeUserPresets.json` | Persönliche Presets | ✅ JA |

### 4.3 Pfad-Konventionen

Das Build-System erwartet standardmäßig diese Pfade:

| Typ | Pfad-Muster |
|-----|-------------|
| Executable | `projects/demos/exec/{name}/src/` |
| Library | `projects/demos/libs/{name}/` |
| Test | `projects/tests/{type}/{name}/src/` |

Du kannst diese mit dem `path` Feld überschreiben.

---

## 5. Solution.json – Das Herzstück

### 5.1 Grundstruktur

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "MySolution",
        "version": "1.0.0"
    },
    "settings": { ... },
    "externals": { ... },
    "libraries": [ ... ],
    "executables": [ ... ],
    "tests": [ ... ]
}
```

### 5.2 Abschnitte erklärt

| Abschnitt | Zweck | Required? |
|-----------|-------|----------|
| `schemaVersion` | Schema-Version | ✅ Yes |
| `solution` | Projekt-Metadaten | ✅ Yes |
| `settings` | Globale Einstellungen | ❌ Optional |
| `externals` | Externe Bibliotheken | ❌ Optional |
| `libraries` | Interne Libraries | ❌ Optional |
| `executables` | Ausführbare Programme | ❌ Optional |
| `tests` | Test-Executables | ❌ Optional |

### 5.3 Settings-Abschnitt

```json
"settings": {
    "standards": {
        "cxx_standard": 20,
        "c_standard": 17
    },
    "output": {
        "binDir": "bin",
        "libDir": "lib"
    }
}
```

### 5.4 Reihenfolge der Verarbeitung

```
1. settings      → Globale Einstellungen
2. externals     → Externe Bibliotheken laden
3. libraries     → Interne Libraries erstellen
4. executables   → Executables erstellen (können Libraries nutzen)
5. tests         → Tests erstellen (können Libraries nutzen)
```

**Important:** Libraries müssen VOR Executables definiert werden, wenn diese voneinander abhängen.

---

## 6. Erstes Executable erstellen

### 6.1 Workflow

```
1. Solution.json Eintrag erstellen  ← IMMER ZUERST!
2. CMake konfigurieren
3. Dateien anlegen
4. Bauen und testen
```

### 6.2 Solution.json Eintrag

```json
{
    "executables": [
        {
            "name": "MyApp",
            "displayName": "My Application",
            "version": "1.0.0",
            "type": "CONSOLE"
        }
    ]
}
```

### 6.3 Executable-Typen

| Typ | Description | Windows-Subsystem |
|-----|--------------|-------------------|
| `CONSOLE` | Konsolen-Anwendung | CONSOLE |
| `GUI` | Grafische Anwendung (kein Konsolenfenster) | WINDOWS |

### 6.4 Vollständige Configuration

```json
{
    "name": "MyApp",
    "displayName": "My Application",
    "version": "1.0.0",
    "type": "CONSOLE",
    "path": "projects/demos/exec/MyApp/src",
    "dependencies": ["CoreLib", "UtilLib"],
    "externals": ["spdlog", "bass"],
    "defines": ["APP_VERSION=\"1.0.0\""],
    "compile_options": ["-Wall"]
}
```

### 6.5 Felder-Reference

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `name` | string | **Required** | Target-Name |
| `displayName` | string | `name` | Anzeigename |
| `version` | string | `"0.1.0"` | Semantic Version |
| `type` | string | `"CONSOLE"` | CONSOLE oder GUI |
| `path` | string | Convention | Source-Verzeichnis |
| `dependencies` | string[] | `[]` | Interne Libraries |
| `externals` | string[] | `[]` | Externe Libraries |

### 6.6 Source-Dateien

Erstelle die Source-Dateien im angegebenen Path:

**main.cpp:**
```cpp
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Hello from MyApp!" << std::endl;
    return 0;
}
```

Das Build-System findet automatisch alle `.cpp`, `.c`, `.hpp`, `.h` Dateien im Source-Verzeichnis.

---

## 7. Erste Library erstellen

### 7.1 Solution.json Eintrag

```json
{
    "libraries": [
        {
            "name": "CoreLib",
            "version": "1.0.0",
            "type": "STATIC"
        }
    ]
}
```

### 7.2 Library-Typen

| Typ | Description | Wann verwenden? |
|-----|--------------|-----------------|
| `STATIC` | Statisch gelinkt (.lib/.a) | Standard, einfachste Option |
| `SHARED` | Dynamisch gelinkt (.dll/.so) | Wenn mehrere Programme teilen |
| `INTERFACE` | Header-only | Keine .cpp Dateien |

### 7.3 Vollständige Configuration

```json
{
    "name": "CoreLib",
    "displayName": "Core Library",
    "version": "1.0.0",
    "type": "STATIC",
    "path": "projects/demos/libs/CoreLib",
    "public_headers": "projects/demos/libs/CoreLib/include",
    "dependencies": [],
    "externals": []
}
```

### 7.4 Verzeichnisstruktur

```
projects/demos/libs/CoreLib/
├── include/
│   └── CoreLib/
│       └── Calculator.h
└── src/
    └── Calculator.cpp
```

### 7.5 Header-Datei

**include/CoreLib/Calculator.h:**
```cpp
#pragma once

namespace CoreLib {

class Calculator {
public:
    int add(int a, int b) const;
    int subtract(int a, int b) const;
    int multiply(int a, int b) const;
    double divide(int a, int b) const;
};

} // namespace CoreLib
```

### 7.6 Implementation

**src/Calculator.cpp:**
```cpp
#include "CoreLib/Calculator.h"
#include <stdexcept>

namespace CoreLib {

int Calculator::add(int a, int b) const {
    return a + b;
}

int Calculator::subtract(int a, int b) const {
    return a - b;
}

int Calculator::multiply(int a, int b) const {
    return a * b;
}

double Calculator::divide(int a, int b) const {
    if (b == 0) {
        throw std::domain_error("Division by zero");
    }
    return static_cast<double>(a) / b;
}

} // namespace CoreLib
```

### 7.7 Library in Executable verwenden

```json
{
    "executables": [
        {
            "name": "Calculator",
            "dependencies": ["CoreLib"]
        }
    ]
}
```

**main.cpp:**
```cpp
#include <iostream>
#include "CoreLib/Calculator.h"

int main() {
    CoreLib::Calculator calc;
    std::cout << "2 + 3 = " << calc.add(2, 3) << std::endl;
    return 0;
}
```

---

## 8. Build-Befehle

### 8.1 CMake Presets verwenden

```bash
# Verfügbare Presets anzeigen
cmake --list-presets

# Konfigurieren
cmake --preset windows-ninja-debug

# Bauen
cmake --build out/build/windows-ninja-debug

# Oder mit Preset
cmake --build --preset build-windows-ninja-debug
```

### 8.2 Häufige Presets

| Preset | Plattform | Compiler | Build-Type |
|--------|-----------|----------|------------|
| `windows-ninja-debug` | Windows | Auto | Debug |
| `windows-ninja-release` | Windows | Auto | Release |
| `windows-ninja-debug-clang` | Windows | Clang | Debug |
| `windows-ninja-debug-msvc` | Windows | MSVC | Debug |
| `linux-ninja-debug` | Linux | GCC | Debug |
| `macos-ninja-debug` | macOS | Clang | Debug |

### 8.3 Einzelnes Target bauen

```bash
# Nur bestimmtes Target
cmake --build out/build/windows-ninja-debug --target MyApp

# Nur Tests
cmake --build out/build/windows-ninja-debug --target all_tests
```

### 8.4 Clean Build

```bash
# Cache löschen und neu konfigurieren
cmake --preset windows-ninja-debug --fresh

# Build-Verzeichnis löschen
rm -rf out/build/windows-ninja-debug
cmake --preset windows-ninja-debug
```

### 8.5 Parallel bauen

```bash
# Mit 8 Jobs
cmake --build out/build/windows-ninja-debug -j8

# Alle CPUs
cmake --build out/build/windows-ninja-debug -j
```

---

## 9. Externals verwenden

### 9.1 Lokales External

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    },
    "executables": [
        {
            "name": "MyApp",
            "externals": ["doctest"]
        }
    ]
}
```

### 9.2 Git External

```json
{
    "externals": {
        "spdlog": {
            "git": "https://github.com/gabime/spdlog.git",
            "tag": "v1.12.0"
        }
    }
}
```

### 9.3 Usage im Code

```cpp
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("Hello from spdlog!");
    return 0;
}
```

Mehr Details: → [Externals UserGuide](Externals.md)

---

## 10. Tests hinzufügen

### 10.1 Solution.json Eintrag

```json
{
    "externals": {
        "doctest": { "path": "externals/doctest" }
    },
    "tests": [
        {
            "name": "CoreLib_Tests",
            "type": "unit",
            "framework": "doctest",
            "dependencies": ["CoreLib"],
            "externals": ["doctest"]
        }
    ]
}
```

### 10.2 Test-Datei

**projects/tests/unit/CoreLib_Tests/src/test_main.cpp:**
```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "CoreLib/Calculator.h"

TEST_CASE("Calculator::add") {
    CoreLib::Calculator calc;
    CHECK(calc.add(2, 3) == 5);
    CHECK(calc.add(-1, 1) == 0);
}
```

### 10.3 Tests ausführen

```bash
# Mit Tests bauen
cmake --preset windows-ninja-debug -DBUILD_TESTS=ON
cmake --build out/build/windows-ninja-debug

# Tests ausführen
ctest --test-dir out/build/windows-ninja-debug
```

Mehr Details: → [Testing UserGuide](Testing.md)

---

## 11. Workflow-Summary

### Der goldene Workflow

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Solution.json Eintrag erstellen       ← IMMER ZUERST!   │
│    └─ Name, Version, Typ definieren                        │
├─────────────────────────────────────────────────────────────┤
│ 2. CMake konfigurieren                                      │
│    └─ cmake --preset windows-ninja-debug                   │
├─────────────────────────────────────────────────────────────┤
│ 3. Dateien anlegen                                          │
│    └─ main.cpp / Header / Source                           │
├─────────────────────────────────────────────────────────────┤
│ 4. Bauen                                                    │
│    └─ cmake --build out/build/windows-ninja-debug          │
├─────────────────────────────────────────────────────────────┤
│ 5. Testen                                                   │
│    └─ Programm ausführen / ctest                           │
└─────────────────────────────────────────────────────────────┘
```

### Checkliste: Neues Modul hinzufügen

- [ ] Solution.json Eintrag schreiben
- [ ] CMake konfigurieren / generieren
- [ ] Dateien anlegen (src/, include/)
- [ ] Bauen und Error beheben
- [ ] Dokumentation erstellen
- [ ] Changelog aktualisieren

### Reihenfolge bei Dependencies

```json
{
    "libraries": [
        { "name": "BaseLib" },           // 1. Keine Dependencies
        { "name": "MiddleLib", "dependencies": ["BaseLib"] },  // 2.
        { "name": "TopLib", "dependencies": ["MiddleLib"] }    // 3.
    ],
    "executables": [
        { "name": "App", "dependencies": ["TopLib"] }  // 4. Nutzt alles
    ]
}
```

---

## 12. Stolpersteine und Lösungen

### 12.1 "Target existiert nicht"

**Problem:**
```
CMake Error: The target "MyLib" does not exist.
```

**Ursache:** Library nicht in Solution.json definiert oder Schreibfehler.

**Lösung:**
1. Prüfe ob Library in `libraries` Array existiert
2. Prüfe Schreibweise (case-sensitive!)
3. Stelle sicher, dass Library VOR dem Executable definiert ist

### 12.2 "Source-Dateien nicht gefunden"

**Problem:**
```
[E0401] No source files found in path: projects/demos/exec/MyApp/src
```

**Ursache:** Pfad existiert nicht oder enthält keine Source-Dateien.

**Lösung:**
1. Verzeichnis erstellen: `mkdir -p projects/demos/exec/MyApp/src`
2. main.cpp anlegen
3. Oder expliziten `path` in Solution.json angeben

### 12.3 "Include-Pfad nicht gefunden"

**Problem:**
```
fatal error: 'CoreLib/Calculator.h' file not found
```

**Ursache:** Library nicht in `dependencies` aufgeführt.

**Lösung:**
```json
{
    "name": "MyApp",
    "dependencies": ["CoreLib"]  // ← Hinzufügen!
}
```

### 12.4 "CMake-Cache veraltet"

**Problem:** Changes in Solution.json werden nicht übernommen.

**Ursache:** CMake-Cache enthält alte Configuration.

**Lösung:**
```bash
# Cache neu generieren
cmake --preset windows-ninja-debug --fresh
```

### 12.5 "Preset nicht gefunden"

**Problem:**
```
CMake Error: Could not read presets: No such preset: "my-preset"
```

**Ursache:** Preset existiert nicht oder JSON-Syntaxfehler.

**Lösung:**
```bash
# Verfügbare Presets anzeigen
cmake --list-presets

# JSON-Syntax prüfen
cat CMakePresets.json | python -m json.tool
```

---

## 13. Troubleshooting

### Checkliste bei Build-Problemen

1. ☐ Solution.json Syntax korrekt? (JSON validieren)
2. ☐ Alle Dependencies definiert?
3. ☐ Pfade existieren?
4. ☐ CMake-Cache aktuell? (`--fresh`)
5. ☐ Compiler installiert und im PATH?
6. ☐ Preset existiert? (`--list-presets`)

### Häufige Error

| Error | Ursache | Lösung |
|--------|---------|--------|
| "Target does not exist" | Dependency fehlt | Zu `libraries` hinzufügen |
| "No source files" | Leeres Verzeichnis | Dateien anlegen |
| "file not found" | Include fehlt | `dependencies` prüfen |
| "undefined reference" | Linking-Error | `dependencies` oder `externals` |
| "Preset not found" | Tipfehler | `--list-presets` |

### Debug-Tips

```bash
# Verbose CMake-Output
cmake --preset ... --debug-output

# CMake-Variablen anzeigen
cmake -LAH out/build/windows-ninja-debug

# Generierte Targets anzeigen
cmake --build out/build/... --target help
```

---

## 14. Nächste Schritte

### Du hast die Grundlagen gemeistert! Weiter geht's mit:

| Thema | Guide | Was du lernst |
|-------|-------|---------------|
| **Externe Bibliotheken** | [Externals UserGuide](Externals.md) | BASS, ImGui, spdlog einbinden |
| **Neue Externals hinzufügen** | [Adding_Externals UserGuide](Adding_Externals.md) | Eigene Hooks schreiben |
| **Testing** | [Testing UserGuide](Testing.md) | doctest, GoogleTest, Catch2 |
| **Qt6** | [Qt6_Integration UserGuide](Qt6_Integration.md) | Qt6 GUI-Anwendungen |
| **User Presets** | [CMakeUserPresets UserGuide](CMakeUserPresets.md) | Persönliche Build-Configuration |

### Fortgeschrittene Themen

- **Multi-Plattform-Builds** – Windows, Linux, macOS gleichzeitig
- **CI/CD-Integration** – GitHub Actions, GitLab CI
- **Code-Qualität** – clang-format, clang-tidy
- **Packaging** – Installer erstellen mit CPack

---

## 15. See Also

- [Solution_Schema.md](../references/Solution_Schema.md) – Vollständige JSON-Schema-Reference
- [Externals.md](../references/Externals.md) – Alle verfügbaren Externals
- [ErrorCodes.md](../references/ErrorCodes.md) – Errorcodes und Lösungen
- [CMakePresets_Reference.md](../references/CMakePresets.md) – Preset-Reference

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Initial: Umfassender Einstiegs-Guide für CMake Architecture** |
