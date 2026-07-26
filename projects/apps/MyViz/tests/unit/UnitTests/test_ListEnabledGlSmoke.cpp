/**
 ****************************************************************************************
 * @file   test_ListEnabledGlSmoke.cpp
 * @brief  GL-Gate fuer die `enabled`-Semantik einer Effect-List mit EEL-Code
 *         (S50): der im Preset gespeicherte Schalter ist nur die VORBELEGUNG
 *         der EEL-Variablen, danach entscheidet das Skript
 *         (r_list.cpp:399 setzt, :419 liest zurueck).
 *
 * Das Idiom dahinter steht 75-mal allein im Pack "Whacko Revisited": die Liste
 * liegt AUSGESCHALTET im Preset und schaltet sich per
 * `enabled=bnot(equal(lw,w)*equal(lh,h))` im ersten Frame (und nach jedem
 * Resize) selbst ein, um einen Global-Buffer einmalig zu fuellen. Wer den
 * Knoten wegen des Schalters ueberspringt, fuehrt den Code nie aus — die Liste
 * bleibt fuer immer aus und der Puffer leer.
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

constexpr int kSize = 64;

double centerLuma(const QImage& img)
{
    const QColor c = img.pixelColor(img.width() / 2, img.height() / 2);
    return (c.redF() + c.greenF() + c.blueF()) / 3.0;
}

/// Chain: Liste (evtl. deaktiviert, mit EEL) die auf WEISS clear-t.
ChainNode makeChain(bool listEnabled, const std::string& frameCode)
{
    ChainNode root;
    root.params = ListParams{};

    ChainNode inner;
    ListParams lp;
    lp.useCode = !frameCode.empty();
    lp.frameCode = frameCode;
    inner.params = lp;
    inner.enabled = listEnabled;

    ChainNode clear;
    ClearParams cp;
    cp.color = 0xFFFFFF;
    inner.children.push_back(std::move(clear));
    inner.children.back().params = cp;

    root.children.push_back(std::move(inner));
    return root;
}

double renderCenter(ChainNode chain)
{
    MultiEffectVisualizer vis;
    vis.setChain(std::move(chain));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
    vis.render(1.0f / 60.0f);
    const QImage img = vis.debugGrabRootSurface();
    REQUIRE(!img.isNull());
    return centerLuma(img);
}

}  // namespace

TEST_CASE("ListEnabledGlSmoke: EEL-Code ueberstimmt den gespeicherten Schalter")
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

    // Referenz: aktivierte Liste ohne Code -> weiss.
    CHECK(renderCenter(makeChain(true, "")) > 0.9);
    // Bestand: deaktivierte Liste ohne Code bleibt aus -> schwarz.
    CHECK(renderCenter(makeChain(false, "")) < 0.05);

    // Kern des Gates: deaktiviert gespeichert, aber der Code schaltet ein.
    CHECK(renderCenter(makeChain(false, "enabled=1;")) > 0.9);
    // Gegenrichtung: aktiviert gespeichert, aber der Code schaltet aus.
    CHECK(renderCenter(makeChain(true, "enabled=0;")) < 0.05);
    // Ohne Zuweisung gewinnt die Vorbelegung — in BEIDE Richtungen.
    CHECK(renderCenter(makeChain(false, "q=1;")) < 0.05);
    CHECK(renderCenter(makeChain(true, "q=1;")) > 0.9);
}
