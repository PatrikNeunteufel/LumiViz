# MilkdropRef — Referenz-Renderer um den originalen MilkDrop3-Kern

**Zweck:** Ground-Truth für die MilkDrop-Kalibrierung (Session 63). Rendert
`.milk`-Presets mit dem ORIGINALEN MilkDrop3-Code (`../ref/MilkDrop3`,
BSD-3-Lizenz) über D3D9 in ein unsichtbares Fenster und dumpt BMP +
Luma-Statistik. Gegenstück: **MilkdropStandalone** (LumiViz-Renderpfad) mit
formelgleichem synthetischem Audio.

> **Nur lokales Werkzeug** — nicht verteilen (wie AvsRef). Der
> Referenz-Quellbaum bleibt unangetastet; nötige Anpassungen leben als
> gepatchte Kopien in `patched/`.

**MessageBox → stderr (S67):** `patched/ref_msgbox.h` wird per `/FI` in alle
Übersetzungseinheiten gezwungen — die Original-Fehlerdialoge („MILKDROP
ERROR", z. B. „Unable to read the data file" bei falscher Preset-Basis)
blockierten Batch-Läufe; jetzt stderr-Zeile + IDOK. Merke außerdem:
`data/include.fx` wird relativ zu `<presetordner>/../data` gesucht —
EEL-/Verhaltens-Sonden deshalb nach `asset/Milkdrop3/sonden/` legen
(Nachbar von `data/`, außerhalb des Triage-Korpus `presets/`).

## Setup: d3dx9 (einmalig)

Das NuGet-Paket **Microsoft.DXSDK.D3DX** nach `third_party/dxsdk-d3dx/`
entpacken (untracked — bewusst nicht committet):

```bash
curl -L -o dxsdk.zip https://www.nuget.org/api/v2/package/Microsoft.DXSDK.D3DX/9.29.952.8
unzip dxsdk.zip -d third_party/dxsdk-d3dx
```

Erwartet werden `third_party/dxsdk-d3dx/build/native/include/d3dx9.h` und
`.../release/lib/x86/d3dx9.lib` (+ `bin/x86/D3DX9_43.dll`, wird neben die Exe
kopiert).

## Build (32-bit zwingend — ns-eel2-JIT ist x86, wie das Original-Projekt)

```bash
cmake -S tools/MilkdropRef -B tools/MilkdropRef/build -G "Visual Studio 17 2022" -A Win32
cmake --build tools/MilkdropRef/build --config Release
```

## Nutzung

```bash
tools/MilkdropRef/build/Release/MilkdropRef.exe <preset.milk|ordner> [--frames N] [--out DIR] [--size WxH] [--silence] [--show] [--audio-muster klassisch|musik] [--saat N] [--zufall-lumiviz] [--mesh WxH]
```

Ohne Preset-Argument gibt die Exe die Kurzform dieser Liste aus.

### Argumente

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `<preset.milk\|ordner>` | — (Pflicht) | ein Preset oder alle `.milk` eines Ordners (sortiert) |
| `--frames N` | 240 | Frames je Preset; der letzte wird als BMP gespeichert |
| `--out DIR` | `out_milkdropref` | Zielordner für `<stem>_ref.bmp` |
| `--size WxH` | 640x480 | Client-Größe = Rendergröße (min. 64) |
| `--silence` | aus | Null-Signal statt synthetischem Audio (Hunger-Test) |
| `--show` | aus | Fenster sichtbar (sonst unsichtbar) |
| `--audio-muster klassisch\|musik` | klassisch | Muster des synthetischen Signals (`SynthAudio.hpp`, gemeinsam mit MilkdropStandalone) |
| `--saat N` | 0 = aus | Startwert des Feedback-Puffers (`0x…` = hex); gleicher Wert wie `LUMIVIZ_MILKDROP_SEED` auf unserer Seite. Seit S81 **vor** dem ersten Frame eingespeist, die Erstlöschung des Kerns (`milkdropfs.cpp:971`) wird dafür übersprungen — der erste Warp liest die Saat wie bei LumiViz (vorher nach Frame 0, einen Frame zu spät) |
| `--zufall-lumiviz` | aus | Zufallsdaten wie LumiViz (S80, Abschnitt unten) |
| `--mesh WxH` | 64×64 (Kern) | Warp-Mesh; `32x24` = Winamp-Original und LumiViz |

### Umgebungsvariablen

| Variable | Wirkung |
|---|---|
| `LUMIVIZ_REF_BILDSCHIRM` | Fenster auf einen **festen Monitor** (S81): `DISPLAY2` (Gerätename, bevorzugt) oder Index `1` (0-basiert). Oben links im Arbeitsbereich. Unbekannter Wert → Warnung mit Monitorliste, Standardort. Wird genauso von MilkdropStandalone und AvsStandalone gelesen (`projects/exec/common/RefBildschirm.hpp`); AvsRef ist fensterlos und meldet sie nur |
| `MILKDROPREF_AUDIO_DUMP=N` | für Frame N: PCM, ausgerichtete Wellenform, Spektrum des Kerns auf stdout |
| `MILKDROPREF_AUDIO_SEQ=N` | PCM der Frames 0…N−1, im letzten zusätzlich AlignWaves-Ergebnis + `mysound.imm` |
| `MILKDROPREF_RAND_LOG=N` | die ersten N EEL-`rand()`-Züge loggen (Messkopie `patched/nseel-cfunc.c`) |

**Den Monitor bestimmen:** Windows-Einstellungen → System → Anzeige →
„Identifizieren“ — die angezeigte Ziffer n entspricht `DISPLAYn`. Oder als
Liste:

```bash
powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.Screen]::AllScreens | ft DeviceName,Primary,Bounds"
```

Für alle Läufe einer Shell-Sitzung einmal setzen (die Python-Harnesse reichen
die Umgebung an beide Exes weiter):

```bash
export LUMIVIZ_REF_BILDSCHIRM=DISPLAY2
```

PowerShell: `$env:LUMIVIZ_REF_BILDSCHIRM = "DISPLAY2"`. Dauerhaft für neu
gestartete Programme: `setx LUMIVIZ_REF_BILDSCHIRM DISPLAY2` (bereits
laufende Shells und Apps sehen das erst nach einem Neustart).

Die Platzierung ändert nur den Ort, nicht die Rendergröße oder das Bild.

- **Audio:** synthetisch, formelgleich zu `MilkdropStandalone::feedSyntheticAudio`
  (t = frame/60, Sinus 220 Hz + 120-BPM-Puls), als 576-Sample-8-bit-PCM im
  **Zweierkomplement** — so liefert Winamp (`VIS.cpp:766/774`), und so wandelt
  der Kern (`(pcm ^ 128) - 128`, `pluginshell.cpp:1668`). **Bis S80 stand hier
  „128-Mitte“ — falsch:** der Kern sah eine zerhackte Wellenform (Befund S80,
  `Offene_Punkte.md` §3); alle MilkDrop-Referenzzahlen davor stehen unter
  Vorbehalt. `--silence` = Null-Signal (Bytes 0).
- **Audio-Ausgabe:** `MILKDROPREF_AUDIO_DUMP=N` gibt für Frame N das PCM
  (FFT-Eingang), die ausgerichtete Wellenform (nach `AlignWaves`) und das
  Spektrum des Kerns aus — Quelle der Sollwerte von `test_MilkdropFft`
  (`MILKDROPREF_AUDIO_SEQ` entsprechend für `test_MilkdropAudio`).
- **Ausgabe:** je Preset der letzte Frame als 32-bpp-BMP (top-down, BGRX,
  Alpha genullt) + `mean RGB`/`Luma min/max` im AvsStandalone-Format.
- **Texturen/Sprites:** `m_szMilkdrop2Path` zeigt auf den Großeltern-Ordner
  des Presets — Layout `asset/Milkdrop3/{presets,textures,sprites}` wird
  direkt gefunden, kein `plugins\Milkdrop2`-Verzeichnis nötig.
- **Batch-Vertrag:** Preset-Lock an, kein Auto-Wechsel/Hard-Cut, Blend 0.

## Determinismus-Vorbehalte (§9)

- **GPU-Rendering** ⇒ nicht bit-deterministisch — Vergleich über
  Statistik/Montagen, nie Pixelgleichheit.
- **Wanduhr:** der Kern misst reale Zeit (`DoTime`); `EnforceMaxFPS` taktet
  windowed auf 60 fps, damit Sim-Zeit ≈ frame/60 bleibt. Unter Last driftet
  das — Langzeitvergleiche entsprechend einordnen.
- `warand()`/Preset-Zufall sind nicht gesät — Läufe sind untereinander nicht
  bit-stabil (wie AvsRef, dort `rand()`). **Ausnahme ab S80:** `--zufall-lumiviz`.

## `--zufall-lumiviz` (S80): gleicher Zufall wie LumiViz

Der Kern würfelt an drei Stellen, die ein Preset sichtbar prägen: die
Rausch-Texturen `noise_*`/`noisevol_*` aus der **Uhr**
(`plugin.cpp:2480`, `srand(QueryPerformanceCounter ^ rand())`), dazu
`rand_preset` und die `rot_*`-Parameter bei jeder Ladung
(`state.cpp:1875`). Bei Presets, die davon leben, zeigen zwei Läufe
verschiedene Bilder — der Vergleich misst dann den Würfel (Befund S80 an
`martin - stratospheric turbulences`: Ref↔Ref MAE 0,05–0,08).

Mit `--zufall-lumiviz` schreibt der Host nach dem Hochfahren die sechs
Rausch-Texturen und vor jedem Frame `rand_preset`/`rot_*` mit **denselben**
Werten, die LumiViz benutzt (gemeinsamer Header
`projects/apps/LumiViz/include/visualizers/milkdrop/MilkdropZufall.hpp`).
Der Kern selbst bleibt unangetastet — derselbe Weg wie `--saat`. Das Log
nennt den Ladungs-Seed und `rand_preset`; LumiViz schreibt dieselbe Zeile ins
Trace-Log. Wirkung am Befund-Preset: Ref↔Ref 0,003–0,009.

Zusätzlich setzt `--zufall-lumiviz` den EEL-`rand()`-Strom direkt vor
`LoadPreset` auf den Prozessstart zurück: der Kern lädt beim Hochfahren schon
ein Startpreset, dessen `per_frame_init` Züge verbraucht. Dafür gibt es die
Messkopie `patched/nseel-cfunc.c` (zählt die Züge, `MILKDROPREF_RAND_LOG=N`
loggt die ersten N; einziger Eingriff `lumi_rand_reset()`).

`--mesh WxH` (S80) setzt das Warp-Mesh: MilkDrop3 legt 64×64 fest im Code
fest, das Winamp-Original und LumiViz rechnen 32×24.

**Grenzen:** `rand_frame` würfelt der Kern mitten im Frame neu (für den Host
unerreichbar). Das EEL-`rand()` der Skripte ist im Kern ohnehin
deterministisch (Mersenne-Twister, fester Startwert `0x4141f00d`,
`nseel-cfunc.c:42`) — LumiViz bildet es nach (`NseelZufall.hpp`, die ersten
Werte stimmen exakt); offen ist, in welcher Reihenfolge die Gitterpunkte im
`per_pixel`-Code ziehen (`Offene_Punkte.md` §3).
