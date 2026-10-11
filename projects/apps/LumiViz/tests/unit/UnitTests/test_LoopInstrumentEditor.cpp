/**
 ****************************************************************************************
 * @file   test_LoopInstrumentEditor.cpp
 * @brief  Loop, Stufe 9b: der Instrument-Editor — das Raster der Noten ueber
 *         der Klaviatur und das Fenster darum
 *         (docs/audio/Loop_Konzept.md §4f)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster: Raster und Editor werden nicht gezeigt, die Maus
 * kommt als Ereignis. `LUMIVIZ_INSTRUMENT_EDITOR_BILD=<datei.png>` legt ein
 * Bild des Editors mit dem Beispiel `asset/loop/instrument_demo.lvloop` ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QSlider>
#include <QStringList>
#include <QTemporaryDir>

#include <vector>

using namespace lumi::loop;

namespace
{

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::LeftButton, Qt::NoButton);
}

void ziehe(QWidget& ziel, QPoint von, QPoint nach)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, nach, Qt::NoButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::LeftButton, Qt::NoButton);
}

/// Ein Loop mit einem Instrument-Muster „Mel" (vier Takte) und einem Drumkit „D".
Loop probe()
{
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = QStringLiteral("piano");
    Muster drums;
    drums.name = QStringLiteral("D");
    loop.muster = {mel, drums};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}, {QStringLiteral("Mel"), 8, 1}}},
                    {QStringLiteral("Drums"), true, {{QStringLiteral("D"), 0, 1}}}};
    loop.richte();
    return loop;
}

/// Die Groesse, bei der 64 Schritte und 20 Tonhoehen zu sehen sind.
constexpr int kBreite = NotenRaster::kTastenBreite + 64 * NotenRaster::kSchrittBreite;
constexpr int kHoehe = NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe;

}  // namespace

TEST_SUITE("LoopInstrumentEditor")
{
    TEST_CASE("Noten-Raster: Klick setzt — Ziehen verlaengert und verschiebt — Rechtsklick und Entf loeschen")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        NotenRaster raster;  // nicht gezeigt: kein Fenster
        raster.resize(kBreite, kHoehe);
        Muster muster = probe().muster[0];
        raster.setMuster(muster, 4, 4);
        QStringList gemeldet;
        QObject::connect(&raster, &NotenRaster::geaendert, &raster, [&](const Muster& neu, const QString& was) {
            gemeldet << was;
            // wie der Editor: die Fassung gilt, sortiert
            Loop loop;
            loop.muster = {neu};
            loop.richte();
            muster = loop.muster[0];
            raster.setMuster(muster, 4, 4);
        });
        std::vector<int> toene;
        QObject::connect(&raster, &NotenRaster::tonVerlangt, &raster, [&](int hoehe) { toene.push_back(hoehe); });

        // --- Rollen und Geometrie: sechs Oktaven, 20 Zeilen sind zu sehen
        CHECK(raster.rollXMax() == 0);
        CHECK(raster.rollYMax() == (kHoeheMax - kHoeheMin + 1 - 20) * NotenRaster::kZeilenHoehe);
        raster.setRollen(-5, 99999);
        CHECK(raster.rollX() == 0);
        CHECK(raster.rollY() == raster.rollYMax());
        CHECK(raster.hoeheBei(raster.lautOben() - 1) == kHoeheMin);  // ganz unten: C1
        raster.zeigeHoehe(60);
        const auto ort = [&](int schritt, int hoehe) {
            return QPoint(raster.xVonSchritt(schritt) + 4, raster.yVonHoehe(hoehe) + 5);
        };
        CHECK(raster.hoeheBei(ort(0, 60).y()) == 60);
        CHECK(raster.hoeheBei(ort(0, 69).y()) == 69);
        CHECK(raster.schrittBei(ort(5, 60).x()) == 5);
        CHECK(raster.schrittBei(10) == -1);                      // die Klaviatur
        CHECK(raster.hoeheBei(5) == -1);                         // der Kopf
        CHECK(raster.hoeheBei(raster.lautOben() + 5) == -1);     // die Zeile der Lautstaerken
        CHECK(raster.noteBei(ort(4, 60)) == -1);
        // der Griff am Ende einer Note: ein Drittel von ihr, mindestens 5 und hoechstens 9 Pixel
        CHECK(raster.anKante(Note{4, 1, 60, 1.0}, raster.xVonSchritt(5) - 5));
        CHECK_FALSE(raster.anKante(Note{4, 1, 60, 1.0}, raster.xVonSchritt(5) - 6));
        CHECK(raster.anKante(Note{8, 4, 64, 1.0}, raster.xVonSchritt(12) - 9));
        CHECK_FALSE(raster.anKante(Note{8, 4, 64, 1.0}, raster.xVonSchritt(12) - 10));

        // --- ein Klick auf eine freie Stelle: eine Note von einem Schritt, sie ist gewaehlt
        klick(raster, ort(4, 60));
        REQUIRE(gemeldet == QStringList{QStringLiteral("Add note")});
        REQUIRE(muster.noten.size() == 1);
        CHECK(muster.noten[0] == Note{4, 1, 60, 1.0});
        CHECK(raster.auswahl() == 0);
        CHECK(raster.noteBei(ort(4, 60)) == 0);

        // --- beim Setzen weitergezogen: die Note wird laenger
        ziehe(raster, ort(8, 64), ort(11, 64));
        REQUIRE(gemeldet.size() == 2);
        CHECK(gemeldet.last() == QStringLiteral("Add note"));
        REQUIRE(muster.noten.size() == 2);
        CHECK(muster.noten[1] == Note{8, 4, 64, 1.0});

        // --- die rechte Kante ziehen: laenger und kuerzer, in ganzen Schritten
        ziehe(raster, QPoint(raster.xVonSchritt(5) - 2, ort(4, 60).y()), ort(7, 60));
        CHECK(gemeldet.last() == QStringLiteral("Change note length"));
        CHECK(muster.noten[0] == Note{4, 4, 60, 1.0});
        ziehe(raster, QPoint(raster.xVonSchritt(8) - 2, ort(4, 60).y()), ort(5, 60));
        CHECK(muster.noten[0] == Note{4, 2, 60, 1.0});
        // nicht ueber das Ende des Musters
        ziehe(raster, QPoint(raster.xVonSchritt(12) - 2, ort(8, 64).y()), QPoint(kBreite + 400, ort(8, 64).y()));
        CHECK(muster.noten[1] == Note{8, 56, 64, 1.0});
        ziehe(raster, QPoint(raster.xVonSchritt(64) - 2, ort(8, 64).y()), ort(11, 64));
        CHECK(muster.noten[1] == Note{8, 4, 64, 1.0});

        // --- die Note selbst ziehen: in Zeit und Tonhoehe, der Griff bleibt unter der Maus
        ziehe(raster, ort(9, 64), ort(13, 67));
        CHECK(gemeldet.last() == QStringLiteral("Move note"));
        REQUIRE(muster.noten.size() == 2);
        CHECK(muster.noten[1] == Note{12, 4, 67, 1.0});
        CHECK(raster.auswahl() == 1);
        // auf den Einsatz einer anderen Note derselben Tonhoehe geht es nicht
        klick(raster, ort(20, 67));
        REQUIRE(muster.noten.size() == 3);
        const int vorher = static_cast<int>(gemeldet.size());
        ziehe(raster, ort(20, 67), ort(12, 67));
        CHECK(gemeldet.size() == vorher);
        CHECK(muster.noten[2] == Note{20, 1, 67, 1.0});
        // ein Klick auf eine Note waehlt sie nur
        klick(raster, ort(4, 60));
        CHECK(gemeldet.size() == vorher);
        CHECK(raster.auswahl() == 0);

        // --- unten: die Lautstaerke der Note, die an dem Schritt beginnt
        const int mitte = raster.lautOben() + 3 + (NotenRaster::kLautHoehe - 6) / 2;
        klick(raster, QPoint(raster.xVonSchritt(12) + 3, mitte));
        CHECK(gemeldet.last() == QStringLiteral("Change note volume"));
        CHECK(muster.noten[1].laut == doctest::Approx(0.5));
        CHECK(muster.noten[0].laut == doctest::Approx(1.0));
        // ganz unten bleibt ein Rest: ohne Lautstaerke gaebe es die Note nicht
        klick(raster, QPoint(raster.xVonSchritt(12) + 3, raster.height() - 1));
        CHECK(muster.noten[1].laut == doctest::Approx(0.05));
        REQUIRE(muster.noten.size() == 3);

        // --- eine Taste der Klaviatur verlangt ihren Ton
        klick(raster, QPoint(10, raster.yVonHoehe(62) + 5));
        CHECK(toene == std::vector<int>{62});

        // --- Rechtsklick loescht die Note darunter, Entf die gewaehlte
        maus(raster, QEvent::MouseButtonPress, ort(13, 67), Qt::RightButton, Qt::RightButton);
        CHECK(gemeldet.last() == QStringLiteral("Delete note"));
        REQUIRE(muster.noten.size() == 2);
        CHECK(muster.noten[1] == Note{20, 1, 67, 1.0});
        klick(raster, ort(4, 60));
        QKeyEvent entf(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::sendEvent(&raster, &entf);
        REQUIRE(muster.noten.size() == 1);
        CHECK(muster.noten[0] == Note{20, 1, 67, 1.0});
        CHECK(raster.auswahl() == -1);
        CHECK_FALSE(raster.loescheAuswahl());
    }

    TEST_CASE("Instrument-Creator: Regler stellen die Kopie — Save as schreibt die Datei — eine Taste spielt an")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());

        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        QStringList gespeichert;
        QObject::connect(&creator, &InstrumentCreator::gespeichert, &creator,
                         [&](const QString& pfad) { gespeichert << pfad; });
        QStringList toene;
        QObject::connect(&creator, &InstrumentCreator::tonVerlangt, &creator, [&](const QString& nennung, int hoehe) {
            toene << QStringLiteral("%1 %2").arg(hoehe).arg(nennung);
        });
        const auto schieber = [&](const char* kennung) {
            auto* s = creator.findChild<QSlider*>(QString::fromLatin1(kennung));
            REQUIRE(s != nullptr);
            return s;
        };

        // --- ein mitgeliefertes Instrument: eine Kopie, Save geht nicht, nur Save as
        const Instrument* strings = findeInstrument(QStringLiteral("strings"));
        REQUIRE(strings != nullptr);
        creator.oeffne(*strings);
        CHECK(creator.windowTitle() == QStringLiteral("Instrument: Strings"));
        CHECK(creator.instrument() == *strings);
        CHECK_FALSE(creator.geaendert());
        CHECK(creator.datei().isEmpty());
        CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorBasedOn"))->text() == QStringLiteral("based on Strings"));
        auto* save = creator.findChild<QPushButton*>(QStringLiteral("creatorSave"));
        auto* reset = creator.findChild<QPushButton*>(QStringLiteral("creatorReset"));
        REQUIRE(save != nullptr);
        REQUIRE(reset != nullptr);
        CHECK_FALSE(save->isEnabled());
        CHECK_FALSE(reset->isEnabled());
        CHECK_FALSE(creator.speichere());
        // die Regler zeigen das Instrument
        CHECK(schieber("creatorVoices")->value() == 3);
        CHECK(schieber("creatorDetune")->value() == 9);
        CHECK(schieber("creatorAttack")->value() == 160);
        CHECK(schieber("creatorCutoff")->value() == 60);  // das Sechsfache des Grundtons
        // die Bilder: eine Schwingung und die Huellkurve einer Note
        auto* welle = creator.findChild<KurvenBild*>(QStringLiteral("creatorCycle"));
        auto* huelle = creator.findChild<KurvenBild*>(QStringLiteral("creatorEnvelope"));
        REQUIRE(welle != nullptr);
        REQUIRE(huelle != nullptr);
        CHECK(welle->werte().size() > 300);  // C3 an 44100 Hz: 337 Abtastwerte
        CHECK(huelle->werte().size() == 160);
        CHECK(huelle->werte().front() < 0.2f);  // weicher Einsatz
        CHECK(huelle->werte().back() < 0.05f);  // der Klang endet bei null
        CHECK_FALSE(creator.findChild<QLabel*>(QStringLiteral("creatorPeak"))->text().isEmpty());

        // --- ein Regler stellt die Kopie; der Titel traegt einen Stern; Reset nimmt es zurueck
        schieber("creatorAttack")->setValue(300);
        CHECK(creator.schicht().anstiegMs == doctest::Approx(300.0));
        CHECK(creator.geaendert());
        CHECK(creator.windowTitle() == QStringLiteral("Instrument: Strings *"));
        CHECK(reset->isEnabled());
        CHECK_FALSE(save->isEnabled());  // weiter ohne eigene Datei
        const std::vector<float> weich = huelle->werte();
        schieber("creatorAttack")->setValue(0);
        CHECK(huelle->werte() != weich);  // das Bild zieht nach
        reset->click();
        CHECK(creator.instrument() == *strings);
        CHECK_FALSE(creator.geaendert());
        // ein Regler, an dem niemand zieht, rundet nichts: die Violine hat 5,6 Hz Vibrato
        creator.oeffne(*findeInstrument(QStringLiteral("violin")));
        schieber("creatorLevel")->setValue(50);
        CHECK(creator.schicht().vibratoHz == doctest::Approx(5.6));
        CHECK(creator.schicht().laut == doctest::Approx(0.5));
        CHECK(creator.instrument().laut == doctest::Approx(1.0));  // das ganze Instrument hat seinen eigenen Regler

        // --- die Schwingung bestimmt, welche Regler es gibt
        creator.oeffne(*strings);
        CHECK_FALSE(schieber("creatorVoices")->isHidden());
        CHECK(schieber("creatorHarmonic2")->isHidden());
        CHECK(schieber("creatorDamping")->isHidden());
        creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sinus; });
        CHECK_FALSE(schieber("creatorHarmonic2")->isHidden());
        schieber("creatorHarmonic3")->setValue(40);
        REQUIRE(creator.schicht().obertoene.size() == 2);
        CHECK(creator.schicht().obertoene[1] == doctest::Approx(0.4));
        creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Saite; });
        CHECK(schieber("creatorVoices")->isHidden());
        CHECK(schieber("creatorCutoff")->isHidden());
        CHECK(schieber("creatorHarmonic2")->isHidden());
        CHECK_FALSE(schieber("creatorDamping")->isHidden());
        CHECK_FALSE(schieber("creatorAttack")->isHidden());
        auto* wellenWahl = creator.findChild<QComboBox*>(QStringLiteral("creatorWaveform"));
        REQUIRE(wellenWahl != nullptr);
        CHECK(wellenWahl->currentText() == QStringLiteral("Plucked string"));
        CHECK(wellenWahl->count() == 5);

        // --- Save as: die Datei entsteht, sie ist jetzt die Datei des Creators
        creator.oeffne(*strings);
        creator.aendere([](Instrument& i) {
            i.titel = QStringLiteral("My strings");
            i.schichten[0].verstimmungCent = 20.0;
        });
        const QString pfad = ordner.filePath(QStringLiteral("my_strings.lvinst"));
        const std::uint64_t fassung = instrumentFassung();
        REQUIRE(creator.speichereUnter(pfad));
        CHECK(gespeichert == QStringList{pfad});
        CHECK(creator.datei() == pfad);
        CHECK_FALSE(creator.geaendert());
        CHECK(creator.windowTitle() == QStringLiteral("Instrument: My strings"));
        CHECK(instrumentFassung() > fassung);
        Instrument ausDatei;
        REQUIRE(ladeInstrument(pfad, ausDatei));
        CHECK(ausDatei == creator.instrument());
        CHECK(ausDatei.schichten[0].verstimmungCent == doctest::Approx(20.0));
        // das Original bleibt
        CHECK(findeInstrument(QStringLiteral("strings"))->schichten[0].verstimmungCent == doctest::Approx(9.0));

        // --- Save: in die eigene Datei
        schieber("creatorSustain")->setValue(50);
        CHECK(save->isEnabled());
        save->click();
        CHECK(gespeichert.size() == 2);
        CHECK_FALSE(creator.geaendert());
        REQUIRE(ladeInstrument(pfad, ausDatei));
        CHECK(ausDatei.schichten[0].halten == doctest::Approx(0.5));
        // eine eigene Datei oeffnen: Save geht gleich
        creator.oeffne(ausDatei);
        CHECK(creator.datei() == pfad);
        CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorBasedOn"))->text() == QStringLiteral("my_strings.lvinst"));

        // --- anspielen: die Kopie — auch ungespeichert — steht in der Vorhoer-Datei
        schieber("creatorRelease")->setValue(7);  // 70 ms
        REQUIRE(creator.geaendert());
        REQUIRE(creator.spieleAn(60));
        CHECK(toene == QStringList{QStringLiteral("60 ") + InstrumentCreator::vorhoerDatei()});
        Instrument vorgehoert;
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        CHECK(vorgehoert.schichten[0].ausklangMs == doctest::Approx(70.0));
        REQUIRE(ladeInstrument(pfad, ausDatei));
        CHECK(ausDatei.schichten[0].ausklangMs != doctest::Approx(70.0));  // die eigene Datei bleibt, bis Save
        // die Klaviatur: C3 bis C5, schwarze Tasten liegen oben
        auto* tasten = creator.findChild<Tastenleiste*>(QStringLiteral("creatorKeys"));
        REQUIRE(tasten != nullptr);
        tasten->resize(300, 48);
        for (const int hoehe : {48, 49, 60, 61, 71, 72}) CHECK(tasten->hoeheBei(tasten->ortVon(hoehe)) == hoehe);
        CHECK(tasten->hoeheBei(QPoint(-5, 10)) == -1);
        klick(*tasten, tasten->ortVon(61));
        CHECK(toene.last().startsWith(QStringLiteral("61 ")));
        creator.setzeZurueck();  // nichts Ungespeichertes zurueckbleiben lassen
        QFile::remove(InstrumentCreator::vorhoerDatei());
    }

    TEST_CASE("Instrument-Creator: Schichten — anlegen und waehlen und ordnen — die Regler gelten der gewaehlten")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        const auto schieber = [&](const char* kennung) {
            auto* s = creator.findChild<QSlider*>(QString::fromLatin1(kennung));
            REQUIRE(s != nullptr);
            return s;
        };
        const auto knopf = [&](const char* kennung) {
            auto* k = creator.findChild<QPushButton*>(QString::fromLatin1(kennung));
            REQUIRE(k != nullptr);
            return k;
        };
        auto* liste = creator.findChild<QListWidget*>(QStringLiteral("creatorLayers"));
        auto* kopf = creator.findChild<QLabel*>(QStringLiteral("creatorLayerTitle"));
        auto* legende = creator.findChild<QLabel*>(QStringLiteral("creatorLegend"));
        auto* nurWahl = creator.findChild<QCheckBox*>(QStringLiteral("creatorSelectedOnly"));
        auto* welle = creator.findChild<KurvenBild*>(QStringLiteral("creatorCycle"));
        auto* huelle = creator.findChild<KurvenBild*>(QStringLiteral("creatorEnvelope"));
        auto* spitze = creator.findChild<QLabel*>(QStringLiteral("creatorPeak"));
        REQUIRE(liste != nullptr);
        REQUIRE(kopf != nullptr);
        REQUIRE(legende != nullptr);
        REQUIRE(nurWahl != nullptr);
        REQUIRE(welle != nullptr);
        REQUIRE(huelle != nullptr);

        // --- ein Instrument mit einer Schicht: wie bisher, nur mit der Liste
        const Instrument* organ = findeInstrument(QStringLiteral("organ"));
        REQUIRE(organ != nullptr);
        creator.oeffne(*organ);
        REQUIRE(liste->count() == 1);
        CHECK(liste->item(0)->text() == QStringLiteral("Layer 1 · Sine\nLevel %1 % · 0 st").arg(qRound(organ->schichten[0].laut * 100.0)));
        CHECK(liste->item(0)->checkState() == Qt::Checked);
        CHECK(liste->currentRow() == 0);
        CHECK(kopf->text() == QStringLiteral("Layer 1"));
        CHECK(legende->isHidden());
        CHECK_FALSE(nurWahl->isEnabled());
        CHECK_FALSE(knopf("creatorRemoveLayer")->isEnabled());
        CHECK_FALSE(knopf("creatorLayerUp")->isEnabled());
        CHECK_FALSE(knopf("creatorLayerDown")->isEnabled());
        CHECK(knopf("creatorAddLayer")->isEnabled());
        CHECK(welle->zweite().empty());  // eine Schicht: keine zweite Kurve
        CHECK_FALSE(creator.entferne());
        CHECK_FALSE(creator.schiebe(1));
        CHECK(schieber("creatorMasterLevel")->value() == 100);
        const QString spitzeEine = spitze->text();

        // --- Add layer: Grundeinstellung, gewaehlt; die Regler zeigen jetzt sie
        knopf("creatorAddLayer")->click();
        REQUIRE(creator.instrument().schichten.size() == 2);
        CHECK(creator.gewaehlt() == 1);
        CHECK(creator.schicht() == Schicht{});
        CHECK(creator.geaendert());
        CHECK(liste->count() == 2);
        CHECK(liste->currentRow() == 1);
        CHECK(kopf->text() == QStringLiteral("Layer 2"));
        CHECK_FALSE(legende->isHidden());
        CHECK(nurWahl->isEnabled());
        CHECK(knopf("creatorRemoveLayer")->isEnabled());
        CHECK(knopf("creatorLayerUp")->isEnabled());
        CHECK(schieber("creatorLevel")->value() == 100);
        CHECK(schieber("creatorTranspose")->value() == 0);
        CHECK(welle->zweite().size() == welle->werte().size());  // die gewaehlte gestrichelt
        CHECK(huelle->zweite().size() == 160);
        CHECK(spitze->text() != spitzeEine);  // Peak gilt fuer die Summe

        // --- die Regler der Schicht: Transpose, Start offset, Drive, Level, Name
        schieber("creatorTranspose")->setValue(12);
        schieber("creatorStartOffset")->setValue(8);  // 40 ms
        schieber("creatorDrive")->setValue(30);
        schieber("creatorLevel")->setValue(35);
        creator.aendereSchicht([](Schicht& s) { s.name = QStringLiteral("Shimmer"); });
        CHECK(creator.schicht().versatzHalbtoene == 12);
        CHECK(creator.schicht().einsatzMs == doctest::Approx(40.0));
        CHECK(creator.schicht().verzerrung == doctest::Approx(0.3));
        CHECK(creator.schicht().laut == doctest::Approx(0.35));
        CHECK(liste->item(1)->text() == QStringLiteral("2 Shimmer · Sine\nLevel 35 % · +12 st"));
        CHECK(InstrumentCreator::schichtTitel(creator.schicht(), 1) == QStringLiteral("2 Shimmer"));
        CHECK(creator.instrument().schichten[0] == organ->schichten[0]);  // die erste blieb, wie sie war
        // der Regler des ganzen Instruments
        schieber("creatorMasterLevel")->setValue(80);
        CHECK(creator.instrument().laut == doctest::Approx(0.8));
        CHECK(creator.schicht().laut == doctest::Approx(0.35));

        // --- waehlen: ein Klick in der Liste; die Mitte zeigt die andere Schicht
        liste->setCurrentRow(0);
        CHECK(creator.gewaehlt() == 0);
        CHECK(kopf->text() == QStringLiteral("Layer 1"));
        CHECK(schieber("creatorTranspose")->value() == 0);
        CHECK(schieber("creatorLevel")->value() == qRound(organ->schichten[0].laut * 100.0));
        CHECK(creator.instrument().schichten[1].versatzHalbtoene == 12);  // nichts verstellt beim Wechseln
        CHECK_FALSE(knopf("creatorLayerUp")->isEnabled());
        CHECK(knopf("creatorLayerDown")->isEnabled());

        // --- der Haken schaltet stumm; Peak faellt auf die andere Schicht
        const QString spitzeBeide = spitze->text();
        liste->item(0)->setCheckState(Qt::Unchecked);
        CHECK_FALSE(creator.instrument().schichten[0].an);
        CHECK(creator.instrument().schichten[1].an);
        CHECK(spitze->text() != spitzeBeide);
        liste->item(0)->setCheckState(Qt::Checked);
        CHECK(creator.instrument().schichten[0].an);

        // --- Add layer from instrument: kopiert, heisst wie das Instrument
        const Instrument* strings = findeInstrument(QStringLiteral("strings"));
        REQUIRE(creator.schichtenAus(*strings) == 1);
        REQUIRE(creator.instrument().schichten.size() == 3);
        CHECK(creator.gewaehlt() == 2);
        CHECK(creator.schicht().name == QStringLiteral("Strings"));
        CHECK(creator.schicht().stimmen == 3);
        CHECK(schieber("creatorVoices")->value() == 3);
        CHECK(findeInstrument(QStringLiteral("strings"))->schichten[0].name.isEmpty());  // das Original bleibt

        // --- Duplicate, ordnen, Remove
        creator.waehle(1);
        knopf("creatorDuplicateLayer")->click();
        REQUIRE(creator.instrument().schichten.size() == 4);
        CHECK(creator.gewaehlt() == 2);
        CHECK(creator.instrument().schichten[2] == creator.instrument().schichten[1]);
        CHECK(creator.instrument().schichten[3].name == QStringLiteral("Strings"));
        // vier sind das Ende
        CHECK_FALSE(knopf("creatorAddLayer")->isEnabled());
        CHECK_FALSE(knopf("creatorAddLayerFrom")->isEnabled());
        CHECK_FALSE(knopf("creatorDuplicateLayer")->isEnabled());
        CHECK_FALSE(creator.neueSchicht());
        CHECK_FALSE(creator.verdopple());
        CHECK(creator.schichtenAus(*strings) == 0);
        creator.waehle(3);
        knopf("creatorLayerUp")->click();
        CHECK(creator.gewaehlt() == 2);
        CHECK(creator.instrument().schichten[2].name == QStringLiteral("Strings"));
        CHECK(creator.instrument().schichten[3].name == QStringLiteral("Shimmer"));
        knopf("creatorLayerDown")->click();
        CHECK(creator.gewaehlt() == 3);
        CHECK(creator.instrument().schichten[3].name == QStringLiteral("Strings"));
        CHECK_FALSE(creator.schiebe(1));
        knopf("creatorRemoveLayer")->click();
        REQUIRE(creator.instrument().schichten.size() == 3);
        CHECK(creator.gewaehlt() == 2);  // die letzte ging: die davor ist gewaehlt
        CHECK(creator.schicht().name == QStringLiteral("Shimmer"));
        CHECK(liste->count() == 3);
        CHECK(knopf("creatorAddLayer")->isEnabled());

        // --- anspielen: alle eingeschalteten — oder die gewaehlte allein, auch wenn sie stumm ist
        creator.aendereSchicht([](Schicht& s) { s.an = false; });
        REQUIRE(creator.spieleAn(60));
        Instrument vorgehoert;
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        REQUIRE(vorgehoert.schichten.size() == 3);
        CHECK_FALSE(vorgehoert.schichten[2].an);
        nurWahl->setChecked(true);
        CHECK(creator.nurGewaehlte());
        REQUIRE(creator.spieleAn(60));
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        REQUIRE(vorgehoert.schichten.size() == 1);
        CHECK(vorgehoert.schichten[0].name == QStringLiteral("Shimmer"));
        CHECK(vorgehoert.schichten[0].an);
        CHECK(vorgehoert.laut == doctest::Approx(0.8));
        creator.setNurGewaehlte(false);
        CHECK_FALSE(nurWahl->isChecked());

        // --- Save as schreibt alle Schichten; Reset stellt sie wieder her
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = ordner.filePath(QStringLiteral("layered.lvinst"));
        REQUIRE(creator.speichereUnter(pfad));
        Instrument ausDatei;
        REQUIRE(ladeInstrument(pfad, ausDatei));
        CHECK(ausDatei == creator.instrument());
        CHECK(ausDatei.schichten.size() == 3);
        REQUIRE(creator.entferne());
        REQUIRE(creator.entferne());
        CHECK(creator.instrument().schichten.size() == 1);
        creator.setzeZurueck();
        CHECK(creator.instrument().schichten.size() == 3);
        CHECK(creator.gewaehlt() == 0);
        CHECK_FALSE(creator.geaendert());
        // ein anderes Instrument oeffnen: die Wahl steht wieder vorn
        creator.waehle(2);
        creator.oeffne(*organ);
        CHECK(creator.gewaehlt() == 0);
        CHECK(liste->count() == 1);
        QFile::remove(InstrumentCreator::vorhoerDatei());
    }

    TEST_CASE("Instrument-Editor: Edit instrument oeffnet den Creator — gespeichert nennt die Basis seine Datei")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());

        InstrumentEditor editor;  // nicht gezeigt: kein Fenster
        Loop loop = probe();
        QStringList gemeldet;
        QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            gemeldet << was;
            loop = neu;
            editor.setLoop(loop);
        });
        int gespeichert = 0;
        QObject::connect(&editor, &InstrumentEditor::instrumentGespeichert, &editor, [&] { ++gespeichert; });
        QStringList toene;
        QObject::connect(&editor, &InstrumentEditor::instrumentTonVerlangt, &editor,
                         [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(hoehe).arg(nennung); });

        CHECK_FALSE(editor.oeffneCreator(false));  // noch keine Basis
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        CHECK(editor.creator() == nullptr);
        REQUIRE(editor.oeffneCreator(false));
        InstrumentCreator* creator = editor.creator();
        REQUIRE(creator != nullptr);
        CHECK(creator->instrument() == *findeInstrument(QStringLiteral("piano")));

        // der Creator speichert eine eigene Datei: die Basis nennt sie jetzt
        creator->aendere([](Instrument& i) { i.titel = QStringLiteral("My piano"); });
        const QString pfad = ordner.filePath(QStringLiteral("my_piano.lvinst"));
        REQUIRE(creator->speichereUnter(pfad));
        CHECK(gemeldet == QStringList{QStringLiteral("Change instrument")});
        CHECK(loop.muster[0].instrument == pfad);
        CHECK(gespeichert == 1);
        auto* auswahl = editor.findChild<QComboBox*>(QStringLiteral("instrumentSound"));
        REQUIRE(auswahl != nullptr);
        CHECK(auswahl->currentText() == QStringLiteral("My piano (my_piano.lvinst)"));
        CHECK(auswahl->count() == static_cast<int>(instrumente().size()) + 1);

        // noch einmal Save in dieselbe Datei: am Loop aendert sich nichts, gemeldet wird es trotzdem
        creator->aendereSchicht([](Schicht& s) { s.halten = 0.3; });
        REQUIRE(creator->speichere());
        CHECK(gemeldet.size() == 1);
        CHECK(gespeichert == 2);

        // der Ton des Creators geht weiter
        REQUIRE(creator->spieleAn(64));
        CHECK(toene == QStringList{QStringLiteral("64 ") + InstrumentCreator::vorhoerDatei()});

        // Edit instrument noch einmal: jetzt mit der eigenen Datei
        REQUIRE(editor.oeffneCreator(false));
        CHECK(creator->datei() == pfad);
        CHECK(creator->schicht().halten == doctest::Approx(0.3));

        // ein Instrument, das es nicht gibt, laesst sich nicht bearbeiten
        editor.setzeInstrument(QStringLiteral("tuba"));
        CHECK_FALSE(editor.oeffneCreator(false));
        QFile::remove(InstrumentCreator::vorhoerDatei());

        // `LUMIVIZ_INSTRUMENT_CREATOR_BILD=<datei.png>` legt ein Bild des Creators ab
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_CREATOR_BILD"); !bild.isEmpty())
        {
            // drei Schichten, die dritte stumm: so sieht man Liste, Haken und beide Kurven
            InstrumentCreator gezeigt;
            gezeigt.oeffne(*findeInstrument(QStringLiteral("house_piano")));
            gezeigt.aendereSchicht([](Schicht& s) { s.name = QStringLiteral("Body"); });
            (void)gezeigt.schichtenAus(*findeInstrument(QStringLiteral("strings")));
            gezeigt.aendereSchicht([](Schicht& s) {
                s.name = QStringLiteral("Shimmer");
                s.laut = 0.35;
                s.versatzHalbtoene = 12;
                s.einsatzMs = 60.0;
            });
            (void)gezeigt.schichtenAus(*findeInstrument(QStringLiteral("guitar")));
            gezeigt.aendereSchicht([](Schicht& s) { s.an = false; });
            gezeigt.waehle(1);
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(bild));
        }
    }

    TEST_CASE("Instrument-Editor: zeigt eine Basis — jede Aenderung ist eine neue Fassung des Loops")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        InstrumentEditor editor;  // nicht gezeigt: kein Fenster
        Loop loop = probe();
        QStringList gemeldet;
        QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            gemeldet << was;
            loop = neu;
            editor.setLoop(loop);  // wie das Panel: die Fassung gilt
        });

        CHECK_FALSE(editor.zeige(loop, QStringLiteral("gibt es nicht")));
        CHECK_FALSE(editor.zeige(loop, QStringLiteral("D")));  // ein Drumkit: nicht hier
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        CHECK(editor.windowTitle() == QStringLiteral("Instrument pattern: Mel"));
        CHECK(editor.findChild<QLabel*>(QStringLiteral("instrumentUsers"))->text() == QStringLiteral("· used by 2 clips"));
        auto* auswahl = editor.findChild<QComboBox*>(QStringLiteral("instrumentSound"));
        REQUIRE(auswahl != nullptr);
        const int mitgeliefert = static_cast<int>(instrumente().size());
        REQUIRE(mitgeliefert >= 7);
        REQUIRE(auswahl->count() == mitgeliefert);
        CHECK(auswahl->itemText(0) == instrumente().front().titel);  // nach Titel sortiert
        CHECK(auswahl->currentData().toString() == QStringLiteral("piano"));

        // --- Instrument, Lautstaerke, Panorama, Stumm, Takte
        editor.setzeInstrument(QStringLiteral("strings"));
        CHECK(gemeldet.last() == QStringLiteral("Change instrument"));
        CHECK(loop.muster[0].instrument == QStringLiteral("strings"));
        CHECK(auswahl->currentText() == QStringLiteral("Strings"));
        editor.setzeInstrument(QStringLiteral("strings"));  // dasselbe noch einmal: nichts
        CHECK(gemeldet.size() == 1);
        editor.setzeLaut(0.5);
        CHECK(gemeldet.last() == QStringLiteral("Change pattern volume"));
        CHECK(loop.muster[0].laut == doctest::Approx(0.5));
        editor.setzePan(-2.0);
        CHECK(loop.muster[0].pan == doctest::Approx(-1.0));
        editor.setzeStumm(true);
        CHECK(gemeldet.last() == QStringLiteral("Mute pattern"));
        CHECK(loop.muster[0].stumm);
        editor.setzeStumm(false);
        editor.setzeTakte(8);
        CHECK(gemeldet.last() == QStringLiteral("Change pattern length"));
        CHECK(loop.muster[0].takte == 8);
        CHECK(editor.raster()->muster().takte == 8);
        // beide Clips der Basis sind mitgewachsen: der zweite beginnt, wo der erste jetzt endet
        CHECK(loop.kanaele[0].clips[1].startTakt == 8);
        CHECK(loop.durchlaufTakte() == 16);

        // --- eine Note im Raster: die neue Fassung traegt sie, sortiert
        NotenRaster* raster = editor.raster();
        raster->resize(kBreite, kHoehe);
        raster->zeigeHoehe(60);
        const auto ort = [&](int schritt, int hoehe) {
            return QPoint(raster->xVonSchritt(schritt) + 4, raster->yVonHoehe(hoehe) + 5);
        };
        klick(*raster, ort(8, 62));
        klick(*raster, ort(2, 60));
        CHECK(gemeldet.last() == QStringLiteral("Add note"));
        REQUIRE(loop.muster[0].noten.size() == 2);
        CHECK(loop.muster[0].noten[0] == Note{2, 1, 60, 1.0});
        CHECK(loop.muster[0].noten[1] == Note{8, 1, 62, 1.0});
        CHECK(raster->muster().noten == loop.muster[0].noten);
        CHECK(raster->auswahl() == 0);  // die zuletzt gesetzte bleibt gewaehlt, auch nach dem Sortieren

        // --- Play pattern und eine Taste der Klaviatur gehen ans Panel
        QStringList verlangt;
        QObject::connect(&editor, &InstrumentEditor::spielenVerlangt, &editor,
                         [&](bool an) { verlangt << (an ? QStringLiteral("an") : QStringLiteral("aus")); });
        QObject::connect(&editor, &InstrumentEditor::tonVerlangt, &editor,
                         [&](int hoehe) { verlangt << QString::number(hoehe); });
        auto* play = editor.findChild<QPushButton*>(QStringLiteral("instrumentPlay"));
        REQUIRE(play != nullptr);
        play->click();
        editor.setSpielt(true, 3.5);
        CHECK(play->text() == QStringLiteral("Stop"));
        play->click();
        editor.setSpielt(false, 0.0);
        CHECK(play->text() == QStringLiteral("Play pattern"));
        klick(*raster, QPoint(10, raster->yVonHoehe(64) + 5));
        CHECK(verlangt == QStringList{QStringLiteral("an"), QStringLiteral("aus"), QStringLiteral("64")});
        // der Hinweis, warum die Tasten stumm bleiben
        auto* hinweis = editor.findChild<QLabel*>(QStringLiteral("instrumentHint"));
        REQUIRE(hinweis != nullptr);
        CHECK(hinweis->text().isEmpty());
        editor.setHinweis(QStringLiteral("Keys are silent while the loop plays"));
        CHECK(hinweis->text() == QStringLiteral("Keys are silent while the loop plays"));
        editor.setHinweis(QString());
        // die Spitze: ohne Wiedergabe ein Strich
        auto* spitze = editor.findChild<QLabel*>(QStringLiteral("instrumentPeak"));
        editor.setSpitze(0.42);
        CHECK(spitze->text() == QStringLiteral("0.42"));
        editor.setSpitze(-1.0);
        CHECK(spitze->text() == QStringLiteral("–"));

        // --- ein Instrument, das dieses Programm nicht kennt, bleibt in der Auswahl stehen
        Loop fremd = loop;
        musterIn(fremd, QStringLiteral("Mel"))->instrument = QStringLiteral("tuba");
        editor.setLoop(fremd);
        CHECK(auswahl->count() == mitgeliefert + 1);
        CHECK(auswahl->currentText() == QStringLiteral("tuba (not found)"));
        editor.setLoop(loop);
        CHECK(auswahl->count() == mitgeliefert);

        // --- die Basis wird umbenannt: der Editor folgt; es gibt sie nicht mehr: er ist leer
        Loop umbenannt = loop;
        REQUIRE(benenneBasisUm(umbenannt, QStringLiteral("Mel"), QStringLiteral("Tune")));
        CHECK(folgeBasis(loop, QStringLiteral("Mel"), umbenannt) == QStringLiteral("Tune"));
        editor.setLoop(umbenannt);
        CHECK(editor.muster() == QStringLiteral("Tune"));
        CHECK(editor.windowTitle() == QStringLiteral("Instrument pattern: Tune"));
        Loop ohne = umbenannt;
        REQUIRE(loescheClip(ohne, 0, 1));
        REQUIRE(loescheClip(ohne, 0, 0));
        CHECK(folgeBasis(umbenannt, QStringLiteral("Tune"), ohne).isEmpty());
        editor.setLoop(ohne);
        CHECK(editor.muster().isEmpty());

        // `LUMIVIZ_INSTRUMENT_EDITOR_BILD=<datei.png>` legt ein Bild des Editors mit dem Beispiel ab
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_EDITOR_BILD"); !bild.isEmpty())
        {
            QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
            for (int i = 0; i < 6; ++i) wurzel.cdUp();
            Loop beispiel;
            REQUIRE(ladeLoop(wurzel.filePath(QStringLiteral("asset/loop/instrument_demo.lvloop")), beispiel));
            InstrumentEditor gezeigt;
            REQUIRE(gezeigt.zeige(beispiel, QStringLiteral("Melody")));
            gezeigt.resize(1100, 560);
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            gezeigt.setSpielt(true, 21.5);
            gezeigt.setSpitze(0.62);
            CHECK(gezeigt.grab().save(bild));
        }
    }
}
