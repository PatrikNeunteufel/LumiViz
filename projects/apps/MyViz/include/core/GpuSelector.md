# GpuSelector — GPU Selection and Configuration

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Core::GpuSelector  
> **Dateien:** GpuSelector.hpp, GpuSelector.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** GpuInfo  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konfiguration](#2-konfiguration)
3. [API](#3-api)
4. [Hybrid Graphics](#4-hybrid-graphics)
5. [Verwendung](#5-verwendung)
6. [Changelog](#6-changelog)

---

## 1. Übersicht

### 1.1 Zweck

**GpuSelector** verwaltet GPU-Präferenzen für Systeme mit mehreren Grafikkarten (Hybrid Graphics). Er ermöglicht:

- Laden/Speichern von GPU-Präferenzen
- Auswahl der besten GPU basierend auf Kriterien
- Erkennung ob die "richtige" GPU aktiv ist
- Export-Flags für NVIDIA Optimus / AMD PowerXpress

### 1.2 Problem: Hybrid Graphics

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Laptop mit Hybrid Graphics                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────────┐              ┌──────────────────┐             │
│  │   Intel UHD      │              │  NVIDIA RTX 4070 │             │
│  │   (Integrated)   │              │   (Dedicated)    │             │
│  │   Low Power      │              │  High Performance│             │
│  └────────┬─────────┘              └────────┬─────────┘             │
│           │                                  │                       │
│           └──────────────┬───────────────────┘                       │
│                          │                                           │
│                          ▼                                           │
│                   ┌──────────────┐                                   │
│                   │   Display    │                                   │
│                   └──────────────┘                                   │
│                                                                      │
│  Problem: OS wählt oft iGPU statt dGPU für Apps                     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Konfiguration

### 2.1 Config-Datei Format (gpu.ini)

```ini
[GPU]
PreferHighPerformance=true
PreferredVendor=NVIDIA
PreferredName=RTX 4070
```

### 2.2 GpuPreference Struktur

```cpp
struct GpuPreference
{
    bool preferHighPerformance{true};       // Dedicated GPU bevorzugen
    std::optional<GpuVendor> preferredVendor;  // NVIDIA, AMD, Intel
    std::optional<std::string> preferredName;  // Teil des GPU-Namens
};
```

---

## 3. API

### 3.1 Laden/Speichern

```cpp
GpuSelector selector;

// Config laden
if (!selector.loadConfig("gpu.ini")) {
    // Config existiert nicht, Default erstellen
    selector.createDefaultConfig("gpu.ini");
}

// Config speichern
selector.saveConfig("gpu.ini");
```

### 3.2 GPU Auswahl

```cpp
// Verfügbare GPUs ermitteln
auto gpus = GpuInfo::enumerate();

// Beste GPU basierend auf Präferenzen auswählen
const GpuDevice* preferred = selector.selectGpu(gpus);
if (preferred) {
    qDebug() << "Recommended GPU:" << preferred->name.c_str();
}
```

**Auswahlreihenfolge:**
1. Wenn `preferredName` gesetzt → Nach Name suchen
2. Wenn `preferredVendor` gesetzt → Nach Vendor suchen
3. Wenn `preferHighPerformance` → Beste dedizierte GPU
4. Fallback → Erste verfügbare GPU

### 3.3 GPU-Mismatch Erkennung

```cpp
// Nach OpenGL-Initialisierung
QString activeGpu = QString::fromLatin1(
    reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

if (!selector.isPreferredGpuActive(activeGpu.toStdString())) {
    QString warning = QString::fromStdString(
        selector.getGpuMismatchWarning(activeGpu.toStdString(), gpus));
    
    QMessageBox::warning(nullptr, "GPU Warning", warning);
}
```

### 3.4 Präferenzen setzen

```cpp
// Programmatisch
selector.setPreferHighPerformance(true);
selector.setPreferredVendor(GpuVendor::NVIDIA);
selector.setPreferredName("RTX 4070");

// Oder komplett
GpuPreference pref;
pref.preferHighPerformance = true;
pref.preferredVendor = GpuVendor::AMD;
selector.setPreference(pref);

// Präferenz zurücksetzen
selector.clearPreferredVendor();
selector.clearPreferredName();
```

---

## 4. Hybrid Graphics

### 4.1 Export-Flags

Für NVIDIA Optimus und AMD PowerXpress werden spezielle Export-Symbole benötigt, die VOR dem Programmstart gelesen werden:

```cpp
// In main.cpp einfügen:
#include "core/GpuSelector.hpp"

MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU

int main(int argc, char* argv[]) {
    // ...
}
```

### 4.2 Was das Makro macht

```cpp
// Windows:
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Linux/macOS: Kein Export nötig (andere Mechanismen)
```

### 4.3 Einschränkungen

| Aspekt | Beschreibung |
|--------|-------------|
| **Timing** | Flags werden beim EXE-Start gelesen, nicht zur Laufzeit |
| **User-Override** | Benutzer kann in Treiber-Settings überschreiben |
| **Garantie** | Keine 100% Garantie, nur "Hinweis" an Treiber |

---

## 5. Verwendung

### 5.1 Typischer Startup-Flow

```cpp
// 1. Export-Flags (in main.cpp)
MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU

int main() {
    // 2. Config laden
    GpuSelector selector;
    selector.loadConfig("gpu.ini");
    
    // 3. GPUs enumerieren
    auto gpus = GpuInfo::enumerate();
    GpuInfo::logGpuInfo(gpus);
    
    // 4. Empfohlene GPU bestimmen
    const GpuDevice* preferred = selector.selectGpu(gpus);
    qDebug() << "Recommended:" << preferred->name.c_str();
    
    // 5. Qt App starten
    QApplication app(argc, argv);
    
    // 6. OpenGL Context erstellen (MainWindow/VisualizerWidget)
    MainWindow window;
    
    // 7. Aktive GPU prüfen
    QString activeGpu = window.getActiveGpuName();
    if (!selector.isPreferredGpuActive(activeGpu.toStdString())) {
        // Warnung anzeigen
    }
    
    return app.exec();
}
```

### 5.2 GPU in Settings-Dialog

```cpp
// ConfigPanel oder PreferencesDialog
void GpuSettingsWidget::populate() {
    auto gpus = GpuInfo::enumerate();
    
    for (const auto& gpu : gpus) {
        m_gpuCombo->addItem(
            QString::fromStdString(gpu.name),
            QVariant::fromValue(gpu)
        );
    }
}

void GpuSettingsWidget::onSave() {
    auto gpu = m_gpuCombo->currentData().value<GpuDevice>();
    
    GpuSelector selector;
    selector.setPreferredName(gpu.name);
    selector.saveConfig("gpu.ini");
    
    QMessageBox::information(this, "GPU", 
        "Changes take effect after restart.");
}
```

---

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-31** | **Initial: Selection, Config, Export-Flags** |
