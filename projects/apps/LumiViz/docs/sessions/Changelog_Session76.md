# Changelog Session 76 (2026-08-12)

> **Die frische MilkDrop-Pipeline startet jetzt wirklich leer.** Vorher waberte
> sie ohne erkennbaren Grund — das waren die MilkDrop-Defaults selbst (Warp-
> Ripple, Decay-Trail, Basis-Welle, Motion-Vector-Raster). Jetzt beginnt sie
> schwarz, und jeden Baustein schaltet man bewusst zu. Nebenbefund mit Tiefe:
> frisch angelegte GL-Puffer waren nie gecleart, und „New Effect Chain" leerte
> gar nichts.

## Teil 1 — Blanke Pipeline, schaltbare Bausteine

Der Palette-Eintrag „Milkdrop (Preset-Pipeline)" liefert eine leere Fläche:
`warp=0`, `decay=1`, Basis-Welle aus, Motion-Vectors unsichtbar, keine
Rauschsaat. **Importierte .milk-Presets und gespeicherte Ketten bleiben
unverändert** — die MilkDrop-Defaults gelten dort weiter.

Neu schaltbar (alle mit Auge im Baum **und** Checkbox im Panel, beidseitig
synchron — SSOT ist der Preset-Zustand):

- **Basis-Waveform** (`basicWaveEnabled`) — MilkDrop kennt nur „Alpha 0";
  jetzt ein ehrlicher Schalter. Das Waves-Label zeigt den Zustand.
- **Sprites** (`enabled` je Sprite) — wie Waves/Shapes, inkl. „(aus)"-Suffix
  und Zähler „Sprites (N aktiv)".
- **Kaltstart-Rauschsaat** (`kaltstartSaat`) — je Preset abschaltbar
  (aus = definiert schwarzer Start). Grundstein der S75-Idee „Preset-Seed";
  die Seed-Wahl selbst bleibt offen.

Dazu die Sprite-Bedienung angeglichen: der Host zeigt kompakt nur
„Sprite X aktiv" + Code, die Einzel-Sicht alle Startwerte; die überbreiten
Panels sind auf kurze Labels + Tooltips umgestellt.

## Teil 2 — Saubere Puffer

Der blanke Default (`decay=1` = verlustfreie Kopie) machte sichtbar, was
vorher der Decay stillschweigend wegblendete:

- **Frisch alloziertes VRAM ist undefiniert.** `FeedbackBuffer::ensure()` und
  `OffscreenBufferPool::get()` clearen neue FBOs jetzt schwarz — Frame 0
  zeigt keine Treiber-Restbilder mehr, und ein nie beschriebener
  Buffer-Save-Slot liest 0 (AVS-Vertrag).
- **„New Effect Chain" leert jetzt wirklich:** `setChain()` stößt den
  Runtime-Reset an (Knoten-Runtimes, Buffer-Save-Slots, Multi-Delay-Ringe,
  Erstframe-Clear). Knoten-**Einfügen** räumt bewusst weiterhin nichts
  Globales weg — andere Module behalten ihre Puffer.

## Teil 3 — Motion-Vector-Overlay (Analyse)

Die „weißen Artefakte" der frischen Pipeline waren MilkDrops Motion-Vectors
(12×9-Raster, `mv_a=1` als Original-Default). Neu gibt es — wie beim
Kalibrier-Raster — den Knoten-Schalter **„Motion-Vector-Overlay"** (Default
aus): er zeichnet das Warp-Flussfeld erzwungen sichtbar **nach** dem Composite,
geht also nicht in den Feedback-Loop ein. Die `mv*`-Werte des Presets bleiben
unangetastet Gestaltungsmittel.

## Verifikation

`LumiViz.UnitTests` **599/599 grün, 0 Skips** — neue Roundtrip-Tests für alle
vier Felder, Migrations-Checks (fehlendes Feld = bisheriges Verhalten),
Feld-Inventar 782 Felder / FieldDocs 0 Lücken. Sichttest steht aus.
