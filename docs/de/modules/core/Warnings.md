# Warnings.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Warnings.md](../../en/modules/core/Warnings.md)  
> **Modul:** [`cmake/core/Warnings.cmake`](../../../../cmake/core/Warnings.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Third-Party Code](#6-third-party-code)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `Warnings.cmake` Modul konfiguriert **compiler-spezifische Warning-Level** für Targets. Es aktiviert hohe Warnstufen um Code-Qualität zu fördern.

### Kernidee

Einheitliche, hohe Warnstufen für alle Targets — konsistent über alle Compiler hinweg.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| MSVC | /W4 (hohe Warnstufe) |
| GCC | -Wall -Wextra -Wpedantic |
| Clang | -Wall -Wextra -Wpedantic |

### Verwendung durch

- ExecutableCreate.cmake
- LibraryCreate.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| — | — | Keine (Standalone-Modul) |

**Design-Entscheidung:** Warnings.cmake ist ein minimalistisches Standalone-Modul ohne Debug- oder Error-Integration.

→ Siehe [Offene Abklärungen](../../../projects/buildsystem/concepts/Offene_Abklaerungen_Core_Module.md) für Details zu möglicher dbg()-Integration.

---

## 3. Konzept

### 3.1 Compiler-spezifische Flags

| Compiler | Flags | Beschreibung |
|----------|-------|--------------|
| MSVC | `/W4` | Hohe Warnstufe |
| GCC | `-Wall -Wextra -Wpedantic` | Standard + Extra + Pedantic |
| Clang | `-Wall -Wextra -Wpedantic` | Wie GCC |

### 3.2 Warning-Level (MSVC)

| Level | Bedeutung |
|-------|-----------|
| `/W0` | Keine Warnungen |
| `/W1` | Nur schwere |
| `/W2` | Signifikante |
| `/W3` | Produktions-Qualität |
| `/W4` | Informative (empfohlen) ✅ |
| `/Wall` | Alle (sehr viele!) |

### 3.3 Warning-Flags (GCC/Clang)

| Flag | Bedeutung |
|------|-----------|
| `-Wall` | "All" (wichtigste Warnungen) |
| `-Wextra` | Zusätzliche Warnungen |
| `-Wpedantic` | Strikte ISO-C++ Konformität |

---

## 4. API-Referenz

### 4.1 apply_warnings()

Setzt Warning-Level für ein Target.

```cmake
apply_warnings(<TARGET_NAME>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `TARGET_NAME` | String | ✓ | CMake Target (muss bereits existieren) |

**Voraussetzung:** Das Target muss bereits mit `add_executable()` oder `add_library()` erstellt worden sein.

**Rückgabe:** Keine (setzt Compile-Options)

**Beispiel:**

```cmake
add_executable(MyApp main.cpp)
apply_warnings(MyApp)
```

---

## 5. Verwendungsbeispiele

### 5.1 Einfache Anwendung

```cmake
add_executable(MyApp main.cpp)
apply_warnings(MyApp)
```

### 5.2 In der Executable-Pipeline

```cmake
function(_create_executable_target CTX)
    ctx_get(${CTX} NAME _name)
    
    add_executable(${_name} ${_sources})
    setup_output_dirs(${_name})
    apply_warnings(${_name})           # ← Warnungen
    apply_compiler_options(${_name})   # ← Weitere Optionen
endfunction()
```

### 5.3 Für Libraries

```cmake
add_library(CoreLib STATIC core.cpp)
apply_warnings(CoreLib)
```

---

## 6. Third-Party Code

### 6.1 Warnungen deaktivieren

Für externen Code ohne strikte Warnungen:

```cmake
# Eigener Code mit Warnungen
add_library(MyLib ...)
apply_warnings(MyLib)

# Third-Party ohne strikte Warnungen
add_library(ThirdParty ...)
if(MSVC)
    target_compile_options(ThirdParty PRIVATE /W0)
else()
    target_compile_options(ThirdParty PRIVATE -w)
endif()
```

### 6.2 Einzelne Warnungen deaktivieren

```cmake
# MSVC: Specific warnings deaktivieren
target_compile_options(${TARGET} PRIVATE 
    /wd4996   # deprecated functions
    /wd4100   # unused parameter
)

# GCC/Clang: Specific warnings deaktivieren
target_compile_options(${TARGET} PRIVATE 
    -Wno-unused-parameter
    -Wno-deprecated-declarations
)
```

### 6.3 Zusätzliche Warnungen aktivieren

```cmake
# Optionale strikte Warnungen
target_compile_options(${TARGET} PRIVATE 
    -Wshadow           # Variable shadows another
    -Wconversion       # Implicit conversions
    -Wnon-virtual-dtor # Non-virtual destructors
)
```

---

## 7. Siehe auch

- [CompilerOptions.cmake](CompilerOptions.md) — Weitere Compiler-Optionen (Clang-Tidy, RTTI, Exceptions)
- [ExecutableCreate.cmake](../project/ExecutableCreate.md) — Verwendet apply_warnings
- [LibraryCreate.cmake](../project/LibraryCreate.md) — Verwendet apply_warnings

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): apply_warnings für MSVC/GCC/Clang |
