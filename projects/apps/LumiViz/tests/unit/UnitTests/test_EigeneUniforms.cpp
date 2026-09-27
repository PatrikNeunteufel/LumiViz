/**
 ****************************************************************************************
 * @file   test_EigeneUniforms.cpp
 * @brief  Welche Uniforms deklariert ein GLSL-Modul selbst? (Block E, S79)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/EigeneUniforms.hpp"

using lumi::multieffect::EigenesUniform;
using lumi::multieffect::eigeneUniforms;

TEST_CASE("EigeneUniforms: Zahltypen werden erkannt, andere nicht")
{
    const auto u = eigeneUniforms(
        "uniform float wobble;\n"
        "uniform int steps;\n"
        "uniform bool flip;\n"
        "uniform vec3 tint;\n"
        "uniform sampler2D tex;\n"
        "uniform mat4 m;\n"
        "uniform float arr[4];\n");
    REQUIRE(u.size() == 4);
    CHECK(u[0].name == "wobble");
    CHECK(u[0].typ == EigenesUniform::Typ::Float);
    CHECK(u[1].typ == EigenesUniform::Typ::Int);
    CHECK(u[2].typ == EigenesUniform::Typ::Bool);
    CHECK(u[3].name == "tint");
    CHECK(u[3].typ == EigenesUniform::Typ::Vec);
    CHECK(u[3].komponenten == 3);
}

TEST_CASE("EigeneUniforms: Listen, Praezision, Initialisierer, Doppelte")
{
    const auto u = eigeneUniforms(
        "uniform highp float a, b;\n"
        "uniform float c = 1.0;\n"
        "uniform float a;\n");
    REQUIRE(u.size() == 3);
    CHECK(u[0].name == "a");
    CHECK(u[1].name == "b");
    CHECK(u[2].name == "c");
}

TEST_CASE("EigeneUniforms: Kommentare belegen keinen Namen")
{
    const auto u = eigeneUniforms(
        "// uniform float alt;\n"
        "/* uniform float auch;\n uniform float nicht; */\n"
        "uniform float echt; // uniform float dahinter;\n"
        "float myuniform = 1.0;\n");
    REQUIRE(u.size() == 1);
    CHECK(u[0].name == "echt");
}
