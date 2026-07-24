# Changelog — Session 43 (2026-07-23)

Sichttest-Nachzüge (Basis-Waveform, Icons, Auge-Toggles), der
Loudness-Zufuhr-Fix („Rock The House schwarz"), **Shader-Stufe C3 komplett**
(HLSL→GLSL-Übersetzer deckt jetzt 98–99 % der Preset-Shader ab),
Bild-Suche über die Asset-Ordner + **.lvfx-Bild-Einbettung**, und das neue
Diagnose-Werkzeug **AvsStandalone**. Tests am Ende: **409 Cases grün,
0 Skips**; Builds VS-Debug/-Testing (`/WX`) + Ninja-Clang-Release grün.

## Behoben

- **Beat-getriebene Milkdrop-Presets blieben schwarz** (z. B. „Rock The
  House"): Im Effect-Chain-Betrieb kam das Spektrum nie bei der
  Band-Loudness an — bass/mid/treb standen konstant auf 1.0, alles mit
  `above(bass, …)`-Gates blieb unsichtbar. Die Loudness liest jetzt die
  Stereo-Kanaldaten (mit Mono-Fallback); zur Diagnose schreibt der Kern
  alle ~5 s eine `loudness:`-Zeile ins Trace-Log.
- **Texturen aus dem Asset-Pack wurden nicht gefunden** (z. B. `lines2` —
  lag in `sprites/`, gesucht wurde nur in `textures/`; Presets in
  Unterordnern fanden gar nichts): Die Suche läuft jetzt vom Preset-Ordner
  aufwärts über `textures/`, `sprites/` und den Ordner selbst.
- **Import-Meldungen „nicht übersetzbar"/GL-Fehler bei vielen
  Community-Presets** (Schleifen, tex3D, `#if`, `float2x3`, Zuweisungen an
  q-Variablen, `mod`/`noise3` als Variablennamen, …) — siehe „Neu: Stufe C3".

## Neu

- **Shader-Stufe C3 komplett:** Der HLSL→GLSL-Übersetzer beherrscht jetzt
  Schleifen (for/while/do-while, break/continue, ++/--), Arrays, `tex3D`
  mit echten 3D-Volumen-Noise-Texturen, `#if`-Blöcke, out-Parameter,
  Nicht-Quadrat-Matrizen, alle `mul()`-Formen, die include.fx-Konstanten
  und diverse fxc-Eigenheiten (stilles Vektor-Kürzen, Schreibzugriffe auf
  globale Konstanten, bool-Arithmetik, GLSL-Namenskollisionen).
  **Abdeckung: warp 98,6 %, comp 99,5 %** über beide Preset-Packs; im
  311er-Pack kompilieren alle Custom-Shader fehlerfrei (2 bewusste
  Fallbacks: `rot_*`-Rotationsmatrizen, 1 defektes Preset).
- **Milkdrop-Editor:** Die immer gerenderte **Basis-Waveform** hat jetzt ein
  eigenes Element unter „Waves" (vorher versteckt in den Parametern);
  Wave-/Shape-Elemente haben **Auge-Toggles**; Import-Browser und alle
  Milkdrop-Unterknoten zeigen **Format-Icons** (AVS/MilkDrop/LumiViz).
- **.lvfx mit eingebetteten Bildern:** Beim Speichern werden die aktuell
  referenzierten Texturen/Sprite-Bilder in die Datei eingebettet — die
  Datei ist damit portabel. Bilder kommen weiterhin aus den Asset-Ordnern
  (Dateien haben Vorrang); nicht mehr referenzierte Einbettungen werden
  beim Speichern automatisch entfernt.
- **AvsStandalone:** Eigenständiges Testfenster für den AVS-Renderpfad
  (wie MilkdropStandalone) — Batch-Läufe mit Screenshots/Statistik,
  Ketten-Dump als JSON und `.lvfx`-Laden für die Preset-Bisektion.
  Erster Korpus-Lauf: 35/35 Presets laden, 10 rendern schwarz →
  Startliste für die AVS-Kalibrier-Runde.

## Bekannt / offen

- **Wormhole sieht weiterhin falsch aus** (Patrik-Sichtbefund): Die
  auffälligen „Balken" sind zwar Original-Verhalten des Presets
  (255-px-Linien als Wirbel-Material), aber das Gesamtbild passt noch
  nicht. Belegte Original-Abweichungen für die nächste Runde: `d`/`r` der
  Movement-Skripte im Pixel- statt NDC-Raum; Set-Render-Mode-Zustand wird
  im Original je Frame zurückgesetzt.
- Sichttest mit Musik nach dem Loudness-Fix steht aus; ebenso die
  E6-Visual-Playlist (Konzept braucht das Update auf Host-Gruppen).
