# Local Externals — Media

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Local_Externals_Media.md](../../en/references/externals/Local_Externals_Media.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [BASS Audio Library](#2-bass-audio-library)
3. [See Also](#3-siehe-auch)
4. [Changelog](#4-changelog)

---

## 1. Overview

This document describes lokale Media-Externals für das CMake Architecture Build-System: Audio, Bild, Video.

| Library | Description | Lizenz |
|---------|--------------|--------|
| **BASS** | Professionelle Audio-Library mit Plugins | Kommerziell / Free for non-commercial |

---

## 2. BASS Audio Library

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Typ** | 📦 Vorkompiliert |
| **Pfad** | `externals/bass` |
| **Include.cmake** | `cmake/externals/includes/bass/Include.cmake` |
| **Plattformen** | Windows, Linux, macOS |
| **Website** | [un4seen.com](https://www.un4seen.com/bass.html) |

### Solution.json

```json
{
    "externals": {
        "bass": {
            "path": "externals/bass"
        }
    }
}
```

### Verzeichnisstruktur

```
externals/bass/
├── include/
│   ├── bass.h
│   ├── bassflac.h
│   ├── bassopus.h
│   ├── bass_fx.h
│   └── ...
└── lib/
    ├── x64/
    │   ├── bass.dll
    │   ├── bass.lib
    │   ├── bassflac.dll
    │   └── ...
    └── x86/
        └── ...
```

### Plugin-System

BASS unterstützt Plugins via `external_options`:

```json
{
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"],
            "external_options": {
                "bass": {
                    "BASS_FLAC": true,
                    "BASS_OPUS": true,
                    "BASS_FX": true
                }
            }
        }
    ]
}
```

### Verfügbare Options

#### Decoder

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_FLAC` | bassflac | FLAC Audio Decoder |
| `BASS_OPUS` | bassopus | Opus Audio Decoder |
| `BASS_DSD` | bassdsd | DSD (Direct Stream Digital) |
| `BASS_WV` | basswv | WavPack Decoder |
| `BASS_APE` | bassape | Monkey's Audio |
| `BASS_MPC` | bass_mpc | Musepack Decoder |
| `BASS_ALAC` | bassalac | Apple Lossless |
| `BASS_TTA` | bass_tta | True Audio |
| `BASS_CD` | basscd | CD Audio Ripper |
| `BASS_WEBM` | basswebm | WebM Container |

#### Encoder

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_ENC` | bassenc | Encoding-Basis (Auto bei Encoder) |
| `BASS_ENC_MP3` | bassenc_mp3 | MP3 Encoder |
| `BASS_ENC_OGG` | bassenc_ogg | OGG Vorbis Encoder |
| `BASS_ENC_FLAC` | bassenc_flac | FLAC Encoder |

#### Effekte & Mixing

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_FX` | bass_fx | DSP Effekte (Tempo, Pitch, Reverb) |
| `BASS_MIX` | bassmix | Multi-Channel Mixing |
| `BASS_LOUD` | bassloud | Loudness Messung (EBU R128) |
| `BASS_MIDI` | bassmidi | MIDI Playback |

### Usagesbeispiel

```cpp
#include <bass.h>

int main() {
    // Initialisieren
    BASS_Init(-1, 44100, 0, nullptr, nullptr);
    
    // Stream laden
    HSTREAM stream = BASS_StreamCreateFile(
        FALSE, "music.mp3", 0, 0, 0
    );
    
    // Abspielen
    BASS_ChannelPlay(stream, FALSE);
    
    // Warten
    while (BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING) {
        Sleep(100);
    }
    
    // Aufräumen
    BASS_StreamFree(stream);
    BASS_Free();
    
    return 0;
}
```

### Mit Plugins (FLAC + Effekte)

```cpp
#include <bass.h>
#include <bassflac.h>
#include <bass_fx.h>

int main() {
    BASS_Init(-1, 44100, 0, nullptr, nullptr);
    
    // FLAC laden (Plugin wird automatisch erkannt)
    HSTREAM original = BASS_StreamCreateFile(
        FALSE, "music.flac", 0, 0, BASS_STREAM_DECODE
    );
    
    // Tempo-Stream für Effekte
    HSTREAM tempo = BASS_FX_TempoCreate(original, BASS_FX_FREESOURCE);
    
    // Tempo anpassen: -20% langsamer
    BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO, -20.0f);
    
    // Pitch: 3 Halbtöne höher
    BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO_PITCH, 3.0f);
    
    BASS_ChannelPlay(tempo, FALSE);
    
    // ...
    
    return 0;
}
```

### Vergleich mit Git-Alternativen

| Feature | BASS (Local) | miniaudio (Git) | openal-soft (Git) |
|---------|--------------|-----------------|-------------------|
| **Typ** | Vorkompiliert | Header-Only | CMake |
| **Lizenz** | Kommerziell | Public Domain | LGPL |
| **Plugins** | ✅ 20+ | ❌ | ❌ |
| **Effekte** | ✅ | ❌ | ❌ |
| **3D Audio** | ✅ | ✅ | ✅ |
| **Streaming** | ✅ | ✅ | ❌ |
| **Recording** | ✅ | ✅ | ✅ |

### Empfehlung

| Anwendungsfall | Empfehlung |
|----------------|------------|
| Musik-Player mit Formaten | **BASS** |
| Einfache Sounds | miniaudio |
| 3D-Spiele Audio | openal-soft |
| Professionelle Effekte | **BASS** |

### Detail-Dokumentation

→ [bass_Include.md](../../modules/externals/includes/bass/Bass_Include.md)

---

## 3. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Local_Externals.md](Local_Externals.md) — Local Externals Overview
- [Git_Externals_Media.md](Git_Externals_Media.md) — Git Media-Externals (miniaudio, openal-soft, stb)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Eigene Kategorie Media (parallel zu Git)** |
