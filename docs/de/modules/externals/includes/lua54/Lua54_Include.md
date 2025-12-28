# lua54/Include.cmake — Lua 5.4 Scripting Engine Integration

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler, C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [lua54_Include.md](../../en/modules/externals/includes/lua54_Include.md)  
> **Modul:** [cmake/externals/includes/lua54/Include.cmake](../../../../cmake/externals/includes/lua54/Include.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Verfügbare Variablen](#2-verfügbare-variablen)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Options](#4-options)
5. [Plattform-Unterstützung](#5-plattform-unterstützung)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Verzeichnisstruktur](#7-verzeichnisstruktur)
8. [Fehlerbehandlung](#8-fehlerbehandlung)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

Die `lua54/Include.cmake` integriert die **Lua 5.4 Scripting Engine** als lokales External in das Build-System.

### Kernfunktionen

| Funktion | Beschreibung |
|----------|--------------|
| Embedded Scripting | Lua in C++ Anwendungen einbetten |
| Konfiguration | Lua-Dateien als Konfigurationsformat |
| Erweiterbar | C-Funktionen nach Lua exportieren |
| Lightweight | Minimaler Footprint (~300KB) |

### Target-Erstellung

Erstellt ein `STATIC` Target `lua54` das die Lua-Quellen kompiliert.

---

## 2. Verfügbare Variablen

Diese Variablen werden vom Orchestrator bereitgestellt:

| Variable | Beschreibung |
|----------|--------------|
| `EXTERNAL_NAME` | `"lua54"` |
| `EXTERNAL_PATH` | Absoluter Pfad zu `externals/lua54` |
| `EXTERNAL_JSON` | JSON-Element aus Solution.json |
| `EXTERNAL_OPTIONS` | Target-spezifische Options (JSON) |

---

## 3. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `lua54` | STATIC | Lua 5.4 Library (aus Quellen kompiliert) |

### Include-Directories

```cmake
target_include_directories(lua54 PUBLIC
    "${EXTERNAL_PATH}/src"
)
```

---

## 4. Options

| Option | Typ | Default | Beschreibung |
|--------|-----|---------|--------------|
| `LUA_EMBEDDED` | bool | `true` | Statisch linken (empfohlen) |
| `LUA_32BITS` | bool | `false` | 32-bit Integer/Float |
| `LUA_USE_C89` | bool | `false` | C89-Kompatibilität |

### 4.1 LUA_EMBEDDED

Wenn `true` (Default), wird Lua statisch gelinkt. Dies ist die empfohlene Einstellung für eingebettete Anwendungen.

### 4.2 LUA_32BITS

Für ressourcenbeschränkte Systeme. Verwendet 32-bit Integer und Float statt 64-bit.

---

## 5. Plattform-Unterstützung

### 5.1 Kompilierung aus Quellen

Da Lua aus Quellen kompiliert wird, funktioniert es auf allen Plattformen:

| Plattform | Status | Hinweise |
|-----------|--------|----------|
| Windows | ✅ | MSVC, Clang, MinGW |
| Linux | ✅ | GCC, Clang |
| macOS | ✅ | Clang, GCC |

### 5.2 Plattform-Defines

```cmake
if(UNIX AND NOT APPLE)
    target_compile_definitions(lua54 PRIVATE LUA_USE_LINUX)
elseif(APPLE)
    target_compile_definitions(lua54 PRIVATE LUA_USE_MACOSX)
endif()
```

---

## 6. Verwendungsbeispiele

### 6.1 Basis-Verwendung

**Solution.json:**

```json
{
    "externals": {
        "lua54": { "path": "externals/lua54" }
    },
    "executables": [
        {
            "name": "ScriptedApp",
            "externals": ["lua54"]
        }
    ]
}
```

**C++:**

```cpp
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    luaL_dostring(L, "print('Hello from Lua!')");
    
    lua_close(L);
    return 0;
}
```

### 6.2 Konfigurationsdatei laden

**config.lua:**

```lua
config = {
    window_width = 1920,
    window_height = 1080,
    fullscreen = false,
    title = "My Game"
}
```

**C++:**

```cpp
lua_State* L = luaL_newstate();
luaL_openlibs(L);

if (luaL_dofile(L, "config.lua") == LUA_OK) {
    lua_getglobal(L, "config");
    
    lua_getfield(L, -1, "window_width");
    int width = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, -1, "fullscreen");
    bool fullscreen = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, -1, "title");
    const char* title = lua_tostring(L, -1);
    lua_pop(L, 2);
    
    // Verwende width, fullscreen, title...
}

lua_close(L);
```

### 6.3 C-Funktion nach Lua exportieren

**C++:**

```cpp
// Funktion die von Lua aufgerufen werden kann
int lua_add(lua_State* L) {
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    lua_pushnumber(L, a + b);
    return 1;  // Ein Rückgabewert
}

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    // Funktion registrieren
    lua_pushcfunction(L, lua_add);
    lua_setglobal(L, "add");
    
    // In Lua verwenden
    luaL_dostring(L, "print(add(2, 3))");  // Ausgabe: 5
    
    lua_close(L);
    return 0;
}
```

---

## 7. Verzeichnisstruktur

```
externals/lua54/
└── src/
    ├── lua.h
    ├── luaconf.h
    ├── lualib.h
    ├── lauxlib.h
    ├── lapi.c
    ├── lcode.c
    ├── ldebug.c
    ├── ldo.c
    ├── lfunc.c
    ├── lgc.c
    ├── llex.c
    ├── lmem.c
    ├── lobject.c
    ├── lopcodes.c
    ├── lparser.c
    ├── lstate.c
    ├── lstring.c
    ├── ltable.c
    ├── ltm.c
    ├── lundump.c
    ├── lvm.c
    ├── lzio.c
    ├── lauxlib.c
    ├── lbaselib.c
    ├── lcorolib.c
    ├── ldblib.c
    ├── liolib.c
    ├── lmathlib.c
    ├── loadlib.c
    ├── loslib.c
    ├── lstrlib.c
    ├── ltablib.c
    └── lutf8lib.c
```

---

## 8. Fehlerbehandlung

| Code | Beschreibung |
|------|--------------|
| E213 | Include.cmake nicht gefunden |
| E214 | External-Pfad existiert nicht |

### Lua Runtime-Fehler

```cpp
if (luaL_dofile(L, "script.lua") != LUA_OK) {
    const char* error = lua_tostring(L, -1);
    std::cerr << "Lua Error: " << error << std::endl;
    lua_pop(L, 1);
}
```

---

## 9. Siehe auch

- [Externals_Reference.md](../../../reference/Externals_Reference.md) — Alle Externals
- [Externals_UserGuide.md](../../../guides/Externals_UserGuide.md) — Verwendungsanleitung
- [Attach_cmake.md](../Attach_cmake.md) — Local External Handler
- [Lua 5.4 Reference Manual](https://www.lua.org/manual/5.4/) — Offizielle Dokumentation

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0, Convention-Pfad `cmake/externals/includes/lua54/`** |
| 0.1.0 | 2025-12-05 | Initial: Lua 5.4 aus Quellen kompiliert |
