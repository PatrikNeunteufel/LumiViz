# Local/Attach.cmake — Lokale External Handler

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Attach_cmake.md](../../en/modules/externals/Attach_cmake.md)  
> **Modul:** [cmake/externals/Local/Attach.cmake](../../../cmake/externals/Local/Attach.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Include.cmake Konvention](#4-includecmake-konvention)
5. [Verwendungsbeispiele](#5-verwendungsbeispiele)
6. [Fehlerbehandlung](#6-fehlerbehandlung)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

`Local/Attach.cmake` verarbeitet lokale Externals — Libraries, die manuell im Projekt abgelegt werden (z.B. proprietäre Libraries wie BASS).

### Kernfunktionen

- **Path-Validierung** — Prüft ob External-Verzeichnis existiert
- **Include.cmake Lookup** — Convention oder Custom Path
- **Best-Practice-Checks** — Warnungen für problematische Patterns
- **Registrierung** — Speichert Pfade für spätere Verwendung

---

## 2. Abhängigkeiten

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Fehlerbehandlung (`cmake_fatal`, `cmake_warn`) |
| `Debug.cmake` | Debug-Ausgaben (`dbg`) |
| `Json.cmake` | JSON-Parsing |
| `Validation.cmake` | Source-Validierung |

---

## 3. API-Referenz

### 3.1 _attach_local_external()

```cmake
_attach_local_external(EXT_NAME EXT_JSON)
```

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals |
| `EXT_JSON` | JSON | JSON-Definition mit `path` Feld |

**Setzt Properties:**

| Property | Beschreibung |
|----------|--------------|
| `EXTERNAL_${NAME}_PATH` | Absoluter Pfad zum External |
| `EXTERNAL_${NAME}_INCLUDE` | Pfad zur Include.cmake |
| `EXTERNAL_${NAME}_REGISTERED` | TRUE wenn registriert |

---

### 3.2 is_external_registered()

```cmake
is_external_registered(EXT_NAME OUT_VAR)
```

Prüft ob ein External registriert ist.

---

## 4. Include.cmake Konvention

### Convention Path (Default)

```
cmake/externals/includes/{name}/Include.cmake
```

**Beispiel:** External `bass` → `cmake/externals/includes/bass/Include.cmake`

### Custom Include Path (optional)

```json
{
    "bass": {
        "path": "externals/bass",
        "include": "cmake/custom/bass_special.cmake"
    }
}
```

> **Hinweis:** Das `include` Feld ist optional. Ohne Angabe wird der Convention Path verwendet.

---

## 5. Verwendungsbeispiele

### Minimal (Convention)

```json
{
    "bass": {
        "path": "externals/bass"
    }
}
```

→ Sucht automatisch: `cmake/externals/includes/bass/Include.cmake`

### Mit Version

```json
{
    "lua54": {
        "path": "externals/lua54",
        "version": "5.4.6"
    }
}
```

→ Sucht: `cmake/externals/includes/lua54/Include.cmake`

---

## 6. Fehlerbehandlung

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E001 | Path leer | `path` Feld ist leer |
| E213 | Include.cmake fehlt | Include.cmake nicht gefunden |
| E214 | Pfad existiert nicht | External-Verzeichnis nicht vorhanden |

### Warnungen

| Code | Warnung | Beschreibung |
|------|---------|--------------|
| W103 | add_executable | External erstellt Executable |
| W104 | add_subdirectory | Examples/Tests werden hinzugefügt |

---

## 7. Siehe auch

- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Type Dispatcher
- [Externals_Reference.md](../../reference/Externals_Reference.md) — Verzeichnisstrukturen

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
