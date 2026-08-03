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
  Out-Blend → Parent. Alle 14 Blend-Modi als ein Shader (`uMode`) — seit
  Batch 2 auch Subtractive 1-2/2-1, Every-other-line/-pixel, XOR und Buffer
  (Pool-Depth als Alpha, `bufferIn/out` + Invert). OnBeat-Aktivierung + EEL-Listen-Slots
  (`enabled/clear/beat/alphain/alphaout`) über `ScriptSlotHost` mit geteiltem
  `ScriptContext`.
- **Effekt-Portierungen (5.3)** sind 1:1 aus `ref/vis_avs` (r_bright, r_fastbright,
  r_blur, r_mirror, r_nfclr, r_colorfade) — Konvention `0x00RRGGBB` = GLSL-RGB.
- **§5.2-Effekte (Shader):** **Mosaic** (`r_mosaic` — `quality`×`quality`-Block­
  raster, OnBeat→`quality2` mit Ease-Back, Blend Replace/Additiv/50-50) ·
  **Grain** (`r_grain` — Zufalls-Verdunkelung gegateter Pixel, `amount`/static/
  Blend, Hash-Noise) · **Scatter** (`r_scat` — per-Pixel-Zufallsversatz ±4 px) ·
  **Interferences** (`r_interf` — ≤8 gedrehte Kopien, per-Frame akkumulierende
  Rotation + OnBeat-Morph zum *2-Satz, optional RGB-Kanaltrennung; LeafRuntime-
  State `interfRotation/interfStatus`) · **Water** (`r_water` — Farb-Ripple:
  Nachbar-Mittel des aktuellen Frames minus Vorframe; persistenter Per-Node-
  `waterLast`-FBO im LeafRuntime, Blit-Back nach jedem Pass) · **Bump**
  (`r_bump` — Bump-Lighting aus Luminanz-Gradient; Lichtposition per EEL
  init/frame/beat → x,y über `ScriptSlotHost`; Depth-OnBeat-Ease wie Mosaic) ·
  **Water Bump** (`r_waterbump` — Höhenfeld-Wellensimulation: RGBA16F-Ping-Pong
  (.r aktuell/.g vorher), 8-Nachbar-Propagation + Dämpfung, Beat-Tropfen, dann
  Refraktion des Bildes über den Höhengradienten; `displaceScale` sichttest-kalibriert).
- **§5.2-Renderer (Content):** **Starfield** (`r_stars` — CPU-Sternfeld, Z→0
  pro Frame + Projektion `x/z`, Helligkeit ~(1−z)·speed, WarpSpeed-OnBeat-Ease;
  additiv über den `ScopeRenderer` gezeichnet; per-Node `stars`-Vektor im LeafRuntime) ·
  **Timescope** (`r_timescope` — scrollendes Spektrogramm: pro Frame eine
  Spektrum-Spalte an vorrückender x-Position, per Scissor + Spektrum-Textur
  in-place ins persistente Bild; Blend Replace/Additiv/50-50) · **Dot Grid**
  (`r_dotgrid` — scrollendes Farb-Punktraster, zyklende Farbtabelle) · **Dot
  Plane** (`r_dotpln` — rotierende Audio-Punktebene, Höhe aus Spektrum, 5-Stop-
  Gradient) · **Dot Fountain** (`r_dotfnt` — 3D-Partikel-Fontäne, per-Node-
  `fountain`-Vektor). Die 3D-Projektions-/Physik-Skalen sind sichttest-kalibriert.
- **§5.2-Builtin-APEs (ein `uType`-Shader):** **Channel Shift** (RGB-Permutation,
  OnBeat-Zufall) · **Color Reduction** (Quantisierung auf 2^levels) · **Multiplier**
  (Skalierung x8..x0.125). Dispatch per `apeId`-String (Parser `decodeApe`,
  Translator `mapApe`; Channel-Shift-`mode` = Windows-IDC → 0..5 gemappt).
- **§5.2-Delays:** **Video Delay** (`r_videodelay`, APE Holden04 — Bild von N
  Frames zurück; per-Node FBO-Ringpuffer im LeafRuntime) · **Multi Delay**
  (`r_multidelay`, APE Holden05 — **6 host-globale** FBO-Ringpuffer `m_mdRing[6]`,
  effektübergreifend geteilt: Input-Knoten füllen, Output-Knoten lesen das
  verzögerte Bild). `delay` auf 128 gedeckelt; `useBeats` grob ~30 Frames/Beat.
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
- **Neutrale Vorbelegungen der Skript-Träger gehören VOR den Frame-Slot, nicht
  in die Punktschleife.** Zweimal gemessen an der Referenz: `sizex`/`sizey`
  (S51) und `red`/`green`/`blue` (S52) standen je nach Punkt bzw. nach dem
  Frame-Slot und löschten damit genau das, was das Skript gerade gesetzt hatte.
  AVS belegt diese Variablen **einmal je Frame** vor; ein Point-Slot, der sie
  setzt, überschreibt sie weiterhin je Punkt. Beleg jeweils als Paar-Sonde
  (`asset/calibration/avs/probes/5_vars/`, `6_alloy/`): dieselbe Größe/Farbe
  einmal im Point-, einmal im Frame-Slot — stimmen die *Referenz*bilder, muss
  der Frame-Wert die Punktschleife überleben. Wirkung des Farb-Falls: „Alien
  Alloy" zeichnete durchgehend weiße Sprites und lief nach Schwarz
  (0,647 → **0,008**).

## Milkdrop-Meganode (N1/N2, Session 41 — Entscheide E1/E2)

- **`runMilkdropNode`**: rendert die komplette MilkDrop-Pipeline
  ([MilkdropVisualizer](MilkdropVisualizer.md) als Engine, per `nodeId` im
  LeafRuntime) in den aktiven Chain-Buffer. Vertrag: der Kern erfasst das beim
  Frame-Start gebundene Draw-FBO als Composite-Ziel — der Host bindet vor
  `render()` `bindActive()` und stellt danach FBO/Viewport wieder her.
- **Revision-Vertrag:** `MilkdropNodeParams.revision` bumpt bei jedem Edit
  (Panel/Import); nur dann übernimmt der Render-Thread den Preset-State
  (`applyPresetState` — Skript-Compile + Shader-Transpile, GL-frei).
- **Audio:** Host-Kanal-Kopien werden interleaved an den Kern durchgereicht
  (`feedMilkAudio` → `updateAudioStereo`).
- **Laden:** `loadMilkFile` (.milk) / `loadMilkDocument` (milkdrop-.lvfx,
  M6.1-Format) installieren eine Wurzel-Liste mit einem Milkdrop-Node;
  Report-Parität über eine GL-freie Probe-Instanz. Speichern läuft immer über
  den Chain-Serializer (Key `"milkdrop"`, Preset eingebettet).
- **Feedback-Erhalt beim .milk→.milk-Wechsel (S63):** Ist die Wurzel bereits
  ein einzelner Milkdrop-Node, tauscht `replaceMilkdropPresetInPlace` nur die
  Params (Revision++) statt `setChain` + Runtime-Reset — gleiche Node-Id ⇒
  gleiche Runtime ⇒ der FeedbackBuffer überlebt. Original-Semantik: ein Preset
  erbt das Bild des Vorgängers (der Hauptpuffer wird beim Wechsel nie
  gelöscht); Verstärker-Presets ohne eigene Energiequelle blieben sonst
  schwarz (Ground-Truth-Befund via MilkdropRef, `tools/MilkdropRef`).
- **Puffer-Wechsel-Schalter (S66):** Das Erbe ist je Node steuerbar —
  `MilkdropNodeParams.pufferWechsel` ∈ AppEinstellung/Behalten/Loeschen/
  Fading/Ausblenden (+ `pufferFading` 0..1 als EINMALIGER Mix-Anteil,
  `pufferAusblendSek` 0.1–60 als Zeit-Ausblendung; Panel-Edit OHNE
  Revision-Bump, der Schalter wirkt erst beim nächsten Tausch).
  `replaceMilkdropPresetInPlace` löst den effektiven Erbe-Anteil auf
  (AppEinstellung → App-Default via `setMilkdropPufferWechselDefault`, vom
  MainWindow vor jedem loadMilk* aus QSettings `milkdrop/pufferWechsel` +
  `pufferFadingProzent` + `pufferAusblendSek` gesetzt — Muster Render-Scale
  S47) und bumpt `wechselZaehler`; `runMilkdropNode` meldet bei neuem
  Zählerstand `wechselErbe` via `requestFeedbackErbe()` an (Ausblenden:
  Erbe 1.0 + `requestFeedbackAusblenden(wechselAusblendSek)` — der Kern
  dämpft das Echo je Frame nach dem Warp, bis das Erbe gestorben ist).
  Die Wechsel-Felder sind transient (nicht im Chain-Dokument); persistiert
  werden `pufferWechsel`/`pufferFading`/`pufferAusblendSek`
  (fehlend ⇒ AppEinstellung + 0.5 + 2.0).
- GL-Freigabe des Kerns via `cleanup()` in `resetRuntimes` (Kontext current).

## Absicherung

Datenmodell: `test_EffectChain.cpp` (GL-frei). GL-Pfad: Sichttest
(Kette rendert, Trails, Resize/Undock/Fullscreen) — GL-Klassen sind nicht
unit-testbar (kein Kontext im Test, R4-Erfahrung).
Milkdrop-Node: `test_ChainSerializer.cpp` (Suite „MilkdropNode" — Roundtrip,
Clamps, `loadMilkFile`→Node).

