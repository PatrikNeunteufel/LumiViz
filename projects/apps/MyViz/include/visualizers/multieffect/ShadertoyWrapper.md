# ShadertoyWrapper — GLSL-ES→330-Wrapper des Shadertoy-Nodes

> **Version:** 1.1.0
> **Datum:** 2026-08-02
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Strang S, S1–S4 — Session 65)
> **Modul:** `lumi::shadertoy` (header-only, GL-frei)
> **Dateien:** ShadertoyWrapper.hpp
> **Abhängigkeiten:** keine (Text → Text; Host: MultiEffectVisualizer::runShadertoy)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Bettet einen Shader in Shadertoy-Konvention (`mainImage(out vec4, in vec2)`)
in ein GLSL-330-core-Fragmentprogramm ein (Plan: `Regelwerk_und_Neue_Module_Plan.md`
§S1/S2). Der Node ist ein „Modern"-Bürger erster Klasse — kein Legacy-Ballast.

- **Prälude:** voller Shadertoy-Uniform-Satz (`iResolution, iTime, iTimeDelta,
  iFrame, iFrameRate, iMouse, iDate, iSampleRate, iChannelTime[4],
  iChannelResolution[4], iChannel0–3`) + LumiViz-Audio-Uniforms
  (`bass, mid, treb, vol, beat`) + Wrapper-Interna (`_lumi*`-Namensraum).
- **`#line 1`** direkt vor dem Nutzer-Code: GL-Kompilierfehler tragen die
  Zeilennummern des NUTZER-Codes (Panel zeigt sie); Wrapper-Zeilen sind per
  `#line 100000` unverwechselbar.
- **Epilog:** `main()` ruft `mainImage(c, gl_FragCoord.xy)` und blendet über
  `_lumiBlend` (0 ersetzen · 1 additiv · 2 50/50) gegen `_lumiPrev`
  (aktuelles Chain-Bild) — ein Pass, FBO in Chain-Auflösung.
- **`starterShader()`:** eigener audioreaktiver Beispiel-Shader für neue Nodes
  (KEIN Shadertoy-Inhalt — Lizenz-Vorbehalt CC BY-NC-SA, Plan §S3).

Host-Verträge (MultiEffectVisualizer): `iTime` = deterministische Sim-Uhr
(`m_scriptClock`, Batch-Renderer-tauglich), `iFrame` = Frames seit Kompilierung,
Audio = geteilte 512×2-R8-Textur (Zeile 0 FFT-Spektrum, Zeile 1 Waveform
0,5+0,5·x) am per Param wählbaren `iChannel`; Kompilierung nur bei
Code-Wechsel (Snapshot-Vertrag), Fehler ⇒ Passthrough + `shadertoyError()`.

**Multipass (S4, 1.1.0):** Buffer A–D als `ShadertoyPass` (Code + 4
Kanal-Bindungen; Kodierung: −1 nichts, 0..3 Buffer A..D, 4 Audio — SSOT
`imageInput`/`pass.input`, das frühe `audioChannel`-Feld ist reine
Lese-Migration). Buffer rendern je Frame A→D in RGBA32F-Ping-Pongs
(Chain-Auflösung, `wrapBufferFragment` = roher Epilog ohne Blend/Clamp);
nach jedem Pass wird geswapt — Selbst-/Vorwärts-Referenz liest damit das
Vorframe, Rückwärts-Referenz das frische Bild (Original-Semantik). Der
URL-Import löst die Buffer-Topologie über die Output-Ids des API-JSON auf;
`common` wird ALLEN Pässen vorangestellt.

**Grenzen:** keine Video-/Cubemap-/Keyboard-/Textur-Inputs (Platzhalter +
Report), `iMouse` = 0, `iDate` deterministisch, Sound-Pässe übersprungen.
PORT-Annahme Audio-Skala: Original ist dB-skaliert (getByteFrequencyData),
wir speisen die lineare App-Skala — Feinabstimmung per Sonde offen.

## 2. Tests

`test_ShadertoyWrapper.cpp`: Prälude-Vertrag (Uniform-Satz, #line-Positionen),
Einbettung ohne Schluss-Newline, Starter-Inhalt, ChainSerializer-Roundtrip
(inkl. Metadaten + Default-Fallbacks). Sonden: `asset/effectchain/shadertoy_
{uvgrad,zeit,audioringe}.lvfx` (Ergebnis S65: uvgrad punktgenau, Orientierung
y-up korrekt, Zeit-Puls deterministisch, Ringe audio-moduliert).

## 3. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.1.0 | 2026-08-02 | S4 Multipass (Session 65): `wrapBufferFragment` (roher Epilog), `ShadertoyPass` + Kanal-Bindungs-Kodierung (SSOT imageInput/input, audioChannel nur noch Lese-Migration), RGBA32F-Ping-Pong je Buffer im Host (Swap nach jedem Pass = Original-Lese-Semantik), Import löst Buffer-Topologie über Output-Ids, common in allen Pässen; Panel: 4 Bindungs-Combos je Pass + Buffer-Editoren (hinzufügen/entfernen). Sonde shadertoy_feedback.lvfx (Lissajous-Trail beweist das Vorframe-Lesen) |
| 1.0.0 | 2026-08-02 | Erstfassung (Session 65, Strang S1+S2): Prälude/Epilog/Starter, #line-Vertrag, Blend-Epilog; Host-Integration als Chain-Node (`ShadertoyParams`, Serializer-Typ „shadertoy", Panel-Editor mit Fehleranzeige, 512×2-Audio-Textur + LumiViz-Uniforms) |
