/**
 ****************************************************************************************
 * @file   test_LoopCreatorSpielweise.cpp
 * @brief  Spielweisen, Stufe 24d: der Instrument-Creator — die Zeile Strum unter Tuning
 *         (N), die Gruppe Playing (O), Play as an der Klaviatur (P)
 *         (docs/audio/Saiten_Konzept.md §4b; Skizze gebilligt, Patrik S134: S-W8 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_CREATOR_SPIEL_BILD=<datei.png>` legt ein
 * Bild des Creators mit Strum und Playing ab (die Gruppen String und Body eingeklappt).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/StrumZeile.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>

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

/// Eine Gitarre mit einer Saite des Verfahrens und der Standardstimmung.
Instrument gitarre(SaitenModell modell)
{
    Instrument instrument;
    instrument.titel = QStringLiteral("Strummer");
    instrument.schichten[0].schwingung = Schwingung::Saite;
    instrument.schichten[0].saitenModell = modell;
    instrument.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 648.0};
    return instrument;
}

}  // namespace

TEST_SUITE("LoopCreatorSpielweise")
{
    TEST_CASE("Loop Creator Spielweise: was bei welchem Instrument steht — Strum, Playing, Play as")
    {
        Anwendung anwendung;
        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        auto* zeile = creator.strumZeile();
        auto* spiel = creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_playing"));
        auto* mute = creator.findChild<QSlider*>(QStringLiteral("creatorMute"));
        auto* spielAls = creator.findChild<QComboBox*>(QStringLiteral("creatorPlayAs"));
        REQUIRE(zeile != nullptr);
        REQUIRE(spiel != nullptr);
        REQUIRE(mute != nullptr);
        REQUIRE(spielAls != nullptr);
        for (const char* kennung : {"creatorUpLevel", "creatorTapLevel", "creatorTouch"})
            CHECK(creator.findChild<QSlider*>(QLatin1String(kennung)) != nullptr);

        // --- Waveguide mit Tuning: alles da
        creator.oeffne(gitarre(SaitenModell::Wellenleiter));
        CHECK_FALSE(zeile->isHidden());
        CHECK_FALSE(spiel->isHidden());
        CHECK_FALSE(mute->isHidden());
        CHECK_FALSE(spielAls->isHidden());
        CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorStrumHint"))->text() == QStringLiteral("default for new patterns"));
        CHECK(spielAls->count() == 9);  // seit 26g mit Hammer-on, Slide, Bend

        // --- Extended KS: dasselbe; Classic: Strum und Play as ja, Playing nein (S-W6 a)
        creator.oeffne(gitarre(SaitenModell::ErweiterterKs));
        CHECK_FALSE(spiel->isHidden());
        creator.oeffne(gitarre(SaitenModell::Klassisch));
        CHECK(spiel->isHidden());
        CHECK(mute->isHidden());
        CHECK_FALSE(zeile->isHidden());
        CHECK_FALSE(spielAls->isHidden());

        // --- ohne Tuning: nichts davon (S-W5 a)
        Instrument ohne = gitarre(SaitenModell::Wellenleiter);
        ohne.saiten = Saitensatz{};
        creator.oeffne(ohne);
        CHECK(zeile->isHidden());
        CHECK(spiel->isHidden());
        CHECK(spielAls->parentWidget()->isHidden());  // die Zeile Play as
    }

    TEST_CASE("Loop Creator Spielweise: N — die Zeile Strum stellt die Vorgabe des Instruments")
    {
        Anwendung anwendung;
        InstrumentCreator creator;
        creator.oeffne(gitarre(SaitenModell::Wellenleiter));
        StrumZeile* zeile = creator.strumZeile();

        // 1/2 beat bei 4/4: acht Stellen, ↓ ↑ ↓ ↑ …, keine hinterlegt
        CHECK(zeile->stellen() == 8);
        CHECK(zeile->feld(0)->text() == QStringLiteral("↓"));
        CHECK(zeile->feld(1)->text() == QStringLiteral("↑"));
        CHECK(zeile->feld(8) == nullptr);
        for (int s = 0; s < 8; ++s) CHECK_FALSE(zeile->weichtAb(s));
        CHECK_FALSE(creator.findChild<QPushButton*>(QStringLiteral("creatorStrumAlternate"))->isEnabled());

        // ein Klick auf die sechste Stelle: Down statt Up — hinterlegt, am Instrument
        zeile->feld(5)->click();
        CHECK(zeile->weichtAb(5));
        CHECK(zeile->feld(5)->isChecked());
        CHECK(zeile->feld(5)->text() == QStringLiteral("↓"));
        CHECK(handAlsText(creator.instrument().saiten.strum.hand) == QStringLiteral("DUDUDD"));
        CHECK(creator.geaendert());
        // noch einmal: zurueck, nichts mehr abweichend
        zeile->feld(5)->click();
        CHECK(creator.instrument().saiten.strum.hand.empty());

        // Spread und Hand
        zeile->feld(2)->click();
        auto* spread = creator.findChild<QDoubleSpinBox*>(QStringLiteral("creatorStrumSpread"));
        REQUIRE(spread != nullptr);
        CHECK(spread->value() == doctest::Approx(kSpreadVorgabeMs));
        CHECK(spread->maximum() == doctest::Approx(kSpreadMaxMs));
        spread->setValue(12.5);
        CHECK(creator.instrument().saiten.strum.spreadMs == doctest::Approx(12.5));
        auto* hand = creator.findChild<QComboBox*>(QStringLiteral("creatorStrumHand"));
        REQUIRE(hand != nullptr);
        hand->setCurrentIndex(hand->findData(static_cast<int>(HandRaster::Schritt)));
        Q_EMIT hand->activated(hand->currentIndex());
        CHECK(creator.instrument().saiten.strum.raster == HandRaster::Schritt);
        CHECK(zeile->stellen() == 16);
        CHECK(zeile->weichtAb(2));  // die gedrehte Stelle bleibt

        // Alternate: alle zurueck
        creator.findChild<QPushButton*>(QStringLiteral("creatorStrumAlternate"))->click();
        CHECK(creator.instrument().saiten.strum.hand.empty());
        CHECK(creator.instrument().saiten.strum.raster == HandRaster::Schritt);  // das Raster bleibt
    }

    TEST_CASE("Loop Creator Spielweise: O — die Gruppe Playing stellt die Schicht, mit Klartext")
    {
        Anwendung anwendung;
        InstrumentCreator creator;
        creator.oeffne(gitarre(SaitenModell::Wellenleiter));
        const auto schieber = [&](const char* kennung) {
            auto* s = creator.findChild<QSlider*>(QLatin1String(kennung));
            REQUIRE(s != nullptr);
            return s;
        };
        schieber("creatorUpLevel")->setValue(70);
        CHECK(creator.schicht().aufschlagLaut == doctest::Approx(0.7));
        schieber("creatorMute")->setValue(80);
        CHECK(creator.schicht().handDaempfung == doctest::Approx(0.8));
        schieber("creatorTapLevel")->setValue(50);
        CHECK(creator.schicht().tappingLaut == doctest::Approx(0.5));
        schieber("creatorTouch")->setValue(40);
        CHECK(creator.schicht().knotenDruck == doctest::Approx(0.4));
        // daneben, was es bewirkt: Mute nennt die Klingdauer (Bericht S134 §1.4: 0,8 mit 3,2 s → 0,55 s)
        QStringList texte;
        for (QLabel* label : creator.findChildren<QLabel*>()) texte << label->text();
        CHECK(texte.contains(QStringLiteral("80 % · rings 0.55 s at 220 Hz")));
        CHECK(texte.contains(QStringLiteral("70 % · starts from the other side")));
        CHECK(texte.contains(QStringLiteral("50 % · at the fret, at 220 Hz")));  // S-W18 a
        CHECK(texte.contains(QStringLiteral("40 % · finger on the node")));

        // --- Stufe 25c (S-L10 a, S-L11 a): Slide time und Legato level
        auto* gleiten = schieber("creatorSlideTime");
        CHECK(gleiten->minimum() == static_cast<int>(kGleitMsMin));
        CHECK(gleiten->maximum() == static_cast<int>(kGleitMsMax));
        CHECK(gleiten->value() == static_cast<int>(kGleitMsVorgabe));
        gleiten->setValue(35);
        CHECK(creator.schicht().gleitMsJeHalbton == doctest::Approx(35.0));
        schieber("creatorLegatoLevel")->setValue(60);
        CHECK(creator.schicht().legatoLaut == doctest::Approx(0.6));
        texte.clear();
        for (QLabel* label : creator.findChildren<QLabel*>()) texte << label->text();
        CHECK(texte.contains(QStringLiteral("35 ms per semitone")));
        CHECK(texte.contains(QStringLiteral("60 % · hammer-on, pull-off")));
        // bei Classic nicht da, wie die uebrigen Spielweisen
        creator.oeffne(gitarre(SaitenModell::Klassisch));
        CHECK(gleiten->isHidden());

        // --- Fretless in der Zeile Tuning (S-L9 a): die Vorlage Violin setzt es, ein Klick schaltet
        creator.oeffne(gitarre(SaitenModell::Wellenleiter));
        auto* bundlos = creator.findChild<QCheckBox*>(QStringLiteral("creatorFretless"));
        REQUIRE(bundlos != nullptr);
        CHECK_FALSE(bundlos->isHidden());
        CHECK_FALSE(bundlos->isChecked());
        auto* vorlage = creator.findChild<QComboBox*>(QStringLiteral("creatorTuning"));
        REQUIRE(vorlage != nullptr);
        const int violine = vorlage->findText(QStringLiteral("Violin"));
        REQUIRE(violine >= 0);
        vorlage->setCurrentIndex(violine);
        Q_EMIT vorlage->activated(violine);
        CHECK(creator.instrument().saiten.bundlos);
        CHECK(bundlos->isChecked());
        bundlos->click();
        CHECK_FALSE(creator.instrument().saiten.bundlos);
        // ohne Tuning: kein Haken
        Instrument ohne = gitarre(SaitenModell::Wellenleiter);
        ohne.saiten = Saitensatz{};
        creator.oeffne(ohne);
        CHECK(bundlos->isHidden());
    }

    TEST_CASE("Loop Creator Spielweise: P — Play as, und was die Tasten dann spielen")
    {
        Anwendung anwendung;
        InstrumentCreator creator;
        Instrument instrument = gitarre(SaitenModell::Wellenleiter);
        instrument.saiten.capo = 2;
        instrument.saiten.strum.spreadMs = 15.0;
        creator.oeffne(instrument);
        std::vector<Muster> gespielt;
        QObject::connect(&creator, &InstrumentCreator::vorhoerenVerlangt, &creator,
                         [&](const QString&, const Muster& noten, int) { gespielt.push_back(noten); });

        // Normal: eine Note, ohne Artikulation; Strum des Instruments wie ein neues Muster
        REQUIRE(creator.spieleAn(57));
        REQUIRE(gespielt.size() == 1);
        REQUIRE(gespielt[0].noten.size() == 1);
        CHECK(gespielt[0].noten[0].hoehe == 57);
        CHECK(gespielt[0].noten[0].artikulationen == 0);
        CHECK(gespielt[0].strum.spreadMs == doctest::Approx(15.0));
        CHECK(gespielt[0].anschlaege.empty());

        auto* wahl = creator.findChild<QComboBox*>(QStringLiteral("creatorPlayAs"));
        const auto waehle = [&](InstrumentCreator::SpielAls wie) {
            wahl->setCurrentIndex(wahl->findData(static_cast<int>(wie)));
            Q_EMIT wahl->activated(wahl->currentIndex());
            CHECK(creator.spielAls() == wie);
        };
        waehle(InstrumentCreator::SpielAls::Auf);
        CHECK(creator.vorhoerNoten(57).anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Auf}});
        waehle(InstrumentCreator::SpielAls::PalmMute);
        CHECK(creator.vorhoerNoten(57).noten[0].artikulationen == kPalmMute);
        waehle(InstrumentCreator::SpielAls::Tapping);
        CHECK(creator.vorhoerNoten(57).noten[0].artikulationen == kTapping);
        waehle(InstrumentCreator::SpielAls::Harmonic);
        CHECK(creator.vorhoerNoten(64).noten[0].artikulationen == kHarmonic);
        // Strum chord: die leeren Saiten mit Capo, jede auf ihrer — gleich, welche Taste
        waehle(InstrumentCreator::SpielAls::Akkord);
        const Muster akkord = creator.vorhoerNoten(30);
        REQUIRE(akkord.noten.size() == 6);
        for (int s = 0; s < 6; ++s)
        {
            CHECK(akkord.noten[static_cast<std::size_t>(s)].saite == s);
            CHECK(akkord.noten[static_cast<std::size_t>(s)].hoehe == instrument.saiten.leer[static_cast<std::size_t>(s)] + 2);
        }
        REQUIRE(creator.spieleAn(30));
        CHECK(gespielt.back().noten.size() == 6);

        // Stufe 26g: Hammer-on, Slide, Bend — sie kommen auf dem angeklickten Ton an
        CHECK(wahl->count() == 9);
        waehle(InstrumentCreator::SpielAls::HammerOn);
        {
            const Muster m = creator.vorhoerNoten(57);
            REQUIRE(m.noten.size() == 2);
            CHECK(m.noten[0] == Note{0, 2, 55, 1.0});
            CHECK(m.noten[1].schritt == 2);
            CHECK(m.noten[1].hoehe == 57);
            CHECK(m.noten[1].artikulationen == kLegato);
        }
        waehle(InstrumentCreator::SpielAls::Slide);
        CHECK(creator.vorhoerNoten(57).noten.back().artikulationen == kSlide);
        waehle(InstrumentCreator::SpielAls::Bend);
        {
            const Muster m = creator.vorhoerNoten(57);
            REQUIRE(m.noten.size() == 1);
            CHECK(m.noten[0].hoehe == 55);
            CHECK(m.noten[0].bend == bendVorlage(BendVorlage::Bend, 2.0));
        }
        // ganz unten gibt es keinen Ganzton darunter: der Ton allein
        waehle(InstrumentCreator::SpielAls::HammerOn);
        CHECK(creator.vorhoerNoten(kHoeheMin + 1).noten.size() == 1);
        CHECK(creator.vorhoerNoten(kHoeheMin + 1).noten[0].artikulationen == 0);
        waehle(InstrumentCreator::SpielAls::Normal);

        // ohne Tuning spielt jede Taste ihren Ton, wie immer
        Instrument ohne = instrument;
        ohne.saiten = Saitensatz{};
        creator.oeffne(ohne);
        CHECK(creator.vorhoerNoten(30).noten.size() == 1);
        CHECK(creator.vorhoerNoten(30).noten[0].hoehe == 30);
    }

    TEST_CASE("Loop Creator Spielweise 25c-4: In a legato im Kasten Source, \"~\" in der Liste (S-L13 a)")
    {
        Anwendung anwendung;
        InstrumentCreator creator;
        auto* wahl = creator.findChild<QComboBox*>(QStringLiteral("creatorInALegato"));
        auto* hinweis = creator.findChild<QLabel*>(QStringLiteral("creatorInALegatoHint"));
        auto* liste = creator.findChild<QListWidget*>();
        REQUIRE(wahl != nullptr);
        REQUIRE(hinweis != nullptr);
        REQUIRE(liste != nullptr);

        Instrument instrument = gitarre(SaitenModell::Wellenleiter);
        Schicht flaeche;
        flaeche.name = QStringLiteral("Pad");
        flaeche.schwingung = Schwingung::Sinus;
        instrument.schichten.push_back(flaeche);
        creator.oeffne(instrument);

        // --- A/B: die Saite folgt immer — grau, „Follow string (always)“; die Liste zeigt „~“
        creator.waehle(0);
        CHECK_FALSE(wahl->isHidden());
        CHECK_FALSE(wahl->isEnabled());
        CHECK(wahl->currentText() == QStringLiteral("Follow string (always)"));
        CHECK(hinweis->text() == QStringLiteral("The string itself carries the legato"));
        CHECK(liste->item(0)->text().section(QLatin1Char('\n'), 0, 0).endsWith(QStringLiteral(" ~")));
        CHECK_FALSE(liste->item(1)->text().section(QLatin1Char('\n'), 0, 0).endsWith(QStringLiteral(" ~")));

        // --- die Flaeche: Own note, waehlbar; Follow string setzt das Feld, mit Undo
        creator.waehle(1);
        CHECK(wahl->isEnabled());
        CHECK(wahl->currentText() == QStringLiteral("Own note"));
        CHECK(hinweis->text().startsWith(QStringLiteral("Each joined note plays this layer anew")));
        wahl->setCurrentIndex(1);
        Q_EMIT wahl->activated(1);
        CHECK(creator.instrument().schichten[1].folgtSaite);
        CHECK(creator.geaendert());
        CHECK(wahl->currentText() == QStringLiteral("Follow string"));
        CHECK(hinweis->text().startsWith(QStringLiteral("Goes with the string as one tone")));
        CHECK(liste->item(1)->text().section(QLatin1Char('\n'), 0, 0).endsWith(QStringLiteral(" ~")));
        CHECK(liste->item(1)->toolTip().contains(QStringLiteral("~ follows the string")));

        // --- nur im Kasten Source: ein anderer Kasten blendet die Zeile aus
        creator.waehleStelle(InstrumentCreator::kStelleLevel);
        CHECK(wahl->isHidden());
        creator.waehleStelle(InstrumentCreator::kStelleSource);
        CHECK_FALSE(wahl->isHidden());

        // --- ohne Tuning: keine Zeile und kein „~“
        Instrument ohne = creator.instrument();
        ohne.saiten = Saitensatz{};
        creator.oeffne(ohne);
        creator.waehle(1);
        CHECK(wahl->isHidden());
        CHECK(hinweis->isHidden());
        CHECK_FALSE(liste->item(1)->text().section(QLatin1Char('\n'), 0, 0).endsWith(QStringLiteral(" ~")));

        // --- Bild: der Kasten Source der Flaeche mit Follow string
        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CREATOR_LEGATO_BILD"); !ziel.isEmpty())
        {
            InstrumentCreator gezeigt;
            Instrument mit = instrument;
            mit.schichten[1].folgtSaite = true;
            gezeigt.oeffne(mit);
            gezeigt.waehle(1);
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.resize(1280, 900);
            gezeigt.show();
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(ziel));
        }
    }

    TEST_CASE("Loop Creator Spielweise: Bild (LUMIVIZ_CREATOR_SPIEL_BILD)")
    {
        const QString ziel = qEnvironmentVariable("LUMIVIZ_CREATOR_SPIEL_BILD");
        if (ziel.isEmpty())
        {
            MESSAGE("LUMIVIZ_CREATOR_SPIEL_BILD nicht gesetzt — kein Bild");
            return;
        }
        Anwendung anwendung;
        InstrumentCreator gezeigt;
        Instrument wg = gitarre(SaitenModell::Wellenleiter);
        wg.titel = QStringLiteral("Strummer");
        wg.saiten.strum.hand = handAusText(QStringLiteral("DUDUDD"));
        gezeigt.oeffne(wg);
        gezeigt.setzeSpielAls(InstrumentCreator::SpielAls::PalmMute);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 900);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        gezeigt.setzeGruppeZu(QStringLiteral("string"), true);
        gezeigt.setzeGruppeZu(QStringLiteral("body"), true);
        gezeigt.setzeGruppeZu(QStringLiteral("pickups"), true);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        gezeigt.setzeGruppeZu(QStringLiteral("string"), false);
        gezeigt.setzeGruppeZu(QStringLiteral("body"), false);
        gezeigt.setzeGruppeZu(QStringLiteral("pickups"), false);
    }
}
