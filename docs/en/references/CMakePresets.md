# CMakePresets — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [CMakePresets_Reference.md](../../en/references/CMakePresets_Reference.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Configure Presets](#3-configure-presets)
4. [Build Presets](#4-build-presets)
5. [Test Presets](#5-test-presets)
6. [Package Presets](#6-package-presets)
7. [Workflow Presets](#7-workflow-presets)
8. [Hidden Building Blocks](#8-hidden-building-blocks)
9. [Cache-Variablen](#9-cache-variablen)
10. [Schnellreferenz](#10-schnellreferenz)
11. [Usage](#11-verwendung)
12. [See Also](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Overview

This reference documents alle verfügbaren Team-Presets in `CMakePresets.json` für das CMake Architecture Build-System.

### Preset-Kategorien

| Kategorie | Anzahl | Description |
|-----------|--------|--------------|
| Configure Presets | 35+ | Build-Umgebung konfigurieren |
| Build Presets | 20+ | Kompilieren |
| Test Presets | 12 | CTest ausführen |
| Package Presets | 3 | Installer erstellen |
| Workflow Presets | 5 | Komplette Pipelines |

### Vendor-Metadaten

```json
{
    "version": 6,
    "vendor": {
        "cmake-architecture-v2": {
            "version": "0.5.0",
            "date": "2025-12-14",
            "description": "Team-weite Presets für CMake Architecture"
        }
    }
}
```

---

## 2. Konventionen

### Namensschema

```
[plattform]-[generator]-[arch]-[config]_[variante]

plattform: windows, linux, macos
generator: vs, ninja, xcode
arch:      x64, ARM64, x86_64, arm64
config:    debug, release, testing
variante:  dynamic, static, asan, quality
```

### Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ✓ | Aktiviert/Vorhanden |
| – | Standard/Nicht gesetzt |

---

## 3. Configure Presets

### 3.1 Windows • Visual Studio

| Preset | Arch | Config | CRT | Besonderheit |
|--------|------|--------|-----|--------------|
| `windows-vs-x64-debug_dynamic` | x64 | Debug | /MDd | Standard-Entwicklung |
| `windows-vs-x64-release_dynamic` | x64 | Release | /MD | Release-Builds |
| `windows-vs-x64-testing_dynamic` | x64 | Testing | /MD | Tests aktiviert |
| `windows-vs-ARM64-release_dynamic` | ARM64 | Release | /MD | ARM64-Builds |
| `windows-vs-x64-debug_clangcl_asan` | x64 | Debug | – | Clang-CL + ASan |
| `windows-vs-x64-debug_quality` | x64 | Debug | /MDd | Clang-Tidy aktiviert |
| `windows-vs-x64-debug_apps-only` | x64 | Debug | /MDd | BUILD_TESTS=OFF |

### 3.2 Windows • Ninja (Single-Config)

| Preset | Compiler | Config | Besonderheit |
|--------|----------|--------|--------------|
| `windows-ninja-debug-msvc` | MSVC | Debug | – |
| `windows-ninja-release-msvc` | MSVC | Release | – |
| `windows-ninja-testing-msvc` | MSVC | Testing | Tests aktiviert |
| `windows-ninja-debug-clang` | Clang | Debug | – |
| `windows-ninja-release-clang` | Clang | Release | – |
| `windows-ninja-testing-clang` | Clang | Testing | Tests aktiviert |

### 3.3 Windows • Ninja (Multi-Config)

| Preset | Configs | Besonderheit |
|--------|---------|--------------|
| `windows-ninja-multi` | Debug, Release, Testing | Standard |
| `windows-ninja-multi-tests` | Debug, Release, Testing | ENABLE_TESTING_CONFIG=ON |

### 3.4 Linux • Ninja

| Preset | Compiler | Config | Besonderheit |
|--------|----------|--------|--------------|
| `linux-gcc-debug` | GCC | Debug | – |
| `linux-gcc-release` | GCC | Release | – |
| `linux-gcc-testing` | GCC | Testing | Tests aktiviert |
| `linux-clang-debug` | Clang | Debug | – |
| `linux-clang-release` | Clang | Release | – |
| `linux-clang-testing` | Clang | Testing | Tests aktiviert |
| `linux-clang-debug-asan` | Clang | Debug | AddressSanitizer |

### 3.5 macOS

| Preset | Generator | Arch | Config |
|--------|-----------|------|--------|
| `macos-xcode-arm64` | Xcode | ARM64 | Multi (Debug, Release) |
| `macos-xcode-x86_64` | Xcode | x86_64 | Multi (Debug, Release) |
| `macos-ninja-debug` | Ninja | – | Debug |
| `macos-ninja-testing` | Ninja | – | Testing |

---

## 4. Build Presets

### 4.1 Windows • Visual Studio

| Preset | Configure Preset | Config |
|--------|------------------|--------|
| `build-vs-x64-Debug` | windows-vs-x64-debug_dynamic | Debug |
| `build-vs-x64-Release` | windows-vs-x64-release_dynamic | Release |
| `build-vs-x64-Testing` | windows-vs-x64-testing_dynamic | Testing |
| `build-vs-ARM64-Release` | windows-vs-ARM64-release_dynamic | Release |

### 4.2 Windows • Ninja (Single)

| Preset | Configure Preset |
|--------|------------------|
| `build-ninja-debug-msvc` | windows-ninja-debug-msvc |
| `build-ninja-release-msvc` | windows-ninja-release-msvc |
| `build-ninja-testing-msvc` | windows-ninja-testing-msvc |
| `build-ninja-debug-clang` | windows-ninja-debug-clang |
| `build-ninja-release-clang` | windows-ninja-release-clang |
| `build-ninja-testing-clang` | windows-ninja-testing-clang |

### 4.3 Windows • Ninja (Multi)

| Preset | Configure Preset | Config |
|--------|------------------|--------|
| `build-ninja-multi-Debug` | windows-ninja-multi | Debug |
| `build-ninja-multi-Release` | windows-ninja-multi | Release |
| `build-ninja-multi-Testing` | windows-ninja-multi-tests | Testing |

### 4.4 Linux

| Preset | Configure Preset |
|--------|------------------|
| `build-linux-gcc-Debug` | linux-gcc-debug |
| `build-linux-gcc-Release` | linux-gcc-release |
| `build-linux-gcc-Testing` | linux-gcc-testing |
| `build-linux-clang-Debug` | linux-clang-debug |
| `build-linux-clang-Release` | linux-clang-release |
| `build-linux-clang-Testing` | linux-clang-testing |
| `build-linux-clang-Debug-asan` | linux-clang-debug-asan |

### 4.5 macOS

| Preset | Configure Preset | Config |
|--------|------------------|--------|
| `build-macos-xcode-Debug` | macos-xcode-arm64 | Debug |
| `build-macos-xcode-Release` | macos-xcode-arm64 | Release |
| `build-macos-ninja-Debug` | macos-ninja-debug | – |
| `build-macos-ninja-Testing` | macos-ninja-testing | – |

---

## 5. Test Presets

### 5.1 Windows

| Preset | Configure Preset | Config |
|--------|------------------|--------|
| `ctest-vs-x64-Debug` | windows-vs-x64-debug_dynamic | Debug |
| `ctest-vs-x64-Testing` | windows-vs-x64-testing_dynamic | Testing |
| `ctest-ninja-multi-Debug` | windows-ninja-multi | Debug |
| `ctest-ninja-multi-Testing` | windows-ninja-multi-tests | Testing |

### 5.2 Linux

| Preset | Configure Preset |
|--------|------------------|
| `ctest-linux-gcc-Testing` | linux-gcc-testing |
| `ctest-linux-gcc-Release` | linux-gcc-release |
| `ctest-linux-clang-Testing` | linux-clang-testing |
| `ctest-linux-clang-Debug-asan` | linux-clang-debug-asan |

### 5.3 macOS

| Preset | Configure Preset | Config |
|--------|------------------|--------|
| `ctest-macos-ninja-Testing` | macos-ninja-testing | – |
| `ctest-macos-xcode-Release` | macos-xcode-arm64 | Release |

---

## 6. Package Presets

| Preset | Generator | Plattform | Config |
|--------|-----------|-----------|--------|
| `package-windows-inno-Release` | INNOSETUP | Windows | Release |
| `package-macos-dmg-Release` | External | macOS | Release |
| `package-linux-tgz-Release` | TGZ | Linux | Release |

---

## 7. Workflow Presets

| Preset | Schritte | Description |
|--------|----------|--------------|
| `wf-vs-x64-release-inno` | Configure → Build → Test → Package | Windows Release mit Inno Setup |
| `wf-linux-gcc-release-tgz` | Configure → Build → Test → Package | Linux Release als TGZ |
| `wf-macos-xcode-release-dmg` | Configure → Build → Test → Package | macOS Release als DMG |
| `wf-ninja-multi-testing` | Configure → Build → Test | Ninja Multi-Config Testing |
| `wf-linux-clang-debug-asan` | Configure → Build → Test | Linux mit AddressSanitizer |

---

## 8. Hidden Building Blocks

Diese Presets werden nur zur Vererbung verwendet (`hidden: true`).

### 8.1 CRT-Bausteine (Windows)

| Preset | CRT |
|--------|-----|
| `MultiThreaded_Static` | /MT bzw. /MTd |
| `MultiThreaded_Static_force_debug` | /MTd (immer) |
| `MultiThreaded_Dynamic` | /MD bzw. /MDd |
| `MultiThreaded_Dynamic_force_debug` | /MDd (immer) |

### 8.2 Plattform-Basen

| Preset | Description |
|--------|--------------|
| `windows-vs-base` | VS Generator, C++20, Architecture Variables |
| `windows-ninja-base` | Ninja, compile_commands.json |
| `linux-base` | Ninja, C++20 |
| `macos-base` | C++20, PIC |

### 8.3 Architecture-Basen

| Preset | Architecture |
|--------|-------------|
| `vs-Win32-base` | x86 |
| `vs-x64-base` | x64 |
| `vs-ARM-base` | ARM |
| `vs-ARM64-base` | ARM64 |

---

## 9. Cache-Variablen

Alle Configure Presets setzen diese Variablen:

| Variable | Default | Description |
|----------|---------|--------------|
| `BUILD_TESTS` | ON | Tests aktivieren |
| `BUILD_ONLY` | "" | Selektive Targets |
| `ENABLE_CLANG_TIDY` | OFF | Code-Qualität |
| `ENABLE_CLANG_FORMAT_CHECK` | OFF | Format-Prüfung |
| `ACTIVE_CONFIGURE_PRESET` | ${presetName} | Aktuelles Preset |

---

## 10. Schnellreferenz

### 10.1 Häufigste Configure Presets

| Anwendungsfall | Preset |
|----------------|--------|
| Windows Debug (IDE) | `windows-vs-x64-debug_dynamic` |
| Windows Release | `windows-vs-x64-release_dynamic` |
| Windows Tests | `windows-vs-x64-testing_dynamic` |
| Linux Debug | `linux-gcc-debug` |
| Linux Tests | `linux-gcc-testing` |
| macOS Debug | `macos-ninja-debug` |
| Sanitizer | `linux-clang-debug-asan` |
| Code-Qualität | `windows-vs-x64-debug_quality` |

### 10.2 Häufigste Workflows

| Anwendungsfall | Preset |
|----------------|--------|
| Windows Release + Installer | `wf-vs-x64-release-inno` |
| Linux Release + TGZ | `wf-linux-gcc-release-tgz` |
| Sanitizer-Tests | `wf-linux-clang-debug-asan` |

---

## 11. Usage

### 11.1 Configure

```bash
cmake --preset windows-vs-x64-debug_dynamic
```

### 11.2 Build

```bash
cmake --build --preset build-vs-x64-Debug
```

### 11.3 Test

```bash
ctest --preset ctest-vs-x64-Debug
```

### 11.4 Workflow

```bash
cmake --workflow --preset wf-vs-x64-release-inno
```

### 11.5 Presets auflisten

```bash
cmake --list-presets
cmake --list-presets=build
cmake --list-presets=test
```

---

## 12. See Also

- [CMakePresets_Manual.md](CMakePresets_Manual.md) — Concepte und Best Practices
- [CMakeUserPresets_Reference.md](CMakeUserPresets_Reference.md) — User-Presets
- [CMakeUserPresets_Example.md](../userguides/CMakeUserPresets.md) — Template

---

## 13. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Format: Nummeriertes TOC, Reference-Header, Schnellreferenz, Usage** |
| 0.1.0 | 2025-12-03 | Initial: Alle Team-Presets dokumentiert, Hidden Building Blocks, Variablen |
