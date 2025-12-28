# App Template Reference

> **Version:** 1.0.0  
> **Datum:** 2025-12-18  
> **Typ:** Reference  
> **Status:** Entwurf  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Verzeichnisstruktur](#2-verzeichnisstruktur)
3. [main.cpp](#3-maincpp)
4. [Application Interface](#4-application-interface)
5. [Source.cmake](#5-sourcecmake)
6. [Precompiled Header](#6-precompiled-header)
7. [Build-Defines](#7-build-defines)
8. [CMake-Integration](#8-cmake-integration)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Das App-Template definiert die Standardstruktur für App-Container im CMake Architecture Build-System. Es trennt Entry Point (main/) von Anwendungslogik (include/, src/) für maximale Testbarkeit.

### Pfad

```
projects/templates/App/
```

---

## 2. Verzeichnisstruktur

```
App/
├── README.md                      # Kurzanleitung
├── include/                       # Header
│   ├── Source.cmake
│   └── Application.hpp
├── src/                           # Implementation
│   ├── Source.cmake
│   └── Application.cpp
├── main/                          # Entry Point
│   ├── Source.cmake
│   └── main.cpp                   # Generisch, nicht ändern
├── pch/                           # Precompiled Header
│   └── pch.h
└── tests/
    └── unit/                      # Unit Tests
        └── Application_Tests.cpp
```

### Komponenten

| Verzeichnis | Beschreibung | Testbar |
|-------------|--------------|---------|
| `include/` | Header-Dateien (.hpp) | ✅ |
| `src/` | Implementation (.cpp) | ✅ |
| `main/` | Entry Point (main.cpp) | ❌ |
| `pch/` | Precompiled Header | — |
| `tests/unit/` | Unit Tests | — |

---

## 3. main.cpp

### Pfad

```
main/main.cpp
```

### Inhalt

```cpp
#include "Application.hpp"

#if defined(_WIN32)
    #include <Windows.h>
#endif

namespace {

int commonMain(int argc, char* argv[]) {
    Application app;
    
    if (!app.init(argc, argv)) {
        return 1;
    }
    
    int result = app.run();
    app.shutdown();
    
    return result;
}

} // namespace

#if defined(_WIN32) && defined(APP_GUI)

int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
) {
    return commonMain(__argc, __argv);
}

#else

int main(int argc, char* argv[]) {
    return commonMain(argc, argv);
}

#endif
```

### Eigenschaften

| Eigenschaft | Wert |
|-------------|------|
| Änderbar | ❌ Nein |
| Plattformspezifisch | ✅ Ja (via Präprozessor) |
| Abhängigkeiten | Nur `Application.hpp` |

---

## 4. Application Interface

### Pfad

```
include/Application.hpp
```

### Pflicht-Interface

```cpp
class Application {
public:
    Application();
    ~Application();
    
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    [[nodiscard]] bool init(int argc, char* argv[]);
    [[nodiscard]] int run();
    void shutdown();
    
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& version() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
};
```

### Lifecycle-Methoden

| Methode | Beschreibung | Rückgabe |
|---------|--------------|----------|
| `init()` | Initialisierung (Qt, Audio, Config) | `true` bei Erfolg |
| `run()` | Hauptschleife (`QApplication::exec()`) | Exit-Code |
| `shutdown()` | Ressourcen freigeben | — |

### Aufruf-Reihenfolge

```
main() → Application() → init() → run() → shutdown() → ~Application()
```

---

## 5. Source.cmake

Jeder Ordner mit Quellcode enthält eine `Source.cmake` zur expliziten Datei-Registrierung.

### Struktur

```cmake
# Source.cmake für {ordner}/
dbg(DBG_OFTEN "${CMAKE_CURRENT_LIST_DIR}/Source.cmake..." ID INCLUDE_MSG)

set(_local_sources
    # "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
)
set(_local_headers
    # "${CMAKE_CURRENT_LIST_DIR}/File.hpp"
)
set(_local_templates
    # "${CMAKE_CURRENT_LIST_DIR}/File.tpp"
)
set(_local_inlines
    # "${CMAKE_CURRENT_LIST_DIR}/File.inl"
)
set(_local_impl
    # "${CMAKE_CURRENT_LIST_DIR}/File.impl"
)

# Aggregation
list(APPEND ${EXECUTABLE_NAME}_PROJECT_SOURCES ${_local_sources})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_HEADERS ${_local_headers})
# ...

# Cleanup
unset(_local_sources)
# ...

# Subfolder (optional)
# include("${CMAKE_CURRENT_LIST_DIR}/subfolder/Source.cmake")
```

### Datei-Typen

| Variable | Extensions | Beschreibung |
|----------|------------|--------------|
| `_local_sources` | .c, .cpp | Kompilierbare Quellen |
| `_local_headers` | .h, .hpp | Header-Dateien |
| `_local_templates` | .t, .tpp | Template-Implementationen |
| `_local_inlines` | .inl | Inline-Implementationen |
| `_local_impl` | .impl | Pimpl-Implementationen |

---

## 6. Precompiled Header

### Pfad

```
pch/pch.h
```

### Zweck

Häufig verwendete, stabile Includes zur Reduzierung der Build-Zeit.

### Inhalt

```cpp
#pragma once

// Standard Library
#include <memory>
#include <string>
#include <vector>
// ...

// Framework (projektspezifisch)
// #include <QApplication>
// #include <bass.h>
```

### Extension

Die `.h` Extension ist Standard für PCH, auch bei C++. Einige Compiler/Tools verarbeiten `.hpp` nicht korrekt.

### Suchpfad-Priorität

Wenn PCH aktiviert und kein custom `path` angegeben ist:

| Priorität | Pfad |
|-----------|------|
| 1 | `{app-path}/pch/{header}` |
| 2 | `{app-path}/src/{header}` |
| 3 | `{app-path}/{header}` |

Bei custom `path`: `projects/{path}/{header}`

### Best Practices

| Include | PCH geeignet |
|---------|--------------|
| Standard Library | ✅ Ja |
| Qt/Framework Headers | ✅ Ja |
| Eigene Header | ❌ Nein (ändern sich oft) |

---

## 7. Build-Defines

Das Build-System setzt automatisch:

| Define | Bedingung | Beschreibung |
|--------|-----------|--------------|
| `APP_GUI` | `runner.type = "GUI"` | Windows: WinMain |
| `APP_CONSOLE` | `runner.type = "CONSOLE"` | Standard main |

### Plattform-Verhalten

| runner.type | Windows | Linux | macOS |
|-------------|---------|-------|-------|
| `GUI` | `WinMain` (kein Console) | `main` | `main` |
| `CONSOLE` | `main` (mit Console) | `main` | `main` |

---

## 8. CMake-Integration

### Generierte Targets

Für eine App `MyApp`:

```cmake
# Executable
add_executable(MyApp WIN32  # Bei GUI
    # Sources aus Source.cmake aggregiert
)

# Include Directories
target_include_directories(MyApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/pch
)

# Externals
target_link_libraries(MyApp PRIVATE Qt6::Widgets bass::bass)

# Define
target_compile_definitions(MyApp PRIVATE APP_GUI)

# PCH
target_precompile_headers(MyApp PRIVATE pch/pch.h)
```

### WIN32 Flag

Bei `runner.type = "GUI"` wird unter Windows das `WIN32` Flag gesetzt:

```cmake
if(WIN32 AND runner_type STREQUAL "GUI")
    add_executable(${APP_NAME} WIN32 ...)
else()
    add_executable(${APP_NAME} ...)
endif()
```

---

## 9. Siehe auch

- [App Creation Guide](../guides/App_Creation_Guide.md) — Schritt-für-Schritt Anleitung
- [Solution Schema](Solution_Schema.md) — apps Array Dokumentation
- [AppContainer Concept](../concepts/AppContainer_Concept.md) — Architektur-Konzept

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.1** | **2025-12-18** | **PCH Suchpfad-Priorität dokumentiert** |
| 0.1.0 | 2025-12-17 | Initial mit korrekter Struktur (include/, src/, main/, pch/) |
