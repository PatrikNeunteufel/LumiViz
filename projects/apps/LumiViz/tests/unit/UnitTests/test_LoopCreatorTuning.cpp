/**
 ****************************************************************************************
 * @file   test_LoopCreatorTuning.cpp
 * @brief  Saiteninstrumente, Stufe 23e: die Zeile Tuning des Instrument-Creators
 *         (docs/audio/Saiten_Konzept.md §4; Skizze gebilligt, Patrik S132: S-U1 a, S-U3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_CREATOR_TUNING_BILD=<datei.png>` legt
 * zwei Bilder ab: den Creator mit Guitar standard, und (mit `_schmal` im Namen) die
 * Zeile in einer schmalen Breite, umgebrochen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/Saiten.hpp"
#include "loop/SaitenZeile.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QSpinBox>

#include <vector>

using namespace lumi::loop;

namespace
{

/// Eine Qt-Anwendung fuer einen Fall.
struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

int platzVon(const char* schluessel)
{
    const std::vector<Stimmungsvorlage>& vorlagen = stimmungsvorlagen();
    for (std::size_t i = 0; i < vorlagen.size(); ++i)
        if (QLatin1String(vorlagen[i].schluessel) == QLatin1String(schluessel)) return static_cast<int>(i);
    return -99;
}

}  // namespace

TEST_CASE("Loop Instrument Creator Tuning: ein Tonfeld liest Namen und Zahlen")
{
    CHECK(TonFeld::hoeheAus(QStringLiteral("E2")) == 40);
    CHECK(TonFeld::hoeheAus(QStringLiteral("C4")) == 60);
    CHECK(TonFeld::hoeheAus(QStringLiteral("C#4")) == 61);
    CHECK(TonFeld::hoeheAus(QStringLiteral("Bb3")) == 58);
    CHECK(TonFeld::hoeheAus(QStringLiteral("eb3")) == 51);
    CHECK(TonFeld::hoeheAus(QStringLiteral("g4")) == 67);
    CHECK(TonFeld::hoeheAus(QStringLiteral("C-1")) == 0);
    CHECK(TonFeld::hoeheAus(QStringLiteral(" A4 ")) == 69);
    CHECK(TonFeld::hoeheAus(QStringLiteral("64")) == 64);
    CHECK(TonFeld::hoeheAus(QStringLiteral("H2")) == -1);
    CHECK(TonFeld::hoeheAus(QStringLiteral("E")) == -1);
    CHECK(TonFeld::hoeheAus(QStringLiteral("G9")) == 127);  // der hoechste
    CHECK(TonFeld::hoeheAus(QStringLiteral("G#9")) == -1);  // 128
    CHECK(TonFeld::hoeheAus(QStringLiteral("200")) == -1);
}

TEST_CASE("Loop Instrument Creator Tuning: die Zeile — None, Vorlage, Custom, Strings, Capo, Scale, String choice")
{
    Anwendung anwendung;
    SaitenZeile zeile;
    int meldungen = 0;
    Saitensatz zuletzt;
    QObject::connect(&zeile, &SaitenZeile::geaendert, [&](const Saitensatz& s) {
        ++meldungen;
        zuletzt = s;
    });
    auto* tuning = zeile.findChild<QComboBox*>(QStringLiteral("creatorTuning"));
    auto* anzahl = zeile.findChild<QSpinBox*>(QStringLiteral("creatorStringCount"));
    auto* capo = zeile.findChild<QSpinBox*>(QStringLiteral("creatorCapo"));
    auto* mensur = zeile.findChild<QDoubleSpinBox*>(QStringLiteral("creatorScale"));
    auto* wahl = zeile.findChild<QComboBox*>(QStringLiteral("creatorStringChoice"));
    auto* erste = zeile.findChild<TonFeld*>(QStringLiteral("creatorString1"));
    auto* siebte = zeile.findChild<TonFeld*>(QStringLiteral("creatorString7"));
    REQUIRE(tuning != nullptr);
    REQUIRE(anzahl != nullptr);
    REQUIRE(capo != nullptr);
    REQUIRE(mensur != nullptr);
    REQUIRE(wahl != nullptr);
    REQUIRE(erste != nullptr);
    REQUIRE(siebte != nullptr);

    // None: kein Saiteninstrument, nur die Auswahl
    CHECK(zeile.vorlagePlatz() == SaitenZeile::kKeine);
    CHECK(tuning->currentText() == QStringLiteral("None"));
    CHECK(anzahl->parentWidget()->isHidden());
    CHECK(erste->isHidden());
    CHECK(capo->parentWidget()->isHidden());
    CHECK(wahl->parentWidget()->isHidden());
    // was es ohne Saiten nicht gibt, aendert nichts
    zeile.setzeCapo(3);
    zeile.setzeAnzahl(4);
    CHECK(meldungen == 0);
    // die Auswahl: None, die sechs Vorlagen, Custom (nicht waehlbar)
    CHECK(tuning->count() == static_cast<int>(stimmungsvorlagen().size()) + 2);
    CHECK(tuning->itemText(tuning->count() - 1) == QStringLiteral("Custom"));

    // Guitar standard: sechs Saiten, die Mensur der Vorlage
    zeile.waehleVorlage(platzVon("guitar"));
    CHECK(meldungen == 1);
    CHECK(zuletzt.leer == std::vector<int>{40, 45, 50, 55, 59, 64});
    CHECK(zuletzt.mensurMm == 648.0);
    CHECK(tuning->currentText() == QStringLiteral("Guitar standard"));
    CHECK(anzahl->value() == 6);
    CHECK_FALSE(anzahl->parentWidget()->isHidden());
    CHECK(erste->text() == QStringLiteral("E2"));
    CHECK_FALSE(zeile.findChild<TonFeld*>(QStringLiteral("creatorString6"))->isHidden());
    CHECK(siebte->isHidden());
    CHECK(mensur->value() == 648.0);

    // ein Ton anders: die Vorlage ist dann Custom
    erste->setValue(38);  // wie ein Klick auf den Pfeil hinunter, zweimal
    CHECK(zuletzt.leer.front() == 38);
    CHECK(zeile.vorlagePlatz() == platzVon("dropD"));  // D2 A2 D3 G3 B3 E4 ist Drop D
    CHECK(tuning->currentText() == QStringLiteral("Drop D"));
    zeile.setzeTon(5, 63);
    CHECK(zeile.vorlagePlatz() == SaitenZeile::kEigene);
    CHECK(tuning->currentText() == QStringLiteral("Custom"));

    // Strings: mehr = eine Quarte ueber der hoechsten, weniger = die obersten fallen weg
    anzahl->setValue(7);
    CHECK(zuletzt.anzahl() == 7);
    CHECK(zuletzt.leer.back() == 68);  // 63 + 5
    CHECK_FALSE(siebte->isHidden());
    CHECK(siebte->text() == QStringLiteral("G#4"));
    anzahl->setValue(4);
    CHECK(zuletzt.leer == std::vector<int>{38, 45, 50, 55});
    CHECK(siebte->isHidden());

    // Capo, Scale, String choice
    capo->setValue(2);
    CHECK(zuletzt.capo == 2);
    capo->setValue(99);
    CHECK(zuletzt.capo == kCapoMax);
    mensur->setValue(864.0);
    CHECK(zuletzt.mensurMm == 864.0);
    zeile.setzeWahl(Saitenwahl::Hand);
    CHECK(zuletzt.wahl == Saitenwahl::Hand);
    CHECK(wahl->currentText() == QStringLiteral("Follow hand"));

    // eine andere Vorlage: ihre Toene und ihre Mensur, Capo und String choice bleiben
    zeile.waehleVorlage(platzVon("ukulele"));
    CHECK(zuletzt.leer == std::vector<int>{67, 60, 64, 69});
    CHECK(zuletzt.mensurMm == 381.0);
    CHECK(zuletzt.capo == kCapoMax);
    CHECK(zuletzt.wahl == Saitenwahl::Hand);
    // Custom laesst sich nicht waehlen
    const int vorher = meldungen;
    zeile.waehleVorlage(SaitenZeile::kEigene);
    CHECK(meldungen == vorher);

    // zeige: ohne Meldung
    Saitensatz bass;
    bass.leer = {28, 33, 38, 43};
    zeile.zeige(bass);
    CHECK(meldungen == vorher);
    CHECK(tuning->currentText() == QStringLiteral("Bass 4"));
    CHECK(mensur->value() == kMensurVorgabeMm);  // keine gesetzt: die Vorgabe

    // None: keine Saiten mehr
    zeile.waehleVorlage(SaitenZeile::kKeine);
    CHECK_FALSE(zuletzt.istSaiteninstrument());
    CHECK(erste->isHidden());
}

TEST_CASE("Loop Instrument Creator Tuning: im Creator — die Saiten gehoeren dem Instrument, die Zeile bricht um")
{
    Anwendung anwendung;
    InstrumentCreator creator;  // nicht gezeigt: kein Fenster
    REQUIRE(creator.saitenZeile() != nullptr);

    Instrument ohne;
    ohne.titel = QStringLiteral("Plain");
    creator.oeffne(ohne);
    CHECK(creator.saitenZeile()->vorlagePlatz() == SaitenZeile::kKeine);
    CHECK_FALSE(creator.geaendert());

    creator.saitenZeile()->waehleVorlage(platzVon("guitar"));
    CHECK(creator.instrument().saiten.leer == std::vector<int>{40, 45, 50, 55, 59, 64});
    CHECK(creator.instrument().saiten.mensurMm == 648.0);
    CHECK(creator.geaendert());
    // Reset: zurueck ohne Saiten, die Zeile zieht nach
    creator.setzeZurueck();
    CHECK_FALSE(creator.instrument().saiten.istSaiteninstrument());
    CHECK(creator.saitenZeile()->vorlagePlatz() == SaitenZeile::kKeine);

    // ein Instrument mit Saiten: die Zeile zeigt sie
    Instrument violine;
    violine.titel = QStringLiteral("Fiddle");
    violine.saiten = Saitensatz{{55, 62, 69, 76}, 0, Saitenwahl::JeKlang, 327.0};
    creator.oeffne(violine);
    CHECK(creator.saitenZeile()->vorlagePlatz() == platzVon("violin"));
    CHECK(creator.saitenZeile()->saiten() == violine.saiten);
    CHECK_FALSE(creator.geaendert());

    // der Umbruch: schmal braucht die Zeile mehr Hoehe als breit
    creator.oeffne([] {
        Instrument zwoelf;
        zwoelf.saiten.leer = {40, 52, 45, 57, 50, 62, 55, 67, 59, 59, 64, 64};
        return zwoelf;
    }());
    SaitenZeile* zeile = creator.saitenZeile();
    REQUIRE(zeile->hasHeightForWidth());
    const int breit = zeile->heightForWidth(2400);
    const int schmal = zeile->heightForWidth(420);
    MESSAGE("Tuning mit zwoelf Saiten: Hoehe " << breit << " bei 2400 px, " << schmal << " bei 420 px");
    CHECK(schmal > breit);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CREATOR_TUNING_BILD"); !ziel.isEmpty())
    {
        InstrumentCreator gezeigt;
        Instrument gitarre;
        gitarre.titel = QStringLiteral("Nylon");
        gitarre.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 650.0};
        gezeigt.oeffne(gitarre);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 820);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        // schmal: die Zeile allein, zwoelf Saiten, umgebrochen
        SaitenZeile allein;
        Saitensatz zwoelf;
        zwoelf.leer = {40, 52, 45, 57, 50, 62, 55, 67, 59, 59, 64, 64};
        allein.zeige(zwoelf);
        allein.setAttribute(Qt::WA_DontShowOnScreen);
        allein.resize(420, allein.heightForWidth(420));
        allein.show();
        QCoreApplication::processEvents();
        const QFileInfo info(ziel);
        CHECK(allein.grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_schmal.") + info.suffix())));
    }
}
