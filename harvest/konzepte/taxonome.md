Kurz:

* **UI → Controller**,
* **Audio/Rendering/Assets/Plugins → Manager/Host/Service**,
* **OS/Window/Device → System**.

Damit bleiben Zuständigkeiten klar, Abhängigkeiten einseitig und der Shutdown planbar.

---

# Empfohlene Taxonomie

| Kategorie         | Zweck                                                  | Beispiele                                                          | Besitzt Ressourcen?           | Typische API                                                 |
| ----------------- | ------------------------------------------------------ | ------------------------------------------------------------------ | ----------------------------- | ------------------------------------------------------------ |
| **System**        | OS-/HW-Integration, Low-Level Handles                  | `WindowsSystem`, `InputSystem`                                     | Ja (HW/OS Handles)            | `initialize()`, `pumpMessages()`, `shutdown()`               |
| **Manager**       | Lebenszyklus + Ownership von **fachlichen** Ressourcen | `AudioManager`, `RenderManager`, `TextureManager`, `PluginManager` | Ja (Devices, Contexts, Pools) | `initialize()`, `load*()`, `start()`, `stop()`, `shutdown()` |
| **Host**          | Kapselt erweiterbare Einheiten/Plugins                 | `VisualizerHost`, `ScriptHost`                                     | Ja (Plugin-Handles)           | `register*()`, `activate()`, `deactivate()`                  |
| **Service**       | Querschnitt, zustandsarm, wiederverwendbar             | `Settings`, `EventBus`, `Profiler`                                 | Eher nein/leicht              | `publish()`, `subscribe()`, `get()/set()`                    |
| **Controller**    | Orchestriert **User-Flows** und UI-Interaktionen       | `UiController`, `FileDialogController`, `PresetController`         | Nein                          | `onFrame()`, `handleAction()`, `shutdown()`                  |
| **Module/Plugin** | Funktionale Bausteine (Visuals, Effekte)               | `IVisualizer`, `IEffect`                                           | Nein (nutzt Manager)          | `update()`, `render()`, `configure()`                        |

**Faustregel:**

* Alles, was **Geräte, Handles, Threads, GPU/Audio-Kontexte** hält → **Manager/System**.
* Alles, was **UI-Intention/Flows** steuert → **Controller**.
* **Visuals** als Plugins: Interface `IVisualizer` + `VisualizerHost` (Manager/Host), der die Instances besitzt. Ein `VisualsController` (optional) steuert nur Auswahl/Parameter aus der UI.

---

# Abhängigkeitsrichtung (einseitig)

```
System  →  Manager/Host  →  Services  →  Controller  →  UI Widgets
    ^            ^              ^             |
    |            |              |             v
   OS/HW      Plugins       EventBus       User Input
```

* **Controller kennt Manager-Interfaces**, nie umgekehrt.
* **Application** hält nur **Interfaces** + Factories (wie besprochen).

---

# Init-/Shutdown-Reihenfolge

**Init:**

1. `WindowsSystem`
2. `MainWindow` (Contexts vorhanden)
3. **Services** (`EventBus`, `Settings`)
4. **Manager/Host** (`AudioManager` → `RenderManager` → `Plugin/VisualizerHost`)
5. **Controller** (UI-Flows, verbinden sich mit Services/Managern)
6. Runloop

**Shutdown (noexcept, idempotent):**

1. **Controller** (Callbacks lösen)
2. **Persist Settings**
3. **Host/Manager** (reverse init: Plugins → Render → Audio)
4. **Services** (EventBus/Settings)
5. `MainWindow`
6. `WindowsSystem`

---

# Namenskonzept (konkret)

* **Audio:** `IAudioManager` + `AudioManager` (PIMPL), optional `AudioController` (UI-seitig: Play/Pause/Device-Auswahl).
* **Rendering:** `IRenderManager` + `RenderManager` (owning OpenGL/Swapchain/ImGui), optional `RenderController` (Profiling, Toggles).
* **Visuals:**

  * `IVisualizer` (Plugin-Interface)
  * `VisualizerHost` (Manager/Host, besitzt Visualizer-Instanzen)
  * `VisualsController` (UI-Auswahl, Parameter-Editing)

---

# Ordner & Interfaces (leichtgewichtig)

```
src/
  app/                Application.* (nur Interfaces includen)
  systems/            WindowsSystem.*, InputSystem.*
  services/           Settings.*, EventBus.*
  managers/           AudioManager.*, RenderManager.*, VisualizerHost.*
  controllers/
    IControllers.hpp  // nur Interfaces (IUiController, IFileDialogController, IVisualsController, ...)
    ControllerFactory.*  // erzeugt konkrete Controller
    ui/               // konkrete Controller-Impls (kennen Manager)
  visuals/
    IVisualizer.hpp
    plugins/...       // konkrete Visualizer
```

**Application.hpp** enthält nur:

* `IControllers.hpp`
* Manager-/Service-**Interfaces** (wenn du sie auch als Interfaces führst)
* Forward decl für `WindowsSystem`/`MainWindow`

---

# Kleine Code-Schablone

```cpp
// managers/IAudioManager.hpp
#pragma once
struct AudioDeviceInfo { /*...*/ };
class IAudioManager {
public:
    virtual ~IAudioManager() = default;
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() noexcept = 0;
    virtual void shutdown() noexcept = 0;
    virtual std::vector<AudioDeviceInfo> devices() const = 0;
};

// visuals/IVisualizer.hpp
#pragma once
class IVisualizer {
public:
    virtual ~IVisualizer() = default;
    virtual void update(double dt) = 0;
    virtual void render() = 0;
};

// managers/VisualizerHost.hpp (Manager/Host)
#pragma once
#include <memory>
#include <vector>
#include "visuals/IVisualizer.hpp"
class VisualizerHost {
public:
    bool initialize(/* deps: IRenderManager&, IAudioManager& */);
    void add(std::unique_ptr<IVisualizer> v);
    void updateRenderAll(double dt);
    void shutdown() noexcept;
private:
    std::vector<std::unique_ptr<IVisualizer>> m_Plugins; // owns
};
```

**Controller nutzt nur Interfaces:**

```cpp
// controllers/VisualsController.hpp (Interface)
#pragma once
class IVisualsController {
public:
    virtual ~IVisualsController() = default;
    virtual void onFrame() = 0;          // ImGui-UI to select visual, tweak params
    virtual void shutdown() noexcept = 0;
};
```

---

# Warum **nicht** „AudioController“ als Hauptding?

* Weil „Controller“ semantisch **UI/Use-Case** bedeutet.
* Der, der **Audio-Streams/Devices** besitzt, **ist** ein **Manager**.
* Der, der Buttons/Shortcuts/Presets bedient, **ist** ein **Controller**.

So bleiben Tests einfacher (Controller kann gemockte `IAudioManager`/`IRenderManager` nutzen) und die Lebenszyklen sind klar.

---

# Erweiterungen

* **Factory-Schicht** auch für Manager (`CreateAudioManager()`, `CreateRenderManager()`), Implementierung verborgen → leichteres Swappen (z. B. Dummy-Renderer in Tests).
* **Capabilities-Query** (`IRenderManager::capabilities()`) für Feature-Gating in Visuals/Controllern.
* **Diagnostics/ProfilerService**: zentral, UI über Controller.
* **EventBus** als *Service*, nicht als Manager.
* **ThreadPools** gehören typischerweise zu Managern (Audio/Render) oder zentralem `TaskManager`.

---

## TL;DR

* Nenne **ressourcenhaltende** Bausteine **Manager/Host**, UI-Flows **Controller**, OS-Integration **System**.
* Visuals als **Plugins** mit `IVisualizer` + `VisualizerHost`, optional UI-seitiger `VisualsController`.
* Application sieht **nur Interfaces + Factories**.
* Reihenfolge beibehalten: **System → Services → Manager/Host → Controller** (Init) und **umgekehrt** (Shutdown).
