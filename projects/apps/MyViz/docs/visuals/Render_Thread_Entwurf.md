# MyViz — Render-Thread-Entkopplung (Entwurf)

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** Entwurf (Freigabe-Gate für die Umsetzung)
> **Status:** Umgesetzt (Session 31) — Sichttests Fullscreen/Config ✅; Undock-Runde alle 5 Visualizer + 6.3-Vergleichsmessung offen
> **Bezug:** [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md) ·
> [Visualizer_Architecture.md](Visualizer_Architecture.md) ·
> 6.3-Frametime-Messung (Session 30) · Auftrag Patrik 2026-07-19
> **Sprache:** Deutsch

## Motivation / Ist-Zustand

Qt-Repaints und GL-Rendering teilen sich heute den **Main-Thread**:

- `Application::run()` treibt per QTimer (PreciseTimer) `MainWindow::requestRender()`
  → `DockManager::requestRenderAll()` → `update()` auf jedem `VisualizerWidget`
  (**QOpenGLWidget**) → `paintGL()` läuft im GUI-Thread.
- Audio-Daten pullt ein 30-Hz-Timer in MainWindow (`onAudioUpdate`) aus BASS
  (`getFFTData`/`getWaveformData`) und schreibt sie synchron per
  `updateSpectrum/updateWaveform` in die Visualizer.
- VSync ist aus (`setSwapInterval(0)`), Frame-Pacing rein per Software-Timer.

**Messung 6.3 (Session 30):** Basis 60 fps; Live-Previews an ~52 fps,
Panel-Scrollen Spitzen bis ~45 fps — jeder Widget-Repaint verdrängt GL-Frames
und umgekehrt. Das ist strukturell, kein Optimierungsproblem einzelner Stellen.

## Zielbild

GL-Rendering läuft in einem **eigenen Render-Thread je Visualizer-Fenster**;
der Main-Thread macht nur noch Qt-UI. Panel-Scrollen/Previews kosten keine
Visualizer-FPS mehr, und VSync wird nutzbar (blockiert nur den Render-Thread).

## 1. Architektur

```
MainWindow / DockManager (Main-Thread)
   └── VisualizerWidget (QWidget-Fassade, API bleibt)
         ├── QWidget::createWindowContainer(…)
         └── VisualizerWindow : QWindow (SurfaceType OpenGLSurface)
               │   Events: Expose/Resize/DoubleClick → Signale/Flags
               ├── QOpenGLContext (von UNS besessen, per moveToThread)
               └── RenderThread (QThread)
                     Schleife: warten bis exposed → Kommandos abarbeiten
                     → Audio-Snapshot übernehmen → visualizer->render(dt)
                     → swapBuffers (VSync-Pacing)
```

- **`VisualizerWindow` (neu):** nacktes `QWindow` mit `OpenGLSurface`; besitzt
  den `QOpenGLContext` selbst (3.3 Core, 4x MSAA wie heute) und reicht Expose-/
  Resize-/Maus-Events als thread-sichere Flags/Signale weiter. Kein QOpenGLWidget
  mehr — dessen FBO-Komposition ist an den GUI-Thread gebunden.
- **`VisualizerWidget` bleibt als Fassade** (weiterhin `WidgetBase`-Erbe): kapselt
  Container + Window + Thread und behält die öffentliche API (`setVisualizer`,
  `updateSpectrum/updateWaveform`, `visualizer()`, Signale). DockManager,
  MainWindow, ConfigPanel und VisualSelectPanel bleiben quellkompatibel;
  Änderungen konzentrieren sich auf Widget-Innenleben + Application-Takt.
- **Ein Render-Thread pro Fenster** (DockManager kann mehrere Visualizer
  erzeugen): isolierte Kontexte, kein Shared-State zwischen Fenstern — wie heute.
- **Visualizer-Ownership wandert konzeptionell in den Render-Thread:**
  `initialize/render/resize/cleanup` laufen NUR noch dort. `setVisualizer()`
  wird zum Kommando an den Thread (Wechsel inkl. GL-Cleanup am Frame-Anfang).

## 2. Thread-Grenzen & Synchronisation

Berührungspunkte heute (alle Main-Thread) und ihr Mechanismus danach:

| Zugriff | Von | Mechanismus (neu) |
|---|---|---|
| `render/initialize/cleanup/resize` | Application-Takt | Render-Thread-Schleife (Resize als Flag mit letzter Größe) |
| `updateSpectrum/updateWaveform` | Audio-Tick 30 Hz | **Audio-Snapshot-Puffer** (§3), kein Direktschreiben mehr |
| `setParam/getParam` (ConfigPanel, SetParamCommand) | UI | **Render-Mutex** (§2.1) |
| `gradients()` — Editor schreibt in ColorGradientModule | UI | Render-Mutex |
| `tapPoints()[].sample()` (Preview-Tick 20 Hz) | UI | Render-Mutex (Kopie unter Lock, wie bisher pull-basiert) |
| `audioSourceModule()` (Modul-Presets) | UI | Render-Mutex |
| `setVisualizer` / applyPreset | UI | Kommando-Queue an den Render-Thread |
| VSync/FrameMode-Umschaltung | UI | Kommando-Queue (SwapInterval gehört dem Render-Thread) |

### 2.1 Render-Mutex statt Vollumbau (bewusste Stufe 1)

Ein Mutex je VisualizerWidget: Der Render-Thread hält ihn während
`render(dt)` (< ~5 ms), UI-Zugriffe (Param-Edits, Gradient-Editor, Preview-
Sample) locken kurz. UI-Zugriffe sind selten (Nutzer-Interaktion bzw. 20 Hz
Preview) — Kontention ist praktisch null, und das Modul-System
(paramDescs/get/set, GradientHandles, Taps) bleibt **unverändert**.
Eine vollständige Message-Queue-/Snapshot-Architektur wäre sauberer, kostet
aber einen Umbau aller 5 Visualizer + ConfigPanel-Bindings — bewusst NICHT
Teil dieses Pakets (→ Nicht-Ziele).

## 3. Audio-Übergabepuffer

- Struktur `AudioFrame { spectrum[512]; waveform[1024]; seq; }` als
  **Double-Buffer mit Mutex** im VisualizerWidget (Schreiber: Audio-Tick,
  Leser: Render-Thread übernimmt am Frame-Anfang den jüngsten Stand).
- Der Render-Thread ruft `updateSpectrum/updateWaveform` des Visualizers
  selbst auf (innerhalb des Render-Mutex) — für die Visualizer ändert sich
  **nichts**, die Daten kommen nur aus dem Puffer statt direkt vom Timer.
- 30-Hz-Audio-Tick bleibt im Main-Thread (BASS-Calls unverändert); eine
  spätere Verlagerung in einen Audio-Thread bleibt möglich, ist aber nicht
  Teil dieses Pakets.

## 4. Undocking / Kontext-Handling (heikle Stelle)

Heute erzwingt Qt-ADS-Undocking bei QOpenGLWidget einen **neuen Kontext** →
Context-Tracking-Pattern in allen 5 Visualizern (m_lastContext).

Mit eigenem Kontext ändert sich die Lage grundlegend:

- Der `QOpenGLContext` gehört UNS und dem Render-Thread — er überlebt das
  Reparenting des Containers. GL-Ressourcen bleiben gültig; das
  Context-Tracking wird zum **Sicherheitsnetz** statt Routinepfad
  (es bleibt drin, unverändert).
- Zu behandeln ist die **Surface**, nicht der Kontext:
  - `exposeEvent(false)` / verdeckt → Render-Thread pausiert (Wait-Condition).
  - `QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed` (kommt beim nativen
    Reparenting vor) → Thread anhalten, `doneCurrent()`, nach Recreate auf
    neue Surface `makeCurrent()` — Ressourcen bleiben erhalten.
- **Restrisiko:** natives Fenster (Window-Container) im ADS-Docking —
  Stacking/Fokus/Drag-Preview-Verhalten ist plattformabhängig. Darum Spike
  zuerst (§6 Schritt 0) mit genau dem Testszenario aus
  OpenGL_Context_Handling.md §10 (abdocken, andocken, Tab-Wechsel, Fullscreen).

## 5. Frame-Pacing, VSync, FPS

- Pacing wandert in die Render-Thread-Schleife: **VSync** = swapBuffers
  blockiert (Default-Empfehlung, jetzt gefahrlos), **Limited** = Sleep auf
  Soll-Frametime, **Unlimited** = freies Rennen. FrameMode-Menü bleibt.
- Der Application-QTimer (`requestRender`) und `requestRenderAll()`
  **entfallen**; die FPS-Messung wandert in den Render-Thread und wird per
  Queued-Signal an die Statusbar gemeldet (je Fenster; Anzeige: primärer
  Visualizer wie heute).
- Doppelklick-Fullscreen: `VisualizerWindow::mouseDoubleClickEvent` publiziert
  wie bisher `ToggleFullscreenEvent` (EventBus-publish ist per Queued-Dispatch
  abzusichern bzw. via Signal an die Fassade — Detail der Umsetzung).

## 6. Umsetzungsschritte

0. **Spike (Freigabe-Gate 2):** Wegwerf-Prototyp — ein `VisualizerWindow` +
   Render-Thread mit dem Pulsing-Visualizer im bestehenden Dock-Layout;
   Testszenario §10 des Context-Handling-Guides + Fullscreen. Erst bei
   Bestehen wird migriert. (Scheitert der Spike am ADS-Verhalten, Abbruch
   mit Befundbericht — Plan B wäre QOpenGLWidget behalten + Arbeit auslagern,
   deutlich schwächerer Effekt.)
1. **Infrastruktur:** `VisualizerWindow`, `RenderThread`, Audio-Snapshot-
   Puffer, Render-Mutex; `VisualizerWidget` auf Fassade umbauen (API stabil).
2. **Taktgeber-Umbau:** Application-Frame-Timer raus, FrameMode/VSync/FPS in
   den Thread, Statusbar-Anbindung per Signal.
3. **UI-Zugriffe absichern:** ConfigPanel/SetParamCommand/Gradient-Editor/
   Preview-Tick über Render-Mutex bzw. Kommando-Queue führen (zentral in der
   Fassade, nicht in den Visualizern).
4. **Sichttests + Messung:** Szenario wie 6.3 (Basis / Previews an / Scrollen),
   Ziel-Nachweis §7; Undock-Dauertest alle 5 Visualizer.
5. **Doku-Nachzug:** Visualizer_Architecture (Threading-Kapitel),
   OpenGL_Context_Handling (Rolle des Patterns als Sicherheitsnetz),
   Changelog. Danach Neubewertung Preview-Tick 20→10 Hz (Entscheid 2026-07-19).

## 7. Akzeptanzkriterien

- Visualizer hält seine Ziel-FPS (60) bei Panel-Scrollen und eingeschalteten
  Live-Previews (Toleranz: kein Einbruch > 5 %); Messmethode wie 6.3.
- Undock/Andock/Tab-Wechsel/Fullscreen crashfrei mit allen 5 Visualizern.
- UI bleibt flüssig bei Unlimited-FrameMode (GUI-Thread unbelastet).
- Suite bleibt grün (91 Cases, 0 Skips); Visualizer-Code selbst unverändert
  bis auf nichts (Ziel: 0 Änderungen in den 5 Visualizern).

## Nicht-Ziele (bewusst)

Keine Message-Queue-/Snapshot-Vollarchitektur (§2.1), kein eigener
Audio-Thread, kein Multi-Window-übergreifendes Context-Sharing, keine
Änderung an Modul-System/Pipeline-Schema, kein Umbau der TapPreviews
(sie pollen weiter, nur unter Lock).

## Offene Fragen (bitte mit Freigabe entscheiden)

1. **Default-FrameMode danach:** VSync als neuer Default (empfohlen) oder
   Limited/60 wie heute?
2. **FPS-Anzeige** bei mehreren Visualizern: primärer wie heute (empfohlen)
   oder je Fenster im Dock-Titel?

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 1.0.0 | 2026-07-19 | Umsetzung abgeschlossen inkl. Nacharbeiten: DPI-korrekte Viewports (physische Pixel), ConfigPanel-Preset-Combo-Bestandsbug (Reset bei jedem Rebuild) gefixt, echtes Fullscreen (Widget top-level statt Dock-Verstecken; Esc via GL-Fenster-Fokus; Native-Handle-Recreate gegen versetzte Fenster nach Reparent — per Fenster-Diff verifiziert), Fullscreen folgt der Event-Quelle. Doku-Nachzug (Schritt 5) erledigt. Offen: Undock-Sichttest-Runde alle 5 Visualizer, 6.3-Vergleichsmessung |
| 0.2.0 | 2026-07-19 | Freigegeben; Spike bestanden (Befunde: BasicLogger nicht thread-safe → FPS via Queued-Signal; deleteLater erreicht bei App-Ende keinen Destruktor → Thread-Stop am aboutToQuit). Schritte 0–3 umgesetzt (VisualizerRenderThread + VisualizerWidget-Fassade, Taktgeber-Umbau, Render-Mutex-Guards inkl. GradientEditor/SetParamCommand); Limited-Pacing mit absolutem Frame-Fahrplan |
| 0.1.0 | 2026-07-19 | Initial (Session 31) — zur Freigabe |
