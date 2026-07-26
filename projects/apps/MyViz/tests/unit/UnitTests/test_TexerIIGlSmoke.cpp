/**
 ****************************************************************************************
 * @file   test_TexerIIGlSmoke.cpp
 * @brief  GL-Smoke-Gate fuer Texer II (S50): rendert die ECHTE Chain
 *         (Clear -> Texer II) offscreen und haelt die beiden an AvsRef
 *         gemessenen Eigenschaften fest — das Default-Sprite ist 20 px breit
 *         (nicht 16), und der Renderer blendet NICHT fest additiv, sondern
 *         folgt dem BLEND_LINE-Modus mit Default REPLACE.
 *
 * Beide Befunde fallen bei EINEM Frame auf Schwarz nicht auf: Replace und
 * Additiv liefern dort dasselbe Bild. Sichtbar werden sie erst, wenn Sprites
 * uebereinander liegen — also genau in den Presets, die davon leben
 * (Whacko Revisited: Milky Way Xtreme, High Voltage, Mister Santa).
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

constexpr int kSize = 128;

double luma(const QImage& img, int x, int y)
{
    const QColor c = img.pixelColor(x, y);
    return (c.redF() + c.greenF() + c.blueF()) / 3.0;
}

double lumaSum(const QImage& img)
{
    double sum = 0.0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) sum += luma(img, x, y);
    return sum;
}

/// Breite der Zeile durch die Bildmitte, in der ueberhaupt Licht liegt.
int litWidth(const QImage& img, int y, double threshold = 8.0 / 255.0)
{
    int first = -1;
    int last = -1;
    for (int x = 0; x < img.width(); ++x)
    {
        if (luma(img, x, y) > threshold)
        {
            if (first < 0) first = x;
            last = x;
        }
    }
    return first < 0 ? 0 : last - first + 1;
}

}  // namespace

TEST_CASE("TexerIIGlSmoke: Default-Sprite 20 px, Blend folgt dem Render-Mode")
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

    // Chain: Clear NUR im ersten Frame -> Texer II mit einem Sprite in der
    // Mitte. Ohne Clear ab Frame 2 macht sich ein additiver Blend sofort als
    // wachsende Energie bemerkbar.
    ChainNode root;
    root.params = ListParams{};

    ChainNode clear;
    ClearParams cp;
    cp.onlyFirst = true;
    clear.params = cp;

    ChainNode texer;
    TexerIIParams tp;
    tp.initCode = "n=1";
    tp.pointCode = "x=0;y=0;";  // ein Sprite, Originalgroesse
    texer.params = tp;

    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(texer));

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    ctx.functions()->glViewport(0, 0, kSize, kSize);

    const float dt = 1.0f / 60.0f;
    vis.render(dt);
    const QImage first = vis.debugGrabRootSurface();
    REQUIRE(!first.isNull());
    REQUIRE(first.width() == kSize);
    REQUIRE(first.height() == kSize);

    // Gate 1: Das Default-Sprite ist 20 px breit. Gemessen an AvsRef mit der
    // echten texer2.ape: Ausdehnung = 20 px * sizex, linear ueber sizex
    // 0,5..8. Der alte 16er-Kegel kam wegen des quadrierten Alphas sogar nur
    // auf 14 sichtbare Pixel.
    const int width = litWidth(first, kSize / 2);
    CHECK(width >= 19);
    CHECK(width <= 21);

    // Gate 2: Mitte hell, Rand dunkel — das Sprite sitzt, wo es soll.
    CHECK(luma(first, kSize / 2, kSize / 2) > 0.9);
    CHECK(luma(first, 4, 4) < 0.01);

    // Gate 3: zweiter Frame OHNE Clear, gleicher Sprite auf dieselbe Stelle.
    // Default-Blend ist REPLACE -> die Energie darf NICHT wachsen. Fest
    // additiv gerendert stieg sie in der Messung auf das 1,86-fache.
    const double sumFirst = lumaSum(first);
    REQUIRE(sumFirst > 0.0);
    vis.render(dt);
    const QImage second = vis.debugGrabRootSurface();
    REQUIRE(!second.isNull());
    const double sumSecond = lumaSum(second);
    CHECK(sumSecond == doctest::Approx(sumFirst).epsilon(0.02));

    // Gate 4: mit Set Render Mode = additiv (1) MUSS sie dagegen wachsen —
    // der Modus wird also wirklich beachtet und nicht nur ignoriert.
    ChainNode srm;
    SetRenderModeParams sp;
    sp.lineBlend = 1;  // additiv
    srm.params = sp;
    vis.chain().children.insert(vis.chain().children.begin() + 1, std::move(srm));
    vis.recompileChain();
    vis.render(dt);  // Frame mit Clear-Zustand von oben, jetzt additiv
    const double sumAdd = lumaSum(vis.debugGrabRootSurface());
    CHECK(sumAdd > sumFirst * 1.2);
}
