# 🧾 EventBus Cheatsheet

Kurzreferenz für den typsicheren EventBus in viz2025.

---

## 💡 Essentials
- **Topic == Event-Typ** (`struct`)
- **Subscribe** → `HandlerId` **oder** RAII `SubscriberHandle`
- **Weak-Abo** → Lebenszeit an `shared_ptr` binden
- **Thread-sicher** → Snapshot-Dispatch

```cpp
#include "core/EventBus.hpp"
using viz::core::EventBus;
```

---

## ✅ Publish
```cpp
struct SettingsChanged { std::string key; std::string value; };
EventBus bus;

bus.publish(SettingsChanged{"ui.theme", "dark"});
```

---

## ✅ Subscribe (ID-basiert)
```cpp
auto id = bus.subscribe<SettingsChanged>([](const SettingsChanged& e){
    // handle event
});
// later
bus.unsubscribe<SettingsChanged>(id);
```

---

## ✅ Scoped (RAII)
```cpp
{
    auto h = bus.subscribeScoped<SettingsChanged>([](const SettingsChanged& e){ /* ... */ });
    bus.publish(SettingsChanged{"volume", "75"});
}
// h out-of-scope → auto-unsubscribe
```

---

## ✅ Weak-Abos (Owner-gebunden)
```cpp
struct Panel : std::enable_shared_from_this<Panel> {
    EventBus::SubscriberHandle m_sub;
    void connect(EventBus& bus) {
        auto self = shared_from_this();
        m_sub = bus.subscribeScopedWeak<SettingsChanged>(self, [this](const SettingsChanged& e){ apply(e); });
    }
    void apply(const SettingsChanged& e) {/*...*/}
};
```
> Tipp: `subscribeWeak(shared_ptr<Owner>, cb)` gibt es ebenfalls (ID-basiert).

---

## 🔍 Introspection
```cpp
u64 n = bus.subscriberCount<SettingsChanged>();
bus.clearTopic<SettingsChanged>();
bus.clearAll();
```

---

## 🧱 Event-Typen (Beispiele)
```cpp
struct AppStarted { int argc{}; };
struct ValueChanged { int before{}, after{}; };
struct AudioFrame { float rms{}, peak{}; };
```

---

## 🧩 DI (ServiceContainer)
```cpp
// Registration once
// di.addSingletonFactory<EventBus>([](auto&){ return std::make_shared<EventBus>(); }, "EventBus");

// Usage
// auto& bus = *di.get<EventBus>();
```

---

## 🧪 Tests (Catch2)
- Executable: `eventbus_tests`
- Selektiv ausführen: `-DVIZ_TEST_ONLY=EVENTBUS`
- CTest: `ctest -C RelWithDebInfo -R eventbus_tests --output-on-failure`

---

## ⚠️ Do / Don’t
**Do**
- Kleine, fokussierte Event-Structs verwenden
- Panels/Controller mit `subscribeScopedWeak` anbinden
- Keine teuren Arbeiten im Callback (kurz halten)

**Don’t**
- Keine String-Topics
- Keine Captures auf temporäre/kurzlebige Objekte
- Kein `unsubscribe` innerhalb desselben Callbacks erzwingen (Snapshot kann einmal noch feuern)

---

## 🛠️ Troubleshooting
- **Callbacks feuern nicht** → Prüfe, ob Weak-Owner noch lebt.
- **MSVC/RTTI-Issues** → EventBus nutzt adressbasierten Topic-Key (schon gefixt).
- **Doppelte Calls** → Snapshot-Semantik: Unsubscribe kann im laufenden Publish noch einmal auftreten.

---

## 🚀 Performance-Tipps
- Leichte Event-Payloads (POD/kleine Strings)
- Kritische Pfade: Early-out in Callback, ggf. `VIZ_LIKELY`
- Viele Topics? Event-Typen konsolidieren oder priorisieren

---

## 🔮 Erweiterbar (optional)
- Prioritäten pro Subscription
- Async `post<T>()` + Worker
- Sticky Events
- Telemetrie (Zähler/Latenzen)

---

## 📎 Muster
```cpp
// Common pattern in UI module
struct ThemeChanged { std::string name; };

class ThemePanel : public std::enable_shared_from_this<ThemePanel> {
    viz::core::EventBus::SubscriberHandle m_sub;
public:
    void attach(EventBus& bus) {
        auto self = shared_from_this();
        m_sub = bus.subscribeScopedWeak<ThemeChanged>(self, [this](const ThemeChanged& e){ updateTheme(e.name); });
    }
    void updateTheme(const std::string& name);
};
```

