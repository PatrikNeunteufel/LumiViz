# ServiceContainer – Cheatsheet

Kurzrezepte für häufige Integrationen & Patterns.

---

## 1. ServiceContainer im Application-Bootstrap

```cpp
viz::core::ServiceContainer di;

// zentrale Services registrieren
di.addSingletonFactory<EventBus>([](auto&){ return std::make_shared<EventBus>(); }, "EventBus");
di.addSingletonFactory<CommandBus>([](auto&){ return std::make_shared<CommandBus>(); }, "CommandBus");

// Application starten
Application app{di};
app.run();
```

---

## 2. EventBus via DI nutzen

```cpp
struct MyEvent { int value; };

// Registration (z. B. im ApplicationController)
auto& bus = di.get<EventBus>();

bus.subscribe<MyEvent>([](const MyEvent& e){
    std::cout << "Got event with value=" << e.value << "\n";
});

// Publish
di.get<EventBus>().publish(MyEvent{123});
```

---

## 3. CommandBus via DI nutzen

```cpp
struct OpenFileCommand : viz::core::ICommand {
    std::string path;
    void execute() override { /* open file */ }
    void undo() override { /* close file */ }
};

auto& cmdBus = di.get<CommandBus>();

cmdBus.dispatch(std::make_unique<OpenFileCommand>("song.mp3"));
```

---

## 4. ImGui + ServiceContainer (z. B. UIController)

```cpp
struct UIController {
    void draw() {
        if (ImGui::Button("Send Event")) {
            bus_.publish(MyEvent{42});
        }
    }
    EventBus& bus_;
};

di.addSingletonFactory<UIController>([&](auto& r){
    return std::make_shared<UIController>(r.get<EventBus>());
}, "UIController");
```

---

## 5. BASS / AudioEngine via DI

```cpp
struct AudioEngine {
    AudioEngine() { BASS_Init(-1, 44100, 0, 0, nullptr); }
    ~AudioEngine() { BASS_Free(); }
};

di.addSingletonFactory<AudioEngine>([](auto&){ return std::make_shared<AudioEngine>(); }, "Audio");
```

---

## 6. LuaEngine via DI

```cpp
struct LuaEngine {
    LuaEngine(){ luaL_newstate(); /* ... */ }
};

di.addSingletonFactory<LuaEngine>([](auto&){ return std::make_shared<LuaEngine>(); }, "Lua");
```

---

## 7. Manager/Controller Pattern

```cpp
struct PanelManager {
    void open(const std::string& name) {/*...*/}
};

di.addSingletonFactory<PanelManager>([](auto&){ return std::make_shared<PanelManager>(); }, "PanelMgr");

// Use in UIController
auto& mgr = di.get<PanelManager>();
mgr.open("Player");
```

---

## 8. Tipps
- Für **Events/Commands** den Container als zentrales Register verwenden.
- Manager/Controller als **Singleton** registrieren.
- Panels/Dialoge eher **Scoped**, wenn pro Session/Frame unterschiedlich.
- Audio/Rendering **Singleton** mit RAII-Init/Free.
- Lua **Singleton** (ein State) oder **Scoped** (pro Script-Context).

---

## 9. Quick Reference (erweitert)

```cpp
// Register services
di.addSingletonFactory<T>(factory, name, eager);
di.replaceWithSingleton<T>(instance, name);
di.addScoped<T>(factory, name);
di.addTransient<T>(factory, name);

// Resolve
get<T>();
tryGet<T>();

// Scopes
createScope();
scopedCount();

// Manage
isRegistered<T>();
remove<T>();
buildSingletons();
```

---

Diese Kurzrezepte helfen, den ServiceContainer nahtlos mit EventBus, CommandBus, ImGui, BASS und Lua zu verbinden.

