/**
 ****************************************************************************************
 * @file   test_IsfFilterWrapper.cpp
 * @brief  Tests fuer die ISF-Praelude des isfFilter-Knotens (S72)
 *
 * Der Wrapper ist die Stelle, an der ISF zur Sprache wird — er ersetzt das
 * frueher noetige Uebersetzen im Importeur. Geprueft wird deshalb genau das,
 * worauf sich der unveraenderte Nutzer-Code verlaesst: das Vokabular, der
 * `#line 1`-Vertrag und der `#define main`-Kniff.
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/IsfFilterWrapper.hpp"

#include <string>

using lumi::multieffect::IsfBildQuelle;
using lumi::multieffect::ParamGruppe;
using lumi::multieffect::ParamTyp;
using lumi::multieffect::ParamWert;

namespace {

[[nodiscard]] bool hat(const std::string& hay, const std::string& needle)
{
    return hay.find(needle) != std::string::npos;
}

[[nodiscard]] ParamWert wert(const char* key, ParamTyp typ)
{
    ParamWert w;
    w.key = key;
    w.typ = typ;
    return w;
}

}  // namespace

TEST_CASE("IsfFilterWrapper: die Praelude stellt das ISF-Vokabular bereit")
{
    const std::string frag = lumi::isffilter::wrapFragment(
        "void main() { gl_FragColor = IMG_THIS_PIXEL(inputImage); }\n",
        {{"inputImage", -1}}, {});

    CHECK(frag.rfind("#version 330 core", 0) == 0);

    // Die ISF-Groessen und -Makros, auf die JEDE importierte Datei baut.
    for (const char* v :
         {"uniform vec2 RENDERSIZE", "uniform float TIME", "uniform float TIMEDELTA",
          "uniform int FRAMEINDEX", "uniform vec4 DATE", "in vec2 isf_FragNormCoord",
          "#define vv_FragNormCoord isf_FragNormCoord", "#define texture2D texture",
          "#define IMG_SIZE", "#define IMG_NORM_PIXEL", "#define IMG_PIXEL",
          "#define IMG_THIS_PIXEL", "#define IMG_THIS_NORM_PIXEL"})
    {
        CHECK_MESSAGE(hat(frag, v), v);
    }
    // LumiViz-Extras (dieselben wie bei den anderen GPU-Knoten).
    for (const char* v : {"uniform float bass", "uniform float mid",
                          "uniform float treb", "uniform float vol",
                          "uniform float beat"})
    {
        CHECK_MESSAGE(hat(frag, v), v);
    }
    // Die Bildquelle wird unter IHREM Namen zum Sampler.
    CHECK(hat(frag, "uniform sampler2D inputImage;"));

    // `gl_FragColor` gibt es in 330 Core nicht — es ist ein #define, KEINE
    // Ausgabevariable. Waere es eine, laesse sich der Shader nicht linken.
    CHECK(hat(frag, "#define gl_FragColor"));
    CHECK_FALSE(hat(frag, "out vec4 gl_FragColor"));

    // WAECHTER S70: `filter` ist in GLSL ein RESERVIERTES Wort — der Wrapper
    // darf es nie als Bezeichner tragen (AMD lehnt den Shader sonst ab).
    CHECK_FALSE(hat(frag, " filter "));
    CHECK_FALSE(hat(frag, "float filter"));
}

TEST_CASE("IsfFilterWrapper: das AVS-Audiovokabular steht im Shader bereit")
{
    // Wunsch Patrik S72: dieselben Namen wie in den EEL-/Lua-Skripten —
    // niemand soll umlernen muessen, nur weil der Effekt auf der GPU laeuft.
    const std::string frag = lumi::isffilter::wrapFragment("void main(){}", {}, {});
    CHECK(hat(frag, "float getosc(float band, float width, float channel)"));
    CHECK(hat(frag, "float getspec(float band, float width, float channel)"));
    // Sie brauchen ihre Texturen — und die haengen IMMER an, anders als die
    // ISF-Audio-Eingaenge, die ein Shader erst deklarieren muesste.
    CHECK(hat(frag, "uniform sampler2D _lumiAudioWave;"));
    CHECK(hat(frag, "uniform sampler2D _lumiAudioFft;"));
    // Auch in der Vertex-Stufe: ISF-Vertex-Shader duerfen ebenso audio-
    // reaktiv sein (der Korpus hat solche).
    const std::string vert = lumi::isffilter::wrapVertex("void main(){}", {});
    CHECK(hat(vert, "float getspec("));

    // Die Waveform liegt als 0.5 + 0.5*x in der Textur — getosc muss auf
    // -1..1 zurueckrechnen, sonst waere ein stilles Signal 0.5 statt 0.
    CHECK(hat(frag, "* 2.0 - 1.0"));

    // IMG_SIZE muss die SAMPLER-Groesse liefern, nicht die Bildgroesse:
    // ein Shader bestimmt damit die Sample-Anzahl seines Audio-Eingangs
    // (Befund S72 — pauschal RENDERSIZE gab dort die Bildbreite).
    CHECK(hat(frag, "#define IMG_SIZE(_img) vec2(textureSize(_img, 0))"));
}

TEST_CASE("IsfFilterWrapper: der #define-main-Kniff haelt den #line-Vertrag")
{
    const std::string userCode = "void main() { gl_FragColor = vec4(1.0); }\n";
    const std::string frag = lumi::isffilter::wrapFragment(userCode, {}, {});

    // Das Nutzer-main wird per Praeprozessor umbenannt …
    CHECK(hat(frag, "#define main isf_main"));
    // … der Epilog hebt das auf und stellt das echte main().
    CHECK(hat(frag, "#undef main"));
    CHECK(frag.find("#undef main") > frag.find("#define main isf_main"));
    CHECK(hat(frag, "isf_main();"));

    // #line 1 steht DIREKT vor dem Nutzer-Code: Treiber-Fehler melden die
    // Zeilennummern, die der Nutzer im Editor sieht. Genau das waere beim
    // Umschreiben im Text verloren gegangen.
    const std::size_t line1 = frag.find("#line 1\n");
    REQUIRE(line1 != std::string::npos);
    CHECK(frag.compare(line1 + 8, userCode.size(), userCode) == 0);
    // Wrapper-Zeilen sind unverwechselbar markiert.
    CHECK(hat(frag, "#line 100000"));

    // Blend und Mix laufen im Epilog — der Nutzer-Code weiss davon nichts.
    CHECK(hat(frag, "_lumiPrev"));
    CHECK(hat(frag, "_lumiBlend"));
    CHECK(hat(frag, "_lumiMixAmount"));
}

TEST_CASE("IsfFilterWrapper: leerer Code ergibt trotzdem ein linkbares Programm")
{
    // Ein frisch eingefuegter Knoten hat noch keinen Shader. Ohne `main()`
    // liesse sich gar nichts linken — der Knoten reicht dann durch.
    const std::string frag = lumi::isffilter::wrapFragment("", {}, {});
    CHECK(hat(frag, "void main()"));
    const std::string vert = lumi::isffilter::wrapVertex("", {});
    CHECK(hat(vert, "void main()"));
}

TEST_CASE("IsfFilterWrapper: jeder Regler-Typ wird ein Uniform seines Typs")
{
    // Die Regler sind ECHTE Uniforms, kein const-Block: ein Zug am Schieber
    // setzt einen Wert und kostet keine Neuuebersetzung.
    ParamGruppe g;
    g.werte.push_back(wert("zahl", ParamTyp::Zahl));
    g.werte.push_back(wert("ganz", ParamTyp::Ganzzahl));
    g.werte.push_back(wert("wahl", ParamTyp::Auswahl));
    g.werte.push_back(wert("ja", ParamTyp::Bool));
    g.werte.push_back(wert("punkt", ParamTyp::Punkt2D));
    g.werte.push_back(wert("farbe", ParamTyp::Farbe));
    g.werte.push_back(wert("wort", ParamTyp::Text));

    const std::string frag = lumi::isffilter::wrapFragment("void main(){}", {}, g);
    CHECK(hat(frag, "uniform float zahl;"));
    CHECK(hat(frag, "uniform int ganz;"));
    CHECK(hat(frag, "uniform int wahl;"));  // Auswahl traegt den WERT, nicht den Index
    CHECK(hat(frag, "uniform bool ja;"));
    CHECK(hat(frag, "uniform vec2 punkt;"));
    CHECK(hat(frag, "uniform vec4 farbe;"));
    // Text hat in GLSL keine Entsprechung — er wird ausgelassen, nicht
    // als irgendein Notbehelf deklariert.
    CHECK_FALSE(hat(frag, "wort;"));

    // Dieselben Regler stehen auch in der Vertex-Stufe: ISF-Vertex-Shader
    // lesen sie (die Kanten-Filter rechnen ihre Offsets damit).
    const std::string vert = lumi::isffilter::wrapVertex("void main(){}", g);
    CHECK(hat(vert, "uniform float zahl;"));
    CHECK(hat(vert, "uniform vec2 punkt;"));
}

TEST_CASE("IsfFilterWrapper: Namenskollision nach der Import-Regel des Projekts")
{
    // ScriptBaseKeys.hpp (Entscheid D2): „reserviert" wirkt RELATIV zum
    // Quellformat. `bass`/`getspec` sind UNSERE Zutat und in ISF ohne
    // Bedeutung — ein gleichnamiger INPUT ist ein gewoehnlicher Nutzer-Name
    // und wird umbenannt. Ohne das stuende die Deklaration zweimal da und der
    // ganze Shader braeche.
    ParamGruppe g;
    g.werte.push_back(wert("bass", ParamTyp::Zahl));
    g.werte.push_back(wert("hell", ParamTyp::Zahl));
    const std::string frag = lumi::isffilter::wrapFragment("void main(){}", {}, g);

    // Unser `bass` bleibt genau EINMAL deklariert …
    CHECK(hat(frag, "uniform float bass;"));
    CHECK(frag.find("uniform float bass;") == frag.rfind("uniform float bass;"));
    // … der Regler bekommt einen eigenen Namen …
    CHECK(hat(frag, "uniform float _lumiIn_bass;"));
    // … und der ISF-Code bleibt UNVERAENDERT: er schreibt weiter `bass` und
    // meint damit seinen Regler — genau die Semantik des Datei-Autors.
    CHECK(hat(frag, "#define bass _lumiIn_bass"));

    // Ein Name ohne Kollision wird NICHT angefasst.
    CHECK(hat(frag, "uniform float hell;"));
    CHECK_FALSE(hat(frag, "_lumiIn_hell"));

    // ISF-BUILTINS werden nie umbenannt — `TIME` heisst in ISF `TIME`.
    CHECK_FALSE(lumi::isffilter::istPraeludenName("TIME"));
    CHECK_FALSE(lumi::isffilter::istPraeludenName("RENDERSIZE"));
    CHECK(lumi::isffilter::istPraeludenName("bass"));
    CHECK(lumi::isffilter::istPraeludenName("getspec"));
    // Unser eigener Reserviert-Raum ebenso.
    CHECK(lumi::isffilter::istPraeludenName("_lumiPrev"));

    // Auch Sampler-Namen: ein Bild-Input namens `beat` darf die Prälude
    // nicht zerschiessen.
    const std::string s =
        lumi::isffilter::wrapFragment("void main(){}", {{"beat", -1}}, {});
    CHECK(hat(s, "uniform sampler2D _lumiIn_beat;"));
    CHECK(hat(s, "#define beat _lumiIn_beat"));
}

TEST_CASE("IsfFilterWrapper: mehrere Bildquellen werden zu mehreren Samplern")
{
    // Ein Uebergang hat zwei — und keinen Sonderfall im Wrapper.
    const std::vector<IsfBildQuelle> q = {{"startImage", -1}, {"endImage", 2}};
    const std::string frag = lumi::isffilter::wrapFragment("void main(){}", q, {});
    CHECK(hat(frag, "uniform sampler2D startImage;"));
    CHECK(hat(frag, "uniform sampler2D endImage;"));

    // Ein Generator hat keine — dann gibt es auch keinen Sampler.
    const std::string gen = lumi::isffilter::wrapFragment("void main(){}", {}, {});
    CHECK_FALSE(hat(gen, "uniform sampler2D inputImage;"));

    // Eine namenlose Zeile wird ausgelassen statt `uniform sampler2D ;` zu
    // erzeugen — das waere ein Syntaxfehler im ganzen Shader.
    const std::string leer =
        lumi::isffilter::wrapFragment("void main(){}", {{"", -1}}, {});
    CHECK_FALSE(hat(leer, "sampler2D ;"));
}

TEST_CASE("IsfFilterWrapper: die Vertex-Stufe stellt isf_vertShaderInit()")
{
    const std::string vert = lumi::isffilter::wrapVertex(
        "void main() { isf_vertShaderInit(); }\n", {});
    CHECK(hat(vert, "layout(location = 0) in vec2 _lumiPos;"));
    CHECK(hat(vert, "out vec2 isf_FragNormCoord;"));
    CHECK(hat(vert, "#define isf_vertShaderInit()"));
    CHECK(hat(vert, "#define vv_vertShaderInit isf_vertShaderInit"));
    CHECK(hat(vert, "gl_Position"));
    // Der Epilog ruft die Initialisierung IMMER — auch ein Shader, der sie
    // vergisst, bekommt eine gueltige Position und ein gefuelltes Varying.
    const std::size_t epilog = vert.find("#undef main");
    REQUIRE(epilog != std::string::npos);
    CHECK(vert.find("isf_vertShaderInit();", epilog) != std::string::npos);

    // Auch hier: #line 1 direkt vor dem Nutzer-Code.
    CHECK(hat(vert, "#line 1\nvoid main() { isf_vertShaderInit(); }"));
}

TEST_CASE("IsfFilterWrapper: Starter-Shader erfuellt den Vertrag")
{
    const std::string starter = lumi::isffilter::starterFragment();
    CHECK(hat(starter, "void main"));
    CHECK(hat(starter, "gl_FragColor"));
    CHECK(hat(starter, "IMG_THIS_PIXEL"));
    CHECK(hat(starter, "bass"));  // audio-reaktiv ab dem ersten Frame
    // Er laeuft durch den Wrapper, ohne dass etwas fehlt.
    const std::string frag =
        lumi::isffilter::wrapFragment(starter, {{"inputImage", -1}}, {});
    CHECK(hat(frag, "uniform sampler2D inputImage;"));
    CHECK(hat(frag, "#undef main"));
}
