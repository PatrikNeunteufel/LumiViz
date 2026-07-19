# Changelog Session 31 — Render-Thread-Entkopplung + Session-Playlist

> **Datum:** 2026-07-19
> **Typ:** Produkt-Changelog
> **Sprache:** Deutsch

## Neu

- **Render-Thread-Entkopplung** ([Render_Thread_Entwurf.md](../visuals/Render_Thread_Entwurf.md),
  Entwurf → Freigabe → Spike → Umsetzung in einer Session):
  - GL-Rendering läuft je Visualizer-Fenster in einem **eigenen Render-Thread**
    (`VisualizerRenderThread` + `VisualizerGLWindow`, eingebettet per
    `createWindowContainer`); `VisualizerWidget` ist Fassade mit stabiler API,
    **0 Änderungen in den 5 Visualizern**.
  - Panel-Scrollen/Previews kosten keine Visualizer-FPS mehr; der
    Application-Frame-Timer entfällt, Frame-Pacing (Limited mit absolutem
    Fahrplan / Unlimited / VSync) und FPS-Messung leben im Render-Thread
    (Statusbar via Queued-Signal vom primären Visualizer).
  - Audio-Übergabe per Snapshot-Puffer (Thread-Grenze); UI-Zugriffe auf den
    Visualizer über einen **Render-Mutex** abgesichert (ConfigPanel,
    SetParamCommand/Undo, Gradient-Editor, Preview-Sample) — Vertrag:
    [Visualizer_Architecture.md §12](../visuals/Visualizer_Architecture.md).
  - Der GL-Kontext gehört jetzt der App und **überlebt Undocking/Fullscreen**
    (Surface-Handshake); das Context-Tracking-Pattern der Visualizer wird zum
    Sicherheitsnetz ([OpenGL_Context_Handling.md](../visuals/OpenGL_Context_Handling.md) 1.1.0).
- **Echter Fullscreen:** Doppelklick zeigt den geklickten Visualizer als
  randloses Top-Level-Fenster — ohne Tab-Leiste, Dock-Titel und Panel-Tabs;
  Esc/Doppelklick/F11 führen zurück ins Dock (inkl. Neuaufbau der nativen
  Fenster-Handles nach dem Reparent).
- **Session-Playlist:** Beim Beenden wird eine nicht-leere Playlist
  automatisch gespeichert (`AppDataLocation/session.m3u8` + aktueller Index in
  QSettings) und beim nächsten Start wiederhergestellt — ohne Autoplay; eine
  leere Playlist räumt die Session-Datei weg
  ([Audio_System.md §5](../audio/Audio_System.md)).

## Gefixt

- **ConfigPanel-Reset-Bestandsbug:** `refreshPresetList()` setzte die
  Preset-Combo nach dem Signal-Unblock auf „Default" → `resetToDefaults()`
  bei JEDEM UI-Aufbau (sichtbar als Farbsprung beim Panel-Öffnen). Initial-
  Selektion läuft jetzt unter Signal-Sperre.
- **DPI-Skalierung:** Viewport nutzt physische Pixel
  (`size() * devicePixelRatio()`) — auf skalierten Displays (150 %) füllte
  das Bild sonst nur ⅔ der Fläche.
- **Versetzte native Fenster nach Fullscreen-Exit** („doppelter
  Player-/Status-Balken"): Qt ließ die native Fassade nach dem Reparent von
  Top-Level um die alten Rahmen-Offsets verschoben stehen; Fix per
  Native-Handle-Recreate (per Fenster-Diff vor/nach verifiziert).
- Esc im Fullscreen: Tastaturfokus wird explizit ins eingebettete GL-Fenster
  gelegt (`requestActivate` + Fokus-Kette), plus Fassaden-Fallback.

## Intern / Merkposten

- BasicLogger ist **nicht thread-safe** — Render-Threads loggen nie direkt
  (FPS via Queued-Signal); für spätere Thread-Arbeit: Logger härten oder
  GUI-Umleitung beibehalten.
- `deleteLater` erreicht beim App-Ende keinen Destruktor mehr —
  Render-Threads stoppen deterministisch am `aboutToQuit`.
- Offen aus Session 31: Undock-Sichttest-Runde über alle 5 Visualizer,
  6.3-Vergleichsmessung (Scroll/Previews), Neubewertung Preview-Tick
  20→10 Hz, Defaults-Mismatch `PulsingVisualizer::resetToDefaults()`
  (liefert Solid statt des konstruierten Gradients).
