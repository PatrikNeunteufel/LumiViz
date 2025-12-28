# PostFetch/qt-ads.cmake — Qt-ADS PostFetch Hook

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Qt-Ads_PostFetch.md](../../../../en/modules/externals/hooks/postfetch/Qt-Ads.md)  
> **Hook:** [cmake/externals/hooks/postfetch/qt-ads.cmake](../../../../../../cmake/externals/hooks/postfetch/qt-ads.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Warum ein PostFetch Hook?](#2-warum-ein-postfetch-hook)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Target-Erkennung](#4-target-erkennung)
5. [Solution.json Configuration](#5-solutionjson-konfiguration)
6. [Usage in Executables/Apps](#6-verwendung-in-executablesapps)
7. [Dependencies](#7-abhängigkeiten)
8. [Errorbehandlung](#8-fehlerbehandlung)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

Der `qt-ads.cmake` PostFetch Hook registriert das Qt Advanced Docking System Target in der External-Registry, da Qt-ADS ein nicht-standardmäßiges Target-Naming verwendet.

---

## 2. Warum ein PostFetch Hook?

Qt-ADS erstellt Targets mit dem Namen `qt6advanceddocking` (Qt6) bzw. `qtadvanceddocking` (Qt5), nicht `qt-ads`. Das Build-System kann diese Targets ohne Hook nicht automatisch erkennen.

**Ohne Hook:** `[E201] Fetched external 'qt-ads': No target in registry`

**Mit Hook:** Target wird korrekt registriert und kann verwendet werden.

---

## 3. Erstellte Targets

| Target | Typ | Description |
|--------|-----|--------------|
| `qt6advanceddocking` | STATIC | Qt6 Advanced Docking System Library |
| `qtadvanceddocking` | STATIC | Qt5 Fallback (wenn Qt6 nicht verfügbar) |

---

## 4. Target-Erkennung

Der Hook prüft beide möglichen Target-Namen:

```cmake
if(TARGET qt6advanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qt6advanceddocking" PRIMARY)
elseif(TARGET qtadvanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qtadvanceddocking" PRIMARY)
endif()
```

Dadurch funktioniert der Hook sowohl mit Qt6 als auch mit Qt5.

---

## 5. Solution.json Configuration

```json
{
    "externals": {
        "Qt6": {
            "system": true,
            "package": "Qt6",
            "components": ["Core", "Widgets", "Gui"],
            "hints": ["${QT_ROOT}"]
        },
        "qt-ads": {
            "git": "https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git",
            "tag": "4.3.1"
        }
    }
}
```

---

## 6. Usage in Executables/Apps

### In Executable

```json
{
    "executables": [{
        "name": "MyDockingApp",
        "type": "GUI",
        "externals": ["Qt6", "qt-ads"]
    }]
}
```

### In App-Container

```json
{
    "apps": [{
        "name": "MyVisualizer",
        "core": {
            "externals": ["Qt6", "qt-ads"]
        },
        "runner": {
            "type": "GUI",
            "externals": ["Qt6"]
        }
    }]
}
```

---

## 7. Dependencies

| External | Typ | Description |
|----------|-----|--------------|
| `Qt6` | System | Qt6 Core, Widgets, Gui (muss vorher geladen sein) |

### Reihenfolge in Solution.json

Qt6 **muss vor** qt-ads in der alphabetischen Sortierung stehen:
- ✅ `Qt6` → `qt-ads` (Großbuchstabe Q vor Kleinbuchstabe q)
- ❌ `qt6` → `qt-ads` (beide mit kleinem q, falsche Reihenfolge)

---

## 8. Errorbehandlung

| Situation | Verhalten |
|-----------|-----------|
| Qt6 Target gefunden | Registrierung erfolgreich |
| Qt5 Target gefunden | Fallback-Registrierung |
| Kein Target gefunden | Warning ausgegeben |

### Mögliche Warning

```
[qt-ads] No target found (qt6advanceddocking or qtadvanceddocking)
```

**Ursache:** Qt wurde nicht gefunden oder qt-ads konnte nicht kompiliert werden.

**Lösung:** Sicherstellen dass Qt6 vor qt-ads geladen wird (siehe Reihenfolge).

---

## 9. See Also

- [Qt-Ads_PreFetch.md](../prefetch/Qt-Ads.md) — PreFetch Hook (Optionen)
- [HookLoader.md](../../hooks/HookLoader_cmake.md) — Hook-System
- [Targets.md](../../registry/Targets_cmake.md) — Target-Registrierung
- [Qt6.md](../../system/packages/Qt6.md) — Qt6 System External

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-21** | **Initial: PostFetch Hook für Qt-ADS Target-Registrierung** |
