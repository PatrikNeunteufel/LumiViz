/**
 ****************************************************************************************
 * @file   GpuParticlesWrapper.hpp
 * @brief  Shader-Bausteine des GPU-Partikel-Nodes (Strang G2,
 *         Regelwerk_und_Neue_Module_Plan §G — Session 69)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Die „moderne Antwort" auf die searchlight-Klasse: zehntausende Partikel
 * statt 800×16-gmegabuf-Schleifen. Zustand (pos.xy + vel.xy) lebt als EIN
 * RGBA32F-Texel je Partikel in einem Ping-Pong-Texturpaar; ein Fragment-Pass
 * integriert je Frame (Schwerkraft + Drag + optionales Nutzer-Kraftfeld),
 * ein instanzierter Quad-Draw zeichnet (texelFetch im Vertex-Shader — VTF,
 * GL 3.3 core).
 *
 * ALTER OHNE SPEICHER (deterministisch): jedes Partikel hat eine feste,
 * hash-basierte Phase und Lebensdauer — `alter01 = fract((t+phase·leben)/
 * leben)`. Der Update-Pass erkennt den Zyklus-Wechsel über floor()-Vergleich
 * mit dem Vorframe und setzt das Partikel dann frisch an den Spawn (Richtung/
 * Streuung ebenfalls hash-basiert, je Zyklus neu). Kein rand(), keine CPU-
 * Arbeit pro Partikel — mit Sim-Uhr + festem dt voll reproduzierbar.
 *
 * Nutzer-Kraftfeld (optional, Mesh-Warp-Muster): der Code definiert
 *     vec2 kraft(vec2 pos, vec2 vel, float alter)   // alter 0..1
 * und darf uTime/uDelta/uResolution + bass/mid/treb/vol/beat lesen; Prälude
 * endet mit `#line 1` (Treiberfehler tragen Nutzer-Zeilen). Leer = keine
 * Zusatzkraft.
 *
 * Vertragsgrenze wie MeshWarpWrapper: Text → Text, kein GL, kein Qt.
 ****************************************************************************************
 */

#pragma once

#include <string>

namespace lumi::gpuparticles {

/// Zustands-Textur: feste Breite, Höhe = ceil(count/kStateWidth).
inline constexpr int kStateWidth = 256;
/// Partikel-Klemmen (kStateWidth² = 65536 — „zehntausende", Plan §G2)
inline constexpr int kMinCount = 1;
inline constexpr int kMaxCount = kStateWidth * kStateWidth;

/// Zeilen der Zustands-Textur für `count` Partikel (SSOT Renderer/Tests).
[[nodiscard]] inline int stateRows(int count)
{
    return (count + kStateWidth - 1) / kStateWidth;
}

/// Gemeinsames GLSL-Prälude von Update- und Render-Pass: Uniforms + Hash +
/// Lebenslauf-Rechnung — BEIDE Pässe müssen dieselben Formeln sehen, sonst
/// zeichnet der Render-Pass ein anderes Alter, als der Update-Pass springt.
[[nodiscard]] inline const char* commonPrelude()
{
    return R"(
uniform vec2 uResolution;
uniform float uTime;
uniform float uDelta;
uniform int uCount;
uniform float uLife;
uniform float uLifeJitter;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float beat;

// Deterministischer Hash (Sinus-frei, treiberstabil): uint-Mix nach Wang.
float _lumiHash(uint x)
{
    x = (x ^ 61u) ^ (x >> 16);
    x *= 9u;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2du;
    x = x ^ (x >> 15);
    return float(x & 0x00ffffffu) / 16777215.0;
}
float _lumiHash2(uint i, uint k) { return _lumiHash(i * 1664525u + k * 1013904223u + 1u); }

// Lebensdauer + Phase je Partikel (hash-fest), Alter aus der Sim-Uhr.
float _lumiLeben(uint i)
{
    return max(0.05, uLife * (1.0 + (_lumiHash(i * 2654435761u) - 0.5) * 2.0 * uLifeJitter));
}
float _lumiPhase(uint i) { return _lumiHash(i * 40503u + 7u); }
// Zyklus-Index (steigt um 1 je Respawn) und Alter 0..1 zum Zeitpunkt t.
float _lumiZyklus(uint i, float t, float leben)
{
    return floor(t / leben + _lumiPhase(i));
}
float _lumiAlter01(uint i, float t, float leben)
{
    return fract(t / leben + _lumiPhase(i));
}
)";
}

/**
 * @brief Update-Pass (Fragment, Ziel = Zustands-Textur): Integration +
 *        zyklischer Respawn. `userForce` leer = keine Zusatzkraft.
 */
[[nodiscard]] inline std::string updateFragment(const std::string& userForce)
{
    std::string s = "#version 330 core\n";
    s += commonPrelude();
    s += R"(
uniform sampler2D uState;
uniform vec2 uSpawn;
uniform float uSpread;
uniform float uSpeed;
uniform float uDir;
uniform float uFan;
uniform vec2 uGravity;
uniform float uDrag;
uniform int uReset;
out vec4 _lumiOut;
)";
    if (userForce.empty())
    {
        s += "vec2 kraft(vec2 pos, vec2 vel, float alter) { return vec2(0.0); }\n";
    }
    else
    {
        s += "#line 1\n";
        s += userForce;
        if (userForce.back() != '\n') s += '\n';
    }
    s += R"(
#line 100000
vec4 _lumiSpawn(uint i, uint zyklus)
{
    float wStreu = (_lumiHash2(i, zyklus * 2u + 11u)) * 6.2831853;
    float rStreu = sqrt(_lumiHash2(i, zyklus * 2u + 12u)) * uSpread;
    vec2 pos = uSpawn + rStreu * vec2(cos(wStreu), sin(wStreu));
    float w = radians(uDir) + (_lumiHash2(i, zyklus * 2u + 13u) - 0.5) * radians(uFan);
    float v = uSpeed * (0.6 + 0.8 * _lumiHash2(i, zyklus * 2u + 14u));
    return vec4(pos, v * vec2(cos(w), sin(w)));
}
void main()
{
    ivec2 texel = ivec2(gl_FragCoord.xy);
    uint i = uint(texel.y) * uint(textureSize(uState, 0).x) + uint(texel.x);
    if (i >= uint(uCount)) { _lumiOut = vec4(0.0); return; }
    float leben = _lumiLeben(i);
    float zyklusJetzt = _lumiZyklus(i, uTime, leben);
    float zyklusVor = _lumiZyklus(i, uTime - uDelta, leben);
    if (uReset == 1 || zyklusJetzt != zyklusVor)
    {
        _lumiOut = _lumiSpawn(i, uint(max(zyklusJetzt, 0.0)));
        return;
    }
    vec4 zustand = texelFetch(uState, texel, 0);
    vec2 pos = zustand.xy;
    vec2 vel = zustand.zw;
    float alter = _lumiAlter01(i, uTime, leben);
    vec2 beschleunigung = uGravity + kraft(pos, vel, alter);
    vel += beschleunigung * uDelta;
    vel *= max(0.0, 1.0 - uDrag * uDelta);
    pos += vel * uDelta;
    _lumiOut = vec4(pos, vel);
}
)";
    return s;
}

/**
 * @brief Render-Vertex-Pass: instanzierter Quad (aPos -1..1 vom geteilten
 *        Fullscreen-VAO als Ecke), Zustand per texelFetch (VTF).
 * Groesse in Pixeln; Alter steuert Groessen-/Farbverlauf im Fragment.
 */
[[nodiscard]] inline std::string renderVertex()
{
    std::string s = R"(#version 330 core
layout(location = 0) in vec2 aPos;
)";
    s += commonPrelude();
    s += R"(
uniform sampler2D uState;
uniform float uSize;
uniform float uSizeEnd;
out vec2 _lumiEcke;
out float _lumiAlter;
void main()
{
    uint i = uint(gl_InstanceID);
    ivec2 texel = ivec2(int(i) % textureSize(uState, 0).x,
                        int(i) / textureSize(uState, 0).x);
    vec4 zustand = texelFetch(uState, texel, 0);
    float leben = _lumiLeben(i);
    float alter = _lumiAlter01(i, uTime, leben);
    float groesse = mix(uSize, uSize * uSizeEnd, alter);
    vec2 offset = aPos * groesse / uResolution;
    vec2 clip = zustand.xy * 2.0 - 1.0 + offset;
    _lumiEcke = aPos;
    _lumiAlter = alter;
    gl_Position = (i < uint(uCount)) ? vec4(clip, 0.0, 1.0)
                                     : vec4(-9.0, -9.0, 0.0, 1.0);
}
)";
    return s;
}

/// Render-Fragment: weicher Kreis-Sprite, Farbverlauf + Ausblenden übers Alter.
[[nodiscard]] inline const char* renderFragment()
{
    return R"(#version 330 core
in vec2 _lumiEcke;
in float _lumiAlter;
uniform vec3 uColorStart;
uniform vec3 uColorEnd;
out vec4 _lumiFrag;
void main()
{
    float d = length(_lumiEcke);
    float rand = smoothstep(1.0, 0.55, d);
    float blende = 1.0 - smoothstep(0.6, 1.0, _lumiAlter);
    vec3 farbe = mix(uColorStart, uColorEnd, _lumiAlter);
    _lumiFrag = vec4(farbe, rand * blende);
}
)";
}

/// Starter-Kraftfeld der Palette (sichtbar + audio-reaktiv): Bass-Auftrieb
/// mit leichtem Wirbel um die Bildmitte.
[[nodiscard]] inline const char* starterForce()
{
    return R"(// GPU-Partikel: vec2 kraft(vec2 pos, vec2 vel, float alter) — Zusatz-
// Beschleunigung in UV/s². Uniforms: uTime, uDelta, uResolution,
// bass, mid, treb, vol, beat. alter laeuft 0..1 ueber die Lebensdauer.
vec2 kraft(vec2 pos, vec2 vel, float alter)
{
    vec2 d = pos - vec2(0.5);
    vec2 wirbel = vec2(-d.y, d.x) * (0.35 + 0.8 * treb);
    vec2 auftrieb = vec2(0.0, 0.25 * bass);
    return wirbel + auftrieb;
}
)";
}

} // namespace lumi::gpuparticles
