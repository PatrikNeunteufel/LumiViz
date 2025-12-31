# EventBus — Publish/Subscribe Event System

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Services::EventBus  
> **Dateien:** IEventBus.hpp, EventBus.hpp, EventBus.cpp, events/Event.hpp, events/UIEvents.hpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** C++17 STL  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [Events definieren](#3-events-definieren)
4. [API](#4-api)
5. [Verwendung](#5-verwendung)
6. [Existierende Events](#6-existierende-events)
7. [Best Practices](#7-best-practices)
8. [Thread-Sicherheit](#8-thread-sicherheit)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Der **EventBus** implementiert das **Publish/Subscribe Pattern** für lose Kopplung zwischen Komponenten. Anstatt direkte Methodenaufrufe zwischen Objekten zu verwenden, kommunizieren Komponenten über Events.

### 1.2 Vorteile

| Aspekt | Ohne EventBus | Mit EventBus |
|--------|---------------|--------------|
| **Kopplung** | Direkte Referenzen | Lose über Events |
| **Erweiterbarkeit** | Änderung am Publisher | Neuer Subscriber |
| **Testbarkeit** | Mock-Objekte nötig | Events verifizieren |
| **Multi-Listener** | Manuell verwalten | Automatisch |

### 1.3 Konzept

```
┌─────────────┐     publish()     ┌─────────────┐     notify()     ┌─────────────┐
│  Publisher  │ ─────────────────► │  EventBus   │ ─────────────────► │ Subscriber  │
│ (AudioEngine)│                   │             │                   │  (Panel)    │
└─────────────┘                   └─────────────┘                   └─────────────┘
                                         │
                                         ├──────────────────────────► Subscriber 2
                                         │
                                         └──────────────────────────► Subscriber 3
```

---

## 2. Architektur

### 2.1 Klassendiagramm

```
                    ┌─────────────────────┐
                    │     IEventBus       │  (Interface)
                    ├─────────────────────┤
                    │ + subscribe<T>()    │
                    │ + unsubscribe()     │
                    │ + publish<T>()      │
                    │ + queue<T>()        │
                    │ + dispatchQueued()  │
                    └──────────┬──────────┘
                               │
                               ▼
                    ┌─────────────────────┐
                    │      EventBus       │  (Implementation)
                    ├─────────────────────┤
                    │ - m_subscribers     │
                    │ - m_queue           │
                    │ - m_nextId          │
                    │ - m_mutex           │
                    └─────────────────────┘
```

### 2.2 Event-Basisklasse

```
                    ┌─────────────────────┐
                    │       Event         │  (Basisklasse)
                    ├─────────────────────┤
                    │ + typeName()        │
                    │ + handled           │
                    └──────────┬──────────┘
                               │
         ┌─────────────────────┼─────────────────────┐
         ▼                     ▼                     ▼
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│ PlaybackState   │  │ AudioDataEvent  │  │ TrackChanged    │
│     Event       │  │                 │  │     Event       │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

---

## 3. Events definieren

### 3.1 Event-Struktur

```cpp
// In events/MyEvents.hpp
#pragma once
#include "services/events/Event.hpp"

/**
 * @struct MyCustomEvent
 * @brief Beschreibung des Events
 */
struct MyCustomEvent : public Event
{
    EVENT_TYPE_NAME("MyCustomEvent")
    
    // Event-Daten
    int value = 0;
    QString message;
};
```

### 3.2 EVENT_TYPE_NAME Makro

Das Makro definiert die `typeName()` Methode für Debugging:

```cpp
#define EVENT_TYPE_NAME(name) \
    static constexpr const char* staticTypeName() { return name; } \
    [[nodiscard]] const char* typeName() const override { return name; }
```

### 3.3 Event-Kategorien

| Kategorie | Beispiele | Datei |
|-----------|-----------|-------|
| **UI Events** | VisualizerChanged, PanelOpened | UIEvents.hpp |
| **Audio Events** | TrackChanged, PlaybackState, AudioData | AudioEvents.hpp |
| **System Events** | ConfigChanged, ThemeChanged | SystemEvents.hpp |

---

## 4. API

### 4.1 Subscribe

```cpp
// Einfache Subscription
auto id = eventBus.subscribe<AudioDataEvent>([](const AudioDataEvent& e) {
    updateVisualization(e.spectrum, e.size);
});

// Mit Priorität (niedrigere Werte = früher aufgerufen)
auto id = eventBus.subscribe<AudioDataEvent>(
    [](const AudioDataEvent& e) { /* High priority handler */ },
    -10  // Priority
);
```

### 4.2 Unsubscribe

```cpp
// Im Destruktor oder bei Deaktivierung
eventBus.unsubscribe(m_subscriptionId);

// Oder in RAII-Style
class MyPanel {
    IEventBus::SubscriberId m_subId;
    
    ~MyPanel() {
        m_eventBus.unsubscribe(m_subId);
    }
};
```

### 4.3 Publish (synchron)

```cpp
// Event erstellen und sofort publishen
AudioDataEvent event;
event.spectrum = spectrumData;
event.size = 1024;
eventBus.publish(event);

// Oder inline
eventBus.publish(AudioDataEvent{spectrumData, 1024});
```

### 4.4 Queue (asynchron, thread-safe)

```cpp
// Von einem Worker-Thread
void WorkerThread::processAudio() {
    AudioDataEvent event;
    event.spectrum = analyze();
    
    // Queue statt publish (thread-safe)
    m_eventBus.queue(event);
}

// Im Main-Thread (Render-Loop)
void Application::update() {
    m_eventBus.dispatchQueued();  // Dispatched alle queued Events
}
```

### 4.5 Utility-Methoden

```cpp
// Anzahl Subscriber für Event-Typ
size_t count = eventBus.subscriberCount<AudioDataEvent>();

// Alle Subscriptions löschen
eventBus.clear();
```

---

## 5. Verwendung

### 5.1 In Panels

```cpp
class SpectrumPanel : public PanelBase
{
public:
    SpectrumPanel(ServiceContainer& services)
        : PanelBase(services)
        , m_eventBus(services.resolve<IEventBus>())
    {}

protected:
    void onActivate() override
    {
        // Subscribe wenn Panel aktiviert wird
        m_audioSubId = m_eventBus.subscribe<AudioDataEvent>(
            [this](const AudioDataEvent& e) {
                updateSpectrum(e.spectrum);
            });
    }
    
    void onDeactivate() override
    {
        // Unsubscribe wenn Panel deaktiviert wird
        m_eventBus.unsubscribe(m_audioSubId);
    }
    
private:
    IEventBus& m_eventBus;
    IEventBus::SubscriberId m_audioSubId = 0;
};
```

### 5.2 In Services

```cpp
class AudioService
{
public:
    AudioService(IEventBus& eventBus)
        : m_eventBus(eventBus)
    {}
    
    void onTrackLoaded(const QString& path)
    {
        TrackChangedEvent event;
        event.track.filePath = path;
        m_eventBus.publish(event);
    }
    
    void update()
    {
        if (isPlaying()) {
            AudioDataEvent event;
            event.spectrum = getFFTData();
            m_eventBus.publish(event);
        }
    }
    
private:
    IEventBus& m_eventBus;
};
```

### 5.3 Cross-Thread Kommunikation

```cpp
// Worker-Thread: Audio-Analyse
class AudioAnalyzerThread : public QThread
{
    void run() override
    {
        while (m_running) {
            AudioDataEvent event;
            event.spectrum = analyze();
            
            // NICHT publish() verwenden (nicht thread-safe für Handler)!
            m_eventBus.queue(event);  // Thread-safe
            
            msleep(16);  // ~60 FPS
        }
    }
};

// Main-Thread: Update-Loop
void Application::update()
{
    // Dispatched Events im Main-Thread
    m_eventBus.dispatchQueued();
    
    // Jetzt sind alle Handler im Main-Thread aufgerufen worden
}
```

---

## 6. Existierende Events

### 6.1 UI Events (UIEvents.hpp)

```cpp
struct VisualizerChangedEvent : public Event
{
    EVENT_TYPE_NAME("VisualizerChangedEvent")
    QString visualizerId;
    QString previousId;
};

struct PanelStateChangedEvent : public Event
{
    EVENT_TYPE_NAME("PanelStateChangedEvent")
    QString panelId;
    bool isOpen;
};
```

### 6.2 Audio Events (AudioEvents.hpp)

```cpp
struct TrackChangedEvent : public Event
{
    EVENT_TYPE_NAME("TrackChangedEvent")
    TrackInfo track;
    int playlistIndex;
};

struct PlaybackStateEvent : public Event
{
    EVENT_TYPE_NAME("PlaybackStateEvent")
    PlaybackState state;
    PlaybackState previousState;
};

struct PlaybackPositionEvent : public Event
{
    EVENT_TYPE_NAME("PlaybackPositionEvent")
    int positionMs;
    int durationMs;
    float progress;
};

struct AudioDataEvent : public Event
{
    EVENT_TYPE_NAME("AudioDataEvent")
    std::vector<float> spectrum;
    std::vector<float> waveform;
    float levelLeft;
    float levelRight;
    bool beatDetected;
};
```

---

## 7. Best Practices

### 7.1 Subscription-Lifetime verwalten

```cpp
// ❌ Schlecht: Memory Leak / Dangling Reference
class Panel {
    void init() {
        eventBus.subscribe<Event>([this](auto& e) { ... });
        // ID nicht gespeichert → kann nicht unsubscriben!
    }
};

// ✅ Gut: RAII-Pattern
class Panel {
    IEventBus::SubscriberId m_subId = 0;
    
    void init() {
        m_subId = eventBus.subscribe<Event>([this](auto& e) { ... });
    }
    
    ~Panel() {
        if (m_subId) eventBus.unsubscribe(m_subId);
    }
};
```

### 7.2 Events nicht mutieren

```cpp
// ❌ Schlecht: Event modifizieren
eventBus.subscribe<AudioDataEvent>([](AudioDataEvent& e) {  // Non-const!
    e.spectrum.clear();  // Beeinflusst andere Subscriber!
});

// ✅ Gut: Const Reference
eventBus.subscribe<AudioDataEvent>([](const AudioDataEvent& e) {
    auto copy = e.spectrum;  // Kopie wenn nötig
});
```

### 7.3 Kleine Events bevorzugen

```cpp
// ❌ Schlecht: Große Daten im Event
struct HugeEvent : public Event {
    std::vector<float> data;  // 10MB Daten werden kopiert!
};

// ✅ Gut: Pointer/Reference auf existierende Daten
struct SmallEvent : public Event {
    const float* data;  // Pointer auf existierende Daten
    size_t size;
    // ACHTUNG: Daten müssen während dispatch() gültig bleiben!
};

// ✅ Besser: Shared Pointer für Ownership
struct SafeEvent : public Event {
    std::shared_ptr<std::vector<float>> data;
};
```

### 7.4 Prioritäten sparsam nutzen

```cpp
// Logging/Debugging zuerst
eventBus.subscribe<Event>(loggingHandler, -100);

// Normale Handler
eventBus.subscribe<Event>(normalHandler, 0);

// UI-Updates zuletzt
eventBus.subscribe<Event>(uiHandler, 100);
```

---

## 8. Thread-Sicherheit

### 8.1 Garantien

| Operation | Thread-Safe | Bemerkung |
|-----------|-------------|-----------|
| `subscribe()` | ✅ Ja | Mutex-geschützt |
| `unsubscribe()` | ✅ Ja | Mutex-geschützt |
| `publish()` | ⚠️ Teilweise | Dispatch ist synchron |
| `queue()` | ✅ Ja | Für Cross-Thread |
| `dispatchQueued()` | ⚠️ Main-Thread | Nur aus Main-Thread aufrufen |

### 8.2 Empfohlenes Pattern

```cpp
// Worker-Thread: Nur queue() verwenden
void WorkerThread::run() {
    eventBus.queue(event);  // ✅ Thread-safe
}

// Main-Thread: publish() und dispatchQueued()
void MainThread::update() {
    eventBus.publish(uiEvent);      // ✅ Synchron, Main-Thread
    eventBus.dispatchQueued();      // ✅ Dispatched queued Events
}
```

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-31** | **Initial: Publish/Subscribe, Queue, Priority** |
