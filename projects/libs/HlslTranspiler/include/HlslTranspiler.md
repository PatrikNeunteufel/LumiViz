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

`test_HlslTranspiler.cpp`: 26 Fixture-Cases (Promotions, Makros, Casts,
Funktionen, Schleifen, Arrays, tex3D, Konstanten, #if, float2x3, out-Params)
+ **Korpus-Gate** über beide Preset-Packs: **warp 566/574 (98,6 %), comp
595/598 (99,5 %)** — Rest: `rot_*`-Rotationsmatrizen (1) + defekte
Preset-Texte. 311er-Pack-Sweep: **0 GL-Kompilierfehler, 2 Fallbacks**.

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 40, C1): Präprozessor, Lexer/Parser/CodeGen, Typ-Inferenz + Promotions, Intrinsic-Tabelle, Funktionsdefinitionen, Korpus-Gate |
| 1.1.0 | 2026-07-23 | Session 43 (GreatWho-Sichttest-Befund): **implizite HLSL-Verengung auch bei Compound-Zuweisungen** (`ret -= tex2D(...)`, `ret.xy *= GetBlurN(...)` — fxc kürzte still, GLSL lehnte ab → stiller MD1-Fallback erst zur GL-Laufzeit) + **log10** (→ `log(x)·1/ln 10`, komponentenweise); comp-Korpus 409→410 |
| 1.2.0 | 2026-07-23 | Session 43: **C3-Kern** (Import-Fehler-Befund Patrik, 311er-Pack) — `for`/`while` + `break`/`continue` + `++`/`--` (Präfix/Postfix) · `tex3D` auf die eingebauten `noisevol_lq/hq` (auch als `sampler` redeklariert; 3D-Texturen + Uniforms liefert der Host) · include.fx-Konstanten `M_PI`/`M_PI_2`(=2π)/`M_INV_PI_2` + rohe q-Bänke `_qa`–`_qh` · Alias-`#define` auf Funktionsnamen (`#define sat saturate`; Body-Trim) · **HLSL-Truncation in Intrinsics** (gemischte Vektor-Breiten → kleinste Breite) · Vektor-Vergleiche (→ `lessThan()`-Familie als 0/1-Vektor) · `float↔bool`-Konvertierungen · `smoothstep`. Korpus: warp 526→**563/574 (98 %)**, comp 509→**566/598 (95 %)**. C3-Rest (sauberer Fehler): Arrays, do/while, `#if`, float2x3, modf, out-Parameter |
| 1.3.0 | 2026-07-23 | Session 43: **C3-Rest — Stufe C3 abgeschlossen** (Entscheid Patrik): Arrays (lokal/global, Index mit `int()`-Cast, `{…}`-Init) · `#if`/`#ifdef`/`#else`/`#endif` (konstante Bedingungen) · do/while · out/inout-Parameter (GLSL-nativ, L-Value-Durchreichung) · float2x3/float3x2 (transponierte Konstruktor-Emission, GLSL matCxR) · `mul()`-Formen inkl. Vektor·Vektor = dot · **Uniform-Schattenkopien generalisiert** (fxc erlaubte Schreibzugriffe auf globale Konstanten: `q25=…`, `texsize.xy=…`) · **GLSL-Reserviert-Namen-Aliase** (`mod`/`noise3`/`noise4`/… als Preset-Bezeichner → `_hl`-Suffix) · Statement-Makros ungeklammert · Bool in Arithmetik/Unary numerisch · `%`-Harmonisierung · Skalar→Matrix-Splat · `round`, `modf`, `smoothstep`. **Korpus: warp 566/574 (98,6 %), comp 595/598 (99,5 %); 311er-Sweep: 0 GL-Fehler, 2 Fallbacks** (rot_* ×1, defektes Preset ×1) |
