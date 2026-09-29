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
tools/AvsRef/build/Release/AvsRef.exe <preset.avs|ordner> [--frames N] [--out DIR] [--size WxH] [--save-every M] [--beat-period N] [--tick-hz N] [--silence] [--ape-dir DIR] [--audio-muster klassisch|musik]
```

Ohne Preset-Argument gibt die Exe eine Kurzform der Liste aus. Die
Mess-Skripte, die diese Exe aufrufen:
[`asset/calibration/avs/README.md`](../../asset/calibration/avs/README.md).

### Argumente

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `<preset.avs\|ordner>` | — (Pflicht) | ein Preset oder alle `.avs` eines Ordners (sortiert). ANSI-Programm: Pfade außerhalb cp1252 kommen als `?` an (`compare_avsref.py` legt dafür ASCII-Kopien an) |
| `--frames N` | 120 | Frames je Preset; der letzte wird als BMP gespeichert |
| `--out DIR` | `.` | Zielordner für `<datei_mit_unterstrich>_f<NNNN>_ref.bmp` |
| `--size WxH` | 800x600 | Rendergröße (min. 64×64) |
| `--save-every M` | 0 = nur letzter | zusätzlich jeden M-ten Frame speichern (0-basiert gezählt, Dateiname 1-basiert — wie AvsStandalone) |
| `--beat-period N` | 0 = Detektor | exakt alle N Frames ein Beat statt Original-Onset + `refineBeat`; schlägt auch die Beat-Spur des Musters `musik` |
| `--tick-hz N` | 0 = Wanduhr | EEL-`gettime()` und AVI-Vorschub als Frame-Uhr (`frame·1000/N` ms) — ohne sie messen gettime-Presets die Batch-Geschwindigkeit (S59/S60) |
| `--silence` | aus | Stille statt Kalibrier-Signal |
| `--ape-dir DIR` | `<exe>\ape` (leer) | APE-Sammlung, deren Plugin-DLLs der Kern lädt; ohne sie rendert die Referenz keine APEs |
| `--audio-muster klassisch\|musik` | klassisch | Muster des synthetischen Signals (`SynthAudio.hpp`, gemeinsam mit AvsStandalone); im Muster `musik` kommt der Beat aus der Beat-Spur der Vorlage |

### Umgebungsvariablen

| Variable | Wirkung |
|---|---|
| `LUMIVIZ_REF_BILDSCHIRM` | **ohne Wirkung** (S81): AvsRef rendert fensterlos, es gibt nichts zu platzieren. Ist die Variable gesetzt, meldet das Log `LUMIVIZ_REF_BILDSCHIRM=… ohne Wirkung: AvsRef rendert fensterlos`. Die Gegenseite AvsStandalone legt ihr Fenster danach auf den festen Monitor; Wert und Bestimmung des Monitors: [`tools/MilkdropRef/README.md`](../MilkdropRef/README.md) |
| `AVSREF_DEBUG` | Diagnose auf stderr: Lade-Trace, Frame-Trace, EEL-Selbsttest (`n=200`) |
| `AVSREF_EELTEST` | Funktions-Selbsttest aller ns-eel-Funktionen (Compile + JIT gegen die C-Bibliothek), vorab die Fragmentgrößen der JIT-Tabelle. Eigener Schalter, weil er den Prozess bei einem JIT-Schaden absichtlich sterben lässt |

- **Audio:** synthetisch, byte-identisch zur LumiViz-Seite — Signal wie
  `AvsStandalone::feedSyntheticAudio` (frame-getaktet, t = frame/60), Byte-
  Abbildung wie `MultiEffectVisualizer::buildVisData` (kSpecGain=8 +
  AVS-Log-Kurve; Waveform `int(w*127)&0xFF`).
- **Beat:** Original-Onset (main.cpp:290-329) + `refineBeat` (bpm.cpp,
  `cfg_smartbeat=0` = Original-Default, deterministisch). `--beat-period N`
  erzwingt exakt alle N Frames einen Beat.
- **Ausgabe:** je Preset der letzte Frame als 32-bpp-BMP (top-down, BGRX,
  Alpha genullt) + `mean RGB`/`Luma min/max` im AvsStandalone-Format.

## patched/ — warum welche Datei

| Datei | Änderung |
|---|---|
| `r_avi.cpp` | AVI-Vorschub optional auf die virtuelle Frame-Uhr (`--tick-hz`, S60) — `avsref_tick()` statt `GetTickCount()` an 2 Stellen; ohne Schalter Original-Wanduhr |
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
