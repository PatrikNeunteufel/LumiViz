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
        CHECK(fertigAb == groesse.height() - 1);  // eine Zeile je Bild, im letzten fertig
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
}
