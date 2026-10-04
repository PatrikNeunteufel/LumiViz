/**
 ****************************************************************************************
 * @file   test_MovementScheiben.cpp
 * @brief  Movement-Tabelle in Scheiben (S94): dieselbe Tabelle, ueber mehrere
 *         Bilder verteilt gebaut, dazu der gemeinsame Tabellen-Speicher
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Der Knoten Movement wertet sein Skript je Pixel aus (r_trans). In einem
 * Stueck steht das Bild dafuer in Full HD rund eine halbe Sekunde. Belegt wird:
 *  - Die in Scheiben gebaute Tabelle ist Eintrag fuer Eintrag die Tabelle aus
 *    einem Stueck — auch bei einem Skript, das Zustand von Pixel zu Pixel traegt.
 *  - Im Renderer: Vorgabe „sofort" wie immer; im Scheiben-Betrieb ist das Bild
 *    nach dem Bau bitgleich mit dem Bild aus dem Sofort-Betrieb; der zweite
 *    Renderer nimmt die fertige Tabelle aus dem Speicher; ein Skript, das den
 *    gemeinsamen Zustand liest, baut auch in Scheiben, teilt seine Tabelle
 *    aber nicht (Entscheid Patrik S94).
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Mischer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/scripting/ScriptGridModule.hpp"
#include "visualizers/multieffect/EffectChain.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

using namespace lumi::multieffect;
using lumi::modules::ScriptGridModule;

namespace
{

/// Tabelle in Scheiben mit kleinstem Budget (eine Zeile je Aufruf).
std::vector<int> inScheiben(ScriptGridModule& grid, int w, int h, bool wrap, bool subpixel,
                            int* aufrufe)
{
    REQUIRE(grid.beginTransTable(w, h, wrap, subpixel));
    int n = 0;
    ScriptGridModule::TransBau stand = ScriptGridModule::TransBau::Laeuft;
    while (stand == ScriptGridModule::TransBau::Laeuft)
    {
        CHECK(grid.transTableImBau());
        stand = grid.continueTransTable(1.0e-9);
        ++n;
        REQUIRE(n <= h + 1);
    }
    REQUIRE(stand == ScriptGridModule::TransBau::Fertig);
    if (aufrufe != nullptr) *aufrufe = n;
    std::vector<int> tab = grid.takeTransTable();
    CHECK_FALSE(grid.transTableImBau());
    return tab;
}

ChainNode movementKette(const std::string& code, bool rect)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode balken;
    balken.params = DebugBarsParams{0x40C0FF, -2.5f};
    ChainNode move;
    MovementParams mp;
    mp.code = code;
    mp.rectCoords = rect;
    move.params = mp;
    root.children = {clear, balken, move};
    return root;
}

int verschieden(const QImage& a, const QImage& b)
{
    if (a.size() != b.size()) return -1;
    int n = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            if (a.pixel(x, y) != b.pixel(x, y)) ++n;
    return n;
}

/// Stellt die prozessweite Vorgabe am Ende wieder her — auch nach einem REQUIRE.
struct ScheibenWaechter
{
    ~ScheibenWaechter()
    {
        MultiEffectVisualizer::setMovementScheibeMs(0.0);
        MultiEffectVisualizer::leereMovementSpeicher();
    }
};

}  // namespace

TEST_SUITE("MovementScheiben")
{
    // Kein Test im engeren Sinn: laeuft nur mit LUMIVIZ_MOVEMENT_PROBE=<preset>
    // und rendert das Preset einmal sofort und einmal in Scheiben, je
    // LUMIVIZ_MOVEMENT_PROBE_BILDER Bilder (Vorgabe 300) bei 1280 x 720, ohne
    // Audio und mit festem Takt. Schreibt `sofort.png` und `scheiben.png` nach
    // LUMIVIZ_MOVEMENT_PROBE_AUS und nennt den Unterschied der Endbilder —
    // fuer Befunde wie „sieht in der App anders aus" (Sichttest S94, F7).
    TEST_CASE("Probe: ein Preset sofort gegen in Scheiben (nur mit LUMIVIZ_MOVEMENT_PROBE)")
    {
        const QString preset = qEnvironmentVariable("LUMIVIZ_MOVEMENT_PROBE");
        if (preset.isEmpty()) return;
        const QString aus = qEnvironmentVariable("LUMIVIZ_MOVEMENT_PROBE_AUS");
        int bilder = qEnvironmentVariableIntValue("LUMIVIZ_MOVEMENT_PROBE_BILDER");
        if (bilder <= 0) bilder = 300;

        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QGuiApplication app{argc, argv};
        QOpenGLContext ctx;
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        QOffscreenSurface surface;
        REQUIRE(ctx.create());
        surface.setFormat(ctx.format());
        surface.create();
        REQUIRE(ctx.makeCurrent(&surface));
        ScheibenWaechter waechter;

        const QSize groesse(1280, 720);
        QImage ende[2];
        int fertigAb[2] = {-1, -1};
        for (int modus = 0; modus < 2; ++modus)
        {
            MultiEffectVisualizer::leereMovementSpeicher();
            MultiEffectVisualizer::setMovementScheibeMs(modus == 0 ? 0.0 : 4.0);
            MultiEffectVisualizer vis;
            const QString endung = preset.section(QLatin1Char('.'), -1).toLower();
            const bool geladen = endung == QLatin1String("avs")    ? vis.loadAvsFile(preset)
                                 : endung == QLatin1String("milk") ? vis.loadMilkFile(preset)
                                                                   : vis.loadChainFile(preset);
            REQUIRE(geladen);
            // fester Beat und kuenstliche Audiodaten, in beiden Laeufen dieselben —
            // ohne sie arbeiten viele Presets gar nicht (Listen „on beat")
            vis.setBeatPeriodOverride(30);
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            std::vector<float> spektrum(256), welle(576);
            for (int i = 0; i < bilder; ++i)
            {
                const double t = i / 60.0;
                const double schlag = std::exp(-8.0 * std::fmod(t, 0.5));
                for (size_t n = 0; n < welle.size(); ++n)
                    welle[n] = static_cast<float>((0.3 + 0.6 * schlag) *
                                                  std::sin(0.05 * static_cast<double>(n) + 6.0 * t));
                for (size_t n = 0; n < spektrum.size(); ++n)
                    spektrum[n] = static_cast<float>((0.2 + 0.8 * schlag) *
                                                     std::exp(-static_cast<double>(n) / 40.0));
                vis.updateSpectrum(spektrum.data(), static_cast<int>(spektrum.size()));
                vis.updateWaveform(welle.data(), static_cast<int>(welle.size()));
                bild.rendere(vis, 1.0f / 60.0f, groesse);
                if (vis.movementTabelleImBau()) fertigAb[modus] = i + 1;  // letztes Bau-Bild + 1
            }
            ende[modus] = bild.bild().convertToFormat(QImage::Format_RGB32);
            if (!aus.isEmpty())
                ende[modus].save(aus + (modus == 0 ? QStringLiteral("/sofort.png")
                                                   : QStringLiteral("/scheiben.png")));
            vis.cleanup();
        }
        long long summe = 0;
        int anders = 0;
        for (int y = 0; y < groesse.height(); ++y)
            for (int x = 0; x < groesse.width(); ++x)
            {
                const QRgb a = ende[0].pixel(x, y);
                const QRgb b = ende[1].pixel(x, y);
                const int d = std::abs(qRed(a) - qRed(b)) + std::abs(qGreen(a) - qGreen(b)) +
                              std::abs(qBlue(a) - qBlue(b));
                summe += d;
                if (d != 0) ++anders;
            }
        const double flaeche = static_cast<double>(groesse.width()) * groesse.height();
        MESSAGE("Probe " << preset.toStdString() << ": nach " << bilder
                         << " Bildern — Tabelle in Scheiben fertig ab Bild " << fertigAb[1]
                         << " (-1 = nie im Bau)"
                         << "; verschiedene Pixel " << anders << " ("
                         << 100.0 * anders / flaeche << " %), mittlere Abweichung je Kanal "
                         << static_cast<double>(summe) / (flaeche * 3.0) << " von 255");
    }

    TEST_CASE("Tabelle in Scheiben == Tabelle aus einem Stueck, Eintrag fuer Eintrag")
    {
        struct Fall
        {
            const char* code;
            bool rect, wrap, subpixel;
        };
        const Fall faelle[] = {
            {"x = x + (cos(y * 18) * .02); y = y + (sin(x * 14) * .03);", true, false, true},
            {"r = r + 0.04; d = d * (0.96 + cos(d * $PI) * 0.05);", false, false, true},
            {"r = cos(r * 3)", false, true, true},
            {"x=x+0.25;y=y+0.25", true, false, false},
            {"y = (r*6)/($PI); x = d;", true, true, false},
            // traegt Zustand von Pixel zu Pixel: nur in derselben Reihenfolge gleich
            {"n = n + 1; x = x + sin(n * 0.001) * 0.1;", true, false, true},
        };
        constexpr int w = 97, h = 61;  // ungerade: w/2 und h/2 runden
        for (const Fall& f : faelle)
        {
            CAPTURE(f.code);
            ScriptGridModule ganz;
            ganz.setPointCode(f.code);
            ganz.setRectCoords(f.rect);
            std::vector<int> soll;
            REQUIRE(ganz.buildTransTable(w, h, f.wrap, f.subpixel, soll));

            ScriptGridModule stueck;
            stueck.setPointCode(f.code);
            stueck.setRectCoords(f.rect);
            int aufrufe = 0;
            const std::vector<int> ist = inScheiben(stueck, w, h, f.wrap, f.subpixel, &aufrufe);
            CHECK(aufrufe == h);  // eine Zeile je Aufruf
            REQUIRE(ist.size() == soll.size());
            CHECK(ist == soll);
        }
    }

    TEST_CASE("Scheiben: Fortschritt, grosses Budget, ohne Punkt-Slot, Neubeginn mitten im Bau")
    {
        constexpr int w = 40, h = 30;
        ScriptGridModule grid;
        grid.setPointCode("x=x*0.9");
        grid.setRectCoords(true);
        CHECK(grid.continueTransTable(1.0) == ScriptGridModule::TransBau::Fehler);  // nichts begonnen

        REQUIRE(grid.beginTransTable(w, h, false, true));
        CHECK(grid.transTableFortschritt() == doctest::Approx(0.0));
        CHECK(grid.continueTransTable(1.0e-9) == ScriptGridModule::TransBau::Laeuft);
        CHECK(grid.transTableFortschritt() == doctest::Approx(1.0 / h));

        // Neubeginn verwirft den halben Bau; ohne Grenze geht er in einem Stueck durch
        REQUIRE(grid.beginTransTable(w, h, false, true));
        CHECK(grid.continueTransTable(0.0) == ScriptGridModule::TransBau::Fertig);
        const std::vector<int> a = grid.takeTransTable();
        std::vector<int> b;
        REQUIRE(grid.buildTransTable(w, h, false, true, b));
        CHECK(a == b);

        ScriptGridModule leer;
        CHECK_FALSE(leer.beginTransTable(w, h, false, true));
        CHECK_FALSE(leer.transTableImBau());
    }

    TEST_CASE("Renderer: Vorgabe sofort; in Scheiben nach dem Bau bitgleich; Speicher; unreines Skript")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QGuiApplication app{argc, argv};
        QOpenGLContext ctx;
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        QOffscreenSurface surface;
        bool gut = ctx.create();
        if (gut)
        {
            surface.setFormat(ctx.format());
            surface.create();
            gut = surface.isValid() && ctx.makeCurrent(&surface);
        }
        if (!gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        ScheibenWaechter waechter;
        CHECK(MultiEffectVisualizer::movementScheibeMs() == doctest::Approx(0.0));  // Vorgabe
        MultiEffectVisualizer::leereMovementSpeicher();

        const QSize groesse(96, 64);
        const std::string code = "x = x + (cos(y * 18) * .02); y = y + (sin(x * 14) * .03);";
        constexpr int kBilder = 80;  // mehr als Zeilen: der Bau ist sicher fertig
        constexpr float kDelta = 0.02f;

        // --- sofort: die Tabelle steht im ersten Bild ------------------------
        std::vector<QImage> soll;
        {
            MultiEffectVisualizer vis;
            vis.setChain(movementKette(code, true));
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            for (int i = 0; i < kBilder; ++i)
            {
                bild.rendere(vis, kDelta, groesse);
                CHECK_FALSE(vis.movementTabelleImBau());
                soll.push_back(bild.bild());
            }
            vis.cleanup();
        }
        CHECK(MultiEffectVisualizer::movementTabellenImSpeicher() == 0);  // sofort teilt nichts
        CHECK(verschieden(soll[0], soll[kBilder - 1]) > 0);               // die Balken wandern

        // --- in Scheiben: erst Gitter, nach dem Bau dieselben Bilder ---------
        MultiEffectVisualizer::setMovementScheibeMs(1.0e-9);  // eine Zeile je Bild
        int fertigAb = -1;
        {
            MultiEffectVisualizer vis;
            vis.setChain(movementKette(code, true));
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            for (int i = 0; i < kBilder; ++i)
            {
                bild.rendere(vis, kDelta, groesse);
                if (vis.movementTabelleImBau())
                {
                    CHECK(fertigAb < 0);  // einmal fertig, immer fertig
                }
                else
                {
                    if (fertigAb < 0) fertigAb = i;
                    CAPTURE(i);
                    CHECK(verschieden(bild.bild(), soll[static_cast<size_t>(i)]) == 0);
                }
            }
            vis.cleanup();
        }
        // kleinstes Budget: eine Zeile beim Besuch des Knotens, eine im Frame-Takt
        CHECK(fertigAb > 1);
        CHECK(fertigAb <= groesse.height());
        CHECK(MultiEffectVisualizer::movementTabellenImSpeicher() == 1);

        // --- zweiter Renderer: die Tabelle kommt aus dem Speicher ------------
        {
            MultiEffectVisualizer vis;
            vis.setChain(movementKette(code, true));
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            for (int i = 0; i < 3; ++i)
            {
                bild.rendere(vis, kDelta, groesse);
                CHECK_FALSE(vis.movementTabelleImBau());
                CHECK(verschieden(bild.bild(), soll[static_cast<size_t>(i)]) == 0);
            }
            // andere Groesse = andere Tabelle: wieder ein Bau
            const QSize gross(128, 80);
            vis.resize(gross);
            bild.rendere(vis, kDelta, gross);
            CHECK(vis.movementTabelleImBau());
            vis.cleanup();
        }
        CHECK(MultiEffectVisualizer::movementTabellenImSpeicher() == 1);  // der halbe Bau zaehlt nicht

        // --- liest den gemeinsamen Zustand (Entscheid Patrik S94: a): baut
        //     auch in Scheiben, wird aber nicht geteilt. reg00 ist hier nie
        //     gesetzt (0), die Tabelle also dieselbe wie im Sofort-Betrieb.
        const std::string unrein = "x = x + reg00 * 0.01 + 0.05;";
        QImage unreinSoll;
        {
            MultiEffectVisualizer::setMovementScheibeMs(0.0);
            MultiEffectVisualizer vis;
            vis.setChain(movementKette(unrein, true));
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            for (int i = 0; i < kBilder; ++i) bild.rendere(vis, kDelta, groesse);
            unreinSoll = bild.bild();
            vis.cleanup();
            MultiEffectVisualizer::setMovementScheibeMs(1.0e-9);
        }
        for (int lauf = 0; lauf < 2; ++lauf)
        {
            CAPTURE(lauf);
            MultiEffectVisualizer vis;
            vis.setChain(movementKette(unrein, true));
            vis.initialize();
            vis.resize(groesse);
            lumi::composer::Leinwand bild;
            bild.rendere(vis, kDelta, groesse);
            CHECK(vis.movementTabelleImBau());  // auch der zweite Lauf baut selbst
            for (int i = 1; i < kBilder; ++i) bild.rendere(vis, kDelta, groesse);
            CHECK_FALSE(vis.movementTabelleImBau());
            CHECK(verschieden(bild.bild(), unreinSoll) == 0);
            vis.cleanup();
        }
        CHECK(MultiEffectVisualizer::movementTabellenImSpeicher() == 1);  // nichts dazugekommen
    }

    TEST_CASE("Renderer: der Bau laeuft je Bild weiter, auch wenn der Knoten nur beim Beat drankommt")
    {
        // Befund Patrik S94 (Sichttest F7, „pixeliger"): ein Movement in einer
        // Liste, die nur beim Beat rechnet, bekam eine Scheibe je Beat — das
        // Gitter-Bild blieb viele Sekunden stehen.
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QGuiApplication app{argc, argv};
        QOpenGLContext ctx;
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        QOffscreenSurface surface;
        bool gut = ctx.create();
        if (gut)
        {
            surface.setFormat(ctx.format());
            surface.create();
            gut = surface.isValid() && ctx.makeCurrent(&surface);
        }
        if (!gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        ScheibenWaechter waechter;
        MultiEffectVisualizer::leereMovementSpeicher();
        MultiEffectVisualizer::setMovementScheibeMs(1.0e-9);  // eine Zeile je Scheibe

        // Wurzel → Liste „nur beim Beat, ein Bild lang" → Movement
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode balken;
        balken.params = DebugBarsParams{0x40C0FF, -2.5f};
        ChainNode beatListe;
        ListParams lp;
        lp.onBeatRender = true;
        lp.onBeatFrames = 1;
        beatListe.params = lp;
        ChainNode move;
        MovementParams mp;
        mp.code = "x = x * 0.9; y = y * 0.9;";
        mp.rectCoords = true;
        move.params = mp;
        beatListe.children = {move};
        root.children = {clear, balken, beatListe};

        const QSize groesse(96, 64);  // 64 Zeilen
        MultiEffectVisualizer vis;
        vis.setChain(root);
        vis.setBeatPeriodOverride(30);  // Beat auf Bild 0, 30, 60, 90 …
        vis.initialize();
        vis.resize(groesse);
        lumi::composer::Leinwand bild;
        int bauBilder = 0;
        for (int i = 0; i < 100; ++i)
        {
            bild.rendere(vis, 0.02f, groesse);
            if (vis.movementTabelleImBau()) ++bauBilder;
        }
        // Vier Besuche des Knotens haetten allein vier Zeilen ergeben; mit dem
        // Bau je Bild steht die Tabelle nach gut 64 Bildern und ist beim Beat
        // auf Bild 90 uebernommen (und liegt im Speicher).
        CHECK(bauBilder > 30);
        CHECK(bauBilder < 70);
        CHECK_FALSE(vis.movementTabelleImBau());
        CHECK(MultiEffectVisualizer::movementTabellenImSpeicher() == 1);
        vis.cleanup();
    }
}
