# Makefile Standard — Stil-Richtlinien

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Build System Developers  
> **Scope:** Alle Makefile-basierten Build-Systeme  
> **Durchsetzung:** Code Review  
> **Language:** English

---

## Table of Contents

1. [Zweck und Scope](#1-zweck-und-geltungsbereich)
2. [Grundprinzipien](#2-grundprinzipien)
3. [Datei-Organisation](#3-datei-organisation)
4. [Formatierung](#4-formatierung)
5. [Namenskonventionen](#5-namenskonventionen)
6. [Variablen](#6-variablen)
7. [Targets und Regeln](#7-targets-und-regeln)
8. [Kommentare](#8-kommentare)
9. [Errorbehandlung](#9-fehlerbehandlung)
10. [Diagnose-Targets](#10-diagnose-targets)
11. [Shell-Befehle](#11-shell-befehle)
12. [Portabilität](#12-portabilität)
13. [See Also](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Zweck und Scope

Dieser Standard definiert **Coding-Konventionen für Makefiles** im Unternehmen.

### 1.1 Zielgruppe

Build System Developers, die Makefiles für Embedded- oder PC-Projekte schreiben.

### 1.2 Anwendungsbereich

| Projekt-Typ | Gilt für |
|-------------|----------|
| Embedded Build-Systeme | TMS320, Microchip, etc. |
| PC Build-Systeme | Standalone Makefiles |
| CMake-Projekte | Nicht direkt (siehe CMake_Standard) |

---

## 2. Grundprinzipien

1. **Transparenz** — Jeder Schritt soll nachvollziehbar sein
2. **Fail-Fast** — Error früh erkennen und klar melden
3. **Modularität** — Wiederverwendbare, unabhängige Module
4. **Dokumentation** — Diagnose-Targets für Debugging

### 2.1 TODO-Marker

Für unfertige Aufgaben: `# TODO:` im Kommentar verwenden.

```makefile
# TODO: Add fallback for missing compiler
# TODO: Implement parallel build support
```

---

## 3. Datei-Organisation

### 3.1 Dateinamen

| Typ | Konvention | Example |
|-----|------------|----------|
| Haupt-Makefile | `Makefile` | `Makefile` |
| Modul | `<name>.mk` | `toolchain.mk`, `paths.mk` |
| Ordner | `lowercase` | `common/`, `make/` |

### 3.2 Modul-Struktur

```
make/
├── common/          /* Gemeinsame Module    */
│   ├── config.mk
│   ├── paths.mk
│   └── ...
└── <project>/       /* Projekt-spezifisch   */
    ├── config.mk
    ├── sources.mk
    └── rules.mk
```

### 3.3 Include-Reihenfolge

Die Reihenfolge der Includes ist kritisch:

```makefile
# 1. Bootstrap/Argument-Validierung
include common/arguments.mk

# 2. Configuration
include common/config.mk
include common/paths.mk

# 3. Outputs
include common/outputs.mk

# 4. Toolchain
include common/toolchain.mk

# 5. Projekt-spezifisch
include $(Project)/config.mk
include $(Project)/sources.mk
include $(Project)/rules.mk
```

---

## 4. Formatierung

### 4.1 Overview

| Aspekt | Regel |
|--------|-------|
| Einrückung in Rezepten | **Tab** (Required in Make) |
| Einrückung außerhalb | 2 Spaces |
| Zeilenlänge | 100 Zeichen (soft limit) |
| Leerzeilen | Zwischen Sektionen |

### 4.2 Zeilenumbruch

```makefile
# Lange Listen mit Backslash umbrechen
SOURCES := \
    file1.c \
    file2.c \
    file3.c

# Lange Befehle umbrechen
$(CC) \
    $(CFLAGS) \
    -o "$@" \
    "$<"
```

### 4.3 Ausrichtung

```makefile
# Gleichheitszeichen ausrichten
CC      := $(CG_TOOL_ROOT)/bin/cl2000
AR      := $(CG_TOOL_ROOT)/bin/ar2000
LD      := $(CG_TOOL_ROOT)/bin/lnk2000

# Werte ausrichten
PROJECT     = MyProject
CONFIG      = Debug
OUTPUT_DIR  = $(BUILD_DIR)/out
```

---

## 5. Namenskonventionen

### 5.1 Overview

| Entität | Konvention | Example |
|---------|------------|----------|
| **Exportierte Variable** | UPPER_CASE | `CG_TOOL_ROOT`, `CFLAGS` |
| **Lokale Variable** | lower_case oder UPPER_CASE | `_temp`, `LOCAL_VAR` |
| **Funktion (call)** | lower_case mit _ | `_require_dir`, `normpath` |
| **Target** | lower-case mit - | `all`, `clean`, `check-paths` |
| **Phony Target** | lower-case mit - | `check-all`, `print-%` |

### 5.2 Präfixe

| Präfix | Bedeutung | Example |
|--------|-----------|----------|
| `_` | Interne/lokale Variable | `_CG_CAND` |
| `check-` | Diagnose-Target | `check-paths` |
| `print-` | Variable ausgeben | `print-CFLAGS` |

### 5.3 Modul-Namensraum

Variablen aus einem Modul können mit Modul-Präfix versehen werden:

```makefile
# In paths.mk
PATHS_INCLUDE_DIRS := ...
PATHS_LIB_DIRS := ...

# Oder generisch exportieren
INCLUDE_DIRS := ...
export INCLUDE_DIRS
```

---

## 6. Variablen

### 6.1 Expansion-Arten

| Art | Syntax | Usage |
|-----|--------|------------|
| **Sofortige Expansion** | `:=` | Pfade, Listen, konstante Werte |
| **Verzögerte Expansion** | `=` | Werte die von späteren Variablen abhängen |
| **Bedingte Zuweisung** | `?=` | Defaults, die überschrieben werden können |
| **Anhängen** | `+=` | Listen erweitern |

### 6.2 Best Practices

```makefile
# ✅ Sofortige Expansion für Pfade
OUT_DIR := $(ProjectDir)/Build/out

# ✅ Verzögerte Expansion für abhängige Werte
GLOBAL_LIB = $(OUT_DIR)/Global$(ABI_SUFFIX).lib

# ✅ Default mit ?=
Config ?= Debug

# ✅ Override für Argument-Normalisierung
override ProjectDir := $(call normpath,$(ProjectDir))

# ✅ Export für rekursive Aufrufe
export CC CFLAGS LDFLAGS
```

### 6.3 Argument-Validierung

```makefile
# Pattern: Required argument
Project ?= Unknown
ifeq ($(Project),Unknown)
  $(error [FATAL] Argument Project was not provided!)
endif

# Pattern: Allowed values
ifneq ($(Config),Debug)
  ifneq ($(Config),Release)
    $(error [FATAL] Config must be Debug or Release)
  endif
endif
```

---

## 7. Targets und Regeln

### 7.1 .PHONY

Alle Targets ohne Datei-Output als `.PHONY` deklarieren:

```makefile
.PHONY: all clean check-all print-%
```

### 7.2 Standard-Targets

| Target | Description |
|--------|--------------|
| `all` | Standard-Build (Default) |
| `clean` | Build-Artefakte löschen |
| `check-all` | Alle Diagnosen |

### 7.3 Pattern Rules

```makefile
# Pattern Rule für C-Dateien
$(OBJ_DIR)/%.obj: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo [CC] $<
	@"$(CC)" $(CFLAGS) -o "$@" "$<"
```

### 7.4 Order-Only Prerequisites

```makefile
# | trennt normale von Order-Only Prerequisites
$(OUT_DIR)/app.out: $(OBJS) | $(OUT_DIR)
	@echo [LD] $@
	@"$(LD)" -o "$@" $(OBJS)

# Order-Only: Verzeichnis wird erstellt, triggert aber keinen Rebuild
$(OUT_DIR):
	@mkdir -p "$@"
```

---

## 8. Kommentare

### 8.1 Datei-Header

```makefile
# <module>.mk - <kurze Description>
# <optionale Details>
# !!!!!!!!!!!!!!!!!!!!!!!!! DON'T TOUCH WITHOUT ANY GOOD REASON !!!!!!!!!!!!!!!!!!!!!!
```

### 8.2 Sektion-Header

```makefile
# ==== Section Name ===============================================================
# Optionale Description
```

### 8.3 Inline-Kommentare

```makefile
CC := $(CG_TOOL_ROOT)/bin/cl2000  # TI C2000 compiler
```

### 8.4 Parsing-Info

```makefile
$(info *** parsing <module>.mk ***)
$(info )
```

---

## 9. Errorbehandlung

### 9.1 Fail-Fast

```makefile
# Fehlende Variable
ifeq ($(REQUIRED_VAR),)
  $(error [FATAL] REQUIRED_VAR is not set)
endif

# Fehlende Datei/Verzeichnis
_require_dir  = $(if $(wildcard $(1)/.),,$(error [FATAL] Missing directory: "$(1)" - $(2)))
_require_file = $(if $(wildcard $(1)),,$(error [FATAL] Missing file: "$(1)" - $(2)))

$(call _require_dir,$(TOOL_PATH),Toolchain not found)
```

### 9.2 Error in Rezepten

```makefile
# Errorprüfung nach Befehl
$(OUT)/app.out: $(OBJS)
	@"$(LD)" -o "$@" $(OBJS)
	@[ -f "$@" ] || (echo "[FATAL] Link failed: $@"; exit 2)
```

### 9.3 Errormeldungs-Format

```
[FATAL] <Description> - <Kontext>
[WARN]  <Description>
[INFO]  <Description>
```

---

## 10. Diagnose-Targets

### 10.1 Check-Target Pattern

```makefile
.PHONY: check-<aspect>

check-<aspect>:
	@echo "*** Check <Aspect> ***"
	@echo "**********************"
	@echo "VAR1= $(VAR1)"
	@echo "VAR2= $(VAR2)"
	@echo ""
```

### 10.2 Meta-Target

```makefile
.PHONY: check-all

check-all: check-arguments check-paths check-outputs check-abi
	@echo "*** All checks completed ***"
```

### 10.3 Variable Inspection

```makefile
print-%:
	@echo '$*=$($*)'
```

---

## 11. Shell-Befehle

### 11.1 Unterdrücken von Echo

```makefile
# @ unterdrückt Echo des Befehls
@echo [CC] $<
@$(CC) $(CFLAGS) -o "$@" "$<"
```

### 11.2 Mehrere Befehle

```makefile
# ; oder && für Verkettung
target:
	@echo "Step 1"; echo "Step 2"
	@cd $(DIR) && $(MAKE)
```

### 11.3 Bedingte Ausführung

```makefile
clean:
	@if [ -d "$(OUT_DIR)" ]; then rm -rf "$(OUT_DIR)"; fi
```

### 11.4 Shell-Variable vs Make-Variable

```makefile
# $$ für Shell-Variable
target:
	@for f in *.c; do echo "$$f"; done
```

---

## 12. Portabilität

### 12.1 Pfad-Normalisierung

```makefile
# \ zu / und /.. auflösen
fwd      = $(subst \,/,$(1))
normpath = $(strip $(call fwd,$(abspath $(call fwd,$(1)))))

override PATH_VAR := $(call normpath,$(PATH_VAR))
```

### 12.2 Shell-Configuration

```makefile
# Explizite Shell setzen für Portabilität
SHELL := /bin/sh
.SHELLFLAGS := -c
```

### 12.3 Tool-Pfade

```makefile
# Tools über Variablen, nicht hardcoded
MKDIR := mkdir
RM    := rm

# Oder aus Toolchain ableiten
MKDIR := $(CYGWIN_PATH)/mkdir
RM    := $(CYGWIN_PATH)/rm
```

---

## 13. See Also

- [Makefile_Blueprint.md](../blueprints/Makefile_Blueprint.md) — Strukturvorlage
- [Makefile_Build_System_Concept.md](Makefile_Build_System_Concept.md) — Reference-Implementation
- [CMake_Standard.md](CMake_Standard.md) — CMake-Konventionen

---

## 14. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Basierend auf TMS320 Build-System Best Practices** |
