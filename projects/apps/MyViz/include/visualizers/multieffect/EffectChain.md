# EffectChain

> **Modul:** `lumi::multieffect` · header-only, GL-frei ·
> **Seit:** Import-Phase Roadmap 5.1 (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md`

GL-freies Laufzeit-Datenmodell der Multieffekt-Kette: Baum aus `ChainNode`
(Container-Listen + Effekt-Blätter), Topologie-Spiegel des
`AvsParser::EffectNode`-Baums. Der Effekttyp steckt in der Variant-Alternative
von `ChainNode::params` — aktuell: `ListParams` (mit Blend/OnBeat/EEL-Slots),
`ClearParams`, `FadeoutParams`, `InvertParams`, `BrightnessParams`,
`FastBrightnessParams`, `BlurParams`, `MirrorParams`, `OnBeatClearParams`,
`ColorfadeParams`, `DebugBarsParams` (host-eigen), `PassthroughParams` (wächst
mit den 5.x-Schritten, Entscheid E2).

`ListParams` trägt die 14 `BlendMode`-Werte (In/Out, AVS-Reihenfolge =
Enum-Wert). Seit Batch 2 (Session 35) sind **alle 14 Modi implementiert**
(`isBlendModeImplemented()` == true durchweg) — der frühere Replace-Fallback
entfällt. `BlendMode::Buffer` nutzt zusätzlich `bufferIn/bufferOut` (Pool-Slot
0..7) + `bufferInInvert/bufferOutInvert` (AVS `ininvert/outinvert`).

## Verwendung

```cpp
using namespace lumi::multieffect;
ChainNode root;
root.params = ListParams{};
root.children.push_back({/* Fadeout, DebugBars, … */});

CompileResult r = compileChain(root);   // nach JEDER Mutation (Entscheid E4)
// r.ok == false nur bei strukturellem Schaden (Root keine Liste);
// alles andere sind pfad-präfixierte Warnungen ("root/1/0", nie Hard-Fail)
```

## Verträge

- **Kette ist editierbar (E5):** GUI-Mutationen unter `renderMutex()` des
  Widgets; danach `compileChain()` — dort landet auch das
  Set-Render-Mode-Ausrollen mit Re-Propagation (E4, ab dem SRM-Batch).
- **`children` nur bei Listen** — Kinder an Blättern werden gewarnt und beim
  Rendern ignoriert (kein destruktives Kappen).
- Compile-Pass **normalisiert**: leere `displayName` ← Typname
  (`effectTypeName`), Parameter-Clamps (z. B. `fadeLen` 0..92).
- `nodeCount()` zählt ohne Root (AvsParser-`effectCount`-Regel).
- Unbekannte Effekte = `PassthroughParams{sourceId, note}` — konserviert,
  rendert nichts (AVS-Philosophie: nie hart abbrechen).

## Tests

`tests/unit/UnitTests/test_EffectChain.cpp` — Baumaufbau, Compile-Pass
(Root-Regel, displayName, Clamps, Warnungspfade), `nodeCount`, Passthrough.
