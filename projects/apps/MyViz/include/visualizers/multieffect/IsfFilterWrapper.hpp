/**
 ****************************************************************************************
 * @file   IsfFilterWrapper.hpp
 * @brief  Die ISF-Sprache als GLSL-330-Prälude — Vertex und Fragment für den
 *         `isfFilter`-Knoten (S72)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Der `isfFilter`-Knoten führt ISF-Shader **unverändert** aus. Möglich macht
 * das dieser Wrapper: er stellt die ISF-Sprache bereit, statt sie in unsere
 * zu übersetzen (Entscheid Patrik S72 — dadurch entfielen im Importeur alle
 * Ablehnungen und jedes Umschreiben, s. `IsfImport.hpp`).
 *
 * ### Der Kniff: `#define main isf_main`
 *
 * Ein ISF-Shader bringt sein eigenes `main()` mit — wir können also kein
 * zweites danebenlegen, brauchen aber eines für Blend und Mix. Die Prälude
 * benennt das Nutzer-`main` per Präprozessor um, der Epilog hebt das `#define`
 * auf und definiert das echte `main()`.
 *
 * Warum nicht im Text ersetzen: ein `#define` **verschiebt keine Zeile**. Der
 * `#line 1`-Vertrag bleibt exakt, Treiber-Fehler zeigen weiter auf die Zeile,
 * die der Nutzer im Editor sieht. `mainImage` oder `domain` sind eigene
 * Token und bleiben unangetastet — der Präprozessor arbeitet auf Token, nicht
 * auf Zeichenketten.
 *
 * ### Was die Prälude sonst bereitstellt
 *
 * | ISF | hier |
 * |---|---|
 * | `gl_FragColor` | `#define` auf die 330-Ausgabevariable (die es in Core nicht mehr gibt) |
 * | `texture2D()` | `#define` auf `texture()` (GLSL-1.2-Erbe vieler Dateien) |
 * | `IMG_THIS_PIXEL` · `IMG_THIS_NORM_PIXEL` · `IMG_NORM_PIXEL` · `IMG_PIXEL` · `IMG_SIZE` | Makros über die Bild-Sampler |
 * | `RENDERSIZE` (vec2) · `TIME` · `TIMEDELTA` · `FRAMEINDEX` · `DATE` | Uniforms |
 * | `isf_FragNormCoord` · `vv_FragNormCoord` | Varying aus der Vertex-Stufe |
 * | `isf_vertShaderInit()` | Makro in der Vertex-Prälude |
 * | jeder `INPUTS`-Regler | ein Uniform seines Typs |
 *
 * **Die Regler sind echte Uniforms, kein `const`-Block:** ein Zug am Schieber
 * setzt nur einen Uniform-Wert und kostet keine Neuübersetzung des Shaders.
 *
 * ### Geometrie
 *
 * Drei Bauarten (Entscheid Patrik S72), Attribut `_lumiPos` als
 * **Clip-Koordinate −1..1** — dieselbe Konvention wie `aPos` des geteilten
 * Quad-VAO, damit der Vorgabe-Fall ohne eigenen Puffer auskommt:
 * **Quad** (ISF-treu, Vorgabe) · **ein übergroßes Dreieck** (keine Diagonale
 * ⇒ keine geknickte Varying-Interpolation) · **Gitter** (echte Verformung).
 * Die Bauart bestimmt nur, was gezeichnet wird — der Shader-Text ist
 * derselbe.
 *
 * Vertragsgrenze wie `PixelFilterWrapper`: Text → Text, kein GL, keine GUI —
 * voll unit-testbar.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <string>
#include <vector>

namespace lumi::isffilter {

/// Name des Ausgabe-Uniforms, unter dem der Host die Regler setzt: der
/// `key` des `ParamWert` IST der Uniform-Name (so heisst er im Shader).
/// Eigene Namen des Wrappers tragen das Praefix `_lumi` — Reserviert-Raum.

/// Alle `IMG_*`-Makros und ISF-Groessen. Steht in BEIDEN Stufen, weil
/// ISF-Vertex-Shader dieselben Groessen benutzen (`RENDERSIZE`,
/// `isf_FragNormCoord`).
[[nodiscard]] inline std::string isfGemeinsam()
{
    return R"(uniform vec2 RENDERSIZE;
uniform int PASSINDEX;
uniform float TIME;
uniform float TIMEDELTA;
uniform int FRAMEINDEX;
uniform vec4 DATE;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float beat;
// AVS-Vokabular auch im Shader (Wunsch Patrik S72): dieselben Namen und
// dieselbe Bedeutung wie in den EEL-/Lua-Skripten, damit niemand umlernen
// muss, nur weil der Effekt auf der GPU laeuft. Die Texturen liegen IMMER
// an — anders als die ISF-Audio-Eingaenge, die ein Shader erst deklarieren
// muesste.
// `band` 0..1 · `width` 0..1 (Fenster, ueber das gemittelt wird).
// `channel` gibt es der AVS-Signatur wegen; unsere Analyse ist heute MONO,
// der Wert wird also nicht ausgewertet.
uniform sampler2D _lumiAudioWave;
uniform sampler2D _lumiAudioFft;
float _lumiAudioMittel(sampler2D _tex, float band, float width)
{
    float h = max(width, 0.0) * 0.5;
    float s = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        float t = clamp(band - h + 2.0 * h * (float(i) / 7.0), 0.0, 1.0);
        s += texture(_tex, vec2(t, 0.5)).r;
    }
    return s / 8.0;
}
float getosc(float band, float width, float channel)
{
    // Waveform liegt als 0.5 + 0.5*x in der Textur — zurueck auf -1..1.
    return _lumiAudioMittel(_lumiAudioWave, band, width) * 2.0 - 1.0;
}
float getspec(float band, float width, float channel)
{
    return _lumiAudioMittel(_lumiAudioFft, band, width);
}
#define vv_FragNormCoord isf_FragNormCoord
#define texture2D texture
// IMG_SIZE muss die Groesse DES SAMPLERS liefern, nicht die Bildgroesse:
// ein Shader bestimmt damit z. B. die Sample-Anzahl seines Audio-Eingangs
// (Befund S72 — pauschal RENDERSIZE gab dort die Bildbreite zurueck).
// `textureSize` kann GLSL 330 und ist fuer JEDEN Sampler exakt.
#define IMG_SIZE(_img) vec2(textureSize(_img, 0))
#define IMG_NORM_PIXEL(_img, _uv) texture(_img, _uv)
#define IMG_PIXEL(_img, _px) texture(_img, (_px) / RENDERSIZE)
#define IMG_THIS_PIXEL(_img) texture(_img, isf_FragNormCoord)
#define IMG_THIS_NORM_PIXEL(_img) texture(_img, isf_FragNormCoord)
)";
}

/**
 * @brief Kollidiert `name` mit einem Namen, den die Prälude selbst vergibt?
 *
 * Import-Kollisionsregel des Projekts (`ScriptBaseKeys.hpp`, Entscheid D2):
 * „Reserviert" wirkt **relativ zum Quellformat**. Ein Name mit Builtin-
 * Bedeutung im Quellformat wird nie umbenannt — er bindet ans Builtin, das
 * IST die Original-Semantik. Umbenannt wird nur, was dort KEINE hat.
 *
 * Auf ISF angewandt: `TIME`, `RENDERSIZE`, `PASSINDEX`, `isf_FragNormCoord`
 * sind **ISF-Builtins** und bleiben. `bass`/`mid`/`treb`/`vol`/`beat` sind
 * laut `kInjectedKeys` Builtins von **MilkDrop** bzw. (bei `beat`) **AVS** —
 * in ISF haben sie keine Bedeutung, ein gleichnamiger `INPUT` ist dort also
 * ein gewöhnlicher Nutzer-Name und weicht. `getosc`/`getspec` sind
 * AVS-Funktionsnamen, die wir zusätzlich in den Shader ziehen; für ISF gilt
 * dasselbe.
 *
 * @todo (S72, Einwand Patrik) Diese Liste ist eine ZWEITE QUELLE neben
 * `ScriptBaseKeys::kInjectedKeys`. Richtig wäre, die Tabelle dort um die
 * Herkunft `Isf` samt ISF-Builtins zu erweitern und hier
 * `collidesOnIsfImport()` zu fragen — dann steht die Zuordnung je Format an
 * EINER Stelle. Zu beachten dabei: die AVS-Regel vergleicht
 * case-INSENSITIV (EEL), GLSL ist case-SENSITIV — `time` und `TIME` sind
 * hier zwei verschiedene Namen.
 *
 * Im Vidvox-Korpus kommt eine Kollision (Stand S72) **kein einziges Mal** vor
 * — die Regel greift erst bei der ersten Datei, die es tut.
 */
[[nodiscard]] inline bool istPraeludenName(const std::string& name)
{
    static const char* kUnsere[] = {"bass",   "mid",     "treb",
                                    "vol",    "beat",    "getosc",
                                    "getspec"};
    for (const char* n : kUnsere)
    {
        if (name == n) return true;
    }
    // Alles mit unserem Praefix gehoert ohnehin dem Wrapper.
    return name.rfind("_lumi", 0) == 0;
}

/// Name, unter dem ein Regler/Sampler wirklich deklariert wird. Bei einer
/// Kollision bekommt er ein Praefix — der Nutzer-Code sieht ihn trotzdem
/// unter seinem eigenen Namen (ein `#define` biegt ihn um, s. `umbenennung`).
[[nodiscard]] inline std::string echterName(const std::string& key)
{
    return istPraeludenName(key) ? "_lumiIn_" + key : key;
}

/// Die `#define`-Zeile, die den Nutzer-Namen auf den echten umbiegt (leer,
/// wenn es keine Kollision gibt). So bleibt der ISF-Code UNVERAENDERT: er
/// schreibt weiter `bass`, gemeint ist dann sein eigener Regler — genau die
/// Semantik, die der Autor der Datei im Sinn hatte.
[[nodiscard]] inline std::string umbenennung(const std::string& key)
{
    if (!istPraeludenName(key)) return {};
    return "#define " + key + " _lumiIn_" + key + "\n";
}

/// Uniform-Deklarationen der Regler — der `key` ist der Name im Shader.
/// Nur die Wurzel-Werte: GLSL hat fuer Untergruppen keinen Namensraum.
[[nodiscard]] inline std::string uniformDeklarationen(
    const lumi::multieffect::ParamGruppe& g)
{
    using lumi::multieffect::ParamTyp;
    std::string s;
    for (const auto& w : g.werte)
    {
        if (w.key.empty()) continue;
        const std::string n = echterName(w.key);
        switch (w.typ)
        {
            case ParamTyp::Bool: s += "uniform bool " + n + ";\n"; break;
            case ParamTyp::Ganzzahl:
            case ParamTyp::Auswahl: s += "uniform int " + n + ";\n"; break;
            case ParamTyp::Punkt2D: s += "uniform vec2 " + n + ";\n"; break;
            case ParamTyp::Farbe: s += "uniform vec4 " + n + ";\n"; break;
            case ParamTyp::Text: continue;  // GLSL kennt keine Zeichenketten
            default: s += "uniform float " + n + ";\n"; break;
        }
        s += umbenennung(w.key);
    }
    return s;
}

/// Sampler-Deklarationen der Bildquellen — der Name ist der aus dem Shader
/// (`inputImage`, `startImage`, …). Eine Quelle ohne Namen wird ausgelassen.
[[nodiscard]] inline std::string samplerDeklarationen(
    const std::vector<lumi::multieffect::IsfBildQuelle>& quellen)
{
    std::string s;
    for (const auto& q : quellen)
    {
        if (q.name.empty()) continue;
        s += "uniform sampler2D " + echterName(q.name) + ";\n";
        s += umbenennung(q.name);
    }
    return s;
}

/**
 * @brief Komplettes Fragmentprogramm um einen ISF-`.fs`-Rumpf
 *
 * Aufbau: 330-Kopf · Varying · ISF-Sprache · Sampler · Regler-Uniforms ·
 * Blend-Uniforms · `#define main isf_main` · `#line 1` · NUTZER-CODE ·
 * Epilog mit dem echten `main()`.
 */
[[nodiscard]] inline std::string wrapFragment(
    const std::string& userCode,
    const std::vector<lumi::multieffect::IsfBildQuelle>& quellen,
    const lumi::multieffect::ParamGruppe& parameter)
{
    std::string s = R"(#version 330 core
in vec2 isf_FragNormCoord;
out vec4 _lumiFrag;
)";
    s += isfGemeinsam();
    s += samplerDeklarationen(quellen);
    s += uniformDeklarationen(parameter);
    s += R"(uniform sampler2D _lumiPrev;
uniform int _lumiBlend;
uniform float _lumiMixAmount;
vec4 _lumiIsfOut;
// `gl_FragColor` gibt es in 330 Core nicht mehr — der Nutzer-Code schreibt
// in eine gewoehnliche Variable, die der Epilog ausliest.
#define gl_FragColor _lumiIsfOut
// Das Nutzer-main() wird umbenannt, damit der Epilog das echte stellen kann.
// Ein #define verschiebt KEINE Zeile: der #line-1-Vertrag bleibt exakt.
#define main isf_main
#line 1
)";
    if (userCode.empty())
    {
        // Leer = Durchreichen. Ohne `main` gaebe es kein Programm zu linken.
        s += "void main() { gl_FragColor = IMG_THIS_PIXEL(_lumiPrev); }\n";
    }
    else
    {
        s += userCode;
        if (userCode.back() != '\n') s += '\n';
    }
    s += R"(
#line 100000
#undef main
void main()
{
    _lumiIsfOut = vec4(0.0);
    isf_main();
    vec4 _lumiAlt = texture(_lumiPrev, isf_FragNormCoord);
    vec3 _lumiNeu = mix(_lumiAlt.rgb, _lumiIsfOut.rgb,
                        clamp(_lumiMixAmount, 0.0, 1.0));
    if (_lumiBlend == 1)       _lumiNeu = min(_lumiAlt.rgb + _lumiNeu, vec3(1.0));
    else if (_lumiBlend == 2)  _lumiNeu = 0.5 * (_lumiAlt.rgb + _lumiNeu);
    _lumiFrag = vec4(_lumiNeu, 1.0);
}
)";
    return s;
}

/**
 * @brief Komplettes Vertexprogramm um einen ISF-`.vs`-Rumpf (leer erlaubt)
 *
 * `_lumiPos` kommt als Clip-Koordinate −1..1 herein (Konvention des geteilten
 * Quad-VAO) — daraus macht `isf_vertShaderInit()` `gl_Position` und das
 * 0..1-Varying. Der Epilog ruft die Initialisierung IMMER, auch wenn der
 * Nutzer sie vergisst; sie ist idempotent, ein zweiter Aufruf im Nutzer-Code
 * schadet also nicht.
 */
[[nodiscard]] inline std::string wrapVertex(
    const std::string& userCode,
    const lumi::multieffect::ParamGruppe& parameter)
{
    std::string s = R"(#version 330 core
layout(location = 0) in vec2 _lumiPos;
out vec2 isf_FragNormCoord;
)";
    s += isfGemeinsam();
    s += uniformDeklarationen(parameter);
    s += R"(#define isf_vertShaderInit() { isf_FragNormCoord = _lumiPos * 0.5 + 0.5; gl_Position = vec4(_lumiPos, 0.0, 1.0); }
#define vv_vertShaderInit isf_vertShaderInit
#define main isf_main
#line 1
)";
    if (!userCode.empty())
    {
        s += userCode;
        if (userCode.back() != '\n') s += '\n';
    }
    else
    {
        // Kein eigener Vertex-Code: nur die Standard-Initialisierung.
        s += "void main() { isf_vertShaderInit(); }\n";
    }
    s += R"(
#line 100000
#undef main
void main()
{
    isf_vertShaderInit();
    isf_main();
}
)";
    return s;
}

/// Starter-Shader eines frisch eingefuegten Knotens: ein gueltiger
/// ISF-Filter, der ab dem ersten Frame etwas Sichtbares tut und den
/// Vertrag vorfuehrt.
[[nodiscard]] inline const char* starterFragment()
{
    return R"(// ISF-Filter: der Code laeuft unveraendert, so wie auf isf.video.
// Bildquellen holst du dir ueber IMG_THIS_PIXEL(<Name aus der Quellen-Liste>).
// Groessen: RENDERSIZE, TIME, TIMEDELTA, FRAMEINDEX
// LumiViz-Extras: bass, mid, treb, vol, beat
void main()
{
    vec4 quelle = IMG_THIS_PIXEL(inputImage);
    float welle = 0.5 + 0.5 * sin(TIME + isf_FragNormCoord.y * 12.0);
    gl_FragColor = vec4(quelle.rgb * (0.6 + 0.4 * welle) + bass * 0.2, 1.0);
}
)";
}

}  // namespace lumi::isffilter
