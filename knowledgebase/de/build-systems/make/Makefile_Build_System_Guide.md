# Makefile Build System — Guide

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Guide  
> **Status:** In Entwicklung  
> **Zielgruppe:** Embedded-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Makefile_Build_System_Guide.md](../../../en/build-systems/make/Makefile_Build_System_Guide.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [Verzeichnisstruktur](#3-verzeichnisstruktur)
4. [Module](#4-module)
5. [Verwendung](#5-verwendung)
6. [Konfiguration](#6-konfiguration)
7. [Best Practices](#7-best-practices)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Dieses Guide beschreibt ein modulares **Makefile-basiertes Build-System** für Embedded-Projekte, entwickelt für TI C2000 Dual-Core Mikrocontroller, aber auf andere Plattformen übertragbar.

### Ziele

| Ziel | Beschreibung |
|------|--------------|
| **Maximale Kontrolle** | Volle Transparenz über Build-Prozess |
| **Reproduzierbar** | Unabhängig von IDE-Versionen |
| **IDE-unabhängig** | CCS, VS Code, Command Line |
| **Dual-Core** | Unterstützung für CPU1 + CLA, CPU2 |

### Kernprinzipien

1. **Modularer Aufbau** — Gemeinsame Funktionalität in `common/`, projektspezifisch separat
2. **Argument-basiert** — Keine hardcoded Pfade, alles via Make-Argumente
3. **Fail-Fast** — Klare Fehlermeldungen bei fehlenden Argumenten
4. **Shared Library** — Gemeinsamer Code in Library, on-demand gebaut

---

## 2. Architektur

### 2.1 Build-Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                          Build-Einstieg                          │
│  make -f Master_CLA_Build/Makefile CCS_PATH=... TI_VERSION=...  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│ common/arguments.mk    │  Validiert Pflicht-Argumente           │
│ common/shellSetup.mk   │  Konfiguriert POSIX-Shell (rm, mkdir)  │
│ common/config.mk       │  Globale Konfiguration (ABI, RTS)      │
│ common/paths.mk        │  Include-Pfade, C2000Ware              │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│ Master_CLA/sources.mk  │  Projekt-spezifische Quellen           │
│ Master_CLA/flags.mk    │  Projekt-spezifische Compiler-Flags    │
│ Master_CLA/rules.mk    │  Build-Regeln (.c → .obj → .out)       │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                          Output                                  │
│  out/Master_CLA.out, obj/*.obj, dep/*.d                         │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Dependency Graph

```
Master_CLA.out ──────────────────────────────────────────────────►
       │                                                          │
       ├── Master_CLA/*.obj (CPU1-spezifisch)                     │
       │                                                          │
       └── Global.lib ◄─────────────────────────────────────────  │
               │                                                  │
               └── Global/*.obj (Shared Code)                     │
                                                                  │
Slave.out ────────────────────────────────────────────────────────►
       │                                                          │
       ├── Slave/*.obj (CPU2-spezifisch)                          │
       │                                                          │
       └── Global.lib ◄───────────────────────────────────────────┘
```

---

## 3. Verzeichnisstruktur

```
Build/
├── Global_Build/           # Build-Einstieg für Global-Library
│   └── Makefile
│
├── Master_CLA_Build/       # Build-Einstieg für CPU1 + CLA
│   └── Makefile
│
├── Slave_Build/            # Build-Einstieg für CPU2
│   └── Makefile
│
└── make/
    ├── common/             # Gemeinsame Module
    │   ├── help.mk         # Diagnose-Targets
    │   ├── arguments.mk    # Argument-Validierung
    │   ├── shellSetup.mk   # Shell-Konfiguration
    │   ├── config.mk       # Globale Konfiguration
    │   ├── paths.mk        # Include-Pfade
    │   ├── outputs.mk      # Output-Verzeichnisse
    │   ├── tools.mk        # Compiler-Definitionen
    │   └── rules.mk        # Pattern-Rules
    │
    ├── Global/             # Global-Library-spezifisch
    │   ├── sources.mk
    │   └── flags.mk
    │
    ├── Master_CLA/         # Master_CLA-spezifisch
    │   ├── sources.mk
    │   └── flags.mk
    │
    └── Slave/              # Slave-spezifisch
        ├── sources.mk
        └── flags.mk
```

---

## 4. Module

### 4.1 common/arguments.mk

Validiert Pflicht-Argumente mit Fail-Fast:

```makefile
# Pflicht-Argumente
REQUIRED_ARGS := CCS_PATH TI_VERSION PROJECT_ROOT

# Prüfung
$(foreach arg,$(REQUIRED_ARGS),\
    $(if $($(arg)),,$(error ERROR: $(arg) not defined. Use: make $(arg)=...)))
```

### 4.2 common/shellSetup.mk

Konfiguriert POSIX-kompatible Shell-Tools:

```makefile
# CCS bringt Cygwin mit
SHELL := $(CCS_PATH)/utils/cygwin/bin/bash.exe
RM := rm -rf
MKDIR := mkdir -p
```

### 4.3 common/paths.mk

Definiert Include-Pfade:

```makefile
C2000WARE := $(CCS_PATH)/ccs_base/c2000ware_$(TI_VERSION)

INCLUDES := \
    -I$(PROJECT_ROOT)/source \
    -I$(C2000WARE)/driverlib/f28p65x/driverlib \
    -I$(C2000WARE)/device_support/f28p65x/common/include
```

### 4.4 project/sources.mk

Listet Projekt-Quellen:

```makefile
# CPU1-spezifische Quellen
SOURCES := \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/hal/gpio.c \
    $(SRC_DIR)/app/control.c

# CLA-Quellen (spezielles Handling)
CLA_SOURCES := \
    $(SRC_DIR)/cla/cla_tasks.cla
```

---

## 5. Verwendung

### 5.1 Build starten

```bash
# Vollständiger Build
make -f Master_CLA_Build/Makefile \
    CCS_PATH="C:/ti/ccs1271" \
    TI_VERSION="5_02_00_00" \
    PROJECT_ROOT="$(pwd)"

# Nur Global-Library
make -f Global_Build/Makefile \
    CCS_PATH="C:/ti/ccs1271" \
    TI_VERSION="5_02_00_00" \
    PROJECT_ROOT="$(pwd)"
```

### 5.2 Clean

```bash
make -f Master_CLA_Build/Makefile clean
```

### 5.3 Diagnose

```bash
# Konfiguration anzeigen
make -f Master_CLA_Build/Makefile show-config

# Alle Quellen anzeigen
make -f Master_CLA_Build/Makefile show-sources
```

---

## 6. Konfiguration

### 6.1 Compiler-Flags

```makefile
# common/config.mk
ABI := eabi
OPT_LEVEL := 2
DEBUG := 1

# project/flags.mk
CFLAGS := \
    -v28 \
    -ml \
    -mt \
    --abi=$(ABI) \
    -O$(OPT_LEVEL) \
    $(if $(DEBUG),-g,)
```

### 6.2 Linker-Konfiguration

```makefile
LDFLAGS := \
    -z \
    --stack_size=0x200 \
    --heap_size=0x100 \
    -m"$(OUT_DIR)/$(TARGET).map"

LIBS := \
    $(GLOBAL_LIB) \
    $(RTS_LIB)
```

---

## 7. Best Practices

### 7.1 Keine hardcoded Pfade

```makefile
# ❌ Schlecht
INCLUDES := -IC:/ti/c2000ware/...

# ✅ Gut
INCLUDES := -I$(C2000WARE)/...
```

### 7.2 Fail-Fast

```makefile
# Früh prüfen, nicht erst beim Linken scheitern
ifeq ($(wildcard $(LINKER_CMD)),)
    $(error Linker command file not found: $(LINKER_CMD))
endif
```

### 7.3 Dependency-Tracking

```makefile
# Automatische Header-Dependencies
DEPFLAGS = -MMD -MP -MF $(DEP_DIR)/$*.d

# Include generierte Dependencies
-include $(DEPS)
```

### 7.4 Parallel Build

```makefile
# Aktiviere parallele Kompilierung
.PHONY: all
all:
	$(MAKE) -j$(shell nproc) $(TARGET)
```

---

## 8. Siehe auch

- [CMake_Migration_Guide.md](../cmake/CMake_Migration_Guide.md) — Alternative: CMake
- [EPWM_Configuration_Reference.md](../../embedded/ti-c2000/EPWM_Configuration_Reference.md) — TI C2000 Peripherals
- [GNU Make Manual](https://www.gnu.org/software/make/manual/) — Offizielle Referenz

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Konsolidiert aus BuildSystem_makefile Dokumenten** |
