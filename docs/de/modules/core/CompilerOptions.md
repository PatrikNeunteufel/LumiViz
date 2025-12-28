# CompilerOptions.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [CompilerOptions.md](../../en/modules/core/CompilerOptions.md)  
> **Modul:** [`cmake/core/CompilerOptions.cmake`](../../../../cmake/core/CompilerOptions.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [API-Referenz](#4-api-referenz)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Cache-Variablen](#6-cache-variablen)
7. [Plattform-Details](#7-plattform-details)
8. [Best Practices](#8-best-practices)
9. [Performance-Impact](#9-performance-impact)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Das `CompilerOptions.cmake` Modul konfiguriert compiler-spezifische Optionen für Targets. Es unterstützt präzise Compiler-Erkennung und Per-Target Overrides.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Compiler-Erkennung | MSVC, GCC, Clang (inkl. Apple/Clang-CL) |
| Strict Conformance | `/permissive-`, `/Zc:*` (MSVC) |
| Feature-Kontrolle | Exceptions, RTTI |
| Code-Qualität | Clang-Tidy Integration |
| Per-Target Overrides | Flags für spezielle Targets |

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Debug.cmake | 0.1.1 | `dbg_init`, `dbg`, `enddbgblock` |
| Errors.cmake | 0.1.2 | `cmake_warn` |

---

## 3. Konzept

### 3.1 Präzise Compiler-Erkennung

```cmake
if(MSVC)                                    # Windows MSVC
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GCC
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(APPLE)                               # Apple Clang
    elseif(WIN32)                           # Clang-CL
    else()                                  # Linux Clang
    endif()
endif()
```

### 3.2 Per-Target Override Flags

| Flag | Funktion |
|------|----------|
| `SKIP_STRICT_CONFORMANCE` | MSVC `/permissive-` überspringen |
| `SKIP_NOMINMAX` | `NOMINMAX` nicht definieren |
| `FORCE_EXCEPTIONS` | Exceptions aktiviert lassen |
| `FORCE_RTTI` | RTTI aktiviert lassen |
| `SKIP_CLANG_TIDY` | Clang-Tidy überspringen |

---

## 4. API-Referenz

### apply_compiler_options()

Wendet Compiler-Optionen auf ein Target an.

```cmake
apply_compiler_options(<TARGET_NAME>
    [SKIP_STRICT_CONFORMANCE]
    [SKIP_NOMINMAX]
    [FORCE_EXCEPTIONS]
    [FORCE_RTTI]
    [SKIP_CLANG_TIDY]
    [SHOW_DEBUG]
    [DEBUG_TAG <tag>]
)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `TARGET_NAME` | String | ✓ | CMake Target |
| `SKIP_STRICT_CONFORMANCE` | Flag | — | MSVC strict mode überspringen |
| `SKIP_NOMINMAX` | Flag | — | min/max Makros erlauben |
| `FORCE_EXCEPTIONS` | Flag | — | Exceptions erzwingen (ignoriert `NO_EXCEPTIONS`) |
| `FORCE_RTTI` | Flag | — | RTTI erzwingen (ignoriert `NO_RTTI`) |
| `SKIP_CLANG_TIDY` | Flag | — | Clang-Tidy überspringen |
| `SHOW_DEBUG` | Flag | — | Debug-Ausgaben aktivieren |
| `DEBUG_TAG` | String | — | Custom Tag für Debug-Ausgaben |

**Rückgabe:** Keine (modifiziert Target-Properties)

**Fehler:**

| Code | Bedingung |
|------|-----------|
| W201 | `ENABLE_CLANG_TIDY=ON` aber clang-tidy nicht gefunden |

**Beispiel:**

```cmake
apply_compiler_options(MyApp)
apply_compiler_options(LegacyLib SKIP_STRICT_CONFORMANCE FORCE_EXCEPTIONS)
```

---

## 5. Verwendungsbeispiele

### 5.1 Basis-Verwendung

```cmake
apply_compiler_options(MyTarget)
```

**Resultat (MSVC):**
- `/permissive-`
- `/Zc:preprocessor`
- `/Zc:__cplusplus`
- `NOMINMAX`

### 5.2 Legacy-Code

```cmake
# Strikte Konformität würde Legacy-Code brechen
apply_compiler_options(LegacyApp SKIP_STRICT_CONFORMANCE)
```

### 5.3 Win32 API

```cmake
# Win32 API nutzt min/max Makros
apply_compiler_options(Win32App SKIP_NOMINMAX)
```

### 5.4 Externe Library mit Exceptions

```cmake
# Globales NO_EXCEPTIONS=ON, aber diese Library braucht Exceptions
apply_compiler_options(ThirdPartyLib FORCE_EXCEPTIONS)
```

### 5.5 Tests ohne Clang-Tidy

```cmake
# Clang-Tidy würde Tests langsamer machen
apply_compiler_options(SlowTests SKIP_CLANG_TIDY)
```

### 5.6 Kombiniert

```cmake
apply_compiler_options(SpecialApp
    SKIP_STRICT_CONFORMANCE
    FORCE_EXCEPTIONS
    SKIP_CLANG_TIDY
)
```

---

## 6. Cache-Variablen

### 6.1 Globale Optionen

| Variable | Typ | Default | Beschreibung |
|----------|-----|---------|--------------|
| `ENABLE_STRICT_CONFORMANCE` | BOOL | ON | MSVC `/permissive-` |
| `NO_EXCEPTIONS` | BOOL | OFF | `-fno-exceptions` / `/EHs-c-` global |
| `NO_RTTI` | BOOL | OFF | `-fno-rtti` / `/GR-` global |

### 6.2 Code-Qualität

| Variable | Typ | Default | Beschreibung |
|----------|-----|---------|--------------|
| `ENABLE_CLANG_TIDY` | BOOL | OFF | Clang-Tidy aktivieren |
| `CLANG_TIDY_STRICT` | BOOL | OFF | Warnings = Errors |
| `ENABLE_CLANG_FORMAT_CHECK` | BOOL | OFF | Format-Check (Phase 7) |

---

## 7. Plattform-Details

### 7.1 Windows

| Compiler | Erkannt als | Optionen |
|----------|-------------|----------|
| MSVC | `MSVC` | `/permissive-`, `/Zc:*`, `NOMINMAX` |
| MinGW GCC | `GNU` + `WIN32` | GCC-Standard |
| Clang-CL | `Clang` + `WIN32` | Wie MSVC |

### 7.2 Linux

| Compiler | Erkannt als | Optionen |
|----------|-------------|----------|
| GCC | `GNU` | GCC-Standard |
| Clang | `Clang` | Clang-Standard |

### 7.3 macOS

| Compiler | Erkannt als | Optionen |
|----------|-------------|----------|
| Apple Clang | `Clang` + `APPLE` | Apple-spezifisch |
| Homebrew GCC | `GNU` + `APPLE` | GCC-Standard |

---

## 8. Best Practices

1. **Default = Strict** — Nur bei Bedarf überspringen
2. **Globale Defaults via Cache** — `-DNO_EXCEPTIONS=ON`
3. **Lokale Overrides sparsam** — Nur wo nötig
4. **Clang-Tidy nur für Quality-Checks** — Zu langsam für tägliche Builds
5. **.clang-tidy im Projekt-Root** — Wird automatisch verwendet

---

## 9. Performance-Impact

### 9.1 Clang-Tidy

| Konfiguration | Build-Zeit | Faktor |
|---------------|------------|--------|
| Ohne | 1:00 min | 1.0x |
| Mit | 3:30 min | 3.5x |

### 9.2 NO_EXCEPTIONS/NO_RTTI

| Konfiguration | Binary Size | Performance |
|---------------|-------------|-------------|
| Standard | 2.4 MB | Baseline |
| NO_EXCEPTIONS | 2.1 MB (-12%) | +2% |
| NO_RTTI | 2.3 MB (-4%) | +1% |
| Beide | 2.0 MB (-17%) | +3% |

---

## 10. Siehe auch

- [Warnings.cmake](Warnings.md) — Warning-Level
- [Debug.cmake](Debug.md) — Debug-System
- [Errors.cmake](Errors.md) — Fehlerbehandlung
- [guidelines](../../../concepts/guidelines.md) — Konventionen
- [CMakePresets_Reference](../../../reference/CMakePresets_Reference.md) — Presets

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung, korrigierte Abhängigkeiten** |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Inhalte aus v2.0 übernommen, Blueprint-Format, Per-Target Overrides |
