/**
 ****************************************************************************************
 * @file   test_ComposerSichttestStart.cpp
 * @brief  Composer: die Aktionen unter „Vorbereiten" der Sichttests erreichen aus jedem
 *         wiederhergestellten Startzustand denselben Stand (S105, Beleg vor dem Bau der
 *         Sichttest-Steuerung)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Entscheid Patrik W1 (S104): `--testing` aendert den Start nicht — LumiViz stellt die
 * zuletzt geladene Komposition wieder her wie immer, erst danach laufen die Aktionen des
 * Testers. Dieser Test belegt, was daraus folgt: je Fall ein erster Lauf, der einen Stand
 * hinterlaesst (`Sitzung::sichereStand`, wie bei `aboutToQuit`), dann ein zweiter Lauf mit
 * neuem Fenster und neuem Panel, der ihn wiederherstellt (Naht N4) und danach die vier
 * Aktionen `komposition_laden`, `composer`, `edit`, `fit` ausfuehrt.
 *
 * Zwei Wege fuer `komposition_laden` stehen nebeneinander: `starteKomposition` (der Weg
 * von File → Open Composition, er spielt sofort ab) und `Sitzung::lade` (haelt am Anfang).
 *
 * Nicht erfasst: das Preset der Effektkette, die Playlist und die Wiederaufnahme des
 * Players — sie gehoeren dem Hauptfenster, das dieser Test nicht baut.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "composer/Sitzung.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>
#include <QStringList>
#include <QTemporaryDir>
#include <QThread>

#include <functional>
#include <memory>
#include <string>
#include <vector>

using namespace lumi::composer;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerSichttestStart"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 8000)
{
    QElapsedTimer uhr;
    uhr.start();
    while (!bedingung())
    {
        if (uhr.elapsed() > hoechstensMs) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    }
    return true;
}

QString vorlage(const char* name)
{
    return QDir::cleanPath(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() +
                           QStringLiteral("/../../../../../../asset/sichttest/") +
                           QString::fromLatin1(name));
}

/// Ein Programmlauf, soweit der Composer ihn sieht: Fenster mit Render-Thread und Panel.
struct Lauf
{
    ServiceContainer dienste;
    VisualizerWidget widget;
    ComposerPanel panel;

    Lauf() : widget(dienste), panel(dienste)
    {
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        panel.setFenster(&widget);
        panel.resize(900, 260);
        testfenster::platziere(panel);
        panel.show();
    }

    [[nodiscard]] bool steht()
    {
        return widget.hasVisualizer() && warte([&] { return widget.isVisible(); }) &&
               warte([&] { return panel.zeitachse()->width() > 400; });
    }
    [[nodiscard]] Sitzung& sitzung() { return Sitzung::fuer(widget); }
    [[nodiscard]] QCheckBox* haken(const char* name)
    {
        return panel.findChild<QCheckBox*>(QString::fromLatin1(name));
    }
};

enum class Weg
{
    Oeffnen,  ///< `starteKomposition` — wie File → Open Composition
    Laden,    ///< `Sitzung::lade` — haelt am Anfang
};

/// Die vier Aktionen in der Reihenfolge der Listen: Load..., Composer on, Edit, Fit.
bool bereiteVor(Lauf& lauf, const QString& datei, Weg weg, bool edit)
{
    QStringList bericht;
    const bool geladen = weg == Weg::Oeffnen ? starteKomposition(lauf.widget, datei, &bericht)
                                             : lauf.sitzung().lade(datei, &bericht);
    if (!geladen) return false;
    lauf.sitzung().setAn(true);
    lauf.haken("composerEdit")->setChecked(edit);
    lauf.panel.zeitachse()->setzeZoomZurueck();
    QCoreApplication::processEvents();
    return bericht.isEmpty();
}

/// Was die vier Aktionen bestimmen sollen. `mitLauf`: auch „laeuft" und die Stelle.
QStringList kern(Lauf& lauf, const Komposition& soll, bool mitLauf)
{
    Sitzung& s = lauf.sitzung();
    const Zeitachse& achse = *lauf.panel.zeitachse();
    QStringList text{
        QStringLiteral("geladen=%1").arg(s.geladen()),
        QStringLiteral("datei=%1").arg(QFileInfo(s.datei()).fileName()),
        QStringLiteral("wie die Datei=%1").arg(s.geladen() && gleich(s.komposition(), soll)),
        QStringLiteral("ungespeichert=%1").arg(s.ungespeichert()),
        QStringLiteral("Composer on=%1").arg(s.an()),
        QStringLiteral("Haken Composer on=%1").arg(lauf.haken("composerAn")->isChecked()),
        QStringLiteral("Haken Edit=%1").arg(lauf.haken("composerEdit")->isChecked()),
        QStringLiteral("Achse bearbeiten=%1").arg(achse.bearbeiten()),
        QStringLiteral("Zeit ganz=%1").arg(achse.zeitGanz()),
        QStringLiteral("Zeilenhoehe=%1").arg(achse.zeilenProzent()),
        QStringLiteral("rollX=%1").arg(achse.rollX()),
        QStringLiteral("rollY=%1").arg(achse.rollY()),
        QStringLiteral("Auswahl=%1/%2").arg(achse.gewaehlterKanal()).arg(achse.gewaehlterClip()),
        QStringLiteral("Bindung=%1").arg(static_cast<int>(s.bindung())),
    };
    if (mitLauf)
        text << QStringLiteral("laeuft=%1").arg(s.laeuft())
             << QStringLiteral("stelle=%1").arg(s.stelleMs());
    return text;
}

/// Ein erster Lauf und was er hinterlaesst.
struct Fall
{
    const char* name;
    std::function<void(Lauf&)> ersterLauf;  ///< leer = es gab keinen
    bool schleife = false;
    bool sync = false;
    bool mitlaufen = true;
    bool oeffnenHaelt = false;
    bool dateiFehlt = false;
};

}  // namespace

TEST_SUITE("ComposerSichttestStart")
{
    TEST_CASE("S105 Vorbereiten: Load, Composer on, Edit, Fit fuehren aus jedem Startzustand zum selben Stand")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        const QString vorlageB = vorlage("composer_B_viele.lvcomp");
        const QString vorlageA = vorlage("composer_A_kurz.lvcomp");
        REQUIRE_MESSAGE(QFileInfo::exists(vorlageB), vorlageB.toStdString());
        REQUIRE(QFileInfo::exists(vorlageA));
        Komposition soll;
        REQUIRE(ladeKomposition(vorlageB, soll));
        soll.richte();
        soll.vergibFarben();  // wie `Sitzung::lade`

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString fluechtig = dir.filePath(QStringLiteral("weg.lvcomp"));

        const std::vector<Fall> faelle = {
            {"erster Start, nichts gemerkt", {}},
            {"dieselbe Vorlage lief, Resume aus", [&](Lauf& l) { REQUIRE(l.sitzung().oeffne(vorlageB)); }},
            {"andere Vorlage lief bei 0:25, Resume an",
             [&](Lauf& l) {
                 Sitzung::setResume(true);
                 REQUIRE(l.sitzung().oeffne(vorlageA));
                 l.sitzung().springe(25000);
             }},
            {"dieselbe Vorlage angehalten bei 0:12, Resume an, Edit an, hineingezoomt",
             [&](Lauf& l) {
                 Sitzung::setResume(true);
                 REQUIRE(l.sitzung().oeffne(vorlageB));
                 l.sitzung().pausiere();
                 l.sitzung().springe(12000);
                 l.haken("composerEdit")->setChecked(true);
                 l.panel.zeitachse()->zoomeZeit(3.0, 300);
             }},
            {"Composer aus, Stelle 0:40, Resume an",
             [&](Lauf& l) {
                 Sitzung::setResume(true);
                 REQUIRE(l.sitzung().lade(vorlageB));
                 l.sitzung().springe(40000);
             }},
            {"die gemerkte Datei gibt es nicht mehr",
             [&](Lauf& l) {
                 REQUIRE(QFile::copy(vorlageA, fluechtig));
                 Sitzung::setResume(true);
                 REQUIRE(l.sitzung().oeffne(fluechtig));
             },
             false, false, true, false, true},
            {"Zeilenhoehe 250 %, Loop an, Sync an, Scroll with playhead aus",
             [&](Lauf& l) {
                 REQUIRE(l.sitzung().oeffne(vorlageB));
                 l.panel.zeitachse()->setZeilenProzent(250);
                 l.sitzung().setSchleife(true);
                 l.sitzung().setSync(true);
                 Sitzung::setMitlaufenAn(false);
             },
             true, true, false},
            {"Einstellung: Open Composition haelt am Anfang",
             [&](Lauf& l) {
                 Sitzung::setOeffnenHaelt(true);
                 REQUIRE(l.sitzung().oeffne(vorlageB));
             },
             false, false, true, true},
            {"am Ende schaltet der Composer sich aus, Resume an, lief",
             [&](Lauf& l) {
                 Sitzung::setAmEnde(AmEnde::ComposerAus);
                 Sitzung::setResume(true);
                 REQUIRE(l.sitzung().oeffne(vorlageA));
                 l.sitzung().springe(50000);
             }},
        };

        for (const Weg weg : {Weg::Laden, Weg::Oeffnen})
        {
            const bool mitLauf = weg == Weg::Laden;
            CAPTURE(mitLauf);
            QStringList erster;       // der Stand nach Vorbereiten im ersten Fall
            QSet<QString> anfaenge;   // die wiederhergestellten Staende, vor Vorbereiten

            for (const Fall& fall : faelle)
            {
                const std::string fallName = fall.name;
                CAPTURE(fallName);
                QSettings().clear();
                QFile::remove(fluechtig);

                if (fall.ersterLauf)
                {
                    Lauf davor;
                    if (!davor.steht())
                    {
                        MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
                        return;
                    }
                    fall.ersterLauf(davor);
                    QCoreApplication::processEvents();
                    davor.sitzung().sichereStand();  // wie bei aboutToQuit
                }
                if (fall.dateiFehlt) REQUIRE(QFile::remove(fluechtig));

                // --- der Neustart: neues Fenster, neues Panel, Naht N4
                Lauf lauf;
                if (!lauf.steht())
                {
                    MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
                    return;
                }
                const bool wiederDa = stelleKompositionWiederHer(lauf.widget);
                QCoreApplication::processEvents();
                CHECK(wiederDa == (static_cast<bool>(fall.ersterLauf) && !fall.dateiFehlt));
                anfaenge.insert(
                    (kern(lauf, soll, false)
                     << QStringLiteral("laeuft=%1").arg(lauf.sitzung().laeuft())
                     << QStringLiteral("sekunde=%1").arg(lauf.sitzung().stelleMs() / 1000)
                     << QStringLiteral("loop=%1").arg(lauf.sitzung().schleife()))
                        .join(QLatin1Char('|')));

                // --- Vorbereiten
                REQUIRE(bereiteVor(lauf, vorlageB, weg, /*edit=*/false));
                const QStringList stand = kern(lauf, soll, mitLauf);
                if (erster.isEmpty()) erster = stand;
                CHECK_MESSAGE(stand == erster, stand.join(QStringLiteral(" | ")).toStdString());

                // was die vier Aktionen NICHT anfassen: es kommt aus den Einstellungen durch
                CHECK(lauf.sitzung().schleife() == fall.schleife);
                CHECK(lauf.sitzung().sync() == fall.sync);
                CHECK(lauf.haken("composerScrollWithPlayhead")->isChecked() == fall.mitlaufen);

                if (weg == Weg::Laden)
                {
                    // der Stand steht: angehalten am Anfang, auch eine halbe Sekunde spaeter
                    QElapsedTimer uhr;
                    uhr.start();
                    (void)warte([&] { return uhr.elapsed() > 500; });
                    CHECK(kern(lauf, soll, true) == erster);
                }
                else
                {
                    // der Weg von Open Composition spielt ab — ausser die Einstellung haelt
                    CHECK(lauf.sitzung().laeuft() == !fall.oeffnenHaelt);
                    if (!fall.oeffnenHaelt)
                        CHECK(warte([&] { return lauf.sitzung().stelleMs() > 300; }));
                }

                // --- in derselben Sitzung: ein Abschnitt davor hat alles verstellt
                {
                    Sitzung& s = lauf.sitzung();
                    lauf.haken("composerEdit")->setChecked(true);
                    Komposition anders = s.komposition();
                    anders.kanaele[0].clips[0].endeMs -= 5000;
                    s.setzeKomposition(anders);
                    CHECK(s.ungespeichert());
                    lauf.panel.zeitachse()->zoomeZeit(3.0, 300);
                    lauf.panel.zeitachse()->setZeilenProzent(200);
                    lauf.panel.zeitachse()->waehleGesamt();
                    s.spiele();
                    s.springe(30000);
                    QCoreApplication::processEvents();
                    CHECK(kern(lauf, soll, mitLauf) != erster);  // wirklich verstellt

                    REQUIRE(bereiteVor(lauf, vorlageB, weg, /*edit=*/false));
                    const QStringList wieder = kern(lauf, soll, mitLauf);
                    CHECK_MESSAGE(wieder == erster, wieder.join(QStringLiteral(" | ")).toStdString());
                }
            }

            // die Faelle haben wirklich verschiedene Anfaenge geliefert (zweimal
            // „nichts geladen", zweimal „dieselbe Vorlage, Composer aus, am Anfang")
            CHECK(anfaenge.size() == 7);
            MESSAGE("Weg " << std::string(weg == Weg::Laden ? "Sitzung::lade" : "starteKomposition") << ": "
                           << faelle.size() << " Faelle, " << anfaenge.size()
                           << " verschiedene Anfaenge, Stand nach Vorbereiten: "
                           << erster.join(QStringLiteral(" | ")).toStdString());
        }
    }
}
