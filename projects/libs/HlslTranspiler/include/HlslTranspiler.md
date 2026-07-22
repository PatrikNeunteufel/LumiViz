# HlslTranspiler — HLSL(ps_2/3-Teilmenge) → GLSL 330 (Import-Zeit)

> **Version:** 1.0.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Stufe C1, Session 40 — Entscheid E4)  
> **Modul:** lumi::hlsl (Lib **HlslTranspiler**, header-only INTERFACE)  
> **Dateien:** HlslTranspiler.hpp  
> **Namespace:** lumi::hlsl (detail: lumi::hlsl::detail)  
> **Abhängigkeiten:** keine (kein Qt, kein GL) — Text → Text, voll unit-testbar  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Übersetzt NUR den Preset-Anteil eines MilkDrop-warp_/comp_-Shaders
(Deklarationen + Funktionen + `shader_body { ... }`) nach GLSL — die Umgebung
(include.fx: sampler_*, GetBlurN, q1–q32, bass/…, rand_*) stellt der Host als
GLSL-Präambel (`milkCustomPreamble()` in MilkdropVisualizer.cpp).

**Pipeline:** `preprocess` (#define-Makros, Objekt- + Funktionsform) → Lexer
(2-Token-Lookahead) → Parser (Präzedenz-Kaskade; Deklarationen, if/else,
return, Funktionsdefinitionen mit Semantics-Skip) → CodeGen mit
**Typ-Inferenz** (float/vec2–4/mat2–4/bool/Sampler) und GLSL-Promotions:
Skalar→vecN bei Zuweisung/Deklaration, HLSL-Implicit-Truncation (vecN op vecM
→ min), pow/mix-Argument-Promotion, numerische Bedingungen → `!= 0.0`,
Int-Literale → Float, `f`-Suffixe weg, C-Style-Casts → Konstruktoren,
`tex2D`→`texture` (fxc-case-insensitiv), `mul`→`*`, `saturate`→`clamp`,
Globals → Null-Init global + Initialisierung am main-Anfang (GLSL verlangt
konstante Global-Initialisierer).

**Bewusst Stufe C3 (klarer Fehler):** for/while, Arrays, tex3D/Volumen-Noise,
out-Parameter, Vektor-Vergleiche, Nicht-#define-Direktiven.

## 2. API

- `transpile(text, ShaderKind::Warp|Comp) → HlslResult{ok, glslBody,
  glslGlobals, customSamplers, customTexsizes, usesTex3d, error}`
- Fehler mit Zeilennummer („Zeile N: …"); der Host baut daraus die
  Import-Report-Meldung und fällt auf den MD1-Pfad zurück.

## 3. Tests

`test_HlslTranspiler.cpp`: 18 Fixture-Cases (Promotions, Makros, Casts,
Funktionen, C3-Grenzen) + **Korpus-Gate** über beide Preset-Packs:
**warp 462/574 (80 %), comp 409/598 (68 %)** — Rest überwiegend C3.

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 40, C1): Präprozessor, Lexer/Parser/CodeGen, Typ-Inferenz + Promotions, Intrinsic-Tabelle, Funktionsdefinitionen, Korpus-Gate |
