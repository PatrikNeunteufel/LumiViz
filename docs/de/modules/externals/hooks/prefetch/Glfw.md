# PreFetch/glfw.cmake — GLFW PreFetch Hook

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [glfw_PreFetch.md](../../en/modules/externals/glfw_PreFetch.md)  
> **Hook:** [cmake/externals/hooks/prefetch/glfw.cmake](../../../cmake/externals/hooks/prefetch/glfw.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PreFetch Hook?](#2-warum-ein-prefetch-hook)
3. [Gesetzte Optionen](#3-gesetzte-optionen)
4. [Solution.json Konfiguration](#4-solutionjson-konfiguration)
5. [Erstellte Targets](#5-erstellte-targets)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Der `glfw.cmake` PreFetch Hook konfiguriert GLFW vor dem Build, um unnötige Komponenten zu deaktivieren.

---

## 2. Warum ein PreFetch Hook?

**Ohne Hook:** 20+ zusätzliche Targets im Solution Explorer (Examples, Tests, Docs).

**Mit Hook:** Nur `glfw` und `update_mappings` Targets.

---

## 3. Gesetzte Optionen

| Option | Wert | Beschreibung |
|--------|------|--------------|
| `GLFW_BUILD_EXAMPLES` | `OFF` | Keine Example-Programme |
| `GLFW_BUILD_TESTS` | `OFF` | Keine Test-Programme |
| `GLFW_BUILD_DOCS` | `OFF` | Keine Dokumentation |
| `GLFW_INSTALL` | `OFF` | Kein Install-Target |

---

## 4. Solution.json Konfiguration

```json
{
    "glfw": {
        "git": "https://github.com/glfw/glfw.git",
        "tag": "3.4"
    }
}
```

---

## 5. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `glfw` | STATIC | GLFW Library |

---

## 6. Siehe auch

- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [imgui_PostFetch.md](imgui_PostFetch.md) — ImGui Hook (verwendet glfw)

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
