# MeshWarpWrapper — GPU-Vertex-Warp des Mesh-Warp-Nodes

> **Version:** 1.0.0
> **Datum:** 2026-08-06
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Strang G1, Session 69)
> **Modul:** lumi::meshwarp (freie Funktionen)
> **Dateien:** MeshWarpWrapper.hpp (header-only)
> **Namespace:** lumi::meshwarp
> **Abhängigkeiten:** keine (kein GL, kein Qt — Text/Zahlenfelder, testbar)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Textbausteine + Gitter-Erzeugung des **Mesh-Warp-Nodes** (`MeshWarpParams`,
`Regelwerk_und_Neue_Module_Plan.md` §G1): die GPU-Antwort auf die
Movement-Klasse. Eine Nutzer-GLSL-Funktion `vec2 warp(vec2 uv)` läuft je
GITTER-VERTEX im Vertex-Shader und liefert die QUELL-UV, an der das aktuelle
Chain-Bild abgetastet wird (MilkDrop-Warp-Semantik: das Gitter bleibt, die
Quelle wandert). Zustandslos-parallel, modernes Regelwerk — Legacy-Imports
bleiben CPU-Mesh (Sequenz-Vertrag des EEL, Offene_Punkte §7-Notiz).

**Entscheid Patrik (S69):** G1 startet VOR der Vereinheitlichung V2 — Audio
kommt ad-hoc als Uniforms nach dem Shadertoy-Muster (`computeAudioBands` +
`m_audioLevel` + `m_frameBeat`); V2 zentralisiert später nur die Quelle,
der Uniform-Vertrag des Nodes bleibt.

## 2. API-Kern

- `wrapVertex(userCode)` — GLSL-330-Vertexprogramm: Prälude (Uniforms
  `uResolution/uTime/uDelta/uFrame` + `bass/mid/treb/vol/beat`, endet mit
  `#line 1` → Treiberfehler tragen Nutzer-Zeilen), Nutzer-Code (leer =
  Identität), Epilog-`main()` (`#line 100000` = Wrapper-Zeilen). Eigene
  Namen im `_lumi`-Reserviert-Raum.
- `fragmentShader()` — tastet `uTex` an der gewarpten UV ab; `uWrap`
  (fract vs. clamp) IM Shader, damit die Filter-/Wrap-States der geteilten
  Chain-Surface unangetastet bleiben; `uMixAmount` mischt gegen die
  ungewarpte Abtastung (1 = ersetzen — dieselbe Textur, kein zweiter Pass).
- `starterWarp()` — Palette-Vorbelegung: Bass-Swirl + Atmung (sichtbar ab
  Frame 1, audio-reaktiv).
- `buildGridVertices(gx, gy)` / `buildGridIndices(gx, gy)` — Gitter 0..1,
  (gx+1)·(gy+1) Punkte, 6·gx·gy Triangle-Indizes, konsistente Windung.
- Klemmen: `kMinGrid`=2, `kMaxGridX`=256, `kMaxGridY`=192 (Plan §G1) —
  SSOT für Serializer-Leser, Panel-Spinner und Renderer.

## 3. Render-Pfad (MultiEffectVisualizer::runMeshWarp)

transformPass-Muster: Quelle = `active().current()`, Ziel = Partner, danach
Swap. Programm-Rebuild nur bei Code-Wechsel (Snapshot `mwCompiled`), Gitter-
Rebuild nur bei Auflösungs-Wechsel (VAO/VBO/IBO, Terrain-Muster). Für den
Draw wird der Quell-Filter auf LINEAR gestellt und danach WIEDERHERGESTELLT
(geteilte Surface — AVS-treue Effekte verlassen sich auf ihre Abtastung).
Kompilierfehler landen im geteilten `stError` → `shadertoyError(nodeId)`
versorgt Panel-Anzeige und den Apply-Poll des Groß-Editors (S69).
Parameter-Skript (Strang D): `gridx`, `gridy`, `mixamount`.

## 4. Tests

`tests/unit/UnitTests/test_MeshWarpWrapper.cpp` — 9 Cases: #line-Vertrag,
Identitäts-Fallback, Uniform-Satz, Starter, Gitter-Zahlen/Randwerte/Windung,
Serializer-Roundtrip aller Felder, Leser-Klemmen. Sichtbeweis:
`asset/effectchain/meshwarp_sonde.lvfx` (Sinus-Warp über Linien-Gitter,
AvsStandalone `out/meshwarp_sonde_s69/`, Warnungen=0).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-06 | Erstfassung — Strang G1 (Session 69) |
