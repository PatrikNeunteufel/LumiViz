# Tiefenanalyse **PlugInSystem** – Schwachstellen & konkrete Verbesserungen

> Stand der Analyse: Archiv **PlugInSystem.zip** (Projektstruktur: Application, Core/Bus, Logging, Settings, UI-Manager, Visualizer, Externals, CMake)

---

## 1) Kurzfazit

- **Architektur:** gut modularisiert (Application, Services, Manager, Registries). Erweiterbar und verständlich.
- **Haupt-Risiken:** Thread-Safety (Event/Command/Logger), Persistenz/Settings (Schema & Migration), Service-Locator/Hidden Dependencies, Speicherlebenszeit in Registries/Managern, plattformnahe UI/Render-Kopplung, CMake-Vereinheitlichung.
- **Quick Wins:** robustere IDs & Mutex-Strategie im EventBus, LogLevel + thread-sicherer Logger, Settings→JSON mit Schema, `unique_ptr`/`weak_ptr` in Registries, Modern-CMake-Konsequenz.

---

## 2) Prüfmatrix (Überblick)

| Bereich             | Befund                                | Risiko               | Priorität | Maßnahme (kurz)                                              |
| ------------------- | ------------------------------------- | -------------------- | --------- | ------------------------------------------------------------ |
| ServiceContainer    | Bequemer Locator, fördert Hidden Deps | Testbarkeit sinkt    | **Hoch**  | Schlank halten, DI-Punkte definieren, Zugriffe dokumentieren |
| EventBus            | Typ-sicher, aber ID/Threading fragil  | Race/Leaks           | **Hoch**  | `uint64_t` IDs, feingranulare Locks, Safe-Unsubscribe        |
| CommandBus          | ok, Overhead möglich                  | unnötige Komplexität | Mittel    | Richtlinien: wann benutzen, wann nicht                       |
| Logger              | minimal, kein Level/Thread-Schutz     | Datenrennen, Spam    | **Hoch**  | LogLevel, Queue, File/Console-Targets                        |
| Settings            | rudimentär, kein Schema/Migration     | korrupt/inkonsistent | **Hoch**  | JSON+Schema, Defaults, Versionierung                         |
| UI-Manager/Registry | Wachstum zu „God Objects“             | Kopplung/Leaks       | Mittel    | Ownership klar, `unique_ptr`, Lebenszyklus dokumentieren     |
| RenderWindow        | Plattformkopplung                     | Portabilität         | Mittel    | Backend-Interface einziehen                                  |
| Visualizer          | Modular, aber Lifecycle & Threads     | Leaks/Races          | Mittel    | Factory/Owner, klare `initialize/shutdown`                   |
| CMake               | teils modern, teils uneinheitlich     | Build-Drift          | Mittel    | `target_*` überall, Features, Presets, Install/Export        |
| Externals/Lizenzen  | BASS proprietär                       | Lizenzrisiko         | Mittel    | LICENSE-Notizen, Distribution prüfen                         |

---

## 3) Application & ServiceContainer

**Befund:** Zentraler Service-Locator ist bequem, kann aber Abhängigkeiten verbergen und Tests erschweren.

**Verbesserungsvorschläge**

1. **Verwendungshygiene & Dokumentation**
   - *Warum:* Entwickler wissen sonst nicht, wo Abhängigkeiten herkommen.
   - *Wie:* README-Abschnitt: „Welche Services gibt es? Wer darf sie lesen/schreiben?“ + kurze Codebeispiele.
2. **Injection-Punkte definieren**
   - *Warum:* Tests werden einfacher, wenn Konstruktoren explizit Services annehmen können.
   - *Wie:* Klassen bevorzugt via Konstruktor-Parameter mit Dependencies versorgen; ServiceContainer nur als Fallback.
3. **Lebenszyklus klarstellen**
   - *Warum:* Reihenfolge von `initialize()`/`shutdown()` verhindert Dangling-Refs.
   - *Wie:* Start-/Stop-Sequenz zentral definieren und dokumentieren.

---

## 4) Core: EventBus

**Befund:** Typ-sicher per `std::type_index`. Risiken: ID-Kollisionen (32-bit), Threading, Unsubscribe-Leaks.

**Verbesserungsvorschläge**

1. **Stabile Handler-IDs**
   - *Warum:* Viele Subscriptions → `size_t` kann auf 32-bit knapp werden.
   - *Wie:* `std::atomic<uint64_t> s_nextId{1}; using HandlerId = uint64_t;`
2. **Feingranulare Synchronisation**
   - *Warum:* `publish()` sollte nicht globale Locks halten → Latenz & Deadlock-Risiko.
   - *Wie:* Beim Publish *Kopie* der Handlerliste unter kleinem Lock erstellen und **Lock vor Callback** freigeben.

```cpp
// Idee (Ausschnitt):
std::vector<Callback> local;
{
    std::scoped_lock lk(mutex_);
    local = handlers_[type]; // kopieren
}
for (auto &cb : local) cb(evt); // ohne Lock aufrufen
```

3. **Safe-Unsubscribe**
   - *Warum:* Abmeldung während Dispatch.
   - *Wie:* Entweder (a) Deferred-Remove (Markierung + Sweeping nach Publish) oder (b) Generation-Counter.
4. **Leak-Resistenz**
   - *Warum:* Vergessene Unsubscribe-IDs.
   - *Wie:* Optional `Subscription`-RAII-Objekt zurückgeben, das im Dtor unsubscribt.

```cpp
class Subscription {
  EventBus* bus_{}; HandlerId id_{}; std::type_index type_;
public:
  ~Subscription(){ if(bus_) bus_->unsubscribe(type_, id_); }
};
```

5. **Dokumentierte Thread-Semantik**
   - *Warum:* Konsumentenerwartung klären.
   - *Wie:* README: „EventBus ist threadsafe für subscribe/unsubscribe/publish; Callbacks laufen auf Call-Thread, reentrant verboten/erlaubt?“

---

## 5) Core: CommandBus

**Befund:** Sauber, aber kann unnötigen Overhead erzeugen.

**Verbesserungsvorschläge**

1. **Einsatzrichtlinie**
   - *Warum:* Verhindert Missbrauch als Allzweck-Hub.
   - *Wie:* „CommandBus nur für *gerichtete, einmalige Aktionen* (Undo/Redo, Persistence-Commands); EventBus für Broadcasts.“
2. **Rückgabewerte/Errors**
   - *Warum:* Fehlerpfade sichtbar machen.
   - *Wie:* `expected<T, Error>` oder Status-Codes vereinbaren; Logging-Hooks.

---

## 6) Logger

**Befund:** Minimal, keine Level/Filter/Thread-Safety/Targets.

**Verbesserungsvorschläge**

1. **LogLevel + Filter**

```cpp
enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };
```

- *Warum:* Rauschen reduzieren, Produktionslogs kontrollieren.

2. **Thread-sichere Pipeline**
   - *Wie:* Locking oder Single-Producer-Queue → Background-Flush-Thread optional.
3. **Mehrere Sinks**
   - *Wie:* ConsoleSink (optional ANSI-Farben), FileSink (Rolling), DebugSink.
4. **Konfiguration über Settings**
   - *Warum:* Level/Format/Datei-Pfade ohne Rebuild.

---

## 7) Settings (Persistenz)

**Befund:** Rudimentär, kein Schema/Migration/Typprüfung.

**Verbesserungsvorschläge**

1. **JSON + Schema + Version**
   - *Wie:* `settings.json` mit `"version": 1`. Bei Änderungen Migrationstabellen pflegen.
2. **Type-Safe Zugriff**
   - *Wie:* `get<T>(key, default)`; Validierung gegen Schema (z. B. JSON-Schema lib) beim Laden.
3. **Defaults & Fallback**
   - *Wie:* Central `DefaultSettings`-Map; fehlende Keys automatisch anlegen.
4. **Atomic Save**
   - *Wie:* Speichern nach `settings.json.tmp` → rename; Crash-sicher.

---

## 8) UI: Dialog/Panel Manager & Registries

**Befund:** Gute Struktur, Risiko: „God Object“, Ownership unklar.

**Verbesserungsvorschläge**

1. **Ownership explizit**
   - *Wie:* Manager hält `std::unique_ptr<Base>`; externe nur `weak_ptr` oder IDs.
2. **Lebenszyklus-API**
   - *Wie:* `create → initialize → show/hide → shutdown`; Reihenfolge dokumentieren + enforced.
3. **Registrierung über Factory**
   - *Wie:* `PanelFactory::create(id, cfg)` statt verstreute `new`.
4. **Entkopplung UI↔Render**
   - *Wie:* Render-Backend-Interface (z. B. `IRenderContext`) einziehen.

---

## 9) RenderWindow / Backend-Abstraktion

**Befund:** Mögliche Plattformkopplung (Win/Linux), direkte OpenGL/ImGui-Nutzung.

**Verbesserungsvorschläge**

1. **Backend-Interface**
   - *Wie:* `struct IRenderBackend { init(), beginFrame(), endFrame(), shutdown() };` Backends: Win32+GL, GLFW+GL, SDL+GL.
2. **Resource-Lifetime**
   - *Wie:* RAII für FBO/VAO/VBO; `std::optional`/`expected` für Fehler.

---

## 10) Visualizer & Manager

**Befund:** Sauberes Interface (`IVisualizer`), Risiken bei Wechsel/Lifecycle/Threads.

**Verbesserungsvorschläge**

1. **Factory + Ownership beim Manager**
   - *Wie:* Manager besitzt `unique_ptr<IVisualizer>`; Wechsel → altes `shutdown()` + Zerstörung **vor** create/init neu.
2. **Threading-Policy**
   - *Wie:* Callbacks laufen auf UI/Render-Thread; Hintergrund-FFT nur via lockfreie Mailbox.
3. **Parameter-System**
   - *Wie:* Zentrales `ParameterManager` mit Typen (Float/Int/Bool/Enum/Color/Vec2/Vec3) + Ranges + Presets.

---

## 11) CMake & Build

**Befund:** Teils modern, aber inkonsistent; fehlende Install/Export; Feature-Enforcement.

**Verbesserungsvorschläge**

1. **Features deklarieren**

```cmake
target_compile_features(PlugInSystem PRIVATE cxx_std_20)
```

2. **Strikte Sichtbarkeit**

```cmake
target_include_directories(App PRIVATE src PUBLIC include)
```

3. **Presets nutzen**

- Release/RelWithDebInfo mit sanitizern (Linux) + `/W4` (MSVC) konsistent.

4. **Install/Export**

```cmake
install(TARGETS App EXPORT AppTargets)
install(EXPORT AppTargets NAMESPACE PlugIn:: DESTINATION lib/cmake/App)
```

---

## 12) Externals & Lizenzen

**Befund:** BASS (proprietär), ImGuiFileDialog (MIT), GLAD okay.

**Verbesserungsvorschläge**

1. **LICENSE-Notice**
   - *Wie:* `THIRD_PARTY_NOTICES.md` mit Lizenztexten und Hinweisen (BASS nicht mitverteilen, nur Loaderpfad etc.).
2. **Optionaler Build**
   - *Wie:* `-DENABLE_BASS=ON/OFF` → Mock-Audio für CI.

---

## 13) Fehlerkultur, Tests, Diagnose

**Befund:** Wenig sichtbar.

**Verbesserungsvorschläge**

1. **Unit-Tests für Bus/Settings/Logger**
   - *Wie:* Subscriptions, Safe-Unsubscribe, Parallel-Publish, Settings-Migration, Log-Rotation.
2. **Asserts & Contracts**
   - *Wie:* `Expects/Ensures` (GSL) oder eigene leichte Macros.
3. **Structured Logging**
   - *Wie:* Optional JSON-Logs für Fehlerreports.

---

## 14) Performance & Threading

**Befund:** Potenzielle Hotpaths in EventBus/Visualizer.

**Verbesserungsvorschläge**

1. **Copy-on-Publish** (s. EventBus) vermeidet lange Locks.
2. **Lockfreie Queues** für Audio→UI Übergabe.
3. **Frame-Budgets** messen (ImGui Plot/Overlay „ms/frame“).

---

## 15) Persistenz/Config – Best Practices

- **Atomic Saves** (Temp + Rename)
- **Backup-Rotation** (`settings.bak1..3`)
- **Schema-Validation** vor Übernahme
- **Migrationspfad** (Version + Transform)

---

## 16) Architektur-Risiken & Leitlinien

1. **Service-Locator-Scope klein halten** (nur zentrale Services).
2. **Manager≠God-Object** – Funktionen delegieren, Builder/Factories einsetzen.
3. **Hart verdrahtete Singletons vermeiden** – Testbarkeit!
4. **Schnittstellen zuerst** – Backends austauschbar halten (Render, Audio, FileDialog).

---

## 17) Security & Robustheit (leichtgewichtig)

- **Input-Validation** für Presets/Settings-Dateien (kein Pfad-Traversal, erlaubte Verzeichnisse whitelisten).
- **Crash-Safe Saves** (Temp, Flush, fsync, Rename).
- **Defensives Laden externer Ressourcen** (nur bekannte Erweiterungen/Signaturen).

---

## 18) Konkrete Roadmap

**Woche 1 (Quick Wins)**

- EventBus: `uint64_t` IDs, Copy-on-Publish, RAII-Subscription.
- Logger: LogLevel + Console/File-Sinks, Mutex.
- Settings: JSON, Defaults, Atomic Save.

**Woche 2**

- UI/Registries: Ownership→`unique_ptr`, Factory einziehen.
- VisualizerManager: klare Lifecycle-Sequenz + Tests.
- CMake: `target_compile_features`, Presets vereinheitlichen.

**Woche 3**

- Render-Backend-Interface, Abstraktion gegen GLFW/SDL vorbereiten.
- Tests: Bus (parallel), Settings (Migration), Logger (Rotation).
- Third-Party Notices + BASS optional.

---

## 19) Checkliste (Abhakbar)

-

---

### Anhang: Mini-Beispiel Settings-API (Skizze)

```cpp
struct Settings {
  int windowWidth = 1280;
  int windowHeight = 720;
  std::string theme = "dark";
  int logLevel = 2; // Info
  int version = 1;

  static Settings load(const std::filesystem::path& p);
  void save(const std::filesystem::path& p) const; // atomic
};
```

```cpp
// Atomic save (Skizze)
void atomicSave(const std::string& json, const fs::path& path){
  auto tmp = path; tmp += ".tmp";
  std::ofstream o(tmp, std::ios::binary|std::ios::trunc); o << json; o.flush();
#if defined(_WIN32)
  o.close();
  std::filesystem::rename(tmp, path); // ersetzt atomar auf NTFS
#else
  ::fsync(o.rdbuf()->fd()); // falls verfügbar
  o.close();
  std::filesystem::rename(tmp, path);
#endif
}
```

---

**Hinweis:** Diese Analyse fokussiert auf Struktur- und Risikoaspekte. Für Deep-Dives (z. B. vollständige API-Revision EventBus/Logger/Settings inkl. Code) können wir je Modul einen kurzen Refactor-PR-Plan erstellen (Interface, Tests, Migrationsschritte).

