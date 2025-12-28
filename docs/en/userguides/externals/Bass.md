# BASS — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [bass.md](../../../en/userguides/externals/Bass.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Installation](#2-installation)
3. [Solution.json Configuration](#3-solutionjson-konfiguration)
4. [C++ Usage](#4-c-verwendung)
5. [Plugins](#5-plugins)
6. [Effekte](#6-effekte)
7. [Fortgeschrittene Techniken](#7-fortgeschrittene-techniken)
8. [Troubleshooting](#8-troubleshooting)
9. [Weiterführende Informationen](#9-weiterführende-informationen)
10. [Changelog](#10-changelog)

---

## 1. Overview

**BASS** ist eine professionelle Audio-Library mit umfangreichem Plugin-System.

| Aspekt | Wert |
|--------|------|
| **Typ** | Local External (Vorkompiliert) |
| **Pfad** | `externals/bass` |
| **Lizenz** | Free for non-commercial / Commercial |
| **Website** | [un4seen.com](https://www.un4seen.com/bass.html) |

### Warum BASS?

| Vorteil | Description |
|---------|--------------|
| 🎵 **Plugins** | 20+ Format-Plugins |
| 🎚️ **Effekte** | DSP, Tempo, Pitch, Reverb |
| 🔊 **3D Audio** | Surround Sound Support |
| 📱 **Cross-Platform** | Windows, Linux, macOS |

---

## 2. Installation

### 2.1 Download

Von [un4seen.com/bass.html](https://www.un4seen.com/bass.html):

1. BASS Library herunterladen
2. Benötigte Add-ons herunterladen (FLAC, FX, etc.)
3. Nach `externals/bass/` entpacken

### 2.2 Verzeichnisstruktur

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
    │   ├── bass_fx.dll
    │   └── ...
    └── x86/
        └── ...
```

### 2.3 DLLs kopieren

Die DLLs müssen zur Laufzeit verfügbar sein. Das Build-System kopiert sie automatisch in das Output-Verzeichnis.

---

## 3. Solution.json Configuration

### 3.1 Minimal

```json
{
    "externals": {
        "bass": {
            "path": "externals/bass"
        }
    },
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"]
        }
    ]
}
```

### 3.2 Mit Plugins

```json
{
    "externals": {
        "bass": {
            "path": "externals/bass"
        }
    },
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

### 3.3 Alle Options

#### Decoder

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_FLAC` | bassflac | FLAC Audio |
| `BASS_OPUS` | bassopus | Opus Audio |
| `BASS_DSD` | bassdsd | DSD (Direct Stream Digital) |
| `BASS_WV` | basswv | WavPack |
| `BASS_APE` | bassape | Monkey's Audio |
| `BASS_MPC` | bass_mpc | Musepack |
| `BASS_ALAC` | bassalac | Apple Lossless |
| `BASS_TTA` | bass_tta | True Audio |
| `BASS_CD` | basscd | CD Audio |
| `BASS_WEBM` | basswebm | WebM Container |

#### Encoder

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_ENC` | bassenc | Encoding-Basis |
| `BASS_ENC_MP3` | bassenc_mp3 | MP3 Encoder |
| `BASS_ENC_OGG` | bassenc_ogg | OGG Vorbis Encoder |
| `BASS_ENC_FLAC` | bassenc_flac | FLAC Encoder |

#### Effekte & Mixing

| Option | Plugin | Description |
|--------|--------|--------------|
| `BASS_FX` | bass_fx | DSP Effekte |
| `BASS_MIX` | bassmix | Multi-Channel Mixing |
| `BASS_LOUD` | bassloud | Loudness (EBU R128) |
| `BASS_MIDI` | bassmidi | MIDI Playback |

---

## 4. C++ Usage

### 4.1 Grundlagen

```cpp
#include <bass.h>
#include <iostream>

int main() {
    // BASS initialisieren
    // -1 = Default Device, 44100 Hz, 0 = Default Flags
    if (!BASS_Init(-1, 44100, 0, nullptr, nullptr)) {
        std::cerr << "BASS_Init failed: " << BASS_ErrorGetCode() << std::endl;
        return 1;
    }
    
    std::cout << "BASS initialized successfully!" << std::endl;
    std::cout << "Version: " << HIWORD(BASS_GetVersion()) << "." 
              << LOWORD(BASS_GetVersion()) << std::endl;
    
    // ... Audio abspielen ...
    
    // BASS beenden
    BASS_Free();
    return 0;
}
```

### 4.2 MP3/WAV abspielen

```cpp
#include <bass.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    BASS_Init(-1, 44100, 0, nullptr, nullptr);
    
    // Stream erstellen
    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,          // Nicht aus Speicher
        "music.mp3",    // Dateipfad
        0,              // Offset
        0,              // Länge (0 = ganze Datei)
        0               // Flags
    );
    
    if (!stream) {
        std::cerr << "Failed to create stream: " << BASS_ErrorGetCode() << std::endl;
        BASS_Free();
        return 1;
    }
    
    // Abspielen starten
    BASS_ChannelPlay(stream, FALSE);
    
    // Warten bis fertig
    while (BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING) {
        // Position anzeigen
        double pos = BASS_ChannelBytes2Seconds(stream, BASS_ChannelGetPosition(stream, BASS_POS_BYTE));
        double len = BASS_ChannelBytes2Seconds(stream, BASS_ChannelGetLength(stream, BASS_POS_BYTE));
        std::cout << "\rPosition: " << (int)pos << "s / " << (int)len << "s" << std::flush;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << std::endl << "Playback finished!" << std::endl;
    
    // Aufräumen
    BASS_StreamFree(stream);
    BASS_Free();
    return 0;
}
```

### 4.3 Lautstärke und Position

```cpp
HSTREAM stream = BASS_StreamCreateFile(FALSE, "music.mp3", 0, 0, 0);
BASS_ChannelPlay(stream, FALSE);

// Lautstärke (0.0 - 1.0)
BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, 0.5f);

// Lautstärke auslesen
float volume;
BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &volume);

// Position setzen (in Sekunden)
double newPos = 30.0;  // 30 Sekunden
QWORD bytes = BASS_ChannelSeconds2Bytes(stream, newPos);
BASS_ChannelSetPosition(stream, bytes, BASS_POS_BYTE);

// Pause
BASS_ChannelPause(stream);

// Fortsetzen
BASS_ChannelPlay(stream, FALSE);

// Stop (zurück zum Anfang)
BASS_ChannelStop(stream);
```

### 4.4 Stream aus URL

```cpp
// Internet-Stream
HSTREAM stream = BASS_StreamCreateURL(
    "http://example.com/radio.mp3",
    0,                  // Offset
    BASS_STREAM_BLOCK,  // Download in Blöcken
    nullptr,            // Callback
    nullptr             // User data
);

if (stream) {
    BASS_ChannelPlay(stream, FALSE);
}
```

### 4.5 Mehrere Streams gleichzeitig

```cpp
HSTREAM music = BASS_StreamCreateFile(FALSE, "music.mp3", 0, 0, 0);
HSTREAM sfx = BASS_StreamCreateFile(FALSE, "explosion.wav", 0, 0, 0);

// Beide abspielen
BASS_ChannelPlay(music, FALSE);
BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, 0.7f);

BASS_ChannelPlay(sfx, FALSE);
BASS_ChannelSetAttribute(sfx, BASS_ATTRIB_VOL, 1.0f);
```

---

## 5. Plugins

### 5.1 FLAC abspielen

```cpp
#include <bass.h>
#include <bassflac.h>

int main() {
    BASS_Init(-1, 44100, 0, nullptr, nullptr);
    
    // FLAC-Plugin wird automatisch verwendet
    HSTREAM stream = BASS_StreamCreateFile(FALSE, "music.flac", 0, 0, 0);
    
    if (stream) {
        BASS_ChannelPlay(stream, FALSE);
        // ...
        BASS_StreamFree(stream);
    }
    
    BASS_Free();
    return 0;
}
```

### 5.2 Opus abspielen

```cpp
#include <bass.h>
#include <bassopus.h>

HSTREAM stream = BASS_StreamCreateFile(FALSE, "music.opus", 0, 0, 0);
```

### 5.3 Format-Informationen

```cpp
BASS_CHANNELINFO info;
BASS_ChannelGetInfo(stream, &info);

std::cout << "Channels: " << info.chans << std::endl;
std::cout << "Sample Rate: " << info.freq << " Hz" << std::endl;
std::cout << "Original Freq: " << info.origres << " Hz" << std::endl;

// Format-Typ
switch (info.ctype) {
    case BASS_CTYPE_STREAM_MP3: std::cout << "MP3" << std::endl; break;
    case BASS_CTYPE_STREAM_OGG: std::cout << "OGG" << std::endl; break;
    case BASS_CTYPE_STREAM_FLAC: std::cout << "FLAC" << std::endl; break;
    case BASS_CTYPE_STREAM_WAV: std::cout << "WAV" << std::endl; break;
    default: std::cout << "Unknown" << std::endl;
}
```

---

## 6. Effekte

### 6.1 BASS_FX Setup

```cpp
#include <bass.h>
#include <bass_fx.h>

BASS_Init(-1, 44100, 0, nullptr, nullptr);

// Stream als DECODE erstellen (für FX-Processing)
HSTREAM original = BASS_StreamCreateFile(
    FALSE, "music.mp3", 0, 0, 
    BASS_STREAM_DECODE  // Important!
);

// Tempo-Stream erstellen
HSTREAM tempo = BASS_FX_TempoCreate(original, BASS_FX_FREESOURCE);
```

### 6.2 Tempo ändern

```cpp
// Tempo: -50% bis +100%
// -20 = 20% langsamer
BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO, -20.0f);

// Tempo auslesen
float currentTempo;
BASS_ChannelGetAttribute(tempo, BASS_ATTRIB_TEMPO, &currentTempo);
```

### 6.3 Pitch ändern

```cpp
// Pitch in Halbtönen: -12 bis +12
// 3 = 3 Halbtöne höher
BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO_PITCH, 3.0f);
```

### 6.4 Playback Rate

```cpp
// Rate: 0.0 bis 4.0 (normal = 1.0)
// Ändert Tempo UND Pitch (wie Vinyl)
BASS_ChannelSetAttribute(tempo, BASS_ATTRIB_TEMPO_FREQ, 
    44100 * 1.5f);  // 50% schneller
```

### 6.5 Reverb-Effekt

```cpp
// Reverb hinzufügen
HFX reverb = BASS_ChannelSetFX(stream, BASS_FX_DX8_REVERB, 1);

// Parameters setzen
BASS_DX8_REVERB params;
params.fInGain = 0.0f;
params.fReverbMix = -10.0f;
params.fReverbTime = 1000.0f;
params.fHighFreqRTRatio = 0.5f;
BASS_FXSetParameterss(reverb, &params);
```

### 6.6 Equalizer

```cpp
// 3-Band EQ
HFX eq = BASS_ChannelSetFX(stream, BASS_FX_DX8_PARAMEQ, 1);

BASS_DX8_PARAMEQ params;
params.fCenter = 100.0f;   // Bass: 100 Hz
params.fBandwidth = 12.0f;
params.fGain = 6.0f;       // +6 dB
BASS_FXSetParameterss(eq, &params);
```

---

## 7. Fortgeschrittene Techniken

### 7.1 Callbacks

```cpp
// Position-Sync Callback
void CALLBACK SyncCallback(HSYNC handle, DWORD channel, DWORD data, void* user) {
    std::cout << "Reached sync point!" << std::endl;
}

// Bei 30 Sekunden
QWORD pos = BASS_ChannelSeconds2Bytes(stream, 30.0);
BASS_ChannelSetSync(stream, BASS_SYNC_POS, pos, SyncCallback, nullptr);

// Am Ende
BASS_ChannelSetSync(stream, BASS_SYNC_END, 0, SyncCallback, nullptr);
```

### 7.2 Visualisierung (FFT)

```cpp
float fft[256];
BASS_ChannelGetData(stream, fft, BASS_DATA_FFT512);

// fft[0..255] enthält Frequenzdaten
for (int i = 0; i < 256; i++) {
    float magnitude = fft[i];
    // Visualisieren...
}
```

### 7.3 Recording

```cpp
// Recording starten
HRECORD record = BASS_RecordStart(44100, 2, 0, nullptr, nullptr);

// ... aufnehmen ...

// Recording stoppen und als WAV speichern
BASS_ChannelStop(record);

// Manuell WAV-Datei erstellen oder bassenc verwenden
```

### 7.4 Loop-Punkte

```cpp
// Loop setzen
QWORD loopStart = BASS_ChannelSeconds2Bytes(stream, 10.0);
QWORD loopEnd = BASS_ChannelSeconds2Bytes(stream, 20.0);

BASS_ChannelSetPosition(stream, loopStart, BASS_POS_BYTE);
BASS_ChannelFlags(stream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

// Benutzerdefinierter Loop
BASS_ChannelSetSync(stream, BASS_SYNC_POS | BASS_SYNC_MIXTIME, 
    loopEnd, [](HSYNC, DWORD channel, DWORD, void* start) {
        BASS_ChannelSetPosition(channel, *(QWORD*)start, BASS_POS_BYTE);
    }, &loopStart);
```

---

## 8. Troubleshooting

### 8.1 BASS_Init failed

**Problem:** `BASS_Init` gibt FALSE zurück

**Lösungen:**
```cpp
int error = BASS_ErrorGetCode();
switch (error) {
    case BASS_ERROR_DEVICE:
        std::cerr << "Invalid device" << std::endl;
        break;
    case BASS_ERROR_ALREADY:
        std::cerr << "Already initialized" << std::endl;
        break;
    case BASS_ERROR_DRIVER:
        std::cerr << "No audio driver" << std::endl;
        break;
}
```

### 8.2 Stream nicht erstellt

**Problem:** `BASS_StreamCreateFile` gibt 0 zurück

**Lösungen:**
```cpp
int error = BASS_ErrorGetCode();
switch (error) {
    case BASS_ERROR_FILEOPEN:
        std::cerr << "File not found" << std::endl;
        break;
    case BASS_ERROR_FILEFORM:
        std::cerr << "Unknown format (plugin missing?)" << std::endl;
        break;
    case BASS_ERROR_CODEC:
        std::cerr << "Codec not available" << std::endl;
        break;
}
```

### 8.3 DLL nicht gefunden

**Problem:** `bass.dll` nicht gefunden

**Lösung:** DLLs müssen im gleichen Verzeichnis wie die EXE sein oder im PATH.

### 8.4 Plugin nicht geladen

**Problem:** FLAC/Opus-Dateien werden nicht abgespielt

**Lösung:** `external_options` in Solution.json prüfen:
```json
"external_options": {
    "bass": { "BASS_FLAC": true }
}
```

---

## 9. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **Website** | [un4seen.com](https://www.un4seen.com/) |
| **BASS Download** | [un4seen.com/bass.html](https://www.un4seen.com/bass.html) |
| **Dokumentation** | [un4seen.com/doc](https://www.un4seen.com/doc/) |
| **Forum** | [un4seen.com/forum](https://www.un4seen.com/forum/) |
| **Add-ons** | [un4seen.com/bass.html#addons](https://www.un4seen.com/bass.html#addons) |

### See Also

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Local_Externals_Media.md](../../references/externals/Local_Externals_Media.md) — Reference
- [Git_Externals_Media.md](../../references/externals/Git_Externals_Media.md) — Alternativen (miniaudio, openal-soft)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für BASS** |
