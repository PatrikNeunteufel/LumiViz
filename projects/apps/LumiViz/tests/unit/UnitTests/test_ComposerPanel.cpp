/**
 ****************************************************************************************
 * @file   test_ComposerPanel.cpp
 * @brief  Composer, Stufe C2b: das Panel zum Ansehen
 *         (docs/ui/Composer_Konzept.md §7)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Das Panel an der echten Fassade (`VisualizerWidget` mit Render-Thread): die
 * Knoepfe bedienen die Sitzung, ein Klick ins Lineal springt, ein Klick auf
 * einen Clip zeigt ihn in der Clip-Zeile.
 *
 * `LUMIVIZ_COMPOSER_PANEL_BILD=<datei.png>` legt ein Bild des Panels ab.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "composer/Sitzung.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <functional>

using namespace lumi::multieffect;
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
        QCoreApplication::setApplicationName(QStringLiteral("ComposerPanel"));
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

VisualClip clip(const QString& datei, std::int64_t start, std::int64_t ende)
{
    VisualClip c;
    c.datei = datei;
    c.startMs = start;
    c.endeMs = ende;
    return c;
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

void klick(QWidget& ziel, QPoint ort)
{
    QMouseEvent druck(QEvent::MouseButtonPress, QPointF(ort), QPointF(ziel.mapToGlobal(ort)),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &druck);
    QMouseEvent los(QEvent::MouseButtonRelease, QPointF(ort), QPointF(ziel.mapToGlobal(ort)),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &los);
}

}  // namespace

TEST_SUITE("ComposerPanel")
{
    TEST_CASE("C2b Panel: die Zeit als Text")
    {
        CHECK(ComposerPanel::zeitText(0) == QStringLiteral("0:00.0"));
        CHECK(ComposerPanel::zeitText(83400) == QStringLiteral("1:23.4"));
        CHECK(ComposerPanel::zeitText(59999) == QStringLiteral("0:59.9"));
        CHECK(ComposerPanel::zeitText(3600000) == QStringLiteral("60:00.0"));
        CHECK(ComposerPanel::zeitText(-5) == QStringLiteral("0:00.0"));
    }

    TEST_CASE("C2b Panel: Knoepfe, Lineal und Clips bedienen die Sitzung des Fensters")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
        Komposition k;
        Kanal unten;
        unten.name = QStringLiteral("CH1");
        unten.visual = {clip(rot, 0, 20000), clip(blau, 30000, 60000)};
        unten.visual[0].setzeBlenden(3000, 5000);
        Kanal oben;
        oben.name = QStringLiteral("CH2");
        oben.visual = {clip(blau, 15000, 35000)};
        oben.visual[0].mischart = Mischart::Additive;
        k.kanaele = {unten, oben};
        const QString datei = dir.filePath(QStringLiteral("probe.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 260);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));

        auto* an = panel.findChild<QCheckBox*>(QStringLiteral("composerAn"));
        auto* spielen = panel.findChild<QPushButton*>(QStringLiteral("composerPlay"));
        auto* stopp = panel.findChild<QPushButton*>(QStringLiteral("composerStop"));
        auto* schleife = panel.findChild<QPushButton*>(QStringLiteral("composerLoop"));
        auto* resume = panel.findChild<QPushButton*>(QStringLiteral("composerResume"));
        auto* zeit = panel.findChild<QLabel*>(QStringLiteral("composerZeit"));
        auto* name = panel.findChild<QLabel*>(QStringLiteral("composerDatei"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        REQUIRE(an != nullptr);
        REQUIRE(spielen != nullptr);
        REQUIRE(stopp != nullptr);
        REQUIRE(schleife != nullptr);
        REQUIRE(resume != nullptr);
        REQUIRE(zeit != nullptr);
        REQUIRE(name != nullptr);
        REQUIRE(clipZeile != nullptr);
        Sitzung* sitzung = panel.sitzung();
        REQUIRE(sitzung == Sitzung::finde(widget));
        Zeitachse* achse = panel.zeitachse();

        // --- nichts geladen: nichts zu bedienen
        CHECK_FALSE(an->isEnabled());
        CHECK_FALSE(spielen->isEnabled());
        CHECK_FALSE(stopp->isEnabled());
        CHECK(zeit->text().isEmpty());
        CHECK(clipZeile->text().isEmpty());

        // --- geladen (ueber die Sitzung, wie es das Menue tut): das Panel zieht nach
        REQUIRE(sitzung->lade(datei));
        CHECK(an->isEnabled());
        CHECK_FALSE(an->isChecked());  // geladen ist nicht an
        CHECK(spielen->isEnabled());
        CHECK(name->text() == QStringLiteral("probe.lvcomp"));
        CHECK(zeit->text() == QStringLiteral("0:00.0 / 1:00.0"));
        CHECK(clipZeile->text() == QStringLiteral("No clip selected"));
        // die Achse zeigt die ganze Laenge auf ihrer Breite
        CHECK(achse->xVon(0) == Zeitachse::kKopfBreite);
        CHECK(achse->xVon(60000) == achse->width() - Zeitachse::kRand);
        CHECK(achse->stelleVon(achse->xVon(30000)) == doctest::Approx(30000).epsilon(0.01));
        // der oberste Kanal des Stapels steht in der obersten Zeile
        CHECK(achse->kanalVonZeile(0) == 1);
        CHECK(achse->kanalVonZeile(1) == 0);

        // --- Clips anklicken
        klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(0)));  // CH2, Blau
        CHECK(achse->gewaehlterKanal() == 1);
        CHECK(achse->gewaehlterClip() == 0);
        CHECK(clipZeile->text() ==
              QStringLiteral("CH2 · Blau.lvfx · 0:15.0 – 0:35.0 · Additive"));
        klick(*achse, QPoint(achse->xVon(45000), achse->yVonZeile(1)));  // CH1, zweiter Clip
        CHECK(achse->gewaehlterKanal() == 0);
        CHECK(achse->gewaehlterClip() == 1);
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH1 · Blau.lvfx · 0:30.0 – 1:00.0")));
        klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(1)));  // Luecke in CH1
        CHECK(achse->auswahl() == nullptr);
        CHECK(clipZeile->text() == QStringLiteral("No clip selected"));

        // --- Klick ins Lineal bei ausgeschaltetem Composer: merkt die Stelle
        klick(*achse, QPoint(achse->xVon(30000), 6));
        CHECK(sitzung->stelleMs() == achse->stelleVon(achse->xVon(30000)));
        CHECK_FALSE(sitzung->an());
        CHECK(zeit->text().startsWith(QStringLiteral("0:30.0")));

        // --- Composer on
        an->click();
        REQUIRE(sitzung->an());
        CHECK(an->isChecked());
        CHECK_FALSE(sitzung->laeuft());

        // --- Play / Pause mit einem Knopf, Stopp
        spielen->click();
        CHECK(sitzung->laeuft());
        CHECK(spielen->toolTip() == QStringLiteral("Pause"));
        REQUIRE(warte([&] { return sitzung->stelleMs() > 30100; }));
        CHECK(zeit->text().startsWith(QStringLiteral("0:30.")));
        spielen->click();
        CHECK_FALSE(sitzung->laeuft());
        CHECK(spielen->toolTip() == QStringLiteral("Play"));
        klick(*achse, QPoint(achse->xVon(10000), 6));  // Sprung bei laufendem Composer
        CHECK(sitzung->stelleMs() == achse->stelleVon(achse->xVon(10000)));
        stopp->click();
        CHECK(sitzung->stelleMs() == 0);
        CHECK(sitzung->an());

        // --- Loop und Resume
        CHECK_FALSE(sitzung->schleife());
        schleife->click();
        CHECK(sitzung->schleife());
        CHECK(schleife->isChecked());
        CHECK_FALSE(Sitzung::resumeAn());
        resume->click();
        CHECK(Sitzung::resumeAn());
        CHECK(resume->isChecked());
        resume->click();
        CHECK_FALSE(Sitzung::resumeAn());

        const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD");
        if (!bild.isEmpty())
        {
            klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(0)));
            sitzung->springe(22000);
            QCoreApplication::processEvents();
            CHECK(panel.grab().save(bild));
        }

        // --- Composer aus ueber den Schalter; die Komposition bleibt
        an->click();
        CHECK_FALSE(sitzung->an());
        CHECK_FALSE(an->isChecked());
        CHECK(sitzung->geladen());

        // --- Close Composition: das Panel ist wieder leer
        sitzung->entlade();
        CHECK_FALSE(an->isEnabled());
        CHECK(name->text().isEmpty());
        CHECK(zeit->text().isEmpty());
        CHECK(clipZeile->text().isEmpty());
    }
}
