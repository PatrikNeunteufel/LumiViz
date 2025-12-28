# bass/Include.cmake — BASS Audio Library Integration

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers, C++ Developers  
> **Language:** English  
> **German:** [bass_Include.md](../../en/modules/externals/includes/bass_Include.md)  
> **Module:** [cmake/externals/includes/bass/Include.cmake](../../../../../../cmake/externals/includes/bass/Include.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Verfügbare Variablen](#2-verfügbare-variablen)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Options](#4-options)
5. [Plattform-Unterstützung](#5-plattform-unterstützung)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Verzeichnisstruktur](#7-verzeichnisstruktur)
8. [Errorbehandlung](#8-fehlerbehandlung)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

Die `bass/Include.cmake` integriert die **BASS Audio Library** von un4seen als lokales External in das Build-System.

### Kernfunktionen

| Funktion | Description |
|----------|--------------|
| Audio-Playback | MP3, WAV, OGG, FLAC (mit Plugin) |
| Streaming | Internet-Streams, Shoutcast |
| Recording | Audio-Aufnahme |
| Plugins | Decoder, Encoder, Effekte |

### Target-Erstellung

Erstellt ein `SHARED IMPORTED` Target `bass` mit plattformspezifischen Library-Pfaden.

---

## 2. Verfügbare Variablen

Diese Variablen werden vom Orchestrator bereitgestellt:

| Variable | Description |
|----------|--------------|
| `EXTERNAL_NAME` | `"bass"` |
| `EXTERNAL_PATH` | Absoluter Pfad zu `externals/bass` |
| `EXTERNAL_JSON` | JSON-Element aus Solution.json |
| `EXTERNAL_OPTIONS` | Target-spezifische Options (JSON) |

---

## 3. Erstellte Targets

### 3.1 Primäres Target

| Target | Typ | Description |
|--------|-----|--------------|
| `bass` | SHARED IMPORTED | BASS Core Library |

### 3.2 Plugin-Targets (Optional)

Werden nur erstellt wenn die entsprechende Option aktiviert ist:

| Target | Option | Description |
|--------|--------|--------------|
| `bass::flac` | `BASS_FLAC` | FLAC Decoder |
| `bass::opus` | `BASS_OPUS` | Opus Decoder |
| `bass::fx` | `BASS_FX` | DSP Effekte |
| `bass::mix` | `BASS_MIX` | Multi-Channel Mixing |
| `bass::enc` | `BASS_ENC` | Encoding-Basis |
| `bass::enc_mp3` | `BASS_ENC_MP3` | MP3 Encoder |
| `bass::enc_ogg` | `BASS_ENC_OGG` | OGG Encoder |
| `bass::enc_flac` | `BASS_ENC_FLAC` | FLAC Encoder |
| `bass::midi` | `BASS_MIDI` | MIDI Playback |
| `bass::loud` | `BASS_LOUD` | Loudness (EBU R128) |

---

## 4. Options

### 4.1 Decoder

| Option | Plugin | Header | Description |
|--------|--------|--------|--------------|
| `BASS_FLAC` | bassflac | `bassflac.h` | FLAC Audio Decoder |
| `BASS_OPUS` | bassopus | `bassopus.h` | Opus Audio Decoder |
| `BASS_DSD` | bassdsd | `bassdsd.h` | DSD (Direct Stream Digital) |
| `BASS_WV` | basswv | `basswv.h` | WavPack Decoder |
| `BASS_APE` | bassape | `bass_ape.h` | Monkey's Audio |
| `BASS_MPC` | bass_mpc | `bass_mpc.h` | Musepack Decoder |
| `BASS_ALAC` | bassalac | `bassalac.h` | Apple Lossless |
| `BASS_TTA` | bass_tta | `bass_tta.h` | True Audio |
| `BASS_CD` | basscd | `basscd.h` | CD Audio Ripper |
| `BASS_WEBM` | basswebm | `basswebm.h` | WebM Container |

### 4.2 Encoder

| Option | Plugin | Header | Description |
|--------|--------|--------|--------------|
| `BASS_ENC` | bassenc | `bassenc.h` | Encoding-Basis (Auto bei Encoder) |
| `BASS_ENC_MP3` | bassenc_mp3 | `bassenc_mp3.h` | MP3 Encoder |
| `BASS_ENC_OGG` | bassenc_ogg | `bassenc_ogg.h` | OGG Vorbis Encoder |
| `BASS_ENC_FLAC` | bassenc_flac | `bassenc_flac.h` | FLAC Encoder |

### 4.3 Plugins

| Option | Plugin | Header | Description |
|--------|--------|--------|--------------|
| `BASS_FX` | bass_fx | `bass_fx.h` | DSP Effekte (Tempo, Pitch, Reverb) |
| `BASS_MIX` | bassmix | `bassmix.h` | Multi-Channel Mixing |
| `BASS_LOUD` | bassloud | `bassloud.h` | Loudness Messung (EBU R128) |
| `BASS_MIDI` | bassmidi | `bassmidi.h` | MIDI Playback |

---

## 5. Plattform-Unterstützung

### 5.1 Library-Pfade

| Plattform | DLL/SO | Import-Lib |
|-----------|--------|------------|
| Windows x64 | `lib/x64/bass.dll` | `lib/x64/bass.lib` |
| Windows x86 | `lib/x86/bass.dll` | `lib/x86/bass.lib` |
| Linux | `lib/libbass.so` | — |
| macOS | `lib/libbass.dylib` | — |

### 5.2 Architecture-Erkennung

```cmake
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_arch "x64")
else()
    set(_arch "x86")
endif()
```

---

## 6. Usagesbeispiele

### 6.1 Basis-Usage

**Solution.json:**

```json
{
    "externals": {
        "bass": { "path": "externals/bass" }
    },
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"]
        }
    ]
}
```

**C++:**

```cpp
#include <bass.h>

int main() {
    BASS_Init(-1, 44100, 0, nullptr, nullptr);
    
    HSTREAM stream = BASS_StreamCreateFile(FALSE, "music.mp3", 0, 0, 0);
    BASS_ChannelPlay(stream, FALSE);
    
    // Warten bis fertig
    while (BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING) {
        Sleep(100);
    }
    
    BASS_StreamFree(stream);
    BASS_Free();
    return 0;
}
```

### 6.2 Mit Plugins

**Solution.json:**

```json
{
    "executables": [
        {
            "name": "FlacPlayer",
            "externals": ["bass"],
            "external_options": {
                "bass": {
                    "BASS_FLAC": true,
                    "BASS_FX": true
                }
            }
        }
    ]
}
```

**C++:**

```cpp
#include <bass.h>
#include <bassflac.h>
#include <bass_fx.h>

// FLAC wird automatisch erkannt
HSTREAM stream = BASS_StreamCreateFile(FALSE, "music.flac", 0, 0, BASS_STREAM_DECODE);

// Tempo-Stream für Pitch/Speed-Änderung
HSTREAM tempo = BASS_FX_TempoCreate(stream, BASS_FX_FREESOURCE);
BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO, -20.0f);  // 20% langsamer
```

---

## 7. Verzeichnisstruktur

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

---

## 8. Errorbehandlung

| Code | Description |
|------|--------------|
| E213 | Include.cmake nicht gefunden |
| E214 | External-Pfad existiert nicht |
| W105 | Plugin-DLL nicht gefunden (Option aktiviert aber Datei fehlt) |

---

## 9. See Also

- [Externals_Reference.md](../../../../references/Externals.md) — Alle Options im Detail
- [Externals_UserGuide.md](../../../../userguides/Externals.md) — Usagesanleitung
- [Attach_cmake.md](../../locals/Attach_cmake.md) — Local External Handler

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.1** | **2025-12-16** | **SYSTEM-Includes für Compiler-Warningsunterdrückung** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0, Convention-Pfad `cmake/externals/includes/bass/` |
| 0.1.1 | 2025-12-09 | Plugin-System mit Options, Encoder-Targets |
| 0.1.0 | 2025-12-05 | Initial: BASS Core Target |
