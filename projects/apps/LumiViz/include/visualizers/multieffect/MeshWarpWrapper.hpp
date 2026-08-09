/**
 ****************************************************************************************
 * @file   MeshWarpWrapper.hpp
 * @brief  GLSL-Vertex-Wrapper + Gitter-Erzeugung für den Mesh-Warp-Node
 *         (Strang G1, Regelwerk_und_Neue_Module_Plan §G — Session 69)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Der Mesh-Warp-Node ist die GPU-Antwort auf die Movement-Klasse: statt
 * per-Pixel-Tabellen (CPU) rechnet eine NUTZER-GLSL-Funktion je GITTER-VERTEX
 * im Vertex-Shader, wo das Quellbild abgetastet wird — zustandslos-parallel
 * (modernes Regelwerk; Legacy-Imports bleiben CPU, §7-Notiz GPU-Vertex-Module).
 *
 * Vertrag des Nutzer-Codes: er definiert
 *     vec2 warp(vec2 uv)   // uv 0..1 (y oben), Rückgabe = QUELL-UV
 * und darf die Uniforms uTime, uDelta, uFrame, uResolution sowie bass, mid,
 * treb, vol, beat lesen. Prälude endet mit `#line 1` — GL-Kompilierfehler
 * tragen die Zeilennummern des Nutzer-Codes (Shadertoy-Muster).
 *
 * Vertragsgrenze wie ShadertoyWrapper: Text → Text bzw. reine Zahlenfelder,
 * kein GL, keine Qt — voll unit-testbar (Gitter-Erzeugung inklusive).
 ****************************************************************************************
 */

#pragma once

#include <string>
#include <vector>

namespace lumi::meshwarp {

/// Gitter-Grenzen (Plan §G1: „frei bis 256×192 — GPU skaliert")
inline constexpr int kMinGrid = 2;
inline constexpr int kMaxGridX = 256;
inline constexpr int kMaxGridY = 192;

/**
 * @brief Prälude vor dem Nutzer-Code (endet mit `#line 1`)
 *
 * Eigene Namen tragen das Präfix `_lumi` (Reserviert-Raum des Wrappers).
 * `aPos` ist die Gitter-Position 0..1; sie wird unverändert als Bildschirm-
 * Position ausgegeben — der Nutzer-Warp verschiebt die ABTAST-Koordinate
 * (MilkDrop-Warp-Semantik: das Gitter bleibt, die Quelle wandert).
 */
[[nodiscard]] inline const char* vertexPrelude()
{
    return R"(#version 330 core
layout(location = 0) in vec2 _lumiPos;
out vec2 _lumiUv;
out vec2 _lumiScreenUv;
uniform vec2 uResolution;
uniform float uTime;
uniform float uDelta;
uniform int uFrame;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float beat;
#line 1
)";
}

/// Epilog: main() ruft warp() — `#line 100000` = Wrapper-Zeilen im Treiber-Log
[[nodiscard]] inline const char* vertexEpilog()
{
    return R"(
#line 100000
void main()
{
    _lumiScreenUv = _lumiPos;
    _lumiUv = warp(_lumiPos);
    gl_Position = vec4(_lumiPos * 2.0 - 1.0, 0.0, 1.0);
}
)";
}

/// Komplettes Vertexprogramm aus einem warp()-Quelltext (leer = Identität)
[[nodiscard]] inline std::string wrapVertex(const std::string& userCode)
{
    std::string s = vertexPrelude();
    if (userCode.empty())
    {
        s += "vec2 warp(vec2 uv) { return uv; }\n";
    }
    else
    {
        s += userCode;
        if (userCode.back() != '\n') s += '\n';
    }
    s += vertexEpilog();
    return s;
}

/**
 * @brief Fragmentprogramm: Quellbild an der gewarpten UV abtasten.
 *
 * `uWrap` wählt die Randbehandlung im SHADER (fract vs. clamp) — die
 * Textur-Wrap-States der geteilten Chain-Surface bleiben unangetastet.
 * `uMixAmount` mischt gegen die UNGEWARPTE Abtastung (1 = ersetzen) —
 * dieselbe Textur, zweite Koordinate, kein zweiter Pass nötig.
 */
[[nodiscard]] inline const char* fragmentShader()
{
    return R"(#version 330 core
in vec2 _lumiUv;
in vec2 _lumiScreenUv;
uniform sampler2D uTex;
uniform float uMixAmount;
uniform int uWrap;
out vec4 _lumiFrag;
void main()
{
    vec2 uv = (uWrap == 1) ? fract(_lumiUv) : clamp(_lumiUv, 0.0, 1.0);
    vec3 warped = texture(uTex, uv).rgb;
    vec3 quelle = texture(uTex, _lumiScreenUv).rgb;
    _lumiFrag = vec4(mix(quelle, warped, clamp(uMixAmount, 0.0, 1.0)), 1.0);
}
)";
}

/// Starter-Warp (sichtbar ab dem ersten Frame, audio-reaktiv): Bass-Swirl
/// um die Bildmitte + leichte Atmung. Wie der Shadertoy-Starter bewusst
/// harmlos — Feedback-Aufschaukeln vermeidet der Zoom-Faktor < 1 nicht
/// zwingend, aber mixAmount/Decay der Kette begrenzen ihn.
[[nodiscard]] inline const char* starterWarp()
{
    return R"(// Mesh-Warp: vec2 warp(vec2 uv) liefert die QUELL-UV je Gitterpunkt.
// Uniforms: uTime, uDelta, uFrame, uResolution, bass, mid, treb, vol, beat
vec2 warp(vec2 uv)
{
    vec2 d = uv - 0.5;
    float winkel = 0.30 * sin(uTime * 0.4) + 1.2 * bass * exp(-6.0 * dot(d, d));
    float s = sin(winkel);
    float c = cos(winkel);
    float zoom = 1.0 + 0.03 * sin(uTime * 0.9) + 0.05 * treb;
    return 0.5 + mat2(c, -s, s, c) * d / zoom;
}
)";
}

/**
 * @brief Gitter-Vertizes (x, y ∈ 0..1), zeilenweise — (gx+1)·(gy+1) Punkte.
 * @param gx Quads in X (>= 1), @param gy Quads in Y (>= 1)
 */
[[nodiscard]] inline std::vector<float> buildGridVertices(int gx, int gy)
{
    std::vector<float> v;
    if (gx < 1 || gy < 1) return v;
    v.reserve(static_cast<std::size_t>(gx + 1) * static_cast<std::size_t>(gy + 1) * 2);
    for (int y = 0; y <= gy; ++y)
    {
        for (int x = 0; x <= gx; ++x)
        {
            v.push_back(static_cast<float>(x) / static_cast<float>(gx));
            v.push_back(static_cast<float>(y) / static_cast<float>(gy));
        }
    }
    return v;
}

/// Triangle-Indizes zum Vertex-Feld oben — 6 je Quad, gx·gy Quads.
[[nodiscard]] inline std::vector<unsigned int> buildGridIndices(int gx, int gy)
{
    std::vector<unsigned int> idx;
    if (gx < 1 || gy < 1) return idx;
    idx.reserve(static_cast<std::size_t>(gx) * static_cast<std::size_t>(gy) * 6);
    const auto at = [gx](int x, int y) {
        return static_cast<unsigned int>(y * (gx + 1) + x);
    };
    for (int y = 0; y < gy; ++y)
    {
        for (int x = 0; x < gx; ++x)
        {
            idx.push_back(at(x, y));
            idx.push_back(at(x + 1, y));
            idx.push_back(at(x, y + 1));
            idx.push_back(at(x + 1, y));
            idx.push_back(at(x + 1, y + 1));
            idx.push_back(at(x, y + 1));
        }
    }
    return idx;
}

} // namespace lumi::meshwarp
