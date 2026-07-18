# Refactor‑PR‑Plan für **PlugInSystem**

> **Was ist ein PR‑Plan?**\
> Ein *Pull‑Request‑Plan* (PR‑Plan) beschreibt **Ziele, Scope, konkrete Änderungen, Migrationsschritte, Tests, Akzeptanzkriterien, Risiken und Rollback** für einen Refactoring‑PR. Er sorgt dafür, dass Reviewer schnell verstehen, **was** geändert wird, **warum** es nötig ist und **wie** die Änderung sicher eingeführt und geprüft wird.

---

## 1) Zielbild & Leitplanken

- **Zielbild Architektur:** modular, testbar, austauschbare Backends, sichere Persistenz, sauberes Logging, klare Thread‑Semantik, moderne CMake‑Konfiguration.
- **Leitplanken (Style & Struktur):**
  - **C++20**, UTF‑8, `#pragma once`, keine Header‑Guards.
  - **Member‑Präfix **``, weitere Präfixe nach Bedarf dokumentieren.
  - **Enums statt „magischer Zahlen“**, `enum class` mit klaren Bezeichnern.
  - **Header nur notwendige Includes**, `pch` nur in `.cpp` (wie gewünscht).
  - **Namensräume** flach halten; pro Modul klarer Namespace (z. B. `core::`, `ui::`).
  - **Dateibenennung:** `CamelCase` für Klassen; eine Klasse = eine Header/CPP‑Kombination.
  - **Fehlerbehandlung:** `expected<T,E>`/Statuscodes + zentrales Logging; Ausnahmen nur an Modulgrenzen abfangen.
  - **Thread‑Policy:** Bus/Logger/Settings sind **thread‑safe**, Visualizer‑Callbacks laufen auf dem **Render/UI‑Thread**; Hintergrundarbeit via Queues/Messages.

---

## 2) Konsistenzrichtlinien (Projektweit)

**Struktur:**

```
src/
  app/              # Application, ServiceContainer, Bootstrap
  core/             # EventBus, CommandBus, Common Utils
  log/              # Logger, Sinks, Formatter
  settings/         # Settings, Persistence, Schema, Migration
  ui/               # Managers (Panel/Dialog), Registries, RenderWindow
  visuals/          # IVisualizer, Manager, Registry, Parameter
  backends/         # render/ (GLFW/SDL/Win32), audio/ (BASS Mock)
  common/           # Error, Result/expected, RAII, Span/Range utils
```

**CMake:** eine **Library pro Modul** (OBJECT/STATIC/DLL je nach Bedarf), sichtbare Includes per `target_include_directories`, Features per `target_compile_features`. Kein globales `include_directories`.

**Code‑Style (Auszug):**

- Datei‑Kopfkommentar (Datei, Beschreibung, Autor, Datum).
- Öffentliche Schnittstellen auf Englisch; klare Inline‑Kommentare bei Variablen & Methoden.
- UTF‑8 strings; für Win‑APIs ggf. `std::wstring`/`OutputDebugStringW`.

---

## 3) Baseline (Status) vs. Target (Soll)

| Bereich      | Baseline                             | Target                                                                                    |
| ------------ | ------------------------------------ | ----------------------------------------------------------------------------------------- |
| EventBus     | typ‑sicher, aber ID/Threading fragil | stabile `uint64_t`‑IDs, Copy‑on‑Publish, RAII‑Subscription, dokumentierte Thread‑Semantik |
| CommandBus   | ok, Nutzen unscharf                  | klare Einsatzrichtlinien, Status/Errors, Tests                                            |
| Logger       | minimal                              | Level, Sinks (Console/File), thread‑safe Queue, konfigurierbar                            |
| Settings     | rudimentär                           | JSON + Schema + Version, Atomic Save, Migration, type‑safe API                            |
| UI‑Manager   | solide, Ownership unklar             | `unique_ptr`‑Ownership, Factory‑Registrierung, Lebenszyklus API                           |
| RenderWindow | plattformnah gekoppelt               | `IRenderBackend`‑Interface (GLFW/SDL/Win32)                                               |
| Visualizer   | modular, Lifecycle/Threads diffus    | klare Lifecycle‑Sequenz, Parameter‑System, Threading‑Policy                               |
| CMake        | teilweise modern                     | konsequent modern, Presets, Install/Export                                                |
| Externals    | BASS/ImGuiFileDialog                 | Third‑Party Notices, optionale Builds/Mocks                                               |

---

## 4) Refactor‑PR‑Plan nach Modulen

### 4.1 Core/EventBus

**Ziele:** thread‑sicher, stabile IDs, sicheres Unsubscribe, minimale Latenz.

**Änderungen (Scope):**

- `using HandlerId = uint64_t;` und `std::atomic<uint64_t> s_nextId{1};`.
- **Copy‑on‑Publish:** Handlerliste unter kleinem Lock kopieren, **Callbacks ohne Lock** aufrufen.
- **RAII‑Subscription:** `Subscription`‑Objekt, das im Destruktor automatisch `unsubscribe` aufruft.
- **Safe‑Unsubscribe während Dispatch:** deferred removal (mark & sweep) oder Generation‑Zähler.
- **Dokumentation Thread‑Semantik** (Callbacks laufen auf Aufrufer‑Thread; Reentrancy‑Hinweise).

**Tests:**

- Sub/Unsub parallel zu Publish (Stress, 10k Iterationen, TSAN bei Linux Preset).
- Prüfung: kein Lost‑Update, keine Double‑Free, keine Deadlocks.

**Akzeptanzkriterien:** deterministisches Verhalten unter Race, keine Datenrennen (TSAN clean), API stabil.

**Risiken & Rollback:** Falls Latenz steigt → Schalter für „direct dispatch“ (nur für Tests). Rollback: alte Map‑Strategie hinter `#ifdef EVENTBUS_LEGACY`.

---

### 4.2 Core/CommandBus

**Ziele:** klarer Use‑Case, Fehlerpfade sichtbar.

**Änderungen:**

- Richtlinie: **CommandBus** für gerichtete, einmalige Aktionen (Undo/Redo, Persistenz); **EventBus** für Broadcast.
- Handler signieren mit `expected<void, Error>` oder Statuscode.
- Optional: Sync/Async‑Ausführung (Queue)

**Tests:** Rückgabepfade, Error‑Propagation, „kein Handler registriert“ → definierter Status.

**Akzeptanzkriterien:** Dokumentierte Semantik, eindeutige Fehlercodes, Tests grün.

---

### 4.3 log/Logger

**Ziele:** observability, kontrollierbares Rauschen, sicher im Multi‑Thread.

**Änderungen:**

- `enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };`
- **Sinks:** Console (optional ANSI), File (Rolling, z. B. 5×5 MB), Debug.
- **Thread‑Safety:** Mutex oder lockfreie MPSC‑Queue + Background‑Flush‑Thread (konfigurierbar).
- **Konfiguration:** über Settings (Level, Format, Datei‑Pfad).

**Tests:**

- Parallel‑Logging 100 Threads → Reihenfolge/Verluste prüfen.
- Rotation: Größe > Schwellwert → neue Datei, alte aufbewahren.

**Akzeptanzkriterien:** Kein interleaved Text, Level‑Filterung funktioniert, Rotation greift.

---

### 4.4 settings/Settings

**Ziele:** robuste Persistenz, sichere Defaults, Migration.

**Änderungen:**

- **Format:** `settings.json` mit `{"version": 1, ...}`.
- **Schema‑Validierung** (leichtgewichtig; wenn keine Lib: manuelle Validierung per Typ/Range).
- **Type‑safe API:** `get<T>(key, default)` / `set<T>(key, value)`.
- **Atomic Save:** schreiben nach `.tmp` → flush → rename; Backup‑Rotation `.bak1..3`.
- **Migrationspfad:** `version` hochzählen, Transformationsfunktion je Schritt.

**Tests:**

- fehlende/kaputte Datei → Defaults + persistierter Neuaufbau.
- Migration 1→2→3 mit gezielten Änderungen.

**Akzeptanzkriterien:** Keine Korruption nach Crash (fsync/rename), Schemafehler werden geloggt + Defaults greifen.

---

### 4.5 ui/PanelManager, DialogManager, Registries

**Ziele:** klare Ownership, keine God‑Objects, einfache Erweiterbarkeit.

**Änderungen:**

- Manager besitzen ``; Außenwelt erhält IDs oder `weak_ptr`.
- **Factory‑Registrierung:** `registerPanel("Settings", []{ return std::make_unique<SettingsPanel>(); });`
- **Lebenszyklus‑API:** `create → initialize → show/hide → shutdown` dokumentiert + enforced.

**Tests:**

- Erzeugen/Entfernen in Schleifen (Leak‑Check), doppelte Registrierung abfangen.

**Akzeptanzkriterien:** Keine Leaks (ASAN/Valgrind), nachvollziehbarer Lifecycle.

---

### 4.6 backends/render (IRenderBackend)

**Ziele:** Portabilität + Testbarkeit.

**Änderungen:**

- Interface `IRenderBackend { init(); beginFrame(); endFrame(); shutdown(); }`.
- Implementierungen: `Win32GLBackend`, `GLFWGLBackend` (optional `SDLGLBackend`).
- **RAII** für GL‑Ressourcen (VAO/VBO/FBO); Fehler als `expected<void, Error>`.

**Tests:**

- Smoke‑Tests pro Backend (Fenster öffnen, Frame zählen), Headless‑Mode für CI (off‑screen).

**Akzeptanzkriterien:** Backends austauschbar ohne UI‑Code‑Änderungen.

---

### 4.7 visuals/ (IVisualizer, Manager, Parameter)

**Ziele:** reines Interface, sauberer Wechsel, Parameter‑System.

**Änderungen:**

- **Manager besitzt **``; Wechsel: `shutdown()` alt → destroy → `create/init` neu.
- **Threading‑Policy:** Visualizer‑APIs werden **nur** auf Render/UI‑Thread aufgerufen; Datenzufuhr via lockfreier Mailbox.
- **ParameterManager:** Typen (Float/Int/Bool/Enum/Color/Vec2/Vec3), Ranges, Presets, spätere Persistenz via Settings.

**Tests:**

- Hot‑Swap zwischen 10+ Visualizern in Schleife (Frame‑Stabilität, keine Leaks).

**Akzeptanzkriterien:** Keine Stalls/Leaks beim Wechsel, Parameter persistierbar.

---

### 4.8 app/ServiceContainer

**Ziele:** weniger Hidden Dependencies, testbarer Code.

**Änderungen:**

- **DI‑Punkte** definieren (Konstruktor‑Injection bevorzugt), ServiceContainer nur als Fallback.
- Dokumentation: Welche Services existieren? Wer besitzt sie? Lebenszyklus?

**Tests:** Unit‑Tests mit Test‑Doubles (Mock‑Logger/Mock‑Settings) via Konstruktor.

**Akzeptanzkriterien:** Module lassen sich isoliert konstruieren.

---

### 4.9 CMake

**Ziele:** Moderne, reproduzierbare Builds, klare Sichtbarkeiten.

**Änderungen:**

- `target_compile_features(<tgt> PRIVATE cxx_std_20)`
- `target_include_directories(<tgt> PUBLIC $<BUILD_INTERFACE:...> $<INSTALL_INTERFACE:include>)`
- **Warnflags:** MSVC `/W4 /permissive- /Zc:preprocessor /Zc:__cplusplus`, GCC/Clang `-Wall -Wextra -Wpedantic`.
- **Presets:** Debug/RelWithDebInfo/ASAN/TSAN (Linux), MSVC‑Release.
- **Install/Export:** `install(TARGETS ...)` + `install(EXPORT ...)` + `package`‑Vorbereitung.

**Tests:** CI‑Matrix (Win+Linux) baut **ohne** optionale BASS‑Lib (Mock‑Audio).

**Akzeptanzkriterien:** Ein Kommando je Plattform baut + testet konsistent.

---

### 4.10 Externals & Lizenzen

**Ziele:** Rechtssicherheit, reproduzierbare Builds.

**Änderungen:**

- `THIRD_PARTY_NOTICES.md` mit Lizenztexten/Verweisen.
- BASS als **optional** (`-DENABLE_BASS=ON/OFF`) + **Mock** für CI/ohne Distribution.

**Akzeptanzkriterien:** Repo kann ohne proprietäre Bits gebaut werden; Hinweise klar dokumentiert.

---

## 5) Querschnitt: Fehlerkultur, Tests, Observability

- **Unit‑Tests:** EventBus/CommandBus/Settings/Logger/VisualizerManager (Lifecycle).
- **Thread‑Sanitizer/Address‑Sanitizer** Presets (Linux) + `/W4` (MSVC).
- **Structured Logging (optional):** JSON‑Logs für Fehlermeldungen.
- **ImGui‑Overlay:** `ms/frame`, Queue‑Längen, aktiver Visualizer.

**Akzeptanzkriterien:** >80% Line‑Coverage in Core/Settings/Logger; Bus‑Tests TSAN‑clean.

---

## 6) Konsistenz‑Checks & Korrekturen (integriert)

- **Benennung:** `m_` für Member; `k`‑Präfix für Konstanten; `CamelCase` für Klassen/Enums; `snake_case` für Dateien optional *nicht*, wir bleiben bei Projektkonvention (Dateien wie Klassen).
- **Includes:** Header minimal, Implementierung in `.cpp`; `pch.h` ausschließlich in `.cpp`.
- **Namespaces:** Modulpräfixe `app:: core:: log:: settings:: ui:: visuals:: backends::`.
- **Enums:** statt `#define`/int‑Konstanten; Traits/`toString()` zentral.
- **UTF‑8‑Konformität:** alle Quelltexte; Win‑Calls über `W`‑APIs.

---

## 7) Migrationsplan

1. **Phase 1 (Core zuerst):** EventBus, Logger, Settings – neue APIs parallel einführen, alte Adapter bieten.
2. **Phase 2 (UI/Visualizer):** Ownership/Factories, Lifecycle; Render‑Backend‑Interface hinzufügen.
3. **Phase 3 (Aufräumen):** Alte Pfade/Adapter entfernen, CMake Install/Export, Third‑Party‑Hinweise.

Rollback: Jede Phase als eigener PR mit **Feature‑Flag** (z. B. `USE_NEW_EVENTBUS`, `USE_NEW_LOGGER`).

---

## 8) Roadmap & Aufwand (grobe Schätzung)

- **Woche 1:** EventBus (1–2 PT), Logger (2–3 PT), Settings (2 PT)
- **Woche 2:** UI‑Managers Ownership+Factory (2 PT), Visualizer Lifecycle (2 PT)
- **Woche 3:** Render‑Backend (2–3 PT), CMake Install/Export + Presets (1–2 PT), Third‑Party Notices (0.5 PT)\
  *PT = Personentage*

---

## 9) Akzeptanz‑Checkliste (abzuhaken)

-

---

### Anhang A: API‑Skizzen (Kurz)

**EventBus – Subscription (RAII):**

```cpp
class EventBus {
public:
  using HandlerId = std::uint64_t;
  template<typename E, typename F>
  [[nodiscard]] auto subscribe(F&& cb) -> Subscription; // RAII
  template<typename E>
  void publish(const E& e);
  template<typename E>
  void unsubscribe(HandlerId id);
};
```

**Logger – Konfiguration:**

```cpp
struct LogConfig {
  LogLevel level{LogLevel::Info};
  bool console{true};
  bool file{true};
  std::string filePath{"logs/app.log"};
  std::size_t rotateSizeBytes{5*1024*1024};
  int rotateCount{5};
};
```

**Settings – Atomic Save (Skizze):**

```cpp
void atomicSave(const std::string& json, const fs::path& path) {
  auto tmp = path; tmp += ".tmp";
  {
    std::ofstream o(tmp, std::ios::binary|std::ios::trunc);
    o << json; o.flush();
  }
  std::filesystem::rename(tmp, path);
}
```

---

**Hinweis für Reviewer:** Dieser PR‑Plan bringt konsistente Strukturen, definiert klare Policies (Threading, Logging, Persistenz) und minimiert Risiken durch inkrementelle Einführung + Tests. Jede Phase ist isoliert review‑bar, mit Rückfalloption über Feature‑Flags.

