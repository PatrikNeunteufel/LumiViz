# PreFetch/qt-ads.cmake — Qt-ADS PreFetch Hook

> **Version:** 1.1.0  
> **Datum:** 2025-12-27  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Qt-Ads_PreFetch.md](../../../../en/modules/externals/hooks/prefetch/Qt-Ads.md)  
> **Hook:** [cmake/externals/hooks/prefetch/qt-ads.cmake](../../../../../../cmake/externals/hooks/prefetch/qt-ads.cmake)  
> **Modul-Version:** 1.1.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PreFetch Hook?](#2-warum-ein-prefetch-hook)
3. [Gesetzte Optionen](#3-gesetzte-optionen)
4. [Das DLL-Problem und seine Lösung](#4-das-dll-problem-und-seine-lösung)
5. [Solution.json Konfiguration](#5-solutionjson-konfiguration)
6. [Abhängigkeiten](#6-abhängigkeiten)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Der `qt-ads.cmake` PreFetch Hook konfiguriert das Qt Advanced Docking System vor dem Build. Die wichtigste Funktion ist das Setzen von `BUILD_STATIC=ON`, um qt-ads als **statische Library** zu bauen und damit alle DLL-Deployment-Probleme zu vermeiden.

---

## 2. Warum ein PreFetch Hook?

**Ohne Hook:** 
- Examples und Demo-Targets werden gebaut (10+ zusätzliche Targets)
- **Shared Library (DLL)** wird erzeugt → windeployqt-Fehler!

**Mit Hook:** 
- Nur `qt6advanceddocking` Target
- **Statische Library** (kein DLL-Deployment nötig)
- windeployqt läuft fehlerfrei

---

## 3. Gesetzte Optionen

| Option | Wert | Beschreibung |
|--------|------|--------------|
| `BUILD_STATIC` | `ON` | **Statische Library** statt Shared (DLL) |
| `BUILD_EXAMPLES` | `OFF` | Keine Example-Programme |

### Wichtig: Korrekter Variablenname

Die Variable heißt **`BUILD_STATIC`**, nicht `ADS_BUILD_STATIC`!

```cmake
# RICHTIG:
set(BUILD_STATIC ON CACHE BOOL "Build qt-ads as static library" FORCE)

# FALSCH (funktioniert NICHT):
set(ADS_BUILD_STATIC ON CACHE BOOL "..." FORCE)
```

Siehe: [Qt-ADS CMakeLists.txt](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/blob/master/src/CMakeLists.txt)

---

## 4. Das DLL-Problem und seine Lösung

### 4.1 Das Problem

Wenn qt-ads als **Shared Library (DLL)** gebaut wird, tritt folgender Fehler auf:

```
Unable to find dependent libraries of C:\Qt\6.10.1\msvc2022_64\bin\qt6advanceddockingd.dll
Cannot open 'C:/Qt/6.10.1/msvc2022_64/bin/qt6advanceddockingd.dll': Das System kann die angegebene Datei nicht finden.
```

### 4.2 Ursache

1. **windeployqt** scannt die Executable nach DLL-Abhängigkeiten
2. Es findet die Abhängigkeit `qt6advanceddockingd.dll`
3. windeployqt sucht diese DLL im **Qt-bin-Verzeichnis** (`C:\Qt\...\bin\`)
4. Die DLL ist dort nicht vorhanden (sie wurde via FetchContent gebaut)
5. **windeployqt bricht ab** — auch die Qt-DLLs werden nicht kopiert!

### 4.3 Versuchte Lösungen (die NICHT funktionieren)

| Ansatz | Problem |
|--------|---------|
| DLL vor windeployqt kopieren | windeployqt sucht trotzdem im Qt-Verzeichnis |
| PATH erweitern | windeployqt ignoriert PATH für Dependency-Analyse |
| `--ignore-library-errors` Flag | Nicht in allen Qt-Versionen verfügbar |
| windeployqt-Fehler ignorieren | Qt-DLLs werden trotzdem nicht kopiert |

### 4.4 Die Lösung: Statische Library

Durch `BUILD_STATIC=ON` wird qt-ads als **statische Library** (`.lib`) gebaut:

| Aspekt | Shared (DLL) | Static (LIB) |
|--------|--------------|--------------|
| Output | `qt6advanceddockingd.dll` | `qt6advanceddockingd.lib` |
| Deployment | DLL muss kopiert werden | In Executable eingelinkt |
| windeployqt | ❌ Fehler | ✅ Kein Problem |
| Dateigröße EXE | Kleiner | Größer (~2-3 MB) |

**Vorteile der statischen Library:**
- ✅ Kein DLL-Kopieren nötig
- ✅ windeployqt läuft ohne Fehler
- ✅ Einfacheres Deployment (alles in einer EXE)
- ✅ Keine DLL-Hell-Probleme

---

## 5. Solution.json Konfiguration

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

### Wichtig: Reihenfolge

Qt6 muss **vor** qt-ads verarbeitet werden. Dies wird durch die alphabetische Sortierung sichergestellt:
- `Qt6` (Q=81 ASCII) → wird zuerst verarbeitet
- `qt-ads` (q=113 ASCII) → wird danach verarbeitet

---

## 6. Abhängigkeiten

| External | Typ | Beschreibung |
|----------|-----|--------------|
| `Qt6` | System | Qt6 Core, Widgets, Gui müssen vorher geladen sein |

---

## 7. Fehlerbehandlung

### Problem: qt-ads wird trotzdem als DLL gebaut

**Symptom:**
```
[27/45] Linking CXX shared library x64\bin\qt6advanceddockingd.dll
```

**Ursache:** CMake Cache enthält noch alte Werte.

**Lösung:**
```cmd
rd /s /q .externals\qt-ads
rd /s /q out\build\<preset-name>
```

Dann CMake neu konfigurieren.

### Problem: windeployqt-Fehler trotz PreFetch Hook

**Prüfen im CMake-Output:**
```
-- [qt-ads]   BUILD_STATIC: ON (static library)
```

Falls `ADS_BUILD_STATIC` statt `BUILD_STATIC` angezeigt wird, ist der Hook veraltet.

---

## 8. Siehe auch

- [Qt-Ads_PostFetch.md](../postfetch/Qt-Ads.md) — PostFetch Hook (Target-Registrierung)
- [HookLoader.md](../../hooks/HookLoader_cmake.md) — Hook-System
- [Qt6.md](../../system/packages/Qt6.md) — Qt6 System External

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-27** | **Fix: `BUILD_STATIC` statt `ADS_BUILD_STATIC` (korrekter Variablenname)** |
| | | **Neu: Ausführliche Dokumentation des DLL-Problems und der Lösung** |
| 1.0.0 | 2025-12-21 | Initial: PreFetch Hook für Qt-ADS |
