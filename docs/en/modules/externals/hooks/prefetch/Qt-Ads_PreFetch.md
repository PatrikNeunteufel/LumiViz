# PreFetch/qt-ads.cmake — Qt-ADS PreFetch Hook

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Qt-Ads_PreFetch.md](../../../../en/modules/externals/hooks/prefetch/Qt-Ads.md)  
> **Hook:** [cmake/externals/hooks/prefetch/qt-ads.cmake](../../../../../../cmake/externals/hooks/prefetch/qt-ads.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Warum ein PreFetch Hook?](#2-warum-ein-prefetch-hook)
3. [Gesetzte Optionen](#3-gesetzte-optionen)
4. [Solution.json Configuration](#4-solutionjson-konfiguration)
5. [Dependencies](#5-abhängigkeiten)
6. [See Also](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Overview

Der `qt-ads.cmake` PreFetch Hook konfiguriert das Qt Advanced Docking System vor dem Build, um unnötige Komponenten zu deaktivieren und eine statische Library zu erzeugen.

---

## 2. Warum ein PreFetch Hook?

**Ohne Hook:** 
- Examples und Demo-Targets werden gebaut (10+ zusätzliche Targets)
- Shared Library wird erzeugt (DLL-Deployment nötig)

**Mit Hook:** 
- Nur `qt6advanceddocking` Target
- Statische Library (einfacheres Deployment)

---

## 3. Gesetzte Optionen

| Option | Wert | Description |
|--------|------|--------------|
| `ADS_BUILD_EXAMPLES` | `OFF` | Keine Example-Programme |
| `BUILD_EXAMPLES` | `OFF` | Fallback für ältere Versionen |
| `ADS_BUILD_STATIC` | `ON` | Statische Library statt Shared |

---

## 4. Solution.json Configuration

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

### Important: Reihenfolge

Qt6 muss **vor** qt-ads verarbeitet werden. Dies wird durch die alphabetische Sortierung sichergestellt:
- `Qt6` (Q=81 ASCII) → wird zuerst verarbeitet
- `qt-ads` (q=113 ASCII) → wird danach verarbeitet

---

## 5. Dependencies

| External | Typ | Description |
|----------|-----|--------------|
| `Qt6` | System | Qt6 Core, Widgets, Gui müssen vorher geladen sein |

---

## 6. See Also

- [Qt-Ads_PostFetch.md](../postfetch/Qt-Ads.md) — PostFetch Hook (Target-Registrierung)
- [HookLoader.md](../../hooks/HookLoader_cmake.md) — Hook-System
- [Qt6.md](../../system/packages/Qt6.md) — Qt6 System External

---

## 7. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-21** | **Initial: PreFetch Hook für Qt-ADS** |
