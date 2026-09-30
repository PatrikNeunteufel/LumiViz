/**
 ****************************************************************************************
 * @file   test_ShaderVorlagen.cpp
 * @brief  Shader-Vorlagen (S83): jede nennt ihren Einstieg und laeuft im echten Knoten
 *
 * @details
 * Eine Vorlage, die nicht kompiliert, ist schlimmer als keine — sie ist der
 * Startpunkt, von dem aus jemand weiterschreibt. Deshalb:
 * - Store: „Empty" steht immer vorn, eigene verdraengen gleichnamige mitgelieferte.
 * - Jede Vorlage (auch „Empty") nennt den Einstieg ihres Vertrags.
 * - Jede HLSL-Vorlage uebersetzt der HlslTranspiler.
 * - Jede GLSL-Vorlage kompiliert im echten Knoten (GL-Kontext; ohne Kontext
 *   wird dieser Teil uebersprungen, wie in test_ScriptVariablesGl).
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/panels/EelScriptEditing.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ShaderVorlagen.hpp"

#include <HlslTranspiler.hpp>

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTemporaryDir>

#include <optional>
#include <string>

namespace sv = lumi::multieffect::shadervorlagen;
using namespace lumi::multieffect;

namespace {

/// Schreibt eine Vorlagen-Datei von Hand (der Store schreibt nur in user/).
void schreibe(const QString& pfad, const QString& text)
{
    QDir().mkpath(QFileInfo(pfad).absolutePath());
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write(text.toUtf8());
}

struct TestWurzel
{
    QTemporaryDir dir;
    TestWurzel() { sv::setRootForTesting(dir.path()); }
    ~TestWurzel() { sv::setRootForTesting(QString()); }
};

/// Knoten, dessen Shader-Feld den Vorlagen-Text traegt (nullopt = kein GLSL).
std::optional<EffectParams> knotenMit(const QString& key, const std::string& code)
{
    if (key == QLatin1String("shadertoy"))
    {
        ShadertoyParams p;
        p.code = code;
        return EffectParams{p};
    }
    if (key == QLatin1String("meshwarp"))
    {
        MeshWarpParams p;
        p.code = code;
        return EffectParams{p};
    }
    if (key == QLatin1String("pixelfilter"))
    {
        PixelFilterParams p;
        p.code = code;
        return EffectParams{p};
    }
    if (key == QLatin1String("gpuparticles"))
    {
        GpuParticlesParams p;
        p.forceCode = code;
        return EffectParams{p};
    }
    if (key == QLatin1String("isffilter"))
    {
        IsfFilterParams p;
        p.fragCode = code;
        return EffectParams{p};
    }
    if (key == QLatin1String("isffilter/vs"))
    {
        IsfFilterParams p;
        p.vertexCode = code;
        return EffectParams{p};
    }
    return std::nullopt;
}

}  // namespace

TEST_CASE("ShaderVorlagen: Empty steht vorn und nennt den Einstieg jedes Schluessels")
{
    TestWurzel wurzel;
    for (const QString& key : sv::schluessel())
    {
        INFO(key.toStdString());
        const auto eintraege = sv::liste(key);
        REQUIRE_FALSE(eintraege.empty());
        CHECK(eintraege.front().name == QStringLiteral("Empty"));
        CHECK(eintraege.front().text == sv::leererRumpf(key));
        const auto* vertrag =
            lumi::scriptedit::shaderVertrag(key.section(QLatin1Char('/'), 0, 0));
        REQUIRE(vertrag != nullptr);
        CHECK(lumi::scriptedit::nenntFunktion(eintraege.front().text, vertrag->funktion));
    }
    CHECK(sv::liste(QStringLiteral("gibtsnicht")).empty());
    CHECK(sv::leererRumpf(QStringLiteral("gibtsnicht")).isEmpty());
}

TEST_CASE("ShaderVorlagen: eigene Vorlage verdraengt die gleichnamige mitgelieferte")
{
    TestWurzel wurzel;
    const QString asset = QDir(wurzel.dir.path()).filePath(QStringLiteral("asset/pixelfilter"));
    schreibe(asset + QStringLiteral("/Grayscale.glsl"), QStringLiteral("// werk"));
    schreibe(asset + QStringLiteral("/Posterize.glsl"), QStringLiteral("// werk"));
    schreibe(asset + QStringLiteral("/falsch.hlsl"), QStringLiteral("// andere Endung"));

    QString pfad;
    REQUIRE(sv::speichere(QStringLiteral("pixelfilter"), QStringLiteral("Grayscale"),
                          QStringLiteral("// eigen"), &pfad));
    CHECK(pfad.endsWith(QStringLiteral("user/pixelfilter/Grayscale.glsl")));

    const auto e = sv::liste(QStringLiteral("pixelfilter"));
    REQUIRE(e.size() == 3);  // Empty + Grayscale (eigen) + Posterize
    CHECK(e[1].name == QStringLiteral("Grayscale"));
    CHECK_FALSE(e[1].builtin);
    CHECK(e[1].text == QStringLiteral("// eigen"));
    CHECK(e[2].name == QStringLiteral("Posterize"));
    CHECK(e[2].builtin);

    // Stufen-Schluessel sind eigene Unterordner mit eigener Endung.
    REQUIRE(sv::speichere(QStringLiteral("milkdrop/comp"), QStringLiteral("a/b:c"),
                          QStringLiteral("shader_body { ret = 0; }"), &pfad));
    CHECK(pfad.endsWith(QStringLiteral("user/milkdrop/comp/a_b_c.hlsl")));
    CHECK_FALSE(sv::speichere(QStringLiteral("gibtsnicht"), QStringLiteral("x"),
                              QStringLiteral("x")));
}

TEST_CASE("ShaderVorlagen: jede mitgelieferte Vorlage nennt ihren Einstieg, HLSL uebersetzt")
{
    int geprueft = 0;
    for (const QString& key : sv::schluessel())
    {
        REQUIRE_MESSAGE(!sv::assetDir(key).isEmpty(), "asset/shadertemplates nicht gefunden");
        const auto* vertrag =
            lumi::scriptedit::shaderVertrag(key.section(QLatin1Char('/'), 0, 0));
        REQUIRE(vertrag != nullptr);
        for (const sv::Eintrag& e : sv::liste(key))
        {
            INFO(key.toStdString() << " / " << e.name.toStdString());
            CHECK(lumi::scriptedit::nenntFunktion(e.text, vertrag->funktion));
            // Eine Vorlage darf nie wie eine ISF-Datei aussehen, wenn sie keine ist.
            CHECK_FALSE(lumi::scriptedit::istIsfDatei(e.text));
            if (key.startsWith(QLatin1String("milkdrop")))
            {
                const auto r = lumi::hlsl::transpile(e.text.toStdString(),
                                                     key.endsWith(QLatin1String("warp"))
                                                         ? lumi::hlsl::ShaderKind::Warp
                                                         : lumi::hlsl::ShaderKind::Comp);
                CHECK_MESSAGE(r.ok, r.error);
            }
            if (e.pfad.isEmpty()) continue;
            ++geprueft;
        }
    }
    CHECK(geprueft >= 16);  // Schutz gegen einen Test, der still nichts prueft
}

TEST_CASE("ShaderVorlagen: jede GLSL-Vorlage kompiliert im echten Knoten")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    std::optional<QGuiApplication> app;
    if (QCoreApplication::instance() == nullptr) app.emplace(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    constexpr int kSize = 64;
    int geprueft = 0;
    for (const QString& key : sv::schluessel())
    {
        for (const sv::Eintrag& e : sv::liste(key))
        {
            const auto params = knotenMit(key, e.text.toStdString());
            if (!params) continue;  // HLSL: oben ueber den Transpiler
            INFO(key.toStdString() << " / " << e.name.toStdString());
            ChainNode root;
            root.params = ListParams{};
            ChainNode leaf;
            leaf.params = *params;
            root.children.push_back(std::move(leaf));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.initialize();
            vis.resize(QSize(kSize, kSize));
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
            vis.render(1.0f / 60.0f);
            vis.render(1.0f / 60.0f);
            const std::uint64_t nodeId = vis.chain().children.front().nodeId;
            REQUIRE(nodeId != 0);
            const std::string fehler = vis.shadertoyError(nodeId);
            CHECK_MESSAGE(fehler.empty(), fehler);
            ++geprueft;
        }
    }
    CHECK(geprueft >= 18);  // 6 GLSL-Schluessel × (Leer + Dateien)
}
