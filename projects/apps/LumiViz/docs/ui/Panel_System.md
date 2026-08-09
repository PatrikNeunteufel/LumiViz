# Panel-System — Dockbare UI-Panels mit Qt-ADS und Self-Registration

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Überblick

LumiViz baut seine Oberfläche aus **dockbaren Panels** auf, verwaltet mit **Qt-ADS**
(Advanced Docking System). Panels können angedockt, schwebend, tabuliert, auto-hide
oder versteckt sein. Die Beteiligten:

| Komponente | Rolle | Quelle |
|---|---|---|
| `PanelRegistry` | Singleton; speichert Panel-Deskriptoren + Factories | `services/PanelRegistry.hpp` |
| `PanelAutoReg.cpp` | App-spezifische Registrierungen (`initPanelDefaults`) | `src/UI/panels/PanelAutoReg.cpp` |
| `PanelManager` | Erstellt Panel-Instanzen, Show/Hide/Toggle | `src/UI/managers/PanelManager.cpp` |
| `DockManager` | Kapselt `ads::CDockManager`, Layout-Persistenz, Event-Handler | `src/UI/managers/DockManager.cpp` |
| `PanelBase` | Basisklasse aller Panels (QWidget + IPanel) | `src/UI/panels/PanelBase.cpp` |

```
PanelAutoReg.cpp ──registriert──► PanelRegistry ──Factories──► PanelManager
                                                                    │
                                                 DockManager ◄──────┘
                                                 (ads::CDockManager, Events, Layout)
```

Details zu API und Layout-Persistenz: CppModuleDocs
[PanelBase](../../include/UI/panels/PanelBase.md) und
[DockManager](../../include/UI/managers/DockManager.md).

---

## 2. Registrierte Panels

Maßgeblich ist `src/UI/panels/PanelAutoReg.cpp` (Stand 2026-07-18):

| ID | Titel | Order | Default sichtbar | Klasse | Zweck |
|----|-------|------:|:---:|---|---|
| `player` | Player | 100 | ja | `PlayerPanel` | Wiedergabe: Play/Pause/Stop, Seek, Volume, Loop (Track) |
| `playlist` | Playlist | 200 | ja | `PlaylistPanel` | Playlist-Verwaltung, Multi-Selection, M3U, Shuffle/Loop |
| `config` | Visualizer Config | 300 | nein | `ConfigPanel` | Konfiguration des aktiven Visualizers → [ConfigPanel_Guide](ConfigPanel_Guide.md) |
| `settings` | Settings | 350 | nein | `SettingsPanel` | Globale Einstellungen (Audio, Performance) |
| `visual_select` | Visualizers | 400 | ja | `VisualSelectPanel` | Visualizer-Auswahl aus der VisualizerRegistry |

Das `VisualSelectPanel` listet die tatsächlich registrierten Visualizer
(aktuell: Pulsing, Waveform, Oscilloscope, Superscope, Equalizer) und publiziert
bei Auswahl ein `ChangeVisualizerEvent`.

Die Panel-ID ist zugleich das Qt-`objectName` und damit der Schlüssel für die
Layout-Persistenz — **nach Release nie mehr ändern**.

---

## 3. AutoReg — wie Panels registriert werden

`PanelRegistry::instance()` ruft beim **ersten Zugriff** automatisch
`initPanelDefaults(PanelRegistry&)` auf (Lazy-Init). Die Funktion lebt in
`PanelAutoReg.cpp`; die `extern`-Deklaration in `PanelRegistry.cpp` erzwingt,
dass der Linker die Datei auch bei statischen Libraries einbindet
(„Linker-Garantie").

Registriert wird ein `PanelDescriptor{id, title, order, defaultVisible, menuPath}`
plus Factory-Lambda `[](ServiceContainer&) -> std::unique_ptr<QWidget>`.

### Neues Panel — Checkliste

1. Klasse von `PanelBase` ableiten (`include/UI/panels/`, `src/UI/panels/`).
2. Header in `PanelAutoReg.cpp` inkludieren, Registrierung in
   `initPanelDefaults()` ergänzen.
3. Neue Dateien in die jeweiligen `Source.cmake` eintragen
   (Source-Listen sind explizit, kein Globbing).
4. CMake reconfigure + Build.

---

## 4. PanelBase-Lifecycle

Der Aktivierungszyklus hängt **direkt an der Qt-Sichtbarkeit** — das ist der
zentrale Mechanismus (`PanelBase.cpp`):

```
showEvent() ──► setActive(true)  ──► onActivate()   + Signal activated()
hideEvent() ──► setActive(false) ──► onDeactivate() + Signal deactivated()
```

Konvention: **EventBus-Subscriptions gehören in `onActivate()`, das
Unsubscribe in `onDeactivate()`** — ein verstecktes Panel empfängt so keine
Events und kostet nichts. Die Subscription-IDs werden in einem Member-Vektor
gesammelt und beim Deaktivieren abgemeldet.

```cpp
void MyPanel::onActivate() {
    if (auto* bus = eventBus())
        m_subscriptionIds.push_back(bus->subscribe<MyEvent>([this](const MyEvent& e) { /*...*/ }));
}
void MyPanel::onDeactivate() {
    if (auto* bus = eventBus())
        for (int id : m_subscriptionIds) bus->unsubscribe(id);
    m_subscriptionIds.clear();
}
```

> **Achtung (bekannte Falle):** Wird ein Panel zerstört, **ohne** vorher ein
> `hideEvent` zu durchlaufen, bleibt die Subscription mit `this`-Capture im
> EventBus zurück (Use-after-free-Risiko). Betrifft aktuell v. a. das
> ConfigPanel; ein RAII-Subscription-Handle ist für den Phase-4-Umbau
> vorgesehen (siehe Session-29-Code-Analyse, Abschnitt Risiken).

Weitere PanelBase-Bausteine: `saveState()`/`restoreState()` (QSettings unter
`panels/<id>/`), `preferredArea()`, Helfer `services()` und `eventBus()`.

---

## 5. Event-Integration

### 5.1 DockManager als Event-Handler

Der DockManager abonniert UI-Events selbst — neue Aktionen brauchen keine
MainWindow-Änderung (`DockManager.cpp`, `subscribeToEvents()`):

| Event | Wirkung |
|---|---|
| `CreateVisualizerEvent` | Neues Visualizer-Dock (mit `allowMultiple`-Check der WidgetRegistry) |
| `ResetLayoutEvent` | Default-Layout wiederherstellen |
| `ChangeVisualizerEvent` | Aktiven Visualizer wechseln |
| `TogglePanelEvent` | Panel via PanelManager ein-/ausblenden |
| `SaveDefaultLayoutEvent` | Aktuelles Layout als Default speichern |

Das Menü **View → Panels** befüllt `DockManager::populatePanelsMenu()` mit den
`toggleViewAction()`s von Qt-ADS — Häkchen und Sichtbarkeit bleiben automatisch
synchron (siehe [Menu_System](Menu_System.md)).

### 5.2 Panel-übergreifende Synchronisation

PlayerPanel und PlaylistPanel halten ihre Loop-Buttons über das
`PlaybackModeChangedEvent` konsistent:

- **PlayerPanel-Loop** = `RepeatMode::One` (Track-Wiederholung)
- **PlaylistPanel-Loop** = `RepeatMode::All` (Playlist-Wiederholung)

Beide abonnieren das Event und setzen ihren Button nur, wenn der gemeldete
`repeatMode` „ihrer" ist — aktiviert der eine Loop, geht der andere aus.

Das ConfigPanel abonniert `VisualizerChangedEvent` und baut seine UI für den
neuen Visualizer um.

---

## 6. Siehe auch

- [ConfigPanel_Guide.md](ConfigPanel_Guide.md) — Bedienung + Aufbau des ConfigPanels
- [Menu_System.md](Menu_System.md) — Menü-Self-Registration
- CppModuleDocs: [PanelBase](../../include/UI/panels/PanelBase.md) ·
  [DockManager](../../include/UI/managers/DockManager.md) ·
  [ConfigPanel](../../include/UI/panels/ConfigPanel.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-18 | Neu aus `harvest/old_docs/modules/Panel_System.md` konsolidiert; Panel-Tabelle und Lifecycle gegen Code verifiziert; Lifetime-Falle ergänzt |
