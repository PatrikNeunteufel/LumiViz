/**
 ****************************************************************************************
 * @file   test_ComposerSitzung.cpp
 * @brief  Composer, Stufe C2b: die Sitzung — geladen ist nicht an, Transport,
 *         das Ende, der Stand ueber den Neustart
 *         (docs/ui/Composer_Konzept.md §7, §8)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Alles ueber die echte Fassade (`VisualizerWidget` mit Render-Thread und
 * Fenster): nur so zeigt sich, ob das Bild wirklich von der Komposition bzw.
 * wieder von der Kette kommt. Die Einstellungen liegen in einem eigenen
 * QSettings-Bereich und werden am Ende geloescht.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Komposition.hpp"
#include "composer/Sitzung.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QMutexLocker>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <functional>

using namespace lumi::multieffect;
using namespace lumi::composer;

namespace
{

/// Eigener QSettings-Bereich fuer die Dauer des Tests, danach leer.
struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerSitzung"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

QString farbPreset(const QTemporaryDir& dir, const QString& name, uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    root.children.push_back(std::move(clear));
    const QString pfad = dir.filePath(name);
    REQUIRE(saveChainToFile(root, pfad));
    return pfad;
}

QString kompositionsDatei(const QTemporaryDir& dir, const QString& name, const QString& preset,
                          std::int64_t laengeMs)
{
    VisualClip c;
    c.datei = preset;
    c.startMs = 0;
    c.endeMs = laengeMs;
    Kanal k;
    k.art = KanalArt::Visual;
    k.name = QStringLiteral("CH1");
    k.visual = {c};
    Komposition komposition;
    komposition.kanaele.push_back(k);
    const QString pfad = dir.filePath(name);
    REQUIRE(speichereKomposition(komposition, pfad));
    return pfad;
}

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

}  // namespace

TEST_SUITE("ComposerSitzung")
{
    TEST_CASE("C2b Sitzung: geladen ist nicht an — Composer on, Transport, Ende und Neustart ueber die Fassade")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString lang = kompositionsDatei(dir, QStringLiteral("lang.lvcomp"), rot, 60000);
        const QString kurz = kompositionsDatei(dir, QStringLiteral("kurz.lvcomp"), rot, 300);

        ServiceContainer dienste;
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();

        QImage aufnahme;
        int aufnahmen = 0;
        QObject::connect(&widget, &VisualizerWidget::frameCaptured, &app,
                         [&](const QImage& bild) {
                             aufnahme = bild;
                             ++aufnahmen;
                         });
        const auto nimmAuf = [&]() {
            const int vorher = aufnahmen;
            widget.requestScreenshot();
            return warte([&] { return aufnahmen > vorher; });
        };
        const auto mitte = [&]() {
            return aufnahme.pixelColor(aufnahme.width() / 2, aufnahme.height() / 2);
        };
        // sicher ein Bild NACH dem letzten Eingriff
        const auto bildJetzt = [&]() {
            REQUIRE(nimmAuf());
            REQUIRE(nimmAuf());
            return mitte();
        };
        if (!widget.hasVisualizer() || !nimmAuf())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        const QColor kRot(255, 0, 0);

        CHECK(Sitzung::finde(widget) == nullptr);
        Sitzung& sitzung = Sitzung::fuer(widget);
        CHECK(Sitzung::finde(widget) == &sitzung);
        CHECK(&Sitzung::fuer(widget) == &sitzung);  // eine je Fenster
        int aenderungen = 0;
        QObject::connect(&sitzung, &Sitzung::geaendert, &app, [&] { ++aenderungen; });

        SUBCASE("geladen ist nicht an; Composer on zeigt die gemerkte Stelle, angehalten")
        {
            // ohne Komposition tut nichts etwas
            sitzung.setAn(true);
            sitzung.spiele();
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(sitzung.geladen());
            CHECK(sitzung.laengeMs() == 0);

            REQUIRE(sitzung.lade(lang));
            CHECK(aenderungen >= 1);
            CHECK(sitzung.geladen());
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(kompositionLaeuft(widget));
            CHECK(sitzung.laengeMs() == 60000);
            CHECK(sitzung.komposition().kanaele.size() == 1);
            CHECK(bildJetzt() != kRot);  // die Kette liefert das Bild

            // vorbereiten bei ausgeschaltetem Composer: die Stelle wird gemerkt
            sitzung.springe(20000);
            CHECK(sitzung.stelleMs() == 20000);
            sitzung.springe(999999);
            CHECK(sitzung.stelleMs() == 60000);  // geklemmt
            sitzung.springe(20000);

            sitzung.setAn(true);
            REQUIRE(sitzung.an());
            CHECK(kompositionLaeuft(widget));
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 20000);
            CHECK(bildJetzt() == kRot);
            CHECK(sitzung.stelleMs() == 20000);  // steht

            // Play, Pause, Sprung im Lauf, Stopp
            sitzung.spiele();
            CHECK(sitzung.laeuft());  // sofort, nicht erst nach dem naechsten Bild
            REQUIRE(warte([&] { return sitzung.stelleMs() > 20050; }));
            sitzung.pausiere();
            CHECK_FALSE(sitzung.laeuft());
            const std::int64_t gehalten = sitzung.stelleMs();
            REQUIRE(nimmAuf());
            CHECK(sitzung.stelleMs() == gehalten);
            sitzung.spiele();
            sitzung.springe(40000);
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 40000);
            sitzung.stoppe();
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(sitzung.an());  // Stopp schaltet nicht aus

            // Composer aus: die Kette liefert wieder das Bild, die Stelle bleibt
            sitzung.springe(31000);
            sitzung.setAn(false);
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.geladen());
            CHECK(sitzung.stelleMs() == 31000);
            CHECK(bildJetzt() != kRot);

            // Play bei ausgeschaltetem Composer schaltet ihn ein
            sitzung.spiele();
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 31000);

            sitzung.entlade();
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(sitzung.geladen());
            CHECK(sitzung.datei().isEmpty());
            sitzung.entlade();  // zweimal schadet nicht
        }

        SUBCASE("Oeffnen von Hand: spielt sofort — oder haelt am Anfang (Einstellung)")
        {
            CHECK_FALSE(Sitzung::oeffnenHaelt());
            REQUIRE(sitzung.oeffne(lang));
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());

            Sitzung::setOeffnenHaelt(true);
            REQUIRE(sitzung.oeffne(lang));
            CHECK(sitzung.an());
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() == kRot);

            // keine Komposition: es bleibt alles, wie es war
            CHECK_FALSE(sitzung.oeffne(rot));
            CHECK(sitzung.an());
            CHECK(sitzung.datei() == QFileInfo(lang).absoluteFilePath());
        }

        SUBCASE("das Ende: halten, an den Anfang, Composer aus — und die Schleife")
        {
            CHECK(Sitzung::amEnde() == AmEnde::Halten);
            REQUIRE(sitzung.oeffne(kurz));
            REQUIRE(warte([&] { return !sitzung.laeuft(); }));
            CHECK(sitzung.an());
            CHECK(sitzung.stelleMs() == 300);
            REQUIRE(nimmAuf());
            CHECK(sitzung.an());  // bleibt an
            sitzung.spiele();     // am Ende: von vorn
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() < 300);

            Sitzung::setAmEnde(AmEnde::AnDenAnfang);
            CHECK(Sitzung::amEnde() == AmEnde::AnDenAnfang);
            REQUIRE(warte([&] { return !sitzung.laeuft() && sitzung.stelleMs() == 0; }));
            CHECK(sitzung.an());

            Sitzung::setAmEnde(AmEnde::ComposerAus);
            sitzung.spiele();
            REQUIRE(warte([&] { return !sitzung.an(); }));
            CHECK(sitzung.geladen());  // die Komposition bleibt im Panel
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() != kRot);

            // Schleife: laeuft ueber das Ende hinweg weiter, auch bei „Composer aus"
            sitzung.setSchleife(true);
            CHECK(QSettings().value(QStringLiteral("composer/loop")).toBool());
            sitzung.spiele();
            QElapsedTimer uhr;
            uhr.start();
            REQUIRE(warte([&] { return uhr.elapsed() > 800; }));
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() < 300);
            // Schleife aus im Lauf: haelt wieder am Ende — hier: Composer aus
            sitzung.setSchleife(false);
            REQUIRE(warte([&] { return !sitzung.an(); }));
        }

        SUBCASE("der Neustart: ohne Resume ins Panel, Composer aus — mit Resume der Stand vom Beenden")
        {
            CHECK_FALSE(Sitzung::resumeAn());
            CHECK_FALSE(sitzung.stelleWiederHer());  // es gab nie eine

            REQUIRE(sitzung.oeffne(lang));
            sitzung.springe(25000);
            sitzung.sichereStand();  // wie bei aboutToQuit
            sitzung.entlade();       // „neu gestartet"

            // Resume aus: die Komposition liegt im Panel, die Kette laeuft
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.geladen());
            CHECK(sitzung.datei() == QFileInfo(lang).absoluteFilePath());
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() != kRot);

            // Resume an: lief sie, laeuft sie von der Stelle an weiter
            Sitzung::setResume(true);
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 25000);
            CHECK(sitzung.stelleMs() < 40000);
            CHECK(bildJetzt() == kRot);

            // angehalten beendet: an der Stelle, angehalten
            sitzung.pausiere();
            sitzung.springe(12000);
            sitzung.sichereStand();
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.an());
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 12000);

            // Composer aus beendet: geladen, aus, die Stelle gemerkt
            sitzung.setAn(false);
            sitzung.sichereStand();
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.stelleMs() == 12000);

            // ohne Komposition beendet: der fruehere Stand bleibt nicht stehen
            sitzung.entlade();
            sitzung.sichereStand();
            CHECK_FALSE(sitzung.stelleWiederHer());

            // der Weg des Starts (Naht N4) ueber die Fassade
            REQUIRE(sitzung.oeffne(lang));
            sitzung.sichereStand();
            sitzung.entlade();
            CHECK(stelleKompositionWiederHer(widget));
            CHECK(kompositionLaeuft(widget));
        }
    }
}
