# Changelog — Session 68 (2026-08-03…05)

> Anlass: Shader-Tutorial-Serie (paralleler Doku-/Asset-Strang, kein
> App-Code). Tests unberührt (Stand S67: 512 Cases grün). Verifikation über
> die Standalones: 121 Tutorial-Chains + 6 Heart-Equation-Varianten gerendert,
> alle Warnungen=0 / schwarz=nein.

## Neu

- **Tutorial-Serie komplett in `docs/tutorials/`** (Umzug aus
  `asset/effectchain/shadertoys/`, FNM-01-Namen): 9 Tutorials (PyramidSpiral,
  CrystalLights, StratosphericTunnel, SpaceDebris, PimpedKaleidoscope,
  Juggernaut, CompositePortals/-Postfx/-Transitions) + Kategorie-Overview
  (`ShaderTutorials-overview.md`: Taxonomie, Dokumentenlandkarte + Coverage,
  Lesehilfe, Technik-Index ~45 Einträge) + `Raymarching-reference.md`
  (Blueprint-System Weg C). Alle Tutorials nach Tutorial_Base formalisiert
  (Header, SMART-Lernziele, End-Validierung, Fehlerbehebung, Changelogs 1.2.0)
  und mit Standalone-Screenshots bebildert (je `<name>_schritte/` als
  lauffähige Chains, Markdown = SSOT). INDEX.md 2.2.0.
- **`asset/experiment/` — Heart-Equation-Paket:** die Threads-Formel
  `y = |x|^(2/3) + 0.9·sin(kx)·√(3−x²)` als 2D-Shadertoy (+`.lvfx`),
  3D-Multipass-Shadertoy (Zufallsrotation je Achse mit Stillstand-Phasen ×
  Audio, Raumbewegung, Farb-Stellschrauben), Milkdrop-Presets v1/v2 und
  **echte `.avs`-Binärdateien** v1/v2 (nach AvsParser-Layout geschrieben) —
  alle gegengerendert.

## Befunde (dokumentiert in Offene_Punkte 1.39/1.40 + Tutorial-Abspännen)

- Shadertoy-Buffer-FBOs filtern **GL_NEAREST** statt LINEAR (Sharpen-Feedback
  explodiert; `lesBilinear`-Workaround in den generierten Chains; App-Fix als
  Backlog §7).
- Standalone-Testsignal **sättigt die FFT-dB-Skala** → Absolut-Beat-Gates
  dauer-offen; generierte Chains nutzen die App-Uniforms (B-Regel der Serie).
- Sim-Uhr läuft im `--auto`-Batch über die Preset-Reihenfolge durch;
  Beat-Läufe sind nicht frame-deterministisch.
- Erster shadertoy.com-Sichttest (Transitions): kompiliert; Stolperer
  „wechselt nie" = fehlende iChannel-Selbstreferenz → als Fehlerbehebungs-
  Zeile ins Tutorial übernommen.
- Nachstimm-Kandidaten bestätigt: Tunnel-Schritte 3–7 dunkel,
  Juggernaut-dark heller als Prosa, Postfx-`SCHWELLE` 0.7 in dark leer.

## Arbeitsliste

- `Offene_Punkte.md` **1.40.0**: NEU ⚪ §7 Buffer-FBO-Filter (Fix-Rezept) ·
  NEU ⚪ §7 **Editor-Komfort Apply + Beautify** (Spezifikation; Milkdrop-
  Zeilen-Roundtrip nur bei `.milk`-Rückexport nötig — als `.lvfx` entfällt
  er). Nächster Anstoß laut Patrik: Editor-Komfort als eigene Session.
