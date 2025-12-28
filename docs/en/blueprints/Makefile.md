# Makefile Blueprint

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** Stable  
> **Target Audience:** Build System Developers  
> **Language:** English

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Datei-Struktur](#2-datei-struktur)
3. [Modul-Template](#3-modul-template)
4. [Haupt-Makefile-Template](#4-haupt-makefile-template)
5. [Diagnose-Target-Template](#5-diagnose-target-template)
6. [Best Practices](#6-best-practices)
7. [Anti-Patterns](#7-anti-patterns)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

Dieses Blueprint definiert die Struktur und Konventionen für modulare Makefiles. Es basiert auf dem TMS320F28P65x Build-System und ist für ähnliche Embedded-Projekte anwendbar.

### 1.1 Ziele

- **Modularität** — Wiederverwendbare, unabhängige Module
- **Transparenz** — Klare Diagnose-Möglichkeiten
- **Robustheit** — Fail-Fast bei Configurationsfehlern
- **Wartbarkeit** — Einheitliche Struktur und Konventionen

---

## 2. Datei-Struktur

### 2.1 Empfohlene Verzeichnisstruktur

```
Build/
├── <Project1>_Build/          /* Einstiegspunkt Projekt 1       */
│   └── Makefile
├── <Project2>_Build/          /* Einstiegspunkt Projekt 2       */
│   └── Makefile
└── make/
    ├── common/                /* Gemeinsame Module              */
    │   ├── bootstrap.mk       /* Basis-Functions               */
    │   ├── arguments.mk       /* Argument-Validierung           */
    │   ├── config.mk          /* Globale Configuration          */
    │   ├── paths.mk           /* Pfad-Definitionen              */
    │   ├── outputs.mk         /* Output-Verzeichnisse           */
    │   ├── toolchain.mk       /* Compiler-Configuration         */
    │   └── diagnostics.mk     /* Diagnose-Targets               */
    ├── <Project1>/            /* Projekt-spezifisch             */
    │   ├── config.mk
    │   ├── sources.mk
    │   └── rules.mk
    └── <Project2>/
        ├── config.mk
        ├── sources.mk
        └── rules.mk
```

### 2.2 Modul-Kategorien

| Kategorie | Verantwortlichkeit | Examples |
|-----------|-------------------|-----------|
| **Bootstrap** | Basis-Functions, Argument-Prüfung | `bootstrap.mk`, `arguments.mk` |
| **Configuration** | Pfade, Toolchain, ABI | `config.mk`, `paths.mk`, `toolchain.mk` |
| **Outputs** | Verzeichnisse, Artefakte | `outputs.mk` |
| **Rules** | Compile-/Link-Regeln | `rules.mk` |
| **Diagnostics** | Debug-/Check-Targets | `diagnostics.mk` |

---

## 3. Modul-Template

### 3.1 Basis-Struktur

```makefile
# <module_name>.mk - <kurze Description>
# <optionale Details>
# !!!!!!!!!!!!!!!!!!!!!!!!! DON'T TOUCH WITHOUT ANY GOOD REASON !!!!!!!!!!!!!!!!!!!!!!
$(info *** parsing <module_name>.mk ***)
$(info )

# ==== <Section 1> ================================================================
# <Description der Section>

VARIABLE1 := value1
VARIABLE2 := value2

# ==== <Section 2> ================================================================

# ... weitere Definitionen ...

# Export für nachfolgende Module/rekursive Aufrufe
export VARIABLE1 VARIABLE2

# ==== Diagnose-Target ============================================================
.PHONY: check-<module>

check-<module>:
	@echo "*** Check <Module> ***"
	@echo "**********************"
	@echo "VARIABLE1= $(VARIABLE1)"
	@echo "VARIABLE2= $(VARIABLE2)"
	@echo ""
```

### 3.2 Hilfs-Functions

```makefile
# Pfad-Normalisierung (\ zu /, /.. auflösen)
fwd      = $(subst \,/,$(1))
normpath = $(strip $(call fwd,$(abspath $(call fwd,$(1)))))

# Existenz-Prüfungen
_require_dir  = $(if $(wildcard $(1)/.),,$(error [FATAL] Missing directory: "$(1)" - $(2)))
_require_file = $(if $(wildcard $(1)),,$(error [FATAL] Missing file: "$(1)" - $(2)))
```

---

## 4. Haupt-Makefile-Template

```makefile
# Makefile for <Project>
# ######################

# Bootstrap (if not using common/bootstrap.mk)
fwd      = $(subst \,/,$(1))
normpath = $(strip $(call fwd,$(abspath $(call fwd,$(1)))))

# ==== Argument validation ========================================================
ProjectDir ?= Unknown
ifeq ($(ProjectDir),Unknown)
  $(error [FATAL] Argument ProjectDir was not provided!)
endif
override ProjectDir := $(call normpath,$(ProjectDir))

# ==== Include common modules (order is critical!) ================================
include ${ProjectDir}/Build/make/common/arguments.mk
include ${ProjectDir}/Build/make/common/config.mk
include ${ProjectDir}/Build/make/common/paths.mk
include $(ProjectDir)/Build/make/common/outputs.mk
include $(ProjectDir)/Build/make/common/toolchain.mk
include ${ProjectDir}/Build/make/common/diagnostics.mk

# ==== Include project-specific modules ===========================================
include ${ProjectDir}/Build/make/<Project>/config.mk
include ${ProjectDir}/Build/make/<Project>/sources.mk
include ${ProjectDir}/Build/make/<Project>/rules.mk
```

---

## 5. Diagnose-Target-Template

### 5.1 Einzelnes Check-Target

```makefile
.PHONY: check-<aspect>

check-<aspect>:
	@echo "*** Check <Aspect> ***"
	@echo "**********************"
	@echo "VAR1= $(VAR1)"
	@echo "VAR2= $(VAR2)"
	@echo ""
```

### 5.2 Meta-Target (check-all)

```makefile
# diagnostics.mk
.PHONY: check-all

check-all: check-arguments check-paths check-outputs check-abi check-rules
	@echo "*** All checks completed ***"
```

### 5.3 Ad-hoc Variable Inspection

```makefile
# In rules.mk oder diagnostics.mk
print-%:
	@echo '$*=$($*)'
```

---

## 6. Best Practices

### 6.1 Variablen

| Regel | Example |
|-------|----------|
| **UPPER_CASE** für exportierte Variablen | `CG_TOOL_ROOT`, `CFLAGS` |
| **Sofortige Expansion (`:=`)** für Pfade | `OUT_DIR := $(ProjectDir)/out` |
| **Verzögerte Expansion (`=`)** für abhängige Werte | `GLOBAL_LIB = $(OUT_DIR)/lib.a` |
| **Override** für Argument-Normalisierung | `override ProjectDir := $(call normpath,...)` |

### 6.2 Kommentare

```makefile
# ==== Section Header (4 = signs, 80 chars total) =================================
# Einzelne Zeile erklärt Zweck

# Mehrzeilige Erklärung für komplexe Logik:
# - Punkt 1
# - Punkt 2
```

### 6.3 Errorbehandlung

```makefile
# Fail-Fast Pattern
ifeq ($(REQUIRED_VAR),)
  $(error [FATAL] REQUIRED_VAR is empty or not set)
endif

# Validierung mit Kontext
$(call _require_dir,$(TOOL_PATH),Toolchain not found - check TI_BASE)
```

### 6.4 Shell-Befehle

```makefile
# Immer mit @ für saubere Ausgabe
@echo [CC] $<

# Mit SHELL für Portabilität
@$(SHELL) -c 'if [ -d "$(DIR)" ]; then rm -rf "$(DIR)"; fi'

# Errorprüfung nach Befehl
@[ -f "$@" ] || (echo "[FATAL] Compilation failed"; exit 2)
```

---

## 7. Anti-Patterns

### 7.1 Vermeiden

| Anti-Pattern | Problem | Besser |
|--------------|---------|--------|
| Hardcoded Pfade | Nicht portabel | Argumente oder abgeleitete Pfade |
| Tabs vs Spaces inkonsistent | Make erfordert Tabs in Rezepten | Konsequent Tabs in Rezepten |
| Globale Variablen ohne Export | Rekursive Aufrufe scheitern | `export VAR` |
| Fehlende `.PHONY` | Konflikte mit gleichnamigen Dateien | Immer deklarieren |
| Keine Diagnose-Targets | Debugging erschwert | `check-*` Targets |

### 7.2 Examples

```makefile
# ❌ Schlecht: Hardcoded Pfad
CC := D:/TI/CCS1281/ccs/tools/compiler/bin/cl2000

# ✅ Gut: Abgeleiteter Pfad
CC := $(CG_TOOL_ROOT)/bin/cl2000

# ❌ Schlecht: Keine Errorprüfung
$(CC) $(CFLAGS) -o $@ $<

# ✅ Gut: Mit Errorprüfung
@"$(CC)" $(CFLAGS) -o "$@" "$<"
@[ -f "$@" ] || (echo "[FATAL] Compilation failed: $<"; exit 2)
```

---

## 8. See Also

- [Makefile_Standard.md](../standards/Makefile_Standard.md) — Coding-Konventionen
- [Makefile_Build_System_Concept.md](Makefile_Build_System_Concept.md) — Reference-Implementation

---

## 9. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Basierend auf TMS320 Build-System** |
