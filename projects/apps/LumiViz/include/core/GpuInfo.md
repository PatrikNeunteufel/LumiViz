# GpuInfo — GPU-Erkennung

> **Version:** 2.0.0  
> **Datum:** 2026-08-01  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Core::GpuInfo  
> **Dateien:** GpuInfo.hpp, GpuInfo.cpp  
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
5. [Plattformen](#5-plattformen)
6. [Changelog](#6-changelog)

---

## 1. Übersicht

### 1.1 Zweck

GpuInfo zählt alle verfügbaren GPUs des Systems auf (Name, Hersteller, Typ,
VRAM). Das ist die Anzeige- und Log-Grundlage für Systeme mit Hybrid-Grafik
(integrierte + dedizierte Karte).

**Abgrenzung:** Welche GPU tatsächlich rendert, steuert seit Session 62 der
per-Anwendung-Windows-Eintrag `UserGpuPreferences` — siehe
[GpuPreference.md](GpuPreference.md). Das frühere Duo aus `gpu.ini`/`GpuSelector`
und Export-Flags (`NvOptimusEnablement` u. a.) ist entfernt: es konnte die GPU
nie verbindlich wählen (Windows überstimmt die Flags, sobald ein
UserGpuPreferences-Eintrag existiert — und ohne Eintrag griff es auf diesem
Gerät nachweislich nicht).

### 1.2 Problem: Hybrid-Grafik

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

Windows wählt standardmäßig die integrierte GPU.

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

---

## 4. Verwendung

### 4.1 Beim Anwendungsstart

```cpp
// In Application::init()
auto gpus = GpuInfo::enumerate();
GpuInfo::logGpuInfo(gpus);
```

### 4.2 Anzeige im SettingsPanel

Das SettingsPanel (Performance-Tab) zeigt neben der GPU-Auswahl die
tatsächlich genutzte Karte (`GL_RENDERER` eines frischen Kontexts) und listet
im Tooltip alle per `enumerate()` erkannten Karten.

### 4.3 Nach OpenGL-Context

```cpp
const char* renderer = glGetString(GL_RENDERER);
// "AMD Radeon(TM) 610M"                  ← integrierte Karte
// "NVIDIA GeForce RTX 4090 Laptop GPU"  ← dedizierte Karte
```

---

## 5. Plattformen

### 5.1 Übersicht

| Plattform | Status | API | Benötigt |
|-----------|--------|-----|----------|
| Windows | ✅ Implementiert | DXGI | dxgi.lib (automatisch) |
| Linux | ✅ Implementiert | sysfs + lspci | lspci (optional) |
| macOS | ✅ Implementiert | IOKit | IOKit.framework |

### 5.2 Windows

Verwendet DXGI (DirectX Graphics Infrastructure) für die GPU-Enumeration.
- Keine zusätzlichen Dependencies
- `dxgi.lib` wird automatisch verlinkt

### 5.3 Linux

Verwendet zwei Methoden:
1. **sysfs** (`/sys/class/drm/`) - Für Vendor/Device IDs
2. **lspci** - Für GPU-Namen (optional, Fallback auf IDs)

### 5.4 macOS

Verwendet IOKit für die GPU-Enumeration (Framework-Verlinkung in CMake nötig:
IOKit + CoreFoundation).

### 5.5 Fallback

Wenn die Enumeration auf einer Plattform fehlschlägt:
- Eine leere GPU-Liste wird zurückgegeben
- Der OpenGL-Renderer-String wird später in `initializeGL()` geloggt
- Die Anwendung funktioniert trotzdem

---

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2026-08-01** | **Session 62: GpuSelector/gpu.ini/Export-Flags entfernt — GPU-Wahl lebt jetzt in [GpuPreference.md](GpuPreference.md) (UserGpuPreferences-Registry, SettingsPanel). GpuInfo auf reine Erkennung reduziert** |
| 1.0.0 | 2025-12-28 | Initial: DXGI-Enumeration, Config-System, Export-Flags |
