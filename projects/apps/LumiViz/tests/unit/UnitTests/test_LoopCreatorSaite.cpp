/**
 ****************************************************************************************
 * @file   test_LoopCreatorSaite.cpp
 * @brief  Saiteninstrumente, Stufe 23e: Model und die Gruppen String und Body im
 *         Instrument-Creator (docs/audio/Saiten_Konzept.md §4; Skizze gebilligt,
 *         Patrik S132: S-U1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_CREATOR_SAITE_BILD=<datei.png>` legt
 * zwei Bilder ab: eine Saite Waveguide mit Korpus, und (mit `_classic` im Namen)
 * dieselbe als Classic.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/SaitenBild.hpp"
#include "loop/SignalWeg.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMouseEvent>
#include <QSlider>
#include <QToolButton>

#include <cmath>

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

Instrument saite(SaitenModell modell)
{
    Instrument instrument;
    instrument.titel = QStringLiteral("String");
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = modell;
    return instrument;
}

}  // namespace

TEST_CASE("Loop Instrument Creator Saite: Model, die Gruppen String und Body — was bei welchem Verfahren steht")
{
    Anwendung anwendung;
    InstrumentCreator creator;  // nicht gezeigt: kein Fenster
    auto* modell = creator.findChild<QComboBox*>(QStringLiteral("creatorStringModel"));
    auto* korpus = creator.findChild<QComboBox*>(QStringLiteral("creatorBody"));
    auto* daempfung = creator.findChild<QSlider*>(QStringLiteral("creatorDamping"));
    auto* ort = creator.findChild<QSlider*>(QStringLiteral("creatorPluckPosition"));
    auto* klingen = creator.findChild<QSlider*>(QStringLiteral("creatorRingTime"));
    auto* anteil = creator.findChild<QSlider*>(QStringLiteral("creatorBodyAmount"));
    auto* stringTitel = creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_string"));
    auto* bodyTitel = creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_body"));
    REQUIRE(modell != nullptr);
    REQUIRE(korpus != nullptr);
    REQUIRE(daempfung != nullptr);
    REQUIRE(ort != nullptr);
    REQUIRE(klingen != nullptr);
    REQUIRE(anteil != nullptr);
    REQUIRE(stringTitel != nullptr);
    REQUIRE(bodyTitel != nullptr);
    const auto quelle = [&] { return creator.signalWeg()->kaesten().front().wert; };

    // keine Saite: weder Model noch String noch Body
    Instrument sinus;
    creator.oeffne(sinus);
    CHECK(modell->isHidden());
    CHECK(stringTitel->isHidden());
    CHECK(bodyTitel->isHidden());
    CHECK(korpus->isHidden());

    // Classic: Model und Damping wie bisher, keine Gruppen
    creator.oeffne(saite(SaitenModell::Klassisch));
    CHECK_FALSE(modell->isHidden());
    CHECK(modell->currentText() == QStringLiteral("Classic"));
    CHECK_FALSE(daempfung->isHidden());
    CHECK(stringTitel->isHidden());
    CHECK(ort->isHidden());
    CHECK(korpus->isHidden());
    CHECK(quelle() == QStringLiteral("Plucked string · Classic"));

    // Extended KS ueber die Auswahl: die Gruppen statt Damping
    modell->setCurrentIndex(modell->findData(static_cast<int>(SaitenModell::ErweiterterKs)));
    Q_EMIT modell->activated(modell->currentIndex());
    CHECK(creator.schicht().saitenModell == SaitenModell::ErweiterterKs);
    CHECK(daempfung->isHidden());
    CHECK_FALSE(stringTitel->isHidden());
    CHECK_FALSE(bodyTitel->isHidden());
    CHECK_FALSE(ort->isHidden());
    CHECK_FALSE(korpus->isHidden());
    CHECK(quelle() == QStringLiteral("Plucked string · Extended KS"));
    // ohne Korpus ist Body amount grau
    CHECK(korpus->currentText() == QStringLiteral("None"));
    CHECK_FALSE(anteil->isEnabled());

    // die Regler stellen die Schicht
    ort->setValue(25);
    CHECK(creator.schicht().anschlagOrt == doctest::Approx(0.25));
    CHECK(ort->minimum() == 2);
    CHECK(ort->maximum() == 50);
    klingen->setValue(klingen->maximum());
    CHECK(creator.schicht().klingSekunden == doctest::Approx(kKlingSekundenMax));
    klingen->setValue(klingen->minimum());
    CHECK(creator.schicht().klingSekunden == doctest::Approx(kKlingSekundenMin));
    klingen->setValue(500);  // die Mitte der Skala: das geometrische Mittel, rund 1,73 s
    CHECK(creator.schicht().klingSekunden == doctest::Approx(std::sqrt(kKlingSekundenMin * kKlingSekundenMax)).epsilon(0.01));
    creator.findChild<QSlider*>(QStringLiteral("creatorStiffness"))->setValue(40);
    CHECK(creator.schicht().steifheit == doctest::Approx(0.4));

    // Body: Classical guitar, dann Body amount
    korpus->setCurrentIndex(korpus->findData(static_cast<int>(KorpusArt::Klassik)));
    Q_EMIT korpus->activated(korpus->currentIndex());
    CHECK(creator.schicht().korpus == KorpusArt::Klassik);
    CHECK(anteil->isEnabled());
    anteil->setValue(80);
    CHECK(creator.schicht().korpusAnteil == doctest::Approx(0.8));

    // Waveguide: dieselben Gruppen
    creator.aendereSchicht([](Schicht& s) { s.saitenModell = SaitenModell::Wellenleiter; });
    CHECK(quelle() == QStringLiteral("Plucked string · Waveguide"));
    CHECK_FALSE(stringTitel->isHidden());
    CHECK(korpus->currentText() == QStringLiteral("Classical guitar"));

    // einklappen: String zu, Body bleibt offen
    creator.setzeGruppeZu(QStringLiteral("string"), true);
    CHECK(ort->isHidden());
    CHECK_FALSE(korpus->isHidden());
    creator.setzeGruppeZu(QStringLiteral("string"), false);
    CHECK_FALSE(ort->isHidden());

    // zurueck auf Classic: Damping, die Gruppen weg; die Werte bleiben in der Schicht
    creator.aendereSchicht([](Schicht& s) { s.saitenModell = SaitenModell::Klassisch; });
    CHECK_FALSE(daempfung->isHidden());
    CHECK(stringTitel->isHidden());
    CHECK(creator.schicht().korpus == KorpusArt::Klassik);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CREATOR_SAITE_BILD"); !ziel.isEmpty())
    {
        InstrumentCreator gezeigt;
        Instrument wg = saite(SaitenModell::Wellenleiter);
        wg.titel = QStringLiteral("Nylon");
        wg.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 650.0};
        wg.schichten[0].korpus = KorpusArt::Klassik;
        // zwei Tonabnehmer, damit das Saitenbild ihre Marken zeigt (die Gruppe Pickups kommt in Teilschritt 4)
        wg.schichten[0].abnehmerHals = Abnehmer{AbnehmerArt::SingleCoil, 160.0};
        wg.schichten[0].abnehmerSteg = Abnehmer{AbnehmerArt::Humbucker, 41.0};
        gezeigt.oeffne(wg);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 900);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        const QFileInfo info(ziel);
        CHECK(gezeigt.saitenBild()->grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_bild.") + info.suffix())));
        gezeigt.aendereSchicht([](Schicht& s) { s.saitenModell = SaitenModell::Klassisch; });
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_classic.") + info.suffix())));
    }
}

namespace
{

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButtons knoepfe)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)),
                         art == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton, knoepfe, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

/// Von `von` nach `nach` ziehen, mit der linken Taste.
void ziehe(QWidget& ziel, QPoint von, QPoint nach)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, nach, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::NoButton);
}

}  // namespace

TEST_CASE("Loop Instrument Creator Saite: das Saitenbild — die Marken pluck, neck, bridge lassen sich ziehen")
{
    Anwendung anwendung;
    SaitenBild bild;
    // 600 px Saite fuer 600 mm: ein Punkt ist ein Millimeter
    bild.resize(600 + 2 * 14, 92);
    REQUIRE(bild.xVon(0.0) - bild.xVon(1.0) == 600);
    double anschlag = -1.0;
    int platz = -1;
    double mm = -1.0;
    QObject::connect(&bild, &SaitenBild::anschlagGezogen, [&](double a) { anschlag = a; });
    QObject::connect(&bild, &SaitenBild::abnehmerGezogen, [&](int p, double m) {
        platz = p;
        mm = m;
    });
    bild.setze(0.25, 600.0, {Abnehmer{AbnehmerArt::SingleCoil, 150.0}, Abnehmer{AbnehmerArt::Humbucker, 40.0}}, true);

    // wo die Marken stehen, und was die Maus dort greift
    CHECK(bild.ortVon(SaitenBild::GriffAnschlag).x() == bild.xVon(0.25));
    CHECK(bild.ortVon(SaitenBild::GriffHals).x() == bild.xVon(150.0 / 600.0));
    CHECK(bild.griffBei(bild.ortVon(SaitenBild::GriffAnschlag)) == SaitenBild::GriffAnschlag);
    CHECK(bild.griffBei(bild.ortVon(SaitenBild::GriffHals)) == SaitenBild::GriffHals);
    CHECK(bild.griffBei(bild.ortVon(SaitenBild::GriffSteg)) == SaitenBild::GriffSteg);
    // ein Humbucker greift man auch an seiner zweiten Spule (18 mm zum Sattel hin)
    CHECK(bild.griffBei(QPoint(bild.xVon(58.0 / 600.0), bild.ortVon(SaitenBild::GriffSteg).y())) == SaitenBild::GriffSteg);
    // daneben nichts
    CHECK(bild.griffBei(QPoint(bild.xVon(0.4), bild.ortVon(SaitenBild::GriffAnschlag).y())) == SaitenBild::KeinGriff);

    // der Zeiger wechselt ueber einer Marke — und zurueck, wenn die Maus weiterzieht
    maus(bild, QEvent::MouseMove, bild.ortVon(SaitenBild::GriffAnschlag), Qt::NoButton);
    CHECK(bild.testAttribute(Qt::WA_SetCursor));
    CHECK(bild.cursor().shape() == Qt::SizeHorCursor);
    maus(bild, QEvent::MouseMove, QPoint(bild.xVon(0.45), 10), Qt::NoButton);
    CHECK(bild.cursor().shape() == Qt::ArrowCursor);  // (ein Fenster ohne Eltern behaelt WA_SetCursor)

    // pluck ziehen: der Anteil folgt der Maus, in den Grenzen der Schicht
    ziehe(bild, bild.ortVon(SaitenBild::GriffAnschlag), QPoint(bild.xVon(0.4), 20));
    CHECK(anschlag == doctest::Approx(0.4).epsilon(0.005));
    ziehe(bild, bild.ortVon(SaitenBild::GriffAnschlag), QPoint(bild.xVon(0.0) + 30, 20));
    CHECK(anschlag == kAnschlagOrtMin);
    ziehe(bild, bild.ortVon(SaitenBild::GriffAnschlag), QPoint(bild.xVon(1.0), 20));
    CHECK(anschlag == kAnschlagOrtMax);

    // neck ziehen: die Lage in mm vom Steg, auf den Millimeter
    ziehe(bild, bild.ortVon(SaitenBild::GriffHals), QPoint(bild.xVon(200.0 / 600.0), 60));
    CHECK(platz == 0);
    CHECK(mm == 200.0);
    ziehe(bild, bild.ortVon(SaitenBild::GriffSteg), QPoint(bild.xVon(25.0 / 600.0), 60));
    CHECK(platz == 1);
    CHECK(mm == 25.0);

    // ein Piezo steht fest am Steg; ohne Tonabnehmer (Extended KS) gibt es keine Marken darunter
    bild.setze(0.25, 600.0, {Abnehmer{}, Abnehmer{AbnehmerArt::Piezo, 0.0}}, true);
    CHECK(bild.ortVon(SaitenBild::GriffSteg) == QPoint(-1, -1));
    CHECK(bild.ortVon(SaitenBild::GriffHals) == QPoint(-1, -1));
    bild.setze(0.25, 600.0, {Abnehmer{AbnehmerArt::SingleCoil, 150.0}, Abnehmer{}}, false);
    CHECK(bild.griffBei(QPoint(bild.xVon(0.25), 60)) == SaitenBild::KeinGriff);

    // im Creator: das Ziehen stellt die Schicht
    InstrumentCreator creator;
    Instrument wg = saite(SaitenModell::Wellenleiter);
    wg.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 600.0};
    wg.schichten[0].abnehmerHals = Abnehmer{AbnehmerArt::SingleCoil, 160.0};
    creator.oeffne(wg);
    SaitenBild* imCreator = creator.saitenBild();
    REQUIRE(imCreator != nullptr);
    CHECK_FALSE(imCreator->isHidden());
    imCreator->resize(600 + 2 * 14, 92);
    ziehe(*imCreator, imCreator->ortVon(SaitenBild::GriffAnschlag), QPoint(imCreator->xVon(0.3), 20));
    CHECK(creator.schicht().anschlagOrt == doctest::Approx(0.3).epsilon(0.005));
    CHECK(creator.findChild<QSlider*>(QStringLiteral("creatorPluckPosition"))->value() == 30);  // der Regler zieht nach
    ziehe(*imCreator, imCreator->ortVon(SaitenBild::GriffHals), QPoint(imCreator->xVon(120.0 / 600.0), 60));
    CHECK(creator.schicht().abnehmerHals.mmVomSteg == 120.0);
    // Classic: kein Saitenbild
    creator.aendereSchicht([](Schicht& s) { s.saitenModell = SaitenModell::Klassisch; });
    CHECK(imCreator->isHidden());
}
