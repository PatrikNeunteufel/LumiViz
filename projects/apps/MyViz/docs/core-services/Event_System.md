# Event-System — Publish/Subscribe über den EventBus

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Übersicht

MyViz koppelt Komponenten lose über das **Publish/Subscribe-Pattern**: Publisher
(Menü-Aktionen, Panels, Audio-Services) publizieren typisierte Events auf dem
**EventBus**; Subscriber (DockManager, MainWindow, Panels, Visualizer) reagieren darauf,
ohne den Publisher zu kennen.

```
Publisher ──publish()──► EventBus (IEventBus) ──notify()──► Subscriber 1..n
```

- **Interface:** `IEventBus` — [`include/services/IEventBus.hpp`](../../include/services/IEventBus.hpp)
- **Implementierung:** `EventBus` — [`include/services/EventBus.hpp`](../../include/services/EventBus.hpp), `src/services/EventBus.cpp`
- **Event-Basisklasse:** [`include/services/events/Event.hpp`](../../include/services/events/Event.hpp)
- **Registrierung:** als Singleton im `ServiceContainer` (`registerSingleton<IEventBus, EventBus>()`, siehe [Bootstrap_Integration.md](Bootstrap_Integration.md))

API-Details (Signaturen, Beispiele, Best Practices) stehen in der header-nahen
Modul-Doku [`../../include/services/EventBus.md`](../../include/services/EventBus.md) —
hier nur der App-weite Überblick.

---

## 2. API-Kern

| Operation | Bedeutung |
|-----------|-----------|
| `subscribe<T>(handler [, priority])` | Handler für Event-Typ `T` registrieren; liefert `SubscriberId` (`std::uint64_t`). Niedrigere Priorität wird zuerst aufgerufen (Default 0). |
| `unsubscribe(id)` | Subscription per ID entfernen |
| `publish(event)` | **Synchron** an alle Subscriber dispatchen (im aufrufenden Thread) |
| `queue(event)` / `dispatchQueued()` | Event thread-safe einreihen; Dispatch später im Main-Thread |
| `subscriberCount<T>()` / `clear()` | Utility |

Alle Event-Typen müssen von `Event` ableiten (`static_assert` in `subscribe`/`publish`)
und implementieren `typeName()` über das Makro `EVENT_TYPE_NAME("Name")`. Die Basisklasse
liefert zusätzlich einen Timestamp und `consume()`/`isConsumed()`: ein konsumiertes Event
stoppt die weitere Propagation an nachfolgende Subscriber.

> **Bekannte Wart:** Handler erhalten `const Event&`, `consume()` ist aber non-const —
> ein Subscriber kann ein Event derzeit nicht ohne `const_cast` konsumieren
> (dokumentiert, Behebung geplant in Phase 4).

---

## 3. Event-Typen

| Kategorie | Datei | Beispiele |
|-----------|-------|-----------|
| **UI-Events** | [`include/services/events/UIEvents.hpp`](../../include/services/events/UIEvents.hpp) | `TogglePanelEvent`, `OpenDialogEvent`, `CreateVisualizerEvent`, `ChangeVisualizerEvent`, `VisualizerChangedEvent`, `ResetLayoutEvent`, `SaveDefaultLayoutEvent`, `FrameModeChangedEvent`, `FpsUpdateEvent`, `ToggleFullscreenEvent`, `ExitFullscreenEvent`, `ThemeChangedEvent`, Visualizer-Config-Events (`VisualizerColorSchemeEvent`, `VisualizerSmoothingEvent`, `VisualizerPeakHoldEvent`, `VisualizerShapeEvent`) |
| **Audio-Events** | `include/audio/AudioEvents.hpp` | `TrackChangedEvent`, `PlaybackStateEvent`, `PlaybackPositionEvent`, `VolumeChangedEvent`, `PlaylistChangedEvent`, `PlaylistIndexChangedEvent`, `AudioDataEvent`, `BeatEvent`, `AudioEngineErrorEvent`, `AudioDeviceChangedEvent`, `PlaybackModeChangedEvent` |

Hinweis: `FrameModeChangedEvent.mode` ist ein `int` (0=Limited, 1=Unlimited, 2=VSync),
kein `FrameMode`-Enum — MainWindow reicht den Wert per Qt-Signal
`frameModeChangeRequested(int)` an `Application` weiter, die das Mapping auf `FrameMode`
vornimmt.

---

## 4. Dezentrale Event-Handler (Ist-Stand)

Die Handler sind bewusst dezentral: jede Komponente abonniert nur, was sie selbst
behandelt.

| Event | Subscriber | Aktion |
|-------|-----------|--------|
| `CreateVisualizerEvent` | `DockManager` | neuen Visualizer-Dock erstellen (respektiert `allowMultiple` aus WidgetRegistry) |
| `ResetLayoutEvent` | `DockManager` | Layout zurücksetzen |
| `ChangeVisualizerEvent` | `DockManager` | aktiven Visualizer wechseln |
| `TogglePanelEvent` | `DockManager` | Panel ein-/ausblenden |
| `SaveDefaultLayoutEvent` | `DockManager` | aktuelles Layout als Default speichern |
| `FrameModeChangedEvent` | `MainWindow` | Signal `frameModeChangeRequested(mode)` emittieren |
| `OpenDialogEvent` | `MainWindow` | derzeit nur Debug-Log (siehe Abschnitt 6) |
| `ToggleFullscreenEvent` | `MainWindow` | Fullscreen umschalten |
| `ExitFullscreenEvent` | `MainWindow` | Fullscreen verlassen (Esc) |
| Audio-Events | Panels (Player, Playlist, …) | UI-Updates |

Wichtige Publisher: `MenuAutoReg.cpp` (Menü-Aktionen → `CreateVisualizerEvent`,
`ToggleFullscreenEvent`, `ResetLayoutEvent`, `SaveDefaultLayoutEvent`,
`FrameModeChangedEvent{0|1|2}`, `OpenDialogEvent{"about"}`), `VisualSelectPanel`
(`ChangeVisualizerEvent`), `SettingsPanel` (`FrameModeChangedEvent`), `VisualizerWidget`
(`VisualizerChangedEvent`, `ToggleFullscreenEvent`), `AudioPlayer`/`Playlist`/
`AudioAnalyzer` (Audio-Events).

---

## 5. Lifecycle & Unsubscribe-Pflichten

- **SubscriberId aufbewahren** (z. B. `std::vector<IEventBus::SubscriberId>`) und im
  Destruktor `unsubscribe()` aufrufen — sonst ruft der Bus einen Handler auf ein
  zerstörtes Objekt auf. So macht es der `DockManager`
  (`subscribeToEvents()`/`unsubscribeFromEvents()` mit ID-Liste) und der
  `DialogManager` (Unsubscribe im Destruktor).
- **Ausnahme im Ist-Code:** `MainWindow::setupEventHandlers()` speichert die IDs
  *nicht*. Das ist nur deshalb unkritisch, weil `MainWindow` den `ServiceContainer`
  (und damit den EventBus) selbst besitzt und beide gemeinsam sterben. Für alle anderen
  Komponenten gilt die Aufbewahrungs-/Unsubscribe-Pflicht uneingeschränkt.
- EventBus kann `nullptr` sein: Zugriff defensiv über
  `services.tryResolve<IEventBus>()` mit Null-Check.

---

## 6. DialogManager (Stand 2026-07-18)

Der im alten Doc als TODO vermerkte `DialogManager` **existiert inzwischen als Klasse**
([`include/UI/managers/DialogManager.hpp`](../../include/UI/managers/DialogManager.hpp),
`src/UI/managers/DialogManager.cpp`, wird mitkompiliert): `show()`/`showModeless()`
erzeugen Dialoge aus der `DialogRegistry`, `subscribeToEvents()` abonniert
`OpenDialogEvent`, Unsubscribe im Destruktor.

**Er ist aber noch nicht verdrahtet:** keine Stelle instanziiert ihn. Der
`OpenDialogEvent`-Handler in `MainWindow` trägt weiterhin das TODO und loggt nur —
der About-Dialog wird aktuell über das Menü (Help → About, F1) **nicht geöffnet**.

---

## 7. Thread-Safety

Der EventBus ist — anders als im alten Architektur-Doc behauptet — **weitgehend
thread-safe** (Mutex-geschützt):

| Operation | Thread-safe | Bemerkung |
|-----------|-------------|-----------|
| `subscribe()` / `unsubscribe()` | ja | Mutex |
| `publish()` | teilweise | Subscriber-Liste wird unter Lock kopiert, Handler laufen **ohne Lock im aufrufenden Thread** → aus Worker-Threads nicht verwenden |
| `queue()` | ja | eigener Queue-Mutex, für Cross-Thread gedacht |
| `dispatchQueued()` | Main-Thread | dispatcht alle eingereihten Events |
| `clear()` / `subscriberCount()` | ja | Mutex |

**Ist-Stand der App:** `queue()`/`dispatchQueued()` werden derzeit **nicht benutzt** —
es gibt keinen Aufrufer. Alle Events (inkl. Audio) werden synchron per `publish()` aus
dem Main-Thread publiziert; die Audio-Daten treibt ein 30-Hz-`QTimer` in `MainWindow`
(`onAudioUpdate()`). Der Queue-Mechanismus steht für künftige Worker-Threads bereit.

---

## Siehe auch

- Modul-Doku: [`EventBus.md`](../../include/services/EventBus.md) (API, Beispiele, Best Practices)
- [Registries.md](Registries.md) — Komponenten-Registrierung
- [Bootstrap_Integration.md](Bootstrap_Integration.md) — Init-Reihenfolge und Event-Flow
