/**
 ****************************************************************************************
 * @file   test_HostSwitcher.cpp
 * @brief  Host Switcher (S82): eindeutige Gruppennamen, Umbenennen wie ein
 *         Refactoring, Serialisierung und das Umschalten zur Laufzeit (GL)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Der Switcher spricht Host-Gruppen ueber ihren Namen an — deshalb haelt der
 * Kompilierlauf die Namen eindeutig („Host Group", „Host Group 2", …), und
 * Umbenennen zieht jede Switcher-Liste mit. Geschaltet wird nur zur Laufzeit
 * (Entscheid Patrik S82, 2a): das gespeicherte `enabled` bleibt unberuehrt.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

using namespace lumi::multieffect;

namespace {

ChainNode gruppe(const std::string& name, uint32_t farbe = 0xFFFFFF, bool an = false)
{
    ChainNode g;
    HostGroupParams hp;
    hp.crossfadeSeconds = 0.0;  // sofort — der Test prueft das Schalten, nicht den Blend
    g.params = hp;
    g.displayName = name;
    g.enabled = an;
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    g.children.push_back(std::move(clear));
    return g;
}

ChainNode wurzel()
{
    ChainNode root;
    root.params = ListParams{};
    return root;
}

}  // namespace

TEST_SUITE("HostSwitcher")
{
    TEST_CASE("Kompilieren macht Gruppennamen eindeutig: erster behaelt, weitere zaehlen")
    {
        ChainNode root = wurzel();
        root.children.push_back(gruppe("Host Group"));
        root.children.push_back(gruppe("Host Group"));
        root.children.push_back(gruppe("Host Group 2"));  // belegt schon „2"
        root.children.push_back(gruppe("Host Group"));
        root.children.push_back(gruppe(""));  // leer → Typname → Zaehler
        const CompileResult r = compileChain(root);
        CHECK(root.children[0].displayName == "Host Group");
        CHECK(root.children[1].displayName == "Host Group 3");
        CHECK(root.children[2].displayName == "Host Group 2");
        CHECK(root.children[3].displayName == "Host Group 4");
        CHECK(root.children[4].displayName == "Host Group 5");
        CHECK(r.warnings.size() >= 3);  // jede Umbenennung wird gemeldet

        // stabil: ein zweiter Lauf aendert nichts mehr
        compileChain(root);
        CHECK(root.children[1].displayName == "Host Group 3");
    }

    TEST_CASE("freierHostGruppenName: eigener Name ist frei, fremder nicht")
    {
        ChainNode root = wurzel();
        root.children.push_back(gruppe("Intro"));
        root.children.push_back(gruppe("Refrain"));
        compileChain(root);
        CHECK(freierHostGruppenName(root, "Intro", &root.children[0]) == "Intro");
        CHECK(freierHostGruppenName(root, "Intro", &root.children[1]) == "Intro 2");
        CHECK(freierHostGruppenName(root, "Bridge") == "Bridge");
    }

    TEST_CASE("Umbenennen zieht jede Switcher-Liste mit (Refactoring)")
    {
        ChainNode root = wurzel();
        root.children.push_back(gruppe("A"));
        root.children.push_back(gruppe("B"));
        for (int i = 0; i < 2; ++i)
        {
            ChainNode sw;
            HostSwitcherParams sp;
            sp.groups = {"A", "B", "A"};
            sw.params = sp;
            root.children.push_back(std::move(sw));
        }
        CHECK(hostGruppeUmbenennenInSwitchern(root, "A", "Intro") == 4);
        const auto& g = std::get<HostSwitcherParams>(root.children[2].params).groups;
        CHECK(g == std::vector<std::string>{"Intro", "B", "Intro"});
    }

    TEST_CASE("Serialisierung: Roundtrip aller Felder, Startindex geklemmt")
    {
        ChainNode root = wurzel();
        ChainNode sw;
        HostSwitcherParams sp;
        sp.groups = {"Intro", "Refrain"};
        sp.startIndex = 2;
        sp.intervalSeconds = 12.5;
        sp.nextOnSongChange = true;
        sp.order = 1;
        sp.initCode = "n=0;";
        sp.frameCode = "nextgroup=above(beatcount,31);";
        sp.beatCode = "n=n+1;";
        sp.songCode = "switch=1;";
        sw.params = sp;
        root.children.push_back(std::move(sw));
        compileChain(root);

        const QJsonObject json = nodeToJson(root);
        QStringList report;
        ChainNode zurueck = nodeFromJson(json, &report);
        REQUIRE(zurueck.children.size() == 1);
        const auto* p = std::get_if<HostSwitcherParams>(&zurueck.children.at(0).params);
        REQUIRE(p != nullptr);
        CHECK(p->groups == sp.groups);
        CHECK(p->startIndex == 2);
        CHECK(p->intervalSeconds == doctest::Approx(12.5));
        CHECK(p->nextOnSongChange);
        CHECK(p->order == 1);
        CHECK(p->songCode == "switch=1;");
        CHECK(p->frameCode == sp.frameCode);

        // Startindex jenseits der Liste wird beim Kompilieren geklemmt
        std::get<HostSwitcherParams>(zurueck.children[0].params).startIndex = 9;
        compileChain(zurueck);
        CHECK(std::get<HostSwitcherParams>(zurueck.children[0].params).startIndex == 2);
    }

    TEST_CASE("GL: Switcher schaltet nach Startgruppe, Skript, Zeit und Songwechsel")
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
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — GL-Teil uebersprungen");
            return;
        }
        QOffscreenSurface surface;
        surface.setFormat(ctx.format());
        surface.create();
        REQUIRE(surface.isValid());
        REQUIRE(ctx.makeCurrent(&surface));

        constexpr int kGroesse = 64;
        // Laeuft ein Setup `frames` Frames: Index des Switchers + Mittel-Luma
        struct Lauf
        {
            int index = 0;
            double luma = 0.0;
        };
        const auto laufen = [&](HostSwitcherParams sp, int frames,
                                bool songDazwischen = false) -> Lauf {
            ChainNode root = wurzel();
            ChainNode sw;
            sw.params = std::move(sp);
            root.children.push_back(std::move(sw));
            // beide Gruppen GESPEICHERT aus — nur der Switcher schaltet sie
            root.children.push_back(gruppe("Hell", 0xFFFFFF));
            root.children.push_back(gruppe("Grau", 0x404040));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.initialize();
            vis.resize(QSize(kGroesse, kGroesse));
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kGroesse, kGroesse);
            for (int i = 0; i < frames; ++i)
            {
                if (songDazwischen && i == frames / 2) MultiEffectVisualizer::notifySongChanged();
                vis.render(1.0f / 60.0f);
            }
            const QImage img = vis.debugGrabRootSurface();
            REQUIRE(!img.isNull());
            const QColor c = img.pixelColor(kGroesse / 2, kGroesse / 2);
            // gespeichertes `enabled` bleibt unberuehrt (Entscheid 2a)
            CHECK_FALSE(vis.chain().children[1].enabled);
            CHECK_FALSE(vis.chain().children[2].enabled);
            return {vis.debugSwitcherIndex(vis.chain().children[0].nodeId),
                    (c.redF() + c.greenF() + c.blueF()) / 3.0};
        };

        HostSwitcherParams basis;
        basis.groups = {"Hell", "Grau"};

        SUBCASE("Startgruppe laeuft, die andere ruht")
        {
            const Lauf l = laufen(basis, 3);
            CHECK(l.index == 1);
            CHECK(l.luma > 0.9);
            HostSwitcherParams zwei = basis;
            zwei.startIndex = 2;
            const Lauf l2 = laufen(zwei, 3);
            CHECK(l2.index == 2);
            CHECK(l2.luma == doctest::Approx(0x40 / 255.0).epsilon(0.1));
        }
        SUBCASE("Skript: switch = 2 waehlt die zweite Gruppe")
        {
            HostSwitcherParams s = basis;
            s.frameCode = "switch=2;";
            const Lauf l = laufen(s, 3);
            CHECK(l.index == 2);
            CHECK(l.luma < 0.5);
        }
        SUBCASE("Skript: nextgroup gilt nur fuer den Frame, in dem es gesetzt wird")
        {
            HostSwitcherParams s = basis;
            s.frameCode = "f=f+1; nextgroup=equal(f,2);";  // genau einmal
            CHECK(laufen(s, 6).index == 2);
        }
        SUBCASE("Zeit: nach dem Intervall weiter, danach zaehlt es neu")
        {
            HostSwitcherParams s = basis;
            s.intervalSeconds = 0.05;  // 3 Frames bei 60 fps
            CHECK(laufen(s, 2).index == 1);
            CHECK(laufen(s, 4).index == 2);
            CHECK(laufen(s, 7).index == 1);  // zweimal gewechselt: zurueck
        }
        SUBCASE("Songwechsel: nur mit Haken weiter; der Song-Slot sieht ihn")
        {
            CHECK(laufen(basis, 4, true).index == 1);
            HostSwitcherParams s = basis;
            s.nextOnSongChange = true;
            CHECK(laufen(s, 4, true).index == 2);
            HostSwitcherParams skript = basis;
            skript.startIndex = 2;
            skript.songCode = "switch=1;";
            CHECK(laufen(skript, 4, true).index == 1);
        }
        SUBCASE("Ausgeschalteter Switcher schaltet nichts: gespeicherter Zustand gilt")
        {
            ChainNode root = wurzel();
            ChainNode sw;
            sw.params = basis;
            sw.enabled = false;
            root.children.push_back(std::move(sw));
            root.children.push_back(gruppe("Hell", 0xFFFFFF, false));
            root.children.push_back(gruppe("Grau", 0x404040, true));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.initialize();
            vis.resize(QSize(kGroesse, kGroesse));
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kGroesse, kGroesse);
            vis.render(1.0f / 60.0f);
            vis.render(1.0f / 60.0f);
            const QColor c = vis.debugGrabRootSurface().pixelColor(kGroesse / 2, kGroesse / 2);
            CHECK(c.redF() < 0.5);  // Grau (gespeichert an), nicht Hell
        }
    }
}
