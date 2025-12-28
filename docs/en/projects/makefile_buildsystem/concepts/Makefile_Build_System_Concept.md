# Makefile Build System — Architecture

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Concept  
> **Status:** Stable  
> **Target Audience:** Build System Developers, Firmware-Architekten  
> **Scope:** Makefile-basierte Embedded-Build-Systeme  
> **Language:** English

---

## Table of Contents

1. [Vision und Einleitung](#1-vision-und-einleitung)
2. [Kernprinzipien](#2-kernprinzipien)
3. [Verzeichnisstruktur](#3-verzeichnisstruktur)
4. [Modul-Architecture](#4-modul-architektur)
5. [Include-Reihenfolge](#5-include-reihenfolge)
6. [Datenfluss und Variablen](#6-datenfluss-und-variablen)
7. [Build-Configurationen](#7-build-konfigurationen)
8. [Diagnose-System](#8-diagnose-system)
9. [Dependency-Management](#9-dependency-management)
10. [Shared Library Pattern](#10-shared-library-pattern)
11. [Design-Entscheidungen](#11-design-entscheidungen)
12. [See Also](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Vision und Einleitung

This document describes die Architecture eines modularen Makefile-basierten Build-Systems für Dual-Core Mikrocontroller-Firmware (TMS320F28P65x). Das System verwendet handgeschriebene Makefiles für maximale Kontrolle und Reproduzierbarkeit.

### 1.1 Ziele

| Ziel | Description |
|------|--------------|
| **Maximale Kontrolle** | Volle Transparenz über jeden Build-Schritt |
| **Reproduzierbarkeit** | Identische Builds unabhängig von IDE-Version |
| **IDE-Unabhängigkeit** | CCS, VS Code, Command Line — alle funktionieren |
| **Dual-Core-Support** | CPU1 + CLA und CPU2 als separate Targets |
| **Modularer Aufbau** | Wiederverwendbare Module, klare Trennung |

### 1.2 Nicht-Ziele

- Kompatibilität mit IDE-generierten Makefiles
- Automatische IDE-Integration
- Cross-Platform-Support (fokussiert auf Windows + CCS-Cygwin)

---

## 2. Kernprinzipien

### 2.1 Modularer Aufbau

Strikte Trennung zwischen gemeinsamer und projekt-spezifischer Funktionalität:

```
common/     → Wiederverwendbare Module
<project>/  → Projekt-spezifische Configuration
```

### 2.2 Argument-basierte Configuration

Alle variablen Pfade als Make-Argumente. **Keine hardcodierten Pfade.**

```makefile
// Argumente von außen
make all Project=Master_CLA Config=Debug \
    ProjectDir=/path/to/project \
    TIBase=D:/TI ...
```

### 2.3 Fail-Fast

Bei fehlenden Argumenten oder Pfaden **sofortiger Abbruch** mit klarer Meldung:

```makefile
ifeq ($(Project),Unknown)
  $(error [FATAL] Argument Project was not provided!)
endif
```

### 2.4 Shared Library Pattern

Gemeinsamer Code in Library, CPU-spezifische Projekte linken dagegen. On-Demand-Build für automatische Dependency-Auflösung.

---

## 3. Verzeichnisstruktur

```
Build/
├── Global_Build/              // Build-Einstieg für Global-Library
│   └── Makefile
├── Master_CLA_Build/          // Build-Einstieg für CPU1 + CLA
│   └── Makefile
├── Slave_Build/               // Build-Einstieg für CPU2
│   └── Makefile
└── make/
    ├── common/                // Gemeinsame Module
    │   ├── bootstrap.mk       // Basis-Functions (normpath, etc.)
    │   ├── arguments.mk       // Argument-Validierung
    │   ├── shell_setup.mk     // Shell-Configuration
    │   ├── config.mk          // ABI/RTS-Configuration
    │   ├── paths.mk           // Pfad-Definitionen
    │   ├── outputs.mk         // Output-Verzeichnisse
    │   ├── toolchain.mk       // Compiler-Flags
    │   └── diagnostics.mk     // Diagnose-Targets
    ├── Global/                // Global-spezifisch
    ├── Master_CLA/            // Master_CLA-spezifisch
    └── Slave/                 // Slave-spezifisch
```

---

## 4. Modul-Architecture

### 4.1 Gemeinsame Module (common/)

| Modul | Verantwortlichkeit |
|-------|-------------------|
| **bootstrap.mk** | `normpath`, `_require_dir`, `_require_file` |
| **arguments.mk** | Required-Argumente validieren, Fail-Fast |
| **shell_setup.mk** | POSIX-Shell, Tools (rm, mkdir) |
| **config.mk** | ABI (EABI/COFF), RTS-Basis |
| **paths.mk** | Include-Pfade, C2000Ware, Compiler |
| **outputs.mk** | Output-Verzeichnisse, clean-Targets |
| **toolchain.mk** | Compiler-Flags, RTS-Library |
| **diagnostics.mk** | check-all Meta-Target |

### 4.2 Projekt-spezifische Module

| Modul | Verantwortlichkeit |
|-------|-------------------|
| **config.mk** | INCLUDE_DIRS, LINKER_CMD, MEM |
| **sources.mk** | C_SOURCES, ASM_SOURCES, OBJS, DEPS |
| **rules.mk** | Compile-/Link-Regeln, all-Target |

---

## 5. Include-Reihenfolge

> **Kritisch:** Die Reihenfolge ist essentiell für korrekte Variablen-Auflösung.

```makefile
// 1. Bootstrap (Hilfs-Functions)
include common/bootstrap.mk

// 2. Argument-Validierung (Fail-Fast)
include common/arguments.mk

// 3. Shell-Configuration
include common/shell_setup.mk

// 4. Globale Configuration
include common/config.mk
include common/paths.mk
include common/outputs.mk
include common/toolchain.mk

// 5. Diagnose
include common/diagnostics.mk

// 6. Projekt-spezifisch
include $(Project)/config.mk
include $(Project)/sources.mk
include $(Project)/rules.mk
```

### 5.1 Abhängigkeitsgraph

```
bootstrap.mk → normpath, _require_dir
      ↓
arguments.mk → Project, Config, TIBase validiert
      ↓
shell_setup.mk → SHELL, MKDIR, RM
      ↓
config.mk → FORCE_COFF, RTS_BASE
      ↓
paths.mk → CG_TOOL_ROOT, CC, AR, LD
      ↓
outputs.mk → OUT_DIR, OBJ_DIR, GLOBAL_LIB
      ↓
toolchain.mk → CFLAGS, ABI_SWITCH, RTS_PATH
      ↓
<project>/*.mk → Finale Build-Regeln
```

---

## 6. Datenfluss und Variablen

### 6.1 Eingabe-Argumente

| Argument | Description | Example |
|----------|--------------|----------|
| `Project` | Projektname | `Global`, `Master_CLA`, `Slave` |
| `Config` | Build-Configuration | `Debug`, `Release` |
| `ProjectDir` | Wurzelverzeichnis | `/path/to/project` |
| `DeviceFamily` | Device-Familie | `f28p65x` |
| `TIBase` | TI-Installation | `D:/TI` |
| `CCSVersion` | CCS-Version | `CCS1281` |
| `C2000WareVersion` | C2000Ware | `C2000Ware_6_00_00_00` |
| `CompilerVersion` | Compiler | `ti-cgt-c2000_22.6.2.LTS` |

### 6.2 Abgeleitete Pfade

| Variable | Ableitung |
|----------|-----------|
| `C2000WareRoot` | `$(TIBase)/C2000/$(C2000WareVersion)` |
| `CG_TOOL_ROOT` | `$(TIBase)/$(CCSVersion)/ccs/tools/compiler/$(CompilerVersion)` |
| `DRVLIB_DIR` | `$(C2000WareRoot)/driverlib/$(DeviceFamily)/driverlib` |

### 6.3 Export-Pattern

```makefile
// Importante Variablen exportieren für rekursive Aufrufe
export CG_TOOL_ROOT
export ABI_SUFFIX ABI_SWITCH
export OUT_DIR OBJ_DIR DEP_DIR
```

---

## 7. Build-Configurationen

| Eigenschaft | Debug | Release |
|-------------|-------|---------|
| **MEM** | RAM | FLASH |
| **Linker-Script** | `*_ram_*.cmd` | `*_flash_*.cmd` |
| **Defines** | `DEBUG`, `_RAM` | `NDEBUG`, `_FLASH` |
| **Debug-Symbole** | Yes | No |
| **Optimierung** | Keine | `--opt_level=2` |
| **Persistenz** | Nicht persistent | Persistent |

---

## 8. Diagnose-System

### 8.1 Diagnose-Targets

| Target | Zeigt |
|--------|-------|
| `check-arguments` | Build-Argumente |
| `check-paths` | Include-, Toolchain-Pfade |
| `check-outputs` | Output-Verzeichnisse |
| `check-shell` | Shell-Configuration |
| `check-abi` | ABI, RTS-Library |
| `check-rules` | Compiler-Flags |
| **`check-all`** | **Alle kombiniert** |

### 8.2 Ad-hoc Inspection

```bash
make print-CFLAGS
make print-INCLUDE_DIRS
```

---

## 9. Dependency-Management

Automatische Dependency-Generierung:

```makefile
// Compiler generiert .d-Dateien
$(CC) --preproc_dependency="$(DEP_DIR)/file.d" ...

// Einbinden
-include $(DEPS)
```

---

## 10. Shared Library Pattern

### 10.1 On-Demand Build

```makefile
$(GLOBAL_LIB):
    @echo [MAKE] Building Global library
    @$(MAKE) -s -C "$(ProjectDir)/Build/Global_Build" \
        Project=Global Config=$(Config) ... all
```

### 10.2 Link-Inputs

```makefile
LD_INPUTS := $(OBJS) $(GLOBAL_LIB) $(EXTRA_LIBS) $(RTS_PATH)
```

---

## 11. Design-Entscheidungen

### 11.1 Warum CCS-Cygwin?

Windows `cmd.exe` unterstützt keine POSIX-Befehle. CCS-Cygwin garantiert konsistentes Verhalten.

### 11.2 Warum EABI als Default?

EABI ist der moderne Standard. COFF nur für Legacy (`FORCE_COFF=1`).

### 11.3 Warum keine IDE-generierten Makefiles?

Keine Kontrolle, schwer zu debuggen, IDE-Versions-abhängig.

---

## 12. See Also

- [Makefile_Build_System_Guide.md](Makefile_Build_System_Guide.md) — Anwendung
- [Makefile_Build_System_Future_Enhancements.md](Makefile_Build_System_Future_Enhancements.md) — Verbesserungen
- [Makefile_Standard.md](../standards/Makefile_Standard.md) — Coding-Konventionen
- [Makefile_Blueprint.md](../blueprints/Makefile_Blueprint.md) — Modul-Templates

---

## 13. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Blueprint-konform, basierend auf TMS320 Build-System** |
