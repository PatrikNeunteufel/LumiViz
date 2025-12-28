# Externals — Library-spezifische Guides

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../../en/userguides/externals/README.md)

---

## Quick-Start

**Audio-Anwendung?**
1. [Bass.md](Bass.md) — BASS Audio Library

**GUI-Anwendung?**
1. [Glfw.md](Glfw.md) — GLFW für Window/Input
2. [Imgui.md](Imgui.md) — Dear ImGui für UI
3. [Qt6.md](Qt6.md) — Qt6 Framework

**OpenGL-Rendering?**
1. [Glad.md](Glad.md) — OpenGL Loader
2. [Glfw.md](Glfw.md) — OpenGL Context

**Scripting einbauen?**
1. [Lua54.md](Lua54.md) — Lua 5.4 Scripting Engine

**Tests schreiben?**
1. [Doctest.md](Doctest.md) — Schnell und einfach (empfohlen)
2. [Googletest.md](Googletest.md) — Umfangreich mit Mocking
3. [Catch2.md](Catch2.md) — BDD-Style

---

## Overview

Library-spezifische User Guides zeigen, wie einzelne externe Libraries in Projekten genutzt werden. Jeder Guide enthält Setup, Configuration und Examples.

---

## Dateien

### Audio

| Guide | Library | Typ |
|-------|---------|-----|
| [Bass.md](Bass.md) | BASS Audio Library | Local |

### GUI

| Guide | Library | Typ |
|-------|---------|-----|
| [Glfw.md](Glfw.md) | GLFW | Git (fetched) |
| [Imgui.md](Imgui.md) | Dear ImGui | Git (fetched) |
| [Qt6.md](Qt6.md) | Qt6 Framework | Local |

### OpenGL

| Guide | Library | Typ |
|-------|---------|-----|
| [Glad.md](Glad.md) | GLAD OpenGL Loader | Local |

### Scripting

| Guide | Library | Typ |
|-------|---------|-----|
| [Lua54.md](Lua54.md) | Lua 5.4 | Local |

### Testing

| Guide | Library | Typ | Empfehlung |
|-------|---------|-----|------------|
| [Doctest.md](Doctest.md) | doctest | Git/Local | ⭐ Standard |
| [Googletest.md](Googletest.md) | Google Test | Git | Für komplexe Tests |
| [Catch2.md](Catch2.md) | Catch2 | Git | Für BDD-Style |

---

## See Also

- [../README.md](../README.md) — User Guides Overview
- [../Externals.md](../Externals.md) — Externals Overview
- [../Adding_Externals.md](../Adding_Externals.md) — Neue External hinzufügen
- [../../references/externals/](../../references/externals/README.md) — Externals-Reference
- [../../modules/externals/](../../modules/externals/README.md) — Externals-Module
