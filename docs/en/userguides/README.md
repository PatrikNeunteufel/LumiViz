# User Guides — Benutzerhandbücher

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../en/userguides/README.md)

---

## Quick-Start

**Neu im Projekt?**
1. [Getting_Started.md](Getting_Started.md) — Erste Schritte, Projekt einrichten
2. [../references/Glossar.md](../references/Glossar.md) — Begriffe klären

**Externe Libraries nutzen?**
1. [Externals.md](Externals.md) — Overview Externals-System
2. [Adding_Externals.md](Adding_Externals.md) — Neue External hinzufügen
3. [externals/](externals/README.md) — Library-spezifische Guides

**Tests schreiben?**
1. [Testing.md](Testing.md) — Test-Framework einrichten und nutzen
2. [externals/Doctest.md](externals/Doctest.md) — doctest Guide
3. [externals/Googletest.md](externals/Googletest.md) — GoogleTest Guide

**Build konfigurieren?**
1. [CMakeUserPresets.md](CMakeUserPresets.md) — Eigene Presets erstellen
2. [../references/CMakePresets.md](../references/CMakePresets.md) — Preset-Reference

**Qt6 nutzen?**
1. [Qt6_Integration.md](Qt6_Integration.md) — Qt6 einrichten
2. [externals/Qt6.md](externals/Qt6.md) — Qt6 Details

---

## Overview

User Guides sind praktische Anleitungen für konkrete Aufgaben. Sie folgen dem [Guide Blueprint](../blueprints/Guide.md) und sind auf End Users ausgerichtet.

---

## Dateien

| Datei | Description |
|-------|--------------|
| [Getting_Started.md](Getting_Started.md) | Erste Schritte — Projekt klonen, konfigurieren, bauen |
| [Externals.md](Externals.md) | Externals nutzen — Git + Local Libraries einbinden |
| [Adding_Externals.md](Adding_Externals.md) | Neue External hinzufügen — Schritt-für-Schritt |
| [Testing.md](Testing.md) | Tests schreiben — Framework einrichten, Tests erstellen |
| [CMakeUserPresets.md](CMakeUserPresets.md) | CMakeUserPresets.json — Eigene Build-Configurationen |
| [Qt6_Integration.md](Qt6_Integration.md) | Qt6 Integration — Setup und Usage |

---

## Unterordner

| Ordner | Dateien | Description |
|--------|---------|--------------|
| [externals/](externals/README.md) | 9 | Library-spezifische Guides |

---

## Library-Guides (Direktzugriff)

### Audio

| Guide | Description |
|-------|--------------|
| [externals/Bass.md](externals/Bass.md) | BASS Audio Library — Setup, Plugins, Examples |

### GUI

| Guide | Description |
|-------|--------------|
| [externals/Glfw.md](externals/Glfw.md) | GLFW — Window/Input Library |
| [externals/Imgui.md](externals/Imgui.md) | Dear ImGui — Immediate Mode GUI |
| [externals/Qt6.md](externals/Qt6.md) | Qt6 — Framework-Integration |

### OpenGL

| Guide | Description |
|-------|--------------|
| [externals/Glad.md](externals/Glad.md) | GLAD — OpenGL Loader |

### Scripting

| Guide | Description |
|-------|--------------|
| [externals/Lua54.md](externals/Lua54.md) | Lua 5.4 — Scripting Engine |

### Testing

| Guide | Description |
|-------|--------------|
| [externals/Doctest.md](externals/Doctest.md) | doctest — Schnelles Testing |
| [externals/Googletest.md](externals/Googletest.md) | Google Test — Umfangreiches Testing |
| [externals/Catch2.md](externals/Catch2.md) | Catch2 — BDD-Style Testing |

---

## See Also

- [../README.md](../README.md) — Germane Dokumentation Overview
- [../blueprints/Guide.md](../blueprints/Guide.md) — Guide Blueprint
- [../references/](../references/README.md) — Nachschlagewerke
