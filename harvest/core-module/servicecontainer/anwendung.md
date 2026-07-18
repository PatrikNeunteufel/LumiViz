alles klar — hier ist eine **praxisorientierte Anleitung** zur Verwendung des `viz::core::ServiceContainer` in all seinen Facetten. Sie deckt Konzepte, Registrierungs-Patterns, Scopes, Fehlerbilder, Threading, Tests, Integration und Best Practices ab. Die Codebeispiele sind knapp gehalten; Kommentare in den Schnipseln sind auf Englisch (wie bei euch im Code).

---

# Überblick

Der `ServiceContainer` ist euer zentrales **Dependency-Injection (DI)**-Werkzeug. Er verwaltet:

* **Lifetimes**: `Singleton`, `Scoped`, `Transient`
* **Scopes (RAII)**: deterministische Lebensdauer für Scoped-Objekte
* **Factories** mit `IServiceResolver&` zur Constructor-Injection
* **Thread-Safety**: registrieren/auflösen ist mutex-geschützt
* **Zyklus-Erkennung**: Schutz vor A→B→A
* **Eager-Init**: optionales Vorab-Bauen von Singletons

---

# Kernkonzepte

1. **Registrierung** (Root-Container)

* `addSingleton(instance)` oder `addSingletonFactory(factory, ..., eager)`
* `addScoped(factory)`
* `addTransient(factory)`

2. **Auflösung**

* Root: `di.get<T>()` / `di.tryGet<T>()`
* Pro Kontext (Frame/Panel/Dialog): `auto scope = di.createScope(); scope.get<T>()`

3. **Factory-Resolver**

* Jede Factory erhält `IServiceResolver&` und kann darüber **Abhängigkeiten** auflösen:

  ```cpp
  di.addScoped<MyService>([](viz::core::IServiceResolver& r){
      return std::make_shared<MyService>(r.get<ILogger>(), r.get<ISettings>());
  });
  ```

4. **Zyklus-Erkennung**

* Bei Zyklen (A hängt von B ab, B von A) wird `ServiceCycleError` geworfen.

---

# Lifetimes im Detail

| Lifetime  | Verhalten                                                                                | Typische Kandidaten                                                |
| --------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| Singleton | Eine Instanz pro Prozess (im Root-Container gecached). Lazy oder Eager erstellbar.       | `EventBus`, `CommandBus`, `Logger`, `ConfigService`, `Settings`    |
| Scoped    | Eine Instanz pro Scope (z. B. pro Frame, Dialog, Panel). RAII: Freigabe beim Scope-Ende. | `PanelManager`, `DialogManager`, `AudioFrameContext`, Buffer-Pools |
| Transient | Jede Auflösung erzeugt eine **neue** Instanz.                                            | Parser, temporäre Builder/Filter, Scratch-Buffers                  |

---

# Registrierungsmuster (Root)

```cpp
using viz::core::ServiceContainer;
using viz::core::IServiceResolver;

// 1) Ready-made singleton instance
di.addSingleton<ILogger>(std::make_shared<Logger>(), "Logger", /*eager*/true);

// 2) Singleton via factory (lazy or eager)
di.addSingletonFactory<ISettings>(
    [](IServiceResolver&){ return std::make_shared<Settings>("config.json"); },
    "Settings", /*eager*/true
);

// 3) Scoped service (one per scope)
di.addScoped<PanelManager>([](IServiceResolver& r){
    return std::make_shared<PanelManager>(r.get<EventBus>(), r.get<ILogger>());
}, "PanelManager");

// 4) Transient service (new instance each resolution)
di.addTransient<ScratchBuffer>([](IServiceResolver&){
    return std::make_shared<ScratchBuffer>(/*size*/ 1<<16);
}, "ScratchBuffer");
```

**Hinweise**

* **Interfaces bevorzugen** (z. B. `ILogger` statt `Logger`), um Implementierungen leichter zu tauschen.
* Debug-Namen sind optional, aber für Logs nützlich.
* Für teure Singletons (`AudioEngine`, `GPUBackend`) ggf. `eager=true` und anschließend `di.buildSingletons()` im App-Start.

---

# Auflösung & Scoping

```cpp
// Root resolution (Singleton/Transient)
auto bus   = di.get<EventBus>();
auto optUi = di.tryGet<OptionalUiService>(); // nullptr if not registered

// Per-frame scope (RAII)
{
    auto frameScope = di.createScope();

    // Scoped: cached within this scope
    auto pm = frameScope.get<PanelManager>();
    auto dm = frameScope.get<DialogManager>();

    // Transient: new instance every time
    auto tmp1 = frameScope.get<ScratchBuffer>();
    auto tmp2 = frameScope.get<ScratchBuffer>(); // tmp2 != tmp1

} // frameScope destroyed -> scoped instances released deterministically
```

**Empfehlung**

* **Frame-Scope** (pro Render-Frame) für per-Frame-Kontexte/Buffer.
* **Dialog-/Panel-Scope** beim Öffnen/Schließen für UI-bezogene Manager.

---

# Eager-Initialisierung

```cpp
// Mark singletons as eager at registration (eager=true) ...
// ... then build them in application startup:
di.buildSingletons();
// Errors during construction surface here, not at first usage.
```

**Wann sinnvoll?**

* Teure Init-Prozesse (AudioDevice öffnen, Shader-Kompilierung, Datei-Load)
* Ihr wollt Fehler **früh** sehen (Fail fast).

---

# Ersetzen, Entfernen, Prüfen

```cpp
// Replace existing registration (e.g. in tests, or hot-swap)
di.replaceWithSingleton<ILogger>(std::make_shared<NullLogger>(), "NullLogger");

// Remove a registration
bool removed = di.remove<ISettings>();

// Check if a type is registered
if (!di.isRegistered<EventBus>()) {
    // register or throw
}
```

---

# Fehlerbehandlung & Debug

* **`ServiceError`**: Unregistrierter Typ, fehlende Factory/Instanz, Factory liefert `nullptr`.
* **`ServiceCycleError`**: Zyklische Abhängigkeit entdeckt.
* **Diagnose-Tipps**

  * Debug-Namen pflegen (`debugName` bei Registrierung)
  * Fail fast: `buildSingletons()`
  * Bei `tryGet<T>()` bewusstes optionales Verhalten implementieren

**Beispiele**

```cpp
try {
    auto x = di.get<ExpensiveService>();
} catch (const viz::core::ServiceCycleError& e) {
    // Log cycle info, inspect constructors
} catch (const viz::core::ServiceError& e) {
    // Missing registration or null from factory
}
```

---

# Threading

* Registrierung & Singleton-Caching sind durch **Mutex** geschützt.
* **Factories** werden **außerhalb** des Locks aufgerufen → re-entrant und deadlock-fest.
* Der Container ist für **parallele Auflösungen** nutzbar, solange eure **Factories** selbst **thread-safe** sind (z. B. keine globalen ungeschützten States anlegen).

**Pattern**

* Singletons sind oft **thread-safe** (immutable config, synchronized access).
* Scoped/Transient werden lokal pro Thread/Scope genutzt.

---

# Architektur-Integration (Application, Busse, Manager)

**Application-Startup (Beispiel)**

```cpp
void Application::init() {
    // Singletons
    di_.addSingletonFactory<EventBus>([](IServiceResolver&){ return std::make_shared<EventBus>(); }, "EventBus", true);
    di_.addSingletonFactory<CommandBus>([](IServiceResolver& r){
        return std::make_shared<CommandBus>(r.get<EventBus>());
    }, "CommandBus", true);
    di_.addSingletonFactory<ILogger>([](IServiceResolver&){ return std::make_shared<Logger>(); }, "Logger", true);
    di_.addSingletonFactory<ISettings>([](IServiceResolver&){ return std::make_shared<Settings>("config.json"); }, "Settings");

    // Scoped
    di_.addScoped<PanelManager>([](IServiceResolver& r){
        return std::make_shared<PanelManager>(r.get<EventBus>(), r.get<ILogger>());
    }, "PanelManager");
    di_.addScoped<DialogManager>([](IServiceResolver& r){
        return std::make_shared<DialogManager>(r.get<EventBus>());
    }, "DialogManager");

    // Transient
    di_.addTransient<ScratchBuffer>([](IServiceResolver&){
        return std::make_shared<ScratchBuffer>(1<<16);
    }, "ScratchBuffer");

    // Optional: early construction
    di_.buildSingletons();
}

void Application::frame() {
    auto scope = di_.createScope();
    auto& ui = *scope.get<PanelManager>();
    (void)ui;
    // ... draw/update using scoped managers
}
```

---

# Tests (Unit/Integration)

**Was testen?**

* Singleton-Identität, Scoped-Isolation, Transient-Neuinstanz
* Factory-Resolver (Abhängigkeiten werden korrekt aufgelöst)
* Fehlerfälle (unregistrierter Typ, null-Factory)
* Zyklus-Erkennung

**Mini-Skizze**

```cpp
// pseudo with assert; use gtest/doctest/Catch2 in practice
viz::core::ServiceContainer di;

di.addSingletonFactory<int>([](auto&){ return std::make_shared<int>(7); });
di.addScoped<std::string>([](auto&){ return std::make_shared<std::string>("X"); });
di.addTransient<double>([](auto&){ return std::make_shared<double>(3.14); });

auto s1 = di.get<int>();
auto s2 = di.get<int>();
assert(s1 == s2); // singleton identity

auto sc1 = di.createScope();
auto a1  = sc1.get<std::string>();
auto a2  = sc1.get<std::string>();
assert(a1 == a2); // same scope cache

auto sc2 = di.createScope();
auto b1  = sc2.get<std::string>();
assert(a1 != b1); // different scope

auto t1 = di.get<double>();
auto t2 = di.get<double>();
assert(t1 != t2); // transient fresh instance
```

**Zyklus-Test**

```cpp
struct A { explicit A(std::shared_ptr<struct B>){} };
struct B { explicit B(std::shared_ptr<A>){} };
di.addSingletonFactory<A>([](auto& r){ return std::make_shared<A>(r.get<B>()); });
di.addSingletonFactory<B>([](auto& r){ return std::make_shared<B>(r.get<A>()); });
EXPECT_THROW(di.get<A>(), viz::core::ServiceCycleError);
```

---

# Migrationsleitfaden (von „globalen Pointern“ zu DI)

* **Direkte Member** im Container (z. B. `Logger* logger_`) **entfernen**.
* Stattdessen **registrieren** + **auflösen**:

  * Registrierung beim Start (Application::init)
  * Auflösung wo gebraucht (Constructor-Injection via Factory)
* **Konstruktoren** eurer Manager/Controller auf **Interfaces** ausrichten; vermeidet „Pull“ über globale Singletons.
* Für alte Call-Sites: adapterweise `auto log = di.get<ILogger>();` injizieren.

---

# Best Practices & Stolperfallen

* **Interfaces statt konkreter Klassen** registrieren → Austauschbarkeit/Tests.
* **Scoped bewusst wählen**: Frame-/Dialog-Kontexte sind ideale Scopes.
* **Eager sparsam**: Nur für teure/fehlersensitive Singletons.
* **Keine Arbeit in Konstruktoren**, die wieder DI-Auflösung starten, ohne Not → Zyklen vermeiden.
* **Makros** `VIZ_LIKELY/UNLIKELY` nur an Hotspots (kein Micro-Optimierungs-Overkill).
* **tryGet** bewusst nutzen, wo Optionalität Semantik hat (z. B. optionales Telemetrie-Backend).

---

# Optional: Erweiterungsmuster

* **Named Services/Qualifiers**: Wenn du mehrere Implementierungen desselben Interfaces brauchst (z. B. `ILogger` → „File“, „Console“), kann man einen zusätzlichen „Key“ einführen (`std::pair<std::type_index, std::string>` als Map-Key). Sag Bescheid, wenn du das willst; ich liefere dir ein schlankes API-Add-on.
* **Profiling Hooks**: Dauer pro Auflösung messen, slow-constructors loggen.
* **Health-Check**: `buildSingletons()` + „ping“ der Dienste am Start.

---

Wenn du möchtest, passe ich dir im nächsten Schritt die **Application-Registrierungen** für eure bestehenden Klassen im Repo konkret an (inkl. `EventBus`, `CommandBus`, `UIController`, `Settings/Config`, `AudioEngine`), oder ich liefere dir ein **Testmodul** (gtest/doctest) mit sofort ausführbaren Tests für den Container.
