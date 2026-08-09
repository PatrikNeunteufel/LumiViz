/**
 ****************************************************************************************
 * @file   test_BloomGlSmoke.cpp
 * @brief  GL-Smoke-Gate fuer das Bloom-Modul (Lights-Etappe 1, S48): rendert
 *         die ECHTE Chain (Clear -> SuperScope-Punkt -> Bloom) offscreen und
 *         prueft die Entwurfs-Bedingungen — Nachbarn leuchten auf, der Peak
 *         bleibt, die Gesamtenergie waechst additiv (~2x), Threshold 1
 *         schaltet den Glow ab (Lights_Module_Entwurf.md, Modul 1)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

using namespace lumi::multieffect;

namespace {

constexpr int kSize = 128;  // Testflaeche (quadratisch, Impuls in der Mitte)

/// Mittlere Helligkeit 0..1 eines Pixels (QImage aus toImage()).
double luma(const QImage& img, int x, int y)
{
    const QColor c = img.pixelColor(x, y);
    return (c.redF() + c.greenF() + c.blueF()) / 3.0;
}

/// Summe der Helligkeit ueber das ganze Bild (Energie-Metrik des Gates).
double lumaSum(const QImage& img)
{
    double sum = 0.0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) sum += luma(img, x, y);
    return sum;
}

}  // namespace

TEST_CASE("BloomGlSmoke: Impuls -> Glow (Nachbarn > 0, Peak bleibt, Summe ~2x)")
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

    // Chain: Clear (schwarz) -> SuperScope (ein weisser Punkt, Mitte) -> Bloom
    // (zunaechst deaktiviert — der erste Frame liefert die Referenz ohne Glow).
    ChainNode root;
    root.params = ListParams{};

    ChainNode clear;
    clear.params = ClearParams{};  // schwarz, jeden Frame, replace

    ChainNode scope;
    SuperScopeParams sp;
    sp.initCode = "n=1";
    sp.pointCode = "x=0;y=0;red=1;green=1;blue=1";
    sp.renderMode = 0;  // dots
    sp.dotSize = 8.0f;  // genug Energie, dass der Glow im 8-Bit-Readback lebt
    scope.params = sp;

    ChainNode bloom;
    BloomParams bp;
    bp.downsample = 1;  // Glow-RT 64x64
    bp.radius = 6;      // sigma 6 Glow-px ~= 12 px auf der Surface
    bp.post = false;    // In-Chain-Modus: Composite auf die Surface (messbar
                        // ueber debugGrabRootSurface; post=true testet Gate 5)
    bloom.params = bp;
    bloom.enabled = false;

    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(scope));
    root.children.push_back(std::move(bloom));

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    ctx.functions()->glViewport(0, 0, kSize, kSize);

    const float dt = 1.0f / 60.0f;
    vis.render(dt);
    const QImage before = vis.debugGrabRootSurface();
    REQUIRE(!before.isNull());
    REQUIRE(before.width() == kSize);

    const int cx = kSize / 2;
    const int cy = kSize / 2;
    // Referenzbild: Punkt in der Mitte hell, 16 px daneben schwarz.
    REQUIRE(luma(before, cx, cy) > 0.9);
    REQUIRE(luma(before, cx + 16, cy) < 0.005);

    // Bloom aktivieren (Single-Thread-Test: kein renderMutex noetig) und
    // denselben Frame erneut rendern — Clear macht jeden Frame deterministisch.
    vis.chain().children[2].enabled = true;
    vis.recompileChain();
    vis.render(dt);
    const QImage after = vis.debugGrabRootSurface();
    REQUIRE(!after.isNull());

    // Gate 1: Nachbarn ausserhalb des Punkts leuchten jetzt (Soft-Glow).
    CHECK(luma(after, cx + 16, cy) > 0.01);
    CHECK(luma(after, cx, cy + 16) > 0.01);
    // Gate 2: der Peak bleibt (additives Composite senkt die Mitte nie).
    CHECK(luma(after, cx, cy) >= luma(before, cx, cy) - 0.01);
    // Gate 3: Gesamtenergie waechst additiv — Blur erhaelt die Energie, das
    // Composite addiert sie einmal dazu (~2x; Clamp am Punkt kostet etwas).
    const double sumBefore = lumaSum(before);
    const double sumAfter = lumaSum(after);
    REQUIRE(sumBefore > 0.0);
    CHECK(sumAfter / sumBefore > 1.5);
    CHECK(sumAfter / sumBefore < 2.5);

    // Gate 4: Threshold 1.0 nimmt dem Glow alles — Bild == Referenz (±2 %).
    {
        auto& params = std::get<BloomParams>(vis.chain().children[2].params);
        params.threshold = 1.0f;
    }
    vis.recompileChain();
    vis.render(dt);
    const QImage thresholded = vis.debugGrabRootSurface();
    CHECK(lumaSum(thresholded) == doctest::Approx(sumBefore).epsilon(0.02));
    CHECK(luma(thresholded, cx + 16, cy) < 0.005);

    // Gate 5: post=true (Default) laesst die Chain-Surface unberuehrt — der
    // Glow entsteht erst beim Present (kein Feedback; S48-Befund: in-chain
    // akkumulierte der additive Glow in Fadeout-Ketten bis Weiss).
    {
        auto& params = std::get<BloomParams>(vis.chain().children[2].params);
        params.threshold = 0.0f;
        params.post = true;
    }
    vis.recompileChain();
    vis.render(dt);
    const QImage postSurface = vis.debugGrabRootSurface();
    CHECK(lumaSum(postSurface) == doctest::Approx(sumBefore).epsilon(0.02));
    CHECK(luma(postSurface, cx + 16, cy) < 0.005);

    vis.cleanup();
    ctx.doneCurrent();
}
