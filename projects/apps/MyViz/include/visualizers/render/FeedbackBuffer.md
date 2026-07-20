# FeedbackBuffer / OffscreenBufferPool — Offscreen-Fundament (Roadmap 4.3)

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.3 — Sichttest ausstehend)
> **Module:** lumi::render::FeedbackBuffer · lumi::render::OffscreenBufferPool
> **Dateien:** render/FeedbackBuffer.hpp + FeedbackBuffer.cpp · render/OffscreenBufferPool.hpp (header-only)
> **Namespace:** lumi::render
> **Abhängigkeiten:** Qt6 OpenGL (QOpenGLFramebufferObject, Shader, VAO/VBO)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. FeedbackBuffer

Die MilkDrop-/AVS-Feedback-Essenz als **Opt-in-Render-Fähigkeit** (Entwurf §3):
zwei persistente FBO+Textur-Paare (previous/current) mit Swap pro Frame.
Ablauf im Visualizer-Render (Render-Thread, Kontext current):

```cpp
m_feedback->ensure(w, h);               // anlegen/resize (Blit-Preserve, E1)
m_feedback->beginFrame();               // current-FBO binden
/* clear */
m_feedback->drawPrevious(decay, zoom);  // Echo des Vorframes (interner Quad-Shader)
/* normalen Inhalt zeichnen */
m_feedback->endFrame(defaultFbo, w, h); // Blit auf den Screen + Swap
```

- **Resize (Entscheid E1):** das letzte Bild wird per Blit in den neuen
  previous-Buffer skaliert — Trails überleben Fenster-Resize.
- **Lifecycle:** alle GL-Objekte gehören dem Render-Thread; `destroy()` in
  `onCleanup()`/Kontextwechsel (§12-Vertrag unverändert). Ohne
  Blit-Support (exotisch) fällt `endFrame` auf einen Textur-Quad-Draw zurück.
- **Erster Nutzer:** Superscope `post.trail.*` (enabled/decay/zoom, Default
  aus — ohne Opt-in rendert alles exakt wie zuvor). Klassischer
  Blitter-Feedback-Look: Vorframe gedimmt + leicht gezoomt unter den Punkten.

## 2. OffscreenBufferPool (nur API-Gerüst)

8 benannte On-Demand-FBO-Slots je Besitzer — Semantik von AVS'
`getGlobalBuffer` (ref rlib.cpp:430): Größenwechsel verwirft den Slot,
`allocate=false` liefert nur existierende passende Buffer. Echte Nutzer
(Buffer Save, Blend-Modus „Buffer") kommen mit dem Multieffekt-Host
(Roadmap 5).

## 3. Verifikation

GL-Klassen sind nicht unit-testbar (brauchen Kontext) — Absicherung:
Suite bleibt grün (kein Verhalten ohne Opt-in), **Sichttest** (Trail-Look,
Resize, Undock, Fullscreen, Visualizer-Wechsel) + Frametime-Vergleich stehen
aus (Akzeptanzkriterien 3/5 des Entwurfs).

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — FeedbackBuffer + Pool-Gerüst + Superscope-Trail (Session 33) |
