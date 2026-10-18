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
#include "loop/SignalWeg.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
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
        gezeigt.oeffne(wg);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 900);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        gezeigt.aendereSchicht([](Schicht& s) { s.saitenModell = SaitenModell::Klassisch; });
        QCoreApplication::processEvents();
        const QFileInfo info(ziel);
        CHECK(gezeigt.grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_classic.") + info.suffix())));
    }
}
