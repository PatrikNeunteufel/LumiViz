/**
 ****************************************************************************************
 * @file   test_LoopEditorZoom.cpp
 * @brief  Zoom in den Editoren des Loops (Entscheid Patrik E-Z1 a, S135): Ctrl+Rad die
 *         Zeit, Alt+Rad die Hoehe, Ctrl+Alt+Rad beides, Fit — wie die Zeitachse (S102)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; die Raster bekommen das Rad als Ereignis.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "UI/widgets/Achse.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>

using namespace lumi::loop;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

/// Das Rad an `ort` mit diesen Tasten, eine Raste (120) hinein oder heraus.
void rad(QWidget& ziel, QPoint ort, Qt::KeyboardModifiers tasten, int rasten = 1)
{
    QWheelEvent e(ort, ziel.mapToGlobal(ort), QPoint(), QPoint(0, 120 * rasten), Qt::NoButton, tasten,
                  Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(&ziel, &e);
}

/// Ein Muster ueber vier Takte, Noten von E2 bis E4.
Muster muster()
{
    Muster m;
    m.art = MusterArt::Instrument;
    m.takte = 4;
    m.noten = {Note{0, 4, 40, 1.0, -1}, Note{8, 4, 52, 1.0, -1}, Note{40, 4, 64, 1.0, -1}};
    return m;
}

}  // namespace

TEST_CASE("Loop Editor Zoom: Instrument-Editor — Ctrl+Rad die Zeit, Alt+Rad die Hoehe, um die Maus")
{
    Anwendung anwendung;
    NotenRaster raster;
    raster.setMuster(muster(), 4, 4);
    raster.resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite,
                  NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe);
    raster.zeigeHoehe(52);
    CHECK(raster.schrittBreite() == NotenRaster::kSchrittBreite);
    CHECK(raster.zeilenHoehe() == NotenRaster::kZeilenHoehe);
    CHECK_FALSE(raster.zoomVerstellt());
    int gerollt = 0;
    QObject::connect(&raster, &NotenRaster::gerollt, &raster, [&] { ++gerollt; });

    // Ctrl+Rad hinein: Faktor 1,25 wie die Zeitachse; der Schritt unter der Maus bleibt stehen
    const QPoint maus(raster.xVonSchritt(10) + 3, raster.yVonHoehe(52) + 2);
    rad(raster, maus, Qt::ControlModifier);
    CHECK(raster.schrittBreite() == 20);
    CHECK(raster.schrittBei(maus.x()) == 10);
    CHECK(raster.zeilenProzent() == 100);
    CHECK(gerollt > 0);
    // weiter hinein, bis zur Grenze; dort bleibt es
    for (int i = 0; i < 20; ++i) rad(raster, maus, Qt::ControlModifier);
    CHECK(raster.schrittBreite() == static_cast<int>(NotenRaster::kSchrittBreiteMax));
    for (int i = 0; i < 40; ++i) rad(raster, maus, Qt::ControlModifier, -1);
    CHECK(raster.schrittBreite() == static_cast<int>(NotenRaster::kSchrittBreiteMin));
    CHECK(raster.rollXMax() == 0);  // eng: alles passt hinein

    // Alt+Rad: die Hoehe in Schritten von 10 %; die Tonhoehe unter der Maus bleibt
    raster.zoomeZeit(static_cast<double>(NotenRaster::kSchrittBreite) / raster.schrittBreite(), maus.x());
    const QPoint aufE3(raster.xVonSchritt(8) + 3, raster.yVonHoehe(52) + 3);
    rad(raster, aufE3, Qt::AltModifier);
    CHECK(raster.zeilenProzent() == 100 + lumi::ui::Achse::kHoeheSchritt);
    CHECK(raster.zeilenHoehe() == NotenRaster::kZeilenHoehe * 110 / 100);
    CHECK(raster.hoeheBei(aufE3.y()) == 52);
    CHECK(raster.schrittBreite() == NotenRaster::kSchrittBreite);  // die Zeit nicht
    // ein feines Rad sammelt, bis es eine Raste ist
    QWheelEvent fein(aufE3, raster.mapToGlobal(aufE3), QPoint(), QPoint(0, 60), Qt::NoButton, Qt::AltModifier,
                     Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(&raster, &fein);
    CHECK(raster.zeilenProzent() == 110);
    QCoreApplication::sendEvent(&raster, &fein);
    CHECK(raster.zeilenProzent() == 120);
    // Grenzen der Zeitachse
    for (int i = 0; i < 40; ++i) rad(raster, aufE3, Qt::AltModifier);
    CHECK(raster.zeilenProzent() == lumi::ui::Achse::kHoeheMax);
    for (int i = 0; i < 40; ++i) rad(raster, aufE3, Qt::AltModifier, -1);
    CHECK(raster.zeilenProzent() == lumi::ui::Achse::kHoeheMin);

    // Ctrl+Alt+Rad: beides
    raster.setZeilenProzent(100);
    rad(raster, aufE3, Qt::ControlModifier | Qt::AltModifier);
    CHECK(raster.schrittBreite() == 20);
    CHECK(raster.zeilenProzent() == 110);
    CHECK(raster.zoomVerstellt());

    // das Rad allein rollt weiter die Tonhoehen, Shift+Rad die Zeit — ohne zu zoomen
    const int y = raster.rollY();
    rad(raster, aufE3, Qt::NoModifier, -1);
    CHECK(raster.rollY() > y);
    CHECK(raster.zeilenProzent() == 110);

    // Fit: das ganze Muster in der Breite, die Noten in der Hoehe
    raster.passeEin();
    CHECK(raster.rollXMax() == 0);
    CHECK(raster.schrittBreite() * 64 <= raster.sichtBreite());
    CHECK(raster.yVonHoehe(64) >= raster.rasterOben());
    CHECK(raster.yVonHoehe(40) + raster.zeilenHoehe() <= raster.lautOben());
    CHECK(raster.zeilenProzent() % lumi::ui::Achse::kHoeheSchritt == 0);

    // ein Klick trifft auch gezoomt die richtige Note
    const QPoint aufE4(raster.xVonSchritt(40) + 2, raster.yVonHoehe(64) + 1);
    CHECK(raster.noteBei(aufE4) == 2);
}

TEST_CASE("Loop Editor Zoom: Drum-Editor — Ctrl+Rad die Zeit, Alt+Rad die Spuren, Fit")
{
    Anwendung anwendung;
    Loop loop;
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 4;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                    {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.8, 0.0, false}};
    grund.zeilen = {std::vector<double>(64, 0.0), std::vector<double>(64, 0.0)};
    grund.zeilen[0][0] = 1.0;
    grund.zeilen[1][36] = 1.0;
    loop.muster = {grund};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Grund"), 0, 1}}}};

    DrumEditor editor;
    REQUIRE(editor.zeige(loop, QStringLiteral("Grund")));
    editor.resize(1000, 520);
    editor.setAttribute(Qt::WA_DontShowOnScreen);
    editor.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
    QCoreApplication::processEvents();
    SchrittRaster* raster = editor.raster();
    REQUIRE(raster != nullptr);
    CHECK(raster->schrittBreite() == SchrittRaster::kSchrittBreite);
    CHECK_FALSE(raster->zoomVerstellt());

    // Ctrl+Rad: die Zeit, Faktor 1,25; die Stelle unter der Maus bleibt im Fenster
    auto* rollen = editor.findChild<QScrollArea*>();
    REQUIRE(rollen != nullptr);
    const QPoint maus(raster->xVonSchritt(20) + 3, raster->yVonSpur(0) + 5);
    const int imFenster = maus.x() - rollen->horizontalScrollBar()->value();
    rad(*raster, maus, Qt::ControlModifier);
    CHECK(raster->schrittBreite() == 15);
    CHECK(raster->width() == raster->sizeHint().width());
    const int jetzt = raster->schrittBei(imFenster + rollen->horizontalScrollBar()->value());
    CHECK((jetzt == 20 || jetzt == -1));  // derselbe Schritt (oder die Luft dahinter)
    CHECK(raster->schrittBei(raster->xVonSchritt(36) + 1) == 36);  // ein Klick trifft gezoomt den richtigen

    // Alt+Rad: die Spuren hoeher, die Kopfzeilen links wachsen mit; unter 100 % geht es nicht
    rad(*raster, maus, Qt::AltModifier);
    CHECK(raster->zeilenProzent() == 110);
    CHECK(raster->yVonSpur(1) == SchrittRaster::kKopfHoehe + SchrittRaster::kZeilenHoehe * 110 / 100);
    QCoreApplication::processEvents();
    const auto reihen = editor.findChildren<QWidget*>();
    int passend = 0;
    for (QWidget* w : reihen)
        if (w->property("spur").isValid() && w->minimumHeight() == raster->zeilenHoehe()) ++passend;
    CHECK(passend == 2);  // die zwei Kopfzeilen
    for (int i = 0; i < 5; ++i) rad(*raster, maus, Qt::AltModifier, -1);
    CHECK(raster->zeilenProzent() == SchrittRaster::kZeilenProzentMin);

    // das Rad allein zoomt nicht
    rad(*raster, maus, Qt::NoModifier, -1);
    CHECK(raster->schrittBreite() == 15);

    // Fit: das ganze Muster in der Breite
    auto* fit = editor.findChild<QPushButton*>(QStringLiteral("drumFit"));
    REQUIRE(fit != nullptr);
    CHECK(fit->toolTip().contains(QStringLiteral("Alt+Wheel")));
    rad(*raster, maus, Qt::AltModifier);
    fit->click();
    QCoreApplication::processEvents();
    CHECK(raster->width() <= rollen->viewport()->width());
    CHECK(raster->zeilenProzent() == 100);
}

TEST_CASE("Loop Editor Zoom: Instrument-Editor — der Knopf Fit und die Rollbalken")
{
    Anwendung anwendung;
    InstrumentEditor editor;
    Loop loop;
    Muster m = muster();
    m.name = QStringLiteral("Mel");
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    editor.resize(900, 420);
    editor.setAttribute(Qt::WA_DontShowOnScreen);
    editor.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
    QCoreApplication::processEvents();
    auto* fit = editor.findChild<QPushButton*>(QStringLiteral("instrumentFit"));
    REQUIRE(fit != nullptr);
    CHECK(fit->toolTip().contains(QStringLiteral("Ctrl+Wheel")));
    NotenRaster* raster = editor.raster();
    CHECK(raster->rollXMax() > 0);  // 64 Schritte zu 16 px passen nicht in 900 px
    fit->click();
    CHECK(raster->rollXMax() == 0);
    CHECK(raster->zoomVerstellt());
}
