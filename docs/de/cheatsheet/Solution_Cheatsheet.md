# Solution.json — Cheatsheet

> **Version:** 1.0.0  
> **Letzte Aktualisierung:** 2025-12-18  
> **Für:** CMake Architecture  
> **Sprache:** Deutsch  
> **English:** [Solution_Cheatsheet.md](../../en/cheatsheets/Solution_Cheatsheet.md)

---

## Schnellstart

```json
{
    "schemaVersion": "0.6",
    "solution": { "name": "MyProject", "version": "1.0.0" },
    "externals": { "doctest": { "path": "externals/doctest" } },
    "libraries": [{ "name": "CoreLib" }],
    "executables": [{ "name": "MyApp", "dependencies": ["CoreLib"] }]
}
```

---

## Root-Struktur

| Block | Pflicht | Beschreibung |
|-------|---------|--------------|
| `schemaVersion` | ✅ | `"0.6"` |
| `solution` | ✅ | Name, Version, Beschreibung |
| `settings` | — | C++ Standard, Defaults |
| `externals` | — | Externe Abhängigkeiten |
| `libraries` | — | Interne Libraries |
| `executables` | — | Monolithische Executables |
| `tests` | — | Standalone Tests |
| `apps` | — | App-Container (Core/Runner) |

---

## Externals

### Lokal (im Repo)

```json
"bass": { "path": "externals/bass" }
```

### Fetched (Git)

```json
"glfw": { 
    "git": "https://github.com/glfw/glfw.git", 
    "tag": "3.4" 
}
```

### Mit Hook

```json
"imgui": { 
    "git": "...", 
    "tag": "v1.91.6",
    "hook": "imgui"
}
```

---

## Executables

| Feld | Pflicht | Default | Beschreibung |
|------|---------|---------|--------------|
| `name` | ✅ | — | Target-Name |
| `type` | — | `CONSOLE` | `CONSOLE` / `GUI` |
| `path` | — | `projects/exec/{name}` | Source-Pfad |
| `dependencies` | — | `[]` | Interne Libraries |
| `externals` | — | `[]` | Externe Dependencies |
| `skip` | — | `false` | Überspringen |

```json
{
    "name": "MyApp",
    "type": "GUI",
    "dependencies": ["CoreLib"],
    "externals": ["glfw", "glad"]
}
```

---

## Libraries

| Feld | Pflicht | Default | Beschreibung |
|------|---------|---------|--------------|
| `name` | ✅ | — | Target-Name |
| `type` | — | `STATIC` | `STATIC` / `SHARED` / `INTERFACE` |
| `path` | — | `projects/libs/{name}` | Source-Pfad |

```json
{ "name": "CoreLib", "type": "STATIC" }
```

---

## Apps (Core/Runner)

```json
{
    "name": "MyVisualizer",
    "core": {
        "dependencies": ["CoreLib"],
        "externals": ["bass"]
    },
    "runner": {
        "type": "GUI",
        "externals": ["glfw", "glad"]
    },
    "pch": { "enabled": true },
    "tests": {
        "framework": "doctest",
        "targets": [
            { "name": "UnitTests", "type": "unit" }
        ]
    }
}
```

### Generierte Targets

| Target | Typ |
|--------|-----|
| `{App}.Core` | STATIC Library |
| `{App}` | Executable |
| `{App}.{TestName}` | Test Executable |

---

## Tests

### Standalone (tests[])

```json
{
    "name": "CoreLib_Tests",
    "type": "unit",
    "framework": "doctest",
    "dependencies": ["CoreLib"],
    "timeout": 30,
    "labels": ["unit", "fast"]
}
```

### App-Tests (apps[].tests.targets[])

```json
{
    "name": "UnitTests",
    "type": "unit",
    "skip": false,
    "timeout": 30,
    "parallel": true
}
```

### Test-Typ Defaults

| Typ | Timeout | Parallel |
|-----|---------|----------|
| `unit` | 30s | ✅ |
| `integration` | 120s | ✅ |
| `performance` | 300s | ❌ |
| `system` | 180s | ❌ |
| `smoke` | 10s | ✅ |

---

## Skip-Feature

| Ebene | JSON | Wirkung |
|-------|------|---------|
| App | `"skip": true` | Gesamte App |
| Alle Tests | `"tests": { "skip": true }` | Alle App-Tests |
| Ein Test | `"targets": [{ "skip": true }]` | Einzelner Test |

📌 **Global skip hat Vorrang!**

---

## PCH (Precompiled Header)

```json
"pch": {
    "enabled": true,
    "header": "pch.h"
}
```

Datei liegt in: `{app}/pch/pch.h`

---

## Verzeichnisstruktur

```
projects/
├── apps/{AppName}/
│   ├── include/     → Core PUBLIC Headers
│   ├── src/         → Core Implementation
│   ├── main/        → Runner (main.cpp)
│   ├── pch/         → pch.h
│   └── tests/{type}/{TestName}/
├── exec/{ExeName}/
│   └── src/
├── libs/{LibName}/
│   ├── include/
│   └── src/
└── tests/{TestName}/
```

---

## Fehler-Codes

| Code | Bereich | Bedeutung |
|------|---------|-----------|
| E0xx | JSON | Parsing-Fehler |
| E1xx | Target | Erstellungs-Fehler |
| E2xx | External | External-Fehler |
| E3xx | Test | Test-Fehler |
| E4xx | App | App-Container-Fehler |

---

## CMake-Variablen

| Variable | Default | Wirkung |
|----------|---------|---------|
| `BUILD_TESTS` | ON | Tests aktivieren |
| `BUILD_ONLY` | `""` | Nur bestimmte Targets |
| `EXTERNALS_OFFLINE` | OFF | Kein Netzwerk |
| `EXTERNALS_FORCE_FETCH` | OFF | Cache ignorieren |

---

## Tipps

- 💡 `externals` zentral definieren → nie inline in executables
- 💡 `apps[]` für testbare Anwendungen verwenden
- 💡 `skip: true` für temporär deaktivierte Tests
- 📌 Schema-Version immer angeben!

---

## Siehe auch

- [Solution_Schema.md](../references/Solution_Schema.md) — Vollständige Referenz
- [AppContainer_Concept.md](../projects/buildsystem/concepts/AppContainer_Concept.md) — App-Konzept
- [ErrorCodes.md](../references/ErrorCodes.md) — Alle Fehler-Codes
