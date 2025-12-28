# PostFetch/qt-ads.cmake — Qt-ADS PostFetch Hook

> **Version:** 1.1.0  
> **Datum:** 2025-12-27  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Qt-Ads_PostFetch.md](../../../../en/modules/externals/hooks/postfetch/Qt-Ads.md)  
> **Hook:** [cmake/externals/hooks/postfetch/qt-ads.cmake](../../../../../../cmake/externals/hooks/postfetch/qt-ads.cmake)  
> **Modul-Version:** 1.1.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PostFetch Hook?](#2-warum-ein-postfetch-hook)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Target-Erkennung](#4-target-erkennung)
5. [Deployment (v1.1.0)](#5-deployment-v110)
6. [Solution.json Konfiguration](#6-solutionjson-konfiguration)
7. [Verwendung in Executables/Apps](#7-verwendung-in-executablesapps)
8. [Abhängigkeiten](#8-abhängigkeiten)
9. [Fehlerbehandlung](#9-fehlerbehandlung)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Der `qt-ads.cmake` PostFetch Hook registriert das Qt Advanced Docking System Target in der External-Registry, da Qt-ADS ein nicht-standardmäßiges Target-Naming verwendet.

**Ab v1.1.0:** Der Hook registriert nur noch das Target. DLL-Kopier-Logik wurde entfernt, da qt-ads nun als **statische Library** gebaut wird (siehe PreFetch Hook).

---

## 2. Warum ein PostFetch Hook?

Qt-ADS erstellt Targets mit dem Namen `qt6advanceddocking` (Qt6) bzw. `qtadvanceddocking` (Qt5), nicht `qt-ads`. Das Build-System kann diese Targets ohne Hook nicht automatisch erkennen.

**Ohne Hook:** `[E201] Fetched external 'qt-ads': No target in registry`

**Mit Hook:** Target wird korrekt registriert und kann verwendet werden.

---

## 3. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `qt6advanceddocking` | **STATIC** | Qt6 Advanced Docking System Library |
| `qtadvanceddocking` | **STATIC** | Qt5 Fallback (wenn Qt6 nicht verfügbar) |

**Hinweis:** Seit v1.1.0 werden die Targets als **STATIC** Library gebaut (konfiguriert im PreFetch Hook).

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

## 5. Deployment (v1.1.0)

### Vorherige Versionen (v1.0.x)

In früheren Versionen enthielt der PostFetch Hook eine `POST_LINK` oder `POST_BUILD` Funktion zum Kopieren der qt-ads DLL:

```cmake
# ALT (v1.0.x) - ENTFERNT in v1.1.0
function(_qt_ads_post_link_hook CONSUMER_TARGET)
    add_custom_command(TARGET ${CONSUMER_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qt6advanceddocking>
            $<TARGET_FILE_DIR:${CONSUMER_TARGET}>
    )
endfunction()
```

### Aktuelle Version (v1.1.0)

**Keine DLL-Kopie mehr nötig!**

Da qt-ads nun als **statische Library** gebaut wird (`BUILD_STATIC=ON` im PreFetch Hook), gibt es keine DLL zum Kopieren. Die Library wird direkt in die Executable eingelinkt.

**Vorteile:**
- ✅ Kein kompliziertes Timing (PRE_LINK vs POST_BUILD)
- ✅ Kein Konflikt mit windeployqt
- ✅ Einfacheres Deployment

---

## 6. Solution.json Konfiguration

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

## 7. Verwendung in Executables/Apps

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

## 8. Abhängigkeiten

| External | Typ | Beschreibung |
|----------|-----|--------------|
| `Qt6` | System | Qt6 Core, Widgets, Gui (muss vorher geladen sein) |

### Reihenfolge in Solution.json

Qt6 **muss vor** qt-ads in der alphabetischen Sortierung stehen:
- ✅ `Qt6` → `qt-ads` (Großbuchstabe Q vor Kleinbuchstabe q)
- ❌ `qt6` → `qt-ads` (beide mit kleinem q, falsche Reihenfolge)

---

## 9. Fehlerbehandlung

| Situation | Verhalten |
|-----------|-----------|
| Qt6 Target gefunden | Registrierung erfolgreich |
| Qt5 Target gefunden | Fallback-Registrierung |
| Kein Target gefunden | Warnung ausgegeben |

### Mögliche Warnung

```
[qt-ads] No target found (qt6advanceddocking or qtadvanceddocking)
```

**Ursache:** Qt wurde nicht gefunden oder qt-ads konnte nicht kompiliert werden.

**Lösung:** Sicherstellen dass Qt6 vor qt-ads geladen wird (siehe Reihenfolge).

### windeployqt-Fehler

Falls windeployqt immer noch Fehler wirft:

```
Unable to find dependent libraries of ... qt6advanceddockingd.dll
```

**Ursache:** qt-ads wird als DLL gebaut (PreFetch Hook nicht aktiv).

**Lösung:** 
1. Cache löschen: `rd /s /q .externals\qt-ads out\build\*`
2. Prüfen ob PreFetch Hook `BUILD_STATIC: ON` meldet
3. CMake neu konfigurieren

---

## 10. Siehe auch

- [Qt-Ads_PreFetch.md](../prefetch/Qt-Ads.md) — PreFetch Hook (Build-Optionen, **DLL-Problem-Lösung**)
- [HookLoader.md](../../hooks/HookLoader_cmake.md) — Hook-System
- [Targets.md](../../registry/Targets_cmake.md) — Target-Registrierung
- [Qt6.md](../../system/packages/Qt6.md) — Qt6 System External

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-27** | **Entfernt: POST_LINK Hook für DLL-Kopie (nicht mehr nötig)** |
| | | **Aktualisiert: Dokumentation für statische Library** |
| 1.0.0 | 2025-12-21 | Initial: PostFetch Hook für Qt-ADS Target-Registrierung |
