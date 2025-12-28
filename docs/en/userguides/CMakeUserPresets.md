# CMake User Presets – Benutzerhandbuch

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** C++ Developers  
> **Language:** English  
> **German:** [CMakeUserPresets_UserGuide.md](../../en/userguides/CMakeUserPresets.md)

---

## Table of Contents

1. [Überblick](#1-überblick)
2. [Prerequisites](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Grundlagen](#4-grundlagen)
5. [Configure Presets](#5-configure-presets)
6. [Build Presets](#6-build-presets)
7. [Test Presets](#7-test-presets)
8. [Workflow Presets](#8-workflow-presets)
9. [Praxis-Szenarien](#9-praxis-szenarien)
10. [Best Practices](#10-best-practices)
11. [Stolpersteine und Lösungen](#11-stolpersteine-und-lösungen)
12. [Troubleshooting](#12-troubleshooting)
13. [See Also](#13-siehe-auch)

---

## 1. Überblick

This guide explains, wie du mit `CMakeUserPresets.json` deine persönliche Build-Umgebung konfigurierst.

### Was sind User Presets?

| Datei | Zweck | Git |
|-------|-------|-----|
| `CMakePresets.json` | Team-weite Standards | ✅ Committen |
| `CMakeUserPresets.json` | **Persönliche** Anpassungen | ❌ In .gitignore |

### Wann User Presets verwenden?

- **Lokale Pfade** – vcpkg, Qt, SDK-Installationen
- **Performance** – Anzahl Build-Jobs anpassen
- **Experimente** – Neue Features testen ohne Team-Presets zu ändern
- **Umgebungsvariablen** – Qt-Pfade, Toolchain-Configuration
- **Eigene Kombinationen** – Team-Presets kombinieren

### Vorteile

- Keine Konflikte mit Team-Configuration
- Persönliche Optimierungen
- Schnelles Wechseln zwischen Configurationen
- IDE-Integration (Visual Studio, CLion, VS Code)

---

## 2. Prerequisites

### Checkliste

- [ ] **CMake 3.21+** (für Presets v4+)
- [ ] **CMakePresets.json** vorhanden (Team-Presets)
- [ ] **CMakeUserPresets.json in .gitignore** eingetragen
- [ ] **IDE** mit Preset-Unterstützung (optional)

### .gitignore prüfen

```bash
# Prüfen ob eingetragen
grep CMakeUserPresets .gitignore

# Falls nicht, hinzufügen
echo "CMakeUserPresets.json" >> .gitignore
```

### CMake-Version prüfen

```bash
cmake --version
# Minimum: 3.21 für Presets v4
# Empfohlen: 3.25+ für Presets v6
```

---

## 3. Schnellstart

### 3.1 Datei erstellen

Erstelle `CMakeUserPresets.json` im Projekt-Root:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "my-debug",
            "displayName": "Mein Debug Build",
            "inherits": "windows-ninja-debug",
            "cacheVariables": {
                "BUILD_TESTS": "ON"
            }
        }
    ]
}
```

### 3.2 Preset verwenden

```bash
# Verfügbare Presets anzeigen
cmake --list-presets

# Dein Preset verwenden
cmake --preset my-debug

# Bauen
cmake --build out/build/my-debug
```

### 3.3 In IDE verwenden

**Visual Studio 2022:**
- Öffne Projekt-Ordner
- Wähle Preset aus Dropdown (Toolbar)

**CLion:**
- Settings → Build → CMake
- "Load CMake presets" aktivieren

**VS Code:**
- CMake Tools Extension installieren
- Cmd+Shift+P → "CMake: Select Configure Preset"

---

## 4. Grundlagen

### 4.1 Datei-Struktur

```json
{
    "version": 6,
    "vendor": {
        "user-presets": {
            "version": "0.1.0",
            "author": "Dein Name",
            "lastModified": "2025-12-14"
        }
    },
    "configurePresets": [],
    "buildPresets": [],
    "testPresets": [],
    "packagePresets": [],
    "workflowPresets": []
}
```

### 4.2 Version

| Version | CMake | Features |
|---------|-------|----------|
| 4 | 3.21+ | Basis-Presets |
| 5 | 3.24+ | Package Presets |
| 6 | 3.25+ | Workflow Presets, include |

**Empfehlung:** Version 6 verwenden.

### 4.3 Vererbung (inherits)

```json
{
    "name": "my-preset",
    "inherits": ["windows-ninja-debug", "with-vcpkg"],
    "cacheVariables": {
        "MY_OPTION": "ON"
    }
}
```

- Erbt alle Einstellungen vom Parent
- Eigene Werte überschreiben Parent-Werte
- Mehrere Parents möglich (Array)

### 4.4 Hidden Presets

```json
{
    "name": "my-base",
    "hidden": true,
    "cacheVariables": {
        "VCPKG_ROOT": "C:/Dev/vcpkg"
    }
}
```

- Nicht in `--list-presets` sichtbar
- Nur zum Vererben gedacht
- Ideal für Pfad-Definitionen

---

## 5. Configure Presets

### 5.1 Grundstruktur

```json
{
    "configurePresets": [
        {
            "name": "my-config",
            "displayName": "Meine Configuration",
            "description": "Debug mit meinen Einstellungen",
            "inherits": "windows-ninja-debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/out/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "BUILD_TESTS": "ON"
            },
            "environment": {
                "QT_ROOT": "C:/Qt/6.7.0/msvc2022_64"
            }
        }
    ]
}
```

### 5.2 Importante Felder

| Feld | Description |
|------|--------------|
| `name` | Eindeutiger Bezeichner (für CLI) |
| `displayName` | Anzeigename (für IDE) |
| `inherits` | Parent-Preset(s) |
| `generator` | CMake-Generator |
| `binaryDir` | Build-Verzeichnis |
| `cacheVariables` | CMake-Cache-Variablen |
| `environment` | Umgebungsvariablen |

### 5.3 Umgebungsvariablen setzen

```json
{
    "name": "with-qt",
    "hidden": true,
    "environment": {
        "QT_ROOT": "C:/Qt/6.7.0/msvc2022_64",
        "PATH": "$env{QT_ROOT}/bin;$env{PATH}"
    }
}
```

**Variable-Referenceen:**
- `$env{VAR}` – Umgebungsvariable
- `$penv{VAR}` – Parent-Umgebungsvariable
- `${sourceDir}` – Projekt-Root
- `${presetName}` – Preset-Name

### 5.4 Conditions

```json
{
    "name": "windows-only",
    "condition": {
        "type": "equals",
        "lhs": "${hostSystemName}",
        "rhs": "Windows"
    }
}
```

---

## 6. Build Presets

### 6.1 Grundstruktur

```json
{
    "buildPresets": [
        {
            "name": "my-build",
            "displayName": "Schneller Build",
            "configurePreset": "my-config",
            "configuration": "Debug",
            "jobs": 16,
            "targets": ["MyApp", "CoreLib"]
        }
    ]
}
```

### 6.2 Importante Felder

| Feld | Description |
|------|--------------|
| `configurePreset` | Zugehöriges Configure-Preset |
| `configuration` | Debug/Release (Multi-Config) |
| `jobs` | Anzahl paralleler Jobs |
| `targets` | Nur bestimmte Targets bauen |
| `cleanFirst` | Clean vor Build |

### 6.3 Examples

**Schneller Build:**
```json
{
    "name": "fast",
    "configurePreset": "my-config",
    "jobs": 32
}
```

**Nur bestimmte Targets:**
```json
{
    "name": "app-only",
    "configurePreset": "my-config",
    "targets": ["MyApp"]
}
```

**Clean Build:**
```json
{
    "name": "clean-build",
    "configurePreset": "my-config",
    "cleanFirst": true
}
```

---

## 7. Test Presets

### 7.1 Grundstruktur

```json
{
    "testPresets": [
        {
            "name": "my-tests",
            "displayName": "Meine Tests",
            "configurePreset": "my-config",
            "configuration": "Debug",
            "output": {
                "outputOnFailure": true,
                "verbosity": "verbose"
            },
            "filter": {
                "include": {
                    "label": "unit"
                }
            }
        }
    ]
}
```

### 7.2 Filter

```json
{
    "filter": {
        "include": {
            "label": "unit",
            "name": "CoreLib.*"
        },
        "exclude": {
            "label": "slow"
        }
    }
}
```

### 7.3 Execution

```json
{
    "execution": {
        "jobs": 8,
        "timeout": 120,
        "stopOnFailure": false
    }
}
```

---

## 8. Workflow Presets

### 8.1 Grundstruktur

Workflow Presets kombinieren Configure → Build → Test → Package:

```json
{
    "workflowPresets": [
        {
            "name": "full-cycle",
            "displayName": "Kompletter Build-Zyklus",
            "steps": [
                { "type": "configure", "name": "my-config" },
                { "type": "build", "name": "my-build" },
                { "type": "test", "name": "my-tests" }
            ]
        }
    ]
}
```

### 8.2 Usage

```bash
cmake --workflow --preset full-cycle
```

### 8.3 Release-Workflow

```json
{
    "name": "release",
    "displayName": "Release Build",
    "steps": [
        { "type": "configure", "name": "release-config" },
        { "type": "build", "name": "release-build" },
        { "type": "test", "name": "all-tests" },
        { "type": "package", "name": "installer" }
    ]
}
```

---

## 9. Praxis-Szenarien

### 9.1 Qt6-Umgebung

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "qt-env",
            "hidden": true,
            "environment": {
                "QT_ROOT": "C:/Qt/6.7.0/msvc2022_64"
            }
        },
        {
            "name": "my-qt-debug",
            "displayName": "Debug mit Qt6",
            "inherits": ["windows-ninja-debug", "qt-env"]
        },
        {
            "name": "my-qt-release",
            "displayName": "Release mit Qt6",
            "inherits": ["windows-ninja-release", "qt-env"]
        }
    ]
}
```

### 9.2 vcpkg-Integration

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "with-vcpkg",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_TOOLCHAIN_FILE": "C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake",
                "VCPKG_TARGET_TRIPLET": "x64-windows"
            }
        },
        {
            "name": "my-debug-vcpkg",
            "displayName": "Debug + vcpkg",
            "inherits": ["windows-ninja-debug", "with-vcpkg"]
        }
    ]
}
```

### 9.3 Schnelle Entwicklung

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "dev-fast",
            "displayName": "Schnelle Entwicklung",
            "inherits": "windows-ninja-debug",
            "cacheVariables": {
                "BUILD_TESTS": "OFF",
                "BUILD_ONLY": "MyApp"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "dev-build",
            "configurePreset": "dev-fast",
            "jobs": 16
        }
    ]
}
```

### 9.4 Sanitizer-Builds (Linux/macOS)

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "with-asan",
            "displayName": "AddressSanitizer",
            "inherits": "linux-ninja-debug",
            "cacheVariables": {
                "CMAKE_CXX_FLAGS": "-fsanitize=address -fno-omit-frame-pointer",
                "CMAKE_EXE_LINKER_FLAGS": "-fsanitize=address"
            }
        },
        {
            "name": "with-ubsan",
            "displayName": "UndefinedBehaviorSanitizer",
            "inherits": "linux-ninja-debug",
            "cacheVariables": {
                "CMAKE_CXX_FLAGS": "-fsanitize=undefined",
                "CMAKE_EXE_LINKER_FLAGS": "-fsanitize=undefined"
            }
        }
    ]
}
```

### 9.5 Vollständiges Example

```json
{
    "version": 6,
    "vendor": {
        "user-presets": {
            "version": "0.1.0",
            "author": "Max Mustermann",
            "lastModified": "2025-12-14",
            "description": "Persönliche Entwicklungs-Presets"
        }
    },

    "configurePresets": [
        {
            "name": "qt-env",
            "hidden": true,
            "environment": {
                "QT_ROOT": "C:/Qt/6.7.0/msvc2022_64"
            }
        },
        {
            "name": "vcpkg-env",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_TOOLCHAIN_FILE": "C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
            }
        },
        {
            "name": "my-dev",
            "displayName": "Meine Entwicklungsumgebung",
            "description": "Debug mit Qt und vcpkg",
            "inherits": ["windows-ninja-debug", "qt-env", "vcpkg-env"],
            "cacheVariables": {
                "BUILD_TESTS": "ON"
            }
        },
        {
            "name": "my-release",
            "displayName": "Mein Release Build",
            "inherits": ["windows-ninja-release", "qt-env", "vcpkg-env"]
        }
    ],

    "buildPresets": [
        {
            "name": "dev-fast",
            "displayName": "Schneller Dev-Build",
            "configurePreset": "my-dev",
            "jobs": 16
        },
        {
            "name": "release-full",
            "displayName": "Vollständiger Release",
            "configurePreset": "my-release",
            "jobs": 8
        }
    ],

    "testPresets": [
        {
            "name": "unit-only",
            "displayName": "Nur Unit Tests",
            "configurePreset": "my-dev",
            "output": { "outputOnFailure": true },
            "filter": {
                "include": { "label": "unit" }
            }
        }
    ],

    "workflowPresets": [
        {
            "name": "dev-cycle",
            "displayName": "Entwicklungs-Zyklus",
            "steps": [
                { "type": "configure", "name": "my-dev" },
                { "type": "build", "name": "dev-fast" },
                { "type": "test", "name": "unit-only" }
            ]
        }
    ]
}
```

---

## 10. Best Practices

### ✅ Do's

- **Vendor-Block pflegen** – Version und Datum aktuell halten
- **Aussagekräftige Namen** – `displayName` für IDE-Anzeige
- **Hidden Presets** – Für wiederverwendbare Bausteine
- **Von Team-Presets erben** – Nicht kopieren!
- **Regelmäßig aufräumen** – Ungenutzte Presets entfernen

### ❌ Don'ts

- **Keine absoluten Pfade** ohne `hidden: true`
- **Team-Presets nicht überschreiben** – Eigene Namen verwenden
- **Keine sensiblen Daten** – Passwörter, API-Keys
- **Nicht committen** – Muss in .gitignore sein!
- **Keine komplexe Logik** – Dafür CMake-Module verwenden

### Namenskonventionen

```
my-*          → Persönliche Presets
with-*        → Feature-Bausteine (hidden)
dev-*         → Entwicklungs-Presets
test-*        → Test-Configurationen
```

---

## 11. Stolpersteine und Lösungen

### 11.1 Preset nicht sichtbar

**Problem:** Preset erscheint nicht in `--list-presets`.

**Ursache:** `hidden: true` gesetzt oder JSON-Error.

**Lösung:**
```bash
# Alle Presets (auch hidden)
cmake --list-presets=all

# JSON validieren
cat CMakeUserPresets.json | python -m json.tool
```

### 11.2 Vererbung funktioniert nicht

**Problem:** Einstellungen vom Parent werden nicht übernommen.

**Ursache:** `inherits` als String statt Array.

**Lösung:**
```json
// ❌ Falsch
"inherits": "windows-ninja-debug"

// ✅ Richtig
"inherits": ["windows-ninja-debug"]
```

### 11.3 Umgebungsvariable nicht gesetzt

**Problem:** `$env{QT_ROOT}` ist leer.

**Ursache:** Variable in falscher Sektion oder nicht definiert.

**Lösung:**
```json
{
    "name": "with-qt",
    "hidden": true,
    "environment": {
        "QT_ROOT": "C:/Qt/6.7.0/msvc2022_64"
    }
},
{
    "name": "my-debug",
    "inherits": ["windows-ninja-debug", "with-qt"]
    // Jetzt ist QT_ROOT verfügbar
}
```

### 11.4 Pfade mit Backslashes

**Problem:** Windows-Pfade mit `\` funktionieren nicht.

**Ursache:** Backslash ist JSON-Escape-Zeichen.

**Lösung:**
```json
// ❌ Falsch
"QT_ROOT": "C:\Qt\6.7.0"

// ✅ Richtig (Forward Slashes)
"QT_ROOT": "C:/Qt/6.7.0"

// ✅ Alternativ (Escaped)
"QT_ROOT": "C:\\Qt\\6.7.0"
```

### 11.5 Preset-Version inkompatibel

**Problem:** `CMake Error: Preset version X requires CMake Y`.

**Ursache:** CMake-Version zu alt für Preset-Version.

**Lösung:**
1. CMake aktualisieren, oder
2. Preset-Version reduzieren:
```json
{
    "version": 4  // Statt 6
}
```

---

## 12. Troubleshooting

### Checkliste bei Problemen

1. ☐ JSON-Syntax korrekt? (`python -m json.tool`)
2. ☐ Preset-Name eindeutig?
3. ☐ `inherits` als Array?
4. ☐ Parent-Preset existiert?
5. ☐ CMake-Version ausreichend?
6. ☐ Pfade korrekt (Forward Slashes)?
7. ☐ In .gitignore eingetragen?

### Häufige Error

| Error | Ursache | Lösung |
|--------|---------|--------|
| "No such preset" | Tipfehler / hidden | Namen prüfen |
| "Invalid JSON" | Syntax-Error | JSON validieren |
| "Preset requires CMake X" | Version zu alt | CMake updaten |
| "Cannot find inherited preset" | Parent fehlt | `inherits` prüfen |
| Env-Variable leer | Falsche Sektion | In `environment` setzen |

### Debug-Befehle

```bash
# Alle Presets anzeigen
cmake --list-presets=all

# Preset-Details
cmake --preset my-debug --debug-output

# JSON validieren
python -m json.tool < CMakeUserPresets.json

# Effektive Configuration anzeigen
cmake -LAH out/build/my-debug
```

---

## 13. See Also

- [CMakePresets_Reference.md](../references/CMakePresets.md) – Team-Presets Reference
- [CMakeUserPresets_Reference.md](../references/CMakeUserPresets.md) – User-Presets Reference
- [Qt6_Integration_UserGuide.md](Qt6_Integration.md) – Qt6-Umgebung konfigurieren
- [Getting_Started_UserGuide.md](Getting_Started.md) – Projekt-Grundlagen

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Konformität: Erweitert mit Workflow Presets, Praxis-Szenarien, Stolpersteine/Troubleshooting** |
| 0.1.0 | 2025-12-03 | Initial: Templates, Example-Szenarien, Best Practices |
