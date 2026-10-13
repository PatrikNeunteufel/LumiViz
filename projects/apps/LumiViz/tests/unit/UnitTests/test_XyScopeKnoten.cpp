/**
 * @file test_XyScopeKnoten.cpp
 * @brief Klang aus einer Form, Stufe 19d: der Knoten XY Scope der Effect Chain —
 *        links gegen rechts, ein Punkt je Abtastwert
 *        (docs/audio/Shape_Konzept.md §10; Entscheide Patrik M1 a, S120, und
 *        V4 a, X1 a, S121)
 *
 * Der Knoten in der Datei und in der Feldtabelle, dann mit einem GL-Kontext
 * ohne Fenster im Bild: der Test gibt dem Visualizer gerechnete Stereo-Bloecke
 * (ein Kreis aus Sinus und Cosinus, ein Mono-Ton, zwei Punkte im Wechsel).
 * `LUMIVIZ_XY_SCOPE_BILD=<datei.png>` legt ein Blatt mit sechs Stellungen ab.
 */

#include <doctest.h>

#include "scripting/ScriptSlotHost.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/FieldBindings.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QStringList>
#include <QSurfaceFormat>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

using namespace lumi::multieffect;

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kBild = 128;
constexpr int kWerte = 512;

/// Ein GL-Kontext ohne Fenster; `ok` = false, wo es keinen gibt (dann meldet der Fall es).
struct GlOhneFenster
{
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool ok = false;
    GlOhneFenster()
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        if (!ctx.create()) return;
        surface.setFormat(ctx.format());
        surface.create();
        ok = surface.isValid() && ctx.makeCurrent(&surface);
    }
};

double hell(const QImage& bild, int x, int y)
{
    const QColor c = bild.pixelColor(x, y);
    return std::max({c.redF(), c.greenF(), c.blueF()});
}

int leuchtend(const QImage& bild, int x0, int y0, int x1, int y1)
{
    int zahl = 0;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
            if (hell(bild, x, y) > 0.2) ++zahl;
    return zahl;
}
int leuchtend(const QImage& bild) { return leuchtend(bild, 0, 0, bild.width(), bild.height()); }
bool leuchtetBei(const QImage& bild, int x, int y) { return leuchtend(bild, x - 2, y - 2, x + 3, y + 3) > 0; }

/// Die Kette Clear -> XY Scope.
ChainNode kette(const XyScopeParams& scope)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode knoten;
    knoten.params = scope;
    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(knoten));
    return root;
}

/// Ein Stereo-Block, verschraenkt (links, rechts, links, …).
using Block = std::vector<float>;

/// Links Cosinus, rechts Sinus: ein Kreis mit dem Radius `ausschlag`.
Block kreis(double ausschlag = 1.0)
{
    Block b(2 * kWerte);
    for (int n = 0; n < kWerte; ++n)
    {
        const double a = 2.0 * kPi * n / (kWerte - 1);
        b[static_cast<std::size_t>(2 * n)] = static_cast<float>(ausschlag * std::cos(a));
        b[static_cast<std::size_t>(2 * n + 1)] = static_cast<float>(ausschlag * std::sin(a));
    }
    return b;
}

/// Beide Seiten gleich, von −1 bis 1: ein Mono-Ton.
Block mono()
{
    Block b(2 * kWerte);
    for (int n = 0; n < kWerte; ++n)
    {
        const auto w = static_cast<float>(-1.0 + 2.0 * n / (kWerte - 1));
        b[static_cast<std::size_t>(2 * n)] = w;
        b[static_cast<std::size_t>(2 * n + 1)] = w;
    }
    return b;
}

/// Links abwechselnd −1 und 1, rechts still: zwei Punkte, zwischen denen die Linie hin und her faehrt.
Block zweiPunkte()
{
    Block b(2 * kWerte, 0.0f);
    for (int n = 0; n < kWerte; ++n) b[static_cast<std::size_t>(2 * n)] = n % 2 == 0 ? -1.0f : 1.0f;
    return b;
}

}  // namespace

TEST_CASE("XY Scope Knoten: die Datei — Vorgaben, Hin und Zurueck, Grenzen, die Feldtabelle")
{
    CHECK(effectTypeKey(EffectParams{XyScopeParams{}}) == "xyScope");
    CHECK(std::string(effectTypeName(EffectParams{XyScopeParams{}})) == "XY Scope");

    ChainNode root;
    root.params = ListParams{};
    ChainNode blatt;
    XyScopeParams sp;
    sp.gain = 2.5f;
    sp.drawMode = 1;
    sp.lineWidth = 3.5f;
    sp.rotate45 = true;
    sp.xShift = -20.0f;
    sp.yShift = 15.0f;
    sp.size = 120.0f;
    sp.colorSource = 1;
    sp.colors = {0xFF0000, 0x00FF00};
    sp.gradientPreset = "Fire";
    sp.lineBlend = 3;
    sp.initCode = "t=0";
    sp.frameCode = "t=t+1";
    sp.beatCode = "gain=4";
    sp.pointCode = "x=x*2";
    blatt.params = sp;
    root.children.push_back(blatt);
    const ChainNode zurueck = chainFromJson(chainToJson(root), nullptr);
    REQUIRE(zurueck.children.size() == 1);
    const auto& p = std::get<XyScopeParams>(zurueck.children[0].params);
    CHECK(p.gain == doctest::Approx(2.5f));
    CHECK(p.drawMode == 1);
    CHECK(p.lineWidth == doctest::Approx(3.5f));
    CHECK(p.rotate45);
    CHECK(p.xShift == doctest::Approx(-20.0f));
    CHECK(p.yShift == doctest::Approx(15.0f));
    CHECK(p.size == doctest::Approx(120.0f));
    CHECK(p.colorSource == 1);
    CHECK(p.colors == std::vector<uint32_t>{0xFF0000, 0x00FF00});
    CHECK(p.gradientPreset == "Fire");
    CHECK(p.lineBlend == 3);
    CHECK(p.initCode == "t=0");
    CHECK(p.frameCode == "t=t+1");
    CHECK(p.beatCode == "gain=4");
    CHECK(p.pointCode == "x=x*2");

    // --- was ausserhalb liegt, kommt in seine Grenzen; was fehlt, ist die Vorgabe
    {
        QJsonObject datei = chainToJson(root);
        QJsonObject wurzel = datei.value(QStringLiteral("root")).toObject();
        QJsonArray kinder = wurzel.value(QStringLiteral("children")).toArray();
        REQUIRE(kinder.size() == 1);
        QJsonObject knoten = kinder[0].toObject();
        knoten[QStringLiteral("gain")] = 9999;
        knoten[QStringLiteral("drawMode")] = 7;
        knoten[QStringLiteral("size")] = -5;
        knoten[QStringLiteral("lineBlend")] = 42;
        knoten.remove(QStringLiteral("colors"));
        knoten.remove(QStringLiteral("gradientPreset"));
        kinder[0] = knoten;
        wurzel[QStringLiteral("children")] = kinder;
        datei[QStringLiteral("root")] = wurzel;
        const ChainNode gelesen = chainFromJson(datei, nullptr);
        REQUIRE(gelesen.children.size() == 1);
        const auto& q = std::get<XyScopeParams>(gelesen.children[0].params);
        CHECK(q.gain == doctest::Approx(16.0f));
        CHECK(q.drawMode == 1);
        CHECK(q.size == doctest::Approx(1.0f));
        CHECK(q.lineBlend == 9);
        CHECK(q.colors == XyScopeParams{}.colors);
        CHECK(q.gradientPreset == XyScopeParams{}.gradientPreset);
    }

    // --- die Feldtabelle: eigener Skript-Host, jeder Regler mit Zahl ist Skriptvariable
    const feldtabelle::Typ* typ = feldtabelle::typ(EffectParams{XyScopeParams{}});
    REQUIRE(typ != nullptr);
    CHECK(typ->traeger == feldtabelle::Traeger::EigenerHost);
    for (const char* name : {"gain", "drawmode", "linewidth", "rotate45", "xshift", "yshift", "size", "colorsource", "lineblend"})
    {
        CAPTURE(name);
        const bool da = std::any_of(typ->felder.begin(), typ->felder.end(),
                                    [name](const feldtabelle::Feld& f) { return f.name == name; });
        CHECK(da);
    }
    for (const feldtabelle::Feld& f : typ->felder) CHECK(f.klasse == feldtabelle::Klasse::Live);
}

TEST_CASE("XY Scope Knoten im Bild: links gegen rechts — Gain, Rotate 45, Draw, Farbe, Lage, die Skriptfelder")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);
    GlOhneFenster gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
        return;
    }

    MultiEffectVisualizer vis;
    vis.setChain(kette(XyScopeParams{}));
    vis.initialize();
    vis.resize(QSize(kBild, kBild));
    gl.ctx.functions()->glViewport(0, 0, kBild, kBild);
    const float dt = 1.0f / 60.0f;
    const std::vector<float> spektrum(2 * kWerte, 0.0f);
    const auto bildVon = [&](const XyScopeParams& p, const Block& block, bool skripteAn = true, int bilder = 1) {
        ChainNode k = kette(p);
        k.children[1].scriptsActive = skripteAn;
        vis.setChain(std::move(k));
        vis.updateAudioStereo(spektrum.data(), kWerte, block.data(), kWerte, 2);
        for (int n = 0; n < bilder; ++n) vis.render(dt);
        return vis.debugGrabRootSurface();
    };

    // Vollausschlag fuellt mit Size 50 die halbe Seite: der Kreis hat den Radius 32 um die Mitte.
    XyScopeParams grund;
    grund.size = 50.0f;
    grund.colors = {0xFF0000};
    const QImage bild = bildVon(grund, kreis());
    REQUIRE(!bild.isNull());
    REQUIRE(bild.width() == kBild);
    CHECK(leuchtetBei(bild, 96, 64));
    CHECK(leuchtetBei(bild, 32, 64));
    CHECK(leuchtetBei(bild, 64, 32));
    CHECK(leuchtetBei(bild, 64, 96));
    CHECK_FALSE(leuchtetBei(bild, 64, 64));
    CHECK_FALSE(leuchtetBei(bild, 96, 96));  // ein Kreis, kein Quadrat
    {
        // die Farbe aus der Tafel: rot
        double rot = 0.0, blau = 0.0;
        for (int x = 92; x <= 100; ++x)
        {
            rot = std::max(rot, static_cast<double>(bild.pixelColor(x, 64).redF()));
            blau = std::max(blau, static_cast<double>(bild.pixelColor(x, 64).blueF()));
        }
        CHECK(rot > 0.8);
        CHECK(blau < 0.1);
    }

    // --- rechts zeigt nach oben: ein Punkt mit links 0 und rechts 1 liegt ueber der Mitte
    {
        Block oben(2 * kWerte, 0.0f);
        for (int n = 0; n < kWerte; ++n) oben[static_cast<std::size_t>(2 * n + 1)] = n % 2 == 0 ? 0.0f : 1.0f;
        const QImage strich = bildVon(grund, oben);
        CHECK(leuchtetBei(strich, 64, 40));
        CHECK_FALSE(leuchtetBei(strich, 64, 88));
    }
    // --- Gain: der halbe Ausschlag
    {
        XyScopeParams p = grund;
        p.gain = 0.5f;
        const QImage klein = bildVon(p, kreis());
        CHECK(leuchtetBei(klein, 80, 64));
        CHECK_FALSE(leuchtetBei(klein, 96, 64));
        // und ein leiser Ton mit Gain 2 fuellt wieder
        p.gain = 2.0f;
        CHECK(leuchtetBei(bildVon(p, kreis(0.5)), 96, 64));
    }
    // --- Mono: eine Schraege von links unten nach rechts oben; mit Rotate 45 steht sie senkrecht
    {
        const QImage schraeg = bildVon(grund, mono());
        CHECK(leuchtetBei(schraeg, 80, 48));
        CHECK(leuchtetBei(schraeg, 48, 80));
        CHECK_FALSE(leuchtetBei(schraeg, 80, 80));
        XyScopeParams p = grund;
        p.rotate45 = true;
        const QImage senkrecht = bildVon(p, mono());
        CHECK(leuchtetBei(senkrecht, 64, 32));
        CHECK(leuchtetBei(senkrecht, 64, 96));
        CHECK_FALSE(leuchtetBei(senkrecht, 80, 48));
        CHECK(leuchtend(senkrecht, 0, 0, 58, kBild) == 0);
    }
    // --- Draw: Lines verbindet die Abtastwerte, Dots setzt je Wert einen Punkt
    {
        const QImage linien = bildVon(grund, zweiPunkte());
        CHECK(leuchtetBei(linien, 64, 64));
        XyScopeParams p = grund;
        p.drawMode = 1;
        p.lineWidth = 4.0f;
        const QImage punkte = bildVon(p, zweiPunkte());
        CHECK_FALSE(leuchtetBei(punkte, 64, 64));
        CHECK(leuchtetBei(punkte, 32, 64));
        CHECK(leuchtetBei(punkte, 96, 64));
    }
    // --- X shift, Y shift, Size
    {
        XyScopeParams p = grund;
        p.xShift = 10.0f;
        const QImage rechts = bildVon(p, kreis());
        CHECK(leuchtetBei(rechts, 109, 64));
        CHECK_FALSE(leuchtetBei(rechts, 96, 64));
        p = grund;
        p.yShift = -20.0f;
        const QImage oben = bildVon(p, kreis());
        CHECK(leuchtend(oben, 0, 0, kBild, 40) > leuchtend(oben, 0, 88, kBild, kBild));
        p = grund;
        p.size = 25.0f;
        CHECK(leuchtetBei(bildVon(p, kreis()), 80, 64));
    }
    // --- Color from: die Tafel gibt eine Farbe fuer die ganze Linie, der Verlauf laeuft an ihr entlang
    const auto farbtoene = [](const QImage& b) {
        // der groesste Abstand zwischen zwei leuchtenden Pixeln, im Farbton gemessen
        double kleinster = 2.0, groesster = -1.0;
        for (int y = 0; y < b.height(); ++y)
            for (int x = 0; x < b.width(); ++x)
            {
                const QColor c = b.pixelColor(x, y);
                if (std::max({c.redF(), c.greenF(), c.blueF()}) < 0.5) continue;
                const double anteil = c.redF() / (c.redF() + c.greenF() + c.blueF());
                kleinster = std::min(kleinster, anteil);
                groesster = std::max(groesster, anteil);
            }
        return groesster - kleinster;
    };
    {
        CHECK(farbtoene(bild) < 0.05);
        XyScopeParams p = grund;
        p.colorSource = 1;
        p.gradientPreset = "Neon";
        p.lineBlend = 0;  // ersetzen: Anfang und Ende des Kreises liegen uebereinander
        CHECK(farbtoene(bildVon(p, kreis())) > 0.2);
    }

    // --- Frame, Beat: ein Skript setzt einen Regler; Script active aus: der Regler gilt
    {
        XyScopeParams p = grund;
        p.frameCode = "gain=0.5";
        const QImage klein = bildVon(p, kreis());
        CHECK(leuchtetBei(klein, 80, 64));
        CHECK_FALSE(leuchtetBei(klein, 96, 64));
        CHECK(leuchtetBei(bildVon(p, kreis(), /*skripteAn=*/false), 96, 64));
        p = grund;
        p.beatCode = "rotate45=1";
        vis.setBeatPeriodOverride(1);
        const QImage gedreht = bildVon(p, mono());
        vis.setBeatPeriodOverride(0);
        CHECK(leuchtetBei(gedreht, 64, 32));
    }
    // --- Point: x ist links, y ist rechts; Farbe und skip je Punkt
    {
        XyScopeParams p = grund;
        p.pointCode = "x=0";
        const QImage senkrecht = bildVon(p, kreis());
        CHECK(leuchtetBei(senkrecht, 64, 32));
        CHECK_FALSE(leuchtetBei(senkrecht, 96, 64));
        p.pointCode = "y=y*0.5";
        const QImage flach = bildVon(p, kreis());
        CHECK(leuchtetBei(flach, 96, 64));
        CHECK(leuchtetBei(flach, 64, 48));
        CHECK_FALSE(leuchtetBei(flach, 64, 32));
        p.pointCode = "skip=1";
        CHECK(leuchtend(bildVon(p, kreis())) == 0);
        p.pointCode = "skip=above(i,0.5)";
        const QImage halb = bildVon(p, kreis());
        CHECK(leuchtetBei(halb, 64, 32));        // die erste Haelfte: der obere Bogen (rechts positiv = oben)
        CHECK_FALSE(leuchtetBei(halb, 64, 96));
        p.pointCode = "red=0; green=1; blue=0;";
        const QImage gruen = bildVon(p, kreis());
        double g = 0.0, r = 0.0;
        for (int x = 92; x <= 100; ++x)
        {
            g = std::max(g, static_cast<double>(gruen.pixelColor(x, 64).greenF()));
            r = std::max(r, static_cast<double>(gruen.pixelColor(x, 64).redF()));
        }
        CHECK(g > 0.8);
        CHECK(r < 0.1);
    }
    // --- die Beispiele des Handbuchs (§11 XY Scope), woertlich
    {
        // der Beat blaest das Bild auf, danach faellt es zurueck
        XyScopeParams p = grund;
        p.beatCode = "gain = 2.5;";
        p.frameCode = "gain = gain + (1 - gain) * 0.12;";
        vis.setBeatPeriodOverride(1);
        CHECK(leuchtetBei(bildVon(p, kreis(0.4), true, 3), 96, 64));  // 0,4 mal 2,5: Vollausschlag
        vis.setBeatPeriodOverride(0);
        CHECK(leuchtetBei(bildVon(p, kreis(0.4), true, 120), 77, 64));  // ohne Beat zurueck auf 0,4
        // nur die linke Seite, ueber die Hoehe verteilt: ein Oszilloskop aus dem Scope
        XyScopeParams q = grund;
        q.pointCode = "y = x; x = i * 2 - 1;";
        const QImage welle = bildVon(q, mono());
        CHECK(leuchtetBei(welle, 80, 48));  // die Rampe des Mono-Tons laeuft von links unten nach rechts oben
        CHECK(leuchtetBei(welle, 34, 94));
    }
    // --- Stille: nichts stuerzt, hoechstens ein Punkt in der Mitte
    {
        const Block still(2 * kWerte, 0.0f);
        CHECK(leuchtend(bildVon(grund, still)) < 30);
    }

    // --- die sechs mitgelieferten Presets (Entscheid X2 a, Patrik S121): Goniometer, Lissajous,
    //     Beat Zoom, Slow Spin, Rainbow Trace, Corner Meter — laden, uebersetzen, zeichnen;
    //     wer eines dazulegt, stellt die Zahl nach
    std::vector<std::pair<QString, XyScopeParams>> presets;
    {
        const auto eintraege = nodepresets::list(QStringLiteral("xyScope"));
        REQUIRE(eintraege.size() == 6);
        vis.setBeatPeriodOverride(12);
        for (const nodepresets::Entry& e : eintraege)
        {
            CAPTURE(e.name.toStdString());
            EffectParams params{XyScopeParams{}};
            QStringList bericht;
            REQUIRE(nodepresets::load(e.path, QStringLiteral("xyScope"), params, &bericht));
            const XyScopeParams& p = std::get<XyScopeParams>(params);
            lumi::scripting::ScriptSlotHost host("xyscope", {}, lumi::scripting::ScriptSlotHost::Dialect::Avs);
            using Slot = lumi::scripting::ScriptSlotHost::Slot;
            host.setSource(Slot::Init, p.initCode);
            host.setSource(Slot::Frame, p.frameCode);
            host.setSource(Slot::Beat, p.beatCode);
            host.setSource(Slot::Point, p.pointCode);
            CHECK_MESSAGE(host.compileAll(), host.lastError());
            CHECK(leuchtend(bildVon(p, kreis(0.8), true, 30)) > 20);
            presets.emplace_back(e.name, p);
        }
        vis.setBeatPeriodOverride(0);
    }

    // --- ein Blatt: sechs Stellungen, darunter die sechs Presets
    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_XY_SCOPE_BILD"); !ziel.isEmpty())
    {
        // ein Ton mit zwei Frequenzen: links drei Schwingungen, rechts zwei
        Block figur(2 * kWerte);
        for (int n = 0; n < kWerte; ++n)
        {
            const double a = 2.0 * kPi * n / (kWerte - 1);
            figur[static_cast<std::size_t>(2 * n)] = static_cast<float>(0.9 * std::sin(3.0 * a));
            figur[static_cast<std::size_t>(2 * n + 1)] = static_cast<float>(0.9 * std::sin(2.0 * a + 0.5));
        }
        constexpr int kKachel = 256;
        vis.resize(QSize(kKachel, kKachel));
        gl.ctx.functions()->glViewport(0, 0, kKachel, kKachel);
        QImage blatt(3 * kKachel, 4 * kKachel, QImage::Format_RGB32);
        blatt.fill(Qt::black);
        QPainter maler(&blatt);
        maler.setPen(Qt::white);
        int platz = 0;
        const auto kachel = [&](const QString& name, const XyScopeParams& p, const Block& block, int bilder = 1) {
            const QImage eines = bildVon(p, block, true, bilder);
            const QPoint ecke((platz % 3) * kKachel, (platz / 3) * kKachel);
            maler.drawImage(ecke, eines);
            maler.drawText(ecke + QPoint(6, 14), name);
            ++platz;
        };
        XyScopeParams p;
        kachel("Lines, table (defaults)", p, figur);
        p.colorSource = 1;
        kachel("Lines, gradient Neon", p, figur);
        p.drawMode = 1;
        p.lineWidth = 3.0f;
        kachel("Dots, gradient", p, figur);
        p = XyScopeParams{};
        p.rotate45 = true;
        kachel("Mono, Rotate 45", p, mono());
        p = XyScopeParams{};
        p.pointCode = "a=6.2832*i; red=0.5+0.5*sin(a); green=0.5+0.5*sin(a+2.094); blue=0.5+0.5*sin(a+4.189);";
        kachel("Point: colour by i", p, kreis(0.9));
        p = XyScopeParams{};
        p.size = 30.0f;
        p.xShift = 33.0f;
        p.yShift = -33.0f;
        p.rotate45 = true;
        p.drawMode = 1;
        kachel("Small, in the corner", p, figur);
        vis.setBeatPeriodOverride(12);
        for (const auto& [name, preset] : presets) kachel(QStringLiteral("Preset: ") + name, preset, figur, 40);
        vis.setBeatPeriodOverride(0);
        maler.end();
        CHECK(blatt.save(ziel));
        MESSAGE("Blatt: " << ziel.toStdString());
    }

    vis.cleanup();
    gl.ctx.doneCurrent();
}
