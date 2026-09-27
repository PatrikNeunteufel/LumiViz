# LumiViz — Formelfelder je Modul (Inventar und Vorschläge)

> **Version:** 1.1.0
> **Datum:** 2026-09-27
> **Typ:** Arbeitsdokument — Bestandsaufnahme aller 84 Palettenmodule + Vorschlagsliste
> **Status:** Aktiv — Blöcke A und B umgesetzt (S79), C–F offen
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Knoten_Parameter_Konzept.md](Knoten_Parameter_Konzept.md) (Strang D, §6) ·
> [Skript_Variablen_Konzept.md](Skript_Variablen_Konzept.md) ·
> `include/visualizers/multieffect/ScriptVariables.hpp` (SSOT der Variablen) ·
> `MultiEffectVisualizer::run*` (Bindungen) · [Offene_Punkte.md](../Offene_Punkte.md)
> **Sprache:** Deutsch

---

## 1. Anlass und Entscheide (S79)

Patrik: Viele Module haben Felder, deren Werte sich per Formel (init / frame /
beat / point) verändern lassen; manche haben gar keine. Gewünscht war eine
Liste über alle Module mit Vorschlägen.

**Entscheid 2a (Patrik, S79):** zuerst die Lücken von Hand in die bestehenden
Bindungslisten nachtragen (Blöcke A–E), parallel einen generischen Weg
konzipieren (Block F). **Die Variablenreferenz im Panel wird dabei gleich
mitgezogen** — bis S78 zeigten rund vierzig Module nur „No dedicated variable
reference for this module yet."

**Entscheid 1 (Patrik, S79, gehört zur Modulauswahl, hier nur vermerkt):** Die
Auswahl-Combo wird ersetzt durch ein **nicht-modales Palettenfenster**: „+"
öffnet es; es hat einen Button „Hinzufügen" und nimmt Drag & Drop in die Kette
an; es bleibt offen, bis man es schließt (X oder „Schließen"). Darin Suche,
Kategorie-Baum, Filter nach Herkunft und Rolle, zuletzt benutzt / Favoriten,
Kurzbeschreibung.

## 2. Wie Formelfelder heute funktionieren

Es gibt **kein** generisches Modulationssystem. Die Wege:

1. **Parameter-Skript (Strang D)** — `runParamScript`
   (`MultiEffectVisualizer.cpp`, ~8551). Init/Frame/Beat-Slots; die Liste der
   erreichbaren Felder steht **je Modul von Hand** in dessen `run*`-Funktion.
   Vorbelegung aus dem Feld nur beim ersten Frame bzw. nach einer
   Reglerbewegung, danach schreibt das Skript den Wert fort.
2. **Eigene Skript-Hosts** — Fraktale, 3D-Module, SuperScope, Dynamic
   Movement, Color Modifier, Texer II, Triangle, Bump, Effect List: eigene
   Variablensätze, teils mit Punkt-/Pixel-/Level-Slot.
3. **Geteilter ScriptContext** — `reg00..reg99` und `gmegabuf` verbinden
   Skripte über Modulgrenzen („Global Variables").
4. **`ParamGruppe`/`ParamWert`-Baum** (EC ~158–229) — heute nur für
   ISF-Uniforms, statisch; als Fundament für dynamische Modulparameter
   vorgesehen.

**Seit S79** ist `ScriptVariables.hpp` die einzige Quelle der Variablenliste
für die Referenz im Panel, und der Test `test_ScriptVariablesGl.cpp` prüft sie
gegen die Laufzeit (jeder Tabellenname muss beim Lauf gesetzt oder gelesen
werden).

## 3. Befund je Modul

Abkürzungen: **PS** = Parameter-Skript (Strang D, + `b`/`w`/`h` + Audio-Satz),
**eig.** = eigener Skript-Host. Spalte „Kandidaten" = statische Felder, die
sich zu modulieren lohnen, aber (noch) keine Variable haben. Stand nach
Block A (S79).

| Modul | Herkunft | Slots | Variablen | Kandidaten |
|---|---|---|---|---|
| Effect List | AVS | init/frame (`useCode`) | enabled, clear, beat, alphain, alphaout, w, h | blendIn/Out-Modus, onBeatFrames |
| Host Group | LumiViz | — | — | crossfadeSeconds, outAdjustAlpha |
| Clear | AVS | PS | blend | **color** |
| Fadeout | AVS | PS | fadelen | **color** (Zielfarbe) |
| OnBeat Clear | AVS | PS | everynbeats | color, blend |
| Buffer Save | AVS | PS | slot, dir, adjustalpha | Blend-Modus |
| Buffer Blend | APE | PS | buffera, bufferb, mode | — |
| Custom BPM | AVS | PS | arbitraryms, skipcount, skipfirst ✚ | invert |
| Set Render Mode | AVS | — | — | **lineWidth, adjustAlpha**, lineBlend |
| Render Scale | LumiViz | — | — | divisor |
| Global Variables | APE | init/frame/beat | reg00..99, gmegabuf | — |
| Video Delay | APE | PS | delay | useBeats |
| Multi Delay | APE | PS | delay, buffer | mode |
| Debug Bars | LumiViz | — | — | color, orbitSpeed |
| Milkdrop (Preset-Pipeline) | MilkDrop | per-frame-init/per-frame/per-pixel + HLSL | voller MilkDrop-Satz | meshX/Y, Puffer-Fading; warpAnimSpeed, warpScale |
| Custom Wave / Shape / Sprite | MilkDrop | init/frame/point bzw. init/frame | MilkDrop-Wave-/Shape-Satz | Wave: samples, sep, scaling, smoothing · Shape: instances, texZoom |
| Shadertoy (GLSL) | LumiViz | GLSL Image + Buffer A–D | iTime …, bass…beat, Audio-Textur | **blend**, eigene Uniforms |
| Mesh Warp (GLSL) | LumiViz | GLSL `warp()` + PS | gridx, gridy, mixamount | wrapUv |
| GPU-Partikel | LumiViz | GLSL `kraft()` + PS | spawnx/y, spread, speed, dir, fan, gravx/y, drag, life, size | **colorStart/End**, sizeEndFactor, lifeJitter |
| Pixel Filter (GLSL) | LumiViz | GLSL `farbe()` + PS | mixamount | eigene Uniforms |
| ISF Filter | LumiViz (ISF) | GLSL frag + vertex | ISF-Builtins + Audio | **jeder ISF-INPUT**, blend, mixAmount |
| Video/Kamera | LumiViz | PS | speed, opacity | fit, blend |
| SuperScope | AVS | init/frame/beat/point | n, i, v, b, w, h, t, x, y, red/green/blue, linesize, skip, drawmode | colorCycleFrames, Farbtafel |
| Simple (Scope) | AVS | PS | mode, channel, position | Farben |
| Oscilliscope Star | AVS | PS | size, rot, spokes, rotscale, amplitude | Farben |
| Ring | AVS | PS | size, segments, basescale, audioscale | Farben, source |
| Rotating Stars | AVS | PS | points, skip, stars, rotspeed, orbit, baseradius, audiogain | Farben, bandLo/Hi |
| Bass Spin | AVS | PS | mode, smoothing, spinstep | **colorLeft/Right** |
| Moving Particle | AVS | PS | size, size2, maxdistance, spring, damping | **color**, blend |
| Starfield | AVS | PS | maxstars, warpspeed, beatspeed, durationframes | **color**, blend |
| FyrewurX | APE | PS | sparks, speed, gravity, life, dotsize, huedrift, burstspread | — |
| Metaballs 3D | APE | PS | count, radius, speed, threshold, spread, depth, phase | Farben, blend |
| Tentacles 3D | APE | PS | count, segments, length, thickness, speed, sway, waves, taper | Farben, blend |
| Timescope | AVS | PS | bands, channel, blend ✚ | **color** |
| Dot Grid | AVS | PS | spacing, xmove, ymove, blend ✚ | Farben |
| Dot Plane | AVS | PS | rotvel, angle, camdistance, settle | Farben[5], startRotation |
| Dot Fountain | AVS | PS | rotvel, angle | Farben[5] |
| Fractal 2D | LumiViz | init/frame/beat | cx, cy, zoom, rot, jx, jy, power | **maxIter, escapeR, colorScale, colorCycle, insideColor** |
| Fractal 3D | LumiViz | init/frame/beat | yaw, pitch, dist, power, scale, fold | **fov, juliaX..W, lightYaw/Pitch, ambient, colorScale/Cycle, background** |
| Domain Warp | LumiViz | init/frame/beat | scale, warp, speed, ox, oy | **warpScale, lacunarity, gain, octaves, colorScale/Cycle** |
| Fractal Zoomer | LumiViz | init/frame/beat | cx, cy, zoomspeed, rotspeed | **feedback, juliaX/Y, colorScale/Cycle, insideColor** |
| Lyapunov | LumiViz | init/frame/beat | amin, amax, bmin, bmax | negColor, colorScale/Cycle |
| Kleinian | LumiViz | init/frame/beat | p ✚, q ✚, morph, zoom, rot | iterations, colorScale/Cycle |
| Strange Attractor | LumiViz | init/frame/beat | a, b, c, d, rotation | **scale, rotationSpeed, color, dotSize** |
| Flame | LumiViz | init/frame/beat | variation ✚, scale ✚, rotation | **rotationSpeed, dotSize** |
| Reaction Diffusion | LumiViz | init/frame/beat | feed, kill | **diffA/B, stepsPerFrame, colorScale/Cycle** |
| Picture | AVS | PS | blend | keepAspect |
| Picture II | APE | PS | blend | — |
| Texer | APE | PS | particles, blend | — |
| Texer II | APE | init/frame/beat/point | n, i, v, b, w, h, x, y, sizex/y, red/green/blue, skip | — |
| Triangle | APE | init/frame/beat/point | n, i, b, w, h, x1..y3, red/green/blue | lineWidth |
| Movement | AVS | statischer Punktausdruck | x, y, d, r (einmal ausgewertet) | bewusst ohne PS (EC ~710) |
| Dynamic Movement | AVS | init/frame/beat/point | x, y, d, r, alpha, w, h, sw, sh, time, dt, b | xres/yres, buffern |
| Dynamic Shift | AVS | init/frame/beat | x, y, alpha, w, h, b | — |
| Dynamic Distance Modifier | AVS | init/frame/beat/pixel | d, b | — |
| Blitter Feedback | AVS | PS | scale, scale2 | — |
| Roto Blitter | AVS | PS | zoomscale, zoomscale2, rotdir | beatReverseSpeed |
| Mosaic | AVS | PS | quality, quality2 | durationFrames, blend |
| Water | AVS | — | — | (keine Felder) |
| Water Bump | AVS | PS | density, depth, dropradius, displacescale | dropX/Y |
| Bump | AVS | init/frame/beat | x, y, bi, isbeat, islbeat | **depth, depth2**, durationFrames |
| Interferences | AVS | PS | points, distance, alpha, rotation, rotationinc, speed | distance2, alpha2, rotationInc2 |
| Scatter | AVS | — | — | (keine Felder) |
| Mirror | AVS | PS | mode, slower | — |
| Brightness | AVS | PS | red, green, blue, distance ✚ | Ausschlussfarbe |
| Fast Brightness | AVS | PS | dir | — |
| Blur | AVS | PS | strength | — |
| Invert | AVS | — | — | (keine Felder) |
| Colorfade | AVS | PS | faderr/g/b, beatfaderr/g/b, onbeatframes | — |
| Color Modifier | AVS | init/frame/beat/level | red, green, blue je LUT-Eintrag | — |
| Color Map | APE | PS | key, blendmode, adjustblend | **Verlaufsstopps** |
| Color Clip | AVS | PS | distance | **clipColor, outColor** |
| Unique Tone | AVS | PS | blend | **color** |
| Interleave | AVS | PS | x, y, x2 ✚, y2 ✚ | **color**, beatDuration |
| Convolution | APE | PS | bias, scale, edgemode ✚ | Kernel |
| Normalise | APE | — | — | (keine Felder) |
| MultiFilter | APE | PS | effect | — |
| Channel Shift | APE | PS | mode | — |
| Color Reduction | APE | PS | levels | — |
| Multiplier | APE | PS | mode | — |
| Grain | AVS | PS | amount | blend |
| Add Borders | APE | PS | size | **color** |
| Bloom | LumiViz | PS | intensity, threshold, radius, vignettestrength | downsample |
| 3D Camera | LumiViz | init/frame/beat | px, py, pz, tx, ty, tz, fov, roll, fogstart, fogend, fogcolor_* ✚, b | — |
| SuperScope 3D | LumiViz | init/frame/beat/point | n, b, w, h, i, v, x, y, z, size, red/green/blue, skip | **falloff**, renderMode |
| Terrain 3D | LumiViz | init/frame/beat/point | res, n, rings, relax, flatten, megabuf; point: i, gx, gy, h, red/green/blue | **baseAmp, yOffset, extent, dotSize, falloff, meshColor, colorLow/High** |
| Glow Orbs | LumiViz | init/frame/beat/point | n, b; je Orb: i, x, y, z, radius, sx/sy/sz, red/green/blue, red2/green2/blue2, flash | **haloScale, haloIntensity, falloff** |

✚ = in S79 neu gebunden. Nur per Import erreichbar (nicht in der Palette): Text,
Avi, Comment, ImportNotes, Passthrough — Text hätte mit fontHeight, Farbe,
xShift/yShift und outline gute Kandidaten.

## 4. Vorschläge

### A — Versprochen, aber nicht gebunden ✅ (S79)

Der Struct-Kommentar nannte Variablen, die die Laufzeit nie band:

| Modul | Fix |
|---|---|
| Brightness | `distance` gebunden (Frame-Kopie → `uDistance`) |
| Custom BPM | `skipfirst` gebunden |
| Interleave | `x2`, `y2` gebunden (Beat-Sprung aus der Frame-Kopie) |
| Timescope | `blend` gebunden |
| Dot Grid | `blend` gebunden |
| Convolution | `edgemode` gebunden |
| Kleinian | `p`, `q` gebunden; Kommentar `rotation` → `rot` (so hieß die Bindung schon immer) |
| Flame | `variation`, `scale` gebunden; Kommentar „Variationsbeiwerte" korrigiert (solche Felder gibt es nicht) |

Dazu: Referenz im Panel aus `ScriptVariables.hpp` für alle Module mit Slots;
Test gegen die Laufzeit.

### B — Farben per Formel ✅ (S79)

Je Farbfeld drei Variablen **`<feldname>_r/_g/_b`** (Feldname klein, Bereich
0..1). Das Feld bleibt `0xRRGGBB`; die Laufzeit arbeitet auf einer Frame-Kopie
(`FarbKopie` in `MultiEffectVisualizer.cpp`). **Geklemmt wird erst beim
Zeichnen** — die Fortschreibung behält Werte außerhalb 0..1, damit etwa
`color_r = color_r + 0.01` weiterzählen und per `fmod` zurückgeholt werden kann.

| Modul | Variablen |
|---|---|
| Clear, Fadeout (Zielfarbe), OnBeat Clear, Unique Tone, Interleave, Add Borders, Timescope, Moving Particle, Starfield (Tönung, weiß = ungetönt), Brightness (Ausschlussfarbe), Strange Attractor (ohne Verlauf) | `color_r/_g/_b` |
| Color Clip | `clipcolor_*`, `outcolor_*` |
| Bass Spin | `colorleft_*`, `colorright_*` |
| GPU-Partikel | `colorstart_*`, `colorend_*` |
| 3D Camera | `fogcolor_*` |
| Terrain 3D | `meshcolor_*`, `colorlow_*`, `colorhigh_*` |

Parameter-Skript-Module seeden die Farbe wie jeden Regler (neu nach einer
Panel-Änderung); die Hosts von 3D Camera, Terrain 3D und Strange Attractor
seeden beim Kompilieren (die Farbe steckt im Compile-Schnappschuss) und lesen
je Frame zurück. Referenz: `ScriptVariables.cpp`; Bindungstest grün.

**Offen (Nachgang):** Farbtafeln der Scopes (Simple, Osc Star, Ring, Rotating
Stars, Dot Plane/Fountain/Grid, Metaballs, Tentacles) — dort ist eine Tafel,
keine Einzelfarbe; Vorschlag `colorindex` (welcher Eintrag) oder eine
Überblend-Phase. Ebenso Debug Bars (ohne Skript) und Text (nur Import).

### C — Farbpaletten der Fraktale

Fractal 2D/3D, Domain Warp, Fractal Zoomer, Lyapunov, Kleinian, Reaction
Diffusion: `colorScale`/`colorCycle` (+ Innen-/Hintergrundfarbe) als
Variablen. Größter sichtbarer Gewinn je Zeile Code.

### D — Weitere statische Felder

Siehe Spalte „Kandidaten" (fett = empfohlen). Schwerpunkte: Bump
`depth`/`depth2`, Strange Attractor `scale`/`rotationSpeed`/`dotSize`, Flame
`rotationSpeed`/`dotSize`, Reaction Diffusion `diffA/B`, Terrain 3D, Glow Orbs,
SuperScope 3D `falloff`, Interferences `*2`-Werte, MilkDrop-Wave/Shape-Felder.

### E — Module ganz ohne Formelzugang

| Modul | Vorschlag |
|---|---|
| Set Render Mode | PS mit `linewidth`, `adjustalpha` (Linienbreite im Takt — klassischer AVS-Kniff) |
| ISF Filter | jeder ISF-INPUT als Skriptvariable (größter Hebel: jeder importierte Filter wird modulierbar) |
| Shadertoy / Pixel Filter | `blend` + eigene Uniforms, die ein Skript setzt |
| Host Group | `crossfadeseconds`, `outadjustalpha` |
| Text (Import) | `fontheight`, Farbe, `xshift`/`yshift` |

### F — Generischer Weg (Konzept offen)

1. **Automatische Bindung:** jedes Zahlen- und Farbfeld eines Moduls wird
   Skriptvariable, gestützt auf das Feld-Inventar (`harvest_field_docs.py` +
   Inventar-Golden) und `ScriptVariables.hpp` — statt je Feld vier Stellen von
   Hand.
2. **Mod-Quelle je Feld ohne Formel:** Kontextmenü am Feld → Quelle
   (bass/mid/treb, Beat-Hüllkurve, LFO) × Tiefe. Setzt auf 1. auf.
3. **Modulspezifische Hervorhebung:** `EelScriptEditing.hpp::symbolCategory`
   klassifiziert heute global; mit der Tabelle ließe sie sich je Modul genau
   machen (unbekannte Namen markieren).

Offene Fragen für die Konzept-Session: Fortschreibungs-Semantik (MEV ~8591)
für automatisch gebundene Felder; Kosten bei Punkt-Slots; Umgang mit
Aufzählungsfeldern (int-Modi) und Texten.
