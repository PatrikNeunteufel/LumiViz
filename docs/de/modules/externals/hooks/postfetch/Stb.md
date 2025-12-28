# stb.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-23  
> **Typ:** ModuleDoc  
> **Status:** Stabil  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.1  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **Modul:** [`cmake/externals/hooks/postfetch/stb.cmake`](../../../../cmake/externals/hooks/postfetch/stb.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [Hook-Variablen](#4-hook-variablen)
5. [Erstellte Targets](#5-erstellte-targets)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Verfügbare Header](#7-verfügbare-header)
8. [Best Practices](#8-best-practices)
9. [Einschränkungen](#9-einschränkungen)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Der `stb.cmake` PostFetch Hook erstellt ein INTERFACE-Target für die **stb** Header-Only Library von Sean Barrett. Da stb kein eigenes CMakeLists.txt mitbringt, übernimmt dieser Hook die Target-Erstellung.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Target-Erstellung | Erstellt `stb` INTERFACE-Library |
| Include-Pfade | Setzt Include-Directory auf Source-Root |
| Registrierung | Registriert Target im Externals-System |

### Kurzbeschreibung

```
stb = Single-file public domain libraries for C/C++
     ├── stb_image.h        → Image loading (PNG, JPG, BMP, TGA, GIF, HDR, PIC, PNM)
     ├── stb_image_write.h  → Image writing
     ├── stb_image_resize.h → Image resizing
     └── ... (20+ weitere Header)
```

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
|-------|---------|------------|
| HookLoader.cmake | 0.5.0+ | Stellt Hook-Variablen bereit |
| Registry/Targets.cmake | — | `_register_external_target()` Funktion |

---

## 3. Konzept

### Warum ein PostFetch Hook?

stb ist eine Sammlung von **Header-Only** Libraries ohne Build-System. Das CMake Architecture erwartet jedoch ein Target für jedes External. Der PostFetch Hook schließt diese Lücke:

```
FetchContent_Declare(stb ...)
    ↓
FetchContent_MakeAvailable(stb)
    ↓
[Kein CMakeLists.txt vorhanden!]
    ↓
PostFetch Hook wird ausgeführt
    ↓
Hook erstellt INTERFACE-Target
    ↓
Target wird registriert
```

### INTERFACE vs STATIC

Da stb Header-Only ist, wird ein `INTERFACE`-Target verwendet:

| Aspekt | INTERFACE | STATIC |
|--------|-----------|--------|
| Kompilierung | Keine (nur Header) | Eigene .lib/.a |
| Link-Typ | Nur Include-Propagation | Vollständige Linkage |
| Für stb | ✅ Korrekt | ❌ Unnötig |

---

## 4. Hook-Variablen

Diese Variablen werden vom HookLoader bereitgestellt:

| Variable | Beschreibung | Beispielwert |
|----------|--------------|--------------|
| `HOOK_EXTERNAL_NAME` | Name des Externals | `stb` |
| `HOOK_SOURCE_DIR` | Pfad zum Source-Verzeichnis | `${CMAKE_SOURCE_DIR}/.externals/stb` |
| `HOOK_EXTERNAL_JSON` | JSON-Definition (optional) | `{"git": "...", ...}` |

---

## 5. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `stb` | INTERFACE | Haupt-Target mit Include-Directory |

### Target-Eigenschaften

```cmake
# Automatisch gesetzt durch Hook:
target_include_directories(stb INTERFACE "${HOOK_SOURCE_DIR}")
```

### Verwendung in Executables

```json
// Solution.json
{
    "externals": ["stb"]
}
```

```cmake
# Automatisch gelinkt:
target_link_libraries(MyTarget PRIVATE stb)
```

---

## 6. Verwendungsbeispiele

### Im C++ Code

```cpp
// stb_image - Implementation in EINER .cpp Datei definieren
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// In anderen Dateien nur Header inkludieren
#include <stb_image.h>

// Bild laden
int width, height, channels;
unsigned char* data = stbi_load("image.png", &width, &height, &channels, 0);
if (data) {
    // ... verwenden ...
    stbi_image_free(data);
}
```

### Wichtig: Implementation-Makro

stb Header benötigen ein `#define *_IMPLEMENTATION` in **genau einer** .cpp Datei:

| Header | Makro |
|--------|-------|
| stb_image.h | `STB_IMAGE_IMPLEMENTATION` |
| stb_image_write.h | `STB_IMAGE_WRITE_IMPLEMENTATION` |
| stb_image_resize.h | `STB_IMAGE_RESIZE_IMPLEMENTATION` |

---

## 7. Verfügbare Header

Die wichtigsten stb Header:

| Header | Beschreibung | Use Case |
|--------|--------------|----------|
| `stb_image.h` | Bildladen (PNG, JPG, BMP, etc.) | Textur-Loading |
| `stb_image_write.h` | Bildschreiben | Screenshots |
| `stb_image_resize.h` | Bildskalierung | Mipmap-Generation |
| `stb_truetype.h` | TrueType Font Parsing | Text-Rendering |
| `stb_rect_pack.h` | Rechteck-Packing | Texture Atlases |
| `stb_perlin.h` | Perlin Noise | Prozedurale Texturen |

Vollständige Liste: https://github.com/nothings/stb

---

## 8. Best Practices

### ✅ Empfohlen

```cpp
// Implementation in dedizierter Datei (z.B. stb_impl.cpp)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
```

```cpp
// Andere Dateien: Nur Header
#include <stb_image.h>  // Kein #define!
```

### ❌ Vermeiden

```cpp
// FALSCH: Implementation in Header-Datei
// stb_wrapper.hpp
#define STB_IMAGE_IMPLEMENTATION  // ❌ Mehrfach-Inklusion!
#include <stb_image.h>
```

```cpp
// FALSCH: Implementation in mehreren .cpp
// file1.cpp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// file2.cpp
#define STB_IMAGE_IMPLEMENTATION  // ❌ Linker-Fehler!
#include <stb_image.h>
```

---

## 9. Einschränkungen

| Einschränkung | Beschreibung |
|---------------|--------------|
| Keine Versionierung | stb verwendet `master` Branch |
| Single-Implementation | `*_IMPLEMENTATION` darf nur einmal definiert werden |
| C-Style API | Keine RAII, manuelle Speicherverwaltung |

---

## 10. Siehe auch

| Dokument | Beschreibung |
|----------|--------------|
| [HookLoader.cmake](../HookLoader.md) | Hook-System Dokumentation |
| [Handler.cmake](../../fetched/Handler.md) | Fetched External Handler |
| [OpenGLTexture.cpp](../../../../projects/apps/MyVisualizer/src/gpu/OpenGLTexture.cpp) | Verwendungsbeispiel |

### Externe Ressourcen

- [stb GitHub Repository](https://github.com/nothings/stb)
- [stb_image Documentation](https://github.com/nothings/stb/blob/master/stb_image.h)

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 0.1.0 | 2025-12-23 | Initiale Version für MyVisualizer Phase 2 |

---

*Dokumentation erstellt nach Blueprint v0.5.0*
