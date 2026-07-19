# VisualizerRenderThread — GL-Fenster + dedizierter Render-Thread

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** CppModuleDoc
> **Status:** Implementiert
> **Modul:** MyViz::UI::Widgets::VisualizerRenderThread
> **Dateien:** VisualizerRenderThread.hpp / VisualizerRenderThread.cpp
> **Abhängigkeiten:** Qt6 (Gui/OpenGL), IVisualizer
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Kern der Render-Thread-Entkopplung
([Render_Thread_Entwurf.md](../../docs/visuals/Render_Thread_Entwurf.md)):
`VisualizerGLWindow` (nacktes `QWindow`, OpenGLSurface) wird per
`createWindowContainer` in die `VisualizerWidget`-Fassade eingebettet;
`VisualizerRenderThread` (QThread) besitzt die Render-Schleife. Der
`QOpenGLContext` wird im GUI-Thread erzeugt und vor `start()` per
`moveToThread` in den Render-Thread verschoben; am Ende von `run()` wandert
er zurück, damit die Fassade ihn zerstören darf.

```
Schleife: warten bis exposed ─→ Surface-Release bedienen
        ─→ Kommandos poppen (Visualizer-Swap, Pacing, Resize)
        ─→ Audio-Snapshot übernehmen ─→ render() unter Render-Mutex
        ─→ swapBuffers (AUSSERHALB des Mutex) ─→ Pacing
```

## 2. Threading-Vertrag

- Alle öffentlichen Methoden werden im **GUI-Thread** gerufen; der
  GL-Lebenszyklus des Visualizers läuft NUR in `run()`.
- **Render-Mutex** (von der Fassade gereicht): der Thread hält ihn während
  `render(dt)` und den Lifecycle-Kommandos — nie während `swapBuffers`
  (VSync-Wartezeit blockiert die UI nicht). UI-Schreibzugriffe auf den
  Visualizer locken denselben Mutex (Vertrag:
  [Visualizer_Architecture.md §12](../../docs/visuals/Visualizer_Architecture.md)).
- **Audio-Snapshot:** `updateAudio()` schreibt Double-Buffer (eigener Mutex);
  der Thread ruft `updateSpectrum/updateWaveform` des Visualizers am
  Frame-Anfang — die Visualizer bleiben single-threaded.
- **Surface-Handshake:** `releaseSurfaceBlocking()` (aus
  `SurfaceAboutToBeDestroyed`, GUI-Thread) blockiert begrenzt (2 s), bis der
  Thread `doneCurrent()` gerufen hat — Undock/Fullscreen/Reparent zerstören
  nur die Surface, der Kontext (und alle GL-Ressourcen) überleben.
- **BasicLogger ist nicht thread-safe:** `run()` loggt nie; FPS verlassen den
  Thread als Queued-Signal `fpsMeasured`.

## 3. Pacing (RenderPacing)

| Modus | Verhalten |
|---|---|
| `Limited` | Sleep auf absoluten Frame-Fahrplan (Überschlaf wird im Folgeframe kompensiert — hält Ziel-FPS exakt); SwapInterval 0 |
| `Unlimited` | freies Rennen, SwapInterval 0 |
| `VSync` | `swapBuffers` blockiert auf Display-Refresh (nur diesen Thread); SwapInterval 1 zur Laufzeit via `wglSwapIntervalEXT` (Windows; Apple CGL; Linux: initiales Format) |

## 4. Visualizer-Swap / Größen

- `setVisualizer(next, retire)`: `next` (Ownership bleibt bei der Fassade)
  wird vor seinem ersten Frame im Thread GL-initialisiert; `retire` wird im
  Thread GL-bereinigt und **gelöscht**.
- Expose/Resize liefern **physische Pixel** (`size() * devicePixelRatio()`) —
  `glViewport` braucht Device-Pixel, `QWindow::size()` ist logisch
  (QOpenGLWidget rechnete früher intern um).
- `VisualizerGLWindow` reicht Doppelklick (`doubleClicked`) und Esc
  (`escapePressed`) als Signale an die Fassade (Fullscreen-Steuerung).

## 5. Stolpersteine

- `stopAndWait()` MUSS vor dem QApplication-Teardown laufen — die Fassade
  hängt sich dafür an `aboutToQuit` (`deleteLater` erreicht ohne Event-Loop
  keinen Destruktor mehr).
- `makeCurrent`-Fehlschlag (Surface noch nicht bereit) re-queued die bereits
  gepoppten Kommandos — sonst ginge ein Visualizer-Swap verloren.

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-19 | Initial (Session 31, Render-Thread-Entkopplung): Schleife, Surface-Handshake, Audio-Snapshot, Pacing mit absolutem Fahrplan, DPI-korrekte Viewports, Fullscreen-Signale |
