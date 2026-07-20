# ScopeRenderer

> **Modul:** `lumi::render` · `include/visualizers/render/ScopeRenderer.hpp` +
> `src/visualizers/ScopeRenderer.cpp` · **Seit:** Import-Phase Roadmap 5.4d (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md` (E6)

Wiederverwendbare GL-Zeichenklasse für Scope-Punktwolken: nimmt einen
`std::vector<lumi::modules::SuperscopePoint>` (Position NDC, RGBA, `skip`) und
zeichnet ihn als **Dots** (`GL_POINTS` + weicher Glow-Fragment), **Thin Lines**
(`GL_LINE_STRIP`) oder **Thick Lines** (Triangle-Strip mit Normalen-Dicke). Die
Shader/Technik sind **1:1** aus `SuperscopeVisualizer` extrahiert.

## Verwendung

```cpp
lumi::render::ScopeRenderer renderer;
renderer.ensure();                       // GL-Objekte (Render-Thread)
ScopeRenderer::Params p;
p.mode = SuperscopeRenderMode::Lines;
p.lineWidth = 2.0f;
renderer.draw(points, p);                // zeichnet mit aktueller Blend-State
renderer.destroy();                      // onCleanup / Kontextwechsel
```

## Verträge

- **GL nur im Render-Thread:** `ensure()`/`draw()`/`destroy()` bei aktuellem Kontext.
- **Blend-State gehört dem Aufrufer** — der Renderer zeichnet mit dem, was gesetzt
  ist (wie der Superscope sein Blending extern führt). Der Multieffekt-Host setzt
  z.B. additiv (`GL_SRC_ALPHA, GL_ONE`) um den Scope-Draw.
- **Mode-Wahl:** `Lines` mit `lineWidth>1` rendert als Thick-Triangle-Strip, sonst
  als `GL_LINE_STRIP` (identisch zum Superscope).

## Nutzer & Ausblick

- **Erster Nutzer:** [MultiEffectVisualizer](../MultiEffectVisualizer.md) —
  AVS-SuperScope-Effekt (Punkte aus `SuperscopeModule`, gezeichnet hier).
- **Offen (E6a-Rest):** `SuperscopeVisualizer` auf diese Klasse umziehen (dedup,
  „zwei Nutzer") — rein mechanisch, bewusst separat gehalten, um den bestehenden
  Visualizer stabil zu lassen (Migrationssuite ist das Netz).

## Absicherung

GL-Klasse → kein Unit-Test (kein Kontext); Sichttest über den Host-SuperScope.
Die `SuperscopeModule`-Punktberechnung ist separat getestet; die
Superscope-Migrationssuite bleibt grün (Visualizer unangetastet).
