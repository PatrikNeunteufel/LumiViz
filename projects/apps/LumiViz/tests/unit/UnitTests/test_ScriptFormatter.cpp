/**
 ****************************************************************************************
 * @file   test_ScriptFormatter.cpp
 * @brief  Unit-Tests fuer die Beautify-Kerne (Offene_Punkte §7, Session 69):
 *         EEL-Statement-Umbruch mit Klammertiefen-Einzug, GLSL/HLSL-Brace-
 *         Re-Indent, Format-Optionen (Einzugsbreite, Operator-Abstaende,
 *         Leerzeilen-Klemme) und Token-Erhaltung (Whitespace-only-Vertrag)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/ScriptFormatter.hpp"

#include <algorithm>
#include <cctype>
#include <string>

using lumi::scripting::beautifyEel;
using lumi::scripting::beautifyGlsl;
using lumi::scripting::FormatOptions;

namespace
{

/// Whitespace-only-Vertrag: Eingabe und Ausgabe muessen ohne Weissraum
/// byte-identisch sein — Beautify darf nie Token veraendern.
std::string stripWs(const std::string& s)
{
    std::string out;
    for (const char c : s)
        if (std::isspace(static_cast<unsigned char>(c)) == 0) out += c;
    return out;
}

} // namespace

// =============================================================================
// EEL
// =============================================================================

TEST_CASE("EEL: ein Statement pro Zeile")
{
    const std::string in = "x=1; y=2;z=x+y;";
    CHECK(beautifyEel(in) == "x = 1;\ny = 2;\nz = x + y;");
}

TEST_CASE("EEL: Einzug nach Klammertiefe (loop mit Statement-Liste)")
{
    const std::string in = "loop(8, x=x+1; megabuf(x)=0; );";
    const FormatOptions opt{2, true, 2};
    CHECK(beautifyEel(in, opt) ==
          "loop(8, x = x + 1;\n"
          "  megabuf(x) = 0;\n"
          ");");
}

TEST_CASE("EEL: verschachtelte Bloecke ruecken mehrstufig ein")
{
    const std::string in = "if(above(a,0), exec2(b=1;c=2;), d=3);";
    const FormatOptions opt{2, true, 2};
    CHECK(beautifyEel(in, opt) ==
          "if(above(a, 0), exec2(b = 1;\n"
          "    c = 2;\n"
          "  ), d = 3);");
}

TEST_CASE("EEL: Operator-Abstaende abschaltbar (kompakt)")
{
    const FormatOptions kompakt{4, false, 2};
    CHECK(beautifyEel("x = 1 + 2;", kompakt) == "x=1+2;");
    CHECK(beautifyEel("a>=b; c&&d;", kompakt) == "a>=b;\nc&&d;");
}

TEST_CASE("EEL: unaeres Minus klebt am Operanden")
{
    CHECK(beautifyEel("x=-1; y=a-b; z=(-c);") ==
          "x = -1;\ny = a - b;\nz = (-c);");
    CHECK(beautifyEel("w=rand(100)*-0.5;") == "w = rand(100) * -0.5;");
}

TEST_CASE("EEL: Exponenten-Zahlen bleiben ein Token")
{
    CHECK(beautifyEel("x=1e-5; y=2E+3;") == "x = 1e-5;\ny = 2E+3;");
}

TEST_CASE("EEL: Zeilenkommentar bleibt bei seinem Statement")
{
    const std::string in = "n=5; // fuenf Punkte\nx=0;";
    CHECK(beautifyEel(in) == "n = 5;  // fuenf Punkte\nx = 0;");
}

TEST_CASE("EEL: alleinstehender Kommentar behaelt seine Zeile")
{
    const std::string in = "// Setup\nn=5;";
    CHECK(beautifyEel(in) == "// Setup\nn = 5;");
}

TEST_CASE("EEL: Block-Kommentar wird inline erhalten")
{
    CHECK(beautifyEel("x=/*alt*/5;") == "x = /*alt*/ 5;");
}

TEST_CASE("EEL: Leerzeilen zwischen Statements geklemmt")
{
    const std::string in = "a=1;\n\n\n\n\nb=2;";
    const FormatOptions opt{4, true, 2};
    CHECK(beautifyEel(in, opt) == "a = 1;\n\n\nb = 2;");
    const FormatOptions keine{4, true, 0};
    CHECK(beautifyEel(in, keine) == "a = 1;\nb = 2;");
}

TEST_CASE("EEL: einzeiliger Preset-Wust wird lesbar (Milkdrop-Stil)")
{
    const std::string in =
        "q1=bass_att*0.5;q2=q1+treb;wave_r=0.5+0.5*sin(q1);";
    CHECK(beautifyEel(in) ==
          "q1 = bass_att * 0.5;\n"
          "q2 = q1 + treb;\n"
          "wave_r = 0.5 + 0.5 * sin(q1);");
}

TEST_CASE("EEL: mehrzeiliges Statement wird zusammengezogen")
{
    CHECK(beautifyEel("x =\n  1 +\n  2;") == "x = 1 + 2;");
}

TEST_CASE("EEL: Whitespace-only-Vertrag (Token byte-identisch)")
{
    const std::string in =
        "// kopf\nn=800;t=0; loop(4, megabuf(t)=t*1e-3; t=t+1;);\n"
        "x=if(above(a,b),-1,$PI); /*mitte*/ y=0x1F+2h;";
    for (const bool spaces : {true, false})
    {
        const FormatOptions opt{3, spaces, 1};
        CHECK(stripWs(beautifyEel(in, opt)) == stripWs(in));
    }
}

TEST_CASE("EEL: Beautify ist idempotent")
{
    const std::string in = "loop(8, x=x+1; y=y-2;);\n\nz=sin(x)*-1;";
    const std::string once = beautifyEel(in);
    CHECK(beautifyEel(once) == once);
}

TEST_CASE("EEL: leer und Weissraum")
{
    CHECK(beautifyEel("").empty());
    CHECK(beautifyEel("   \n\n\t ").empty());
}

// =============================================================================
// GLSL / HLSL
// =============================================================================

TEST_CASE("GLSL: Brace-Re-Indent")
{
    const std::string in =
        "void mainImage(out vec4 o, in vec2 f) {\n"
        "vec2 uv = f / iResolution.xy;\n"
        "if (uv.x > 0.5) {\n"
        "o = vec4(1.0);\n"
        "} else {\n"
        "o = vec4(0.0);\n"
        "}\n"
        "}";
    const FormatOptions opt{4, true, 2};
    CHECK(beautifyGlsl(in, opt) ==
          "void mainImage(out vec4 o, in vec2 f) {\n"
          "    vec2 uv = f / iResolution.xy;\n"
          "    if (uv.x > 0.5) {\n"
          "        o = vec4(1.0);\n"
          "    } else {\n"
          "        o = vec4(0.0);\n"
          "    }\n"
          "}");
}

TEST_CASE("GLSL: Zeileninhalt bleibt unangetastet (nur Einzug)")
{
    const std::string in = "  float a=1.0+2.0;   ";
    CHECK(beautifyGlsl(in) == "float a=1.0+2.0;");
}

TEST_CASE("GLSL: Praeprozessor auf Spalte 0")
{
    const std::string in =
        "void f() {\n"
        "#ifdef FOO\n"
        "int x = 1;\n"
        "#endif\n"
        "}";
    CHECK(beautifyGlsl(in) ==
          "void f() {\n"
          "#ifdef FOO\n"
          "    int x = 1;\n"
          "#endif\n"
          "}");
}

TEST_CASE("GLSL: Braces in Kommentaren zaehlen nicht")
{
    const std::string in =
        "void f() {\n"
        "// kommentar mit { und }\n"
        "int x = 1; // noch eine {\n"
        "}";
    CHECK(beautifyGlsl(in) ==
          "void f() {\n"
          "    // kommentar mit { und }\n"
          "    int x = 1; // noch eine {\n"
          "}");
}

TEST_CASE("GLSL: Block-Kommentar-Inneres bleibt roh")
{
    const std::string in =
        "void f() {\n"
        "/* skizze:\n"
        "     { so bleibt es }\n"
        "*/\n"
        "int x = 1;\n"
        "}";
    CHECK(beautifyGlsl(in) ==
          "void f() {\n"
          "    /* skizze:\n"
          "     { so bleibt es }\n"
          "*/\n"
          "    int x = 1;\n"
          "}");
}

TEST_CASE("GLSL: Leerzeilen geklemmt")
{
    const std::string in = "int a;\n\n\n\n\nint b;";
    const FormatOptions opt{4, true, 1};
    CHECK(beautifyGlsl(in, opt) == "int a;\n\nint b;");
}

TEST_CASE("GLSL: einzeilige Bloecke und Mehrfach-Closer")
{
    const std::string in =
        "void f() {\n"
        "if (a) { b(); }\n"
        "for (;;) {\n"
        "g();\n"
        "}}";
    CHECK(beautifyGlsl(in) ==
          "void f() {\n"
          "    if (a) { b(); }\n"
          "    for (;;) {\n"
          "        g();\n"
          "    }}");
}

TEST_CASE("GLSL: Einzugsbreite einstellbar")
{
    const std::string in = "void f() {\nint x;\n}";
    const FormatOptions zwei{2, true, 2};
    CHECK(beautifyGlsl(in, zwei) == "void f() {\n  int x;\n}");
}

TEST_CASE("GLSL: Beautify ist idempotent")
{
    const std::string in =
        "void f() {\nif (a) {\nb();\n}\n\n\nc();\n}";
    const std::string once = beautifyGlsl(in);
    CHECK(beautifyGlsl(once) == once);
}

TEST_CASE("GLSL: Whitespace-only-Vertrag")
{
    const std::string in =
        "#define K 3\nvoid f() {\n/* block { */\nint x=K; // { \n}";
    CHECK(stripWs(beautifyGlsl(in)) == stripWs(in));
}

TEST_CASE("GLSL: leer")
{
    CHECK(beautifyGlsl("").empty());
    CHECK(beautifyGlsl("\n\n\n").empty());
}
