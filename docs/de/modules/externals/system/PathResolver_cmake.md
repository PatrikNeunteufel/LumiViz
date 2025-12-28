# PathResolver.cmake — System External Path Resolution

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [PathResolver_cmake.md](../../../../en/modules/externals/system/PathResolver_cmake.md)  
> **Modul:** [cmake/externals/system/PathResolver.cmake](../../../../../cmake/externals/system/PathResolver.cmake)  
> **Modul-Version:** 1.0.0  
> **Phase:** 9 (System Externals)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Suchpriorität](#4-suchpriorität)
5. [Umgebungsvariablen](#5-umgebungsvariablen)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Debug-Ausgaben](#7-debug-ausgaben)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

`PathResolver.cmake` ist für die **mehrstufige Pfadauflösung** bei System Externals zuständig. Es durchsucht verschiedene Quellen nach Installationspfaden, bevor `find_package()` aufgerufen wird.

### Kernfunktionen

- **ENV-Variablen** — Sucht nach `{PACKAGE}_ROOT`, `{PACKAGE}_DIR`, etc.
- **hints[]** — Verwendet Pfade aus Solution.json
- **Standard-Pfade** — Paket-spezifische Installationsorte
- **Backup** — Fallback-Pfad mit Warnung

### Architektur-Position

```
system/Handler.cmake
       │
       ├── _resolve_system_path()
       ▼
┌─────────────────────┐
│    PathResolver     │  ← Dieses Modul
└─────────────────────┘
       │
       ├── 1. ENV-Variablen
       ├── 2. hints[]
       ├── 3. Package-Hook Pfade
       └── 4. backup
```

---

## 2. Abhängigkeiten

| Modul | Zweck |
|-------|-------|
| `Debug.cmake` | `dbg` für Debug-Ausgaben |

---

## 3. API-Referenz

### 3.1 _resolve_system_path()

Mehrstufige Pfadauflösung für System-Pakete.

```cmake
_resolve_system_path(
    EXT_NAME 
    PACKAGE 
    HINTS 
    ADDITIONAL_PATHS 
    BACKUP 
    OUT_PATH 
    OUT_IS_BACKUP
)
```

**Parameter:**

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals (für Debug) |
| `PACKAGE` | String | Paketname (z.B. "Qt6", "Boost") |
| `HINTS` | List | Hint-Pfade aus Solution.json |
| `ADDITIONAL_PATHS` | List | Pfade vom Package-Hook |
| `BACKUP` | String | Backup-Pfad (optional) |
| `OUT_PATH` | Var | Ausgabe: Aufgelöster Pfad |
| `OUT_IS_BACKUP` | Var | Ausgabe: TRUE wenn Backup verwendet |

**Rückgabe:**
- `OUT_PATH` — Aufgelöster Pfad oder leer
- `OUT_IS_BACKUP` — TRUE wenn Backup-Pfad verwendet wurde

---

## 4. Suchpriorität

Die Suche erfolgt in fester Reihenfolge — der erste existierende Pfad gewinnt:

```
┌─────────────────────────────────────────────────────────────┐
│                    Suchpriorität                             │
├─────────────────────────────────────────────────────────────┤
│  1. ENV-Variablen                                           │
│     ├── ${PACKAGE}_ROOT    (z.B. Qt6_ROOT)                  │
│     ├── ${PACKAGE}_DIR     (z.B. Qt6_DIR)                   │
│     ├── ${PACKAGE}_HOME    (z.B. Qt6_HOME)                  │
│     └── ${BASE}_ROOT       (z.B. QT_ROOT für Qt6)           │
│                                                             │
│  2. hints[] aus Solution.json                               │
│     └── Mit ${VAR} Expansion                                │
│                                                             │
│  3. Standard-Pfade vom Package-Hook                         │
│     └── _get_{Package}_standard_paths()                     │
│                                                             │
│  4. backup aus Solution.json                                │
│     └── Löst W501 Warnung aus                               │
│                                                             │
│  5. Leer (find_package() Defaults)                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 5. Umgebungsvariablen

### Unterstützte Muster

Für ein Paket `Qt6` werden folgende ENV-Variablen geprüft:

| Variable | Beispiel |
|----------|----------|
| `Qt6_ROOT` | `C:/Qt/6.8.0/msvc2022_64` |
| `Qt6_DIR` | `C:/Qt/6.8.0/msvc2022_64` |
| `Qt6_HOME` | `C:/Qt/6.8.0/msvc2022_64` |
| `QT_ROOT` | (ohne Versionsnummer) |
| `QT_DIR` | (ohne Versionsnummer) |
| `QT_HOME` | (ohne Versionsnummer) |

### Beispiel: Qt6

```bash
# Eines davon setzen:
export QT_ROOT="C:/Qt/6.8.0/msvc2022_64"
export Qt6_ROOT="C:/Qt/6.8.0/msvc2022_64"
```

### Beispiel: Boost

```bash
export BOOST_ROOT="C:/local/boost_1_84_0"
export Boost_ROOT="C:/local/boost_1_84_0"
```

---

## 6. Verwendungsbeispiele

### In Handler.cmake (automatisch)

```cmake
_resolve_system_path(
    "${EXT_NAME}"           # "qt6"
    "${_package}"           # "Qt6"
    "${_hints}"             # aus Solution.json
    "${_additional_paths}"  # vom Package-Hook
    "${_backup}"            # Fallback
    _resolved_path          # Ausgabe
    _used_backup            # Ausgabe
)

if(_used_backup)
    cmake_warn("W501" "Using backup location...")
endif()
```

### hints[] mit Variablen-Expansion

```json
{
    "qt6": {
        "system": true,
        "package": "Qt6",
        "hints": [
            "${QT_ROOT}",
            "${HOME}/Qt/6.8.0/gcc_64"
        ]
    }
}
```

Die `${VAR}` Syntax wird zu `$ENV{VAR}` expandiert.

### backup für Entwickler-Maschinen

```json
{
    "qt6": {
        "system": true,
        "package": "Qt6",
        "hints": ["${QT_ROOT}"],
        "backup": "C:/Qt/6.8.0/msvc2022_64"
    }
}
```

**Wichtig:** `backup` löst immer W501 aus — für CI/CD sollte die ENV-Variable gesetzt werden.

---

## 7. Debug-Ausgaben

### Debug-ID: `EXTERNALS`

| Level | Ausgabe |
|-------|---------|
| `DBG_RARE` | Found via ENV {VAR}: {path} |
| `DBG_RARE` | Found via hint: {path} |
| `DBG_RARE` | Found at standard path: {path} |
| `DBG_RARE` | Found at BACKUP: {path} |
| `DBG_RARE` | No path found, relying on find_package() defaults |
| `DBG_ULTRA_RARE` | ENV {VAR} set but doesn't exist |
| `DBG_ULTRA_RARE` | Hint references undefined ENV |

### Aktivierung

```bash
cmake -DDEBUG_CATEGORIES="EXTERNALS" -DDEBUG_LEVEL=3 ..
```

---

## 8. Siehe auch

- [Handler_cmake.md](Handler_cmake.md) — Verwendet PathResolver
- [packages/Qt6_cmake.md](packages/Qt6_cmake.md) — Qt6 Standard-Pfade
- [packages/Boost_cmake.md](packages/Boost_cmake.md) — Boost Standard-Pfade
- [Solution_Schema.md](../../../references/Solution_Schema.md) — hints/backup Syntax

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.6.0** | **2025-12-18** | **Initial: Mehrstufige Pfadauflösung, ENV-Expansion, ${VAR} Syntax in hints** |
