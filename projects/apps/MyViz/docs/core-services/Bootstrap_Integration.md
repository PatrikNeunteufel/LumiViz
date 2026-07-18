# Bootstrap & Integration — Init-Reihenfolge, Services, Event-Flow, Shutdown

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Übersicht

Einstiegskette (verifiziert gegen `main/main.cpp`, `src/Application.cpp`,
`src/UI/MainWindow.cpp`):

```
main() ──► Application::init() ──► Application::run() ──► Application::shutdown()
                 │
                 └── erstellt QApplication + MainWindow
                            │
                            └── MainWindow besitzt ServiceContainer, DockManager,
                                MenuManager und registriert alle Services
```

Wichtig gegenüber der Alt-Doku: **nicht `Application`, sondern `MainWindow` erstellt den
`ServiceContainer`** und registriert die Services. `Application` kümmert sich um
Qt-Lifecycle, GPU-Auswahl, Preset-Verzeichnisse und die Frame-Loop.

DI-Container-Details (Lifetimes, API, Thread-Safety) stehen in der Modul-Doku
[`../../include/services/ServiceContainer.md`](../../include/services/ServiceContainer.md).

---

## 2. Init-Reihenfolge

### 2.1 Application::init() (`src/Application.cpp`)

1. GPU-Enumeration + Auswahl (`GpuInfo`/`GpuSelector`, `gpu.ini`) — vor QApplication
2. `QApplication` erstellen (Name/Version/Organisation setzen)
3. User-Preset-Verzeichnisse setzen (nach QApplication wegen `QStandardPaths`,
   vor MainWindow): `ColorGradientModule`, `SmoothingModule`, `AudioSourceModule`
4. `MainWindow` erstellen (erzeugt OpenGL-Kontext) und `show()`
5. Frame-Timing initialisieren

### 2.2 MainWindow-Konstruktor (`src/UI/MainWindow.cpp`)

1. `ServiceContainer` erstellen
2. `registerSingleton<IEventBus, EventBus>()`
3. `setupAudioServices()` — Audio-Services registrieren **und sofort auflösen**
   (siehe 3.)
4. `setupUi()`:
   - `DockManager` erstellen (erzeugt intern PanelManager + alle Panels aus der
     PanelRegistry, abonniert seine Events)
   - `setupMenuBar()` — `MenuManager` + `buildMenuBar()`; Panels-/Perspectives-Menüs
     vom DockManager befüllen lassen
   - `setupStatusBar()` — FPS-Label
   - `setupDefaultLayout()` — **erst** initialen Visualizer erstellen
     (`setVisualizer("pulsing")`), **dann** `restoreLayout()` (Qt-ADS kann nur
     existierende Widgets positionieren); Default-Perspektive speichern, falls keine
     existiert
   - `setupEventHandlers()` — MainWindow-Events abonnieren
5. Audio-Update-Timer starten (`QTimer`, 33 ms ≈ 30 Hz → `onAudioUpdate()`)

### 2.3 Application::run()

- Frame-Timer (`QTimer`, `Qt::PreciseTimer`) treibt `MainWindow::requestRender()`
  + FPS-Messung; Intervall je nach FrameMode (Limited/Unlimited/VSync)
- Qt-Signal `MainWindow::frameModeChangeRequested(int)` → FrameMode + VSync umschalten
- danach `QApplication::exec()` (Qt-Event-Loop, keine manuelle processEvents-Schleife)

### 2.4 Registries (automatisch)

Die fünf Registries (Menu, Panel, Dialog, Widget, Visualizer) initialisieren sich
**lazy beim ersten `instance()`-Aufruf** über ihre `initXxxDefaults()` — es gibt keinen
expliziten Registrierungs-Schritt im Bootstrap
(Details: [Registries.md](Registries.md)).

---

## 3. Service-Registrierung (Ist-Stand)

Alle Registrierungen passieren im MainWindow (`setupAudioServices()` + Konstruktor):

| Interface | Implementierung | Anmerkung |
|-----------|-----------------|-----------|
| `IEventBus` | `EventBus` | erster registrierter Service |
| `IAudioEngine` | `BassEngine` | Factory ruft `initialize()`; Fehler → nullptr |
| `IPlaylist` | `Playlist` | Factory mit **captured** EventBus-Referenz |
| `IAudioPlayer` | `AudioPlayer` | Factory mit captured Engine/EventBus/Playlist; verbindet Player↔Playlist |

**Deadlock-Regel:** Der historische `std::mutex`-Deadlock (resolve() in einer Factory)
ist zwar seit dem Wechsel auf einen rekursiven Mutex behoben (Test-Fundament Phase 3),
der Code vermeidet verschachtelte `resolve()`-Aufrufe in Factories aber weiterhin:
Abhängigkeiten werden **vor** der Registrierung aufgelöst und in die Factory-Lambda
captured; nach jeder Registrierung erzwingt ein `tryResolve<T>()` die sofortige
Instanziierung. Effektiv sind damit alle Audio-Services am Ende des
MainWindow-Konstruktors bereits erstellt und verdrahtet.

---

## 4. Event-Flow (Beispiele)

### Menü → DockManager

```
User klickt "View → New Visualizer"
  → MenuAutoReg-Callback: eventBus.publish(CreateVisualizerEvent{})
  → DockManager (Subscriber): createVisualizer(...), respektiert allowMultiple
```

### Panel → Visualizer

```
User wählt Visualizer im VisualSelectPanel
  → eventBus.publish(ChangeVisualizerEvent{id})
  → DockManager: aktiven VisualizerWidget auf neuen Effekt umschalten
  → VisualizerWidget publiziert VisualizerChangedEvent (u. a. für ConfigPanel)
```

### Audio → Visualizer (kein EventBus!)

```
QTimer (30 Hz) → MainWindow::onAudioUpdate()
  → IAudioPlayer::update() (publiziert Positions-/State-Events für die UI)
  → IAudioEngine::getFFTData()/getWaveformData()
  → direkt an alle VisualizerWidgets: updateSpectrum()/updateWaveform()
```

Die hochfrequenten FFT-/Waveform-Daten gehen als direkter Methodenaufruf an die
Visualizer, nicht über den EventBus; der Bus transportiert die UI-relevanten
Audio-Events (Track/Position/State, siehe [Event_System.md](Event_System.md)).

---

## 5. Shutdown-Reihenfolge

```
Qt-Event-Loop endet (Fenster zu / Exit)
  → Application::run() kehrt zurück, Frame-Timer stoppt
  → Application::shutdown():
       MainWindow.reset()          // vor QApplication!
         → ~MainWindow:
              DockManager::closeAll() + reset()   // VOR QMainWindow-Basisdestruktor
                → DockManager::unsubscribeFromEvents() (IDs werden abgemeldet)
              ServiceContainer.reset()            // zerstört Audio-Services + EventBus
       QApplication.reset()
```

Kritische Regeln (im Code dokumentiert):

1. **DockManager vor dem QMainWindow-Basisdestruktor zerstören** — Qt-ADS trackt
   Floating-Container per QPointer; falsche Reihenfolge → Dangling-Pointer/Crash.
2. **Layout-Restore erst nach Widget-Erstellung** (siehe 2.2 Schritt 4).
3. **Event-Subscriptions in Destruktoren abmelden** (DockManager macht es vor;
   Unsubscribe-Pflichten: [Event_System.md](Event_System.md), Abschnitt 5).

Abweichung zur Alt-Doku: `MainWindow::~MainWindow` räumt **keine** eigenen
Event-Subscription-IDs auf (es werden keine gespeichert) und es gibt **keinen**
`aboutToQuit`-Connect für die Layout-Speicherung im aktuellen Code-Stand.

---

## Siehe auch

- Modul-Doku: [`ServiceContainer.md`](../../include/services/ServiceContainer.md) (DI-API, Lifetimes)
- [Event_System.md](Event_System.md) — EventBus, Handler, Thread-Safety
- [Registries.md](Registries.md) — Lazy-Init der Komponenten-Registries
