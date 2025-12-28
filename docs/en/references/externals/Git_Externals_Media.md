# Git Externals: Media & Math — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Git_Externals_Media.md](../../en/references/externals/Git_Externals_Media.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Audio](#2-audio)
3. [Image](#3-image)
4. [Math / Geometry](#4-math--geometry)
5. [Entity Component Systems](#5-entity-component-systems)
6. [Schnellreferenz](#6-schnellreferenz)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

This document describes Bibliotheken für Audio, Bild-Verarbeitung und Mathematik.

### Kategorien

| Kategorie | Bibliotheken |
|-----------|--------------|
| Audio | miniaudio, openal-soft |
| Image | stb |
| Math | glm, Eigen |
| ECS | entt |

---

## 2. Audio

### 2.1 miniaudio

> **Zweck:** Single-file Audio-Bibliothek für Playback, Capture und Mixing.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/mackron/miniaudio.git` |
| **Aktueller Tag** | `0.11.21` |
| **CMake Support** | ❌ (Single-header) |
| **Hook** | PostFetch erforderlich 🔧 |

```json
"miniaudio": {
    "git": "https://github.com/mackron/miniaudio.git",
    "tag": "0.11.21",
    "cmakeSupport": false
}
```

**PostFetch Hook (miniaudio.cmake):**
```cmake
add_library(${HOOK_EXTERNAL_NAME} INTERFACE)
target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE "${HOOK_SOURCE_DIR}")
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
message(STATUS "[${HOOK_EXTERNAL_NAME}] Note: Define MINIAUDIO_IMPLEMENTATION in ONE .cpp file")
```

**Usage:**
```cpp
// In EINER .cpp Datei:
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

// Audio initialisieren
ma_device_config config = ma_device_config_init(ma_device_type_playback);
config.playback.format = ma_format_f32;
config.playback.channels = 2;
config.sampleRate = 48000;
config.dataCallback = data_callback;

ma_device device;
ma_device_init(NULL, &config, &device);
ma_device_start(&device);
```

---

### 2.2 openal-soft

> **Zweck:** Software-Implementation der OpenAL 3D Audio API.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/kcat/openal-soft.git` |
| **Aktueller Tag** | `1.23.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"openal-soft": {
    "git": "https://github.com/kcat/openal-soft.git",
    "tag": "1.23.1"
}
```

**PreFetch Hook (openal-soft.cmake):**
```cmake
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <AL/al.h>
#include <AL/alc.h>

ALCdevice* device = alcOpenDevice(NULL);
ALCcontext* context = alcCreateContext(device, NULL);
alcMakeContextCurrent(context);

ALuint buffer, source;
alGenBuffers(1, &buffer);
alGenSources(1, &source);
// Load audio data into buffer...
alSourcei(source, AL_BUFFER, buffer);
alSourcePlay(source);
```

---

## 3. Image

### 3.1 stb

> **Zweck:** Single-file Libraries für Bilder, Fonts, Vorbis und mehr.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/nothings/stb.git` |
| **Aktueller Tag** | – (branch: master) |
| **CMake Support** | ❌ (Single-headers) |
| **Hook** | PostFetch erforderlich 🔧 |

```json
"stb": {
    "git": "https://github.com/nothings/stb.git",
    "branch": "master",
    "cmakeSupport": false
}
```

**PostFetch Hook (stb.cmake):**
```cmake
add_library(${HOOK_EXTERNAL_NAME} INTERFACE)
target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE "${HOOK_SOURCE_DIR}")
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
message(STATUS "[${HOOK_EXTERNAL_NAME}] Note: Define STB_*_IMPLEMENTATION in ONE .cpp file")
```

**Verfügbare Header:**
- `stb_image.h` - Bild laden (JPG, PNG, BMP, GIF, ...)
- `stb_image_write.h` - Bild speichern
- `stb_image_resize2.h` - Bild skalieren
- `stb_truetype.h` - TrueType Font Rendering
- `stb_vorbis.c` - Ogg Vorbis Audio

**Usage:**
```cpp
// In EINER .cpp Datei:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// Bild laden
int width, height, channels;
unsigned char* data = stbi_load("image.png", &width, &height, &channels, 0);

// Verarbeiten...

// Bild speichern
stbi_write_png("output.png", width, height, channels, data, width * channels);
stbi_image_free(data);
```

---

## 4. Math / Geometry

### 4.1 glm (OpenGL Mathematics)

> **Zweck:** Header-only Mathematik-Bibliothek für Grafik-Programmierung (GLSL-ähnliche Syntax).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/g-truc/glm.git` |
| **Aktueller Tag** | `1.0.1` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |

```json
"glm": {
    "git": "https://github.com/g-truc/glm.git",
    "tag": "1.0.1"
}
```

**PreFetch Hook (glm.cmake):**
```cmake
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 position(1.0f, 2.0f, 3.0f);
glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
glm::mat4 view = glm::lookAt(eye, center, up);
glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
glm::mat4 mvp = proj * view * model;
```

---

### 4.2 Eigen

> **Zweck:** Template-Bibliothek für Lineare Algebra (Matrizen, Vektoren, Solver).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://gitlab.com/libeigen/eigen.git` |
| **Aktueller Tag** | `3.4.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |
| **Header-only** | ✅ |

```json
"eigen": {
    "git": "https://gitlab.com/libeigen/eigen.git",
    "tag": "3.4.0"
}
```

**PreFetch Hook (eigen.cmake):**
```cmake
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <Eigen/Dense>

Eigen::Matrix3f A;
A << 1, 2, 3,
     4, 5, 6,
     7, 8, 10;

Eigen::Vector3f b(3, 3, 4);
Eigen::Vector3f x = A.colPivHouseholderQr().solve(b);

// Auch: SVD, LU, Cholesky, Eigenvalues, ...
```

---

## 5. Entity Component Systems

### 5.1 entt

> **Zweck:** Schnelle, moderne Entity Component System (ECS) Bibliothek.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/skypjack/entt.git` |
| **Aktueller Tag** | `v3.13.2` |
| **CMake Support** | ✅ |
| **Hook** | – |
| **Header-only** | ✅ |

```json
"entt": {
    "git": "https://github.com/skypjack/entt.git",
    "tag": "v3.13.2"
}
```

**Usage:**
```cpp
#include <entt/entt.hpp>

struct Position { float x, y; };
struct Velocity { float dx, dy; };

entt::registry registry;

auto entity = registry.create();
registry.emplace<Position>(entity, 0.0f, 0.0f);
registry.emplace<Velocity>(entity, 1.0f, 1.0f);

// Update system
auto view = registry.view<Position, Velocity>();
for (auto [entity, pos, vel] : view.each()) {
    pos.x += vel.dx * dt;
    pos.y += vel.dy * dt;
}
```

---

## 6. Schnellreferenz

| Bibliothek | Tag | CMake | Hook | Hauptverwendung |
|------------|-----|-------|------|-----------------|
| miniaudio | 0.11.21 | ❌ | PostFetch 🔧 | Cross-platform audio |
| openal-soft | 1.23.1 | ✅ | PreFetch | 3D audio API |
| stb | master | ❌ | PostFetch 🔧 | Image loading/saving |
| glm | 1.0.1 | ✅ | PreFetch | Graphics math |
| Eigen | 3.4.0 | ✅ | PreFetch | Linear algebra |
| entt | v3.13.2 | ✅ | – | Entity Component System |

---

## 7. See Also

- [Git_Externals_Reference.md](Git_Externals.md) — Hauptübersicht
- [Git_Externals_GUI.md](Git_Externals_GUI.md) — GUI & Graphics
- [Externals.md](../Externals.md) — Local Externals (BASS Audio)

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Initial: Ausgelagert aus Git_Externals_Reference** |
