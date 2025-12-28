# Source.cmake Templates
## CMake Architecture V2 - Source Collection (v0.6)

## Übersicht

Diese Templates zeigen das neue **v0.6 Format** für Source.cmake Dateien.

### Wichtige Änderungen zu v0.5:

| Alt (v0.5)                              | Neu (v0.6)                        |
|-----------------------------------------|-----------------------------------|
| `${EXECUTABLE_NAME}_PROJECT_SOURCES`    | `${TARGET_NAME}_SOURCES`          |
| `${EXECUTABLE_NAME}_PROJECT_HEADERS`    | `${TARGET_NAME}_HEADERS`          |
| `${EXECUTABLE_NAME}_PROJECT_TEMPLATES`  | `${TARGET_NAME}_TEMPLATES`        |
| `${EXECUTABLE_NAME}_PROJECT_INLINES`    | `${TARGET_NAME}_INLINES`          |
| `${EXECUTABLE_NAME}_PROJECT_IMPL`       | `${TARGET_NAME}_IMPL`             |

### Warum `list(APPEND ...)` statt `set()`?

Bei **rekursiven Subfoldern** ist `list(APPEND ...)` zwingend erforderlich:

```cmake
# Root src/Source.cmake
list(APPEND ${TARGET_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/Application.cpp")

# Inkludiert Subfolder
include("${CMAKE_CURRENT_LIST_DIR}/core/Source.cmake")

# In core/Source.cmake:
list(APPEND ${TARGET_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/Engine.cpp")
# → Beide Dateien sind jetzt in ${TARGET_NAME}_SOURCES!
```

Mit `set()` würde core/Source.cmake die vorherigen Dateien überschreiben!

---

## Template-Dateien

| Template                        | Verwendung                                    |
|---------------------------------|-----------------------------------------------|
| `Source_App_Src_Full.cmake`     | App Core library (src/)                       |
| `Source_App_Include_Full.cmake` | App öffentliche Headers (include/)            |
| `Source_App_Main_Full.cmake`    | App Entry Point (main/)                       |
| `Source_Test_Full.cmake`        | Unit/Integration/Performance Tests            |
| `Source_Subfolder_Full.cmake`   | Subfolder (src/core/, src/audio/, etc.)       |
| `Source_Executable_Full.cmake`  | Standalone Executables (demos/exec/)          |

---

## Unterstützte Dateitypen

| Variable               | Extensions              | Beschreibung                    |
|------------------------|-------------------------|---------------------------------|
| `_SOURCES`             | .c, .cpp, .cxx, .cc     | Kompilierbare Quelldateien      |
| `_HEADERS`             | .h, .hpp, .hxx, .hh     | Header-Dateien                  |
| `_TEMPLATES`           | .tpp, .txx, .ipp        | Template-Implementierungen      |
| `_INLINES`             | .inl                    | Inline-Implementierungen        |
| `_IMPL`                | .impl                   | PIMPL/Detail-Implementierungen  |

---

## Beispiel: CompleteApp

Das Verzeichnis `example/` enthält ein vollständiges App-Container Projekt:

```
example/projects/apps/CompleteApp/
├── include/
│   ├── Source.cmake           ← Öffentliche Headers
│   ├── Application.hpp
│   ├── core/
│   │   ├── Source.cmake       ← Subfolder
│   │   └── Engine.hpp
│   └── utils/
│       └── Source.cmake
├── src/
│   ├── Source.cmake           ← Core Implementation
│   ├── Application.cpp
│   ├── core/
│   │   ├── Source.cmake       ← Mit .tpp, .inl, .impl
│   │   ├── Engine.cpp
│   │   └── Engine.impl
│   ├── audio/
│   │   └── Source.cmake
│   └── utils/
│       └── Source.cmake
├── main/
│   └── Source.cmake           ← Entry Point (nur main.cpp)
└── tests/unit/UnitTests/
    └── Source.cmake           ← Tests
```

---

## Debug-Ausgaben

Die Templates enthalten Debug-Ausgaben für Fehlersuche:

```cmake
# Zeigt gefundene Dateien
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found sources: ${_local_sources}" ID DEB_FOUND_MSG)

# Zeigt aggregierte Totale
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated SOURCES: ${${TARGET_NAME}_SOURCES}" ID DEB_AGG)
```

Aktiviere Debug mit `-DCMAKE_DEBUG_LEVEL=...` oder in Solution.json.

---

## Migration von v0.5

1. Ersetze `${EXECUTABLE_NAME}_PROJECT_*` durch `${TARGET_NAME}_*`
2. Prüfe dass alle Dateien `list(APPEND ...)` verwenden
3. Stelle sicher dass Debug-IDs konsistent sind

---

## Version

- Format: v0.6
- Datum: 2025-12-18
- Kompatibel mit: CMake Architecture V2, Phase 9
