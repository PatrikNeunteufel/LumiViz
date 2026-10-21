/**
 ****************************************************************************************
 * @file   test_LoopClipExpression.cpp
 * @brief  Loop, Stufe 28e: die Expression als Kurve auf dem Clip im Loop-Panel
 *         (docs/audio/Spielweisen_Konzept.md §1a; Entscheide Patrik S140/S141: SF5 d,
 *         SK8 a, SK13 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster, wie im Programm: die Achse meldet, das Panel
 * fuehrt ueber den CommandBus aus. `LUMIVIZ_LOOP_EXPRESSION_BILD=<datei.png>` legt ein
 * Bild der Achse ab (ein markierter Clip mit Kurve, einer ohne, einer mit blasser Kurve).
 *
 * - Geometrie: Wert und Stelle hin und zurueck, Sechzehntel eines Takts, mit Alt frei.
 * - Nur der allein markierte Clip ist greifbar; ohne Kurve die Linie bei 1.
 * - Klick auf die Linie setzt, ziehen verschiebt (zwischen den Nachbarn), Doppelklick
 *   loescht (und oeffnet keinen Editor), *Clear expression* im Menue — je ein Undo-Schritt.
 * - Ein nicht markierter Clip verschiebt sich wie bisher.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "UI/widgets/Bedienhilfe.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>

#include <memory>

using namespace lumi::loop;

namespace
{

/// Zwei Basen (A: 4 Takte, B: 8 Takte), zwei Kanaele — wie in test_LoopArrangement:
/// Drums: A ab Takt 0 zweimal, B ab Takt 8; Perc: A ab Takt 4.
Loop probe()
{
    Loop loop;
    Muster a;
    a.name = QStringLiteral("A");
    a.takte = 4;
    Muster b;
    b.name = QStringLiteral("B");
    b.takte = 8;
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("A"), 0, 2}, {QStringLiteral("B"), 8, 1}}},
                    {QStringLiteral("Perc"), true, {{QStringLiteral("A"), 4, 1}}}};
    return loop;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten,
          Qt::KeyboardModifiers dazu = Qt::NoModifier)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, dazu);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::LeftButton, Qt::NoButton);
}

void ziehe(QWidget& ziel, QPoint von, QPoint nach, Qt::KeyboardModifiers dazu = Qt::NoModifier)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, Qt::LeftButton, dazu);
    maus(ziel, QEvent::MouseMove, (von + nach) / 2, Qt::NoButton, Qt::LeftButton, dazu);
    maus(ziel, QEvent::MouseMove, nach, Qt::NoButton, Qt::LeftButton, dazu);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::LeftButton, Qt::NoButton, dazu);
}

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopClipExpression"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("LoopClipExpression")
{
    TEST_CASE("Loop Clip Expression 28e: die Kurve auf dem Clip — setzen, ziehen, loeschen, Clear expression, Undo")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);
        LoopPanel panel(dienste);  // nicht gezeigt
        panel.resize(1200, 300);
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("loopClip"));
        REQUIRE(clipZeile != nullptr);
        panel.neu();  // wie im Programm: erst New (oder Load), dann ist die Achse bereit
        panel.setzeLoop(probe());
        Taktachse& achse = *panel.taktachse();
        achse.resize(1200, 220);
        int editoren = 0;
        QObject::connect(&achse, &Taktachse::editorVerlangt, &achse, [&editoren](const QString&) { ++editoren; });
        const auto kurve = [&panel](int k, int c) {
            return panel.loop().kanaele[static_cast<std::size_t>(k)].clips[static_cast<std::size_t>(c)].expression;
        };
        const auto clip = [&achse](int k, int c) -> const LoopClip& {
            return achse.loop().kanaele[static_cast<std::size_t>(k)].clips[static_cast<std::size_t>(c)];
        };

        // --- Geometrie: Wert und Stelle hin und zurueck
        const QRect r = achse.clipRechteck(0, 0);
        REQUIRE(r.height() >= 20);
        CHECK(Taktachse::ausdruckBei(r, Taktachse::yVonAusdruck(r, 0.5)) == doctest::Approx(0.5));
        CHECK(Taktachse::yVonAusdruck(r, 1.0) < Taktachse::yVonAusdruck(r, 0.0));
        CHECK(Taktachse::ausdruckBei(r, r.top()) == 1.0);
        CHECK(achse.clipStelleBei(clip(0, 0), achse.xVonClipStelle(clip(0, 0), 2.5), false) == 2.5);
        CHECK(achse.clipStelleBei(clip(0, 0), achse.xVonClipStelle(clip(0, 0), 3.0), false) == 3.0);

        // --- nur der allein markierte Clip ist greifbar (SK13 a)
        const int linie = Taktachse::yVonAusdruck(r, 1.0);
        const QPoint aufLinie(achse.xVonClipStelle(clip(0, 0), 2.0), linie);
        CHECK_FALSE(achse.kurveGreifbar(0, 0));
        CHECK_FALSE(achse.aufAusdruck(0, 0, aufLinie));
        achse.waehle(0, 0);
        CHECK(achse.kurveGreifbar(0, 0));
        CHECK_FALSE(achse.kurveGreifbar(0, 1));
        CHECK(achse.aufAusdruck(0, 0, aufLinie));
        CHECK_FALSE(achse.aufAusdruck(0, 0, aufLinie + QPoint(0, 10)));
        // ueber der Linie: Zeiger und Hinweis
        maus(achse, QEvent::MouseMove, aufLinie, Qt::NoButton, Qt::NoButton);
        CHECK(achse.cursor().shape() == Qt::CrossCursor);
        CHECK(clipZeile->text().startsWith(QStringLiteral("Click to add an expression point")));

        // --- ein Klick auf die Linie setzt einen Punkt — ein Undo-Schritt, im Loop angekommen
        klick(achse, aufLinie);
        CHECK(bus->undoDescription() == "Add expression point");
        CHECK(kurve(0, 0) == std::vector<SpurPunkt>{{2.0, 1.0}});
        CHECK(clip(0, 0).startTakt == 0);  // der Clip ist nicht verschoben

        // --- ziehen: auf Takt 3, halbe Staerke
        const QPoint ziel(achse.xVonClipStelle(clip(0, 0), 3.0), Taktachse::yVonAusdruck(r, 0.5));
        ziehe(achse, aufLinie, ziel);
        CHECK(bus->undoDescription() == "Move expression point");
        CHECK(kurve(0, 0) == std::vector<SpurPunkt>{{3.0, 0.5}});
        CHECK(clip(0, 0).startTakt == 0);
        // ueber dem Punkt: der Hinweis nennt den Pegel (SK9 a)
        maus(achse, QEvent::MouseMove, ziel, Qt::NoButton, Qt::NoButton);
        CHECK(achse.cursor().shape() == Qt::SizeAllCursor);
        CHECK(clipZeile->text().startsWith(QStringLiteral("Expression -12.0 dB")));
        // mit Alt frei
        const QPoint frei(ziel.x() + 3, ziel.y());
        ziehe(achse, ziel, frei, Qt::AltModifier);
        CHECK(kurve(0, 0).front().stelle == achse.clipStelleBei(clip(0, 0), frei.x(), true));
        const double stelleFrei = kurve(0, 0).front().stelle;

        // --- ein zweiter Punkt auf der Kurve; er bleibt beim Ziehen hinter dem ersten
        const QPoint zweiter(achse.xVonClipStelle(clip(0, 0), 6.0), Taktachse::yVonAusdruck(r, 0.5));
        REQUIRE(achse.aufAusdruck(0, 0, zweiter));
        klick(achse, zweiter);
        REQUIRE(kurve(0, 0).size() == 2);
        // (der Zug geht ueber Takt 3,5 — dort war noch Platz, dort bleibt er; Takt 1 laege davor)
        ziehe(achse, zweiter, QPoint(achse.xVonClipStelle(clip(0, 0), 1.0), zweiter.y()));
        REQUIRE(kurve(0, 0).size() == 2);
        CHECK(kurve(0, 0).back().stelle == 3.5);
        CHECK(kurve(0, 0).back().stelle > kurve(0, 0).front().stelle);
        REQUIRE(bus->undo());  // wieder auf Takt 6
        REQUIRE(kurve(0, 0).back().stelle == 6.0);

        // --- ein Doppelklick auf den Punkt loescht ihn — und oeffnet keinen Editor
        maus(achse, QEvent::MouseButtonPress, zweiter, Qt::LeftButton, Qt::LeftButton);
        maus(achse, QEvent::MouseButtonRelease, zweiter, Qt::LeftButton, Qt::NoButton);
        maus(achse, QEvent::MouseButtonDblClick, zweiter, Qt::LeftButton, Qt::LeftButton);
        maus(achse, QEvent::MouseButtonRelease, zweiter, Qt::LeftButton, Qt::NoButton);
        CHECK(bus->undoDescription() == "Delete expression point");
        CHECK(kurve(0, 0) == std::vector<SpurPunkt>{{stelleFrei, 0.5}});
        CHECK(editoren == 0);
        // Undo bringt ihn zurueck
        REQUIRE(bus->undo());
        CHECK(kurve(0, 0).size() == 2);

        // --- das Menue: Clear expression
        QStringList eintraege;
        lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
            QAction* leeren = nullptr;
            for (QAction* a : menue.actions())
                if (a->text() == QStringLiteral("Clear expression"))
                {
                    eintraege << a->text() + (a->isEnabled() ? QString() : QStringLiteral(" (off)"));
                    leeren = a;
                }
            return leeren;
        });
        const QPoint mitte = achse.clipRechteck(0, 0).center();
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, mitte, achse.mapToGlobal(mitte));
        QCoreApplication::sendEvent(&achse, &rechts);
        CHECK(eintraege == QStringList{QStringLiteral("Clear expression")});
        CHECK(bus->undoDescription() == "Clear expression");
        CHECK(kurve(0, 0).empty());
        // ohne Kurve ist der Eintrag grau
        eintraege.clear();
        QCoreApplication::sendEvent(&achse, &rechts);
        lumi::ui::setzeMenueBeobachterFuerTests({});
        CHECK(eintraege == QStringList{QStringLiteral("Clear expression (off)")});
        REQUIRE(bus->undo());  // die Kurve wieder da
        REQUIRE(kurve(0, 0).size() == 2);

        // --- ein nicht markierter Clip verschiebt sich wie bisher, auch an der Hoehe der Linie
        const QRect perc = achse.clipRechteck(1, 0);
        const QPoint percLinie(perc.center().x(), Taktachse::yVonAusdruck(perc, 1.0));
        CHECK_FALSE(achse.aufAusdruck(1, 0, percLinie));
        ziehe(achse, percLinie, percLinie + QPoint(achse.xVonTakt(1) - achse.xVonTakt(0), 0));
        CHECK(bus->undoDescription() == "Move clip");
        CHECK(clip(1, 0).startTakt == 5);
        CHECK(kurve(1, 0).empty());

        // --- mehrere markiert: keiner ist greifbar
        achse.markiere({ClipOrt{0, 0}, ClipOrt{1, 5}});
        CHECK_FALSE(achse.kurveGreifbar(0, 0));
        CHECK(achse.ausdruckPunktBei(0, 0, ziel) == -1);

        if (const QString bild = qEnvironmentVariable("LUMIVIZ_LOOP_EXPRESSION_BILD"); !bild.isEmpty())
        {
            // ein markierter Clip mit Kurve (A x 2), einer ohne (B), einer mit blasser Kurve (Perc)
            Loop mitKurven = panel.loop();
            mitKurven.kanaele[1].clips[0].expression = {{0.0, 1.0}, {1.0, 0.2}};
            panel.setzeLoop(mitKurven);
            achse.resize(1200, 160);
            achse.waehle(0, 0);
            CHECK(achse.grab().save(bild));
        }
    }
}
