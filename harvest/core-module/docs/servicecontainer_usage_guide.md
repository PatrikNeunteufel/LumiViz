# ServiceContainer – ausführliche Anleitung

Der `viz::core::ServiceContainer` ist die zentrale Komponente für **Dependency Injection (DI)** im Projekt. Er verwaltet die Lebenszyklen von Services und sorgt für konsistente Auflösung von Abhängigkeiten.

---

## Grundprinzip
- **Registrierung** von Services: definiert, *wie* Instanzen erzeugt werden.
- **Auflösung** (`get<T>()`): liefert eine Instanz, gemäß dem registrierten Lifetime.
- **Lifetimes**:
  - `Singleton`: eine Instanz für die gesamte Anwendung
  - `Scoped`: eine Instanz pro Scope (z. B. Request/Frame)
  - `Transient`: jedes Mal eine neue Instanz

---

## Registrierungsmethoden

### Singleton
```cpp
viz::core::ServiceContainer di;

// Factory-basiert
struct Config { int value; };
di.addSingletonFactory<Config>([](auto&){
    return std::make_shared<Config>(42);
}, "Config");

// Feste Instanz
auto cfg = std::make_shared<Config>(7);
di.replaceWithSingleton<Config>(cfg, "ConfigInstance");
```

### Scoped
```cpp
struct Session { std::string id; };

di.addScoped<Session>([](auto&){
    return std::make_shared<Session>("abc");
}, "Session");
```

### Transient
```cpp
struct Point { int x, y; };

di.addTransient<Point>([](auto&){
    return std::make_shared<Point>(1,2);
}, "Point");
```

---

## Auflösung
```cpp
// throws if not registered
auto cfg = di.get<Config>();

// safe: returns nullptr if not registered
auto maybe = di.tryGet<Point>();
```

---

## Scopes
```cpp
// Create a new scope
auto scope = di.createScope();

// Scoped type: cached within the scope
auto s1 = scope.get<Session>();
auto s2 = scope.get<Session>();
assert(s1 == s2);

// New scope → new instance
auto scope2 = di.createScope();
auto s3 = scope2.get<Session>();
assert(s1 != s3);
```

---

## Eager Singletons
```cpp
struct Logger { Logger(){ std::cout << "init logger\n"; } };

di.addSingletonFactory<Logger>([](auto&){ return std::make_shared<Logger>(); }, "Logger", true);

// Initialize all eager singletons immediately
di.buildSingletons();
```

---

## Entfernen & Ersetzen
```cpp
// Remove
bool removed = di.remove<Config>();

// Replace with new singleton instance
di.replaceWithSingleton<Config>(std::make_shared<Config>(99), "NewConfig");
```

---

## Dependency Injection in Factories
```cpp
struct Engine { Engine(std::shared_ptr<Config> c){ /* use c */ } };

di.addSingletonFactory<Engine>([](auto& r){
    return std::make_shared<Engine>(r.get<Config>());
}, "Engine");
```

---

## Fehlerfälle & Ausnahmen
- `ServiceError`: allgemeine Fehler (nicht registriert, Factory null, …)
- `ServiceCycleError`: zyklische Abhängigkeit erkannt

Beispiel:
```cpp
CHECK_THROW(di.get<UnregisteredType>());
```

---

## Thread-Sicherheit
- **Singletons**: abgesichert durch `std::call_once` → exakt eine Konstruktion, auch bei parallelen `get<T>()`.
- **Transients**: jeder Thread erhält eine eigene Instanz.
- **Scopes**: nicht threadsicher per se → pro Thread eigenen Scope erzeugen.

---

## Zusammenfassung
Der `ServiceContainer` bietet:
- saubere Abhängigkeitsverwaltung
- flexible Lifetimes (Singleton, Scoped, Transient)
- sichere Cycle Detection
- Integration in Tests
- Thread-Sicherheit für Singletons

Damit eignet er sich ideal als Fundament für komplexe Subsysteme (EventBus, Manager, Controller, Agents).

