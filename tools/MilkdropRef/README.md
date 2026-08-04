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
tools/MilkdropRef/build/Release/MilkdropRef.exe <preset.milk|ordner> [--frames N] [--out DIR] [--size WxH] [--silence] [--show]
```

- **Audio:** synthetisch, formelgleich zu `MilkdropStandalone::feedSyntheticAudio`
  (t = frame/60, Sinus 220 Hz + 120-BPM-Puls), als 576-Sample-8-bit-PCM mit
  128-Mitte (`AnalyzeNewSound`-Kontrakt). `--silence` = Null-Signal.
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
  bit-stabil (wie AvsRef, dort `rand()`).
