/**
 ****************************************************************************************
 * @file   test_LoopFreezeBedienung.cpp
 * @brief  Loop: Freeze bedienen — Marke, Menue, Undo, Speichern, Hinweiszeile
 *         (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30g-2; Entscheide F2 a, F5 a, FZ4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster: das Loop-Panel mit einem CommandBus, ohne Audio-Engine
 * (eingefroren wird in 44100 Hz; die Toene des Klaviers werden gerechnet). Die Dateien
 * liegen in einem Wegwerf-Ordner (`setzeFrozenOrdnerFuerTests`), die Einstellungen unter
 * `LumiVizTest` / `LoopFreezeBedienung`; Menues faengt `setzeMenueBeobachterFuerTests`.
 * `LUMIVIZ_FREEZE_BILD=<datei.png>` legt ein Bild der Achse mit den Marken ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Tonspeicher.hpp"
#include "loop/Wellenform.hpp"  // clipFassung
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QContextMenuEvent>
#include <QImage>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <QTemporaryDir>

using namespace lumi::loop;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopFreezeBedienung"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Faengt das naechste Menue: seine Eintraege, und waehlt `waehle`, wenn es ihn gibt.
struct MenueFang
{
    QStringList texte;
    QString waehle;
    MenueFang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests([this](QMenu& menue) -> QAction* {
            texte.clear();
            QAction* gewaehlt = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                texte << a->text();
                if (a->text() == waehle) gewaehlt = a;
            }
            return gewaehlt;
        });
    }
    ~MenueFang() { lumi::ui::setzeMenueBeobachterFuerTests({}); }
};

void rechts(QWidget& ziel, QPoint ort)
{
    QContextMenuEvent ereignis(QContextMenuEvent::Mouse, ort, ziel.mapToGlobal(ort));
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

/// Keys (Klavier) in zwei gleichen Clips zu vier Takten (breit genug fuer den Text der Marke),
/// Drums (ohne Klang) in einem.
Loop probeLoop()
{
    Muster keys;
    keys.name = QStringLiteral("Keys");
    keys.art = MusterArt::Instrument;
    keys.instrument = QStringLiteral("piano");
    keys.takte = 4;
    keys.noten = {Note{0, 4, 60}, Note{8, 4, 64}};
    Muster leer;
    leer.name = QStringLiteral("Beat");
    leer.takte = 2;
    Loop loop;
    loop.muster = {keys, leer};
    loop.kanaele = {{QStringLiteral("Keys"), true, {{QStringLiteral("Keys"), 0, 1}, {QStringLiteral("Keys"), 4, 1}}},
                    {QStringLiteral("Drums"), true, {{QStringLiteral("Beat"), 0, 2}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("Loop Freeze Bedienung")
{
    TEST_CASE("Loop Freeze 30g-2: Menue, Marke, Undo, Speichern, Hinweiszeile")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeFrozenOrdnerFuerTests(ordner.filePath(QStringLiteral("frozen")));
        tonspeicher().leere();

        const QString pfad = ordner.filePath(QStringLiteral("probe.lvloop"));
        REQUIRE(speichereLoop(probeLoop(), pfad));

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);
        {
            LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
            panel.resize(1000, 320);
            REQUIRE(panel.lade(pfad));
            Taktachse& achse = *panel.taktachse();
            achse.resize(1000, 240);
            MenueFang fang;
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Aus);

            // --- B: im Menue des Clips „Freeze clip" — gewaehlt friert er ein, sofort im Hintergrund
            achse.waehle(0, 0);
            fang.waehle = QStringLiteral("Freeze clip");
            rechts(achse, achse.clipRechteck(0, 0).center());
            CHECK(fang.texte.contains(QStringLiteral("Freeze clip")));
            CHECK(fang.texte.indexOf(QStringLiteral("Freeze clip")) == fang.texte.indexOf(QStringLiteral("Export audio...")) + 1);
            CHECK_FALSE(panel.loop().kanaele[0].clips[0].frozen.isEmpty());
            CHECK(bus->undoDescription() == "Freeze clip");
            CHECK(panel.friert());
            CHECK(panel.freezeHinweis().startsWith(QStringLiteral("Freezing clip 1 of 1")));
            panel.warteAufFreeze();
            CHECK_FALSE(panel.friert());
            CHECK(panel.freezeHinweis().isEmpty());
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Frozen);
            CHECK(Taktachse::freezeText(achse.freezeMarke(0, 0)) == QString(QChar(0x2744)) + QStringLiteral(" frozen"));
            CHECK(frozenStand().dateien == 1);
            CHECK(panel.ungespeichert());  // die Marke ist eine Aenderung am Loop

            // --- der zweite, gleiche Clip: seine Datei liegt schon vor, nichts zu rechnen
            achse.waehle(0, 1);
            REQUIRE(panel.friereEin(true));
            CHECK_FALSE(panel.friert());
            CHECK(achse.freezeMarke(0, 4).zustand == FrozenZustand::Frozen);

            // --- an einem eingefrorenen Clip: „Unfreeze clip"; bei zwei markierten die Zahl
            fang.waehle.clear();
            rechts(achse, achse.clipRechteck(0, 1).center());
            CHECK(fang.texte.contains(QStringLiteral("Unfreeze clip")));
            achse.markiere({ClipOrt{0, 0}, ClipOrt{1, 0}});
            rechts(achse, achse.clipRechteck(0, 0).center());
            CHECK(fang.texte.contains(QStringLiteral("Freeze 2 clips")));  // nicht alle sind eingefroren

            // --- `LUMIVIZ_FREEZE_BILD`: die Marken im Bild (eingefroren, gleich danach veraendert und rechnend)
            const QString bild = qEnvironmentVariable("LUMIVIZ_FREEZE_BILD");
            if (!bild.isEmpty()) CHECK(achse.grab().save(bild));

            // --- C: bearbeitet — veraendert, spielt live; Undo — sofort wieder eingefroren
            Loop geaendert = panel.loop();
            for (Muster& m : geaendert.muster)
                if (m.name == QStringLiteral("Keys")) m.noten[1].hoehe = 67;
            panel.bearbeite(geaendert, QStringLiteral("Edit notes"));
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Veraendert);
            CHECK(Taktachse::freezeText(achse.freezeMarke(0, 0)) == QString(QChar(0x2744)) + QStringLiteral(" frozen · changed"));
            CHECK_FALSE(panel.friert());  // nicht mitten im Bearbeiten (F2 a)
            if (!bild.isEmpty())
            {
                QString zweites = bild;
                zweites.insert(zweites.lastIndexOf(QLatin1Char('.')), QStringLiteral("_changed"));
                CHECK(achse.grab().save(zweites));
            }
            REQUIRE(bus->undo());
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Frozen);
            REQUIRE(bus->redo());
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Veraendert);

            // --- Speichern: die Marken bekommen den jetzigen Abdruck, der neue Stand friert ein
            REQUIRE(panel.speichere(pfad));
            CHECK(panel.friert());
            CHECK(achse.freezeMarke(0, 0).rechnet);
            CHECK(Taktachse::freezeText(achse.freezeMarke(0, 0)).startsWith(QStringLiteral("computing")));
            if (!bild.isEmpty())
            {
                QString drittes = bild;
                drittes.insert(drittes.lastIndexOf(QLatin1Char('.')), QStringLiteral("_computing"));
                CHECK(achse.grab().save(drittes));
            }
            panel.warteAufFreeze();
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Frozen);
            CHECK(achse.freezeMarke(0, 4).zustand == FrozenZustand::Frozen);
            CHECK_FALSE(panel.ungespeichert());
            CHECK(frozenStand().dateien == 2);  // ueberschrieben wird nie (F1 a)
            Loop gelesen;
            REQUIRE(ladeLoop(pfad, gelesen));
            CHECK(gelesen.kanaele[0].clips[0].frozen == panel.loop().kanaele[0].clips[0].frozen);

            // --- F5 a: geraeumt — beim naechsten Blick fehlt sie und wird neu gerechnet
            CHECK(leereFrozen() == 2);
            REQUIRE(panel.lade(pfad));
            CHECK(panel.friert());
            panel.warteAufFreeze();
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Frozen);

            // --- Unfreeze clip: die Marke geht (mit Undo), die Datei bleibt
            achse.markiere({ClipOrt{0, 0}, ClipOrt{0, 4}});
            REQUIRE(panel.friereEin(false));
            CHECK(bus->undoDescription() == "Unfreeze clips");
            CHECK(achse.freezeMarke(0, 0).zustand == FrozenZustand::Aus);
            CHECK(panel.loop().kanaele[0].clips[0].frozen.isEmpty());
            CHECK(frozenStand().dateien == 1);
            CHECK_FALSE(panel.friereEin(false));  // nichts mehr zu loesen
        }
        setzeFrozenOrdnerFuerTests({});
        tonspeicher().leere();
    }

    TEST_CASE("Loop Freeze 30g-2: die Kurzform auf schmalen Clips — Flocke im Quadrat, im Dreieck, Prozent")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        // vier Clips zu einem Takt: frozen, changed, computing, missing
        Muster m;
        m.name = QStringLiteral("Keys");
        m.takte = 1;
        Loop loop;
        loop.muster = {m};
        loop.kanaele = {{QStringLiteral("Keys"), true,
                         {{QStringLiteral("Keys"), 0, 1}, {QStringLiteral("Keys"), 2, 1},
                          {QStringLiteral("Keys"), 4, 1}, {QStringLiteral("Keys"), 6, 1}}}};
        loop.richte();
        Taktachse achse;  // nicht gezeigt
        achse.resize(1000, 200);
        achse.setLoop(loop, true);
        const QImage ohne = achse.grab().toImage();
        achse.setzeFreezeMarken({{{0, 0}, FreezeMarke{FrozenZustand::Frozen, false, 0.0}},
                                 {{0, 2}, FreezeMarke{FrozenZustand::Veraendert, false, 0.0}},
                                 {{0, 4}, FreezeMarke{FrozenZustand::Fehlt, true, 0.4}},
                                 {{0, 6}, FreezeMarke{FrozenZustand::Fehlt, false, 0.0}}});
        const QImage mit = achse.grab().toImage();
        CHECK(mit != ohne);
        // die Flocke im Dreieck und die im Quadrat sehen verschieden aus
        const QRect a = achse.clipRechteck(0, 0);
        const QRect b = achse.clipRechteck(0, 1);
        CHECK(mit.copy(a.adjusted(a.width() / 2, 0, 0, 0)) != mit.copy(b.adjusted(b.width() / 2, 0, 0, 0)));
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_FREEZE_BILD"); !bild.isEmpty())
        {
            QString kurz = bild;
            kurz.insert(kurz.lastIndexOf(QLatin1Char('.')), QStringLiteral("_short"));
            CHECK(mit.save(kurz));
        }
    }

    TEST_CASE("Loop Freeze 30h: die Sanduhr — Text, Tooltip mit beiden Zahlen, Menue, Kurzform")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        MenueFang fang;
        Muster m;
        m.name = QStringLiteral("Riff");
        m.takte = 4;
        Muster schmal;
        schmal.name = QStringLiteral("Fill");
        schmal.takte = 1;
        Loop loop;
        loop.muster = {m, schmal};
        loop.kanaele = {{QStringLiteral("Guitar"), true, {{QStringLiteral("Riff"), 0, 1}, {QStringLiteral("Fill"), 4, 1}}}};
        loop.richte();
        Taktachse achse;  // nicht gezeigt
        achse.resize(1000, 200);
        achse.setLoop(loop, true);

        FreezeMarke zeit;
        zeit.zeigeZeit = true;
        zeit.arbeitMs = 3200.0;
        zeit.threads = 7;
        CHECK(Taktachse::zeitText(zeit) == QString(QChar(0x23F3)) + QStringLiteral(" 3.2 s"));
        CHECK(Taktachse::freezeTooltip(zeit) ==
              QStringLiteral("Computing this clip takes 3.2 s of work (about 0.5 s waiting on 7 threads) — "
                             "Freeze clip to start it at once"));
        FreezeMarke wartend = zeit;
        wartend.warten = true;
        CHECK(Taktachse::zeitText(wartend) == QString(QChar(0x23F3)) + QStringLiteral(" 0.5 s"));
        FreezeMarke eingefroren = zeit;
        eingefroren.zustand = FrozenZustand::Frozen;
        CHECK(Taktachse::zeitText(eingefroren).isEmpty());  // eingefroren: keine Sanduhr

        const QImage ohne = achse.grab().toImage();
        achse.setzeFreezeMarken({{{0, 0}, zeit}, {{0, 4}, zeit}});
        const QImage mit = achse.grab().toImage();
        CHECK(mit != ohne);
        // im Menue die Zeit
        achse.waehle(0, 0);
        rechts(achse, achse.clipRechteck(0, 0).center());
        CHECK(fang.texte.contains(QStringLiteral("Freeze clip (3.2 s to compute)")));
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_FREEZE_BILD"); !bild.isEmpty())
        {
            QString sanduhr = bild;
            sanduhr.insert(sanduhr.lastIndexOf(QLatin1Char('.')), QStringLiteral("_hourglass"));
            CHECK(mit.save(sanduhr));
        }
    }

    TEST_CASE("Loop Freeze 30h: Rechenzeit aus dem Tonspeicher, Settings, Sanduhr im Panel")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeFrozenOrdnerFuerTests(ordner.filePath(QStringLiteral("frozen")));
        tonspeicher().leere();
        const InstrumentVon saiten = instrumentAuskunft();

        // --- Settings: Vorgabe 1 s und computing work, geklemmt, gemerkt
        CHECK(gemerkterFreezeHinweisS() == doctest::Approx(1.0));
        CHECK(gemerkteFreezeHinweisArt() == FreezeHinweisArt::Arbeit);
        merkeFreezeHinweisS(100.0);
        CHECK(gemerkterFreezeHinweisS() == doctest::Approx(60.0));
        merkeFreezeHinweisArt(FreezeHinweisArt::Warten);
        CHECK(gemerkteFreezeHinweisArt() == FreezeHinweisArt::Warten);
        merkeFreezeHinweisArt(FreezeHinweisArt::Arbeit);
        merkeFreezeHinweisS(kFreezeHinweisVorgabeS);
        CHECK_FALSE(QSettings().contains(QStringLiteral("loop/freezeHintS")));  // die Vorgabe wird nicht eingetragen

        // --- vor dem ersten Rechnen keine Zahl; danach die Summe, bei gleichen Clips dieselbe
        const Loop loop = probeLoop();
        CHECK_FALSE(clipRechenzeitMs(loop, ClipOrt{0, 0}, 44100, saiten).has_value());
        {
            const std::optional<Loop> fassung = clipFassung(loop, ClipOrt{0, 0});
            REQUIRE(fassung.has_value());
            Tonauftrag rechne(fassung->klangNamen(saiten), 44100,
                              [](const QString&, int, double) { return std::vector<float>{}; });
            rechne.rechneHier();
        }
        const std::optional<double> gemessen = clipRechenzeitMs(loop, ClipOrt{0, 0}, 44100, saiten);
        REQUIRE(gemessen.has_value());
        CHECK(*gemessen > 0.0);
        MESSAGE("Rechenzeit der Keys, gemessen: " << *gemessen << " ms");
        // die Klaviertoene rechnen in Millisekunden: fuer den Fall mit Sanduhr bekommt jeder Ton
        // im Speicher 600 ms — die Summe ist dann genau bekannt
        int toene = 0;
        {
            const std::optional<Loop> fassung = clipFassung(loop, ClipOrt{0, 0});
            REQUIRE(fassung.has_value());
            for (const QString& name : fassung->klangNamen(saiten))
            {
                TonAngabe ton;
                REQUIRE(zerlegeTon(name, ton));
                Instrument instrument;
                REQUIRE(holeInstrument(ton.instrument, instrument));
                const QString schluessel = tonSchluessel(name, 44100, instrumentAbdruck(instrument));
                const std::shared_ptr<const Klang> klang = tonspeicher().hole(schluessel);
                REQUIRE(klang != nullptr);
                tonspeicher().lege(schluessel, klang, 600.0);
                ++toene;
            }
        }
        REQUIRE(toene == 2);
        const double arbeit = 600.0 * toene;
        CHECK(clipRechenzeitMs(loop, ClipOrt{0, 0}, 44100, saiten) == doctest::Approx(arbeit));
        CHECK(clipRechenzeitMs(loop, ClipOrt{0, 4}, 44100, saiten) == doctest::Approx(arbeit));  // dieselben Toene
        CHECK(clipRechenzeitMs(loop, ClipOrt{1, 0}, 44100, saiten) == doctest::Approx(0.0));  // ohne Klang

        // --- im Panel: ueber der Schwelle die Sanduhr, darunter nicht; eingefroren keine
        const QString pfad = ordner.filePath(QStringLiteral("probe.lvloop"));
        REQUIRE(speichereLoop(loop, pfad));
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        {
            LoopPanel panel(dienste);
            REQUIRE(panel.lade(pfad));  // Schwelle 1 s, computing work: 1,2 s
            Taktachse& achse = *panel.taktachse();
            const FreezeMarke marke = achse.freezeMarke(0, 0);
            CHECK(marke.zeigeZeit);
            CHECK(marke.arbeitMs == doctest::Approx(arbeit));
            CHECK(marke.threads == Tonauftrag::rechenThreads());
            CHECK(Taktachse::zeitText(marke) == QString(QChar(0x23F3)) + QStringLiteral(" 1.2 s"));
            CHECK_FALSE(achse.freezeMarke(1, 0).zeigeZeit);  // 0 s

            // waiting time: 1,2 s geteilt durch die Threads — unter 1 s, sobald es mehr als einer ist
            merkeFreezeHinweisArt(FreezeHinweisArt::Warten);
            REQUIRE(panel.lade(pfad));
            CHECK(achse.freezeMarke(0, 0).zeigeZeit == (arbeit / 1000.0 / Tonauftrag::rechenThreads() >= 1.0));
            merkeFreezeHinweisArt(FreezeHinweisArt::Arbeit);

            merkeFreezeHinweisS(60.0);
            REQUIRE(panel.lade(pfad));
            CHECK_FALSE(achse.freezeMarke(0, 0).zeigeZeit);  // unter 60 s
            merkeFreezeHinweisS(kFreezeHinweisVorgabeS);
            REQUIRE(panel.lade(pfad));
            REQUIRE(achse.freezeMarke(0, 0).zeigeZeit);

            achse.waehle(0, 0);
            REQUIRE(panel.friereEin(true));
            panel.warteAufFreeze();
            CHECK_FALSE(achse.freezeMarke(0, 0).zeigeZeit);  // eingefroren: keine Sanduhr
        }
        setzeFrozenOrdnerFuerTests({});
        tonspeicher().leere();
    }
}
