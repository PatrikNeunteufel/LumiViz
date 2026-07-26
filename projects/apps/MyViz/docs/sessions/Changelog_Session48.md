# Changelog Session 48 (2026-07-25/26)

## Lights-Module (Etappen 1–3 komplett, `Lights_Module_Entwurf.md` = SSOT)

- **Bloom** (`bloom`): Downsample → 25-Tap-Gauß separabel → additives
  Composite, optional Threshold/Vignette. **Default `post=true`**: der Glow
  entsteht erst beim Present (erster aktivierter Knoten gewinnt, wie Render
  Scale) — in-chain akkumulierte der additive Anteil in Feedback-Ketten bis
  Weiß (Sichttest-Befund); In-Chain-Modus bleibt als Schweif-Effekt wählbar.
- **3D Camera** (`camera3d`): Frame-Zustand wie Set Render Mode; die
  EEL-Slots überschreiben px..pz/tx..tz/fov/roll/fogstart/fogend =
  **erste dynamische Modulparameter**. Fallback-Kamera bei +1/tan(fov/2)
  mit Blick auf den Ursprung — three.js-Konvention (x+ rechts, y+ oben,
  z+ zum Betrachter); das Projektions-Gate fing die gespiegelte x-Achse
  des Erstwurfs.
- **SuperScope 3D** (`superScope3d`): EEL-Quartett schreibt x/y/z/size/rgb/
  skip in Weltkoordinaten; additive Soft-Sprites (exp(-r²k)-Falloff, kein
  Bild-Asset), 1/w-Attenuation, Near-Clip, Fog (dämpfend); Linien-Modus.
- **Terrain 3D** (`terrain3d`): Lights-Rezept — Spektrum-Ringe SETZEN die
  Höhe, Feder-Relax (drag = 1−dt·5), flatten; Modi `rings/relax/flatten`
  skriptbar (Slots laufen VOR der Sim, Director-Muster); Höhen-Grid =
  megabuf(gy*res+gx); dunkles Mesh + additive Gitterpunkt-Sprites.
- **Glow Orbs** (`glowOrbs`): Ellipsoide 16×12 mit Zwei-Farb-Verlauf +
  flash (addRGB-Blitz), Achsen-Squash, Fog; Halo-Billboard additiv mit
  Depth-Test ohne Write (Rim-Glow); Point-Slot je Orb.
- **Gemeinsames Depth-RT** (Entwurfs-Entscheid 1): eine Depth-Textur je
  Frame ans jeweils aktive FBO — übersteht das Farb-Ping-Pong; Terrain
  clippt Orbs zu Halbkugeln.
- **UI**: Panel-Gruppen „— Post —" / „— 3D —"; Vorlagen-Dropdowns
  (7 Scope-3D- + 6 Kamera-Templates, Muster SuperScope-Figuren).
- **Assets**: `lights_demo2.lvfx` (Referenzwerte aus github.com/C4RL05/
  Lights: fov 30, Fog schwarz, Bloom σ8/25 Taps **Intensity 1,3**, Vignette
  1−r²·0,3) · `lights_demo3.lvfx` (Kameraflug über das Terrain hinter der
  Wunderkerze; Kamera schreibt q1..q3, die Funken lesen sie = erste
  Modul-Kopplung über den geteilten ScriptContext; Orbs halb versunken).

## AVS-Treue: Matrix-Befunde S47 (6/6 bearbeitet, Matrix 28/41 → 34/41)

- **Simple** 0,46→0,000: Neuschrieb nach r_simple — `mode` 0–5 (solid/line/
  dot × analyzer/scope), rohe visdata-Bytes, solid = Spaltenlinien,
  char-Arithmetik im Center-Mix.
- **Interleave** 0,66→0,000: Qt überträgt QPoint-Uniforms als **float** —
  auf `ivec2` blieb das Uniform (0,0), der Effekt war seit je ein
  Passthrough; dazu Zeilenphase (+1) und GL-y-Flip.
- **Timescope** 0,24→0,000: rohe Spektrum-Bytes (immer Kanal links — das
  Original berechnet which_ch, nutzt es aber nie), Farbe = color·Byte/256.
- **Bass Spin** 0,14→0,006: L/R-Spektrum-Bytes je Kanal, `last_a` als EIN
  Member über beide Kanäle, Modus 1 = gefüllte Dreiecke (neuer
  Flat-Fill-Pfad `drawFlatTriangles`, my_triangle-Ersatz).
- **Blitter Feedback** 0,72→0,003: AVS-Felder (scale/scale2/onBeat/blend/
  subpixel), fpos-Ease ±3, Zoom 64/(f_val+32), blitter_out-Fensterpfad,
  subpixel-Bilinear (Nearest kaskadierte zum Mosaik).
- **Roto Blitter** 0,37→0,12 ◐: Original dreht jede Frame um KONSTANTES
  theta (Rotation akkumuliert übers Feedback) — unsere Winkel-Akkumulation
  war das Pixel-Rauschen. Jetzt 16.16-Fixpunkt-INTs im Shader, BLEND4_16
  mit trunkierenden Mischstufen, Per-Pixel-Wrap. Rest: Verdacht
  Scope-y-0,5-Kleinrest, vom Feedback multipliziert.

## EEL-Kern (Whacko-V-Runde: Neon Coaster / Tie Tunnel, AvsRef-Proben)

- **Nicht-ASCII-Statements**: Original frisst ab Byte ≥0x80 das restliche
  Statement als toten Identifier (No-op, Slot LEBT) — UnConeDs
  `¤ 1st - Line;`-Pseudo-Kommentare. Unser S46-Strippen ließ die Wörter
  stehen → Parse-Fehler → Slot still tot. Lexer schluckt jetzt bis `;`.
- **`%`-Operator** (nseel_asm_mod): Operanden FPU-gerundet, Divisor
  max(b,1), Rest **UNSIGNED 32-bit** — (0−7)%3 = 0 (AvsRef-Linien-Probe;
  zwei S44-Goldens korrigiert).
- **Dot Plane**: Neuschrieb nach r_dotpln — scrollendes 64×64-Physik-Grid,
  Injektionszeile trägt Höhe UND Farbe (color_tab[Byte>>2], wandert mit),
  matrix.cpp-Pipeline, r-abhängige Zeichenreihenfolge. Matrix 0,000/0,000.
- Ergebnisse (5 s/300 Frames): **Neon Coaster OK (0,001)** · Tie Tunnel SSC
  OK groß (0,026), klein 0,051 · Tie Tunnel DM 0,29 — Täter per Bisektion
  isoliert: **r_dmove-Warp braucht den Fixpunkt-Umbau** (Roto-Muster).
- P-Presets: P1 dont_make_a_mess jetzt OK; P3/P4/P5 → eigene Runde.

## Infrastruktur

- `MultiEffectVisualizer::debugGrabRootSurface()` — GL-Gates lesen die
  Root-Surface vor dem Present (Basis aller neuen Smoke-Tests).
- `LuaScriptEngine::megabufValue/setMegabufValue` (Host-Spiegelung des
  Terrain-Grids) · `visSpectrum(ch)/visWaveform(ch)` (benannte
  Raw-Byte-Accessoren, eine Wahrheit für AVS-treue Effekte).
- Fractal Zoomer: eigenes `fracRot` (teilte rotoAngle mit dem Roto).

## Verifikation

MyViz.UnitTests **426/426 grün, 0 Skips** (neu: Bloom-GL-Smoke ×5, Scope3D-
Gates inkl. Frame-vor-Beat-Beweis + Fog ×2, Terrain/Orb-Depth, Demo-Loader
v2+v3, Roundtrips ×6; 2 mod-Goldens auf bewiesene Semantik korrigiert);
Builds VS-Debug/VS-Testing (/WX)/Ninja-Clang grün; Zwillinge **65/65**
(refreezed); Matrix **34/41**; Whacko-V-Messungen s. o.

## Bekannt / offen

r_dmove-Fixpunkt (Tie DM, 3×s2, P-Anteile) · P3/P4/P5-Bisektion · Roto-Rest ·
milde Matrix-Reste (water, water_bump-klein, interferences, dot_grid) ·
Lights-Kür (Vox-Sprite-Sequenz, energyTrend/BASS-Lookahead).
