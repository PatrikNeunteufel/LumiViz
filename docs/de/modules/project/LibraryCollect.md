# LibraryCollect.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-20  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [LibraryCollect.md](../../en/modules/project/LibraryCollect.md)  
> **Modul:** [`cmake/project/LibraryCollect.cmake`](../../../../cmake/project/LibraryCollect.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Context-Keys](#4-context-keys)
5. [JSON-Mapping](#5-json-mapping)
6. [Fehlerbehandlung](#6-fehlerbehandlung)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `LibraryCollect.cmake` Modul **sammelt Daten** aus dem JSON einer Library und speichert sie in einem Context. Es ist für das Parsing und die Normalisierung der Library-Definitionen zuständig.

### Kernidee

Analog zu ExecutableCollect — Trennung von Concerns: Collect sammelt, Create verwendet.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| JSON-Parsing | Alle Felder extrahieren |
| Normalisierung | Defaults setzen, Typen vereinheitlichen |
| Context-Befüllung | Alle Werte als Context-Keys speichern |

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| Json.cmake | 0.5.0 | Alle `_json_*` Funktionen |
| Context.cmake | 0.5.0 | `ctx_set` |
| Debug.cmake | 0.5.0 | `dbg` |

---

## 3. API-Referenz

### 3.1 _collect_library()

Sammelt alle Felder einer Library aus JSON in einen Context.

```cmake
_collect_library(<LIB_JSON> <CTX>)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `LIB_JSON` | String | ✓ | JSON-String der Library |
| `CTX` | String | ✓ | Context-Prefix (z.B. `LIB_0`) |

**Rückgabe:** Keine (befüllt Context)

---

## 4. Context-Keys

### 4.1 Pflichtfelder

| Key | Typ | Beschreibung |
|-----|-----|--------------|
| `NAME` | String | Target-Name (Pflicht in JSON) |

### 4.2 Optionale Felder

| Key | Typ | Default | Beschreibung |
|-----|-----|---------|--------------|
| `VERSION` | String | Solution-Version | Version |
| `PATH` | Path | `projects/libs/{name}/src` | Source-Pfad |
| `TYPE` | Enum | Aus settings | STATIC, SHARED, INTERFACE |
| `PUBLIC_HEADERS` | Path | "" | Öffentliche Header-Verzeichnis |
| `SKIP` | Bool | FALSE | Überspringen |
| `PCH_ENABLED` | Bool | FALSE | PCH aktivieren |
| `PCH_HEADER` | String | `pch.h` | PCH-Header-Name |
| `PCH_PATH` | String | "" | Custom PCH-Pfad (relativ zu projects/) |
| `DEPENDENCIES` | List | "" | Interne Abhängigkeiten |
| `EXTERNALS` | List | "" | Externe Abhängigkeiten |
| `EXTERNAL_OPTIONS` | JSON | "{}" | Per-External Optionen |
| `PLATFORM` | String | "" (= alle) | Ziel-Plattform |

**Hinweis:** PCH wird implizit aktiviert wenn `pch.header` oder `pch.path` angegeben ist und `pch.enabled` nicht explizit `false` ist.

---

## 5. JSON-Mapping

### 5.1 Minimale JSON

```json
{
    "name": "CoreLib"
}
```

### 5.2 Vollständige JSON

```json
{
    "name": "CoreLib",
    "version": "1.0.0",
    "path": "src/libs/core",
    "type": "STATIC",
    "public_headers": "include/core",
    "skip": false,
    "pch": {
        "enabled": true,
        "header": "pch.h"
    },
    "dependencies": ["UtilLib"],
    "externals": ["bass"],
    "external_options": {
        "bass": { "BASS_FLAC": true }
    },
    "platform": "windows"
}
```

---

## 6. Fehlerbehandlung

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | `name`-Feld fehlt | Name hinzufügen |

---

## 7. Siehe auch

- [Libraries.cmake](Libraries.md) — Ruft _collect_library auf
- [LibraryCreate.cmake](LibraryCreate.md) — Verwendet den befüllten Context
- [ExecutableCollect.cmake](ExecutableCollect.md) — Analoges Modul für Executables

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.6.0** | **2025-12-20** | **EXTERNAL_OPTIONS hinzugefügt: Per-External Optionen analog zu ExecutableCollect** |
| 0.5.1 | 2025-12-18 | PCH-Support hinzugefügt: PCH_ENABLED, PCH_HEADER, PCH_PATH |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0 |
| 0.1.0 | 2025-12-07 | Initial (Clean Start): JSON zu Context Mapping |
