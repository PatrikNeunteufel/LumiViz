# User Guides — Benutzerhandbücher

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Sprache:** Deutsch  
> **English:** [README.md](../../en/userguides/README.md)

---

## Quick-Start

**Neu im Projekt?**
1. [Getting_Started.md](Getting_Started.md) — Erste Schritte, Projekt einrichten
2. [../references/Glossar.md](../references/Glossar.md) — Begriffe klären

**Externe Libraries nutzen?**
1. [Externals.md](Externals.md) — Übersicht Externals-System
2. [Adding_Externals.md](Adding_Externals.md) — Neue External hinzufügen
3. [externals/](externals/README.md) — Library-spezifische Guides

**Tests schreiben?**
1. [Testing.md](Testing.md) — Test-Framework einrichten und nutzen
2. [externals/Doctest.md](externals/Doctest.md) — doctest Guide
3. [externals/Googletest.md](externals/Googletest.md) — GoogleTest Guide

**Build konfigurieren?**
1. [CMakeUserPresets.md](CMakeUserPresets.md) — Eigene Presets erstellen
2. [../references/CMakePresets.md](../references/CMakePresets.md) — Preset-Referenz

**Qt6 nutzen?**
1. [Qt6_Integration.md](Qt6_Integration.md) — Qt6 einrichten
2. [externals/Qt6.md](externals/Qt6.md) — Qt6 Details

---

## Übersicht

User Guides sind praktische Anleitungen für konkrete Aufgaben. Sie folgen dem [Guide Blueprint](../blueprints/Guide.md) und sind auf Endnutzer ausgerichtet.

---

## Dateien

| Datei | Beschreibung |
|-------|--------------|
| [Getting_Started.md](Getting_Started.md) | Erste Schritte — Projekt klonen, konfigurieren, bauen |
| [Externals.md](Externals.md) | Externals nutzen — Git + Local Libraries einbinden |
| [Adding_Externals.md](Adding_Externals.md) | Neue External hinzufügen — Schritt-für-Schritt |
| [Testing.md](Testing.md) | Tests schreiben — Framework einrichten, Tests erstellen |
| [CMakeUserPresets.md](CMakeUserPresets.md) | CMakeUserPresets.json — Eigene Build-Konfigurationen |
| [Qt6_Integration.md](Qt6_Integration.md) | Qt6 Integration — Setup und Verwendung |

---

## Unterordner

| Ordner | Dateien | Beschreibung |
|--------|---------|--------------|
| [externals/](externals/README.md) | 9 | Library-spezifische Guides |

---

## Library-Guides (Direktzugriff)

### Audio

| Guide | Beschreibung |
|-------|--------------|
| [externals/Bass.md](externals/Bass.md) | BASS Audio Library — Setup, Plugins, Beispiele |

### GUI

| Guide | Beschreibung |
|-------|--------------|
| [externals/Glfw.md](externals/Glfw.md) | GLFW — Window/Input Library |
| [externals/Imgui.md](externals/Imgui.md) | Dear ImGui — Immediate Mode GUI |
| [externals/Qt6.md](externals/Qt6.md) | Qt6 — Framework-Integration |

### OpenGL

| Guide | Beschreibung |
|-------|--------------|
| [externals/Glad.md](externals/Glad.md) | GLAD — OpenGL Loader |

### Scripting

| Guide | Beschreibung |
|-------|--------------|
| [externals/Lua54.md](externals/Lua54.md) | Lua 5.4 — Scripting Engine |

### Testing

| Guide | Beschreibung |
|-------|--------------|
| [externals/Doctest.md](externals/Doctest.md) | doctest — Schnelles Testing |
| [externals/Googletest.md](externals/Googletest.md) | Google Test — Umfangreiches Testing |
| [externals/Catch2.md](externals/Catch2.md) | Catch2 — BDD-Style Testing |

---

## Siehe auch

- [../README.md](../README.md) — Deutsche Dokumentation Übersicht
- [../blueprints/Guide.md](../blueprints/Guide.md) — Guide Blueprint
- [../references/](../references/README.md) — Nachschlagewerke
