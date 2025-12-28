# Cpp — Standard für C++/C Dateistruktur

> **Version:** 1.0.0  
> **Datum:** 2025-12-13  
> **Typ:** Blueprint  
> **Status:** In Entwicklung  
> **Basiert auf:** Blueprint v0.5  
> **Zielgruppe:** C++ Entwickler, Build-System-Entwickler  
> **Geltungsbereich:** Alle .cpp, .hpp, .tpp, .h, .c Dateien  
> **Sprache:** Deutsch  
> **English:** [Cpp.md](../../en/blueprints/Cpp.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abgrenzung](#2-abgrenzung)
3. [Dateitypen](#3-dateitypen)
4. [Datei-Header](#4-datei-header)
5. [Header-Datei-Struktur (.hpp/.h)](#5-header-datei-struktur-hpph)
6. [Source-Datei-Struktur (.cpp/.c)](#6-source-datei-struktur-cppc)
7. [Template-Datei-Struktur (.tpp)](#7-template-datei-struktur-tpp)
8. [Include-Reihenfolge](#8-include-reihenfolge)
9. [Klassen-Layout](#9-klassen-layout)
10. [Dokumentationskommentare](#10-dokumentationskommentare)
11. [Beispiele](#11-beispiele)
12. [Review-Checkliste](#12-review-checkliste)
13. [Siehe auch](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Übersicht

Dieser Blueprint definiert die **formale Struktur** für C++/C Code-Dateien. Er legt fest, wie Dateien aufgebaut sein müssen — unabhängig vom Coding-Stil.

### Ziele

- Einheitliche Datei-Header mit Metadaten
- Konsistente Dateistruktur
- Vorhersagbare Include-Reihenfolge
- Klare Klassen-Organisation

### Was dieser Blueprint regelt

| Bereich | Inhalt |
|---------|--------|
| Datei-Header | Copyright, Version, Beschreibung |
| Dateistruktur | Reihenfolge der Abschnitte |
| Includes | Sortierung, Gruppierung |
| Klassen-Layout | Reihenfolge der Member |

---

## 2. Abgrenzung

| Dokument | Regelt |
|----------|--------|
| **Cpp Blueprint** (dieses) | Formaler Aufbau, Datei-Header, Struktur |
| **Cpp_Coding_Standard** | Namenskonventionen, Stil, Sprachfeatures |
| **ClangFormat_Blueprint** | Automatische Formatierung |
| **ClangTidy_Blueprint** | Statische Analyse, Warnungen |

**Beispiel:**
- Cpp Blueprint: "Jede Datei beginnt mit einem Header-Kommentar"
- Cpp_Coding_Standard: "Klassen heißen `PascalCase`"

---

## 3. Dateitypen

### 3.1 Übersicht

| Extension | Typ | Beschreibung |
|-----------|-----|--------------|
| `.hpp` | C++ Header | Deklarationen, Inline-Code |
| `.cpp` | C++ Source | Implementierungen |
| `.tpp` | C++ Template | Template-Implementierungen |
| `.h` | C Header | Reine C-Deklarationen |
| `.c` | C Source | Reine C-Implementierungen |

### 3.2 Wann welche Extension?

| Situation | Extension |
|-----------|-----------|
| C++ Klassen-Deklaration | `.hpp` |
| C++ Implementierung | `.cpp` |
| Template-Implementierung (separiert) | `.tpp` |
| C-kompatible Header | `.h` |
| Embedded/C-only Code | `.c` |

---

## 4. Datei-Header

### 4.1 Pflicht-Header

Jede Code-Datei beginnt mit einem Header-Kommentar:

```cpp
/**
 * @file    [Dateiname]
 * @brief   [Kurzbeschreibung in einer Zeile]
 * 
 * @details [Optionale ausführlichere Beschreibung]
 * 
 * @version X.Y.Z
 * @date    YYYY-MM-DD
 * @author  [Name oder Team]
 * 
 * @copyright Copyright (c) YYYY [Firma/Projekt]
 * @license   [Lizenz, z.B. MIT, Proprietary]
 */
```

### 4.2 Header-Felder

| Feld | Pflicht | Beschreibung |
|------|---------|--------------|
| `@file` | ✓ | Dateiname (ohne Pfad) |
| `@brief` | ✓ | Einzeilige Kurzbeschreibung |
| `@details` | — | Ausführlichere Beschreibung |
| `@version` | ✓ | SemVer der Datei/Komponente |
| `@date` | ✓ | Letzte Änderung (ISO 8601) |
| `@author` | ✓ | Autor oder Team |
| `@copyright` | ✓ | Copyright-Vermerk |
| `@license` | — | Lizenz (wenn nicht projekt-global) |

### 4.3 Beispiel: Minimaler Header

```cpp
/**
 * @file    AudioPlayer.hpp
 * @brief   High-level audio playback interface
 * 
 * @version 1.2.0
 * @date    2025-12-13
 * @author  Audio Team
 * 
 * @copyright Copyright (c) 2025 MyProject
 */
```

### 4.4 Beispiel: Vollständiger Header

```cpp
/**
 * @file    AudioPlayer.hpp
 * @brief   High-level audio playback interface
 * 
 * @details Provides a simple interface for audio playback using
 *          the BASS audio library. Supports streaming, 3D audio,
 *          and multiple output devices.
 * 
 *          Thread-safety: All public methods are thread-safe.
 * 
 * @version 1.2.0
 * @date    2025-12-13
 * @author  Audio Team
 * 
 * @copyright Copyright (c) 2025 MyProject
 * @license   MIT
 * 
 * @see     AudioEngine.hpp
 * @see     SoundBuffer.hpp
 */
```

---

## 5. Header-Datei-Struktur (.hpp/.h)

### 5.1 Aufbau

```cpp
// ============================================================
// 1. Datei-Header (Pflicht)
// ============================================================
/**
 * @file    Example.hpp
 * @brief   ...
 */

// ============================================================
// 2. Include Guard (Pflicht)
// ============================================================
#pragma once
// ODER traditionell:
// #ifndef PROJECT_MODULE_EXAMPLE_HPP
// #define PROJECT_MODULE_EXAMPLE_HPP

// ============================================================
// 3. Includes (sortiert nach Abschnitt 8)
// ============================================================
#include <string>
#include <vector>

#include "CoreTypes.hpp"

// ============================================================
// 4. Forward Declarations (wenn möglich)
// ============================================================
namespace audio {
class SoundBuffer;
}

// ============================================================
// 5. Namespace öffnen
// ============================================================
namespace project::module {

// ============================================================
// 6. Konstanten und Typdefinitionen
// ============================================================
constexpr int kDefaultBufferSize = 4096;

using SampleRate = uint32_t;

// ============================================================
// 7. Klassen / Structs / Enums
// ============================================================
class Example
{
    // ... (siehe Abschnitt 9)
};

// ============================================================
// 8. Inline / Template Implementierungen (kurz)
// ============================================================
inline int Example::get_id() const { return m_id; }

// ============================================================
// 9. Namespace schließen
// ============================================================
} // namespace project::module

// ============================================================
// 10. Include für Template-Implementierung (optional)
// ============================================================
#include "Example.tpp"

// ============================================================
// 11. Ende Include Guard (wenn #ifndef verwendet)
// ============================================================
// #endif // PROJECT_MODULE_EXAMPLE_HPP
```

### 5.2 Include Guard

**Bevorzugt:** `#pragma once`

```cpp
#pragma once
```

**Alternativ:** Traditioneller Include Guard

```cpp
#ifndef PROJECT_MODULE_CLASSNAME_HPP
#define PROJECT_MODULE_CLASSNAME_HPP

// ... Inhalt ...

#endif // PROJECT_MODULE_CLASSNAME_HPP
```

**Namenskonvention:** `PROJECT_MODULE_CLASSNAME_HPP` (UPPER_SNAKE_CASE)

---

## 6. Source-Datei-Struktur (.cpp/.c)

### 6.1 Aufbau

```cpp
// ============================================================
// 1. Datei-Header (Pflicht)
// ============================================================
/**
 * @file    Example.cpp
 * @brief   Implementation of Example class
 */

// ============================================================
// 2. Precompiled Header (wenn verwendet)
// ============================================================
#include "pch/pch.hpp"

// ============================================================
// 3. Zugehöriger Header (immer zuerst!)
// ============================================================
#include "Example.hpp"

// ============================================================
// 4. Weitere Includes (sortiert nach Abschnitt 8)
// ============================================================
#include <algorithm>
#include <stdexcept>

#include "Logger.hpp"
#include "Utility.hpp"

// ============================================================
// 5. Anonymer Namespace für lokale Helfer
// ============================================================
namespace {

constexpr int kInternalConstant = 42;

bool validate_input(int value)
{
    return value > 0;
}

} // anonymous namespace

// ============================================================
// 6. Namespace öffnen
// ============================================================
namespace project::module {

// ============================================================
// 7. Statische Member initialisieren
// ============================================================
int Example::s_instance_count = 0;

// ============================================================
// 8. Konstruktoren / Destruktoren
// ============================================================
Example::Example()
    : m_id(0)
    , m_name("default")
{
    ++s_instance_count;
}

Example::~Example()
{
    --s_instance_count;
}

// ============================================================
// 9. Public Methods
// ============================================================
void Example::initialize()
{
    // ...
}

// ============================================================
// 10. Protected Methods
// ============================================================

// ============================================================
// 11. Private Methods
// ============================================================
void Example::internal_helper()
{
    // ...
}

// ============================================================
// 12. Namespace schließen
// ============================================================
} // namespace project::module
```

### 6.2 Wichtige Regel: Eigener Header zuerst

Der zugehörige Header **MUSS** der erste Include nach PCH sein:

```cpp
#include "pch/pch.hpp"    // PCH (wenn verwendet)
#include "MyClass.hpp"    // ← Eigener Header ZUERST!
#include <vector>         // System-Header
#include "Other.hpp"      // Projekt-Header
```

**Begründung:** Stellt sicher, dass der Header selbstständig kompilierbar ist.

---

## 7. Template-Datei-Struktur (.tpp)

### 7.1 Wann .tpp verwenden?

- Template-Implementierungen, die zu lang für den Header sind
- Trennung von Deklaration und Implementierung bei Templates

### 7.2 Aufbau

```cpp
// ============================================================
// 1. Datei-Header
// ============================================================
/**
 * @file    Container.tpp
 * @brief   Template implementations for Container<T>
 * 
 * @note    This file is included at the end of Container.hpp
 *          Do not include directly!
 */

// ============================================================
// 2. KEIN Include Guard (wird vom Header inkludiert)
// ============================================================

// ============================================================
// 3. Template-Implementierungen
// ============================================================
namespace project {

template<typename T>
Container<T>::Container()
    : m_data()
{
}

template<typename T>
void Container<T>::add(const T& item)
{
    m_data.push_back(item);
}

template<typename T>
T Container<T>::get(size_t index) const
{
    if (index >= m_data.size())
    {
        throw std::out_of_range("Index out of bounds");
    }
    return m_data[index];
}

} // namespace project
```

### 7.3 Einbindung im Header

Am Ende des zugehörigen Headers:

```cpp
// Container.hpp
#pragma once

namespace project {

template<typename T>
class Container
{
public:
    Container();
    void add(const T& item);
    T get(size_t index) const;
    
private:
    std::vector<T> m_data;
};

} // namespace project

// Template-Implementierung einbinden
#include "Container.tpp"
```

---

## 8. Include-Reihenfolge

### 8.1 Reihenfolge

```cpp
// 1. Precompiled Header (wenn verwendet)
#include "pch/pch.hpp"

// 2. Zugehöriger Header (nur in .cpp)
#include "ThisFile.hpp"

// 3. C Standard Library
#include <cstdint>
#include <cstring>

// 4. C++ Standard Library
#include <algorithm>
#include <string>
#include <vector>

// 5. Third-Party Libraries
#include <bass.h>
#include <imgui.h>

// 6. Projekt-Header (andere Module)
#include "core/Logger.hpp"
#include "utils/StringUtils.hpp"

// 7. Projekt-Header (gleiches Modul)
#include "LocalHelper.hpp"
```

### 8.2 Regeln

| Regel | Beschreibung |
|-------|--------------|
| Gruppen trennen | Leerzeile zwischen Gruppen |
| Alphabetisch | Innerhalb jeder Gruppe |
| Anführungszeichen | `"..."` für Projekt-Header |
| Spitze Klammern | `<...>` für System/Third-Party |

### 8.3 Beispiel

```cpp
#include "pch/pch.hpp"

#include "AudioPlayer.hpp"

#include <cstdint>

#include <algorithm>
#include <memory>
#include <string>

#include <bass.h>

#include "core/Logger.hpp"
#include "core/Settings.hpp"

#include "SoundBuffer.hpp"
```

---

## 9. Klassen-Layout

### 9.1 Reihenfolge der Sektionen

```cpp
class Example
{
    // ========================================
    // 1. Friend-Deklarationen
    // ========================================
    friend class ExampleFactory;

public:
    // ========================================
    // 2. Typdefinitionen und Nested Types
    // ========================================
    using Callback = std::function<void(int)>;
    
    enum class State { Idle, Running, Stopped };
    
    struct Config
    {
        int buffer_size = 4096;
        bool verbose = false;
    };

    // ========================================
    // 3. Statische Konstanten
    // ========================================
    static constexpr int kMaxInstances = 10;

    // ========================================
    // 4. Konstruktoren / Destruktor
    // ========================================
    Example();
    explicit Example(const Config& config);
    ~Example();
    
    // ========================================
    // 5. Copy/Move (Rule of Five)
    // ========================================
    Example(const Example& other);
    Example& operator=(const Example& other);
    Example(Example&& other) noexcept;
    Example& operator=(Example&& other) noexcept;

    // ========================================
    // 6. Statische Methoden
    // ========================================
    static int get_instance_count();

    // ========================================
    // 7. Öffentliche Methoden
    // ========================================
    void initialize();
    void process();
    
    // ========================================
    // 8. Getter / Setter
    // ========================================
    int get_id() const;
    void set_id(int id);
    
    const std::string& get_name() const;
    void set_name(const std::string& name);

    // ========================================
    // 9. Operatoren
    // ========================================
    bool operator==(const Example& other) const;

protected:
    // ========================================
    // 10. Protected Methoden
    // ========================================
    virtual void on_state_change(State new_state);

    // ========================================
    // 11. Protected Member
    // ========================================
    State m_state = State::Idle;

private:
    // ========================================
    // 12. Private Methoden
    // ========================================
    void internal_update();
    bool validate_config(const Config& config);

    // ========================================
    // 13. Private Member
    // ========================================
    int m_id;
    std::string m_name;
    Config m_config;
    
    // ========================================
    // 14. Statische Member
    // ========================================
    static int s_instance_count;
};
```

### 9.2 Visibility-Reihenfolge

1. `public:` — Öffentliche Schnittstelle zuerst
2. `protected:` — Für Ableitungen
3. `private:` — Implementierungsdetails

**Begründung:** Leser interessieren sich meist zuerst für die öffentliche API.

---

## 10. Dokumentationskommentare

### 10.1 Doxygen-Format

```cpp
/**
 * @brief   Kurze Beschreibung der Funktion
 * 
 * @details Ausführlichere Beschreibung, wenn nötig.
 *          Kann mehrere Zeilen umfassen.
 * 
 * @param   name        Beschreibung des Parameters
 * @param   value       Noch ein Parameter
 * 
 * @return  Was zurückgegeben wird
 * 
 * @throws  std::invalid_argument  Wenn name leer ist
 * @throws  std::runtime_error     Bei internem Fehler
 * 
 * @note    Wichtiger Hinweis
 * @warning Warnung vor Fallstricken
 * 
 * @see     related_function()
 * @since   1.2.0
 * 
 * @example
 * @code
 * Example ex;
 * ex.do_something("test", 42);
 * @endcode
 */
void do_something(const std::string& name, int value);
```

### 10.2 Wann dokumentieren?

| Element | Dokumentation |
|---------|---------------|
| Öffentliche API | Pflicht — vollständig |
| Protected Methoden | Empfohlen |
| Private Methoden | Optional — wenn komplex |
| Triviale Getter/Setter | Nicht nötig |

### 10.3 Inline-Kommentare

```cpp
// Einzeiliger Kommentar für kurze Erklärungen

/* 
 * Mehrzeiliger Kommentar für
 * längere Erklärungen im Code
 */

int value = 42;  // Trailing comment (sparsam verwenden)
```

---

## 11. Beispiele

### 11.1 Vollständiger Header (AudioPlayer.hpp)

```cpp
/**
 * @file    AudioPlayer.hpp
 * @brief   High-level audio playback interface
 * 
 * @version 1.2.0
 * @date    2025-12-13
 * @author  Audio Team
 * 
 * @copyright Copyright (c) 2025 MyProject
 */

#pragma once

#include <cstdint>

#include <functional>
#include <memory>
#include <string>

namespace audio {

class SoundBuffer;

/**
 * @brief   Audio player for sound playback
 * 
 * @details Provides high-level audio playback using BASS library.
 *          Supports streaming, 3D positioning, and effects.
 */
class AudioPlayer
{
public:
    using VolumeCallback = std::function<void(float)>;
    
    enum class State { Stopped, Playing, Paused };

    AudioPlayer();
    ~AudioPlayer();
    
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;
    
    /**
     * @brief   Load audio file for playback
     * @param   filepath    Path to audio file
     * @return  true on success
     */
    bool load(const std::string& filepath);
    
    void play();
    void pause();
    void stop();
    
    State get_state() const;
    
    float get_volume() const;
    void set_volume(float volume);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace audio
```

### 11.2 Vollständige Source (AudioPlayer.cpp)

```cpp
/**
 * @file    AudioPlayer.cpp
 * @brief   Implementation of AudioPlayer class
 * 
 * @version 1.2.0
 * @date    2025-12-13
 * @author  Audio Team
 * 
 * @copyright Copyright (c) 2025 MyProject
 */

#include "pch/pch.hpp"

#include "AudioPlayer.hpp"

#include <stdexcept>

#include <bass.h>

#include "core/Logger.hpp"

namespace {

bool is_valid_volume(float vol)
{
    return vol >= 0.0f && vol <= 1.0f;
}

} // anonymous namespace

namespace audio {

struct AudioPlayer::Impl
{
    HSTREAM stream = 0;
    State state = State::Stopped;
    float volume = 1.0f;
};

AudioPlayer::AudioPlayer()
    : m_impl(std::make_unique<Impl>())
{
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

bool AudioPlayer::load(const std::string& filepath)
{
    // Implementation...
    return true;
}

void AudioPlayer::play()
{
    if (m_impl->stream != 0)
    {
        BASS_ChannelPlay(m_impl->stream, FALSE);
        m_impl->state = State::Playing;
    }
}

void AudioPlayer::pause()
{
    if (m_impl->state == State::Playing)
    {
        BASS_ChannelPause(m_impl->stream);
        m_impl->state = State::Paused;
    }
}

void AudioPlayer::stop()
{
    if (m_impl->stream != 0)
    {
        BASS_ChannelStop(m_impl->stream);
        m_impl->state = State::Stopped;
    }
}

AudioPlayer::State AudioPlayer::get_state() const
{
    return m_impl->state;
}

float AudioPlayer::get_volume() const
{
    return m_impl->volume;
}

void AudioPlayer::set_volume(float volume)
{
    if (!is_valid_volume(volume))
    {
        throw std::invalid_argument("Volume must be between 0.0 and 1.0");
    }
    m_impl->volume = volume;
}

} // namespace audio
```

---

## 12. Review-Checkliste

Vor Commit einer Code-Datei prüfen:

**Datei-Header:**
- [ ] Header-Kommentar vorhanden
- [ ] `@file`, `@brief`, `@version`, `@date`, `@author`, `@copyright` ausgefüllt
- [ ] Version aktuell

**Struktur:**
- [ ] Include Guard vorhanden (`#pragma once` oder `#ifndef`)
- [ ] Includes sortiert nach Reihenfolge (Abschnitt 8)
- [ ] Eigener Header zuerst (in .cpp)
- [ ] Namespaces korrekt geöffnet/geschlossen

**Klassen:**
- [ ] Visibility-Reihenfolge: public → protected → private
- [ ] Member-Reihenfolge gemäß Abschnitt 9
- [ ] Öffentliche API dokumentiert

**Code:**
- [ ] Keine Includes in .tpp Dateien
- [ ] Anonymer Namespace für lokale Helfer (in .cpp)

---

## 13. Siehe auch

- [Cpp_Coding_Standard.md](../standards/Cpp_Coding_Standard.md) — Namenskonventionen, Stil
- [ClangFormat_Blueprint.md](ClangFormat.md) — Automatische Formatierung
- [CMake.md](CMake.md) — CMake-Dateistruktur

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Initial: Datei-Header, Dateistruktur (hpp/cpp/tpp), Include-Reihenfolge, Klassen-Layout, Dokumentationskommentare** |
