# Local Externals — Scripting

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Local_Externals_Scripting.md](../../en/references/externals/Local_Externals_Scripting.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Lua 5.4 Scripting Engine](#2-lua-54-scripting-engine)
3. [See Also](#3-siehe-auch)
4. [Changelog](#4-changelog)

---

## 1. Overview

This document describes lokale Scripting-Externals für das CMake Architecture Build-System.

| Library | Description | Lizenz |
|---------|--------------|--------|
| **lua54** | Lua 5.4 Scripting Engine | MIT |

---

## 2. Lua 5.4 Scripting Engine

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Typ** | 📝 Source-basiert |
| **Pfad** | `externals/lua54` |
| **Include.cmake** | `cmake/externals/includes/lua54/Include.cmake` |
| **Plattformen** | Alle |
| **Website** | [lua.org](https://www.lua.org/) |

### Warum Lua?

| Vorteil | Description |
|---------|--------------|
| **Lightweight** | Minimaler Footprint (~300KB) |
| **Embeddable** | Einfach in C++ einzubetten |
| **Flexibel** | Configuration, Modding, Scripting |
| **Schnell** | Sehr performante VM |

### Solution.json

```json
{
    "externals": {
        "lua54": {
            "path": "externals/lua54"
        }
    },
    "executables": [
        {
            "name": "ScriptedApp",
            "externals": ["lua54"]
        }
    ]
}
```

### Verzeichnisstruktur

```
externals/lua54/
└── src/
    ├── lua.h
    ├── luaconf.h
    ├── lualib.h
    ├── lauxlib.h
    ├── lapi.c
    ├── lcode.c
    ├── ... (weitere .c Dateien)
    └── lutf8lib.c
```

### Options

| Option | Typ | Default | Description |
|--------|-----|---------|--------------|
| `LUA_EMBEDDED` | bool | `true` | Statisch linken (empfohlen) |
| `LUA_32BITS` | bool | `false` | 32-bit Integer/Float |
| `LUA_USE_C89` | bool | `false` | C89-Kompatibilität |

### Usagesbeispiel

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

### Configurationsdatei laden

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
}

lua_close(L);
```

### C-Funktion nach Lua exportieren

```cpp
// Funktion die von Lua aufgerufen werden kann
int lua_add(lua_State* L) {
    double a = luaL_checknumber(L, 1);
    double b = luaL_checknumber(L, 2);
    lua_pushnumber(L, a + b);
    return 1;  // Ein Return Value
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

### Kombination mit sol2 (Git)

Für komfortablere C++ Bindings empfiehlt sich sol2:

```json
{
    "externals": {
        "lua54": { "path": "externals/lua54" },
        "sol2": { "git": "https://github.com/ThePhD/sol2.git", "tag": "v3.3.1" }
    }
}
```

```cpp
#include <sol/sol.hpp>

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    
    // Einfacher!
    lua["add"] = [](int a, int b) { return a + b; };
    
    lua.script("print(add(2, 3))");
    
    return 0;
}
```

### Anwendungsfälle

| Anwendung | Description |
|-----------|--------------|
| **Configuration** | Lua-Dateien statt JSON/XML |
| **Modding** | Spieler können Spiele erweitern |
| **Scripting** | Gameplay-Logik in Lua |
| **Automation** | Build-Skripte, Tools |

### Vergleich mit Git-Alternativen

| Feature | lua54 (Local) | sol2 (Git) | pybind11 (Git) |
|---------|---------------|------------|----------------|
| **Sprache** | Lua | Lua (C++ Wrapper) | Python |
| **Footprint** | ~300KB | Header-Only | Größer |
| **C++ Integration** | Manual | Automatisch | Automatisch |
| **Anwendung** | Core Engine | C++ Bindings | Python Bindings |

### Detail-Dokumentation

→ [lua54_Include.md](../../modules/externals/includes/lua54/Lua54_Include.md)

---

## 3. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Local_Externals.md](Local_Externals.md) — Local Externals Overview
- [Git_Externals_Scripting.md](Git_Externals_Scripting.md) — Git Scripting-Externals (sol2, pybind11)
- [Lua 5.4 Reference Manual](https://www.lua.org/manual/5.4/) — Offizielle Dokumentation

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Eigene Kategorie Scripting (parallel zu Git)** |
