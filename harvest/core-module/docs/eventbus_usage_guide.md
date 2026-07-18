# 📘 EventBus Usage Guide

Dieser Leitfaden zeigt, wie der **EventBus** in viz2025 eingesetzt wird.  
Ziel: typsichere Events zwischen Modulen (UI, Audio, Visuals) austauschen.

---

## 🏗️ Grundprinzip
- Jeder Event-Typ ist eine **struct**.
- Der EventBus unterscheidet Topics anhand des Typs.
- Subscriptions liefern ein `HandlerId` oder ein RAII-`SubscriberHandle`.
- **Weak-Abos** binden Lebenszeit an ein `shared_ptr`-Objekt.

---

## 📥 Events definieren
```cpp
// src/core/events/SettingsEvents.hpp
#pragma once
#include <string>

struct SettingsChanged {
    std::string key;
    std::string value;
};

struct AppStarted {
    // Marker-Event ohne Daten
};
```

---

## 📤 Publish
```cpp
#include "core/EventBus.hpp"

void onUserChangeSetting(viz::core::EventBus& bus) {
    SettingsChanged evt{"theme", "dark"};
    bus.publish(evt);
}
```

---

## 📥 Subscribe (normale Abos)
```cpp
auto id = bus.subscribe<SettingsChanged>([](const SettingsChanged& e) {
    std::cout << "Setting changed: " << e.key << "=" << e.value << "\n";
});

// Abmelden
bus.unsubscribe<SettingsChanged>(id);
```

---

## 🔒 RAII Subscription
```cpp
{
    auto handle = bus.subscribeScoped<SettingsChanged>([](const SettingsChanged& e){
        std::cout << "Scoped: " << e.key << "\n";
    });

    bus.publish(SettingsChanged{"volume", "75"});
    // handle zerstört → auto unsubscribe
}
```

---

## 🪝 Weak Subscription
```cpp
struct Panel : std::enable_shared_from_this<Panel> {
    void connect(viz::core::EventBus& bus) {
        // bindet Abo an Lebenszeit des Panels
        auto self = shared_from_this();
        m_handle = bus.subscribeScopedWeak<SettingsChanged>(self, [this](const SettingsChanged& e){
            this->apply(e);
        });
    }
    void apply(const SettingsChanged& e) {
        std::cout << "Apply in Panel: " << e.key << "=" << e.value << "\n";
    }
    viz::core::EventBus::SubscriberHandle m_handle;
};
```

---

## 🧵 Thread-Sicherheit
- `publish<T>` erstellt einen **Snapshot** der Subscriber-Liste.
- Parallel `subscribe/unsubscribe` ist sicher.
- Callbacks können selbst wieder `publish<T>` aufrufen (Reentranz ok).

---

## 🛠️ Integration mit ServiceContainer
```cpp
// Registrierung (z. B. im Application-Bootstrap)
di.addSingletonFactory<viz::core::EventBus>([](auto&){
    return std::make_shared<viz::core::EventBus>();
}, "EventBus");

// Nutzung
auto& bus = *di.get<viz::core::EventBus>();
bus.publish(AppStarted{});
```

---

## ⚠️ Hinweise
- **Keine String-Keys** → immer eigene Event-Structs.
- **Keine Ref-Captures auf temporäre Objekte** in Callbacks.
- **Unsubscribe während publish()**: Callback kann noch einmal feuern (Snapshot-Semantik).

---

## 🔮 Erweiterungen (optional)
- Subscription-Prioritäten
- Async Dispatch (`post<T>()`)
- Sticky Events
- Telemetrie (Anzahl Subscribers/Events loggen)

---

✅ Mit diesem Guide kannst du den EventBus konsistent im Projekt einsetzen.

