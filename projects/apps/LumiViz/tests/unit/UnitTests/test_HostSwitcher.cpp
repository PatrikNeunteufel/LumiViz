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
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
        sw.params = sp;
        sw.songCode = "switch=1;";  // Song-Slot am Knoten (allgemein, S82)
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
        CHECK(zurueck.children[0].songCode == "switch=1;");
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
                                bool songDazwischen = false,
                                const std::string& songCode = {}) -> Lauf {
            ChainNode root = wurzel();
            ChainNode sw;
            sw.params = std::move(sp);
            sw.songCode = songCode;
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
            CHECK(laufen(skript, 4, true, "switch=1;").index == 1);
        }
        SUBCASE("Mitgelieferte Voreinstellungen (S87): Skripte laufen fehlerfrei")
        {
            namespace np = lumi::multieffect::nodepresets;
            const auto eintraege = np::list(QStringLiteral("hostSwitcher"));
            REQUIRE(eintraege.size() >= 6);
            for (const np::Entry& e : eintraege)
            {
                CAPTURE(e.name.toStdString());
                EffectParams params = basis;
                REQUIRE(np::load(e.path, QStringLiteral("hostSwitcher"), params, nullptr));
                // der Song-Slot liegt am Knoten — aus der Datei dazulesen
                QFile f(e.path);
                REQUIRE(f.open(QIODevice::ReadOnly));
                const std::string song = QJsonDocument::fromJson(f.readAll())
                                             .object()
                                             .value(QStringLiteral("node"))
                                             .toObject()
                                             .value(QStringLiteral("songCode"))
                                             .toString()
                                             .toStdString();
                // die Gruppenliste gehoert nicht zur Voreinstellung: sie bleibt
                CHECK(std::get<HostSwitcherParams>(params).groups == basis.groups);

                ChainNode root = wurzel();
                ChainNode sw;
                sw.params = params;
                sw.songCode = song;
                root.children.push_back(std::move(sw));
                root.children.push_back(gruppe("Hell", 0xFFFFFF));
                root.children.push_back(gruppe("Grau", 0x404040));
                MultiEffectVisualizer vis;
                vis.setChain(std::move(root));
                vis.initialize();
                vis.resize(QSize(kGroesse, kGroesse));
                QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kGroesse,
                                                                          kGroesse);
                const uint64_t id = vis.chain().children[0].nodeId;
                vis.render(1.0f / 60.0f);
                MultiEffectVisualizer::notifySongChanged();
                vis.render(1.0f / 60.0f);
                CHECK(vis.debugSwitcherError(id).empty());
                const int vorher = vis.debugSwitcherIndex(id);
                if (e.name == QLatin1String("One per Song"))
                {
                    CHECK(vorher == 2);  // der Songwechsel hat geschaltet
                }
                else if (e.name == QLatin1String("Random Seconds"))
                {
                    // hoechstens max_seconds = 40: ein Frame von 50 s schaltet sicher,
                    // der naechste (wieder 50 s seit dem Wechsel) schaltet zurueck
                    CHECK(vorher == 1);
                    vis.render(50.0f);
                    CHECK(vis.debugSwitcherIndex(id) == 2);
                    vis.render(50.0f);
                    CHECK(vis.debugSwitcherIndex(id) == 1);
                }
                else
                {
                    CHECK(vorher == 1);  // ohne Beat und ohne Zeit bleibt die Startgruppe
                }
            }
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

    // S89 (Idee Patrik): „Keep group state" — beim Verlassen einer Gruppe
    // bleibt ihr Stand liegen, beim Wiederbetreten laeuft sie dort weiter.
    // `clearbuffers` und der Knopf im Panel machen die ruhenden Gruppen frisch
    // (Entscheid a: auch Skripte, Partikel, Puffer — nicht nur das Bild).
    TEST_CASE("GL S89: Keep group state — Bild, Listen und Laufzeiten bleiben; clearbuffers leert")
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

        // „Spur" loescht nie und hellt je GEZEICHNETEM Frame auf (Fadeout nach
        // Weiss) — die Helligkeit zaehlt also die Frames seit dem letzten
        // leeren Bild. Wahlweise steckt der Fadeout in einer Effect List: dann
        // traegt DEREN Flaeche das Bild.
        const auto spur = [](bool inListe) {
            ChainNode g;
            HostGroupParams hp;
            hp.crossfadeSeconds = 0.0;
            g.params = hp;
            g.displayName = "Spur";
            ChainNode fade;
            fade.params = FadeoutParams{4, 0xFFFFFF};
            if (inListe)
            {
                ChainNode liste;
                liste.params = ListParams{};  // kein Clear, In: Ignore, Out: Replace
                liste.children.push_back(fade);
                g.children.push_back(std::move(liste));
            }
            else
            {
                g.children.push_back(fade);
            }
            return g;
        };
        struct Lauf
        {
            double luma = 0.0;      ///< Bildmitte am Ende
            bool fadeLaufzeit = false;  ///< der Fadeout-Knoten traegt (noch) eine Laufzeit
        };
        // Fahrplan des Skripts: Frames 1–20 Spur · 21–30 Grau · ab 31 wieder Spur
        const auto lauf = [&](bool keep, bool inListe, const std::string& zusatz, int frames,
                              int knopfNachFrame = -1) {
            ChainNode root = wurzel();
            ChainNode sw;
            HostSwitcherParams sp;
            sp.groups = {"Spur", "Grau"};
            sp.keepState = keep;
            sp.initCode = "f=0;";
            sp.frameCode = "f=f+1; switch=if(below(f,21),1,if(below(f,31),2,1));" + zusatz;
            sw.params = sp;
            root.children.push_back(std::move(sw));
            root.children.push_back(spur(inListe));
            root.children.push_back(gruppe("Grau", 0x404040));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.setBeatPeriodOverride(100000);
            vis.initialize();
            vis.resize(QSize(kGroesse, kGroesse));
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kGroesse, kGroesse);
            for (int i = 0; i < frames; ++i)
            {
                if (i == knopfNachFrame)
                    vis.leereSwitcherPuffer(vis.chain().children[0].nodeId);  // Knopf im Panel
                vis.render(1.0f / 60.0f);
            }
            const QImage img = vis.debugGrabRootSurface();
            REQUIRE(!img.isNull());
            const QColor c = img.pixelColor(kGroesse / 2, kGroesse / 2);
            const ChainNode& g = vis.chain().children[1];
            const uint64_t fadeId = inListe ? g.children[0].children[0].nodeId : g.children[0].nodeId;
            return Lauf{(c.redF() + c.greenF() + c.blueF()) / 3.0, vis.debugHatLaufzeit(fadeId)};
        };

        // --- ohne Puffer (wie bisher): nach der Rueckkehr beginnt das Bild leer
        const Lauf ohne = lauf(false, false, "", 35);   // 5 Frames seit dem leeren Bild
        const Lauf kurz = lauf(false, false, "", 5);    // Vergleich: 5 Frames von vorn
        CHECK(ohne.luma == doctest::Approx(kurz.luma).epsilon(0.05));
        CHECK(ohne.luma > 0.02);
        CHECK(ohne.luma < 0.2);

        // --- mit Puffer: die Gruppe laeuft dort weiter, wo sie war (20 + 5 Frames)
        const Lauf mit = lauf(true, false, "", 35);
        // der Fadeout hellt je Frame gleich viel auf: 25 Frames = fuenfmal 5 Frames
        CHECK(mit.luma == doctest::Approx(kurz.luma * 5.0).epsilon(0.05));
        CHECK(mit.luma > ohne.luma * 3.0);

        // --- dasselbe, wenn eine verschachtelte Effect List das Bild traegt:
        //     ihre Laufzeit fiele ohne Puffer am Frame-Ende weg
        const Lauf mitListe = lauf(true, true, "", 35);
        const Lauf ohneListe = lauf(false, true, "", 35);
        CHECK(mitListe.luma == doctest::Approx(mit.luma).epsilon(0.05));
        CHECK(ohneListe.luma == doctest::Approx(ohne.luma).epsilon(0.05));

        // --- waehrend die Gruppe ruht, bleibt die Laufzeit ihrer Knoten liegen …
        CHECK(lauf(true, false, "", 26).fadeLaufzeit);
        // … clearbuffers macht sie frisch: Laufzeit weg, Bild beginnt leer
        const std::string leeren = "clearbuffers=equal(f,25);";
        CHECK_FALSE(lauf(true, false, leeren, 26).fadeLaufzeit);
        CHECK(lauf(true, false, leeren, 35).luma == doctest::Approx(ohne.luma).epsilon(0.05));
        CHECK(lauf(true, true, leeren, 35).luma == doctest::Approx(ohne.luma).epsilon(0.05));
        // der Knopf im Panel tut dasselbe
        CHECK(lauf(true, false, "", 35, 24).luma == doctest::Approx(ohne.luma).epsilon(0.05));
        // die LAUFENDE Gruppe wird nicht angefasst: Leeren, waehrend Spur laeuft
        CHECK(lauf(true, false, "clearbuffers=equal(f,10);", 20).luma ==
              doctest::Approx(lauf(false, false, "", 20).luma).epsilon(0.05));
    }

    TEST_CASE("Serialisierung S89: keepState ueberlebt den Roundtrip, Vorgabe aus")
    {
        ChainNode root = wurzel();
        ChainNode sw;
        HostSwitcherParams sp;
        CHECK_FALSE(sp.keepState);
        sp.keepState = true;
        sw.params = sp;
        root.children.push_back(std::move(sw));
        compileChain(root);
        ChainNode zurueck = nodeFromJson(nodeToJson(root), nullptr);
        REQUIRE(zurueck.children.size() == 1);
        CHECK(std::get<HostSwitcherParams>(zurueck.children[0].params).keepState);
        // alte Dateien ohne das Feld: aus
        QJsonObject alt = nodeToJson(root);
        QJsonArray kinder = alt.value("children").toArray();
        QJsonObject k = kinder[0].toObject();
        // das Feld steht je nach Format direkt am Knoten oder in einem
        // Unterobjekt — ueberall entfernen
        bool entfernt = k.contains("keepState");
        k.remove("keepState");
        for (const QString& schluessel : k.keys())
        {
            if (!k.value(schluessel).isObject()) continue;
            QJsonObject unter = k.value(schluessel).toObject();
            entfernt = entfernt || unter.contains("keepState");
            unter.remove("keepState");
            k[schluessel] = unter;
        }
        REQUIRE(entfernt);
        kinder[0] = k;
        alt["children"] = kinder;
        ChainNode ohneFeld = nodeFromJson(alt, nullptr);
        REQUIRE(ohneFeld.children.size() == 1);
        CHECK_FALSE(std::get<HostSwitcherParams>(ohneFeld.children[0].params).keepState);
    }
}
