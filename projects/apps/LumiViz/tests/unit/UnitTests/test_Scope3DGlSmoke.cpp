/**
 ****************************************************************************************
 * @file   test_Scope3DGlSmoke.cpp
 * @brief  GL-Smoke-Gates fuer 3D-Camera + SuperScope 3D (Lights-Etappe 1,
 *         S48): rendert die echte Chain offscreen und prueft die
 *         Entwurfs-Fixpunkte — Punkt auf der Kameraachse -> Bildmitte,
 *         Size-Attenuation ~1/Tiefe, Kamera-Beat-Slot verschiebt das Bild
 *         (dynamische Modulparameter + Frame-VOR-Beat-Ordnung)
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

#include <filesystem>

using namespace lumi::multieffect;

namespace {

constexpr int kSize = 128;

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

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

/// Spalte des hellsten Pixels (bei Gleichstand die erste).
int brightestColumn(const QImage& img)
{
    int bestX = 0;
    double best = -1.0;
    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x < img.width(); ++x)
        {
            const double l = luma(img, x, y);
            if (l > best)
            {
                best = l;
                bestX = x;
            }
        }
    }
    return bestX;
}

}  // namespace

TEST_CASE("Scope3DGlSmoke: Fixpunkt, 1/z-Attenuation, Kamera-Beat-Slot")
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

    const float dt = 1.0f / 60.0f;
    const int cx = kSize / 2;
    const int cy = kSize / 2;

    // Chain: Clear -> SuperScope 3D (ein Punkt im Ursprung, Fallback-Kamera).
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode scope;
    SuperScope3DParams sp;
    sp.pointCount = 1;
    sp.pointCode = "x=0;y=0;z=0";  // Ursprung = Kameraachse der Fallback-Kamera
    sp.size = 0.3f;                // Welt-Einheiten; gross genug fuers Readback
    scope.params = sp;
    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(scope));

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    ctx.functions()->glViewport(0, 0, kSize, kSize);

    vis.render(dt);
    const QImage imgNear = vis.debugGrabRootSurface();
    REQUIRE(!imgNear.isNull());

    // Gate 1 (Entwurf): Punkt auf der Kameraachse -> Bildmitte; Ecken schwarz.
    CHECK(luma(imgNear, cx, cy) > 0.5);
    CHECK(luma(imgNear, 4, 4) < 0.005);
    CHECK(luma(imgNear, kSize - 5, kSize - 5) < 0.005);
    const double sumNear = lumaSum(imgNear);
    REQUIRE(sumNear > 0.0);

    // Gate 2 (Entwurf): Size-Attenuation ~1/Tiefe. Punkt nach z=-2 (weiter weg
    // von der Fallback-Kamera bei z=+3.73, Blick Richtung -z): Distanz
    // 3.73 -> 5.73, projizierte Groesse x0.65, Energie (Flaeche) ~x0.42 —
    // der Peak bleibt in der Bildmitte.
    {
        auto& p = std::get<SuperScope3DParams>(vis.chain().children[1].params);
        p.pointCode = "x=0;y=0;z=-2";
    }
    vis.render(dt);
    const QImage imgFar = vis.debugGrabRootSurface();
    CHECK(luma(imgFar, cx, cy) > 0.5);  // immer noch Bildmitte
    const double ratio = lumaSum(imgFar) / sumNear;
    CHECK(ratio > 0.2);
    CHECK(ratio < 0.7);

    // Gate 3: Kamera-Knoten mit EEL-Beat-Slot — dynamische Modulparameter +
    // Frame-VOR-Beat-Ordnung (S47): Frame setzt tx=0, der Beat (erzwungen via
    // --beat-period 1) setzt tx=1 DANACH — die Kamera schwenkt nach rechts am
    // Punkt vorbei, der Punkt wandert in die linke Bildhaelfte (Kamera bei +z,
    // Blick -z: Schwenk nach +x schiebt die Szene nach links). Liefe Beat vor
    // Frame, gewaenne tx=0 und der Punkt bliebe in der Mitte.
    ChainNode root2;
    root2.params = ListParams{};
    ChainNode clear2;
    clear2.params = ClearParams{};
    ChainNode cam;
    Camera3DParams cp;
    cp.frameCode = "tx=0";
    cp.beatCode = "tx=1";
    cam.params = cp;
    ChainNode scope2;
    SuperScope3DParams sp2;
    sp2.pointCount = 1;
    sp2.pointCode = "x=0;y=0;z=0";
    sp2.size = 0.3f;
    scope2.params = sp2;
    root2.children.push_back(std::move(clear2));
    root2.children.push_back(std::move(cam));
    root2.children.push_back(std::move(scope2));

    vis.setChain(std::move(root2));
    vis.setBeatPeriodOverride(1);  // deterministischer Beat auf jedem Frame
    vis.render(dt);
    const QImage shifted = vis.debugGrabRootSurface();
    CHECK(luma(shifted, cx, cy) < 0.05);       // Mitte verlassen (Beat gewann)
    CHECK(brightestColumn(shifted) < cx - 5);  // Punkt in der linken Haelfte

    // ------------------------------------------------------------------
    // Gate 4 (S48-Befund "fog scheint nicht zu funktionieren"): Kamera-Fog
    // ueber PARAMS — fogStart=1/fogEnd=3 bei Punktdistanz 3,73 => fogF
    // clampt auf 1, der additive Sprite ist KOMPLETT gedaempft (schwarz).
    // Fog aus (0/0) => wieder hell. Prueft den Param-Pfad OHNE Skripte.
    // ------------------------------------------------------------------
    ChainNode root3;
    root3.params = ListParams{};
    ChainNode clear3;
    clear3.params = ClearParams{};
    ChainNode cam3;
    Camera3DParams cp3;
    cp3.fogStart = 1.0f;
    cp3.fogEnd = 3.0f;
    cam3.params = cp3;
    ChainNode scope3;
    SuperScope3DParams sp3;
    sp3.pointCount = 1;
    sp3.pointCode = "x=0;y=0;z=0";
    sp3.size = 0.3f;
    scope3.params = sp3;
    root3.children.push_back(std::move(clear3));
    root3.children.push_back(std::move(cam3));
    root3.children.push_back(std::move(scope3));

    vis.setBeatPeriodOverride(0);
    vis.setChain(std::move(root3));
    vis.render(dt);
    const QImage fogged = vis.debugGrabRootSurface();
    CHECK(luma(fogged, cx, cy) < 0.01);  // voll im Fog

    {
        auto& p = std::get<Camera3DParams>(vis.chain().children[1].params);
        p.fogStart = 0.0f;
        p.fogEnd = 0.0f;  // start >= end -> Fog aus
    }
    vis.render(dt);
    const QImage unfogged = vis.debugGrabRootSurface();
    CHECK(luma(unfogged, cx, cy) > 0.5);

    // Gate 5: Fog per SKRIPT gesetzt (init: fogstart/fogend) — der Pfad der
    // "Fly Forward (Fog)"-Vorlage. Punkt bei Distanz 3,73 wieder voll im Fog.
    {
        auto& p = std::get<Camera3DParams>(vis.chain().children[1].params);
        p.initCode = "fogstart=1;fogend=3";
    }
    vis.render(dt);
    const QImage scriptFog = vis.debugGrabRootSurface();
    CHECK(luma(scriptFog, cx, cy) < 0.01);

    vis.cleanup();
    ctx.doneCurrent();
}

TEST_CASE("Lights3DGlSmoke: Terrain sichtbar + Orb-Verdeckung (Depth-RT)")
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

    const float dt = 1.0f / 60.0f;
    const int cx = kSize / 2;
    const int cy = kSize / 2;

    // --- Terrain: Gitterpunkte leuchten in der unteren Bildhaelfte -----------
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode terrain;
        Terrain3DParams tp;
        tp.ringAmp = 0.0f;  // ohne Audio deterministisch
        tp.baseAmp = 0.3f;
        tp.colorLow = 0x4080FF;  // beide Palettenenden hell genug fuers Readback
        tp.colorHigh = 0xFFFFFF;
        terrain.params = tp;
        root.children.push_back(std::move(clear));
        root.children.push_back(std::move(terrain));

        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        ctx.functions()->glViewport(0, 0, kSize, kSize);
        vis.render(dt);
        const QImage img = vis.debugGrabRootSurface();
        REQUIRE(!img.isNull());

        // Unterhalb des Horizonts liegt Licht (Gitterpunkte), oben bleibt es
        // schwarz (Terrain bei y=-0.8, Kamera blickt geradeaus).
        double lower = 0.0;
        double upper = 0.0;
        for (int y = 0; y < kSize; ++y)
            for (int x = 0; x < kSize; ++x)
                (y > cy ? lower : upper) += luma(img, x, y);
        CHECK(lower > 1.0);
        CHECK(upper < lower * 0.1);
        vis.cleanup();
    }

    // --- Orbs: vorderer Orb verdeckt den hinteren (gemeinsames Depth-RT) -----
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode orbs;
        GlowOrbsParams gp;
        gp.orbCount = 2;
        gp.haloIntensity = 0.0f;  // reiner Mesh-Test
        // Orb 0 (i=0): vorn bei z=0, REIN ROT — wird ZUERST gezeichnet.
        // Orb 1 (i=1): dahinter bei z=-2, REIN GRUEN — danach gezeichnet.
        // Ohne Depth-Test wuerde Gruen das Rot ueberschreiben.
        gp.pointCode = "x=0;y=0;z=-i*2;radius=0.5;"
                       "red=1-i;green=i;blue=0;red2=1-i;green2=i;blue2=0";
        orbs.params = gp;
        root.children.push_back(std::move(clear));
        root.children.push_back(std::move(orbs));

        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        ctx.functions()->glViewport(0, 0, kSize, kSize);
        vis.render(dt);
        const QImage img = vis.debugGrabRootSurface();
        REQUIRE(!img.isNull());

        const QColor center = img.pixelColor(cx, cy);
        CHECK(center.redF() > 0.5);    // vorderer roter Orb gewinnt
        CHECK(center.greenF() < 0.2);  // hinterer gruener Orb ist verdeckt
        vis.cleanup();
    }

    // --- Lights Demos v2 + v3: laden und rendern (validiert ALLE EEL-Slots —
    // Slot-Ausfaelle waeren still und das Bild bliebe leer/dunkel) ------------
    for (const char* file : {"lights_demo2.lvfx", "lights_demo3.lvfx"})
    {
        const std::filesystem::path preset =
            repoRoot() / "asset" / "effectchain" / file;
        REQUIRE(std::filesystem::exists(preset));

        MultiEffectVisualizer vis;
        QStringList report;
        REQUIRE(vis.loadChainFile(
            QString::fromStdWString(preset.wstring()), &report));
        for (const QString& line : report)
            CHECK_MESSAGE(!line.contains("passthrough"), line.toStdString());

        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        ctx.functions()->glViewport(0, 0, kSize, kSize);
        vis.setBeatPeriodOverride(30);  // deterministische Beats (Orb-Flash)
        for (int frame = 0; frame < 6; ++frame) vis.render(dt);
        const QImage img = vis.debugGrabRootSurface();
        REQUIRE(!img.isNull());
        // Szene lebt: Terrain-Dots/Orbs/Funken leuchten, Ecken nicht weiss.
        CHECK_MESSAGE(lumaSum(img) > 5.0, file);
        CHECK_MESSAGE(luma(img, 2, 2) < 0.5, file);
        vis.cleanup();
    }

    ctx.doneCurrent();
}
