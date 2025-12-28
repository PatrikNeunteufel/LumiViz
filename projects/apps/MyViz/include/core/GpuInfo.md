# GpuInfo & GpuSelector — GPU-Erkennung und Auswahl

> **Version:** 1.0.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Core::GpuInfo, MyViz::Core::GpuSelector  
> **Dateien:** GpuInfo.hpp, GpuInfo.cpp, GpuSelector.hpp, GpuSelector.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** DXGI (Windows), BasicLogger  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Konfigurationsdatei](#5-konfigurationsdatei)
6. [Export-Flags](#6-export-flags)
7. [Plattformen](#7-plattformen)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Diese Module ermöglichen die Erkennung aller verfügbaren GPUs und die Konfiguration, welche GPU für das Rendering verwendet werden soll. Dies ist besonders wichtig für Laptops mit Hybrid-Grafik (NVIDIA Optimus / AMD PowerXpress).

### 1.2 Module

| Modul | Verantwortlichkeit |
|-------|-------------------|
| **GpuInfo** | GPU-Enumeration via DXGI |
| **GpuSelector** | Präferenz-Management und Auswahl |

### 1.3 Problem: Hybrid-Grafik

Moderne Laptops haben oft zwei GPUs:

```
┌─────────────────────────────────────────────────────────────┐
│ System                                                      │
├─────────────────────────────────────────────────────────────┤
│ ┌─────────────────┐     ┌─────────────────────────────────┐ │
│ │ Integrated GPU  │     │      Dedicated GPU              │ │
│ │ (AMD/Intel)     │     │      (NVIDIA RTX)               │ │
│ │                 │     │                                 │ │
│ │ • Stromsparend  │     │ • Hohe Leistung                 │ │
│ │ • Immer aktiv   │     │ • Muss angefordert werden       │ │
│ │ • Standard      │     │ • Mehr Stromverbrauch           │ │
│ └─────────────────┘     └─────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

Windows wählt standardmäßig die integrierte GPU → schlechte Performance!

---

## 2. Abhängigkeiten

| Dependency | Plattform | Zweck |
|------------|-----------|-------|
| DXGI | Windows | GPU-Enumeration |
| dxgi.lib | Windows | Linker-Library |
| BasicLogger | Alle | Logging |

---

## 3. API

### 3.1 Enums

```cpp
enum class GpuVendor { Unknown, NVIDIA, AMD, Intel, Microsoft, Other };
enum class GpuType { Unknown, Integrated, Dedicated, Software };
```

### 3.2 GpuDevice Struktur

```cpp
struct GpuDevice
{
    std::string name;           // "NVIDIA GeForce RTX 4090"
    GpuVendor vendor;           // GpuVendor::NVIDIA
    GpuType type;               // GpuType::Dedicated
    uint32_t vendorId;          // 0x10DE
    uint32_t deviceId;          // ...
    uint64_t dedicatedVideoMemory;  // VRAM in Bytes
    
    uint64_t vramMB() const;           // VRAM in MB
    bool isHighPerformance() const;    // Dediziert oder >512MB VRAM
};
```

### 3.3 GpuInfo Klasse

| Methode | Beschreibung |
|---------|--------------|
| `enumerate()` | Listet alle GPUs auf |
| `findBestGpu(gpus)` | Findet beste dedizierte GPU |
| `findByName(gpus, name)` | Sucht nach Namensteil |
| `findByVendor(gpus, vendor)` | Sucht nach Hersteller |
| `logGpuInfo(gpus)` | Loggt alle GPU-Infos |

### 3.4 GpuSelector Klasse

| Methode | Beschreibung |
|---------|--------------|
| `loadConfig(file)` | Lädt Präferenzen |
| `saveConfig(file)` | Speichert Präferenzen |
| `createDefaultConfig(file)` | Erstellt Standard-Config |
| `selectGpu(gpus)` | Wählt GPU nach Präferenz |
| `isPreferredGpuActive(name)` | Prüft aktive GPU |
| `getGpuMismatchWarning(...)` | Generiert Warntext |

---

## 4. Verwendung

### 4.1 Beim Anwendungsstart

```cpp
// In Application::init()

// 1. GPUs enumerieren
auto gpus = GpuInfo::enumerate();
GpuInfo::logGpuInfo(gpus);

// 2. Config laden
GpuSelector selector;
selector.createDefaultConfig("gpu.ini");
selector.loadConfig("gpu.ini");

// 3. Bevorzugte GPU ermitteln
const GpuDevice* preferred = selector.selectGpu(gpus);
if (preferred)
{
    BasicLogger::logInfo("Preferred: " + preferred->name);
}
```

### 4.2 Nach OpenGL-Context

```cpp
// In VisualizerWidget::initializeGL()
const char* renderer = glGetString(GL_RENDERER);
// "AMD Radeon(TM) 610M"  ← falsche GPU!
// "NVIDIA GeForce RTX 4090 Laptop GPU"  ← richtige GPU!
```

---

## 5. Konfigurationsdatei

### 5.1 Format (gpu.ini)

```ini
# MyViz GPU Configuration

[GPU]
PreferHighPerformance=true
PreferredVendor=NVIDIA
PreferredName=RTX 4090
```

### 5.2 Optionen

| Option | Werte | Beschreibung |
|--------|-------|--------------|
| `PreferHighPerformance` | true/false | Dedizierte GPU bevorzugen |
| `PreferredVendor` | NVIDIA/AMD/Intel | Hersteller bevorzugen |
| `PreferredName` | String | Namensteil suchen |

---

## 6. Export-Flags

### 6.1 Problem

Die GPU-Auswahl passiert **bevor** die Anwendung startet. Der Treiber entscheidet basierend auf:
1. Windows Graphics Settings
2. NVIDIA Control Panel / AMD Adrenalin
3. **Export-Symbole in der Executable**

### 6.2 Lösung: Export-Symbole

In `main.cpp`:

```cpp
#include "Core/GpuSelector.hpp"

// Diese Symbole werden vom Treiber gelesen
MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU
```

Das Macro expandiert zu:

```cpp
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
```

### 6.3 Hinweis

Die Export-Flags sind nur **Hints**. Der User kann sie in Windows-Einstellungen überschreiben. Die GpuSelector-Config kann nicht zur Laufzeit die GPU wechseln - sie dient der Erkennung und Warnung.

---

## 7. Plattformen

### 7.1 Übersicht

| Plattform | Status | API | Benötigt |
|-----------|--------|-----|----------|
| Windows | ✅ Implementiert | DXGI | dxgi.lib (automatisch) |
| Linux | ✅ Implementiert | sysfs + lspci | lspci (optional) |
| macOS | ✅ Implementiert | IOKit | IOKit.framework |

### 7.2 Windows

Verwendet DXGI (DirectX Graphics Infrastructure) für die GPU-Enumeration.
- Keine zusätzlichen Dependencies
- `dxgi.lib` wird automatisch verlinkt

### 7.3 Linux

Verwendet zwei Methoden:
1. **sysfs** (`/sys/class/drm/`) - Für Vendor/Device IDs
2. **lspci** - Für GPU-Namen (optional, Fallback auf IDs)

```bash
# lspci installieren (falls nicht vorhanden)
sudo apt install pciutils  # Debian/Ubuntu
sudo dnf install pciutils  # Fedora
```

### 7.4 macOS

Verwendet IOKit für die GPU-Enumeration.

**CMake-Integration erforderlich:**

```cmake
if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework IOKit"
        "-framework CoreFoundation"
    )
endif()
```

### 7.5 Fallback

Wenn die Enumeration auf einer Plattform fehlschlägt:
- Eine leere GPU-Liste wird zurückgegeben
- Der OpenGL-Renderer-String wird später in `initializeGL()` geloggt
- Die Anwendung funktioniert trotzdem

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-28** | **Initial: DXGI-Enumeration, Config-System, Export-Flags** |
