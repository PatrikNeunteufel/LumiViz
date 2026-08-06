/**
 ****************************************************************************************
 * @file   PixelFilterWrapper.hpp
 * @brief  GLSL-Fragment-Wrapper für den Pixel-Filter-Node (Stilfilter-Strang,
 *         Offene_Punkte §7 — Session 70)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Der Pixel-Filter-Node ist das skriptbare Stilfilter-Modul (Entscheid Patrik
 * S70: EIN Filtermodul + Werks-Voreinstellungen statt 1–3 Festmodule): eine
 * NUTZER-GLSL-Funktion läuft je PIXEL im Fragment-Shader und färbt das
 * Chain-Bild um — Comic/Rotoskopie („Take On Me"), Posterize, Halftone,
 * CRT/VHS, Kuwahara … Ein Knoten = EIN Pass; Nachbar-Abtastung über `uTex`
 * ist erlaubt (Kantenzüge!), Multipass-Looks laufen über den Shadertoy-Knoten.
 *
 * Vertrag des Nutzer-Codes: er definiert
 *     vec4 farbe(vec2 uv, vec4 src)   // uv 0..1 (y oben), src = Quellpixel
 * (`filter` ist in GLSL ein RESERVIERTES Wort — Befund S70, AMD lehnt ab;
 * deutscher Name nach dem kraft()-Muster der GPU-Partikel)
 * und darf `texture(uTex, ...)` für Nachbarn sowie die Uniforms uTime,
 * uDelta, uFrame, uResolution, bass, mid, treb, vol, beat lesen. Prälude
 * endet mit `#line 1` — GL-Kompilierfehler tragen die Zeilennummern des
 * Nutzer-Codes (Shadertoy-Muster). `uMixAmount` mischt das Ergebnis im
 * Epilog gegen das Original (1 = ersetzen).
 *
 * Vertragsgrenze wie Mesh-Warp/ShadertoyWrapper: Text → Text, kein GL,
 * keine Qt — voll unit-testbar. Der Vertex-Anteil ist der geteilte
 * kQuadVertexShader des Hosts (Varying `vTex`).
 ****************************************************************************************
 */

#pragma once

#include <string>

namespace lumi::pixelfilter {

/**
 * @brief Prälude vor dem Nutzer-Code (endet mit `#line 1`)
 *
 * Eigene Namen tragen das Präfix `_lumi` (Reserviert-Raum des Wrappers);
 * `vTex` ist das Varying des geteilten Quad-Vertex-Shaders.
 */
[[nodiscard]] inline const char* fragmentPrelude()
{
    return R"(#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uResolution;
uniform float uTime;
uniform float uDelta;
uniform int uFrame;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float beat;
uniform float uMixAmount;
out vec4 _lumiFrag;
#line 1
)";
}

/// Epilog: main() ruft farbe() — `#line 100000` = Wrapper-Zeilen im
/// Treiber-Log. uMixAmount mischt gegen das Original (1 = ersetzen);
/// Alpha bleibt 1 (die Chain-Surfaces sind opak).
[[nodiscard]] inline const char* fragmentEpilog()
{
    return R"(
#line 100000
void main()
{
    vec4 _lumiSrc = texture(uTex, vTex);
    vec4 _lumiErg = farbe(vTex, _lumiSrc);
    _lumiFrag = vec4(mix(_lumiSrc.rgb, _lumiErg.rgb,
                         clamp(uMixAmount, 0.0, 1.0)), 1.0);
}
)";
}

/// Komplettes Fragmentprogramm aus einem farbe()-Quelltext (leer = Identität)
[[nodiscard]] inline std::string wrapFragment(const std::string& userCode)
{
    std::string s = fragmentPrelude();
    if (userCode.empty())
    {
        s += "vec4 farbe(vec2 uv, vec4 src) { return src; }\n";
    }
    else
    {
        s += userCode;
        if (userCode.back() != '\n') s += '\n';
    }
    s += fragmentEpilog();
    return s;
}

/// Starter-Filter (sichtbar ab dem ersten Frame, audio-reaktiv): kleiner
/// Comic-Vorgeschmack — Farbquantisierung, deren Stufenzahl mit dem Bass
/// atmet, plus Sobel-Kantenzug. Die volle Take-On-Me-Fassung liegt als
/// Werks-Voreinstellung bei.
[[nodiscard]] inline const char* starterFilter()
{
    return R"(// Pixel-Filter: vec4 farbe(vec2 uv, vec4 src) liefert die neue Farbe.
// Nachbarn abtasten: texture(uTex, uv + ...) — z. B. fuer Kantenzuege.
// Uniforms: uTime, uDelta, uFrame, uResolution, bass, mid, treb, vol, beat
vec4 farbe(vec2 uv, vec4 src)
{
    vec2 px = 1.0 / uResolution;
    float stufen = 4.0 + 3.0 * bass;                 // Toene atmen mit dem Bass
    vec3 ton = floor(src.rgb * stufen + 0.5) / stufen;
    vec3 dx = texture(uTex, uv + vec2(px.x, 0.0)).rgb
            - texture(uTex, uv - vec2(px.x, 0.0)).rgb;
    vec3 dy = texture(uTex, uv + vec2(0.0, px.y)).rgb
            - texture(uTex, uv - vec2(0.0, px.y)).rgb;
    float kante = smoothstep(0.10, 0.30,
                             length(vec2(dot(dx, vec3(0.333)),
                                         dot(dy, vec3(0.333)))));
    return vec4(ton * (1.0 - 0.85 * kante), 1.0);
}
)";
}

} // namespace lumi::pixelfilter
