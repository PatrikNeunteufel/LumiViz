# Lights-Module — Entwurf (Session 47, Stand 2026-07-25)

**Ziel:** Die Bildsprache von HelloEnjoy „Lights" (Three.js, 2011 —
Quellcode analysiert: https://github.com/C4RL05/Lights) als **native
LumiViz-Ketten-Module** verfügbar machen. Kein AVS-Gegenstück — Vorbild für
native Module ist Render Scale (S47). Demo mit Bordmitteln:
`asset/effectchain/lights_demo.avs|.lvfx` (zeigt die Lücken: kein Bloom,
keine Soft-Sprites, Hand-Projektion in jedem Skript).

**Quellcode-Befunde (Kurzreferenz):** Terrain = 66×66-Heightmap-Grid,
Spektrum als radiale Ringe (`Höhe = spectrum[dist(x,y,Zentrum)]`),
Feder-Relaxation; Darstellung dunkles Mesh + additive Punkt-Sprites
(size 20–32) mit Vertex-Farben. Orbs = Low-Poly-Kugeln (16×12) mit
Zwei-Farb-Vertex-Verlauf, Shader `vColor×multiply + addRGB` (Beat-Flash),
Fog, Plasma-Halo-Billboards. Strahl = Sprite-Sequenz (bengalSeq). Sync =
vorgebackene VolumeData + Song-Phasen (Director). Post = Bloom (Downsample
512² → separierbarer 25-Tap-Gauß → additiv zurück, KEIN Threshold) +
Vignette (1−r²·0,3).

**Daten-Grundsatz (Entscheid Patrik):** KEINE vorgebackenen Song-Daten.
Stattdessen: BeatEstimator-**Prädiktion** (existiert, `map.beat.predict`),
**BASS-Lookahead-Analyse** (Stream einige 100 ms vorausdekodieren → FFT/
Energie der nahen Zukunft, live), Live-Phasenschätzung (Energie-Trends als
Skript-Variablen `energyTrend`/`dropPredict`).

---

## Modul 1: Bloom (Post-Process)

**✅ Umgesetzt S48** (inkl. Post-Entscheid nach Sichttest-Befund).

- **Typ-Key** `bloom`, Params: `int downsample = 2` (RT = intern/2^n,
  Referenz nutzt fix 512²), `int radius = 8` (buildKernel-σ, 25 Taps),
  `float intensity = 1.0`, `float threshold = 0.0` (0 = Referenz),
  `bool vignette = false`, `float vignetteStrength = 0.3`,
  `bool post = true`.
- **Render:** aktuelle Surface → Downsample-RT → Gauß H → Gauß V →
  additives Composite (Quad-Draw, `min(a+b,1)`); Vignette als
  Abschluss-Multiplikation. Zwei kleine RTs im LeafRuntime (Größe =
  intern/2^n, bei Resize neu).
- **Post-Entscheid (S48-Sichttest):** Default `post = true` — der Glow wird
  erst beim **Present** in die Anzeige gemischt (erster aktivierter Knoten
  gewinnt, wie Render Scale), die Chain-Surface bleibt unberührt. Grund:
  in-chain composited akkumulierte der additive Glow in Feedback-Ketten
  (Fadeout statt Clear) über die Rückkopplung — unter der Kipp-Schwelle
  unsichtbar, darüber Weiß. Genau wie die Referenz: Lights bloomt NACH dem
  Szenen-Render. `post = false` (Composite auf die Surface) bleibt als
  bewusster Glow-Schweif-Modus wählbar.
- **Einbau:** Variant + Serializer + Panel (eigene Gruppe „— Post —"),
  `runBloom()`/`ensureBloomGlow()` in MultiEffectVisualizer; Shader:
  kBloomDown/kBloomGauss (uDir)/kBloomComp (Vignette im Composite).
  Kein Skript-Slot.
- **Gates:** Serializer-Roundtrip (`test_ChainSerializer`); GL-Smoke
  `test_BloomGlSmoke` (echte Chain offscreen via `debugGrabRootSurface()`-
  Hook): Nachbarn > 0, Peak bleibt, Summe ~2× additiv, Threshold 1 = aus,
  post=true lässt die Surface unberührt.

## Modul 2: 3D Camera (Frame-Zustand, Vorbild Set Render Mode)

**✅ Umgesetzt S48.**

- **Typ-Key** `camera3d`, Params: Position (`px,py,pz`), Ziel (`tx,ty,tz`),
  `fov = 30`, `roll = 0` (Grad), `fogStart/fogEnd/fogColor` (Fog aktiv nur
  wenn end > start), EEL-Slots init/frame/beat (dürfen `px..pz, tx..tz,
  fov, roll, fogstart, fogend` überschreiben — erster Fall von
  „dynamische Modulparameter"; Parameterwerte seeden beim (Re-)Compile,
  Panel-Änderung kompiliert neu; Slot-Ordnung Frame VOR Beat).
- **Wirkung:** setzt beim Ketten-Walk den 3D-Zustand (`m_camera3d` analog
  `m_renderMode`, Reset je Frame auf die Fallback-Kamera) — alle FOLGENDEN
  3D-Module (3D-Scope, Terrain, Orbs) nutzen ihn. View-/Proj-Matrix einmal
  je Knoten berechnen (aktuell im 3D-Scope).
- **Konventions-Entscheid (S48, per Gate gefunden):** Fallback-Kamera bei
  `z = +1/tan(fov/2)` (= +3,732 bei 30°) mit Blick auf den Ursprung —
  three.js-Konvention wie die Lights-Referenz: **x+ rechts, y+ oben,
  z+ zum Betrachter**; x/y in [−1,1] bei z=0 füllen das Bild vertikal.
  (Erstentwurf −z spiegelte die x-Achse.) Damit sind 3D-Module ohne
  camera3d-Knoten allein lauffähig (offener Entscheid 2 ✅).

## Modul 3: 3D-SuperScope (Soft-Sprite-Punktwolke)

**✅ Umgesetzt S48.**

- **Typ-Key** `superScope3d`. Wie SuperScope (EEL-Quartett, `n`, `v`, `b`,
  `i`), ABER: Point-Code schreibt `x,y,z` (Welt) + optional `size`
  (Welt-Einheit) + `red/green/blue` + `skip`; Modul transformiert mit
  m_camera3d, projiziert, clippt (near 0,05), Größen-Attenuation
  `halfNdc = size·proj/w`, Fog aus Kamera (bei additiven Sprites reine
  Dämpfung — fogColor beizumischen würde je Sprite Nebel ADDIEREN).
- **Rendering:** Punkt-Sprites als CPU-gefüllte Quads (6 Vertices je Punkt,
  dynamischer VBO wie das Warp-Mesh) mit **eingebautem radialem Falloff**
  im Fragment-Shader (KEIN Bild-Asset; `exp(-r²·k) − exp(-k)`-Profil ≈
  dot.png, Rand läuft exakt auf 0 aus; `falloff`-Param = k), additiv
  (GL_ONE/GL_ONE); Modus `Lines` projiziert und zeichnet wie SuperScope
  über den ScopeRenderer (skip/Near-Clip brechen den Linienzug).
- **Skript-Konvention:** AVS-Raum-Regel gilt am Modulrand NICHT (reines
  LumiViz-Modul): **x+ rechts, y+ oben, z+ zum Betrachter** (s. Modul 2).
  `v` aus visdata (Byte/128−1, Kanal/Spektrum-Params), visdata erreicht den
  Erst-Compile (S47-Regel: feedAudio vor dem Init-Lauf).
- **Gates (`test_Scope3DGlSmoke`, alle grün):** Punkt (0,0,0) → Bildmitte +
  Ecken schwarz · Size-Attenuation ~1/Tiefe (Energie ×0,42 bei 3,73→5,73) ·
  Kamera-Beat-Slot schwenkt das Bild (beweist dynamische Modulparameter UND
  Frame-VOR-Beat: der Beat-Wert gewinnt). Serializer-Roundtrips ×2.

## Modul 4: Heightfield-Terrain

**✅ Umgesetzt S48** (Sim: Ring-Injektion + Feder-Relax `v = v·0,92 +
(h0−h)·(|h|+0,1)·dt·relax` + flatten; Basis = zwei Sinus-Oktaven, fester
Seed; megabuf-Spiegelung nur wenn die Slots megabuf erwähnen; Point-Slot
färbt Gitterpunkte mit `i/gx/gy/h`; Gates: Terrain-Sichtbarkeit unten/oben
+ Roundtrip).

- **Typ-Key** `terrain3d`, Params: `resolution = 64`, `extent` (Weltgröße),
  `baseAmp` (prozeduraler Noise-Seed, deterministisch aus festem Seed —
  KEIN Bild-Asset), Modus-Mix skriptbar: `spectrumRings` (Höhe +=
  spectrum[dist·k]·amp — Original-Rezept), `relax` (Feder zurück zur
  Basis: v = v·drag + (h0−h)·|h|·dt), `flatten`; EEL-Slots init/frame/beat
  mit Zugriff auf `megabuf` = Höhen-Grid (Index x*res+y) für freie Formung.
- **Darstellung:** (a) dunkles Mesh (indexierte Quads, flat/Fog, Farbe
  Param), (b) additive Soft-Sprites an Gitterpunkten (Sprite-Shader von
  Modul 3 wiederverwenden), Punktfarbe: Palette über Höhe/Distanz ODER
  `red/green/blue` im Point-Slot (je Gitterpunkt, i=Index).
- **Zustand** im LeafRuntime (Höhen/Velocities, VBOs), Reset bei
  resolution-Änderung.

## Modul 5: Glow-Orbs (auch oval)

**✅ Umgesetzt S48** (Ellipsoid 16×12 geteilt, Verlauf `mix(color2,color,ny)`
+ flash + Fog; Halo = Sprite-Billboard additiv mit Depth-Test OHNE Write →
Rim-Glow; Tiefensortierung via gemeinsames Depth-RT statt Sortierung;
Gate: vorderer Orb verdeckt hinteren trotz umgekehrter Zeichenreihenfolge).

- **Typ-Key** `glowOrbs`. Slots init/frame/beat/point (point = je Orb,
  `i` = Orb-Index, n Orbs): schreibt `x,y,z`, `radius`, `sx,sy,sz`
  (Achsen-Squash → oval/Halbkugel via sy<1 + Boden-Clip optional),
  `red/green/blue` (+ `red2/green2/blue2` für den Verlauf unten/oben),
  `flash` (0..1 → additiver addRGB-Offset, Original-Beat-Blitz).
- **Rendering:** Ellipsoid-Mesh (instanziert, ~16×12) mit
  Vertex-Farbverlauf im Shader (mix(color2,color,ny)) + `+flash`,
  Fog; dahinter Halo-Billboard (Radial-Falloff-Sprite, additiv,
  Faktor `haloScale`-Param). Tiefensortierung: Orbs nach Kamera-z sortiert
  zeichnen (additiv → Sortierung zweitrangig, Mesh ist opak → doch
  sortieren oder Depth-Buffer nutzen; Entscheid beim Bau: eigener
  Depth-Pass für 3D-Module gemeinsam).

## Offene Entscheide (vor Baustart klären)

1. ✅ Depth-Handling (S48, Etappe 2): EINE gemeinsame Depth-TEXTUR je Frame
   (GL_DEPTH_COMPONENT24), die opake 3D-Module (`begin3DDepth()`) an das
   jeweils aktive Draw-FBO hängen — als eigene Textur überlebt sie das
   Farb-Ping-Pong der Surfaces. Einmal je Frame gelöscht; Terrain-Mesh +
   Orbs schreiben, Sprites/Halos testen nur (kein Write). Chain-Reihenfolge
   bestimmt, wer zuerst Depth schreibt (Terrain vor Orbs ⇒ Verdeckung).
2. ✅ Kamera-Default ohne camera3d-Knoten (S48): Fallback-Kamera bei
   +1/tan(fov/2), three.js-Konvention (s. Modul 2).
3. BASS-Lookahead: eigener Service (AudioLookahead) — Umfang/Session
   separat planen; bis dahin reichen Beat-Prädiktion + gettime().

## Etappen (je ~1 Session)

1. ✅ **Bloom + 3D Camera + 3D-SuperScope** (S48; Bloom als Present-Post,
   Vorlagen-Dropdowns für Scope/Camera).
2. ✅ **Terrain + Glow-Orbs** (S48; gemeinsames Depth-RT, s. Entscheid 1).
3. ✅ **Lights-Demo v2** (S48): `asset/effectchain/lights_demo2.lvfx` —
   Kamerafahrt (Orbit + Beat-Zoom + Fog-Skript), Terrain mit
   Director-Phasenwellen (rings/relax skriptbar), 6 Zwei-Farb-Orbs mit
   Beat-Flash, Star-Dust, Bloom 1,3 + Vignette 0,3. Feinschliff gegen den
   Original-Quellcode (frisch geklont, Werte verifiziert):
   fov 30 ✓ · Fog schwarz (FogExp2 0.002 ≈ linear 2,5–8 in unserer Skala) ·
   Terrain 66→64, `updateSpectrum` SETZT h=h0+spectrum[dist] (übernommen:
   rings-Mix mit hartem SET bei 1) · Feder `drag=1−dt·5` ≈ 0,92 ✓ ·
   Orbs SphereGeometry(16,12) ✓, sechs Zwei-Farb-Paare (Demo: Farbrad) ·
   Bloom buildKernel(8)=σ8/25 Taps ✓, **Additiv-Opacity 1,3** (Demo),
   kein Threshold ✓ · Vignette 1−r²·0,3 ✓ (Shader wortgleich).
   Gate: Demo lädt + rendert offscreen (validiert alle EEL-Slots).
   **V3** (`lights_demo3.lvfx`, Wunsch Patrik): Kameraflug ÜBER das Terrain
   hinter einer **Wunderkerze** her (Vox-Ersatz als 3D-Scope: Funken mit
   Lebenszyklus, radial sprühend, Treble-getrieben) — die Kamera schreibt
   die Kerzen-Position in **q1..q3**, die Funken lesen sie (geteilter
   ScriptContext = erster Fall von Modul-Kopplung über q-Vars); Orbs halb
   im Terrain versenkt (Depth-RT clippt → Halbkugel-Look des Originals).
   Offen als Kür: Sprite-Sequenz-Renderer (Bengal-Strahl/Vox „echt") und
   Phasen-Quelle „echt" (energyTrend/BASS-Lookahead statt Zeitwellen).

**Fog-Hinweis (S48-Sichttest-Befund):** Fog ist per Gate belegt (Param- und
Skript-Pfad). Wirkung beachten: additive Sprites/Halos werden nur GEDÄMPFT
(fogColor beizumischen würde Nebel addieren); die fogColor-Mischung sieht
man nur auf opaker Geometrie (Terrain-Mesh, Orbs). Und das Fog-Fenster muss
die Szenentiefen treffen: Default-Kamera sitzt bei z≈3,73 — eine Szene um
den Ursprung liegt bei Distanz ~3–5 (fogStart 5+ zeigt dann nichts).

Verwandte Notizen: Memory `zukunft-lights-module` (Kurzfassung + Repo-URL),
`zukunft-dynamische-modulparameter`, Kür „Bass-PreFetch".
