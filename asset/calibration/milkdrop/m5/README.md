# Kalibrier-Presets M5 — Blur-Pyramide + Shader-Stufe B

> **Zweck:** isolierte Sichttests für die M5-Bausteine (Session 40): Blur-Pyramide
> (blur1–3, Ranges, Edge-Darken), eingebackene Shader-Konstanten (Md1Default/
> Md1Plus) und den Custom-Fallback. Vorher M3/M4 kalibrieren — alles hier baut
> darauf auf. Stellschrauben: `runBlurPasses`/`compositeToScreen`/`drawWarpPass`
> in `src/visualizers/MilkdropVisualizer.cpp`, Mathe in
> `include/visualizers/milkdrop/MilkdropBlur.hpp`.
>
> **Tipp:** Für Positions-/Bewegungs-Beurteilung das **Kalibrier-Raster**
> einschalten — Visualizer-Config-Panel → Gruppe „Debug" → „Kalibrier-Raster"
> (8×6 + Mittelkreuz, reines Screen-Overlay, geht nicht in den Feedback-Loop
> ein; wirkt über jedem Milkdrop-Preset).

## 01_blur1_glow.milk — Grundfunktion Blur-Term (`ret += GetBlur1`)

**Bild:** weiße Mittellinie (Mode 6) mit weichem GLOW-Halo drumherum.
**Erwartung:** die scharfe Linie bleibt scharf, darüber liegt ein deutlich
sichtbarer, weicher Lichthof (Vorframe-Blur, zieht mit dem Decay-Trail mit).
Import-Report: „Comp-Shader = MD1-Default + Blur-/Gain-Mix → exakt übersetzt".
**Abweichung:** kein Halo → Blur-Pässe laufen nicht (`activeBlurLevels`,
`runBlurPasses`); Halo versetzt/gespiegelt → UV-Orientierung der Blur-Layer;
Halo hart/kantig → Kernel-Konstanten (blurKernelH/V).

## 02_blur3_weich.milk — stärkste Stufe (`GetPixel*0.3 + GetBlur3*1.0`)

**Bild:** stark verweichzeichnetes Gesamtbild, das Original schimmert nur
schwach durch (30 %).
**Erwartung:** blur3 ist DEUTLICH weicher als blur1 aus 01 (3 Halbierungs-
stufen); große weiche Farbflächen, keine Blockartefakte/Treppen.
**Abweichung:** kaum weicher als 01 → Kette samplet falsche Zwischenstufe
(`m_blurTex`-Index 2n+1); Blöcke → Texturgrößen/Filterung (LINEAR?).

## 03_blur2_lerp.milk — Lerp-Form (`lerp(GetBlur2, GetPixel, 0.30)`)

**Bild:** 70 % Blur2 + 30 % scharfes Bild, blaue Kreis-Wave (Mode 2).
**Erwartung:** mittlere Weichheit (zwischen 01 und 02); Gesamthelligkeit
bleibt ~konstant (Koeffizienten summieren zu 1) — nichts pumpt auf.
**Abweichung:** zu hell/dunkel → Koeffizienten-Extraktion der Lerp-Form
(gain=0.3, blurAdd2=0.7) bzw. Un-Bias (max−min)/min im Blur-Layer.

## 04_blur_ranges.milk — Range-Kompression (blur1_min animiert)

**Bild:** wie 01, aber der Halo PUMPT im ~12-Sekunden-Takt (per_frame hebt
`blur1_min` zwischen 0 und 0.5).
**Erwartung:** hohes blur1_min → Halo wird dunkler/kontrastiger (untere Werte
werden weggeschnitten); niedriges min → voller Glow. Übergang weich, kein
Flackern, keine Invertierung.
**Abweichung:** kein Pumpen → blur-Vars werden nicht aus der Engine gelesen
(`pullFrameOutputs`); Invertierung/NaN-Blitzen → `computeBlurPassScales`/
`safeInverse`-Guard.

## 05_edge_darken.milk — Edge-Darken (b1ed=1.0)

**Bild:** dunkles Grundbild (Gamma 0.2), der Glow trägt das Bild.
**Erwartung:** der Glow läuft zu den BILDRÄNDERN hin sichtbar auf Schwarz aus
(edge_darken=1, nur 1. V-Pass); in der Bildmitte voller Glow.
**Abweichung:** Ränder hell → uEdge-Konstanten (nur bei Pass i==1 darken);
harte dunkle Balken → sqrt(t)-Rampe.

## 06_warp_decay_sub.milk — subtraktiver Decay (Datei-Default `ret -= 0.004`)

**Bild:** leichter Zoom-Trail (zoom=1.01), Warp-Shader = Datei-Default.
**Erwartung:** Trails laufen LINEAR auf sattes Schwarz aus — KEINE stehenden
Grau-Geister (der multiplikative 8-Bit-Decay aus M3/05 lässt Reste stehen,
der subtraktive nicht). Report: „Warp-Shader = generierter MD1-Default".
**Abweichung:** Geister bleiben → uDecaySub kommt nicht an (drawWarpPass,
baked-Pfad); alles zu schnell weg → decayMul fälschlich <1 zusätzlich aktiv.

## 07_baked_vs_live.milk — Vertrag „Konstanten sind eingebacken"

**Bild:** Mittellinie, per_frame animiert `gamma = 2 + 2*sin(time*2)`.
**Erwartung:** KEIN Helligkeits-Pumpen! Der Comp-Shader ist erkannt (baked
gamma=1.00) — genau wie im Original ignoriert MD2 die per_frame-Animation,
sobald ein Shader existiert. (Gegenprobe: dieselbe per_frame-Zeile in einem
Preset OHNE comp-Block MUSS pumpen — z. B. Zeile in m3/08 einfügen.)
**Abweichung:** pumpt → compositeToScreen nutzt fälschlich die Live-Werte
statt der gebackenen (`baked`-Auswahl).

## 08_custom_fallback.milk — Custom-Shader (seit C1: ECHT übersetzt!)

**Bild (seit Stufe C1, Session 40):** rote Wave an der **Diagonale gespiegelt**
(uv.yx) plus additiver Blur — der Custom-Comp-Shader wird jetzt per
HlslTranspiler nach GLSL übersetzt und läuft wirklich.
**Erwartung:** Import-Report: „Custom-Comp-Shader (PS2, 2 Zeilen, Blur)" +
„Comp-Shader → GLSL übersetzt (Stufe C1, GL-Kompilierung zur Laufzeit)";
Blur-Pass läuft (usesBlur-Flag). Diagonalen-Spiegelung + Glow sichtbar.
**Abweichung:** Bild wie MD1 ohne Spiegelung → GL-Kompilierung gescheitert
(`m_customGlError` im Debugger prüfen) — stiller Fallback ist das
Sollverhalten, aber hier soll der Shader kompilieren; Crash/schwarz →
feedCustomUniforms/Sampler-Bindung.
