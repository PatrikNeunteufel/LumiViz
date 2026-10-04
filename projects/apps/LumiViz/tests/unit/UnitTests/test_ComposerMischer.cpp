/**
 ****************************************************************************************
 * @file   test_ComposerMischer.cpp
 * @brief  Composer, Stufe C1a: der Mischer in kleinster Fassung und die
 *         Composer-Naehte R1 / R2 am Renderer (docs/ui/Composer_Konzept.md §5, §12)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Zwei Dinge werden belegt:
 *  - Die Naehte aendern nichts, solange niemand sie benutzt („Composer aus"),
 *    und tun, was der Mischer braucht, wenn er sie benutzt.
 *  - Der Mischer rechnet je Mischart und Pegel, was die Formel sagt.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Mischer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QColor>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <algorithm>
#include <cmath>
#include <memory>

using namespace lumi::multieffect;
using lumi::composer::Leinwand;
using lumi::composer::Mischart;
using lumi::composer::Mischer;

namespace
{

struct Gl
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool gut = false;

    Gl()
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        if (!ctx.create()) return;
        surface.setFormat(ctx.format());
        surface.create();
        gut = surface.isValid() && ctx.makeCurrent(&surface);
    }
};

#define GL_ODER_ENDE(gl)                                                          \
    if (!(gl).gut)                                                                \
    {                                                                             \
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");          \
        return;                                                                   \
    }

ChainNode farbKette(uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    root.children.push_back(std::move(clear));
    return root;
}

/// Kette mit eigenem Zustand: wandernde Balken (+ Feedback).
ChainNode balkenKette(uint32_t farbe, bool mitFeedback)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.onlyFirst = mitFeedback;
    clear.params = cp;
    ChainNode balken;
    balken.params = DebugBarsParams{farbe, -2.5f};
    root.children = {clear, balken};
    if (mitFeedback)
    {
        ChainNode blit;
        blit.params = BlitterFeedbackParams{};
        root.children.push_back(std::move(blit));
    }
    return root;
}

void bereite(MultiEffectVisualizer& vis, QSize groesse)
{
    vis.setBeatPeriodOverride(30);
    vis.initialize();
    vis.resize(groesse);
}

int verschieden(const QImage& a, const QImage& b, int toleranz = 0)
{
    if (a.size() != b.size()) return -1;
    int n = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
        {
            const QRgb p = a.pixel(x, y), q = b.pixel(x, y);
            if (std::abs(qRed(p) - qRed(q)) > toleranz || std::abs(qGreen(p) - qGreen(q)) > toleranz ||
                std::abs(qBlue(p) - qBlue(q)) > toleranz)
                ++n;
        }
    return n;
}

// Das Fenster selbst (Framebuffer 0) laesst sich im Test nicht verlaesslich
// lesen: der Kontext haengt an einer unsichtbaren Flaeche ohne feste
// Groesse. Dass der Present mit der Vorgabe unveraendert aufs Fenster geht,
// belegen die bestehenden Tests (unveraendert gruen) und der Vergleichslauf.

/// Die Formel des Mischers, auf der CPU: 0..255 je Kanal.
int erwartet(int unten, int oben, Mischart art, double pegel)
{
    const double d = unten / 255.0, s = oben / 255.0;
    double r = s;
    switch (art)
    {
        case Mischart::Normal: r = s; break;
        case Mischart::Additive: r = std::min(d + s, 1.0); break;
        case Mischart::Subtract: r = std::max(d - s, 0.0); break;
        case Mischart::Maximum: r = std::max(d, s); break;
        case Mischart::Minimum: r = std::min(d, s); break;
        case Mischart::Multiply: r = d * s; break;
        case Mischart::Xor: r = (unten ^ oben) / 255.0; break;
    }
    return static_cast<int>(std::lround((d + (r - d) * pegel) * 255.0));
}

QStringList mitgelieferte(const QString& unterordner, const QString& muster, int hoechstens)
{
    const QString ordner = QDir::cleanPath(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() +
                                           QStringLiteral("/../../../../../../asset/presets/") +
                                           unterordner);
    QStringList pfade;
    QDirIterator it(ordner, {muster}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) pfade.append(it.next());
    pfade.sort();
    while (pfade.size() > hoechstens) pfade.removeLast();
    return pfade;
}

}  // namespace

TEST_SUITE("ComposerMischer")
{
    // -------------------------------------------------------------------------
    // Naht R1 — Present-Ziel
    // -------------------------------------------------------------------------
    TEST_CASE("C1a GL Naht R1: Vorgabe ist das Fenster; mit Present-Ziel landet das fertige Bild in der Leinwand und nur dort")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        const QSize groesse(96, 64);
        auto* f = QOpenGLContext::currentContext()->functions();

        // --- Composer aus: Vorgabe 0, der Present landet im Fenster
        MultiEffectVisualizer vis;
        CHECK(vis.presentZiel() == 0);
        CHECK_FALSE(vis.karaokeAuslassen());
        vis.setChain(balkenKette(0x40C0FF, true));
        bereite(vis, groesse);
        f->glViewport(0, 0, groesse.width(), groesse.height());
        for (int i = 0; i < 5; ++i) vis.render(1.0f / 60.0f);
        const QImage root = vis.debugGrabRootSurface().convertToFormat(QImage::Format_ARGB32);
        REQUIRE(!root.isNull());
        CHECK(vis.presentZiel() == 0);  // nach dem Rendern unveraendert

        // --- derselbe Renderer in eine Leinwand: gleiches Bild wie der
        //     Present
        MultiEffectVisualizer zweiter;
        zweiter.setChain(balkenKette(0x40C0FF, true));
        bereite(zweiter, groesse);
        Leinwand leinwand;
        for (int i = 0; i < 5; ++i) leinwand.rendere(zweiter, 1.0f / 60.0f, groesse);
        CHECK(zweiter.presentZiel() == 0);  // danach wieder das Fenster
        CHECK(leinwand.groesse() == groesse);
        CHECK(verschieden(leinwand.bild().convertToFormat(QImage::Format_ARGB32), root) == 0);

        // --- ein zweites Zielbild bleibt unberuehrt: der Present geht nur
        //     dorthin, wohin er geschickt wird
        QOpenGLFramebufferObject marke(groesse);
        marke.bind();
        f->glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        marke.release();
        for (int i = 0; i < 3; ++i) leinwand.rendere(zweiter, 1.0f / 60.0f, groesse);
        const QColor m = marke.toImage().pixelColor(10, 10);
        CHECK(m.red() == 255);
        CHECK(m.green() == 0);
        CHECK(m.blue() == 255);
    }

    TEST_CASE("C1a GL Naht R1: die Leinwand traegt das Bild in Fenstergroesse — auch mit Render Scale")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        const QSize groesse(128, 128);
        ChainNode kette = balkenKette(0xFF8020, false);
        ChainNode scale;
        RenderScaleParams rs;
        rs.divisor = 2;
        scale.params = rs;
        kette.children.insert(kette.children.begin(), scale);
        MultiEffectVisualizer vis;
        vis.setChain(std::move(kette));
        bereite(vis, groesse);
        Leinwand leinwand;
        for (int i = 0; i < 3; ++i) leinwand.rendere(vis, 1.0f / 60.0f, groesse);
        // die Kette rechnet in halber Groesse, die Leinwand hat das Fenstermass
        CHECK(vis.debugGrabRootSurface().size() == QSize(64, 64));
        CHECK(leinwand.bild().size() == groesse);
    }

    // -------------------------------------------------------------------------
    // Der Mischer
    // -------------------------------------------------------------------------
    TEST_CASE("C1a GL: der Mischer rechnet jede Mischart mit jedem Pegel nach der Formel")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        const QSize groesse(32, 32);
        // unten: 0xC06020 (192, 96, 32) · oben: 0x5080E0 (80, 128, 224)
        const int unten[3] = {192, 96, 32};
        const int oben[3] = {80, 128, 224};
        MultiEffectVisualizer va, vb;
        va.setChain(farbKette(0xC06020));
        vb.setChain(farbKette(0x5080E0));
        bereite(va, groesse);
        bereite(vb, groesse);
        Leinwand la, lb;
        la.rendere(va, 1.0f / 60.0f, groesse);
        lb.rendere(vb, 1.0f / 60.0f, groesse);

        Mischer mischer;
        for (int art = 0; art < lumi::composer::kMischarten; ++art)
            for (double pegel : {0.25, 0.5, 1.0})
            {
                REQUIRE(mischer.beginne(groesse));
                mischer.lege(la, 1.0, Mischart::Normal);
                mischer.lege(lb, pegel, static_cast<Mischart>(art));
                const QColor c = mischer.bild().pixelColor(16, 16);
                CAPTURE(lumi::composer::mischartName(static_cast<Mischart>(art)));
                CAPTURE(pegel);
                CHECK(std::abs(c.red() - erwartet(unten[0], oben[0], static_cast<Mischart>(art), pegel)) <= 1);
                CHECK(std::abs(c.green() - erwartet(unten[1], oben[1], static_cast<Mischart>(art), pegel)) <= 1);
                CHECK(std::abs(c.blue() - erwartet(unten[2], oben[2], static_cast<Mischart>(art), pegel)) <= 1);
            }

        // --- Grund ist Schwarz; Pegel 0 legt nichts; die Reihenfolge zaehlt
        REQUIRE(mischer.beginne(groesse));
        CHECK(mischer.bild().pixelColor(16, 16) == QColor(0, 0, 0));
        mischer.lege(la, 0.0, Mischart::Normal);
        CHECK(mischer.bild().pixelColor(16, 16) == QColor(0, 0, 0));
        mischer.lege(la, 0.5, Mischart::Normal);  // halber Pegel ueber Schwarz
        CHECK(std::abs(mischer.bild().pixelColor(16, 16).red() - 96) <= 1);
        REQUIRE(mischer.beginne(groesse));
        mischer.lege(lb, 1.0, Mischart::Normal);
        mischer.lege(la, 1.0, Mischart::Normal);  // das Obere gewinnt
        CHECK(std::abs(mischer.bild().pixelColor(16, 16).red() - 192) <= 1);

        // --- das Gemischte 1:1 in ein Zielbild (im Betrieb: das Fenster)
        QOpenGLFramebufferObject ziel(groesse);
        mischer.zeichneIn(ziel.handle(), groesse);
        CHECK(verschieden(ziel.toImage().convertToFormat(QImage::Format_ARGB32),
                          mischer.bild().convertToFormat(QImage::Format_ARGB32)) == 0);
    }

    TEST_CASE("C1a GL: zwei Renderer mit Zustand, halb gemischt — Bild fuer Bild das Mittel beider Bilder allein")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        const QSize groesse(160, 120);
        const ChainNode ka = balkenKette(0x40C0FF, true);
        const ChainNode kb = balkenKette(0xFF8020, false);
        const auto allein = [&](const ChainNode& kette) {
            MultiEffectVisualizer vis;
            vis.setChain(kette);
            bereite(vis, groesse);
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, groesse.width(), groesse.height());
            for (int i = 0; i < 30; ++i) vis.render(1.0f / 60.0f);
            return vis.debugGrabRootSurface().convertToFormat(QImage::Format_ARGB32);
        };
        const QImage a = allein(ka);
        const QImage b = allein(kb);

        MultiEffectVisualizer va, vb;
        va.setChain(ka);
        vb.setChain(kb);
        bereite(va, groesse);
        bereite(vb, groesse);
        Leinwand la, lb;
        Mischer mischer;
        for (int i = 0; i < 30; ++i)
        {
            la.rendere(va, 1.0f / 60.0f, groesse);
            lb.rendere(vb, 1.0f / 60.0f, groesse);
            REQUIRE(mischer.beginne(groesse));
            mischer.lege(la, 1.0, Mischart::Normal);
            mischer.lege(lb, 0.5, Mischart::Normal);
        }
        // jeder Renderer fuer sich: bitgleich mit dem Lauf allein
        CHECK(verschieden(la.bild().convertToFormat(QImage::Format_ARGB32), a) == 0);
        CHECK(verschieden(lb.bild().convertToFormat(QImage::Format_ARGB32), b) == 0);
        // das Gemischte: das Mittel beider
        QImage mittel(groesse, QImage::Format_ARGB32);
        for (int y = 0; y < groesse.height(); ++y)
            for (int x = 0; x < groesse.width(); ++x)
            {
                const QRgb p = a.pixel(x, y), q = b.pixel(x, y);
                mittel.setPixel(x, y, qRgb(erwartet(qRed(p), qRed(q), Mischart::Normal, 0.5),
                                           erwartet(qGreen(p), qGreen(q), Mischart::Normal, 0.5),
                                           erwartet(qBlue(p), qBlue(q), Mischart::Normal, 0.5)));
            }
        CHECK(verschieden(mischer.bild().convertToFormat(QImage::Format_ARGB32), mittel, 1) == 0);
    }

    // -------------------------------------------------------------------------
    // Naht R2 — Karaoke-Ebene
    // -------------------------------------------------------------------------
    TEST_CASE("C1a GL Naht R2: die Clips lassen die Karaoke-Ebene aus, der Mischer legt sie einmal darueber")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        namespace ly = lumi::multieffect::lyrics;
        const QSize groesse(320, 240);
        MultiEffectVisualizer::notifyPlayState(false);
        MultiEffectVisualizer::setSongtextTitelVersatz(0);
        MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
        MultiEffectVisualizer::setKaraokeEbene(true);
        MultiEffectVisualizer::notifyPlayPosition(11000);

        // --- Vorgabe: ein Renderer allein zeichnet die Ebene in seinen Present
        MultiEffectVisualizer allein;
        allein.setChain(farbKette(0x203040));
        bereite(allein, groesse);
        Leinwand lAllein;
        for (int i = 0; i < 5; ++i) lAllein.rendere(allein, 1.0f / 60.0f, groesse);
        REQUIRE(allein.debugKaraokeGezeichnet());
        const int textAllein = verschieden(lAllein.bild().convertToFormat(QImage::Format_ARGB32),
                                           allein.debugGrabRootSurface().convertToFormat(QImage::Format_ARGB32));
        REQUIRE(textAllein > 0);

        // --- zwei Clips lassen sie aus: kein Text in ihren Bildern
        MultiEffectVisualizer va, vb, traeger;
        va.setChain(farbKette(0x203040));
        vb.setChain(farbKette(0x000000));
        traeger.setChain(farbKette(0x000000));  // rechnet nur die Ebene
        va.setKaraokeAuslassen(true);
        vb.setKaraokeAuslassen(true);
        bereite(va, groesse);
        bereite(vb, groesse);
        bereite(traeger, groesse);
        Leinwand la, lb, lTraeger;
        Mischer mischer;
        for (int i = 0; i < 5; ++i)
        {
            la.rendere(va, 1.0f / 60.0f, groesse);
            lb.rendere(vb, 1.0f / 60.0f, groesse);
            lTraeger.rendere(traeger, 1.0f / 60.0f, groesse);
            REQUIRE(mischer.beginne(groesse));
            mischer.lege(la, 1.0, Mischart::Normal);
            mischer.lege(lb, 0.0, Mischart::Normal);
        }
        CHECK_FALSE(va.debugKaraokeGezeichnet());
        CHECK_FALSE(vb.debugKaraokeGezeichnet());
        CHECK(verschieden(la.bild().convertToFormat(QImage::Format_ARGB32),
                          va.debugGrabRootSurface().convertToFormat(QImage::Format_ARGB32)) == 0);

        // --- der Mischer legt die Ebene des Traegers EINMAL ueber das Gemischte:
        //     dasselbe Bild wie der Renderer allein
        const QImage ohneText = mischer.bild().convertToFormat(QImage::Format_ARGB32);
        mischer.binde();
        traeger.legeKaraokeEbeneAuf();
        const QImage mitText = mischer.bild().convertToFormat(QImage::Format_ARGB32);
        const int textGemischt = verschieden(mitText, ohneText);
        MESSAGE("Karaoke: allein " << textAllein << " Pixel, ueber dem Gemischten " << textGemischt);
        CHECK(textGemischt == textAllein);
        CHECK(verschieden(mitText, lAllein.bild().convertToFormat(QImage::Format_ARGB32)) == 0);

        MultiEffectVisualizer::setKaraokeEbene(false);
        MultiEffectVisualizer::setSongtext(ly::Songtext{});
        MultiEffectVisualizer::notifyPlayPosition(0);
    }

    // -------------------------------------------------------------------------
    // Offene Punkte aus dem Konzept §5
    // -------------------------------------------------------------------------
    TEST_CASE("C1a GL: MilkDrop-Presets nebeneinander zeigen jedes dasselbe Bild wie allein")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        const QStringList presets = mitgelieferte(QStringLiteral("milkdrop"), QStringLiteral("*.milk"), 4);
        if (presets.size() < 2)
        {
            MESSAGE("asset/presets/milkdrop nicht gefunden — uebersprungen");
            return;
        }
        const QSize groesse(160, 120);
        constexpr int kFrames = 30;
        const auto allein = [&](const QString& pfad) {
            MultiEffectVisualizer vis;
            REQUIRE(vis.loadMilkFile(pfad));
            bereite(vis, groesse);
            Leinwand l;
            for (int i = 0; i < kFrames; ++i) l.rendere(vis, 1.0f / 60.0f, groesse);
            return l.bild();
        };
        for (int p = 0; p + 1 < presets.size(); p += 2)
        {
            const QImage a0 = allein(presets[p]);
            const QImage a1 = allein(presets[p]);
            const QImage b0 = allein(presets[p + 1]);
            MultiEffectVisualizer va, vb;
            REQUIRE(va.loadMilkFile(presets[p]));
            REQUIRE(vb.loadMilkFile(presets[p + 1]));
            bereite(va, groesse);
            bereite(vb, groesse);
            Leinwand la, lb;
            for (int i = 0; i < kFrames; ++i)
            {
                la.rendere(va, 1.0f / 60.0f, groesse);
                lb.rendere(vb, 1.0f / 60.0f, groesse);
            }
            const int da = verschieden(a0, la.bild());
            const int db = verschieden(b0, lb.bild());
            MESSAGE(QFileInfo(presets[p]).fileName().toStdString() << " + "
                    << QFileInfo(presets[p + 1]).fileName().toStdString() << ": allein zweimal "
                    << verschieden(a0, a1) << ", nebeneinander A " << da << ", B " << db << " von "
                    << groesse.width() * groesse.height() << " Pixeln");
            CAPTURE(presets[p].toStdString());
            // BELEG: auch MilkDrop (eigener Renderpfad, Shader, Sprites) — bitgleich
            CHECK(verschieden(a0, a1) == 0);
            CHECK(da == 0);
            CHECK(db == 0);
        }
    }

    TEST_CASE("C1a GL Messung: zwei Presets gemischt — Zeit je Bild gegen eines allein")
    {
        Gl gl;
        GL_ODER_ENDE(gl);
        QStringList presets = mitgelieferte(QStringLiteral("avs"), QStringLiteral("*.avs"), 2);
        if (presets.size() < 2)
        {
            MESSAGE("asset/presets/avs nicht gefunden — uebersprungen");
            return;
        }
        const QSize groesse(1280, 720);
        MultiEffectVisualizer va, vb;
        REQUIRE(va.loadAvsFile(presets[0]));
        REQUIRE(vb.loadAvsFile(presets[1]));
        bereite(va, groesse);
        bereite(vb, groesse);
        Leinwand la, lb;
        Mischer mischer;
        auto* f = QOpenGLContext::currentContext()->functions();
        for (int i = 0; i < 5; ++i)  // anlaufen lassen
        {
            la.rendere(va, 1.0f / 60.0f, groesse);
            lb.rendere(vb, 1.0f / 60.0f, groesse);
        }
        f->glFinish();
        QElapsedTimer t;
        t.start();
        for (int i = 0; i < 20; ++i) la.rendere(va, 1.0f / 60.0f, groesse);
        f->glFinish();
        const double msA = static_cast<double>(t.restart()) / 20.0;
        for (int i = 0; i < 20; ++i) lb.rendere(vb, 1.0f / 60.0f, groesse);
        f->glFinish();
        const double msB = static_cast<double>(t.restart()) / 20.0;
        for (int i = 0; i < 20; ++i)
        {
            la.rendere(va, 1.0f / 60.0f, groesse);
            lb.rendere(vb, 1.0f / 60.0f, groesse);
            REQUIRE(mischer.beginne(groesse));
            mischer.lege(la, 1.0, Mischart::Normal);
            mischer.lege(lb, 0.5, Mischart::Additive);
            mischer.zeichneIn(0, groesse);
        }
        f->glFinish();
        const double msBeide = static_cast<double>(t.restart()) / 20.0;
        for (int i = 0; i < 200; ++i)
        {
            REQUIRE(mischer.beginne(groesse));
            mischer.lege(la, 1.0, Mischart::Normal);
            mischer.lege(lb, 0.5, Mischart::Additive);
            mischer.zeichneIn(0, groesse);
        }
        f->glFinish();
        const double msMischen = static_cast<double>(t.elapsed()) / 200.0;
        // je Renderer: Root-Bildpaar + Leinwand, RGBA8; der Mischer: zwei Bilder
        const double mbBild = groesse.width() * groesse.height() * 4.0 / (1024.0 * 1024.0);
        MESSAGE("MESSUNG 1280x720 (Testing-Build): A " << msA << " ms, B " << msB
                << " ms, beide + mischen " << msBeide << " ms, davon Mischen allein " << msMischen
                << " ms je Bild; Bildspeicher gerechnet: je Renderer mindestens "
                << 3.0 * mbBild << " MB (Bildpaar + Leinwand), Mischer " << 2.0 * mbBild << " MB");
        CHECK(msMischen < 50.0);
    }
}
