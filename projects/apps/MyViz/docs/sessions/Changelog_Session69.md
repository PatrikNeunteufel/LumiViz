# Changelog Session 69 (2026-08-05/06)

> Editor-Komfort komplett + Strang G KOMPLETT (G1 Mesh-Warp, G2 GPU-Partikel)
> + 28 Werks-Voreinstellungen + 5 Demos. Tests 554 grün, alle 3 Builds grün.

## Editor-Komfort (Offene_Punkte §7, Entscheid S68)

- **Apply im Groß-Editor** (EelScriptEditing 1.1.0, `ScriptEditorHooks`):
  übernehmen + recompilen ohne Schließen — Live-Tuning gegen den Viewport.
  Fehler IM Dialog: EEL/HLSL synchron per Transpiler-Probe (Dialekt-bewusst),
  Shadertoy-GL per Nach-Polling auf `shadertoyError` (#line ⇒ Nutzer-Zeilen).
- **Beautify**: NEU `include/scripting/ScriptFormatter.hpp` (pur) — EEL ein
  Statement pro Zeile mit Klammertiefen-Einzug, GLSL/HLSL Brace-Re-Indent;
  Whitespace-only-Vertrag testerzwungen (Beautify kann nie Semantik ändern).
- **Settings-Tab „Editor"**: Einzugsbreite, Operator-Abstände (EEL),
  Leerzeilen-Klemme (QSettings `editor/...`).
- **Import…/Export…** an allen Shader-Feldern: Datei ↔ Editor (UTF-8),
  Vorschlag `preset_name.modul.glsl`, Ordner-Gedächtnis.

## Strang G (Plan 1.6.0 — Entscheid: direkt statt nach Vereinheitlichung V2)

- **G1 `meshWarp`**: Nutzer-GLSL `vec2 warp(vec2 uv)` je Gitter-Vertex im
  Vertex-Shader = Quell-UV-Remap des Chain-Bilds (Gitter 2..256×192, Mix,
  Wrap/Clamp im Shader). NEU `MeshWarpWrapper.hpp` (GL-frei, 9 Tests).
- **G2 `gpuParticles`**: bis 65536 Partikel per Instancing; Zustand = ein
  RGBA32F-Texel (pos+vel) im Ping-Pong; Alter/Respawn hash-basiert OHNE
  Speicher (deterministisch, kein rand()); 17 Regler, Parameter-Skripte,
  optionales Kraftfeld-GLSL mit Audio-Uniforms. NEU `GpuParticlesWrapper.hpp`
  (GL-frei, 7 Tests).
- Beide: Palette „— GPU-Module —", Fehler ins geteilte `stError` (Panel +
  Apply-Poll), Serializer mit Leser-Klemmen, FieldDocs/Inventar 88 Typen /
  756 Felder. Sonden `meshwarp_sonde.lvfx` + `gpuparticles_sonde.lvfx`
  (Render-Beweise, Warnungen=0).

## Inhalte

- **28 Werks-Voreinstellungen** (`asset/nodepresets/`, Wächter-Test grün):
  meshWarp (5), gpuParticles (5), Batch 1 über 12 Bestandstypen (18 — list
  Beat-Gate/Bass-Blende/Puls-Layer/AB-Wechsler, brightness, colorfade,
  colorModifier, mosaic, channelShift, colorClip, multiFilter, addBorders,
  onBeatClear, clear, bufferSave). Sichttest der Formeln offen.
- **5 Demos** (`asset/examples/`): meshWarp Bass-Tunnel / Wellengang /
  Spiegelkabinett, gpuParticles Feuerfontaene / Wirbelnebel (kombiniert
  beide GPU-Module) — alle gegengerendert + sichtgeprüft.

## Kleinfixes & Befunde

- S53-Clang-Warnung (this-Capture) raus; Voll-Rebuild zeigt weitere
  BESTANDS-Warnungen (nicht aus S69, Kleinaufgabe).
- `/bigobj` für `test_ChainSerializer.cpp` (C1128 Debug, Varianten-Wachstum).
- Befund-Klärung: Mesh Warp alleine = schwarz ist KORREKT (Transform braucht
  Quelle) → NEU ⚪ Palette-Rollen-Kennzeichnung (eigene Session).
- NEU ⚪ Szenen-Wechsler-Konzept (Idee Patrik): Szenen-IDs an Listen
  beliebiger Tiefe, Fading, Re-Init; Trigger bis Strophe/Refrain per
  Offline-Pre-Analyse — eigene Konzept-Session.

## Doku

ScriptFormatter.md + MeshWarpWrapper.md + GpuParticlesWrapper.md NEU
(CppModuleDoc) · EelScriptEditing 1.1.0 · MultiEffectPanel.md ·
Regelwerk_und_Neue_Module_Plan 1.6.0 · Offene_Punkte 1.41.0→1.47.0.
