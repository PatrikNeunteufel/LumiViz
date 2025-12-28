# Debug.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Debug.md](../../en/modules/core/Debug.md)  
> **Modul:** [`cmake/core/Debug.cmake`](../../../../cmake/core/Debug.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [Globale Konfiguration](#4-globale-konfiguration)
5. [API-Referenz](#5-api-referenz)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Output-Beispiele](#7-output-beispiele)
8. [Best Practices](#8-best-practices)
9. [Interne Implementierung](#9-interne-implementierung)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Das `Debug.cmake` Modul implementiert ein **kontextbasiertes Debug-System** mit Zwei-Achsen-Filterung. Es ermöglicht granulare Kontrolle über Debug-Ausgaben ohne den Code mit `if(DEBUG)` zu übersäen.

### Kernidee

Jede Message hat eine Wichtigkeit (FREQ), jeder Kontext hat ein Sichtbarkeits-Level (SHOW). Message erscheint wenn `FREQ <= SHOW`.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Filterung | Zwei-Achsen-System (SHOW × FREQ) |
| Kontexte | Isolierte Debug-Bereiche pro Modul |
| Kontrolle | Master-Switch, Level-Override, ONCE-Flag |
| Struktur | Tags, Leerzeilen, Trennlinien |

### Verwendung durch

- CompilerOptions.cmake, Solution.cmake, Executables.cmake
- Alle Module die Debug-Ausgaben benötigen

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| — | — | Keine (Basis-Modul, sollte früh geladen werden) |

**Design-Entscheidung:** Debug.cmake ist ein Basis-Modul ohne Abhängigkeiten, da es von fast allen anderen Modulen verwendet wird.

---

## 3. Konzept

### 3.1 Zwei-Achsen-Filterung

| Achse | Frage | Werte |
|-------|-------|-------|
| **SHOW** | Wie viel will ich sehen? | 1-5 (wenig → alles) |
| **FREQ** | Wie wichtig ist diese Message? | 1-5 (wichtig → Detail) |

**Filterregel:** Message erscheint wenn `FREQ <= SHOW`

### 3.2 SHOW-Level Konstanten

| Konstante | Wert | Bedeutung |
|-----------|------|-----------|
| `DBG_SHOW_LITTLE` | 1 | Nur das Wichtigste |
| `DBG_SHOW_SOME` | 2 | Standard (Default) |
| `DBG_SHOW_MUCH` | 3 | Mehr Details |
| `DBG_SHOW_LOTS` | 4 | Viele Details |
| `DBG_SHOW_ALL` | 5 | Alles |

### 3.3 FREQ-Level Konstanten

| Konstante | Wert | Bedeutung | Beispiel |
|-----------|------|-----------|----------|
| `DBG_OFTEN` | 1 | Häufig, wichtig | Modul-Start, Phase-Start |
| `DBG_COMMON` | 2 | Übliche Info | Gefundene Dateien, Features |
| `DBG_NORMAL` | 3 | Standard | Zwischenschritte |
| `DBG_RARE` | 4 | Selten benötigt | Pfade, Variablen |
| `DBG_ULTRA_RARE` | 5 | Nur tiefes Debugging | Loop-Iterationen |

### 3.4 Beispiel: Filterung

Mit `SHOW = DBG_SHOW_MUCH (3)`:

| FREQ-Level | Wert | Erscheint? | Grund |
|------------|------|------------|-------|
| `DBG_OFTEN` | 1 | ✅ | 1 <= 3 |
| `DBG_COMMON` | 2 | ✅ | 2 <= 3 |
| `DBG_NORMAL` | 3 | ✅ | 3 <= 3 |
| `DBG_RARE` | 4 | ❌ | 4 > 3 |
| `DBG_ULTRA_RARE` | 5 | ❌ | 5 > 3 |

---

## 4. Globale Konfiguration

### 4.1 Cache-Variablen

| Variable | Typ | Default | Beschreibung |
|----------|-----|---------|--------------|
| `DEBUG_MESSAGES` | BOOL | ON | Master-Switch für alle Ausgaben |
| `DEBUG_DEFAULT_LEVEL` | INT | 2 | Standard SHOW-Level für neue Kontexte |

### 4.2 Aktivierung

```bash
# Alle Debug-Ausgaben deaktivieren
cmake -B build -DDEBUG_MESSAGES=OFF

# Mehr Details sehen
cmake -B build -DDEBUG_DEFAULT_LEVEL=4

# Alles sehen
cmake -B build -DDEBUG_DEFAULT_LEVEL=5
```

---

## 5. API-Referenz

### 5.1 dbg_init()

Initialisiert einen Debug-Kontext.

```cmake
dbg_init(
    ID <context_id>
    [LEVEL <show_level>]
    [SWITCH <ON|OFF>]
    [TAG <prefix>]
)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `ID` | String | ✓ | Eindeutiger Kontext-Identifier |
| `LEVEL` | Int/Konstante | — | DBG_SHOW_* oder 1-5 (Default: DEBUG_DEFAULT_LEVEL) |
| `SWITCH` | BOOL | — | ON/OFF (Default: ON) |
| `TAG` | String | — | Prefix in eckigen Klammern |

**Rückgabe:** Keine (speichert als DIRECTORY PROPERTY)

**Beispiel:**

```cmake
# Minimal
dbg_init(ID MY_MODULE)

# Vollständig
dbg_init(
    ID SOLUTION 
    LEVEL ${DBG_SHOW_MUCH} 
    SWITCH ON 
    TAG "Solution"
)

# Deaktiviert (für temporäres Ausschalten)
dbg_init(ID VERBOSE SWITCH OFF)
```

---

### 5.2 dbg()

Gibt eine Debug-Message aus (wenn Filter passt).

```cmake
dbg(<freq_level> "<message>"
    [ID <context_id>]
    [LEVEL <show_level>]
    [SWITCH <ON|OFF>]
    [TAG <extra_tag>]
    [ONCE]
)
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `freq_level` | Int/Konstante | ✓ | DBG_* Wichtigkeit |
| `message` | String | ✓ | Auszugebende Nachricht |
| `ID` | String | — | Kontext-Referenz |
| `LEVEL` | Int | — | Override SHOW-Level |
| `SWITCH` | BOOL | — | Override Switch |
| `TAG` | String | — | Zusätzliches Tag |
| `ONCE` | Flag | — | Pro Kontext nur einmal ausgeben |

**Rückgabe:** Keine (gibt auf stdout aus wenn Filter passt)

**Beispiel:**

```cmake
# Mit Kontext
dbg(${DBG_OFTEN} "=== Module Start ===" ID MY_MODULE)
dbg(${DBG_COMMON} "Processing: ${_file}" ID MY_MODULE)
dbg(${DBG_RARE} "Detail: ${_internal}" ID MY_MODULE)

# ONCE für Warnungen in Schleifen
foreach(_item IN LISTS _items)
    dbg(${DBG_COMMON} "Deprecated feature used" ID MY_MODULE ONCE)
endforeach()

# Standalone (ohne Kontext)
dbg(${DBG_NORMAL} "Standalone message")
```

---

### 5.3 dbgspace()

Gibt eine Leerzeile aus.

```cmake
dbgspace([ID <context_id>] [SWITCH <ON|OFF>])
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `ID` | String | — | Kontext für Switch-Check |
| `SWITCH` | BOOL | — | Override Switch |

**Rückgabe:** Keine

**Beispiel:**

```cmake
dbg(${DBG_OFTEN} "Abschnitt 1" ID MY_DBG)
dbgspace(ID MY_DBG)
dbg(${DBG_OFTEN} "Abschnitt 2" ID MY_DBG)
```

---

### 5.4 enddbgblock()

Gibt eine Trennlinie aus (visuelle Strukturierung).

```cmake
enddbgblock([ID <context_id>] [SWITCH <ON|OFF>])
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `ID` | String | — | Kontext für Switch-Check |
| `SWITCH` | BOOL | — | Override Switch |

**Rückgabe:** Keine

**Ausgabe:**
```
-- -------------------------------------------
```

**Beispiel:**

```cmake
dbg(${DBG_OFTEN} "=== Modul Start ===" ID MY_DBG)
dbg(${DBG_COMMON} "Verarbeitung..." ID MY_DBG)
enddbgblock(ID MY_DBG)
```

---

### 5.5 setup_debug_from_args()

Helper für Funktionen mit optionalen Debug-Flags.

```cmake
setup_debug_from_args(<out_switch> <out_tag> <default_tag> [SHOW_DEBUG] [DEBUG_TAG <tag>])
```

**Parameter:**

| Parameter | Typ | Pflicht | Beschreibung |
|-----------|-----|---------|--------------|
| `out_switch` | Variable | ✓ | Output für Switch (ON/OFF) |
| `out_tag` | Variable | ✓ | Output für Tag |
| `default_tag` | String | ✓ | Default-Tag wenn nicht angegeben |
| `SHOW_DEBUG` | Flag | — | Debug aktivieren |
| `DEBUG_TAG` | String | — | Optionales Tag |

**Rückgabe:** Setzt `out_switch` und `out_tag` via PARENT_SCOPE

**Beispiel:**

```cmake
function(my_function)
    cmake_parse_arguments(ARG "SHOW_DEBUG" "DEBUG_TAG" "" ${ARGN})
    setup_debug_from_args(_sw _tag "MY_FUNC" ${ARGN})
    
    dbg_init(ID MY_FUNC_DBG LEVEL ${DBG_SHOW_MUCH} SWITCH ${_sw} TAG "${_tag}")
    dbg(${DBG_OFTEN} "Function started" ID MY_FUNC_DBG)
    # ...
    enddbgblock(ID MY_FUNC_DBG)
endfunction()

# Aufruf
my_function()                                    # Kein Debug
my_function(SHOW_DEBUG)                          # Debug mit Default-Tag
my_function(SHOW_DEBUG DEBUG_TAG "CustomTag")    # Debug mit eigenem Tag
```

---

## 6. Verwendungsbeispiele

### 6.1 Modul mit Debug-Support

```cmake
# cmake/project/MyModule.cmake
include_guard(GLOBAL)

# Lokaler Switch für schnelles Ein/Ausschalten
set(_SHOW_MY_MODULE_DEBUG OFF)

dbg_init(
    ID MY_MODULE 
    LEVEL ${DBG_SHOW_MUCH} 
    SWITCH ${_SHOW_MY_MODULE_DEBUG} 
    TAG "MyModule"
)

dbg(${DBG_OFTEN} "=== MyModule Loading ===" ID MY_MODULE)

foreach(_item IN LISTS _items)
    dbg(${DBG_COMMON} "Processing: ${_item}" ID MY_MODULE)
    dbg(${DBG_RARE} "  Detail: ${_internal}" ID MY_MODULE)
endforeach()

dbg(${DBG_OFTEN} "MyModule loaded successfully" ID MY_MODULE)
enddbgblock(ID MY_MODULE)
```

### 6.2 Funktion mit optionalem Debug

```cmake
function(apply_compiler_options TARGET_NAME)
    cmake_parse_arguments(ARG "SHOW_DEBUG" "DEBUG_TAG" "" ${ARGN})
    setup_debug_from_args(_sw _tag "CompilerOpts" ${ARGN})
    
    dbg_init(ID CO_DBG LEVEL ${DBG_SHOW_MUCH} SWITCH ${_sw} TAG "${_tag}")
    dbg(${DBG_OFTEN} "Applying to: ${TARGET_NAME}" ID CO_DBG)
    
    # ... Implementation ...
    
    dbg(${DBG_OFTEN} "Done" ID CO_DBG)
    enddbgblock(ID CO_DBG)
endfunction()
```

### 6.3 Debug-Level via Preset

```json
{
    "configurePresets": [
        {
            "name": "debug-verbose",
            "cacheVariables": {
                "DEBUG_MESSAGES": "ON",
                "DEBUG_DEFAULT_LEVEL": "5"
            }
        },
        {
            "name": "release-quiet",
            "cacheVariables": {
                "DEBUG_MESSAGES": "OFF"
            }
        }
    ]
}
```

---

## 7. Output-Beispiele

### 7.1 Standard (SHOW_LEVEL = 2)

```
-- [Solution] Solution.json geladen
-- [Solution] MySolution v0.1.0
-- [Solution] Externals: 8
-- -------------------------------------------
```

### 7.2 Verbose (SHOW_LEVEL = 5)

```
-- [Solution] Solution.json geladen
-- [Solution] Schema-Version: 0.1
-- [Solution] MySolution v0.1.0
-- [Solution] Description: Multi-Project Solution
-- [Solution] Authors: Author Name
-- [Solution] C++ Standard: 20
-- [Solution] C Standard: 17
-- [Solution] Default Library Type: STATIC
-- [Solution] Externals: 8
-- [Solution] Executables: 3, Libraries: 0, Tests: 1
-- -------------------------------------------
```

---

## 8. Best Practices

### 8.1 Konsistente FREQ-Level

```cmake
# ✅ Gut - konsistente Wichtigkeit
dbg(${DBG_OFTEN} "=== Module Start ===" ID MOD)    # Immer sichtbar
dbg(${DBG_COMMON} "Processing file X" ID MOD)       # Übliche Info
dbg(${DBG_RARE} "Internal: ${_var}" ID MOD)         # Nur bei Bedarf

# ❌ Schlecht - alles gleich wichtig
dbg(${DBG_OFTEN} "Processing file X" ID MOD)
dbg(${DBG_OFTEN} "Internal: ${_var}" ID MOD)
```

### 8.2 Aussagekräftige Tags

```cmake
# ✅ Gut
dbg_init(ID SOLUTION TAG "Solution")
dbg_init(ID EXTERNALS TAG "Externals")

# ❌ Schlecht
dbg_init(ID DBG1 TAG "D1")
```

### 8.3 ONCE für Schleifen-Warnungen

```cmake
foreach(_ext IN LISTS _externals)
    # Erscheint nur einmal, nicht bei jedem Durchlauf
    dbg(${DBG_COMMON} "Deprecated feature used" ID MY_DBG ONCE)
endforeach()
```

### 8.4 Lokaler Switch für schnelles Debugging

```cmake
# Oben im Modul - schnell ein/ausschaltbar
set(_SHOW_MODULE_DEBUG OFF)  # ← Hier ändern

dbg_init(ID MOD SWITCH ${_SHOW_MODULE_DEBUG} ...)
```

---

## 9. Interne Implementierung

Das Modul verwendet **DIRECTORY PROPERTY** für Kontext-Speicherung:

| Property | Beschreibung |
|----------|--------------|
| `DBG_${ID}_LEVEL` | SHOW-Level |
| `DBG_${ID}_SWITCH` | ON/OFF |
| `DBG_${ID}_TAG` | Tag-Prefix |
| `DBG_${ID}_ONCE_GUARD` | Liste bereits gezeigter ONCE-Messages |

---

## 10. Siehe auch

- [Errors.cmake](Errors.md) — Für echte Fehler (cmake_fatal, cmake_warn)
- [guidelines](../../../standards/guidelines.md) — Debug-Konventionen
- [CMake.md](../../../blueprints/CMake.md) — Debug in Modulen

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-04 | Initial (Clean Start): Zwei-Achsen-Filterung, dbg_init/dbg/dbgspace/enddbgblock/setup_debug_from_args API |
