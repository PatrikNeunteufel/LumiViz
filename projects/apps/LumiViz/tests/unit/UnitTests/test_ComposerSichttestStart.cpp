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
 * Zwei Wege stehen nebeneinander: die Aktionen der Sichttest-Steuerung selbst
 * (`composer/SichttestAktionen`; `komposition_laden` haelt am Anfang — Entscheid Patrik
 * L1: a) und zum Vergleich `starteKomposition` (der Weg von File → Open Composition, er
 * spielt sofort ab). Dazu: Argumente und Fehler der Aktionen, die Einstellung zum
 * Abspielen, Schalter `--testing`, Root und Dateien. Die Probe der DLL (laden, anmelden,
 * mit dem Tester verbinden) startet `Sichttest.exe` und laeuft deshalb nur mit
 * `LUMIVIZ_SICHTTEST_PROBE=<Ordner mit LumiViz.exe>`.
 *
 * Nicht erfasst: das Preset der Effektkette, die Playlist und die Wiederaufnahme des
 * Players — sie gehoeren dem Hauptfenster, das dieser Test nicht baut.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/panels/KaraokePanel.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "composer/SichttestAktionen.hpp"
#include "composer/Sitzung.hpp"
#include "PappTon.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/IPlaylist.hpp"
#include "audio/Playlist.hpp"
#include "services/EventBus.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "sichttest/Anbindung.hpp"
#include "sichttest/AppAktionen.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <DockManager.h>
#include <DockWidget.h>

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QMainWindow>
#include <QMutexLocker>
#include <QPushButton>
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
    Oeffnen,  ///< `starteKomposition` — wie File → Open Composition (zum Vergleich)
    Laden,    ///< die Aktionen der Sichttest-Steuerung — `komposition_laden` haelt am Anfang
};

/// Der Root des Projekts, wie ihn die Steuerung findet: ueber `.sichttest/sichttest.projekt.json`.
QString root()
{
    return lumi::sichttest::wurzel(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
}

QJsonObject wert(const char* text)
{
    return QJsonObject{{QStringLiteral("wert"), QString::fromLatin1(text)}};
}

/// So nennen die Listen die Vorlage: ab dem Root, mit den Trennzeichen von Windows.
const char* const kVorlageB = "asset\\sichttest\\composer_B_viele.lvcomp";

/// Die vier Aktionen in der Reihenfolge der Listen: Load..., Composer on, Edit, Fit.
bool bereiteVor(Lauf& lauf, const QString& datei, Weg weg, bool edit)
{
    using lumi::sichttest::Ergebnis;
    namespace aktion = lumi::composer::sichttestaktion;
    if (weg == Weg::Oeffnen)
    {
        QStringList bericht;
        if (!starteKomposition(lauf.widget, datei, &bericht) || !bericht.isEmpty()) return false;
        lauf.sitzung().setAn(true);
        lauf.haken("composerEdit")->setChecked(edit);
        lauf.panel.zeitachse()->setzeZoomZurueck();
        QCoreApplication::processEvents();
        return true;
    }
    const Ergebnis geladen = aktion::kompositionLaden(
        lauf.widget, root(), QJsonObject{{QStringLiteral("datei"), QString::fromLatin1(kVorlageB)}});
    if (geladen.status != Ergebnis::Status::Ok || !geladen.text.isEmpty()) return false;
    if (aktion::composer(lauf.widget, wert("an")).status != Ergebnis::Status::Ok) return false;
    if (aktion::edit(lauf.widget, wert(edit ? "an" : "aus")).status != Ergebnis::Status::Ok) return false;
    if (aktion::fit(lauf.widget).status != Ergebnis::Status::Ok) return false;
    QCoreApplication::processEvents();
    return true;
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
            MESSAGE("Weg " << std::string(weg == Weg::Laden ? "Aktionen der Steuerung" : "starteKomposition") << ": "
                           << faelle.size() << " Faelle, " << anfaenge.size()
                           << " verschiedene Anfaenge, Stand nach Vorbereiten: "
                           << erster.join(QStringLiteral(" | ")).toStdString());
        }
    }

    TEST_CASE("S105 Steuerung: Schalter --testing, Root des Projekts, Dateien und an|aus")
    {
        using namespace lumi::sichttest;
        CHECK(verlangt({QStringLiteral("LumiViz.exe"), QStringLiteral("--testing")}));
        CHECK(verlangt({QStringLiteral("LumiViz.exe"), QStringLiteral("x.lvcomp"), QStringLiteral("--testing")}));
        CHECK_FALSE(verlangt({QStringLiteral("LumiViz.exe")}));
        CHECK_FALSE(verlangt({QStringLiteral("--testing")}));  // nur der Programmname
        CHECK_FALSE(verlangt({QStringLiteral("LumiViz.exe"), QStringLiteral("--testing2")}));

        // der Root: der Ordner ueber `.sichttest` — von dieser Datei aus und
        // von dort, wo die Exe der Sichttests liegt (tief unter out/build)
        const QString hier = root();
        REQUIRE_FALSE(hier.isEmpty());
        CHECK(QFileInfo(QDir(hier).filePath(QStringLiteral(".sichttest/sichttest.projekt.json"))).isFile());
        CHECK(QFileInfo(QDir(hier).filePath(QStringLiteral("Solution.json"))).isFile());
        CHECK(wurzel(QDir(hier).filePath(QStringLiteral("asset/sichttest/presets"))) == hier);
        CHECK(wurzel(QString()).isEmpty());
        CHECK(wurzel(QDir::rootPath()).isEmpty());

        // Dateien: relativ ab dem Root, mit beiden Trennzeichen; absolut wie angegeben
        const QString soll = QDir(hier).filePath(QStringLiteral("asset/sichttest/composer_B_viele.lvcomp"));
        CHECK(loeseAuf(QString::fromLatin1(kVorlageB), hier) == soll);
        CHECK(loeseAuf(QStringLiteral("asset/sichttest/composer_B_viele.lvcomp"), hier) == soll);
        CHECK(loeseAuf(soll, QString()) == soll);
        CHECK(loeseAuf(QStringLiteral("asset/sichttest"), hier).isEmpty());        // ein Ordner
        CHECK(loeseAuf(QStringLiteral("asset/sichttest/gibt_es_nicht.lvcomp"), hier).isEmpty());
        CHECK(loeseAuf(QString::fromLatin1(kVorlageB), QString()).isEmpty());      // relativ ohne Root
        CHECK(loeseAuf(QStringLiteral("  "), hier).isEmpty());

        CHECK(anOderAus(wert("an")) == std::optional<bool>(true));
        CHECK(anOderAus(wert("aus")) == std::optional<bool>(false));
        CHECK(anOderAus(wert(" AN ")) == std::optional<bool>(true));
        CHECK_FALSE(anOderAus(wert("vielleicht")).has_value());
        CHECK_FALSE(anOderAus(QJsonObject{}).has_value());

        // Zeiten, wie die Listen sie schreiben
        CHECK(zeitMs(QStringLiteral("0:20")) == std::optional<qint64>(20000));
        CHECK(zeitMs(QStringLiteral("1:23.4")) == std::optional<qint64>(83400));
        CHECK(zeitMs(QStringLiteral(" 14:40 ")) == std::optional<qint64>(880000));
        CHECK(zeitMs(QStringLiteral("20")) == std::optional<qint64>(20000));
        CHECK(zeitMs(QStringLiteral("0.5")) == std::optional<qint64>(500));
        CHECK_FALSE(zeitMs(QStringLiteral("0:60")).has_value());
        CHECK_FALSE(zeitMs(QStringLiteral("-1")).has_value());
        CHECK_FALSE(zeitMs(QStringLiteral("bald")).has_value());
        CHECK_FALSE(zeitMs(QString()).has_value());

        // ein Argument unter seinem Schluessel — oder als einzelner Wert
        CHECK(argument(QJsonObject{{QStringLiteral("zeit"), QStringLiteral("0:20")}}, QStringLiteral("zeit")) ==
              QStringLiteral("0:20"));
        CHECK(argument(wert("0:20"), QStringLiteral("zeit")) == QStringLiteral("0:20"));
        CHECK(argument(QJsonObject{}, QStringLiteral("zeit")).isEmpty());

        // ohne die DLL im Ordner gibt es keine Steuerung — und einen Satz dazu
        QTemporaryDir leer;
        REQUIRE(leer.isValid());
        QString grund;
        CHECK(Anbindung::lade(QStringLiteral("0.0.0"), &grund, leer.path()) == nullptr);
        CHECK_FALSE(grund.isEmpty());
    }

    TEST_CASE("S105 Aktionen des Composers: Argumente, Fehler und die Einstellung zum Abspielen")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        using lumi::sichttest::Ergebnis;
        namespace aktion = lumi::composer::sichttestaktion;

        Lauf lauf;
        if (!lauf.steht())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        Sitzung& s = lauf.sitzung();
        CHECK(aktion::panelFuer(lauf.widget) == &lauf.panel);

        // --- nichts geladen
        CHECK(aktion::composer(lauf.widget, wert("an")).status == Ergebnis::Status::Fehler);
        CHECK_FALSE(s.an());
        CHECK(aktion::composer(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK(aktion::composer(lauf.widget, wert("ein")).status == Ergebnis::Status::Ungueltig);
        CHECK(aktion::edit(lauf.widget, QJsonObject{}).status == Ergebnis::Status::Ungueltig);

        // --- komposition_laden: die Datei fehlt in den Argumenten, gibt es nicht, ist keine Komposition
        CHECK(aktion::kompositionLaden(lauf.widget, root(), QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        const Ergebnis fehlt = aktion::kompositionLaden(
            lauf.widget, root(), QJsonObject{{QStringLiteral("datei"), QStringLiteral("asset/nirgends.lvcomp")}});
        CHECK(fehlt.status == Ergebnis::Status::Ungueltig);
        CHECK(fehlt.text.contains(QStringLiteral("nirgends.lvcomp")));
        CHECK(aktion::kompositionLaden(lauf.widget, root(),
                                       QJsonObject{{QStringLiteral("datei"), QStringLiteral("Solution.json")}})
                  .status == Ergebnis::Status::Fehler);
        CHECK_FALSE(s.geladen());

        // --- die Vorgabe (Entscheid Patrik L1: a): geladen, Composer aus, am Anfang, nichts laeuft
        CHECK_FALSE(Sitzung::sichttestSpieltNachLaden());
        const QJsonObject vorlage{{QStringLiteral("datei"), QString::fromLatin1(kVorlageB)}};
        const Ergebnis geladen = aktion::kompositionLaden(lauf.widget, root(), vorlage);
        CHECK(geladen.status == Ergebnis::Status::Ok);
        CHECK(geladen.text.isEmpty());
        CHECK(s.geladen());
        CHECK_FALSE(s.an());
        CHECK_FALSE(s.laeuft());
        CHECK(s.stelleMs() == 0);

        // --- die Schalter
        CHECK(aktion::composer(lauf.widget, wert("an")).status == Ergebnis::Status::Ok);
        CHECK(s.an());
        CHECK_FALSE(s.laeuft());
        CHECK(aktion::edit(lauf.widget, wert("an")).status == Ergebnis::Status::Ok);
        CHECK(lauf.panel.bearbeiten());
        CHECK(lauf.panel.zeitachse()->bearbeiten());
        CHECK(aktion::edit(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(lauf.panel.zeitachse()->bearbeiten());
        lauf.panel.zeitachse()->zoomeZeit(3.0, 300);
        lauf.panel.zeitachse()->setZeilenProzent(200);
        CHECK(lauf.panel.zeitachse()->zoomVerstellt());
        CHECK(aktion::fit(lauf.widget).status == Ergebnis::Status::Ok);
        CHECK_FALSE(lauf.panel.zeitachse()->zoomVerstellt());
        CHECK(aktion::composer(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.an());

        // --- Abspielmarke: auch bei Composer aus, auf die Laenge begrenzt (Vorlage B: 6:00)
        const auto zeit = [](const char* text) {
            return QJsonObject{{QStringLiteral("zeit"), QString::fromLatin1(text)}};
        };
        CHECK(aktion::abspielmarke(lauf.widget, zeit("0:20")).status == Ergebnis::Status::Ok);
        CHECK(s.stelleMs() == 20000);
        CHECK(aktion::abspielmarke(lauf.widget, wert("2:00.5")).status == Ergebnis::Status::Ok);
        CHECK(s.stelleMs() == 120500);
        const Ergebnis zuWeit = aktion::abspielmarke(lauf.widget, zeit("9:00"));
        CHECK(zuWeit.status == Ergebnis::Status::Ok);
        CHECK_FALSE(zuWeit.text.isEmpty());
        CHECK(s.stelleMs() == 360000);
        CHECK(aktion::abspielmarke(lauf.widget, zeit("bald")).status == Ergebnis::Status::Ungueltig);
        CHECK(aktion::abspielmarke(lauf.widget, QJsonObject{}).status == Ergebnis::Status::Ungueltig);

        // --- Transport: start schaltet den Composer ein, pause haelt, stopp geht an den Anfang
        CHECK(aktion::abspielmarke(lauf.widget, zeit("0:30")).status == Ergebnis::Status::Ok);
        CHECK(aktion::wiedergabe(lauf.widget, wert("start")).status == Ergebnis::Status::Ok);
        CHECK(s.an());
        CHECK(s.laeuft());
        CHECK(warte([&] { return s.stelleMs() > 30300; }));
        CHECK(aktion::wiedergabe(lauf.widget, wert("pause")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.laeuft());
        CHECK(s.stelleMs() > 30000);
        CHECK(aktion::wiedergabe(lauf.widget, wert("stopp")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.laeuft());
        CHECK(s.stelleMs() == 0);
        CHECK(aktion::wiedergabe(lauf.widget, wert("weiter")).status == Ergebnis::Status::Ungueltig);

        // --- Loop und Sync on Effect Chain: die Sitzung und die Schalter des Panels
        CHECK(aktion::schleife(lauf.widget, wert("an")).status == Ergebnis::Status::Ok);
        CHECK(s.schleife());
        CHECK(lauf.panel.findChild<QPushButton*>(QStringLiteral("composerLoop"))->isChecked());
        CHECK(aktion::schleife(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.schleife());
        CHECK(aktion::schleife(lauf.widget, QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        CHECK(aktion::syncKette(lauf.widget, wert("an")).status == Ergebnis::Status::Ok);
        CHECK(s.sync());
        CHECK(lauf.haken("composerSync")->isChecked());
        CHECK(aktion::syncKette(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.sync());
        CHECK_FALSE(lauf.haken("composerSync")->isChecked());
        CHECK(aktion::composer(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);

        // --- schliessen: ohne Rueckfrage, auch mit Ungespeichertem; ein zweites Mal ist auch erledigt
        {
            Komposition anders = s.komposition();
            anders.kanaele[0].clips[0].endeMs -= 5000;
            s.setzeKomposition(anders);
            CHECK(s.ungespeichert());
        }
        CHECK(aktion::kompositionSchliessen(lauf.widget).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.geladen());
        CHECK(aktion::kompositionSchliessen(lauf.widget).status == Ergebnis::Status::Ok);
        CHECK(aktion::abspielmarke(lauf.widget, zeit("0:20")).status == Ergebnis::Status::Fehler);
        CHECK(aktion::wiedergabe(lauf.widget, wert("start")).status == Ergebnis::Status::Fehler);

        // --- ein fehlendes Preset: geladen, die Bemerkung kommt als Text der Antwort
        const Ergebnis bemerkt = aktion::kompositionLaden(
            lauf.widget, root(),
            QJsonObject{{QStringLiteral("wert"), QStringLiteral("asset/sichttest/fehlt_ein_preset.lvcomp")}});
        CHECK(bemerkt.status == Ergebnis::Status::Ok);
        CHECK_FALSE(bemerkt.text.isEmpty());

        // --- die Einstellung an: nach dem Laden Composer on, und es laeuft
        Sitzung::setSichttestSpieltNachLaden(true);
        CHECK(aktion::kompositionLaden(lauf.widget, root(), vorlage).status == Ergebnis::Status::Ok);
        CHECK(s.an());
        CHECK(s.laeuft());
        CHECK(warte([&] { return s.stelleMs() > 300; }));
        // und wieder aus: dieselbe Aktion haelt am Anfang, auch aus dem Lauf heraus
        Sitzung::setSichttestSpieltNachLaden(false);
        CHECK(aktion::kompositionLaden(lauf.widget, root(), vorlage).status == Ergebnis::Status::Ok);
        CHECK_FALSE(s.an());
        CHECK(s.stelleMs() == 0);
    }

    TEST_CASE("S105 Aktionen der App: Panel, Karaoke, Playlist und Player")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        using lumi::sichttest::Ergebnis;
        namespace aktion = lumi::sichttest::appaktion;
        const auto mit = [](const char* schluessel, const QString& text) {
            return QJsonObject{{QString::fromLatin1(schluessel), text}};
        };

        Lauf lauf;
        if (!lauf.steht())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        // --- Panel: der Name fehlt, gibt es nicht — und das Panel dieses Tests
        //     ist ein eigenes Fenster, kein Dock (im Programm sitzt jedes in einem)
        CHECK(aktion::panelZeigen(QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        const Ergebnis keines = aktion::panelZeigen(mit("name", QStringLiteral("gibt es nicht")));
        CHECK(keines.status == Ergebnis::Status::Fehler);
        CHECK(keines.text.contains(lauf.panel.panelId()));  // die Antwort nennt, was es gibt
        CHECK(aktion::panelZeigen(mit("name", lauf.panel.panelId().toUpper())).status == Ergebnis::Status::Fehler);
        CHECK(aktion::panelZeigen(mit("name", lauf.panel.panelTitle())).text.contains(QStringLiteral("not docked")));

        // --- Karaoke on: der Schalter der App
        const bool vorher = MultiEffectVisualizer::karaokeEbene();
        CHECK(aktion::karaoke(lauf.widget, wert("an")).status == Ergebnis::Status::Ok);
        CHECK(MultiEffectVisualizer::karaokeEbene());
        CHECK(aktion::karaoke(lauf.widget, wert("aus")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(MultiEffectVisualizer::karaokeEbene());
        CHECK(aktion::karaoke(lauf.widget, QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        MultiEffectVisualizer::setKaraokeEbene(vorher);

        // --- karaoke_laden: die Datei fehlt; und das Fenster dieses Tests zeigt nicht „Multi Effect"
        CHECK(aktion::karaokeLaden(lauf.widget, root(), QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        CHECK(aktion::karaokeLaden(lauf.widget, root(), mit("datei", QStringLiteral("asset/nirgends.lvkar"))).status ==
              Ergebnis::Status::Ungueltig);
        if (dynamic_cast<MultiEffectVisualizer*>(lauf.widget.visualizer()) == nullptr)
            CHECK(aktion::karaokeLaden(lauf.widget, root(), mit("datei", QStringLiteral("Solution.json"))).status ==
                  Ergebnis::Status::Fehler);

        // --- Playlist und Player: ohne die Dienste sagt die Aktion es
        const QString song = QStringLiteral("asset\\sichttest\\audio\\07_kick_60bpm_30s.mp3");
        CHECK(aktion::titelLaden(lauf.widget, root(), mit("datei", song)).status == Ergebnis::Status::Fehler);
        CHECK(aktion::player(lauf.widget, wert("start")).status == Ergebnis::Status::Fehler);
        const QString kette = QStringLiteral("asset\\sichttest\\presets\\Stern.lvfx");
        CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", kette)).status == Ergebnis::Status::Fehler);

        lauf.dienste.registerInstance<IEventBus>(std::make_unique<EventBus>());

        // --- preset_laden: jede Endung ueber ihren Ladeweg, und immer „schon gefragt" (Naht N24)
        {
            IEventBus* bus = lauf.dienste.tryResolve<IEventBus>();
            QStringList gemeldet;
            const auto a1 = bus->subscribeScoped<LoadEffectChainEvent>([&](const LoadEffectChainEvent& e) {
                gemeldet << QStringLiteral("kette %1 %2").arg(e.confirmed).arg(QString::fromStdString(e.path));
            });
            const auto a2 = bus->subscribeScoped<ImportAvsPresetEvent>([&](const ImportAvsPresetEvent& e) {
                gemeldet << QStringLiteral("avs %1 %2").arg(e.confirmed).arg(QString::fromStdString(e.path));
            });
            const auto a3 = bus->subscribeScoped<ImportMilkPresetEvent>([&](const ImportMilkPresetEvent& e) {
                gemeldet << QStringLiteral("milk %1 %2").arg(e.confirmed).arg(QString::fromStdString(e.path));
            });
            // die Vorgabe des Ereignisses bleibt „fragen": alle bestehenden Absender fragen wie bisher
            CHECK_FALSE(LoadEffectChainEvent{}.confirmed);
            CHECK_FALSE(LoadEffectChainEvent{std::string("x.lvfx")}.confirmed);
            CHECK(LoadEffectChainEvent{std::string("x.lvfx"), true}.confirmed);

            CHECK(aktion::presetLaden(lauf.widget, root(), QJsonObject{}).status == Ergebnis::Status::Ungueltig);
            CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", QStringLiteral("asset/nirgends.lvfx"))).status ==
                  Ergebnis::Status::Ungueltig);
            CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", QStringLiteral("Solution.json"))).status ==
                  Ergebnis::Status::Ungueltig);
            CHECK(gemeldet.isEmpty());

            CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", kette)).status == Ergebnis::Status::Ok);
            REQUIRE(gemeldet.size() == 1);
            CHECK(gemeldet.last() ==
                  QStringLiteral("kette 1 ") + QDir(root()).filePath(QStringLiteral("asset/sichttest/presets/Stern.lvfx")));
            const QString milk = QStringLiteral("asset/Milkdrop3/presets/Blank.milk");
            CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", milk)).status == Ergebnis::Status::Ok);
            REQUIRE(gemeldet.size() == 2);
            CHECK(gemeldet.last() == QStringLiteral("milk 1 ") + QDir(root()).filePath(milk));
            const QString avs = QStringLiteral("asset/calibration/avs/matrix/00_simple/01_default.avs");
            CHECK(aktion::presetLaden(lauf.widget, root(), mit("datei", avs)).status == Ergebnis::Status::Ok);
            REQUIRE(gemeldet.size() == 3);
            CHECK(gemeldet.last() == QStringLiteral("avs 1 ") + QDir(root()).filePath(avs));
        }
        {
            auto liste = std::make_unique<Playlist>(*lauf.dienste.tryResolve<IEventBus>());
            lauf.dienste.registerInstance<IPlaylist>(std::move(liste));
        }
        lauf.dienste.registerInstance<IAudioPlayer>(std::make_unique<papp::PappPlayer>());
        auto* playlist = lauf.dienste.tryResolve<IPlaylist>();
        auto* spieler = dynamic_cast<papp::PappPlayer*>(lauf.dienste.tryResolve<IAudioPlayer>());
        REQUIRE(playlist != nullptr);
        REQUIRE(spieler != nullptr);

        CHECK(aktion::titelLaden(lauf.widget, root(), QJsonObject{}).status == Ergebnis::Status::Ungueltig);
        CHECK(aktion::titelLaden(lauf.widget, root(), mit("datei", QStringLiteral("asset/nirgends.mp3"))).status ==
              Ergebnis::Status::Ungueltig);
        CHECK(playlist->count() == 0);
        CHECK(aktion::titelLaden(lauf.widget, root(), mit("datei", song)).status == Ergebnis::Status::Ok);
        CHECK(playlist->count() == 1);
        // wiederholbar: derselbe Titel steht danach nicht zweimal in der Playlist
        CHECK(aktion::titelLaden(lauf.widget, root(), mit("datei", song)).status == Ergebnis::Status::Ok);
        CHECK(playlist->count() == 1);
        QJsonObject spielen = mit("datei", song);
        spielen.insert(QStringLiteral("spielen"), QStringLiteral("vielleicht"));
        CHECK(aktion::titelLaden(lauf.widget, root(), spielen).status == Ergebnis::Status::Ungueltig);
        // der Player aus Pappe spielt keinen Platz der Playlist: die Aktion meldet es
        spielen.insert(QStringLiteral("spielen"), QStringLiteral("ja"));
        CHECK(aktion::titelLaden(lauf.widget, root(), spielen).status == Ergebnis::Status::Fehler);

        // --- Transport des Players
        CHECK(aktion::player(lauf.widget, wert("start")).status == Ergebnis::Status::Fehler);  // kein Titel
        spieler->laengen.insert(QStringLiteral("x.mp3"), 30000);
        REQUIRE(spieler->load(QStringLiteral("x.mp3")));
        CHECK(aktion::player(lauf.widget, wert("start")).status == Ergebnis::Status::Ok);
        CHECK(spieler->isPlaying());
        CHECK(aktion::player(lauf.widget, wert("pause")).status == Ergebnis::Status::Ok);
        CHECK(spieler->isPaused());
        CHECK(aktion::player(lauf.widget, wert("stopp")).status == Ergebnis::Status::Ok);
        CHECK(spieler->isStopped());
        CHECK(aktion::player(lauf.widget, wert("lauter")).status == Ergebnis::Status::Ungueltig);
    }

    TEST_CASE("S105 panel_zeigen: ein geschlossenes Panel in seinem Dock kommt wieder — ueber Kennung und Titel")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        using lumi::sichttest::Ergebnis;
        namespace aktion = lumi::sichttest::appaktion;

        // Wie im Programm (PanelManager::createDockWidget): das Panel sitzt in
        // einem Dock des Hauptfensters; ein Panel mit eigenen Rollflaechen ohne
        // die von Qt-ADS, ein anderes mit ihr (dann liegt das Dock zwei Eltern hoeher).
        ServiceContainer dienste;
        QMainWindow haupt;
        testfenster::platziere(haupt);
        haupt.resize(900, 500);
        auto* docks = new ads::CDockManager(&haupt);
        auto* composer = new ComposerPanel(dienste);
        ads::CDockWidget* dockComposer = docks->createDockWidget(composer->panelTitle());
        dockComposer->setWidget(composer, ads::CDockWidget::ForceNoScrollArea);
        docks->addDockWidget(ads::BottomDockWidgetArea, dockComposer);
        auto* karaoke = new KaraokePanel(dienste);
        ads::CDockWidget* dockKaraoke = docks->createDockWidget(karaoke->panelTitle());
        dockKaraoke->setWidget(karaoke, ads::CDockWidget::ForceScrollArea);
        docks->addDockWidget(ads::RightDockWidgetArea, dockKaraoke);
        haupt.show();
        if (!warte([&] { return haupt.isVisible(); }))
        {
            MESSAGE("Kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        const auto mit = [](const QString& name) { return QJsonObject{{QStringLiteral("name"), name}}; };

        // --- geschlossen (wie View → Panels abgehakt): die Aktion oeffnet es wieder
        dockComposer->toggleView(false);
        dockKaraoke->toggleView(false);
        REQUIRE(dockComposer->isClosed());
        REQUIRE(dockKaraoke->isClosed());
        CHECK(aktion::panelZeigen(mit(composer->panelId())).status == Ergebnis::Status::Ok);
        CHECK_FALSE(dockComposer->isClosed());
        CHECK(dockKaraoke->isClosed());  // nur das genannte
        // ueber den Titel, wie er im Programm steht, Gross und klein egal — und unter der Rollflaeche
        CHECK(aktion::panelZeigen(mit(karaoke->panelTitle().toUpper())).status == Ergebnis::Status::Ok);
        CHECK_FALSE(dockKaraoke->isClosed());
        CHECK(warte([&] { return karaoke->isVisible() && composer->isVisible(); }));
        // ein offenes Panel bleibt offen (die Vorbereitung ist wiederholbar)
        CHECK(aktion::panelZeigen(mit(composer->panelTitle())).status == Ergebnis::Status::Ok);
        CHECK_FALSE(dockComposer->isClosed());
        // ein einzelner Wert ohne Schluessel
        dockComposer->toggleView(false);
        CHECK(aktion::panelZeigen(wert("composer")).status == Ergebnis::Status::Ok);
        CHECK_FALSE(dockComposer->isClosed());
        // was es nicht gibt, nennt die bekannten Kennungen
        const Ergebnis keines = aktion::panelZeigen(mit(QStringLiteral("Effect Chain")));
        CHECK(keines.status == Ergebnis::Status::Fehler);
        CHECK(keines.text.contains(QStringLiteral("composer")));
        CHECK(keines.text.contains(QStringLiteral("karaoke")));
    }

    TEST_CASE("S105 karaoke_laden: ein Karaoke-Preset kommt ohne Dialog in die Ebene und schaltet sie ein")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        using lumi::sichttest::Ergebnis;
        namespace aktion = lumi::sichttest::appaktion;

        // Die Ablage der laufenden Ebene in einen eigenen Ordner — nie die der App
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString ablageVorher = MultiEffectVisualizer::karaokeAblage();
        const bool ebeneVorher = MultiEffectVisualizer::karaokeEbene();
        const QString ablage = dir.filePath(QStringLiteral("laufend.lvkar"));
        MultiEffectVisualizer::setKaraokeAblage(ablage);
        MultiEffectVisualizer::setKaraokeEbene(false);
        struct Zurueck
        {
            QString ablage;
            bool ebene;
            ~Zurueck()
            {
                MultiEffectVisualizer::setKaraokeAblage(ablage);
                MultiEffectVisualizer::setKaraokeEbene(ebene);
            }
        } zurueck{ablageVorher, ebeneVorher};

        Lauf lauf;
        if (!lauf.steht())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        REQUIRE(lauf.widget.setVisualizer(QStringLiteral("multieffect")));
        auto* host = dynamic_cast<MultiEffectVisualizer*>(lauf.widget.visualizer());
        REQUIRE(host != nullptr);
        const auto knoten = [&] {
            QMutexLocker halt(&lauf.widget.renderMutex());
            return host->karaokeKette().children.size();
        };
        const std::size_t leer = knoten();

        const QJsonObject preset{{QStringLiteral("datei"), QStringLiteral("asset\\karaoke\\Karaoke.lvkar")}};
        REQUIRE(QFileInfo(QDir(root()).filePath(QStringLiteral("asset/karaoke/Karaoke.lvkar"))).isFile());
        const Ergebnis geladen = aktion::karaokeLaden(lauf.widget, root(), preset);
        CHECK_MESSAGE(geladen.status == Ergebnis::Status::Ok, geladen.text.toStdString());
        CAPTURE(leer);
        CHECK(knoten() > 0);                              // die Ebene traegt jetzt das Preset
        CHECK(MultiEffectVisualizer::karaokeEbene());     // und ist eingeschaltet
        CHECK(QFileInfo::exists(ablage));                 // die Ablage ist die des Tests
        // wiederholbar: ein zweites Mal ist auch erledigt, die Ebene bleibt an
        CHECK(aktion::karaokeLaden(lauf.widget, root(), preset).status == Ergebnis::Status::Ok);
        CHECK(MultiEffectVisualizer::karaokeEbene());
        // eine Datei, die kein Karaoke-Preset ist: gescheitert, mit dem Namen in der Antwort
        const Ergebnis falsch = aktion::karaokeLaden(
            lauf.widget, root(), QJsonObject{{QStringLiteral("datei"), QStringLiteral("Solution.json")}});
        CHECK(falsch.status == Ergebnis::Status::Fehler);
        CHECK(falsch.text.contains(QStringLiteral("Solution.json")));
    }

    TEST_CASE("S105 Steuerung: die DLL laden, Aktionen anmelden, mit dem Tester verbinden (nur auf Wunsch)")
    {
        // Startet den Tester `Sichttest.exe` (ein Fenster) — deshalb nur mit
        // LUMIVIZ_SICHTTEST_PROBE=<Ordner mit LumiViz.exe und SichttestSteuerung1.dll>.
        const QString ordner = qEnvironmentVariable("LUMIVIZ_SICHTTEST_PROBE");
        if (ordner.isEmpty())
        {
            MESSAGE("LUMIVIZ_SICHTTEST_PROBE nicht gesetzt — die Probe der DLL laeuft nicht");
            return;
        }
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        Lauf lauf;
        REQUIRE(lauf.steht());
        QString grund;
        std::unique_ptr<lumi::sichttest::Anbindung> steuerung =
            lumi::sichttest::Anbindung::lade(QStringLiteral("0.0.0-probe"), &grund, ordner);
        REQUIRE_MESSAGE(steuerung != nullptr, grund.toStdString());
        CHECK(steuerung->root() == root());
        // die DLL nennt ihre Fassung (Koepfe ab Paket v0.3.1); die Schnittstelle ist S 1
        MESSAGE("DLL: " << steuerung->dllFassung().toStdString());
        CHECK(steuerung->dllFassung().contains(QStringLiteral("interface S 1.")));
        CHECK_FALSE(steuerung->dllFassung().startsWith(QLatin1Char(' ')));  // das Produkt ist nicht leer
        CHECK(steuerung->zustand() == lumi::sichttest::Anbindung::Zustand::Getrennt);

        lumi::composer::meldeSichttestAktionen(*steuerung, lauf.widget);
        lumi::sichttest::meldeAppAktionen(*steuerung, lauf.widget);
        CHECK(steuerung->aktionen() ==
              QStringList{QStringLiteral("komposition_laden"), QStringLiteral("composer"),
                          QStringLiteral("edit"), QStringLiteral("fit"), QStringLiteral("abspielmarke"),
                          QStringLiteral("wiedergabe"), QStringLiteral("schleife"),
                          QStringLiteral("sync_kette"), QStringLiteral("komposition_schliessen"),
                          QStringLiteral("panel_zeigen"), QStringLiteral("preset_laden"),
                          QStringLiteral("karaoke"),
                          QStringLiteral("karaoke_laden"), QStringLiteral("titel_laden"),
                          QStringLiteral("player")});
        // derselbe Name ein zweites Mal wird abgelehnt
        CHECK_FALSE(steuerung->aktion(QStringLiteral("fit"), QStringLiteral("x"), QString(),
                                      [](const QJsonObject&) { return lumi::sichttest::ok(); }));
        // ein Name ausserhalb von [a-z0-9_.] auch
        CHECK_FALSE(steuerung->aktion(QStringLiteral("Fit Now"), QStringLiteral("x"), QString(),
                                      [](const QJsonObject&) { return lumi::sichttest::ok(); }));

        QElapsedTimer uhr;
        uhr.start();
        CHECK(steuerung->verbinde());
        const bool verbunden = warte(
            [&] { return steuerung->zustand() == lumi::sichttest::Anbindung::Zustand::Verbunden; }, 30000);
        MESSAGE("Zustand nach " << uhr.elapsed() << " ms: " << static_cast<int>(steuerung->zustand())
                                << " (2 = verbunden), letzter Fehler: "
                                << steuerung->letzterFehler().toStdString());
        CHECK(verbunden);
        // eine Weile verbunden bleiben, dann sauber schliessen
        uhr.restart();
        (void)warte([&] { return uhr.elapsed() > 1500; });
        CHECK(steuerung->zustand() == lumi::sichttest::Anbindung::Zustand::Verbunden);
        uhr.restart();
        steuerung.reset();
        MESSAGE("Schliessen dauerte " << uhr.elapsed() << " ms");
        CHECK(uhr.elapsed() < 3000);
    }
}
