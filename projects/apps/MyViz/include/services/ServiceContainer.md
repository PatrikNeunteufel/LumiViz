# ServiceContainer — Dependency Injection Container

> **Version:** 1.0.0  
> **Datum:** 2025-12-29  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Services::ServiceContainer  
> **Dateien:** ServiceContainer.hpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** C++17 STL  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konzepte](#2-konzepte)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Best Practices](#5-best-practices)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Der ServiceContainer ist ein **Dependency Injection (DI) Container**, der die zentrale Verwaltung von Services ermöglicht. Er löst das Problem der harten Kopplung zwischen Komponenten.

### 1.2 Warum DI?

**Ohne DI:**
```cpp
class AudioVisualizer {
    AudioEngine m_engine;      // Harte Abhängigkeit
    FileLogger m_logger;       // Harte Abhängigkeit
public:
    AudioVisualizer() 
        : m_engine()           // Welche Konfiguration?
        , m_logger("viz.log")  // Nicht testbar!
    {}
};
```

**Mit DI:**
```cpp
class AudioVisualizer {
    IAudioEngine& m_engine;    // Interface
    ILogger& m_logger;         // Interface
public:
    AudioVisualizer(IAudioEngine& engine, ILogger& logger)
        : m_engine(engine)
        , m_logger(logger)
    {}
};

// Produktion:
container.registerSingleton<IAudioEngine, BassAudioEngine>();
container.registerSingleton<ILogger, FileLogger>();

// Test:
container.registerSingleton<IAudioEngine, MockAudioEngine>();
container.registerSingleton<ILogger, NullLogger>();
```

### 1.3 Vorteile

| Aspekt | Ohne DI | Mit DI |
|--------|---------|--------|
| **Kopplung** | Hart (konkrete Klassen) | Lose (Interfaces) |
| **Testbarkeit** | Schwierig (echte Deps) | Einfach (Mocks) |
| **Konfiguration** | Verstreut | Zentral |
| **Austauschbarkeit** | Recompile nötig | Runtime möglich |

---

## 2. Konzepte

### 2.1 Service-Lifetimes

```
┌─────────────────────────────────────────────────────────────────┐
│                        ServiceContainer                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Singleton (eine Instanz für alle)                              │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ IEventBus ──────────────────────► EventBus (Instanz)    │    │
│  │ IAudioService ──────────────────► AudioService (Inst.)  │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
│  Transient (neue Instanz bei jeder Auflösung)                   │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ IWorker ──► Factory ──► Worker #1                       │    │
│  │ IWorker ──► Factory ──► Worker #2                       │    │
│  │ IWorker ──► Factory ──► Worker #3                       │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Registrierungsarten

| Art | Methode | Beschreibung |
|-----|---------|--------------|
| **Interface→Impl** | `registerSingleton<I, T>()` | Typ-Mapping |
| **Factory** | `registerSingleton<I>(factory)` | Lazy mit Deps |
| **Instance** | `registerInstance<I>(ptr)` | Existierende Instanz |
| **Transient** | `registerTransient<I, T>()` | Immer neue Instanz |

---

## 3. API

### 3.1 Registrierung

```cpp
// Interface → Implementation (Singleton)
container.registerSingleton<IEventBus, EventBus>();

// Mit Factory (für Abhängigkeiten)
container.registerSingleton<IAudioService>([](ServiceContainer& c) {
    return std::make_unique<AudioService>(
        c.resolve<IEventBus>(),
        c.resolve<ILogger>()
    );
});

// Existierende Instanz
auto logger = std::make_unique<FileLogger>("app.log");
container.registerInstance<ILogger>(std::move(logger));

// Transient (immer neue Instanz)
container.registerTransient<IWorker, BackgroundWorker>();
```

### 3.2 Auflösung

```cpp
// Sichere Auflösung (throws wenn nicht vorhanden)
auto& eventBus = container.resolve<IEventBus>();

// Optionale Auflösung (nullptr wenn nicht vorhanden)
if (auto* db = container.tryResolve<IDatabase>()) {
    db->connect();
}

// Transient-Instanz erstellen
auto worker = container.createTransient<IWorker>();
```

### 3.3 Query & Management

```cpp
// Prüfen ob registriert
if (container.has<IEventBus>()) { ... }

// Service entfernen
container.unregister<IEventBus>();

// Alles löschen
container.clear();
```

---

## 4. Verwendung

### 4.1 Typischer Setup in Application

```cpp
class Application {
    ServiceContainer m_services;
    
public:
    bool init() {
        // 1. Core Services
        m_services.registerSingleton<IEventBus, EventBus>();
        m_services.registerSingleton<ILogger, FileLogger>();
        
        // 2. Audio Services
        m_services.registerSingleton<IAudioService>([](ServiceContainer& c) {
            return std::make_unique<AudioService>(c.resolve<IEventBus>());
        });
        
        // 3. UI Services (mit Qt-Integration)
        m_services.registerSingleton<IThemeManager, ThemeManager>();
        
        return true;
    }
    
    ServiceContainer& services() { return m_services; }
};
```

### 4.2 Verwendung in Panels

```cpp
class SpectrumPanel : public PanelBase {
public:
    SpectrumPanel(ServiceContainer& services)
        : m_eventBus(services.resolve<IEventBus>())
        , m_audio(services.resolve<IAudioService>())
    {
        // Subscribe to audio events
        m_eventBus.subscribe<AudioDataEvent>([this](const auto& e) {
            updateSpectrum(e.spectrum);
        });
    }
    
private:
    IEventBus& m_eventBus;
    IAudioService& m_audio;
};
```

### 4.3 Self-Registration Pattern

```cpp
// Am Ende von SpectrumPanel.cpp:
REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)

// Das Makro registriert eine Factory:
PanelRegistry::instance().registerPanel(
    PanelDescriptor{"spectrum", "Spectrum Analyzer", 0, true},
    [](ServiceContainer& svc) { 
        return std::make_unique<SpectrumPanel>(svc); 
    }
);
```

---

## 5. Best Practices

### 5.1 Interface-First Design

```cpp
// ❌ Schlecht: Konkrete Abhängigkeit
class Panel {
    AudioEngine m_engine;  // Konkret!
};

// ✅ Gut: Interface-Abhängigkeit
class Panel {
    IAudioEngine& m_engine;  // Interface!
public:
    Panel(IAudioEngine& engine) : m_engine(engine) {}
};
```

### 5.2 Constructor Injection bevorzugen

```cpp
// ❌ Schlecht: Service Locator Pattern
class Panel {
    void doSomething() {
        auto& audio = g_container.resolve<IAudioService>();  // Versteckte Dep!
    }
};

// ✅ Gut: Constructor Injection
class Panel {
    IAudioService& m_audio;
public:
    Panel(IAudioService& audio) : m_audio(audio) {}
    void doSomething() {
        m_audio.play();  // Explizite Dep!
    }
};
```

### 5.3 Registrierungsreihenfolge beachten

```cpp
// Abhängigkeiten zuerst registrieren!
container.registerSingleton<ILogger, FileLogger>();           // 1. Keine Deps
container.registerSingleton<IEventBus, EventBus>();           // 2. Keine Deps
container.registerSingleton<IAudioService>([](auto& c) {      // 3. Hat Deps!
    return std::make_unique<AudioService>(
        c.resolve<ILogger>(),      // Muss vorher registriert sein
        c.resolve<IEventBus>()     // Muss vorher registriert sein
    );
});
```

---

## 6. Thread-Sicherheit

### 6.1 Garantien

| Operation | Thread-Safe |
|-----------|-------------|
| `registerSingleton()` | ✅ Ja (mutex) |
| `registerInstance()` | ✅ Ja (mutex) |
| `resolve()` | ✅ Ja (mutex) |
| `tryResolve()` | ✅ Ja (mutex) |
| Zugriff auf Service | ❌ Service-abhängig |

### 6.2 Empfehlung

```cpp
// Registrierung: Nur im Main-Thread bei Startup
void Application::init() {
    m_services.registerSingleton<IEventBus, EventBus>();
    // ...
}

// Auflösung: Kann von jedem Thread erfolgen
void WorkerThread::run() {
    auto& logger = m_services.resolve<ILogger>();  // Thread-safe
    logger.log("Worker started");                   // Logger muss thread-safe sein!
}
```

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-29** | **Initial: Singleton, Transient, Factory, Instance** |
