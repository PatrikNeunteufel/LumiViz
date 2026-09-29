# MilkdropStandalone — der LumiViz-Renderpfad ohne App

**Zweck:** Treibt den echten `MilkdropVisualizer` in einem eigenen
GL-3.3-Fenster, ohne Docking, Panels oder Render-Thread. Er ist unsere Seite
jedes MilkDrop-Referenzvergleichs; die Gegenseite ist
[`tools/MilkdropRef`](../../../tools/MilkdropRef/README.md) (der originale
Kern). Beide binden dasselbe synthetische Audio ein
(`projects/exec/common/SynthAudio.hpp`).

**Exe (Release-clang, wie die Harnesse sie erwarten):**
`out/build/windows-ninja-release-clang/exec/MilkdropStandalone/bin/Release/MilkdropStandalone.exe`

Welches Werkzeug wofür, und welche Fallen es gibt: `projects/apps/LumiViz/docs/Werkzeug_Wegleitung.md`.
Die Mess-Skripte, die diese Exe aufrufen: [`asset/calibration/milkdrop/README.md`](../../../asset/calibration/milkdrop/README.md).

## Aufruf

```bash
MilkdropStandalone [preset.milk|ordner ...] [--auto] [--frames N] [--out DIR] [--size WxH] [Optionen]
```

Ohne Preset nimmt er den c1-Kalibrier-Satz (`asset/calibration/milkdrop/c1`,
vom Exe-Pfad aufwärts gesucht). `--help` listet alle Optionen mit Kurztext.

**Interaktiv** (ohne `--auto`/`--ab`): ←/→ Preset wechseln · R neu laden ·
S Screenshot · G Kalibrier-Raster · Esc beenden.

## Argumente

### Lauf und Ausgabe

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `preset …` | c1-Satz | eine oder mehrere `.milk`-Dateien oder Ordner |
| `--auto` | aus | Stapel: jedes Preset N Frames, Screenshot `<stem>_auto.png` + Pixel-Statistik, dann Ende. Exit 1, wenn ein Preset den Custom-Pfad nicht erreicht |
| `--frames N` | 120 | Frames je Preset (`--auto`), bzw. Frames vor dem Wechsel (`--ab`) |
| `--out DIR` | `%TEMP%/lumiviz_standalone` | Screenshot-Verzeichnis |
| `--size WxH` | 800x600 | Fenster- = Rendergröße (physische Pixel; High-DPI-Skalierung ist im Werkzeug abgeschaltet) |
| `--dump-shaders` | aus | übersetzte GLSL-Quellen nach `--out` |
| `--fps-grenze N` | 0 = nur vsync | Bilder/s-Obergrenze (S81). Messläufe: 60 — am 240-Hz-Schirm sonst viermal mehr Last; das Bild hängt nur an der Bildnummer (dt fix 1/60), mit und ohne Grenze bitgleich geprüft |

### Startzustand

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `--seed` | aus = saatlos | Kaltstart-Saat einschalten (App-Verhalten). **Erst das Flag schaltet `LUMIVIZ_MILKDROP_SEED` frei** — ohne setzt der Standalone `LUMIVIZ_MILKDROP_NOSEED` selbst (Falle S75) |
| `--blende` | aus | Sicht-Blende wie in der App (~0,5 s Schwarz-Einblendung nach frischer Saat) |

### A/B-Wechsellauf (Preset-Wechsel-Wächter, S67)

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `--ab` | aus | erstes Preset `--frames` Frames, dann Wechsel aufs letzte, danach Frame-Hashes (FNV-1a) + Mittel-RGB loggen |
| `--ab-frames N` | 300 | Mess-Frames nach dem Wechsel |
| `--wechsel loeschen\|behalten` | loeschen | `loeschen` = App-Lösch-Pfad (`requestFeedbackErbe(0)`) |
| `--audio-neustart` | aus | Audio-Uhr beim Wechsel auf 0 (Kaltstart-Vergleich) |

### Audio

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `--audio-muster klassisch\|musik` | klassisch | synthetisches Signal; **beide** Muster erzeugt MilkdropRef identisch |
| `--silence` | aus | Stille (Hunger-Test) |
| `--audio-beat` | aus | korrelierte Band-Hüllkurven mit Jitter (Beat-Detektor-Presets) |
| `--beat-hz HZ` | 2 | Schlagfolge (2 = 120 BPM) |
| `--klangfarbe` | aus | wandernde Spektralbalance und Oberwellen |
| `--audio-datei PFAD` | — | echte Musik. **Nur für Augenschein — jeder Referenzvergleich damit ist wertlos** (MilkdropRef erzeugt sein Audio selbst) |
| `--audio-start SEK` | 0 | Startversatz in der Datei |
| `--audio-gain F` | 1 | Faktor auf Wellenform und Spektrum der Datei |
| `--audio-stumm` | aus | Datei nicht hörbar ausgeben (im `--auto`/`--ab`-Lauf ohnehin) |

Für Referenzvergleiche taugen nur `--audio-muster` und `--silence`: Nur sie
haben ein Gegenstück in MilkdropRef. `--audio-beat`, `--beat-hz` und
`--klangfarbe` verändern das Signal allein auf unserer Seite.

## Umgebungsvariablen

| Variable | Wirkung |
|---|---|
| `LUMIVIZ_REF_BILDSCHIRM` | Fenster auf einen **festen Monitor** (S81): `DISPLAY2` (Gerätename, bevorzugt) oder Index `1`. Derselbe Wert wie bei MilkdropRef; Bestimmung des Monitors dort in der README |
| `LUMIVIZ_MILKDROP_SEED=0x…` | Startwert der Saat — wirkt nur zusammen mit `--seed` |
| `LUMIVIZ_MILKDROP_NOSEED` | Saat aus (setzt der Standalone ohne `--seed` selbst) |
| `LUMIVIZ_MILKDROP_AUDIO=original\|bisher` | Audio der Custom-Waves erzwingen (Feld `audioOriginal`, Standard original seit S80) |
| `LUMIVIZ_MILKDROP_ASPEKT=original\|bisher` | Seitenverhältnis der Custom-Waves erzwingen (Feld `aspektOriginal`, Standard original seit S81) |
| `LUMIVIZ_MILKDROP_EEL_RAND=original` | ns-eel2-`rand()` erzwingen (Standard seit S80 ohnehin) |
| `LUMIVIZ_MILKDROP_EEL_RAND_SKIP=k` | k Zufallszüge vorab verbrauchen (Stromversatz zur Referenz suchen) |
| `LUMIVIZ_MILKDROP_EEL_RAND_LOG` | Züge je Frame loggen (erste fünf Frames je Ladung), Gegenstück zu `MILKDROPREF_RAND_LOG` |
| `LUMIVIZ_MILKDROP_HALBTEXEL` | Gestaltung „Weicher Sog ↖“ = Stand vor S80 (Vorher/Nachher-Messung) |
| `LUMIVIZ_MILKDROP_REGELWERK=modern\|legacy` | Betriebsart unabhängig vom Dokument erzwingen |
| `LUMIVIZ_MILKDROP_TRACE=0` | Diagnose-Trace aus (Vorgabe an; Log in `%TEMP%/lumiviz_milkdrop_trace.log`) |
| `LUMIVIZ_MILKDROP_TRACE_VARS=a,b,…` | diese Skriptvariablen im Trace mitschreiben (Frames 0–9, dann jeder 60.) |
| `LUMIVIZ_MILKDROP_DUMP_WARP=DIR` | Pufferinhalte der ersten Frames als PNG nach DIR |
