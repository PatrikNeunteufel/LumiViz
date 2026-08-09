# Changelog Session 49 (2026-07-26)

## Dynamic Movement bit-treu (r_dmove-Fixpunkt-Warp)

- **Warp-Pass neu** (`kWarpFxFragmentShader` + `applyGridWarpFx`): Das Original
  legt die Skript-Ergebnisse als 16.16-Fixpunkt-Tabelle ab und interpoliert sie
  **separabel in Ganzzahlen** auf die Bildgröße — je Gitterband eine
  trunkierende Division, danach reine Additionen. Das ist pro Pixel geschlossen
  berechenbar (Bandindex, Restweg, Bandsteigung) und läuft jetzt komplett im
  Shader; die Gittertabelle geht als **RGBA32I-Textur** hin. Das bisherige
  Dreiecksnetz interpolierte baryzentrisch in float — andere Kurve UND kein
  Trunkierungsverlust, der sich über Feedback-Ketten aufsummiert.
- `ScriptGridModule::fieldFx()` liefert die AVS-Rohform (16.16-Pixel,
  Alpha (α·255)<<16, AVS-Zeilenordnung). Das Punkt-Skript läuft jetzt in
  AVS-Reihenfolge (oben→unten) — Skripte mit Seiteneffekten sahen sie bisher
  verkehrt herum. `setAvsGridPositions()` schaltet die **trunkierten**
  r_dmove-Stützstellen zu (`xc_pos += (w<<16)/(XRES-1)`).
- `xres`/`yres`-Grenze 96/72 → **256** (AVS: r_dmove.cpp:235-238).
- **Wirkung**: s2-Sweep dmove 0,03er → 0,011/0,007 bzw. 0,002/0,003 (grün in
  beiden Größen); Tie Tunnel DM 0,29 → 0,154; ZeroG/Rotor/crunchi deutlich
  besser.

## Movement bit-treu (r_trans hat KEIN Gitter)

- Befund beim Lesen der Referenz: r_trans wertet das Skript für **jedes Pixel**
  aus (r_trans.cpp:453-526) und quantisiert den Subpixel-Anteil auf **5 Bit**.
  Unser 96×72-Gitter war dort strukturell falsch.
- Neu: `ScriptGridModule::buildTransTable()` baut die gepackte Tabelle
  (`ow+oh*w | ypartial<<22 | xpartial<<27`) — wie im Original nur bei
  Größen-/Skriptwechsel; `kMoveTabFragmentShader` dekodiert sie samt
  `BLEND4`/`BLEND_AVG` in Ganzzahlen.
- **Wirkung**: `02_movement_zoom_kreis` 0,034 → 0,011 (klein) bzw. 0,041 →
  0,007 (groß).

## Referenz-Kern: AvsRef lädt jetzt die echten APEs

- `AvsRef --ape-dir DIR` setzt `g_path` auf eine echte APE-Sammlung. Bisher
  zeigte das Verzeichnis bewusst ins Leere (avsref_main.cpp:349-357) — Presets
  mit APE waren damit **strukturell unvergleichbar**: wir rendern den Effekt,
  die Referenz nicht. `compare_avsref.py --ape-dir` reicht es durch (Default:
  die externe Preset-Sammlung).
- Gegenstück `AvsStandalone --no-ape` (+ `ChainNode::fromApe`) für Läufe ohne
  Sammlung.
- **Wirkung**: P3 ZeroG 0,74 → 0,050 (der fehlende Color-Map-APE war die ganze
  Geschichte, nicht unser Code).

## rand() — EIN MSVC-Strom je Preset

- NSEEL-`rand(x)` ist `rand()%max(x,1)` (nseel-cfunc.c:54), also die `rand()`
  der MSVC-Laufzeit; AvsRef ruft nie `srand`, der Strom startet reproduzierbar
  bei Seed 1 (zwei Läufe hashen identisch). Nachgebaut in `ScriptContext`
  (`nextRand`, `resetRandom` beim Preset-Wechsel, `skipRandom` per affiner
  Binärexponentiation).
- **Grain** (r_grain): `depthBuffer` aus dem Strom (492 Züge im Konstruktor,
  2 je Pixel beim Anlegen, 1 je Frame) als RG8-Textur; Shader rechnet
  `(c*s)>>8` mit Klemmung. **0,045 → 0,002**.
- **Scatter** (r_scat): ein Zug je Pixel — auf der GPU per **LCG-Sprung** im
  Shader aus dem Zustand vor dem Pass, CPU stellt danach um `w·(h−8)` weiter;
  exakte `fudgetable`-Geometrie, lineare Adressierung. **0,015 → 0,003**
  (= Bodensatz des Basisbildes).
- **Starfield** (r_stars): zeilengetreu — Sterne in ganzzahligen Pixeln um die
  Bildmitte, `(X<<7)/(int)Z + Xoff`, `MaxStars = MulDiv(gesetzt, w·h, 512·384)`
  ≤ 4095, `CreateStar` mit zwei Zügen, `BLEND_ADAPT`, `incBeat`. **→ 0,000**.
- **Mirror / Channel Shift / Moving Particle / Water Bump** ziehen aus dem
  Strom mit den Original-Formeln (Water Bump hält jetzt Radius-Abstand vom
  Rand).

## Color Map gegen die echte APE kalibriert

- Kein Quelltext vorhanden → **gemessen**: `make_colormap_probes.py` (18
  strukturierte Sonden + 310er-Sweep nach `out/`), `analyse_colormap.py`
  rekonstruiert die Kennlinie je Renderer aus den Bildpaaren.
- **LUT-Abgriff** war `GL_LINEAR` mit normalisierter Koordinate (interpolierte
  zwischen Nachbareinträgen, halbes Texel Versatz) → jetzt `texelFetch` +
  `GL_NEAREST`. **Key-Rechnung** und Stützstellen-Interpolation ganzzahlig.
- **Alle zehn Blend-Modi bit-genau** (90/90). Zwei Überraschungen: 50/50 ist
  ein **echter** Mittelwert `(a+b)>>1` (nicht `BLEND_AVG`, das Bit 0 verliert),
  *adjustable* **rundet** (`+128`), während `BLEND_ADJ` im Kern abschneidet.
- Kennlinie 44/44 exakt je Zweierpotenz-Spannweite, sonst ±1 (offener Rest:
  die APE weicht historienabhängig ab).

## Weitere Treue-Korrektur

- **Roto Blitter** 0,12/0,20 → **0,020/0,016**: `BLEND4_16` gewichtet mit
  **255**, nicht 256 (`mmx_blend4_revn = 0x00ff00ff`, render.cpp:64) — der
  S48-Shader war ~0,4 % je Resample zu hell, über die Feedback-Kaskade
  sichtbar. Damit ist der S48-Kleinrest erledigt (die Scope-y-0,5-Hypothese
  war es nicht).

## Tests

- **428 Cases grün, 0 Skips** (S48: 426). Neu: `test_DmoveFixpunkt.cpp` — eine
  zeilengetreue CPU-Nachbildung von `smp_render` (r_dmove.cpp:372-578) plus die
  geschlossene Form, geprüft gegeneinander UND gegen den echten GL-Pass in vier
  Varianten (clamp/wrap × nearest/subpixel/blend), dazu ein Gate für die
  r_trans-Tabellenpackung.
- Matrix **34/41 → 37/41** · Zwillinge 65/65 · alle drei Builds grün.
