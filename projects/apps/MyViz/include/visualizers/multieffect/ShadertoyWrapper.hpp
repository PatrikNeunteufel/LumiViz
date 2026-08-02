/**
 ****************************************************************************************
 * @file   ShadertoyWrapper.hpp
 * @brief  GLSL-ES→GLSL-330-Wrapper für den Shadertoy-Node (Strang S, S1)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Bettet einen Shader in Shadertoy-Konvention (`mainImage(out vec4, in vec2)`)
 * in ein GLSL-330-core-Fragmentprogramm ein: Prälude mit dem vollen
 * Shadertoy-Uniform-Satz (iResolution, iTime, iTimeDelta, iFrame, iFrameRate,
 * iMouse, iDate, iSampleRate, iChannelTime[4], iChannelResolution[4],
 * iChannel0–3) PLUS den LumiViz-Audio-Uniforms (bass, mid, treb, vol, beat —
 * damit werden auch nicht-audioreaktive Shader nachrüstbar, Plan §S2), dann
 * `#line 1` (GL-Kompilierfehler tragen die ZEILENNUMMER DES NUTZER-CODES),
 * dann der Nutzer-Code, dann ein main() mit Blend-Epilog (_lumiBlend:
 * 0 ersetzen · 1 additiv · 2 50/50 gegen _lumiPrev = aktuelles Chain-Bild).
 *
 * Vertragsgrenze wie EelTranspiler/HlslTranspiler: Text → Text, kein GL,
 * keine Qt — voll unit-testbar. Desktop-GLSL 3.30 akzeptiert die üblichen
 * GLSL-ES-Reste (precision-Qualifier) als No-op; was der Treiber ablehnt,
 * meldet der Host als Kompilierfehler mit Nutzer-Zeile (Import-Report-Stil).
 ****************************************************************************************
 */

#pragma once

#include <string>

namespace lumi::shadertoy {

/// Blend-Epilog-Werte des Wrappers (Uniform _lumiBlend)
enum class Blend
{
    Ersetzen = 0,
    Additiv = 1,
    Mix5050 = 2,
};

/**
 * @brief Prälude vor dem Nutzer-Code (endet mit `#line 1`)
 *
 * Eigene Namen tragen das Präfix `_lumi` — sie liegen im Reserviert-Raum
 * des Wrappers und kollidieren nicht mit üblichem Shadertoy-Code.
 */
[[nodiscard]] inline const char* fragmentPrelude()
{
    return R"(#version 330 core
uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform float iFrameRate;
uniform vec4 iMouse;
uniform vec4 iDate;
uniform float iSampleRate;
uniform float iChannelTime[4];
uniform vec3 iChannelResolution[4];
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float beat;
uniform sampler2D _lumiPrev;
uniform int _lumiBlend;
out vec4 _lumiFrag;
#line 1
)";
}

/**
 * @brief Epilog nach dem Nutzer-Code: main() ruft mainImage und blendet
 *
 * `#line 100000` markiert Wrapper-Zeilen in Treiber-Logs unverwechselbar
 * (ein Fehler dort ist ein Wrapper-Bug, keiner des Nutzer-Codes).
 */
[[nodiscard]] inline const char* fragmentEpilog()
{
    return R"(
#line 100000
void main()
{
    vec4 _lumiColor = vec4(0.0);
    mainImage(_lumiColor, gl_FragCoord.xy);
    vec3 _lumiPrevRgb = texture(_lumiPrev, gl_FragCoord.xy / iResolution.xy).rgb;
    vec3 _lumiOut = (_lumiBlend == 1) ? clamp(_lumiPrevRgb + _lumiColor.rgb, 0.0, 1.0)
                  : (_lumiBlend == 2) ? 0.5 * (_lumiPrevRgb + _lumiColor.rgb)
                  : _lumiColor.rgb;
    _lumiFrag = vec4(_lumiOut, 1.0);
}
)";
}

/// Komplettes Fragmentprogramm aus einem mainImage-Quelltext
[[nodiscard]] inline std::string wrapFragment(const std::string& userCode)
{
    std::string s = fragmentPrelude();
    s += userCode;
    if (!userCode.empty() && userCode.back() != '\n') s += '\n';
    s += fragmentEpilog();
    return s;
}

/**
 * @brief Epilog eines BUFFER-Passes (S4): rohes vec4, KEIN Blend/Clamp —
 *        Buffer sind Zustand (RGBA32F, Vorzeichen/Alpha bleiben erhalten)
 */
[[nodiscard]] inline const char* bufferEpilog()
{
    return R"(
#line 100000
void main()
{
    vec4 _lumiColor = vec4(0.0);
    mainImage(_lumiColor, gl_FragCoord.xy);
    _lumiFrag = _lumiColor;
}
)";
}

/// Fragmentprogramm eines Buffer-Passes (gleiche Prälude, roher Epilog)
[[nodiscard]] inline std::string wrapBufferFragment(const std::string& userCode)
{
    std::string s = fragmentPrelude();
    s += userCode;
    if (!userCode.empty() && userCode.back() != '\n') s += '\n';
    s += bufferEpilog();
    return s;
}

/**
 * @brief Eigener Starter-Shader für neue Nodes (KEIN Shadertoy-Inhalt —
 *        Lizenz-Vorbehalt Plan §S3): audioreaktive Farbringe
 */
[[nodiscard]] inline const char* starterShader()
{
    return R"(// LumiViz-Starter: audioreaktive Ringe (bass/mid/treb als Uniforms,
// iChannel0 = 512x2-Audio: Zeile 0 Spektrum, Zeile 1 Waveform)
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float d = length(uv);
    float wave = texture(iChannel0, vec2(fract(d), 0.75)).x - 0.5;
    float ring = smoothstep(0.06, 0.0, abs(d - 0.5 - 0.35 * bass - 0.4 * wave));
    vec3 col = ring * (0.5 + 0.5 * cos(iTime + d * 6.0 + vec3(0.0, 2.1, 4.2)));
    col += 0.15 * treb * (1.0 - d);
    fragColor = vec4(col, 1.0);
}
)";
}

} // namespace lumi::shadertoy
