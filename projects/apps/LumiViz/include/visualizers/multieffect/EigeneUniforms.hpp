/**
 ****************************************************************************************
 * @file   EigeneUniforms.hpp
 * @brief  Uniforms, die ein GLSL-Modul selbst deklariert — als Skriptvariablen
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Block E der Formelfelder (Entscheid Patrik S79, 1a): Wer im Code eines
 * Shadertoy-, Pixel-Filter-, Mesh-Warp- oder GPU-Partikel-Knotens
 * `uniform float wobble;` schreibt, hat im Parameter-Skript die Variable
 * `wobble`. Derselbe Grundsatz wie bei ISF: der Name im Shader IST der Name im
 * Skript.
 *
 * Erkannt werden `float`, `int`, `bool` und `vec2`/`vec3`/`vec4` (Vektoren im
 * Skript als `<name>_x/_y/_z/_w`), auch mehrere Namen je Zeile
 * (`uniform float a, b;`). Sampler, Arrays und Matrizen bleiben aussen vor —
 * ein Skript liefert Zahlen. Kommentare werden vorher entfernt, damit eine
 * auskommentierte Zeile keinen Namen belegt.
 *
 * Header-only und GL-frei: pur testbar (test_EigeneUniforms.cpp).
 ****************************************************************************************
 */
#pragma once

#include <cctype>
#include <string>
#include <vector>

namespace lumi::multieffect {

struct EigenesUniform
{
    enum class Typ
    {
        Float,
        Int,
        Bool,
        Vec
    };
    std::string name;
    Typ typ = Typ::Float;
    int komponenten = 1;  ///< 1, bei Vec 2..4
};

namespace detail {

/// `//`- und `/* */`-Kommentare durch Leerzeichen ersetzen (Zeilen bleiben).
inline std::string ohneKommentare(const std::string& s)
{
    std::string out = s;
    for (std::size_t i = 0; i + 1 < out.size(); ++i)
    {
        if (out[i] == '/' && out[i + 1] == '/')
        {
            while (i < out.size() && out[i] != '\n') out[i++] = ' ';
        }
        else if (out[i] == '/' && out[i + 1] == '*')
        {
            while (i + 1 < out.size() && !(out[i] == '*' && out[i + 1] == '/'))
            {
                if (out[i] != '\n') out[i] = ' ';
                ++i;
            }
            if (i + 1 < out.size()) out[i] = out[i + 1] = ' ';
        }
    }
    return out;
}

inline bool istNamenszeichen(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

}  // namespace detail

/// Alle vom Code selbst deklarierten Zahl-Uniforms, in Reihenfolge, ohne Doppelte.
[[nodiscard]] inline std::vector<EigenesUniform> eigeneUniforms(const std::string& code)
{
    std::vector<EigenesUniform> out;
    const std::string s = detail::ohneKommentare(code);
    std::size_t pos = 0;
    while ((pos = s.find("uniform", pos)) != std::string::npos)
    {
        const bool wortAnfang = pos == 0 || !detail::istNamenszeichen(s[pos - 1]);
        std::size_t i = pos + 7;
        pos = i;
        if (!wortAnfang || i >= s.size() || detail::istNamenszeichen(s[i])) continue;
        const std::size_t ende = s.find(';', i);
        if (ende == std::string::npos) break;
        const std::string decl = s.substr(i, ende - i);
        pos = ende + 1;

        // Woerter zerlegen: Praezision (lowp/mediump/highp) ueberspringen,
        // dann Typ, dann die Namensliste.
        std::vector<std::string> woerter;
        std::string wort;
        for (char c : decl + ' ')
        {
            if (detail::istNamenszeichen(c) || c == '[' || c == ']')
            {
                wort += c;
            }
            else
            {
                if (!wort.empty()) woerter.push_back(wort);
                wort.clear();
            }
        }
        std::size_t w = 0;
        while (w < woerter.size() &&
               (woerter[w] == "lowp" || woerter[w] == "mediump" || woerter[w] == "highp"))
            ++w;
        if (w >= woerter.size()) continue;
        const std::string& typ = woerter[w++];
        EigenesUniform proto;
        if (typ == "float") proto.typ = EigenesUniform::Typ::Float;
        else if (typ == "int") proto.typ = EigenesUniform::Typ::Int;
        else if (typ == "bool") proto.typ = EigenesUniform::Typ::Bool;
        else if (typ == "vec2" || typ == "vec3" || typ == "vec4")
        {
            proto.typ = EigenesUniform::Typ::Vec;
            proto.komponenten = typ[3] - '0';
        }
        else continue;  // sampler2D, mat4, … — kein Skriptwert

        // Namensliste: alles hinter dem Typwort, an Kommas getrennt; ein
        // Initialisierer (`= 1.0`) gehoert nicht zum Namen.
        const std::string rest = decl.substr(decl.find(typ) + typ.size());
        std::size_t start = 0;
        while (start <= rest.size())
        {
            std::size_t komma = rest.find(',', start);
            if (komma == std::string::npos) komma = rest.size();
            std::string teil = rest.substr(start, komma - start);
            teil = teil.substr(0, teil.find('='));
            std::string n;
            for (char c : teil)
                if (!std::isspace(static_cast<unsigned char>(c))) n += c;
            start = komma + 1;
            if (n.empty() || std::isdigit(static_cast<unsigned char>(n[0])) != 0) continue;
            if (n.find('[') != std::string::npos) continue;  // Arrays aussen vor
            bool gueltig = true;
            for (char c : n) gueltig = gueltig && detail::istNamenszeichen(c);
            if (!gueltig) continue;
            bool doppelt = false;
            for (const auto& e : out) doppelt = doppelt || e.name == n;
            if (doppelt) continue;
            EigenesUniform u = proto;
            u.name = n;
            out.push_back(u);
        }
    }
    return out;
}

}  // namespace lumi::multieffect
