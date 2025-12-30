# Tutorial: Registry-Architektur für MyViz

> **Version:** 0.1.0  
> **Datum:** 2025-12-29  
> **Status:** In Planung  
> **Ziel:** Self-Registration für Panels, Dialoge und Menüs

---

## Übersicht

Dieses Tutorial führt schrittweise eine **modulare Registry-Architektur** ein, die es ermöglicht:

1. **Panels** registrieren sich selbst beim Start
2. **Dialoge** registrieren sich selbst beim Start
3. **Menüeinträge** werden automatisch aus Registrierungen generiert
4. **Widgets** können sich für bestimmte Services registrieren

---

## Architektur-Übersicht

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Application                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    ServiceContainer                            │  │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐              │  │
│  │  │  EventBus   │ │ CommandBus  │ │   Settings  │              │  │
│  │  └─────────────┘ └─────────────┘ └─────────────┘              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                       │
│  ┌───────────────────────────┼───────────────────────────────────┐  │
│  │                      Registries                                │  │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐              │  │
│  │  │PanelRegistry│ │DialogRegistry│ │ MenuRegistry│              │  │
│  │  └─────────────┘ └─────────────┘ └─────────────┘              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                       │
│  ┌───────────────────────────┼───────────────────────────────────┐  │
│  │                      Managers                                  │  │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐              │  │
│  │  │PanelManager │ │DialogManager│ │ MenuManager │              │  │
│  │  └─────────────┘ └─────────────┘ └─────────────┘              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                       │
│  ┌───────────────────────────┼───────────────────────────────────┐  │
│  │                      MainWindow                                │  │
│  │  ┌─────────────────────────────────────────────────────────┐  │  │
│  │  │ MenuBar (auto-generated from MenuRegistry)              │  │  │
│  │  ├─────────────────────────────────────────────────────────┤  │  │
│  │  │ DockManager                                             │  │  │
│  │  │  ├── Panel: Spectrum (self-registered)                  │  │  │
│  │  │  ├── Panel: Waveform (self-registered)                  │  │  │
│  │  │  └── Panel: Settings (self-registered)                  │  │  │
│  │  └─────────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Tutorial-Phasen

### Phase 1: ServiceContainer (Dependency Injection)
**Ziel:** Zentrale Verwaltung von Services

```cpp
// Registrierung
container.registerSingleton<IEventBus, EventBus>();
container.registerSingleton<IAudioService, AudioService>();

// Auflösung
auto& eventBus = container.resolve<IEventBus>();
```

**Dateien:**
- `include/Services/ServiceContainer.hpp`
- `include/Services/ServiceContainer.md`

---

### Phase 2: EventBus (Publish/Subscribe)
**Ziel:** Lose Kopplung zwischen Komponenten

```cpp
// Subscribe
eventBus.subscribe<AudioDataEvent>([](const AudioDataEvent& e) {
    // Handle audio data
});

// Publish
eventBus.publish(AudioDataEvent{spectrum, 1024});
```

**Dateien:**
- `include/Services/IEventBus.hpp`
- `include/Services/EventBus.hpp`
- `src/Services/EventBus.cpp`
- `include/Services/Events/Event.hpp`
- `include/Services/Events/AudioEvents.hpp`
- `include/Services/Events/UIEvents.hpp`

---

### Phase 3: PanelRegistry (Self-Registration)
**Ziel:** Panels registrieren sich automatisch

```cpp
// In SpectrumPanel.cpp (am Ende der Datei):
REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)

// Das Makro expandiert zu:
namespace {
    struct SpectrumPanel__AutoPanelReg {
        SpectrumPanel__AutoPanelReg() {
            PanelRegistry::instance().registerPanel(
                PanelDescriptor{"spectrum", "Spectrum Analyzer", 0, true},
                [](ServiceContainer& svc) { 
                    return std::make_unique<SpectrumPanel>(svc); 
                }
            );
        }
    } spectrumPanel__autoPanelRegInstance;
}
```

**Dateien:**
- `include/Services/PanelRegistry.hpp`
- `src/Services/PanelRegistry.cpp`
- `include/UI/Panels/IPanel.hpp`
- `include/UI/Panels/PanelBase.hpp`
- `src/UI/Panels/PanelBase.cpp`

---

### Phase 4: MenuRegistry (Hierarchische Menüs)
**Ziel:** Menüs werden deklarativ aus Registrierungen aufgebaut

```cpp
// Toplevel-Container (File, View, Settings, Help)
REGISTER_MENU_CONTAINER("menu.file", "File", "toplevel", 100)
REGISTER_MENU_CONTAINER("menu.view", "View", "toplevel", 200)

// Items
REGISTER_MENU_ITEM("menu.file.open", "Open Audio...", "menu.file", 100,
    [](ServiceContainer& svc) {
        svc.resolve<IDialogManager>().show("open_audio");
    })

// Panel-Toggle (automatisch für jedes registrierte Panel)
// Wird von PanelManager beim Start generiert
```

**Dateien:**
- `include/Services/MenuRegistry.hpp`
- `src/Services/MenuRegistry.cpp`
- `include/UI/MenuManager.hpp`
- `src/UI/MenuManager.cpp`

---

### Phase 5: DialogRegistry
**Ziel:** Dialoge registrieren sich selbst

```cpp
REGISTER_DIALOG("about", "About MyViz", AboutDialog)
REGISTER_DIALOG("settings", "Settings", SettingsDialog)
```

**Dateien:**
- `include/Services/DialogRegistry.hpp`
- `src/Services/DialogRegistry.cpp`
- `include/UI/Dialogs/IDialog.hpp`
- `include/UI/Dialogs/DialogBase.hpp`

---

### Phase 6: Manager-Integration
**Ziel:** Manager verwenden Registries um UI aufzubauen

```cpp
class PanelManager {
public:
    void initialize(ServiceContainer& svc) {
        // Alle registrierten Panels instantiieren
        for (const auto& desc : PanelRegistry::instance().descriptors()) {
            auto panel = PanelRegistry::instance().create(desc.id, svc);
            
            // In DockManager einhängen
            m_dockManager->addPanel(desc.id, desc.title, std::move(panel));
            
            // Menü-Toggle registrieren
            MenuRegistry::instance().registerItem(
                MenuItemDesc{
                    "menu.view.panels." + desc.id,
                    "menu.view.panels",
                    desc.order,
                    desc.title,
                    [id = desc.id](ServiceContainer& svc) {
                        svc.resolve<IPanelManager>().togglePanel(id);
                    }
                }
            );
        }
    }
};
```

---

## Verzeichnisstruktur (Ziel)

```
MyViz/
├── include/
│   ├── Services/
│   │   ├── ServiceContainer.hpp
│   │   ├── IEventBus.hpp
│   │   ├── EventBus.hpp
│   │   ├── ICommandBus.hpp
│   │   ├── CommandBus.hpp
│   │   ├── PanelRegistry.hpp
│   │   ├── DialogRegistry.hpp
│   │   ├── MenuRegistry.hpp
│   │   ├── Events/
│   │   │   ├── Event.hpp
│   │   │   ├── AudioEvents.hpp
│   │   │   └── UIEvents.hpp
│   │   └── Source.cmake
│   ├── UI/
│   │   ├── Panels/
│   │   │   ├── IPanel.hpp
│   │   │   ├── PanelBase.hpp
│   │   │   ├── SpectrumPanel.hpp
│   │   │   ├── WaveformPanel.hpp
│   │   │   └── Source.cmake
│   │   ├── Dialogs/
│   │   │   ├── IDialog.hpp
│   │   │   ├── DialogBase.hpp
│   │   │   ├── AboutDialog.hpp
│   │   │   └── Source.cmake
│   │   ├── MenuManager.hpp
│   │   ├── PanelManager.hpp
│   │   ├── DialogManager.hpp
│   │   └── Source.cmake
│   └── ...
├── src/
│   ├── Services/
│   │   ├── EventBus.cpp
│   │   ├── CommandBus.cpp
│   │   ├── PanelRegistry.cpp
│   │   ├── DialogRegistry.cpp
│   │   ├── MenuRegistry.cpp
│   │   └── Source.cmake
│   ├── UI/
│   │   ├── Panels/
│   │   │   ├── PanelBase.cpp
│   │   │   ├── SpectrumPanel.cpp
│   │   │   ├── WaveformPanel.cpp
│   │   │   └── Source.cmake
│   │   ├── Dialogs/
│   │   │   ├── DialogBase.cpp
│   │   │   ├── AboutDialog.cpp
│   │   │   └── Source.cmake
│   │   ├── MenuManager.cpp
│   │   ├── PanelManager.cpp
│   │   ├── DialogManager.cpp
│   │   └── Source.cmake
│   └── ...
└── ...
```

---

## Nächste Schritte

1. ~~**Phase 1:** ServiceContainer implementieren~~ ✅
2. ~~**Phase 2:** EventBus mit Event-Typen~~ ✅
3. ~~**Phase 3:** PanelRegistry mit Self-Registration~~ ✅
4. ~~**Phase 4:** MenuRegistry für dynamische Menüs~~ ✅
5. ~~**Phase 5:** DialogRegistry~~ ✅
6. **Phase 6:** Integration in Application/MainWindow (nächster Schritt)
7. **Phase 7:** Beispiel-Panel mit Self-Registration

---

## Referenzen

- ImGui-Version: `registry.zip` (PanelRegistry, MenuRegistry, DialogRegistry)
- LumiPulse: `include.zip` (ServiceContainer, EventBus, PanelBase)
- Qt-ADS: Bereits integriert in DockManager

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.0** | **2025-12-29** | **Initial: Tutorial-Plan** |
