# ServiceContainer – Cheatsheet

Kurz & knackig: wichtigste Befehle, Patterns und Stolpersteine.

---

## Lifetimes & Registrierung

```cpp
// Singleton (Factory)
di.addSingletonFactory<Type>([](auto& r){
    return std::make_shared<Type>(/* deps via r.get<...>() */);
}, "Type", /*eager=*/false);

// Singleton (fertige Instanz)
di.replaceWithSingleton<Type>(std::make_shared<Type>(/*...*/), "Type");

// Scoped
di.addScoped<Type>([](auto&){ return std::make_shared<Type>(); }, "Type");

// Transient
di.addTransient<Type>([](auto&){ return std::make_shared<Type>(); }, "Type");
```

**Tipps**
- **Singleton**: exakt **eine** Instanz (thread-sicher via `std::call_once`).
- **Scoped**: eine Instanz **pro Scope**; Root gibt **keinen** Scope-Cache aus.
- **Transient**: **immer neu**.

---

## Auflösung

```cpp
// Throws if unregistered
auto obj = di.get<T>();

// Safe: nullptr if unregistered
auto ptr = di.tryGet<T>();
```

**Injection in Factory**
```cpp
struct Engine { explicit Engine(std::shared_ptr<Config> c) {/*...*/} };
di.addSingletonFactory<Engine>([](auto& r){
    return std::make_shared<Engine>(r.get<Config>());
}, "Engine");
```

---

## Scopes

```cpp
auto scope = di.createScope();
// Scoped cached within this scope
auto a = scope.get<MyScoped>();
auto b = scope.get<MyScoped>(); // a == b

auto scope2 = di.createScope();
auto c = scope2.get<MyScoped>(); // c != a
```

---

## Eager Singletons

```cpp
// Mark as eager during registration (3rd param = true)
di.addSingletonFactory<Logger>([](auto&){ return std::make_shared<Logger>(); },
                               "Logger", /*eager=*/true);
// Build all eager singletons now
di.buildSingletons();
```

---

## Verwaltung

```cpp
bool removed = di.remove<T>();                    // deregister
bool reg     = di.isRegistered<T>();              // check

di.replaceWithSingleton<T>(std::make_shared<T>(), "T"); // swap instance
```

---

## Fehler & Diagnosen

- `ServiceError` → unregistriert, fehlende Factory, Factory liefert `nullptr`.
- `ServiceCycleError` → zyklische Abhängigkeit erkannt.
- **Debug-Hinweise**: aussagekräftige `debugName`-Strings setzen (3. Arg der Registrierung), erleichtert Logs.

```cpp
// Example: expect failure
CHECK_THROW(di.get<UnknownType>());
```

---

## Concurrency – Best Practices

- Singletons sind **thread-sicher** beim ersten Aufbau (`std::call_once`).
- Transients sind unabhängig – parallel ok.
- Scopes **nicht** zwischen Threads teilen; pro Thread eigenen Scope verwenden.

---

## Häufige Patterns

**Config → Engine (Singleton über Scoped/Transient)**
```cpp
struct Config { int v; };
struct Engine { explicit Engine(std::shared_ptr<Config> c) : v(c->v) {} int v; };

di.addSingletonFactory<Config>([](auto&){ return std::make_shared<Config>(42); }, "Cfg");
di.addSingletonFactory<Engine>([](auto& r){ return std::make_shared<Engine>(r.get<Config>()); }, "Engine");
```

**Scoped Context**
```cpp
struct Ctx { /* per-frame data */ };

di.addScoped<Ctx>([](auto&){ return std::make_shared<Ctx>(); }, "Ctx");
auto s = di.createScope();
auto ctx = s.get<Ctx>();
```

**Transient Tool**
```cpp
struct Tool { /* stateless work */ };

di.addTransient<Tool>([](auto&){ return std::make_shared<Tool>(); }, "Tool");
```

---

## Do / Don’t

**Do**
- Dependencies **immer** über `r.get<>()` in Factories beziehen.
- Für Singletons sinnvolle **eager**-Markierung setzen, wenn Start-up-Zeitplan das verlangt.
- Bei Fehlermeldungen **Typnamen** + `debugName` verwenden.

**Don’t**
- Keine **seiteneffektbehafteten** Factories (doppelte Aufrufe vermeiden, aber möglich bei Fehler/Retry).
- Kein Teilen von **ServiceScope** über Threads.

---

## Kurze CMake/Build Erinnerungen

```bat
:: Konfigurieren (Tests an)
cmake -S . -B build -G "Visual Studio 17 2022" -DVIZ_ENABLE_TESTS=ON
:: Bauen
cmake --build build --config RelWithDebInfo --target servicecontainer_tests
:: Ausführen
ctest --test-dir build -C RelWithDebInfo --output-on-failure -R servicecontainer_tests
```

Optional `BUILD_TESTING=ON` verwenden (Root-CMake unterstützt OR-Logik mit `VIZ_ENABLE_TESTS`).

---

## Troubleshooting

- **Crash 0xC0000409**: Prüfe Guard-Logik (push→active=true; pop nur wenn top).
- **Deadlock / Hänger**: Keine langen Operationen innerhalb Container-Mutex halten; Factories **außerhalb** Lock ausführen.
- **Mehrfachkonstruktion Singleton**: Stelle sicher, dass `std::call_once` pro Registrierung verwendet wird.
- **CTest findet Exe nicht (VS)**: `add_test(NAME ... COMMAND $<TARGET_FILE:...>)` nutzen.

---

## Mini-API (Quick Reference)

```cpp
// Register
addSingletonFactory<T>(Factory, debugName, eager=false)
replaceWithSingleton<T>(shared_ptr<T>, debugName)
addScoped<T>(Factory, debugName)
addTransient<T>(Factory, debugName)

// Resolve
get<T>()              // throws on missing
tryGet<T>() -> T*?    // nullptr on missing

// Scope
auto s = createScope();
s.get<T>(); s.tryGet<T>();
s.scopedCount();

// Admin
isRegistered<T>();
remove<T>();
buildSingletons();
```

---

*Kommentare in Code: Englisch. Nutzung im Projekt: `namespace viz::core`.*

