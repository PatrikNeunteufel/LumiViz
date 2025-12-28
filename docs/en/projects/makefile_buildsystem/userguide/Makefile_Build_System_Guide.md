# Makefile Build System — Anwendung

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Firmware-Entwickler  
> **Based on:** Makefile_Build_System_Concept v1.0.0  
> **Language:** English

---

## Table of Contents

1. [Einführung](#1-einführung)
2. [Prerequisites](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [IDE-Configuration](#4-ide-konfiguration)
5. [Aufgaben](#5-aufgaben)
6. [Troubleshooting](#6-troubleshooting)
7. [Reference](#7-referenz)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Einführung

Diese Anleitung beschreibt die Einrichtung und Usage des Makefile-basierten Build-Systems. Sie richtet sich an Entwickler, die Firmware bauen möchten, ohne sich mit Makefile-Interna zu befassen.

### 1.1 Was macht das Build-System?

- Kompiliert C, ASM und CLA Quellcode
- Baut drei Targets: Global (Library), Master_CLA (CPU1), Slave (CPU2)
- Unterstützt Debug (RAM) und Release (Flash)
- Bietet umfassende Diagnose-Tools

---

## 2. Prerequisites

### 2.1 Software

| Software | Version | Notee |
|----------|---------|----------|
| **Code Composer Studio** | 12.x+ | Empfohlen: aktuelle LTS |
| **C2000Ware** | Passend | z.B. C2000Ware_6_00_00_00 |
| **TI C2000 Compiler** | LTS | z.B. ti-cgt-c2000_22.6.2.LTS |

### 2.2 Erwartete Installation

```
D:\TI\
├── CCS<Version>\
│   └── ccs\tools\compiler\
│       └── ti-cgt-c2000_X.X.X\
└── C2000\
    └── C2000Ware_X_XX_XX\
        └── driverlib\f28p65x\
```

---

## 3. Schnellstart

```bash
# 1. Projekt-Verzeichnis wechseln
cd /path/to/TMS320F28/Build/Master_CLA_Build

# 2. Build starten
make all Project=Master_CLA Config=Debug \
    ProjectDir=/path/to/TMS320F28 \
    DeviceFamily=f28p65x \
    TIBase=D:/TI \
    CCSVersion=CCS1281 \
    C2000WareVersion=C2000Ware_6_00_00_00 \
    CompilerVersion=ti-cgt-c2000_22.6.2.LTS

# 3. Diagnose (optional)
make check-all ...
```

---

## 4. IDE-Configuration

### 4.1 CCS Builder Settings

**Properties → Build → Builder Settings:**

| Einstellung | Wert |
|-------------|------|
| Builder type | **External builder** |
| Generate Makefiles | **Deaktiviert** |
| Build directory | `${workspace_loc:/TMS320F28/Build/<Proj>_Build}` |

### 4.2 Build Command

```
all -k -j 8 Project=${ProjName} Config=${ConfigName} \
ProjectDir=${PROJECT_DIR} DeviceFamily=${DEVICE_FAMILY} \
TIBase=${TI_BASE} CCSVersion=${CCSVERSION} \
C2000WareVersion=${C2000WARE_VER} CompilerVersion=${COMPILER_VER}
```

### 4.3 Umgebungsvariablen

**Properties → Build → Environment:**

| Variable | Example |
|----------|----------|
| `TI_BASE` | `D:\TI` |
| `CCSVERSION` | `CCS1281` |
| `COMPILER_VER` | `ti-cgt-c2000_22.6.2.LTS` |
| `C2000WARE_VER` | `C2000Ware_6_00_00_00` |
| `DEVICE_FAMILY` | `f28p65x` |
| `PROJECT_DIR` | `${CCS_PROJECT_DIR}/..` |

> **Important:** "Append variables to native environment" aktivieren!

---

## 5. Aufgaben

### 5.1 Projekt bauen

```bash
make all Project=Master_CLA Config=Debug ...
```

**Output:**
```
Build/<Project>_Build/<Config>/out/
├── <Project>.out     // Executable
├── <Project>.map     // Memory Map
└── linkinfo.xml      // Link-Informationen
```

### 5.2 Clean Build

```bash
make clean all Project=Master_CLA Config=Debug ...
```

### 5.3 Nur Diagnose

```bash
make check-all Project=Master_CLA Config=Debug ...
```

### 5.4 Debug ↔ Release wechseln

| Configuration | Usage |
|---------------|------------|
| **Debug** | RAM, nicht persistent, Entwicklung |
| **Release** | Flash, persistent, Produktion |

### 5.5 Toolchain aktualisieren

1. Umgebungsvariablen anpassen
2. `make check-paths` ausführen
3. Build testen

---

## 6. Troubleshooting

### 6.1 Diagnose-Workflow

```
1. make check-all ...
       ↓
2. Error analysieren
       ↓
3. Spezifisches check-* Target
       ↓
4. Variable/Pfad korrigieren
       ↓
5. Build wiederholen
```

### 6.2 Häufige Error

| Error | Ursache | Lösung |
|--------|---------|--------|
| `Argument XYZ was not provided` | Variable fehlt | Environment prüfen |
| `Missing directory` | Pfad falsch | `check-paths` |
| `CreateProcess failed` | Shell-Problem | `check-shell` |
| `driverlib.lib not found` | C2000Ware nicht gebaut | driverlib für Config bauen |

### 6.3 Variable inspizieren

```bash
make print-CFLAGS Project=...
make print-INCLUDE_DIRS Project=...
```

---

## 7. Reference

### 7.1 Targets

| Target | Description |
|--------|--------------|
| `all` | Standard-Build |
| `clean` | Artefakte löschen |
| `dirs` | Verzeichnisse erstellen |
| `check-all` | Alle Diagnosen |
| `print-%` | Variable ausgeben |

### 7.2 Diagnose-Targets

| Target | Zeigt |
|--------|-------|
| `check-arguments` | Build-Argumente |
| `check-paths` | Include-, Toolchain-Pfade |
| `check-outputs` | Output-Verzeichnisse |
| `check-shell` | Shell-Configuration |
| `check-abi` | ABI, RTS-Library |
| `check-rules` | Compiler-Flags |

### 7.3 Required-Argumente

| Argument | Description |
|----------|--------------|
| `Project` | `Global`, `Master_CLA`, `Slave` |
| `Config` | `Debug`, `Release` |
| `ProjectDir` | Absoluter Pfad |
| `DeviceFamily` | z.B. `f28p65x` |
| `TIBase` | TI-Root |
| `CCSVersion` | z.B. `CCS1281` |
| `C2000WareVersion` | z.B. `C2000Ware_6_00_00_00` |
| `CompilerVersion` | z.B. `ti-cgt-c2000_22.6.2.LTS` |

### 7.4 Optionale Argumente

| Argument | Default | Description |
|----------|---------|--------------|
| `C2000WareRoot` | (abgeleitet) | Direkter Pfad |
| `FORCE_COFF` | `0` | `1` für COFF statt EABI |
| `USE_DRIVERLIB` | `1` | `0` deaktiviert |

---

## 8. See Also

- [Makefile_Build_System_Concept.md](Makefile_Build_System_Concept.md) — Architecture
- [Makefile_Build_System_Future_Enhancements.md](Makefile_Build_System_Future_Enhancements.md) — Verbesserungen

---

## 9. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **1.0.0** | **2025-12-19** | **Initial: Blueprint-konform** |
