/**
 ****************************************************************************************
 * @file   test_MilkdropGlSmoke.cpp
 * @brief  GL-Smoke-Test (Stufe C1): kompiliert die transpilierten Shader der
 *         c1-Kalibrier-Presets ueber den ECHTEN Qt-Pfad in einem Offscreen-
 *         GL-3.3-Kontext — reproduziert Treiber-/Qt-Fehler, die der reine
 *         glslangValidator nicht sieht (Session-40-Diagnose)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <MilkParser.hpp>

#include "visualizers/MilkdropVisualizer.hpp"

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>

#include <filesystem>

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

} // namespace

TEST_CASE("MilkdropGlSmoke: c1-Shader kompilieren + linken im 3.3-Core-Kontext")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Smoke uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));
    MESSAGE("GL-Kontext: ", ctx.format().majorVersion(), ".", ctx.format().minorVersion(),
            " profile=", static_cast<int>(ctx.format().profile()));

    const std::filesystem::path dir =
        repoRoot() / "asset" / "calibration" / "milkdrop" / "c1";
    REQUIRE(std::filesystem::exists(dir));

    int shaders = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        const lumi::milk::ParseResult parsed = lumi::milk::parseFile(entry.path());
        REQUIRE(parsed.ok);
        const auto compile = [&](const std::string& text, bool isWarp) {
            if (text.empty()) return;
            ++shaders;
            const std::string frag =
                MilkdropVisualizer::debugAssembleFragment(text, isWarp);
            REQUIRE_MESSAGE(frag.rfind("// TRANSPILE-FEHLER", 0) != 0, frag);
            QOpenGLShaderProgram program;
            const bool okVert = program.addShaderFromSourceCode(
                QOpenGLShader::Vertex, MilkdropVisualizer::debugCustomVertexShader());
            const bool okFrag = program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                                frag.c_str());
            const bool allOk = okVert && okFrag && program.link();
            CHECK_MESSAGE(allOk, entry.path().filename().string(), " (",
                          (isWarp ? "warp" : "comp"), "): ", program.log().toStdString());
        };
        compile(parsed.warpShader, true);
        compile(parsed.compShader, false);
    }
    MESSAGE("GL-Smoke: ", shaders, " Shader kompiliert/gelinkt");
    CHECK(shaders == 8);
    ctx.doneCurrent();
}
