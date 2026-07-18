# viz2025 – Taxonomie & Basisklassen (überarbeitet)

*Stand: 28.08.2025 – abgestimmt mit den Architekturentscheidungen aus den bisherigen viz2025‑Chats (Controller/Manager/Registry‑Vereinheitlichung, ServiceContainer, CommandBus/EventBus, Node‑Editor, VisualizerHost, CPU/GPU‑Dualpfad, AVS/Milkdrop‑Kompatibilität).*
*Ziel: Ein konsistentes Vokabular (Taxonomie) + sauberes Set an Basisklassen zur Vermeidung von Redundanz bei voller Kontrolle.*

---

## 1) Leitlinien

* **Ownership klar trennen**: Ressourcenhaltend (HW/Handles/Threads/Contexts) = *Manager/Host/System*; Use‑Cases/UI‑Flows = *Controller*; Querschnitt = *Services*.
* **Abhängigkeitsrichtung einseitig**: `System → Services → Manager/Host → Controller → UI`.
* **Interfaces first**: Application kennt nur Interfaces/Factories. Implementierungen per PIMPL/DI austauschbar.
* **Lifecycle deterministisch**: idempotente `initialize()/shutdown()`; Reverse‑Shutdown.
* **CPU/GPU‑Dualpfad**: identische Operator‑Schnittstellen; Backends über Capability-Query/Factories wählbar.
* **Modularität & Erweiterbarkeit**: Plugins (Visuals/Effekte) über Hosts; Registries erfassen Angebote; Command/Event‑Bus entkoppelt.
* **Node‑Graph**: Typisierte Ports/Links; mandatory vs. optional; Validierung in Editor **und** Pipeline.

---

## 2) Taxonomie (Begriffe & Zuständigkeiten)

| Kategorie         | Zweck                                          | Beispiele                                                                   | Hält Ressourcen?              | Typische API                                   |
| ----------------- | ---------------------------------------------- | --------------------------------------------------------------------------- | ----------------------------- | ---------------------------------------------- |
| **System**        | OS/HW‑Integration, Window, Input               | `WindowsSystem`, `InputSystem`                                              | Ja (HW/OS Handles)            | `initialize()`, `pumpMessages()`, `shutdown()` |
| **Service**       | Querschnitt, zustandsarm, wiederverwendbar     | `Settings`, `ProfilerService`, `EventBus`, `CommandBus`, `ServiceContainer` | Eher nein                     | `get()/set()`, `publish()`, `dispatch()`       |
| **Manager**       | Lebenszyklus + Ownership fachlicher Ressourcen | `AudioManager`, `RenderManager`, `TextureManager`                           | Ja (Devices/Contexts/Threads) | `initialize()`, `start()/stop()`, `shutdown()` |
| **Host**          | Laufzeitcontainer für Plugins/Module           | `VisualizerHost`, `ScriptHost`                                              | Ja (Plugin‑Instanzen)         | `register*()`, `activate()/deactivate()`       |
| **Controller**    | Orchestriert Use‑Cases & UI‑Flows              | `UiController`, `VisualsController`, `FileDialogController`                 | Nein                          | `onFrame()`, `handleAction()`, `shutdown()`    |
| **Registry**      | Katalog/Discovery von Typen/Factories          | `PanelRegistry`, `VisualsRegistry`, `MenuRegistry`                          | Nein (nur Einträge)           | `register()`, `find()/foreach()`               |
| **Agent**         | Autonome Logik/Überwacher/Worker               | `PresetAutoSaverAgent`, `AudioDeviceWatchdogAgent`                          | Optional (leichte Ressourcen) | `tick()`, `start()/stop()`                     |
| **Module/Plugin** | Funktionale Bausteine (Visual/Effekt)          | `IVisualizer`, `IEffect`, Milkdrop/AVS‑Adapter                              | Nein (nutzt Manager)          | `update(dt)`, `render()`, `configure()`        |
| **Node**          | Graph‑Knoten im Composer                       | `AudioSourceNode`, `EffectNode`, `AfterEffectNode`, `PresetIO`              | Nein (nutzt Manager)          | `evaluate()`, `validate()`, `serialize()`      |

---

## 3) Basisklassen & Interfaces (C++‑Schablonen)

> **Konventionen:**
>
> * Interfaces mit `I*`, abstrakte Basen als `*Base`.
> * Alle `shutdown()` sind `noexcept` und idempotent.
> * Optional Doxygen (`///`) für API‑Doku.

```cpp
/// ServiceContainer: zentraler DI‑Knoten
class IServiceContainer {
public:
    virtual ~IServiceContainer() = default;
    template<class T> void set(std::shared_ptr<T> svc);
    template<class T> std::shared_ptr<T> get() const;
};

/// CommandBus
struct ICommand { virtual ~ICommand() = default; };
class ICommandBus {
public:
    virtual ~ICommandBus() = default;
    virtual void dispatch(const ICommand& cmd) = 0;
};

/// EventBus
struct IEvent { virtual ~IEvent() = default; };
class IEventBus {
public:
    virtual ~IEventBus() = default;
    using HandlerId = uint64_t;
    template<class E> HandlerId subscribe(std::function<void(const E&)> cb);
    template<class E> void unsubscribe(HandlerId id);
    template<class E> void publish(const E& ev) const;
};

/// System
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() noexcept = 0;
};

/// Manager/Host
class IManager {
public:
    virtual ~IManager() = default;
    virtual bool initialize(IServiceContainer&) = 0;
    virtual void start() {}
    virtual void stop() noexcept {}
    virtual void shutdown() noexcept = 0;
};

class IHost : public IManager {
public:
    virtual void activate() {}
    virtual void deactivate() noexcept {}
};

class IVisualizer { public: virtual ~IVisualizer()=default; virtual void update(double dt)=0; virtual void render()=0; };
class IVisualizerHost : public IHost {
public:
    virtual void add(std::unique_ptr<IVisualizer> v) = 0;
    virtual void updateRenderAll(double dt) = 0;
};

/// Controller
class IController {
public:
    virtual ~IController() = default;
    virtual void onFrame() = 0;
    virtual void shutdown() noexcept = 0;
};

/// Registry
class IRegistryBase { public: virtual ~IRegistryBase() = default; };

template<class Key, class Value>
class IRegistry : public IRegistryBase {
public:
    using key_type = Key; using value_type = Value;
    virtual void registerItem(const Key& k, Value v) = 0;
    virtual const Value* find(const Key& k) const = 0;
    virtual void foreach(std::function<void(const Key&, const Value&)> f) const = 0;
};

/// Agent
class IAgent {
public:
    virtual ~IAgent() = default;
    virtual void start() {}
    virtual void tick(double dt) = 0;
    virtual void stop() noexcept {}
};

/// Node
class INode {
public:
    virtual ~INode() = default;
    virtual void validate() const = 0;
    virtual void evaluate(double dt) = 0;
    virtual void serialize() const = 0;
};
```

---

## 4) Lebenszyklus & Reihenfolge

1. System → 2. Services → 3. Manager/Hosts → 4. Registries → 5. Controller → 6. Agents → 7. Runloop

Shutdown: umgekehrt, alles `noexcept + idempotent`.

---

## 5) CPU/GPU‑Dualpfad

* Gemeinsames Interface `IEffect`, Implementierungen `EffectCPU`, `EffectGPU`.
* Auswahl via Factory + `RenderCaps`.
* Gemeinsame Datentypen (`AudioFrame`, `Spectrum`, `Image`).

---

## 6) AVS/Milkdrop‑Integration

* Adapter‑Plugins `MilkdropAdapterVisualizer`, `AvsAdapterEffect`.
* Übersetzung Preset‑Graph → Node‑Graph.
* Parameter‑Binding im `VisualsController`.

---

## 7) Ordnerstruktur (empfohlen)

```
src/
  app/
  systems/
  services/
  managers/
  hosts/
  controllers/
  registry/
  nodes/
  visuals/
  effects/
  agents/
  ui/
```

---

## 8) Offen/zu klären

1. `CommandBus` mit Response/Futures?
2. Preset‑Format (JSON/TOML) und Versionierung?
3. GPU‑Fallback Strategy?
4. Feste Agentenliste für v1?
5. Shader‑Kompatibilität Milkdrop v3?

---

## 9) TL;DR

* Ressourcenhaltend = **Manager/Host/System**, UI‑Flows = **Controller**, Querschnitt = **Services**.
* **Interfaces + Registries + Hosts** ermöglichen Plugins und Adapter.
* **Node‑Graph** mit typisierten Ports/Links und Mandatory‑Check ist zentral.
* **Lifecycle**: deterministisch, reverse shutdown, idempotent.
* **CPU/GPU** teilen API, Implementierung via Factory.
