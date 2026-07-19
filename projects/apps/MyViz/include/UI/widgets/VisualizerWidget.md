# VisualizerWidget — Fassade für Visualizer-Rendering im Render-Thread

> **Version:** 3.0.0
> **Datum:** 2026-07-19
> **Typ:** CppModuleDoc
> **Status:** Implementiert
> **Modul:** MyViz::UI::Widgets::VisualizerWidget
> **Dateien:** VisualizerWidget.hpp / VisualizerWidget.cpp
> **Namespace:** (global)
> **Abhängigkeiten:** Qt6 (Widgets/Gui/OpenGL), WidgetBase, VisualizerRenderThread, VisualizerRegistry, EventBus
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Seit v3.0.0 (Render-Thread-Entkopplung, Session 31) ist das Widget **kein
QOpenGLWidget mehr**, sondern eine dünne Fassade:

```
WidgetBase<QWidget>
  └── VisualizerWidget (Fassade — öffentliche API stabil zu v2.x)
        ├── createWindowContainer(…)
        ├── VisualizerGLWindow : QWindow    (eingebettetes natives Fenster)
        ├── QOpenGLContext                  (besessen; lebt im Render-Thread)
        ├── VisualizerRenderThread          (Render-Schleife, Pacing, FPS)
        └── QMutex renderMutex              (UI ↔ Render-Thread)
```

Verantwortlichkeiten: Visualizer-Verwaltung (`setVisualizer` per Registry,
Swap als Thread-Kommando), Audio-Weitergabe in den Snapshot-Puffer,
Frame-Pacing-API (`setFrameMode`), Fullscreen-Signale (Doppelklick/Esc →
`ToggleFullscreenEvent` mit Quelle), FPS-Durchreichung (`fpsMeasured`).
Die Render-Schleife selbst: [VisualizerRenderThread.md](VisualizerRenderThread.md).

## 2. Threading-Vertrag (wichtigster Punkt der API)

`visualizer()` liefert den rohen `IVisualizer*`, der **parallel vom
Render-Thread gerendert wird**. Regeln
([Visualizer_Architecture.md §12](../../docs/visuals/Visualizer_Architecture.md)):

- Schreibzugriffe (setParam/applyPreset/resetToDefaults/Gradient-Mutationen)
  und `TapPoint::sample()` nur unter `renderMutex()`.
- Config-Lesezugriffe (paramDescs/getParam/gradients) und Identitätsdaten
  sind ohne Lock zulässig.
- `VisualizerChangedEvent` transportiert `renderMutex` zu den Abonnenten
  (ConfigPanel, SetParamCommand).

## 3. Lebenszyklus

- Konstruktor: Fenster + Kontext + Thread aufbauen, Kontext per
  `moveToThread` übergeben, Default-Visualizer (pulsing) laden,
  `aboutToQuit`-Hook (deterministischer Thread-Stop).
- `setVisualizer(id)`: Instanz im GUI-Thread erzeugen (Registry, GL-frei),
  Event publizieren, GL-Init/Cleanup übernimmt der Thread.
- Destruktor: `stopAndWait()` (Thread GL-bereinigt den aktiven Visualizer),
  danach sterben Visualizer und Kontext per unique_ptr.
- `recreateNativeWindow()`: Native Handles verwerfen und neu erzeugen —
  nötig nach Reparent von Top-Level (Fullscreen-Exit), weil Qt die native
  Fassade sonst an einer stale Absolutposition stehen lässt.

## 4. Fullscreen

Doppelklick/Esc im GL-Fenster publizieren `ToggleFullscreenEvent{this}` —
MainWindow nimmt DIESES Widget aus seinem Dock und zeigt es als randloses
Top-Level (`showFullScreen`); Rück-Einbettung inkl. Native-Handle-Recreate.
Fokus-Kette Widget → Container → GL-Fenster ist explizit verdrahtet
(`setFocusProxy`, `activateGLWindow()`), damit Esc ankommt.

## 5. Änderungsbilanz v2 → v3

| v2.x (QOpenGLWidget) | v3.0.0 (Fassade) |
|---|---|
| paintGL im Main-Thread, Takt via Application-QTimer | Render-Schleife im eigenen Thread, Pacing dort (Limited/Unlimited/VSync) |
| Undock = neuer Kontext (Context-Tracking als Routinepfad) | Kontext überlebt; Surface-Handshake; Tracking = Sicherheitsnetz |
| `setVSync(bool)` | `setFrameMode(RenderPacing, targetFps)` |
| `setClearColor` (ungenutzt) | entfernt; Fallback-Clear im Thread (dunkel statt Rot) |
| Audio-Push direkt in den Visualizer | Audio-Snapshot-Puffer (Thread-Grenze) |

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 3.0.0 | 2026-07-19 | Render-Thread-Entkopplung: Fassade statt QOpenGLWidget (API stabil), Render-Mutex-Vertrag, Audio-Snapshot, Fullscreen mit Quelle + Native-Handle-Recreate, fpsMeasured |
| 2.3.0 | 2026-07-19 | Toten Config-Event-Kanal entfernt (Phase 4 Schritt 0) |
| 2.1.0 | 2025-12 | WidgetBase<QOpenGLWidget>-Umstellung |
