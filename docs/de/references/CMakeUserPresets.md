# CMakeUserPresets — Referenz

> **Version:** 1.0.0  
> **Datum:** 2025-12-14  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [CMakeUserPresets_Reference.md](../../en/references/CMakeUserPresets_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Hidden Building Blocks](#3-hidden-building-blocks)
4. [Configure Presets](#4-configure-presets)
5. [Build Presets](#5-build-presets)
6. [Test Presets](#6-test-presets)
7. [Package Presets](#7-package-presets)
8. [Workflow Presets](#8-workflow-presets)
9. [Schnellreferenz](#9-schnellreferenz)
10. [Verwendung](#10-verwendung)
11. [Anpassung](#11-anpassung)
12. [Siehe auch](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert die persönlichen User-Presets in `CMakeUserPresets.json`. User-Presets erweitern die Team-Presets um lokale Konfigurationen wie vcpkg-Pfade, parallele Builds und Code-Signing.

### Preset-Kategorien

| Kategorie | Anzahl | Beschreibung |
|-----------|--------|--------------|
| Configure Presets | 7 | vcpkg-Kombinationen |
| Build Presets | 6 | Parallelisierte Builds |
| Test Presets | 3 | CTest-Konfigurationen |
| Package Presets | 1 | Code-Signing |
| Workflow Presets | 1 | Release + Signing |

### Vendor-Metadaten

```json
{
    "version": 6,
    "vendor": {
        "user-presets": {
            "version": "0.5.0",
            "lastModified": "2025-12-14"
        }
    }
}
```

---

## 2. Konventionen

### Namensschema

```
[basis-preset]+[erweiterung]

Beispiele:
- windows-vs-x64-debug_dynamic+vcpkg
- ninjamulti_testing+vcpkg
```

### Symbole

| Symbol | Bedeutung |
|--------|-----------|
| + | Erweiterung eines Team-Presets |
| jobs=N | Parallele Build-Jobs |

---

## 3. Hidden Building Blocks

### 3.1 with-vcpkg

Basis-Preset für lokale vcpkg-Integration:

```json
{
    "name": "with-vcpkg",
    "hidden": true,
    "cacheVariables": {
        "VCPKG_ROOT": "H:/Dev/vcpkg",
        "CMAKE_TOOLCHAIN_FILE": "H:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_FEATURE_FLAGS": "manifests,versions"
    }
}
```

| Variable | Wert | Beschreibung |
|----------|------|--------------|
| `VCPKG_ROOT` | H:/Dev/vcpkg | Lokaler vcpkg-Pfad |
| `CMAKE_TOOLCHAIN_FILE` | .../vcpkg.cmake | Toolchain-Integration |
| `VCPKG_FEATURE_FLAGS` | manifests,versions | Moderne Features |

> **Hinweis:** Pfade anpassen an lokale Installation!

---

## 4. Configure Presets

### 4.1 vcpkg-Kombinationen

| Preset | Erbt von | Beschreibung |
|--------|----------|--------------|
| `windows-vs-x64-debug_dynamic+vcpkg` | windows-vs-x64-debug_dynamic, with-vcpkg | VS Debug + vcpkg |
| `windows-vs-x64-testing_dynamic+vcpkg` | windows-vs-x64-testing_dynamic, with-vcpkg | VS Testing + vcpkg |
| `windows-ninja-multi+vcpkg` | windows-ninja-multi, with-vcpkg | Ninja Multi + vcpkg |
| `windows-ninja-multi-tests+vcpkg` | windows-ninja-multi-tests, with-vcpkg | Ninja Tests + vcpkg |
| `linux-clang-debug-asan+vcpkg` | linux-clang-debug-asan, with-vcpkg | Linux ASan + vcpkg |
| `linux-gcc-testing+vcpkg` | linux-gcc-testing, with-vcpkg | Linux GCC + vcpkg |

---

## 5. Build Presets

### 5.1 Parallelisierte Builds (jobs=8)

| Preset | Configure Preset | Config | Jobs |
|--------|------------------|--------|------|
| `ninjamulti_debug` | windows-ninja-multi | Debug | 8 |
| `ninjamulti_testing` | windows-ninja-multi-tests | Testing | 8 |
| `vs_x64_debug_md` | windows-vs-x64-debug_dynamic | Debug | 8 |
| `vs_x64_testing_md` | windows-vs-x64-testing_dynamic | Testing | 8 |

### 5.2 vcpkg-Varianten

| Preset | Configure Preset | Config | Jobs |
|--------|------------------|--------|------|
| `ninjamulti_testing+vcpkg` | windows-ninja-multi-tests+vcpkg | Testing | 8 |
| `vs_x64_testing_md+vcpkg` | windows-vs-x64-testing_dynamic+vcpkg | Testing | 8 |

---

## 6. Test Presets

| Preset | Configure Preset | Config | Output |
|--------|------------------|--------|--------|
| `ninjamulti_test_testing` | windows-ninja-multi-tests | Testing | outputOnFailure |
| `vs_x64_ctest_debug` | windows-vs-x64-debug_dynamic | Debug | outputOnFailure |
| `vs_x64_ctest_testing` | windows-vs-x64-testing_dynamic | Testing | outputOnFailure |

---

## 7. Package Presets

### 7.1 Code-Signing Variante

```json
{
    "name": "package-windows-inno-Release+sign",
    "inherits": "package-windows-inno-Release",
    "variables": {
        "CPACK_INNOSETUP_EXECUTABLE_ARGUMENTS": "/Qp;/Smysigntool=$p"
    },
    "output": {
        "packageDirectory": "dist"
    }
}
```

| Einstellung | Wert | Beschreibung |
|-------------|------|--------------|
| Basis | package-windows-inno-Release | Team-Preset |
| ISCC Args | /Qp;/Smysigntool=$p | Quiet + Signtool |
| Output | dist/ | Lokales Ausgabeverzeichnis |

---

## 8. Workflow Presets

### 8.1 wf-vs-x64-release-inno+sign

Komplette Release-Pipeline mit Code-Signing:

| Schritt | Type | Preset |
|---------|------|--------|
| 1 | configure | windows-vs-x64-release_dynamic |
| 2 | build | build-vs-x64-Release |
| 3 | package | package-windows-inno-Release+sign |

---

## 9. Schnellreferenz

### 9.1 Häufigste Kombinationen

| Anwendungsfall | Configure | Build |
|----------------|-----------|-------|
| VS Debug + vcpkg | `windows-vs-x64-debug_dynamic+vcpkg` | `vs_x64_debug_md` |
| Ninja Testing + vcpkg | `windows-ninja-multi-tests+vcpkg` | `ninjamulti_testing+vcpkg` |
| Release + Signing | (Teil von Workflow) | `wf-vs-x64-release-inno+sign` |

### 9.2 User-Preset vs Team-Preset

| Aspekt | Team-Preset | User-Preset |
|--------|-------------|-------------|
| Datei | CMakePresets.json | CMakeUserPresets.json |
| Git | ✓ Committed | ✗ .gitignored |
| Lokale Pfade | ✗ | ✓ |
| vcpkg | ✗ | ✓ |
| Code-Signing | ✗ | ✓ |

---

## 10. Verwendung

### 10.1 Tägliche Entwicklung

```bash
# VS Debug mit vcpkg
cmake --preset windows-vs-x64-debug_dynamic+vcpkg

# Schneller Build (8 Jobs)
cmake --build --preset vs_x64_debug_md
```

### 10.2 Tests

```bash
# Ninja Testing
cmake --preset windows-ninja-multi-tests+vcpkg
cmake --build --preset ninjamulti_testing+vcpkg
ctest --preset ninjamulti_test_testing
```

### 10.3 Release mit Signing

```bash
cmake --workflow --preset wf-vs-x64-release-inno+sign
```

---

## 11. Anpassung

### 11.1 vcpkg-Pfad ändern

```json
{
    "name": "with-vcpkg",
    "hidden": true,
    "cacheVariables": {
        "VCPKG_ROOT": "/pfad/zu/vcpkg",
        "CMAKE_TOOLCHAIN_FILE": "/pfad/zu/vcpkg/scripts/buildsystems/vcpkg.cmake"
    }
}
```

### 11.2 Job-Anzahl ändern

```json
{
    "name": "my-fast-build",
    "inherits": "vs_x64_debug_md",
    "jobs": 16
}
```

### 11.3 Eigene Kombinationen

```json
{
    "name": "my-dev",
    "inherits": ["windows-vs-x64-debug_dynamic", "with-vcpkg"],
    "cacheVariables": {
        "BUILD_ONLY": "MyApp",
        "BUILD_TESTS": "OFF"
    }
}
```

---

## 12. Siehe auch

- [CMakePresets_Manual.md](../userguides/CMakePresets.md) — Konzepte
- [CMakePresets_Reference.md](CMakePresets.md) — Team-Presets
- [CMakeUserPresets_Example.md](../userguides/CMakeUserPresets.md) — Template

---

## 13. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Format: Nummeriertes TOC, Reference-Header, Schnellreferenz, Autor-Feld entfernt** |
| 0.1.0 | 2025-12-03 | Initial: vcpkg-Integration, parallelisierte Builds, Code-Signing |
