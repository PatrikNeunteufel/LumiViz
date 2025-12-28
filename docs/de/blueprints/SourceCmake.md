# Source.cmake — Standard für CMake Quelldatei-Listen

> **Version:** 1.0.0  
> **Datum:** 2025-12-26  
> **Typ:** Blueprint  
> **Status:** Stabil  
> **Basiert auf:** CMake Architecture  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  
> **English:** [SourceCmake.md](../../en/blueprints/SourceCmake.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Geltungsbereich](#2-geltungsbereich)
3. [Pflichtstruktur](#3-pflichtstruktur)
4. [Datei-Kategorien](#4-datei-kategorien)
5. [Vollständiges Template](#5-vollständiges-template)
6. [Beispiele](#6-beispiele)
7. [Häufige Fehler](#7-häufige-fehler)
8. [Review-Checkliste](#8-review-checkliste)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert die **Struktur für Source.cmake Dateien** im CMake Architecture V2 Build-System. Source.cmake Dateien sammeln Quelldateien rekursiv in Verzeichnisbäumen.

### Zweck

- Quelldateien eines Verzeichnisses auflisten
- Dateien nach Typ kategorisieren (sources, headers, templates, etc.)
- Rekursive Aggregation an übergeordnete Scopes
- Debug-Ausgaben für Build-Diagnose

### Kernprinzip

> **Jedes Verzeichnis mit Quelldateien benötigt eine Source.cmake.**

---

## 2. Geltungsbereich

Dieser Blueprint gilt für alle Source.cmake Dateien in:

- `projects/apps/[App]/include/` — Header-Dateien
- `projects/apps/[App]/src/` — Implementierungsdateien
- `projects/apps/[App]/main/` — Entry-Point
- `projects/apps/[App]/tests/` — Test-Dateien
- `projects/libs/[Lib]/include/` — Library-Header
- `projects/libs/[Lib]/src/` — Library-Implementierung

---

## 3. Pflichtstruktur

Jede Source.cmake **MUSS** diese Struktur haben:

```
┌─────────────────────────────────────────────────────────────────────┐
│  1. Header-Kommentar mit Pfad                                       │
│  2. dbg() Einstiegs-Nachricht                                       │
│  3. 5 lokale Listen definieren (_local_*)                           │
│  4. dbg() für gefundene Dateien                                     │
│  5. list(APPEND) für Aggregation                                    │
│  6. dbg() für aggregierte Dateien                                   │
│  7. unset() für Cleanup                                             │
│  8. dbg() für Unterordner                                           │
│  9. include() für Unterordner (am Ende!)                            │
└─────────────────────────────────────────────────────────────────────┘
```

### 3.1 Header-Kommentar

```cmake
# ==============================================================================
# Source.cmake for [relativer/pfad]/
# CMake Architecture V2 - [Projektname]
# ==============================================================================
```

### 3.2 Debug-Einstieg

```cmake
dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)
```

### 3.3 Fünf lokale Listen

**ALLE fünf Listen MÜSSEN definiert werden**, auch wenn leer:

```cmake
set(_local_sources
    # (sources - *.c; *.cpp)
)
set(_local_headers
    # (headers - *.h; *.hpp)
)
set(_local_templates
    # (templates - *.t; *.tpp)
)
set(_local_inlines
    # (inlines - *.inl)
)
set(_local_impl
    # (impl - *.impl)
)
```

### 3.4 Debug-Ausgabe (gefunden)

```cmake
dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
```

### 3.5 Aggregation

```cmake
list(APPEND ${EXECUTABLE_NAME}_PROJECT_SOURCES ${_local_sources})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_HEADERS ${_local_headers})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_TEMPLATES ${_local_templates})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_INLINES ${_local_inlines})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_IMPL ${_local_impl})
```

### 3.6 Debug-Ausgabe (aggregiert)

```cmake
dbg(DBG_NORMAL "Aggregated sources  : ${${EXECUTABLE_NAME}_PROJECT_SOURCES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated headers  : ${${EXECUTABLE_NAME}_PROJECT_HEADERS}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated templates: ${${EXECUTABLE_NAME}_PROJECT_TEMPLATES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated inlines  : ${${EXECUTABLE_NAME}_PROJECT_INLINES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated impl     : ${${EXECUTABLE_NAME}_PROJECT_IMPL}" ID DEB_AGG)
```

### 3.7 Cleanup

```cmake
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
```

### 3.8 Unterordner-Includes

```cmake
dbg(DBG_ULTRA_RARE "[Ordnername] subfolders:" ID INCLUDE_MSG)
# Include subfolders recursively
include("${CMAKE_CURRENT_LIST_DIR}/Subfolder1/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Subfolder2/Source.cmake")
```

**WICHTIG:** Unterordner-Includes stehen **IMMER am Ende** der Datei!

---

## 4. Datei-Kategorien

| Liste | Erweiterungen | Beschreibung |
|-------|---------------|--------------|
| `_local_sources` | `.c`, `.cpp`, `.cc`, `.cxx` | Kompilierbare Quelldateien |
| `_local_headers` | `.h`, `.hpp`, `.hxx` | Header-Dateien |
| `_local_templates` | `.t`, `.tpp` | Template-Implementierungen |
| `_local_inlines` | `.inl` | Inline-Implementierungen |
| `_local_impl` | `.impl` | Implementierungs-Details |

### Zuordnungsregeln

| Dateityp | Richtige Liste |
|----------|----------------|
| `MyClass.cpp` | `_local_sources` |
| `MyClass.hpp` | `_local_headers` |
| `MyClass.tpp` | `_local_templates` |
| `MyClass.inl` | `_local_inlines` |
| `Errors.tpp` | `_local_templates` |

---

## 5. Vollständiges Template

```cmake
# ==============================================================================
# Source.cmake for [pfad]/
# CMake Architecture V2 - [Projektname]
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# Set local lists for this directory
set(_local_sources
    # (sources - *.c; *.cpp)
)
set(_local_headers
    # (headers - *.h; *.hpp)
    "${CMAKE_CURRENT_LIST_DIR}/MyHeader.hpp"
)
set(_local_templates
    # (templates - *.t; *.tpp)
)
set(_local_inlines
    # (inlines - *.inl)
)
set(_local_impl
    # (impl - *.impl)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)

# Aggregate to parent scope
list(APPEND ${EXECUTABLE_NAME}_PROJECT_SOURCES ${_local_sources})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_HEADERS ${_local_headers})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_TEMPLATES ${_local_templates})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_INLINES ${_local_inlines})
list(APPEND ${EXECUTABLE_NAME}_PROJECT_IMPL ${_local_impl})

dbg(DBG_NORMAL "Aggregated sources  : ${${EXECUTABLE_NAME}_PROJECT_SOURCES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated headers  : ${${EXECUTABLE_NAME}_PROJECT_HEADERS}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated templates: ${${EXECUTABLE_NAME}_PROJECT_TEMPLATES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated inlines  : ${${EXECUTABLE_NAME}_PROJECT_INLINES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated impl     : ${${EXECUTABLE_NAME}_PROJECT_IMPL}" ID DEB_AGG)

# Optional cleanup (cosmetic)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)

dbg(DBG_ULTRA_RARE "[Ordnername] subfolders:" ID INCLUDE_MSG)
# Include subfolders recursively
# include("${CMAKE_CURRENT_LIST_DIR}/Subfolder/Source.cmake")
```

---

## 6. Beispiele

### 6.1 Header-Verzeichnis mit Unterordnern

```cmake
# ==============================================================================
# Source.cmake for include/Services/
# CMake Architecture V2 - LumiPulse
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    # (no sources)
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/IEventBus.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/EventBus.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ICommand.hpp"
)
set(_local_templates
    # (no templates)
)
set(_local_inlines
    # (no inlines)
)
set(_local_impl
    # (no impl)
)

# ... dbg, list(APPEND), unset ...

dbg(DBG_ULTRA_RARE "Services subfolders:" ID INCLUDE_MSG)
include("${CMAKE_CURRENT_LIST_DIR}/Events/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Commands/Source.cmake")
```

### 6.2 Test-Verzeichnis

```cmake
# ==============================================================================
# Source.cmake for tests/unit/Core_UnitTests/
# CMake Architecture V2 - LumiPulse
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Types_Tests.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Result_Tests.cpp"
)
set(_local_headers
    # (no headers)
)
set(_local_templates
    # (no templates)
)
set(_local_inlines
    # (no inlines)
)
set(_local_impl
    # (no impl)
)

# ... dbg, list(APPEND), unset ...

dbg(DBG_ULTRA_RARE "Core_UnitTests subfolders:" ID INCLUDE_MSG)
# (no subfolders)
```

### 6.3 Leeres Platzhalter-Verzeichnis

```cmake
# ==============================================================================
# Source.cmake for src/Core/
# CMake Architecture V2 - LumiPulse
# Note: Core ist aktuell header-only. Platzhalter für zukünftige .cpp Dateien.
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    # (no sources - Core is header-only)
)
set(_local_headers
    # (no headers)
)
set(_local_templates
    # (no templates)
)
set(_local_inlines
    # (no inlines)
)
set(_local_impl
    # (no impl)
)

# ... dbg, list(APPEND), unset ...

dbg(DBG_ULTRA_RARE "Core subfolders:" ID INCLUDE_MSG)
# (no subfolders)
```

---

## 7. Häufige Fehler

### ❌ Falsch: Nur eine Liste

```cmake
# FALSCH - Fehlende Listen
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/MyHeader.hpp"
)
```

### ✅ Richtig: Alle fünf Listen

```cmake
# RICHTIG - Alle Listen definiert
set(_local_sources
    # (no sources)
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/MyHeader.hpp"
)
set(_local_templates
    # (no templates)
)
set(_local_inlines
    # (no inlines)
)
set(_local_impl
    # (no impl)
)
```

### ❌ Falsch: Unterordner am Anfang

```cmake
# FALSCH - Unterordner-Include am Anfang
include("${CMAKE_CURRENT_LIST_DIR}/Events/Source.cmake")

set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/EventBus.hpp"
)
```

### ✅ Richtig: Unterordner am Ende

```cmake
# RICHTIG - Unterordner-Include am Ende
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/EventBus.hpp"
)
# ... rest ...

include("${CMAKE_CURRENT_LIST_DIR}/Events/Source.cmake")
```

### ❌ Falsch: Fehlende dbg() Aufrufe

```cmake
# FALSCH - Keine Debug-Ausgaben
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/MyHeader.hpp"
)
list(APPEND ${EXECUTABLE_NAME}_PROJECT_HEADERS ${_local_headers})
```

### ❌ Falsch: .tpp in headers statt templates

```cmake
# FALSCH
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/Errors.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Errors.tpp"  # FALSCH!
)
```

### ✅ Richtig: .tpp in templates

```cmake
# RICHTIG
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/Errors.hpp"
)
set(_local_templates
    "${CMAKE_CURRENT_LIST_DIR}/Errors.tpp"  # RICHTIG!
)
```

---

## 8. Review-Checkliste

**Struktur:**

- [ ] Header-Kommentar mit korrektem Pfad
- [ ] dbg(DBG_OFTEN) Einstiegs-Nachricht
- [ ] Alle 5 lokalen Listen definiert
- [ ] dbg(DBG_NORMAL) für gefundene Dateien (5x)
- [ ] list(APPEND) für alle 5 Listen
- [ ] dbg(DBG_NORMAL) für aggregierte Dateien (5x)
- [ ] unset() für alle 5 lokalen Listen
- [ ] dbg(DBG_ULTRA_RARE) für Unterordner
- [ ] Unterordner-Includes am Ende

**Datei-Zuordnung:**

- [ ] .cpp/.c in `_local_sources`
- [ ] .hpp/.h in `_local_headers`
- [ ] .tpp/.t in `_local_templates`
- [ ] .inl in `_local_inlines`
- [ ] .impl in `_local_impl`

**Pfade:**

- [ ] Alle Pfade mit `${CMAKE_CURRENT_LIST_DIR}/` prefix
- [ ] Unterordner-Includes existieren

---

## 9. Siehe auch

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- CMake Architecture V2 Dokumentation
- [ImplementationPlan.md](ImplementationPlan.md) — Für Implementierungspläne

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-26** | **Initial: Vollständiges Template, Beispiele, häufige Fehler** |
