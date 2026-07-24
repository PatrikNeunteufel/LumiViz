# AvsRef — Referenz-Renderer um den originalen vis_avs-Kern

**Zweck:** Ground-Truth für die AVS-Treue-Kalibrierung (Session 46). Rendert
`.avs`-Presets mit dem ORIGINALEN Winamp-AVS-Code (`../ref/vis_avs`,
BSD-3-Lizenz) headless auf einen Speicher-Framebuffer — ohne Winamp, Fenster
oder DDraw — und dumpt BMP + Luma-Statistik. Gegenstück: **AvsStandalone**
(LumiViz-Renderpfad) mit byte-identischem synthetischem Audio.

> **Nur lokales Werkzeug** — nicht verteilen (BSD-3 erlaubte es, Entscheid
> S45/46: lokal). Der Referenz-Quellbaum bleibt unangetastet; alle nötigen
> Anpassungen leben als gepatchte Kopien in `patched/`.

## Build (32-bit zwingend — EEL-JIT + MMX-Inline-Asm sind x86)

```bash
cmake -S tools/AvsRef -B tools/AvsRef/build -G "Visual Studio 17 2022" -A Win32
cmake --build tools/AvsRef/build --config Release
```

## Nutzung

```bash
tools/AvsRef/build/Release/AvsRef.exe <preset.avs|ordner> [--frames N] [--out DIR] [--size WxH] [--save-every M] [--beat-period N]
```

- **Audio:** synthetisch, byte-identisch zur LumiViz-Seite — Signal wie
  `AvsStandalone::feedSyntheticAudio` (frame-getaktet, t = frame/60), Byte-
  Abbildung wie `MultiEffectVisualizer::buildVisData` (kSpecGain=8 +
  AVS-Log-Kurve; Waveform `int(w*127)&0xFF`).
- **Beat:** Original-Onset (main.cpp:290-329) + `refineBeat` (bpm.cpp,
  `cfg_smartbeat=0` = Original-Default, deterministisch). `--beat-period N`
  erzwingt exakt alle N Frames einen Beat.
- **Ausgabe:** je Preset der letzte Frame als 32-bpp-BMP (top-down, BGRX,
  Alpha genullt) + `mean RGB`/`Luma min/max` im AvsStandalone-Format.
- `AVSREF_DEBUG=1` aktiviert Diagnose auf stderr (Lade-Trace, EEL-Selbsttest).

## patched/ — warum welche Datei

| Datei | Änderung |
|---|---|
| `rlib.h` | Default-Argument an Funktionszeiger entfernt (C2383) |
| `rlib.cpp` | Aufrufstellen übergeben `NULL` explizit |
| `bpm.cpp` | `abs((int)…)`-Casts — repliziert VC6-Semantik (nur `abs(int)` existierte) |
| `nseel-cfunc.c` | `__floor`/`__ceil` kollidieren mit UCRT; `&floor` keine Adress-Konstante mehr → Wrapper |
| `nseel-compiler.c` | **JIT-Fix:** nachlaufendes int3-Padding (0xCC) der Fragmente abschneiden — moderne Linker richten Funktionen aus, VC6 packte lückenlos; ohne Fix STATUS_BREAKPOINT/Heap-Korruption |
| `render.h`, `r_list.cpp` | UNVERÄNDERTE Kopien — nur damit Quote-Includes das gepatchte `rlib.h` finden |

Linker-Verträge (CMakeLists): `/OPT:NOICF` (sonst faltet ICF die identischen
leeren `nseel_asm_*_end`-Marker zusammen → Fragmentgrößen kaputt),
`/INCREMENTAL:NO`, `/NXCOMPAT:NO` (JIT-Code liegt in nicht-ausführbarem
GlobalAlloc-Speicher), `/SAFESEH:NO`.

Nicht mitkompiliert: `main/wnd/draw/cfgwin/render/undo/r_transition`
(Winamp/DDraw/UI); `r_colorreplace.cpp` war auch im Original-.dsp nicht
enthalten. Stubs für die restlichen Globals: `src/avsref_stubs.cpp`
(`C_UndoItem::set` wörtlich aus undo.cpp übernommen).

## Determinismus-Vorbehalte

- `cfg_smartbeat=0`: `refineBeat` reicht den Onset durch → voll deterministisch.
  (Smartbeat nutzte `GetTickCount` — nicht aktivieren für Diffs.)
- SMP ist aus (`g_config_smp=0`) — single-threaded wie der LumiViz-Vergleich.
- `rand()` sät das Original mit Systemzeit — Presets mit `rand()` sind
  zwischen Läufen nicht bit-stabil (Vergleich über Stats, nicht Pixel).
