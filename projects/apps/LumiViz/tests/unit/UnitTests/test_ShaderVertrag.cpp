/**
 ****************************************************************************************
 * @file   test_ShaderVertrag.cpp
 * @brief  Tests fuer den Shader-Import/-Export-Vorbau (S71) und den
 *         Herkunfts-Kopf des Exports (Lizenz-Pflicht, S72)
 *
 * Der S71-Vorbau (`ShaderVertrag`-SSOT, Namensschema, Vertragspruefung,
 * freier Dateiname) war bis S72 UNGETESTET — er ist reine Logik und laesst
 * sich ohne GUI pruefen. Diese Datei schliesst die Luecke und haelt dabei die
 * drei Zusagen fest, an denen der Vorbau haengt:
 *   1. Ein fremder Shader im falschen Feld wird ERKANNT und benannt.
 *   2. Der Export-Name bleibt `<preset>[.<slot>].<vertrag>.<endung>`.
 *   3. Der Herkunfts-Kopf sieht NIE aus wie eine ISF-Datei.
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/panels/EelScriptEditing.hpp"

#include <QString>

namespace se = lumi::scriptedit;

TEST_CASE("ShaderVertrag: die SSOT kennt jeden Vertrag genau einmal")
{
    const auto& alle = se::shaderVertraege();
    REQUIRE(!alle.isEmpty());
    for (const auto& v : alle)
    {
        CHECK(!v.key.isEmpty());
        CHECK(!v.funktion.isEmpty());
        CHECK(!v.signatur.isEmpty());
        // Die Signatur muss den Einstieg auch wirklich nennen — sonst zeigt die
        // Warnung beim Import auf eine Funktion, die es so nicht gibt.
        CHECK(v.signatur.contains(v.funktion));
        // Die Endung sagt, was die Datei WIRKLICH ist: Milkdrop = HLSL (S71),
        // ISF = `.fs` (die Konvention der Bibliothek, S72).
        CHECK((v.endung == QStringLiteral("glsl") || v.endung == QStringLiteral("hlsl") ||
               v.endung == QStringLiteral("fs")));
        // Nachschlagen liefert denselben Eintrag zurueck.
        const se::ShaderVertrag* gefunden = se::shaderVertrag(v.key);
        REQUIRE(gefunden != nullptr);
        CHECK(gefunden->funktion == v.funktion);
    }
    CHECK(se::shaderVertrag(QStringLiteral("gibtsnicht")) == nullptr);
    // Der S70-Befund als Waechter: `filter` ist in GLSL RESERVIERT, der
    // pixelFilter-Vertrag heisst deshalb `farbe`.
    const se::ShaderVertrag* pf = se::shaderVertrag(QStringLiteral("pixelfilter"));
    REQUIRE(pf != nullptr);
    CHECK(pf->funktion == QStringLiteral("farbe"));

    // ISF hat einen EIGENEN Knoten (Entscheid Patrik S72) — der Stilfilter
    // bleibt, was er war. Nur dort gehoert `*.fs` in die Vorauswahl des
    // Dateidialogs; anderswo waere es ein falsches Versprechen. (Ohne den
    // Eintrag stand im Dialog nur `*.<vertrag>.<endung>`, eine frisch
    // heruntergeladene ISF-Datei war also unsichtbar — Befund S72, Frage
    // Patrik „wie kann ich die nun importieren?".)
    CHECK(pf->importZusatz.isEmpty());
    const se::ShaderVertrag* isf = se::shaderVertrag(QStringLiteral("isffilter"));
    REQUIRE(isf != nullptr);
    CHECK(isf->importZusatz.contains(QStringLiteral("*.fs")));
    CHECK(isf->endung == QStringLiteral("fs"));
    CHECK(se::shaderVertrag(QStringLiteral("shadertoy"))->importZusatz.isEmpty());
    CHECK(se::shaderVertrag(QStringLiteral("milkdrop"))->importZusatz.isEmpty());
}

TEST_CASE("ShaderVertrag: Import erkennt den fremden Vertrag und benennt ihn")
{
    // Ein Shadertoy-Shader, der in ein pixelFilter-Feld geladen wird: ohne
    // Pruefung gaebe das nur einen kryptischen Compilerfehler aus dem Wrapper.
    const QString shadertoy =
        QStringLiteral("void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
                       "{ fragColor = vec4(1.0); }\n");
    const QString warnung =
        se::pruefeShaderVertrag(shadertoy, QStringLiteral("pixelfilter"));
    CHECK(!warnung.isEmpty());
    CHECK(warnung.contains(QStringLiteral("Shadertoy")));

    // Derselbe Inhalt im RICHTIGEN Feld: keine Warnung.
    CHECK(se::pruefeShaderVertrag(shadertoy, QStringLiteral("shadertoy")).isEmpty());

    // Ein passender pixelFilter ebenfalls nicht.
    const QString filter =
        QStringLiteral("vec4 farbe(vec2 uv, vec4 src) { return 1.0 - src; }\n");
    CHECK(se::pruefeShaderVertrag(filter, QStringLiteral("pixelfilter")).isEmpty());

    // Fragmente/Hilfsfunktionen bleiben erlaubt (Entscheid Patrik S71): nichts
    // Bekanntes erkannt => keine Warnung, nicht etwa ein Verbot.
    CHECK(se::pruefeShaderVertrag(QStringLiteral("float helfer(float x){return x;}"),
                                  QStringLiteral("pixelfilter"))
              .isEmpty());

    // ISF wird am JSON-Kopf erkannt und beim Namen genannt.
    const QString isf = QStringLiteral(
        "/*{\n  \"DESCRIPTION\": \"test\",\n  \"CATEGORIES\": [\"FX\"]\n}*/\n"
        "void main() { gl_FragColor = vec4(0.0); }\n");
    const QString isfWarnung =
        se::pruefeShaderVertrag(isf, QStringLiteral("pixelfilter"));
    CHECK(!isfWarnung.isEmpty());
    CHECK(isfWarnung.contains(QStringLiteral("ISF")));
}

TEST_CASE("ShaderVertrag: Export-Name bleibt <preset>[.<slot>].<vertrag>.<endung>")
{
    // Klassifikation von RECHTS nach LINKS immer spezifischer (Entscheid S71).
    CHECK(se::shaderExportName(QStringLiteral("Mein Preset"),
                               QStringLiteral("shadertoy"),
                               QStringLiteral("image")) ==
          QStringLiteral("Mein_Preset.image.shadertoy.glsl"));
    CHECK(se::shaderExportName(QStringLiteral("Take-On-Me"),
                               QStringLiteral("pixelfilter")) ==
          QStringLiteral("Take-On-Me.pixelfilter.glsl"));
    // Milkdrop-Felder SIND HLSL — die Endung sagt die Wahrheit (Entscheid S71).
    CHECK(se::shaderExportName(QStringLiteral("RTH"), QStringLiteral("milkdrop"),
                               QStringLiteral("warp")) ==
          QStringLiteral("RTH.warp.milkdrop.hlsl"));
    // Pfadgefaehrliche Zeichen fliegen raus, Leeres bekommt einen Ersatznamen.
    CHECK(se::shaderExportName(QStringLiteral("a/b:c*d"),
                               QStringLiteral("pixelfilter")) ==
          QStringLiteral("abcd.pixelfilter.glsl"));
    CHECK(se::shaderExportName(QStringLiteral("   "),
                               QStringLiteral("pixelfilter")) ==
          QStringLiteral("preset.pixelfilter.glsl"));
}

TEST_CASE("Herkunft: der Export-Kopf traegt Autor und Lizenz")
{
    // BEFUND S71: der Export schrieb nur den Code — beim Weitergeben ging die
    // Herkunft verloren. Der Kopf schliesst das (Lizenz-Pflicht S72).
    const QString kopf = se::herkunftKopf(
        QStringLiteral("Seascape"), QStringLiteral("TDM"),
        QStringLiteral("https://www.shadertoy.com/view/Ms2SD1"),
        QStringLiteral("CC BY-NC-SA 3.0"));
    CHECK(kopf.contains(QStringLiteral("Seascape")));
    CHECK(kopf.contains(QStringLiteral("TDM")));
    CHECK(kopf.contains(QStringLiteral("shadertoy.com/view/Ms2SD1")));
    CHECK(kopf.contains(QStringLiteral("CC BY-NC-SA 3.0")));

    // WAECHTER: der Kopf darf NIE wie ein ISF-Kopf aussehen — `/*{` ist dort
    // die Formaterkennung, und ein Export ist keine ISF-Datei.
    CHECK_FALSE(kopf.startsWith(QStringLiteral("/*{")));
    CHECK(se::pruefeShaderVertrag(kopf + QStringLiteral("vec4 farbe(vec2 uv, vec4 s)"
                                                        "{ return s; }"),
                                  QStringLiteral("pixelfilter"))
              .isEmpty());

    // Der Block muss sich schliessen — ein `*/` IM Wert wuerde ihn sonst
    // vorzeitig beenden und den Rest des Shaders zu Kommentar machen.
    const QString boshaft =
        se::herkunftKopf(QStringLiteral("x */ int hack;"), {}, {}, {});
    CHECK_FALSE(boshaft.contains(QStringLiteral("*/ int hack")));
    CHECK(boshaft.endsWith(QStringLiteral("*/\n")));

    // Nichts bekannt => gar kein Kopf (kein leerer Kommentarblock im Export).
    CHECK(se::herkunftKopf({}, {}, {}, {}).isEmpty());
}
