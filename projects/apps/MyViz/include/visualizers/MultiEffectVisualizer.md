# MultiEffectVisualizer

> **Modul:** App-Visualizer (`multieffect`-ID, Kategorie `effects`) ·
> **Seit:** Import-Phase Roadmap 5.1 (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md`

Multieffekt-Host: rendert einen [`EffectChain`](multieffect/EffectChain.md)-Baum
nach dem AVS-Render-Modell (Analyse §5.1). Import-Ziel für .avs-Presets
(Übersetzer folgt in Schritt 5.5).

## Render-Modell (Stand 5.3)

- **Arbeitsfläche:** persistentes Ping-Pong-FBO-Paar in physischen Pixeln
  (Größe aus dem GL-Viewport, lazy in `onRender`). Frame-persistent —
  AVS-`framebuffer`-Semantik (Fadeout zieht Trails über Frames).
- **Render-Effekte** (Clear, OnBeat Clear, DebugBars) zeichnen in-place auf den
  aktuellen Puffer; **Transform-Effekte** (Fadeout, Invert, Brightness, Fast
  Brightness, Blur, Mirror, Colorfade) lesen aktuell → schreiben Partner → Swap
  (`transformPass`).
- **Verschachtelung (5.2):** Nicht-Root-Listen halten einen persistenten
  `thisfb` (eigenes Paar, keyed by `nodeId`): Parent → In-Blend → Kinder →
  Out-Blend → Parent. 14 Blend-Modi als ein Shader (`uMode`); Batch 1 (E3)
  echt, Rest Fallback Replace. OnBeat-Aktivierung + EEL-Listen-Slots
  (`enabled/clear/beat/alphain/alphaout`) über `ScriptSlotHost` mit geteiltem
  `ScriptContext`.
- **Effekt-Portierungen (5.3)** sind 1:1 aus `ref/vis_avs` (r_bright, r_fastbright,
  r_blur, r_mirror, r_nfclr, r_colorfade) — Konvention `0x00RRGGBB` = GLSL-RGB.
- **Skript-Effekte (5.4):** Color Modifier (`ScriptLutModule`→256-LUT-Shader),
  Movement/Dynamic Movement (`ScriptGridModule`→per-Frame-Warp-Mesh), Custom BPM
  (Beat-Mutation), Blitter/Roto Feedback (Roto/Zoom-Feedback-Shader), Buffer Save
  (`OffscreenBufferPool`, 8 Slots), **SuperScope** (`SuperscopeModule` +
  [`ScopeRenderer`](render/ScopeRenderer.md), additiv gezeichnet).
- **Grenze SuperScope:** nutzt (noch) einen **privaten** ScriptContext des
  `SuperscopeModule` — reg/q sind nicht mit den übrigen Effekten geteilt (Folgeschritt:
  `SuperscopeModule` einen geteilten Context annehmen lassen).
- **Frame-Ende:** Blit aktueller Root-Puffer → Fenster-Framebuffer (explizite
  READ/DRAW-Binds, Muster `FeedbackBuffer::endFrame`).
- **Grenzen:** Resize verwirft Pufferinhalt (Blit-Politik = späterer Feinschliff);
  `DebugBars` ist host-eigener Sichttest-Effekt bis der echte Scope-Renderer da
  ist (5.4, E6); Brightness-`blend/blendavg` noch nicht (nur Replace).

## Verträge

- **GL nur im Render-Thread** (§12): FBOs/Shader in `onInitialize`/`onRender`
  auf-, in `onCleanup` abgebaut; kein Logging von dort.
- **Ketten-Zugriff:** `chain()`/`setChain()` — GUI-Schreibzugriffe unter
  `renderMutex()` des Widgets, danach `recompileChain()` (E4/E5).
- Default-Kette (Konstruktor): Fadeout(12) + DebugBars — sichtbarer Beweis,
  dass der Ketten-Walk läuft (Sichttest 5.1).

## Absicherung

Datenmodell: `test_EffectChain.cpp` (GL-frei). GL-Pfad: Sichttest
(Kette rendert, Trails, Resize/Undock/Fullscreen) — GL-Klassen sind nicht
unit-testbar (kein Kontext im Test, R4-Erfahrung).
