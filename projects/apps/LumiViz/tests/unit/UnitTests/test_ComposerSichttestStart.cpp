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
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "composer/SichttestAktionen.hpp"
#include "composer/Sitzung.hpp"
#include "services/ServiceContainer.hpp"
#include "sichttest/Anbindung.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
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
        CHECK(steuerung->aktionen() ==
              QStringList{QStringLiteral("komposition_laden"), QStringLiteral("composer"),
                          QStringLiteral("edit"), QStringLiteral("fit")});
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
