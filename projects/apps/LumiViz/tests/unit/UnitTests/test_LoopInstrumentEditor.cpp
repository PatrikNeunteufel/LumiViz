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

#include "PappAufnahme.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "loop/SignalWeg.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QToolButton>
#include <QWheelEvent>
#include <QSlider>
#include <QStringList>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <variant>
#include <vector>

using namespace lumi::loop;

namespace
{

// Die Gruppen des Creators lassen sich einklappen, und das Programm merkt die Stellung
// (S116). In den Tests wird sie weder gelesen noch gemerkt — sonst hinge ihr Ausgang an dem,
// was jemand im Programm eingeklappt hat. Gilt fuer die ganze Test-Exe, auch fuer Creator,
// die Editoren in anderen Dateien anlegen.
const bool kGruppenNichtMerken = (InstrumentCreator::setzeGruppenMerken(false), true);

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

/// Ein Klick mit gedrueckter Shift-Taste (in der Zeile der Takte: die Einfuegemarke, B4 b).
void shiftKlick(QWidget& ziel, QPoint ort)
{
    for (const auto& [art, tasten] : {std::pair{QEvent::MouseButtonPress, Qt::MouseButtons(Qt::LeftButton)},
                                      std::pair{QEvent::MouseButtonRelease, Qt::MouseButtons(Qt::NoButton)}})
    {
        QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), Qt::LeftButton, tasten, Qt::ShiftModifier);
        QCoreApplication::sendEvent(&ziel, &ereignis);
    }
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

template <class Art>
bool ist(const SchichtBaustein& baustein)
{
    return std::holds_alternative<Art>(baustein);
}

/// Den ersten Baustein dieser Art in der Zeile Signal path waehlen — seine Regler stehen dann
/// unter der Zeile (Stufe 16c). Fehlt ein Filter oder ein Drive, kommt er dazu.
template <class Art>
void waehleBaustein(InstrumentCreator& creator)
{
    const std::vector<SchichtBaustein>& reihe = creator.schicht().reihe;
    for (std::size_t i = 0; i < reihe.size(); ++i)
        if (std::holds_alternative<Art>(reihe[i]))
        {
            creator.waehleStelle(static_cast<int>(i));
            return;
        }
    if constexpr (std::is_same_v<Art, SchichtFilter>)
        REQUIRE(creator.fuegeFilterEin());
    else if constexpr (std::is_same_v<Art, SchichtDrive>)
        REQUIRE(creator.fuegeDriveEin());
}

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

        // --- Rechtsklick loescht die Note darunter, Entf die gewaehlte.
        // Seit S113 nur mit dem Haken „Right-click deletes": ohne ihn markiert
        // der Rechtsklick die Note (das Menue hat dann „Delete") und loescht nicht.
        const auto vorRechtsklick = gemeldet.size();
        CHECK_FALSE(raster.rechtsklickLoescht());
        maus(raster, QEvent::MouseButtonPress, ort(13, 67), Qt::RightButton, Qt::RightButton);
        CHECK(gemeldet.size() == vorRechtsklick);
        REQUIRE(muster.noten.size() == 3);
        CHECK(raster.markierte().size() == 1u);
        raster.setRechtsklickLoescht(true);
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

        // --- ein mitgeliefertes Instrument: eine Kopie; ohne Aenderung ist Save aus
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
        // seit Stufe 16c zeigen Cutoff und Drive den Baustein, der in der Zeile Signal path gewaehlt ist
        CHECK(schieber("creatorCutoff")->value() == 0);
        waehleBaustein<SchichtFilter>(creator);
        CHECK(schieber("creatorCutoff")->value() == 60);  // das Sechsfache des Grundtons
        creator.waehleStelle(InstrumentCreator::kStelleSource);
        CHECK_FALSE(creator.geaendert());  // einen Kasten waehlen aendert nichts
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
        // seit 14b (S114): Save ist auch ohne eigene Datei waehlbar, sobald etwas
        // geaendert ist — der Knopf fragt dann nach dem Namen der eigenen Kopie
        CHECK(save->isEnabled());
        CHECK(creator.datei().isEmpty());
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
        // seit Stufe 16c ist die Envelope ein Baustein der Zeile: ihre Regler stehen da, wenn sie gewaehlt ist
        CHECK(schieber("creatorAttack")->isHidden());
        waehleBaustein<SchichtHuelle>(creator);
        CHECK_FALSE(schieber("creatorAttack")->isHidden());
        CHECK(schieber("creatorDecay")->isHidden());  // die Saite klingt von selbst aus
        CHECK(schieber("creatorDamping")->isHidden());  // gehoert zu Source
        creator.waehleStelle(InstrumentCreator::kStelleSource);
        auto* wellenWahl = creator.findChild<QComboBox*>(QStringLiteral("creatorWaveform"));
        REQUIRE(wellenWahl != nullptr);
        CHECK(wellenWahl->currentText() == QStringLiteral("Plucked string"));
        CHECK(wellenWahl->count() == 8);  // seit S115 dazu Sample, seit S116 Noise, seit S121 Shape

        // --- Stufe 11d: Pitch sweep und Sweep time; die Schwingung Noise
        {
            const auto steht = [&creator](const QString& text) {
                for (const QLabel* feld : creator.findChildren<QLabel*>())
                    if (!feld->isHidden() && feld->text() == text) return true;
                return false;
            };
            CHECK(schieber("creatorPitchSweep")->isHidden());  // die Saite kennt ihn nicht
            CHECK(schieber("creatorSweepTime")->isHidden());
            creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sinus; });
            QSlider* weg = schieber("creatorPitchSweep");
            QSlider* zeit = schieber("creatorSweepTime");
            CHECK_FALSE(weg->isHidden());
            CHECK(weg->minimum() == -24);
            CHECK(weg->maximum() == 48);
            CHECK(weg->value() == 0);
            CHECK(steht(QStringLiteral("off")));
            CHECK_FALSE(zeit->isEnabled());  // ohne Weg keine Zeit
            weg->setValue(22);
            CHECK(creator.schicht().tonhoeheStartHt == 22.0);
            CHECK(steht(QString::fromUtf8("↓ 22 st")));  // sie faellt
            CHECK(zeit->isEnabled());
            CHECK(zeit->value() == 20);  // vorbelegt 100 ms
            zeit->setValue(21);
            CHECK(creator.schicht().tonhoeheZeitMs == doctest::Approx(105.0));
            CHECK(steht(QStringLiteral("105 ms")));
            weg->setValue(-12);
            CHECK(creator.schicht().tonhoeheStartHt == -12.0);
            CHECK(steht(QString::fromUtf8("↑ 12 st")));  // sie steigt
            // auch eine Sample-Schicht hat ihn (N2 a)
            creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sample; });
            CHECK_FALSE(weg->isHidden());
            CHECK(schieber("creatorVoices")->isHidden());
            // Noise: keine Tonhoehe — Voices, Detune, Harmonics, Vibrato und Pitch sweep fehlen
            creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Rauschen; });
            CHECK(wellenWahl->currentText() == QStringLiteral("Noise"));
            for (const char* weg2 : {"creatorPitchSweep", "creatorSweepTime", "creatorVoices", "creatorDetune", "creatorHarmonic2",
                                     "creatorVibratoRate", "creatorDamping", "creatorFineTune"})
            {
                CAPTURE(weg2);
                CHECK(schieber(weg2)->isHidden());
            }
            // Envelope, Filter und Drive sind Bausteine der Zeile: jeder zeigt seine Regler, wenn er gewaehlt ist
            waehleBaustein<SchichtHuelle>(creator);
            for (const char* da : {"creatorAttack", "creatorDecay", "creatorSustain", "creatorRelease"})
            {
                CAPTURE(da);
                CHECK_FALSE(schieber(da)->isHidden());
            }
            CHECK(schieber("creatorCutoff")->isHidden());
            CHECK(schieber("creatorDrive")->isHidden());
            waehleBaustein<SchichtDrive>(creator);
            CHECK_FALSE(schieber("creatorDrive")->isHidden());
            CHECK(schieber("creatorAttack")->isHidden());
            waehleBaustein<SchichtFilter>(creator);
            CHECK_FALSE(schieber("creatorCutoff")->isHidden());
            CHECK(schieber("creatorDrive")->isHidden());
            CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorCycleTitle"))->text() == QStringLiteral("Noise"));

            // --- Stufe 11e: Type und Resonance am Filter (er ist gewaehlt)
            auto* art = creator.findChild<QComboBox*>(QStringLiteral("creatorFilterType"));
            REQUIRE(art != nullptr);
            QSlider* resonanz = schieber("creatorResonance");
            CHECK(art->count() == 3);
            CHECK(art->currentText() == QStringLiteral("Low-pass"));  // jede Schicht von frueher
            CHECK_FALSE(art->isHidden());
            CHECK_FALSE(resonanz->isHidden());
            schieber("creatorCutoff")->setValue(0);  // „open": kein Filter, nichts zu stellen
            CHECK_FALSE(art->isEnabled());
            CHECK_FALSE(resonanz->isEnabled());
            schieber("creatorCutoff")->setValue(46);
            CHECK(art->isEnabled());
            CHECK(resonanz->isEnabled());
            art->setCurrentIndex(art->findText(QStringLiteral("Band-pass")));
            Q_EMIT art->activated(art->currentIndex());
            CHECK(creator.schicht().filter().art == FilterArt::Bandpass);
            resonanz->setValue(15);
            CHECK(creator.schicht().filter().resonanz == doctest::Approx(0.15));
            CHECK(steht(QStringLiteral("15 %")));
            creator.aendereSchicht([](Schicht& s) { s.holeFilter().art = FilterArt::Hochpass; });
            CHECK(art->currentText() == QStringLiteral("High-pass"));  // die Auswahl folgt
            // die Saite hat keinen Filter
            creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Saite; });
            CHECK(art->isHidden());
            CHECK(resonanz->isHidden());

            // --- Gruppen ein- und ausklappen (Wunsch Patrik S116): ein Klick auf die Ueberschrift.
            //     Seit Stufe 16c gibt es nur noch Oscillator und Vibrato, unter Source — Envelope und
            //     Filter sind Bausteine der Zeile Signal path
            CHECK(creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_envelope")) == nullptr);
            CHECK(creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_filter")) == nullptr);
            auto* schwinger = creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_oscillator"));
            auto* vibrato = creator.findChild<QToolButton*>(QStringLiteral("creatorGroup_vibrato"));
            REQUIRE(schwinger != nullptr);
            REQUIRE(vibrato != nullptr);
            creator.waehleStelle(InstrumentCreator::kStelleSource);
            CHECK(schwinger->text() == QStringLiteral("Oscillator"));
            CHECK_FALSE(creator.gruppeZu(QStringLiteral("oscillator")));
            CHECK(schwinger->arrowType() == Qt::DownArrow);
            CHECK_FALSE(wellenWahl->isHidden());
            schwinger->click();
            CHECK(creator.gruppeZu(QStringLiteral("oscillator")));
            CHECK(schwinger->arrowType() == Qt::RightArrow);
            CHECK_FALSE(schwinger->isHidden());  // die Ueberschrift bleibt
            CHECK(wellenWahl->isHidden());
            CHECK(schieber("creatorDamping")->isHidden());
            CHECK_FALSE(schieber("creatorTranspose")->isHidden());  // steht ueber der Gruppe
            // eine Aenderung an der Schicht klappt nichts auf
            creator.aendereSchicht([](Schicht& s) { s.laut = 0.6; });
            CHECK(wellenWahl->isHidden());
            // ein anderer Kasten und zurueck: die Gruppe bleibt zu
            waehleBaustein<SchichtHuelle>(creator);
            CHECK_FALSE(schieber("creatorAttack")->isHidden());
            CHECK(schwinger->isHidden());  // die Ueberschrift gehoert zu Source
            creator.waehleStelle(InstrumentCreator::kStelleSource);
            CHECK_FALSE(schwinger->isHidden());
            CHECK(wellenWahl->isHidden());
            // wieder auf: es fehlt weiter, was die Saite nicht kennt
            schwinger->click();
            CHECK_FALSE(wellenWahl->isHidden());
            CHECK_FALSE(schieber("creatorDamping")->isHidden());
            CHECK(schieber("creatorVoices")->isHidden());
            CHECK(vibrato->isHidden());  // die Saite hat kein Vibrato: auch keine Ueberschrift
            // die Stellung wird gemerkt und gilt fuer das naechste Fenster (in eigenen Einstellungen)
            {
                const QString org = QCoreApplication::organizationName();
                const QString name = QCoreApplication::applicationName();
                QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
                QCoreApplication::setApplicationName(QStringLiteral("LoopCreatorGroups"));
                QSettings().clear();
                InstrumentCreator::setzeGruppenMerken(true);
                {
                    InstrumentCreator erster;
                    erster.setzeGruppeZu(QStringLiteral("vibrato"), true);
                }
                {
                    InstrumentCreator zweiter;
                    CHECK(zweiter.gruppeZu(QStringLiteral("vibrato")));
                    CHECK_FALSE(zweiter.gruppeZu(QStringLiteral("oscillator")));
                    zweiter.oeffne(*strings);
                    CHECK(zweiter.findChild<QSlider*>(QStringLiteral("creatorVibratoRate"))->isHidden());
                    CHECK_FALSE(zweiter.findChild<QSlider*>(QStringLiteral("creatorVoices"))->isHidden());
                }
                InstrumentCreator::setzeGruppenMerken(false);
                QSettings().clear();
                QCoreApplication::setOrganizationName(org);
                QCoreApplication::setApplicationName(name);
            }
            // --- Befund Patrik S116: mit jeder aufgeklappten Gruppe wuchs das Fenster, bis ueber den
            //     Bildschirm. Die Regler der Schicht stehen jetzt in einem Feld, das rollt — das Fenster
            //     behaelt seine Groesse, gleich was auf- oder zugeklappt ist
            {
                auto* rollen = creator.findChild<QScrollArea*>(QStringLiteral("creatorLayerScroll"));
                REQUIRE(rollen != nullptr);
                creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sinus; });  // die meisten Zeilen
                CHECK(creator.minimumSizeHint().height() < 600);  // die Spalte bestimmt die Hoehe nicht mehr
                creator.setAttribute(Qt::WA_DontShowOnScreen);
                // 540 statt 500 (Stufe 23e): die Zeile Tuning braucht eine Zeile Hoehe mehr
                creator.resize(1000, 540);
                creator.show();
                QCoreApplication::processEvents();
                CHECK(creator.size() == QSize(1000, 540));
                CHECK(rollen->verticalScrollBar()->maximum() > 0);  // es passt nicht alles: der Rollbalken ist da
                const int weit = rollen->verticalScrollBar()->maximum();
                // die Zeilen sind nicht zusammengedrueckt: jede hat die Hoehe, die sie braucht — der
                // Inhalt (was zu rollen ist und was zu sehen ist) bleibt so hoch, gleich wie hoch das Fenster
                CHECK(weit + rollen->viewport()->height() > 530);
                CHECK(wellenWahl->height() >= wellenWahl->sizeHint().height());
                CHECK(schieber("creatorAttack")->height() >= schieber("creatorAttack")->sizeHint().height());
                for (const char* gruppe : {"oscillator", "vibrato"}) creator.setzeGruppeZu(QLatin1String(gruppe), true);
                QCoreApplication::processEvents();
                CHECK(creator.size() == QSize(1000, 540));
                CHECK(rollen->verticalScrollBar()->maximum() == 0);  // alles eingeklappt: nichts zu rollen
                for (const char* gruppe : {"oscillator", "vibrato"}) creator.setzeGruppeZu(QLatin1String(gruppe), false);
                QCoreApplication::processEvents();
                CHECK(creator.size() == QSize(1000, 540));  // wieder aufgeklappt: das Fenster ist nicht gewachsen
                CHECK(rollen->verticalScrollBar()->maximum() == weit);
                // das Mausrad ueber einem Regler rollt das Feld und verstellt ihn nicht
                QSlider* laut = schieber("creatorLevel");
                const int vorher = laut->value();
                QWheelEvent rad(QPointF(5, 5), laut->mapToGlobal(QPointF(5, 5)), QPoint(), QPoint(0, -120), Qt::NoButton,
                                Qt::NoModifier, Qt::NoScrollPhase, false);
                QCoreApplication::sendEvent(laut, &rad);
                CHECK(laut->value() == vorher);
                CHECK_FALSE(rad.isAccepted());  // er nimmt es nicht: ein echtes Rad-Ereignis geht damit ans Feld weiter
                // (dass das Feld dabei rollt, laesst sich hier nicht pruefen — Qt reicht nur ein Ereignis
                //  vom System an das umgebende Fenster weiter, kein von Hand geschicktes)
                creator.hide();
                creator.resize(1000, 680);
            }
            creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Rauschen; });
            // `LUMIVIZ_INSTRUMENT_NOISE_BILD=<datei.png>` legt ein Bild des Creators mit einer Noise-Schicht ab,
            // eines mit eingeklappten Gruppen und eines mit Pitch sweep
            if (const QString bild = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_NOISE_BILD"); !bild.isEmpty())
            {
                CHECK(creator.grab().save(bild));
                creator.setzeGruppeZu(QStringLiteral("envelope"), true);
                creator.setzeGruppeZu(QStringLiteral("oscillator"), true);
                CHECK(creator.grab().save(bild + QStringLiteral(".zu.png")));
                creator.setzeGruppeZu(QStringLiteral("envelope"), false);
                creator.setzeGruppeZu(QStringLiteral("oscillator"), false);
                creator.aendereSchicht([](Schicht& s) {
                    s.schwingung = Schwingung::Sinus;
                    s.tonhoeheStartHt = 22.0;
                });
                CHECK(creator.grab().save(bild + QStringLiteral(".sweep.png")));
            }
        }

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

        // --- Repeat note (Wunsch Patrik S116): der angeklickte Ton spielt immer wieder, die Taste ist
        //     dabei hervorgehoben; „every n beats" stellt den Abstand — so lang darf der Ton klingen
        {
            auto* wiederholen = creator.findChild<QCheckBox*>(QStringLiteral("creatorRepeat"));
            auto* abstand = creator.findChild<QComboBox*>(QStringLiteral("creatorRepeatEvery"));
            REQUIRE(wiederholen != nullptr);
            REQUIRE(abstand != nullptr);
            QStringList lang;
            QObject::connect(&creator, &InstrumentCreator::tonLangVerlangt, &creator, [&](const QString&, int hoehe, int klingtMs) {
                lang << QStringLiteral("%1 %2").arg(hoehe).arg(klingtMs);
            });
            CHECK_FALSE(creator.wiederholt());
            CHECK(tasten->gewaehlt() == -1);        // ohne Repeat ist keine Taste hervorgehoben
            CHECK(creator.wiederholHoehe() == 61);  // die zuletzt angeklickte
            CHECK(abstand->count() == 5);
            CHECK(abstand->currentText() == QStringLiteral("every 2 beats"));
            CHECK(creator.wiederholMs() == 1000);   // zwei Schlaege bei 120 BPM
            creator.setzeSchlagMs(100.0);           // ein schnelles Tempo, damit der Test nicht wartet
            CHECK(creator.wiederholMs() == 250);    // nie dichter als 250 ms
            creator.setzeWiederholSchlaege(4);
            CHECK(abstand->currentText() == QStringLiteral("every 4 beats"));
            CHECK(creator.wiederholMs() == 400);
            creator.setzeWiederholSchlaege(3);      // nennt die Auswahl nicht: bleibt
            CHECK(creator.wiederholSchlaege() == 4);
            const int kurz = static_cast<int>(toene.size());

            wiederholen->setChecked(true);
            CHECK(creator.wiederholt());
            CHECK(tasten->gewaehlt() == 61);
            CHECK(lang == QStringList{QStringLiteral("61 400")});  // gleich, und mit der Zeit bis zum naechsten
            QElapsedTimer uhr;
            uhr.start();
            while (lang.size() < 3 && uhr.elapsed() < 3000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            CHECK(lang.size() >= 3);
            CHECK(uhr.elapsed() >= 700);  // zwei weitere im Abstand von 400 ms
            // eine andere Taste: sie spielt sofort, ist hervorgehoben, und der Abstand zaehlt von ihr an
            klick(*tasten, tasten->ortVon(64));
            CHECK(lang.last() == QStringLiteral("64 400"));
            CHECK(tasten->gewaehlt() == 64);
            CHECK(creator.wiederholHoehe() == 64);
            // was ein Regler aendert, ist beim naechsten Ton zu hoeren: die Vorhoer-Datei traegt es
            schieber("creatorRelease")->setValue(9);
            const int davor = static_cast<int>(lang.size());
            uhr.restart();
            while (lang.size() == davor && uhr.elapsed() < 3000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
            CHECK(vorgehoert.schichten[0].ausklangMs == doctest::Approx(90.0));
            // ein anderer Abstand gilt gleich
            abstand->setCurrentIndex(abstand->findData(1));
            Q_EMIT abstand->activated(abstand->currentIndex());
            CHECK(creator.wiederholMs() == 250);
            // aus: es endet, die Taste ist nicht mehr hervorgehoben, ein Klick spielt wieder kurz
            wiederholen->setChecked(false);
            CHECK_FALSE(creator.wiederholt());
            CHECK(tasten->gewaehlt() == -1);
            const int ende = static_cast<int>(lang.size());
            uhr.restart();
            while (uhr.elapsed() < 600) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            CHECK(lang.size() == ende);
            CHECK(toene.size() == kurz);  // waehrend Repeat ging nichts ueber den kurzen Weg
            klick(*tasten, tasten->ortVon(60));
            CHECK(toene.size() == kurz + 1);
            creator.setzeSchlagMs(500.0);
        }
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
        REQUIRE(creator.fuegeDriveEin());  // seit Stufe 16c ein Baustein der Zeile: er kommt mit 30 % dazu
        schieber("creatorDrive")->setValue(30);
        schieber("creatorLevel")->setValue(35);
        // Pan je Schicht (Stufe 18): der Regler im Kasten Level; der Kasten nennt Pan nur neben der Mitte
        REQUIRE(creator.signalWeg() != nullptr);
        CHECK(creator.signalWeg()->kaesten().back().wert == QStringLiteral("35 %"));
        schieber("creatorPan")->setValue(-20);
        CHECK(creator.schicht().pan == doctest::Approx(-0.2));
        CHECK(creator.signalWeg()->kaesten().back().wert == QStringLiteral("35 % · Pan L 20"));
        schieber("creatorPan")->setValue(100);
        CHECK(creator.schicht().pan == doctest::Approx(1.0));
        CHECK(creator.signalWeg()->kaesten().back().wert == QStringLiteral("35 % · Pan R 100"));
        schieber("creatorPan")->setValue(0);
        CHECK(creator.schicht().pan == 0.0);
        CHECK(creator.signalWeg()->kaesten().back().wert == QStringLiteral("35 %"));
        creator.aendereSchicht([](Schicht& s) { s.name = QStringLiteral("Shimmer"); });
        CHECK(creator.schicht().versatzHalbtoene == 12);
        CHECK(creator.schicht().einsatzMs == doctest::Approx(40.0));
        CHECK(creator.schicht().drive().wert == doctest::Approx(0.3));
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
        // acht sind das Ende (bis Stufe 17 vier; Entscheid Patrik S120)
        CHECK(knopf("creatorAddLayer")->isEnabled());
        while (creator.instrument().schichten.size() < 8) REQUIRE(creator.neueSchicht());
        CHECK(liste->count() == 8);
        CHECK(creator.gewaehlt() == 7);  // die neue ist gewaehlt
        CHECK_FALSE(knopf("creatorAddLayer")->isEnabled());
        CHECK_FALSE(knopf("creatorAddLayerFrom")->isEnabled());
        CHECK_FALSE(knopf("creatorDuplicateLayer")->isEnabled());
        CHECK_FALSE(creator.neueSchicht());
        CHECK_FALSE(creator.verdopple());
        CHECK(creator.schichtenAus(*strings) == 0);
        CHECK(knopf("creatorAddLayer")->toolTip().contains(QStringLiteral("up to 8 layers")));
        // die vier neuen wieder weg: sie stehen am Ende
        while (creator.instrument().schichten.size() > 4)
        {
            creator.waehle(static_cast<int>(creator.instrument().schichten.size()) - 1);
            REQUIRE(creator.entferne());
        }
        CHECK(creator.instrument().schichten[3].name == QStringLiteral("Strings"));
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

    TEST_CASE("Instrument-Creator: Type und Genres und Description — Save legt eine eigene Kopie in den Benutzerordner")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
        struct Zurueck
        {
            ~Zurueck() { setzeEigenenInstrumentOrdnerFuerTests(QString()); }
        } zurueck;

        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        QStringList gespeichert;
        QObject::connect(&creator, &InstrumentCreator::gespeichert, &creator,
                         [&](const QString& nennung) { gespeichert << nennung; });
        const Instrument mitgeliefert = *findeInstrument(QStringLiteral("house_piano"));
        creator.oeffne(mitgeliefert);

        auto* art = creator.findChild<QComboBox*>(QStringLiteral("creatorType"));
        auto* genres = creator.findChild<QLineEdit*>(QStringLiteral("creatorGenres"));
        auto* satz = creator.findChild<QLineEdit*>(QStringLiteral("creatorDescription"));
        auto* save = creator.findChild<QPushButton*>(QStringLiteral("creatorSave"));
        REQUIRE(art != nullptr);
        REQUIRE(genres != nullptr);
        REQUIRE(satz != nullptr);
        REQUIRE(save != nullptr);

        // --- die zweite Zeile zeigt, was das Instrument traegt
        CHECK(art->count() == 9);
        CHECK(art->itemText(3) == QStringLiteral("Pads and strings"));
        CHECK(art->currentData().toString() == QStringLiteral("keys"));
        CHECK(genres->text() == QStringLiteral("House"));
        CHECK(satz->text() == mitgeliefert.beschreibung);
        CHECK_FALSE(save->isEnabled());  // nichts geaendert

        // --- aendern: wie jede andere Aenderung — Stern, Reset, und Save ist auch
        //     bei einem mitgelieferten waehlbar (fragt dann nach dem Namen)
        art->setCurrentIndex(art->findData(QStringLiteral("bass")));
        Q_EMIT art->activated(art->currentIndex());
        CHECK(creator.instrument().art == QStringLiteral("bass"));
        CHECK(creator.geaendert());
        CHECK(creator.windowTitle().endsWith(QStringLiteral(" *")));
        CHECK(save->isEnabled());
        CHECK(creator.datei().isEmpty());
        CHECK_FALSE(creator.speichere());  // ohne Namen keine Datei

        genres->setText(QStringLiteral(" Funk, disco ,Funk,, "));
        Q_EMIT genres->editingFinished();
        CHECK(creator.instrument().genres == QStringList{QStringLiteral("Funk"), QStringLiteral("disco")});
        CHECK(genres->text() == QStringLiteral("Funk, disco"));
        satz->setText(QStringLiteral("  My own sentence.  "));
        Q_EMIT satz->editingFinished();
        CHECK(creator.instrument().beschreibung == QStringLiteral("My own sentence."));
        CHECK(InstrumentCreator::genresAus(QString()).isEmpty());
        CHECK(InstrumentCreator::genresAus(QStringLiteral("House")) == QStringList{QStringLiteral("House")});

        creator.setzeZurueck();
        CHECK(creator.instrument() == mitgeliefert);
        CHECK(art->currentData().toString() == QStringLiteral("keys"));
        CHECK(genres->text() == QStringLiteral("House"));

        // --- der vorgeschlagene Name: der Titel mit einer Zahl, solange er der der Vorlage ist
        CHECK(creator.vorschlagTitel() == QStringLiteral("House Piano 2"));
        creator.aendere([](Instrument& i) {
            i.art = QStringLiteral("bass");
            i.genres = {QStringLiteral("Funk")};
        });

        // --- als eigenes speichern: in den Benutzerordner, es heisst danach user:<name>
        CHECK_FALSE(creator.speichereAlsEigenes(QStringLiteral("???")));  // daraus wird kein Name
        CHECK(gespeichert.isEmpty());
        REQUIRE(creator.speichereAlsEigenes(QStringLiteral(" House Piano 2 ")));
        CHECK(gespeichert == QStringList{QStringLiteral("user:house_piano_2")});
        CHECK(creator.nennung() == QStringLiteral("user:house_piano_2"));
        CHECK(QFileInfo(creator.datei()) == QFileInfo(eigeneDatei(QStringLiteral("house_piano_2"))));
        CHECK(QFileInfo::exists(creator.datei()));
        CHECK(creator.instrument().titel == QStringLiteral("House Piano 2"));
        CHECK(creator.instrument().name == QStringLiteral("user:house_piano_2"));
        CHECK_FALSE(creator.geaendert());
        CHECK(creator.windowTitle() == QStringLiteral("Instrument: House Piano 2"));
        CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorBasedOn"))->text() ==
              QStringLiteral("My files: house_piano_2.lvinst"));
        Instrument geholt;
        REQUIRE(holeInstrument(QStringLiteral("user:house_piano_2"), geholt));
        CHECK(geholt.art == QStringLiteral("bass"));
        CHECK(geholt.genres == QStringList{QStringLiteral("Funk")});
        CHECK(geholt.schichten == mitgeliefert.schichten);
        // das mitgelieferte bleibt, wie es ist
        CHECK(*findeInstrument(QStringLiteral("house_piano")) == mitgeliefert);

        // --- Save schreibt jetzt ohne Frage an seinen Platz; es bleibt user:<name>
        creator.aendere([](Instrument& i) { i.laut = 0.5; });
        CHECK(save->isEnabled());
        REQUIRE(creator.speichere());
        CHECK(gespeichert.size() == 2);
        CHECK(gespeichert.back() == QStringLiteral("user:house_piano_2"));
        CHECK(creator.instrument().name == QStringLiteral("user:house_piano_2"));
        REQUIRE(holeInstrument(QStringLiteral("user:house_piano_2"), geholt));
        CHECK(geholt.laut == doctest::Approx(0.5));

        // --- ein eigenes oeffnen: Save kennt seinen Platz
        creator.oeffne(geholt);
        CHECK(creator.nennung() == QStringLiteral("user:house_piano_2"));
        CHECK_FALSE(creator.datei().isEmpty());
        creator.aendere([](Instrument& i) { i.beschreibung = QStringLiteral("Changed."); });
        REQUIRE(creator.speichere());
        REQUIRE(holeInstrument(QStringLiteral("user:house_piano_2"), geholt));
        CHECK(geholt.beschreibung == QStringLiteral("Changed."));

        // --- Save as... bei einem eigenen (Entscheid E1 a, S117): kein Dateidialog — der Dialog mit dem
        //     Namen, und eine Kopie daneben im Benutzerordner; das erste bleibt
        auto* saveAs = creator.findChild<QPushButton*>(QStringLiteral("creatorSaveAs"));
        REQUIRE(saveAs != nullptr);
        CHECK(saveAs->toolTip() == QStringLiteral("Save a copy to My files under a new name"));
        CHECK(creator.kopieSatz().startsWith(QStringLiteral("A copy is saved to My files")));
        CHECK(creator.kopieSatz().contains(QStringLiteral("house_piano_2.lvinst stays as it is")));
        CHECK(creator.vorschlagTitel() == QStringLiteral("House Piano 2 2"));
        REQUIRE(creator.speichereAlsEigenes(QStringLiteral("Zweites Klavier")));
        CHECK(creator.nennung() == QStringLiteral("user:zweites_klavier"));
        CHECK(gespeichert.back() == QStringLiteral("user:zweites_klavier"));
        CHECK(QFileInfo(creator.datei()).absolutePath() == QFileInfo(eigenerInstrumentOrdner()).absoluteFilePath());
        CHECK(QFileInfo::exists(eigeneDatei(QStringLiteral("house_piano_2"))));
        REQUIRE(holeInstrument(QStringLiteral("user:house_piano_2"), geholt));
        CHECK(geholt.titel == QStringLiteral("House Piano 2"));

        // --- der Vorschlag weicht einem vergebenen Namen aus; ein eigener, freier Titel gilt, wie er ist
        creator.oeffne(mitgeliefert);
        CHECK(creator.kopieSatz().startsWith(QStringLiteral("House Piano is an original LumiViz instrument. Originals are not overwritten")));
        CHECK(creator.vorschlagTitel() == QStringLiteral("House Piano 3"));
        creator.aendere([](Instrument& i) { i.titel = QStringLiteral("Mein Klavier"); });
        CHECK(creator.vorschlagTitel() == QStringLiteral("Mein Klavier"));
        creator.aendere([](Instrument& i) { i.titel = QStringLiteral("House Piano 2"); });
        CHECK(creator.vorschlagTitel() == QStringLiteral("House Piano 2 2"));

        // --- eine Datei an freiem Ort (kein Knopf fuehrt mehr dorthin, eine geladene gibt es weiter):
        //     dann heisst es wie seine Datei
        const QString frei = QDir(tmp.path()).filePath(QStringLiteral("frei.lvinst"));
        REQUIRE(creator.speichereUnter(frei));
        CHECK(creator.nennung() == frei);
        CHECK(gespeichert.back() == frei);
        CHECK(creator.instrument().name == frei);

        // --- Save as... von dort: die Kopie kommt in den Benutzerordner, die Datei bleibt
        CHECK(creator.kopieSatz().contains(QStringLiteral("frei.lvinst stays as it is")));
        REQUIRE(creator.speichereAlsEigenes(QStringLiteral("Aus Datei")));
        CHECK(creator.nennung() == QStringLiteral("user:aus_datei"));
        CHECK(QFileInfo::exists(eigeneDatei(QStringLiteral("aus_datei"))));
        CHECK(QFileInfo::exists(frei));
    }

    TEST_CASE("Instrument-Creator: Sample als Schicht — File und Root note — Sample start und Sustain loop im Bild der Aufnahme")
    {
        // Stufe 11a, Schritt 2 (Skizze gebilligt, Patrik S115; Entscheide S1 a, S2 a). Ohne Engine:
        // was der Creator an Daten verlangt, gibt der Test heraus — zwei Sekunden Sinus.
        // `LUMIVIZ_INSTRUMENT_SAMPLE_BILD=<datei.png>` legt ein Bild des Creators mit einer Sample-Schicht ab.
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString org = QCoreApplication::organizationName();
        const QString name = QCoreApplication::applicationName();
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopSampleCreator"));
        QSettings().clear();
        setzeEigenenSampleOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("samples")));
        vergissSampleDaten();
        struct Zurueck
        {
            QString org;
            QString name;
            ~Zurueck()
            {
                QSettings().clear();
                QCoreApplication::setOrganizationName(org);
                QCoreApplication::setApplicationName(name);
                setzeEigenenSampleOrdnerFuerTests(QString());
                vergissSampleDaten();
                QFile::remove(InstrumentCreator::vorhoerDatei());
            }
        } zurueck{org, name};
        const auto datei = [&tmp](const QString& rel, const QByteArray& inhalt) {
            const QString pfad = QDir(tmp.path()).filePath(rel);
            REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(inhalt);
            return pfad;
        };

        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        QStringList verlangt;
        QObject::connect(&creator, &InstrumentCreator::sampleVerlangt, &creator, [&](const QString& nennung) {
            verlangt << nennung;
            if (!QFileInfo::exists(sampleDatei(nennung))) return;
            std::vector<float> daten(2 * 96000);  // zwei Sekunden in 48000
            for (std::size_t t = 0; t < 96000; ++t)
                daten[2 * t] = daten[2 * t + 1] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * 220.0 * static_cast<double>(t) / 48000.0));
            merkeSampleDaten(nennung, 48000, std::move(daten));
        });
        const auto schieber = [&](const char* kennung) {
            auto* s = creator.findChild<QSlider*>(QString::fromLatin1(kennung));
            REQUIRE(s != nullptr);
            return s;
        };
        auto* welle = creator.findChild<QComboBox*>(QStringLiteral("creatorWaveform"));
        auto* dateiZeile = creator.findChild<QLabel*>(QStringLiteral("creatorSampleFile"));
        auto* wahl = creator.findChild<QPushButton*>(QStringLiteral("creatorSampleBrowse"));
        auto* grundton = creator.findChild<QComboBox*>(QStringLiteral("creatorRootNote"));
        auto* schleife = creator.findChild<QCheckBox*>(QStringLiteral("creatorSustainLoop"));
        auto* bild = creator.findChild<KurvenBild*>(QStringLiteral("creatorCycle"));
        auto* bildTitel = creator.findChild<QLabel*>(QStringLiteral("creatorCycleTitle"));
        auto* hinweis = creator.findChild<QLabel*>(QStringLiteral("creatorSampleHint"));
        REQUIRE(welle != nullptr);
        REQUIRE(dateiZeile != nullptr);
        REQUIRE(wahl != nullptr);
        REQUIRE(grundton != nullptr);
        REQUIRE(schleife != nullptr);
        REQUIRE(bild != nullptr);
        REQUIRE(bildTitel != nullptr);
        REQUIRE(hinweis != nullptr);

        Instrument neu;
        neu.titel = QStringLiteral("My Cello");
        creator.oeffne(neu);

        // --- eine gerechnete Schicht: von den Zeilen der Aufnahme ist nichts zu sehen
        CHECK(dateiZeile->isHidden());
        CHECK(wahl->isHidden());
        CHECK(grundton->isHidden());
        CHECK(schleife->isHidden());
        CHECK(schieber("creatorFineTune")->isHidden());
        CHECK(schieber("creatorSampleStart")->isHidden());
        CHECK(schieber("creatorLoopFrom")->isHidden());
        CHECK_FALSE(schieber("creatorVoices")->isHidden());
        CHECK(bildTitel->text() == QStringLiteral("One cycle"));
        CHECK(hinweis->isHidden());
        CHECK(bild->griff(KurvenBild::GriffStart) < 0.0);

        // --- Waveform: Sample — die Zeilen kommen, Voices, Detune und Vibrato gehen
        const int alsSample = welle->findData(static_cast<int>(Schwingung::Sample));
        REQUIRE(alsSample >= 0);
        CHECK(welle->itemText(alsSample) == QStringLiteral("Sample"));
        welle->setCurrentIndex(alsSample);
        Q_EMIT welle->activated(alsSample);
        CHECK(creator.schicht().schwingung == Schwingung::Sample);
        CHECK(creator.schicht().halten == 1.0);  // eine Aufnahme klingt zuerst, wie sie ist
        CHECK_FALSE(dateiZeile->isHidden());
        CHECK_FALSE(wahl->isHidden());
        CHECK_FALSE(grundton->isHidden());
        CHECK_FALSE(schleife->isHidden());
        CHECK_FALSE(schieber("creatorFineTune")->isHidden());
        CHECK_FALSE(schieber("creatorSampleStart")->isHidden());
        CHECK_FALSE(schieber("creatorLoopFrom")->isHidden());
        CHECK_FALSE(schieber("creatorLoopTo")->isHidden());
        for (const char* weg : {"creatorVoices", "creatorDetune", "creatorVibratoRate", "creatorVibratoDepth",
                                "creatorVibratoDelay", "creatorDamping", "creatorHarmonic2"})
        {
            CAPTURE(weg);
            CHECK(schieber(weg)->isHidden());
        }
        for (const char* bleibt : {"creatorTranspose", "creatorStartOffset"})
        {
            CAPTURE(bleibt);
            CHECK_FALSE(schieber(bleibt)->isHidden());
        }
        // Level und Envelope haben seit Stufe 16c ihren Kasten in der Zeile Signal path
        CHECK(schieber("creatorLevel")->isHidden());
        CHECK(schieber("creatorPan")->isHidden());  // Pan steht im Kasten Level (Stufe 18)
        creator.waehleStelle(InstrumentCreator::kStelleLevel);
        CHECK_FALSE(schieber("creatorLevel")->isHidden());
        CHECK_FALSE(schieber("creatorPan")->isHidden());
        CHECK(schieber("creatorTranspose")->isHidden());
        waehleBaustein<SchichtHuelle>(creator);
        for (const char* bleibt : {"creatorAttack", "creatorDecay", "creatorSustain", "creatorRelease"})
        {
            CAPTURE(bleibt);
            CHECK_FALSE(schieber(bleibt)->isHidden());
        }
        creator.waehleStelle(InstrumentCreator::kStelleSource);
        CHECK(dateiZeile->text() == QStringLiteral("(none)"));
        CHECK(bildTitel->text() == QStringLiteral("The sample — choose a file"));
        CHECK_FALSE(hinweis->isHidden());
        CHECK(bild->werte().empty());
        CHECK_FALSE(schleife->isEnabled());
        CHECK_FALSE(schieber("creatorSampleStart")->isEnabled());
        CHECK(verlangt.isEmpty());  // nichts genannt: nichts zu laden
        CHECK(creator.sampleLaengeMs() == 0.0);
        auto* liste = creator.findChild<QListWidget*>(QStringLiteral("creatorLayers"));
        REQUIRE(liste != nullptr);
        CHECK(liste->item(0)->text().startsWith(QStringLiteral("Layer 1 · Sample")));

        // --- File..., kopiert (S2 a): das Sample liegt bei den eigenen und heisst user:<name>
        const QString quelle = datei(QStringLiteral("quelle/Cello A3.wav"), "AAAA");
        REQUIRE(creator.nimmSampleDatei(quelle, true));
        CHECK(creator.schicht().sample == QStringLiteral("user:cello_a3"));
        CHECK(QFileInfo::exists(QDir(tmp.path()).filePath(QStringLiteral("samples/cello_a3.wav"))));
        CHECK(verlangt == QStringList{QStringLiteral("user:cello_a3")});  // einmal verlangt, dann gemerkt
        CHECK(creator.sampleLaengeMs() == doctest::Approx(2000.0));
        CHECK(dateiZeile->text() == QStringLiteral("cello_a3.wav · 2.0 s · My samples"));
        CHECK(bildTitel->text() == QStringLiteral("The sample"));
        CHECK(bild->werte().size() == 400);
        CHECK(bild->griff(KurvenBild::GriffStart) == doctest::Approx(0.0));
        CHECK(bild->griff(KurvenBild::GriffVon) < 0.0);  // noch kein Sustain loop
        CHECK(schieber("creatorSampleStart")->isEnabled());
        CHECK(schieber("creatorSampleStart")->maximum() == 2000);
        CHECK(schieber("creatorLoopTo")->maximum() == 2000);
        CHECK_FALSE(schieber("creatorLoopFrom")->isEnabled());  // erst mit dem Haken
        CHECK(schleife->isEnabled());
        CHECK(creator.geaendert());

        // --- Root note und Fine tune
        CHECK(grundton->currentData().toInt() == 60);
        CHECK(grundton->currentText() == QStringLiteral("C4"));
        const int a3 = grundton->findData(57);
        grundton->setCurrentIndex(a3);
        Q_EMIT grundton->activated(a3);
        CHECK(creator.schicht().grundton == 57);
        CHECK(grundton->currentText() == QStringLiteral("A3"));
        schieber("creatorFineTune")->setValue(12);
        CHECK(creator.schicht().feinCent == doctest::Approx(12.0));

        // --- Detect (Schritt 3): messen und vorschlagen — uebernommen wird erst mit Apply
        auto* detect = creator.findChild<QPushButton*>(QStringLiteral("creatorDetect"));
        auto* gemessenText = creator.findChild<QLabel*>(QStringLiteral("creatorDetected"));
        auto* apply = creator.findChild<QPushButton*>(QStringLiteral("creatorDetectApply"));
        auto* referenz = creator.findChild<QCheckBox*>(QStringLiteral("creatorReferenceTone"));
        REQUIRE(detect != nullptr);
        REQUIRE(gemessenText != nullptr);
        REQUIRE(apply != nullptr);
        REQUIRE(referenz != nullptr);
        CHECK_FALSE(detect->isHidden());
        CHECK(detect->isEnabled());
        CHECK(gemessenText->isHidden());  // noch nichts gemessen
        CHECK(apply->isHidden());
        CHECK_FALSE(creator.gemessen().gefunden);
        CHECK_FALSE(creator.uebernimmGemessenes());
        creator.aendereSchicht([](Schicht& s) { s.grundton = 60; });  // absichtlich falsch gestellt
        detect->click();
        REQUIRE(creator.gemessen().gefunden);
        CHECK(creator.gemessen().hoehe == 57);  // die Aufnahme des Tests: 220 Hz
        CHECK_FALSE(gemessenText->isHidden());
        CHECK(gemessenText->text() == QStringLiteral("Detected: A3"));
        CHECK_FALSE(apply->isHidden());
        CHECK(creator.schicht().grundton == 60);  // gemessen ist noch nicht uebernommen
        CHECK(creator.schicht().feinCent == doctest::Approx(12.0));
        apply->click();
        CHECK(creator.schicht().grundton == 57);
        CHECK(creator.schicht().feinCent == doctest::Approx(0.0));
        CHECK(grundton->currentText() == QStringLiteral("A3"));
        CHECK(schieber("creatorFineTune")->value() == 0);
        CHECK(gemessenText->text() == QStringLiteral("Detected: A3"));  // das Ergebnis bleibt stehen

        // --- Sample start: am Regler und im Bild (der rote Griff)
        schieber("creatorSampleStart")->setValue(500);
        CHECK(creator.schicht().sampleStartMs == doctest::Approx(500.0));
        CHECK(bild->griff(KurvenBild::GriffStart) == doctest::Approx(0.25));
        bild->resize(260, 96);
        CHECK(bild->griffBei(bild->xVon(0.25)) == KurvenBild::GriffStart);
        CHECK(bild->griffBei(bild->xVon(0.25) + KurvenBild::kGriffWeite) == KurvenBild::GriffStart);
        CHECK(bild->griffBei(bild->xVon(0.25) + KurvenBild::kGriffWeite + 2) == KurvenBild::KeinGriff);
        ziehe(*bild, QPoint(bild->xVon(0.25), 40), QPoint(bild->xVon(0.5), 40));
        CHECK(creator.schicht().sampleStartMs == doctest::Approx(1000.0).epsilon(0.02));
        CHECK(schieber("creatorSampleStart")->value() == static_cast<int>(creator.schicht().sampleStartMs));
        // daneben gegriffen: nichts geschieht
        ziehe(*bild, QPoint(bild->xVon(0.9), 40), QPoint(bild->xVon(0.1), 40));
        CHECK(creator.schicht().sampleStartMs == doctest::Approx(1000.0).epsilon(0.02));

        // --- Sustain loop: beim ersten Einschalten der mittlere Teil; von bleibt unter bis
        schleife->setChecked(true);
        CHECK(creator.schicht().schleife);
        CHECK(creator.schicht().schleifeVonMs == doctest::Approx(500.0));
        CHECK(creator.schicht().schleifeBisMs == doctest::Approx(1500.0));
        CHECK(schieber("creatorLoopFrom")->isEnabled());
        CHECK(bild->griff(KurvenBild::GriffVon) == doctest::Approx(0.25));
        CHECK(bild->griff(KurvenBild::GriffBis) == doctest::Approx(0.75));
        schieber("creatorLoopFrom")->setValue(1800);
        CHECK(creator.schicht().schleifeVonMs == doctest::Approx(1499.0));
        CHECK(schieber("creatorLoopFrom")->value() == 1499);
        schieber("creatorLoopFrom")->setValue(400);
        schieber("creatorLoopTo")->setValue(100);
        CHECK(creator.schicht().schleifeBisMs == doctest::Approx(401.0));
        schieber("creatorLoopTo")->setValue(1500);
        // der blaue Griff rechts, im Bild gezogen
        ziehe(*bild, QPoint(bild->xVon(0.75), 40), QPoint(bild->xVon(0.9), 40));
        CHECK(creator.schicht().schleifeBisMs == doctest::Approx(1800.0).epsilon(0.02));
        CHECK(creator.schicht().schleifeVonMs == doctest::Approx(400.0));
        // aus und wieder an: die Stellen bleiben
        schleife->setChecked(false);
        CHECK_FALSE(creator.schicht().schleife);
        CHECK(bild->griff(KurvenBild::GriffVon) < 0.0);
        schleife->setChecked(true);
        CHECK(creator.schicht().schleifeVonMs == doctest::Approx(400.0));

        // --- Anspielen: die Vorhoer-Datei traegt die Schicht, wie sie steht
        QStringList toene;
        QObject::connect(&creator, &InstrumentCreator::tonVerlangt, &creator,
                         [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(hoehe).arg(nennung); });
        REQUIRE(creator.spieleAn(57));
        Instrument vorgehoert;
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        CHECK(vorgehoert.schichten[0] == creator.schicht());

        // --- Reference tone: die Sample-Schicht allein, dazu ein Sinus derselben Note
        CHECK_FALSE(referenz->isHidden());
        CHECK_FALSE(creator.referenzton());
        creator.aendere([](Instrument& i) { i.schichten.push_back(Schicht{}); });  // eine zweite Schicht: sie spielt nicht mit
        creator.aendereSchicht([](Schicht& s) { s.versatzHalbtoene = 12; });
        referenz->setChecked(true);
        CHECK(creator.referenzton());
        REQUIRE(creator.spieleAn(57));
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        REQUIRE(vorgehoert.schichten.size() == 2);
        CHECK(vorgehoert.schichten[0] == creator.schicht());
        CHECK(vorgehoert.schichten[1].name == QStringLiteral("Reference tone"));
        CHECK(vorgehoert.schichten[1].schwingung == Schwingung::Sinus);
        CHECK(vorgehoert.schichten[1].versatzHalbtoene == 12);  // die Note, die die Schicht spielen soll
        CHECK(vorgehoert.schichten[1].obertoene.empty());
        // bei einer gerechneten Schicht gibt es ihn nicht: der Haken ist weg und gilt nicht
        creator.waehle(1);
        CHECK(referenz->isHidden());
        CHECK(detect->isHidden());
        CHECK(gemessenText->isHidden());
        REQUIRE(creator.spieleAn(57));
        REQUIRE(holeInstrument(InstrumentCreator::vorhoerDatei(), vorgehoert));
        CHECK(vorgehoert.schichten.size() == 2);
        CHECK(vorgehoert.schichten[1].name.isEmpty());  // das Instrument, wie es ist
        creator.waehle(0);
        CHECK_FALSE(gemessenText->isHidden());  // zurueck bei der Schicht, an der gemessen wurde
        creator.setReferenzton(false);
        CHECK_FALSE(referenz->isChecked());
        creator.aendere([](Instrument& i) { i.schichten.resize(1); });  // die zweite Schicht wieder weg
        creator.aendereSchicht([](Schicht& s) { s.versatzHalbtoene = 0; });
        REQUIRE(creator.instrument().schichten.size() == 1);
        REQUIRE(creator.schicht().schwingung == Schwingung::Sample);

        // --- gespeichert und wieder geoeffnet: alles ist noch da
        const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("cello.lvinst"));
        REQUIRE(creator.speichereUnter(pfad));
        Instrument gelesen;
        REQUIRE(ladeInstrument(pfad, gelesen));
        CHECK(gelesen.schichten[0] == creator.schicht());
        creator.oeffne(gelesen);
        CHECK(dateiZeile->text() == QStringLiteral("cello_a3.wav · 2.0 s · My samples"));
        CHECK(grundton->currentText() == QStringLiteral("A3"));
        CHECK(schleife->isChecked());

        // --- File..., am Ort gelassen: die Schicht nennt den Pfad; eine andere Aufnahme beginnt von vorn
        const QString andere = datei(QStringLiteral("quelle/andere.wav"), "BBBBBB");
        REQUIRE(creator.nimmSampleDatei(andere, false));
        CHECK(creator.schicht().sample == andere);
        CHECK(creator.schicht().sampleStartMs == 0.0);
        CHECK_FALSE(creator.schicht().schleife);
        CHECK(creator.schicht().grundton == 57);  // der Grundton bleibt, bis man ihn stellt
        CHECK(dateiZeile->text() == QStringLiteral("andere.wav · 2.0 s"));
        CHECK(QDir(tmp.path()).filePath(QStringLiteral("samples")) == eigenerSampleOrdner());
        CHECK(QDir(eigenerSampleOrdner()).entryList(QDir::Files).size() == 1);  // nichts dazukopiert
        // was sich nicht kopieren laesst
        CHECK_FALSE(creator.nimmSampleDatei(QDir(tmp.path()).filePath(QStringLiteral("fehlt.wav")), true));
        CHECK(creator.schicht().sample == andere);
        // was an der vorigen Aufnahme gemessen wurde, gilt fuer diese nicht
        CHECK(gemessenText->isHidden());
        CHECK(apply->isHidden());
        CHECK_FALSE(creator.gemessen().gefunden);

        // --- Detect ohne klaren Ton: Rauschen — es steht da, zu uebernehmen gibt es nichts
        const QString rausch = datei(QStringLiteral("quelle/rauschen.wav"), "CC");
        {
            std::vector<float> daten(2 * 48000);
            std::uint32_t x = 4711u;
            for (std::size_t t = 0; t < 48000; ++t)
            {
                x = x * 1664525u + 1013904223u;
                daten[2 * t] = daten[2 * t + 1] = static_cast<float>(0.8 * (static_cast<double>(x >> 8) / 8388608.0 - 1.0));
            }
            merkeSampleDaten(rausch, 48000, std::move(daten));
        }
        REQUIRE(creator.nimmSampleDatei(rausch, false));
        CHECK_FALSE(creator.messe());
        CHECK_FALSE(gemessenText->isHidden());
        CHECK(gemessenText->text() == QStringLiteral("No clear pitch found"));
        CHECK(apply->isHidden());
        CHECK_FALSE(creator.uebernimmGemessenes());
        CHECK(creator.schicht().grundton == 57);
        REQUIRE(creator.nimmSampleDatei(andere, false));

        // --- Record... (Stufe 11b): das Fenster „Record sample" — Save and use setzt die Aufnahme in die Schicht
        auto* recordKnopf = creator.findChild<QPushButton*>(QStringLiteral("creatorSampleRecord"));
        REQUIRE(recordKnopf != nullptr);
        CHECK_FALSE(recordKnopf->isHidden());
        CHECK(creator.aufnahmeFenster() == nullptr);
        // ohne Engine: das Fenster oeffnet sich und sagt, dass es nicht aufnehmen kann
        REQUIRE(creator.oeffneAufnahme(false));
        AufnahmeFenster* aufnahme = creator.aufnahmeFenster();
        REQUIRE(aufnahme != nullptr);
        CHECK(aufnahme->findChild<QLabel*>(QStringLiteral("recordState"))->text() ==
              QStringLiteral("No audio engine — recording is not available"));
        // mit Engine: gefragt wird beim Oeffnen
        papp::AufnahmeEngine aufnahmeEngine;
        aufnahmeEngine.eingaenge = {papp::eingang(0, QStringLiteral("Default"), true)};
        creator.setEngineGeber([&aufnahmeEngine]() -> IAudioEngine* { return &aufnahmeEngine; });
        REQUIRE(creator.oeffneAufnahme(false));
        CHECK(creator.aufnahmeFenster() == aufnahme);  // dasselbe Fenster, kein zweites
        CHECK(aufnahme->findChild<QPushButton*>(QStringLiteral("recordStart"))->isEnabled());
        CHECK(aufnahme->vorschlagTitel() == QStringLiteral("Recording"));
        {
            std::vector<float> ton(2 * 48000);  // eine Sekunde
            for (std::size_t t = 0; t < 48000; ++t)
                ton[2 * t] = ton[2 * t + 1] = static_cast<float>(0.5 * std::sin(2.0 * 3.14159265358979 * 220.0 * static_cast<double>(t) / 48000.0));
            aufnahme->setzeAufnahme(std::move(ton), 48000);
        }
        verlangt.clear();
        creator.aendereSchicht([](Schicht& s) { s.sampleStartMs = 100.0; });
        CHECK(aufnahme->speichereAls(QStringLiteral("Mein Ton")) == QStringLiteral("user:mein_ton"));
        CHECK(creator.schicht().sample == QStringLiteral("user:mein_ton"));
        CHECK(creator.schicht().sampleStartMs == 0.0);  // eine andere Aufnahme beginnt von vorn
        CHECK(QFileInfo(QDir(eigenerSampleOrdner()).filePath(QStringLiteral("mein_ton.wav"))).size() == 44 + 4 * 48000);
        CHECK(verlangt == QStringList{QStringLiteral("user:mein_ton")});  // das Bild verlangt die neue Aufnahme
        CHECK(dateiZeile->text().startsWith(QStringLiteral("mein_ton.wav · ")));
        CHECK(dateiZeile->text().endsWith(QStringLiteral(" · My samples")));
        CHECK(gemessenText->isHidden());  // was gemessen war, galt der Aufnahme davor
        // Record... noch einmal: von vorn, mit dem naechsten freien Namen
        REQUIRE(creator.oeffneAufnahme(false));
        CHECK(aufnahme->frames() == 0);
        CHECK(aufnahme->vorschlagTitel() == QStringLiteral("Recording"));
        // dasselbe Sample neu aufgenommen (seit Stufe 17 ueber Save an einem geoeffneten):
        // die Schicht nennt dasselbe, die Daten werden neu verlangt
        aufnahme->bearbeite(QStringLiteral("user:mein_ton"), std::vector<float>(2 * 48000, 0.1f), 48000);
        aufnahme->setzeAufnahme(std::vector<float>(2 * 24000, 0.25f), 48000);
        verlangt.clear();
        CHECK(aufnahme->speichere() == QStringLiteral("user:mein_ton"));
        CHECK(creator.schicht().sample == QStringLiteral("user:mein_ton"));
        CHECK(verlangt == QStringList{QStringLiteral("user:mein_ton")});

        // --- Edit... (Stufe 17b; M1 a, M3 a, M4 a): was die Schicht spielt, im selben Fenster zuschneiden
        auto* editKnopf = creator.findChild<QPushButton*>(QStringLiteral("creatorSampleEdit"));
        REQUIRE(editKnopf != nullptr);
        CHECK(editKnopf->text() == QStringLiteral("Edit..."));
        CHECK_FALSE(editKnopf->isHidden());
        CHECK(editKnopf->isEnabled());
        CHECK(creator.bearbeitenHindernis().isEmpty());
        REQUIRE(creator.oeffneBearbeiten(false));
        CHECK(creator.aufnahmeFenster() == aufnahme);  // dasselbe Fenster wie bei Record...
        CHECK(aufnahme->nennung() == QStringLiteral("user:mein_ton"));
        CHECK(aufnahme->windowTitle() == QStringLiteral("Edit sample – mein_ton"));
        CHECK(aufnahme->frames() == 96000);  // die zwei Sekunden, die der Test herausgibt
        CHECK(aufnahme->ausschnitt() == Ausschnitt{0, 96000});
        CHECK(aufnahme->findChild<QPushButton*>(QStringLiteral("recordSave"))->isEnabled());
        // Save, vorn 250 ms weg: Sample start und die Schleife ruecken mit, die Daten werden neu verlangt
        creator.aendereSchicht([](Schicht& s) {
            s.sampleStartMs = 400.0;
            s.schleife = true;
            s.schleifeVonMs = 600.0;
            s.schleifeBisMs = 1200.0;
        });
        aufnahme->setzeAusschnitt({12000, 72000});
        verlangt.clear();
        CHECK(aufnahme->speichere() == QStringLiteral("user:mein_ton"));
        CHECK(creator.schicht().sample == QStringLiteral("user:mein_ton"));
        CHECK(creator.schicht().sampleStartMs == doctest::Approx(150.0));
        CHECK(creator.schicht().schleife);
        CHECK(creator.schicht().schleifeVonMs == doctest::Approx(350.0));
        CHECK(creator.schicht().schleifeBisMs == doctest::Approx(950.0));
        CHECK(verlangt == QStringList{QStringLiteral("user:mein_ton")});
        CHECK(QFileInfo(QDir(eigenerSampleOrdner()).filePath(QStringLiteral("mein_ton.wav"))).size() == 44 + 4 * 60000);
        // Save as..., weiter vorn geschnitten als der Einsatz: er bleibt bei null; die Schicht spielt die Kopie
        REQUIRE(creator.oeffneBearbeiten(false));
        aufnahme->setzeAusschnitt({24000, 96000});
        CHECK(aufnahme->vorschlagTitel() == QStringLiteral("mein_ton 2"));
        CHECK(aufnahme->speichereAls(aufnahme->vorschlagTitel()) == QStringLiteral("user:mein_ton_2"));
        CHECK(creator.schicht().sample == QStringLiteral("user:mein_ton_2"));
        CHECK(creator.schicht().sampleStartMs == 0.0);
        CHECK(creator.schicht().schleifeVonMs == 0.0);
        CHECK(creator.schicht().schleifeBisMs == doctest::Approx(450.0));
        CHECK(creator.schicht().schleife);
        CHECK(QFileInfo(QDir(eigenerSampleOrdner()).filePath(QStringLiteral("mein_ton.wav"))).size() == 44 + 4 * 60000);  // unberuehrt
        CHECK(dateiZeile->text().startsWith(QStringLiteral("mein_ton_2.wav · ")));
        // die Schleife ganz weggeschnitten: sie ist aus
        REQUIRE(creator.oeffneBearbeiten(false));
        aufnahme->setzeAusschnitt({48000, 96000});
        CHECK(aufnahme->speichere() == QStringLiteral("user:mein_ton_2"));
        CHECK_FALSE(creator.schicht().schleife);
        // ein Klang des Kits: schreibgeschuetzt — Save ist grau, Save as... legt ihn zu den eigenen
        creator.setzeSample(QStringLiteral("kit:kick"));
        REQUIRE(QFileInfo::exists(sampleDatei(QStringLiteral("kit:kick"))));
        CHECK(editKnopf->isEnabled());
        REQUIRE(creator.oeffneBearbeiten(false));
        CHECK(aufnahme->windowTitle() == QStringLiteral("Edit sample – kick"));
        CHECK(aufnahme->herkunft() == AufnahmeFenster::Herkunft::Mitgeliefert);
        CHECK_FALSE(aufnahme->findChild<QPushButton*>(QStringLiteral("recordSave"))->isEnabled());
        CHECK(aufnahme->speichere().isEmpty());
        aufnahme->setzeAusschnitt({0, 24000});
        CHECK(aufnahme->speichereAls(QStringLiteral("Mein Kick")) == QStringLiteral("user:mein_kick"));
        CHECK(creator.schicht().sample == QStringLiteral("user:mein_kick"));
        // Record... nach Edit...: wieder von vorn, ohne Sample
        REQUIRE(creator.oeffneAufnahme(false));
        CHECK(aufnahme->nennung().isEmpty());
        CHECK(aufnahme->windowTitle() == QStringLiteral("Record sample"));
        // laenger als 30 Sekunden: Edit... oeffnet es nicht und sagt, warum
        const QString lang = datei(QStringLiteral("quelle/lang.wav"), "DD");
        merkeSampleDaten(lang, 48000, std::vector<float>(2 * 48000 * 31, 0.1f));
        REQUIRE(creator.nimmSampleDatei(lang, false));
        CHECK(editKnopf->isEnabled());
        CHECK_FALSE(creator.oeffneBearbeiten(false));
        CHECK(creator.bearbeitenHindernis() == QStringLiteral("This sample is longer than 30 s. Edit... works on samples up to 30 s."));
        // ohne Sample: grau, nichts zu oeffnen
        creator.setzeSample(QString());
        CHECK_FALSE(editKnopf->isEnabled());
        CHECK_FALSE(creator.oeffneBearbeiten(false));
        CHECK(creator.bearbeitenHindernis().isEmpty());
        creator.setzeSample(QStringLiteral("user:mein_ton"));

        // --- ein Sample, das es nicht gibt: die Zeile sagt es, gefragt wird einmal
        verlangt.clear();
        creator.setzeSample(QStringLiteral("user:gibt_es_nicht"));
        CHECK(dateiZeile->text() == QStringLiteral("gibt_es_nicht (not found)"));
        CHECK(bildTitel->text() == QStringLiteral("The sample (not found)"));
        CHECK(bild->werte().empty());
        CHECK_FALSE(schleife->isEnabled());
        CHECK_FALSE(detect->isEnabled());  // ohne Aufnahme nichts zu messen
        CHECK_FALSE(creator.messe());
        CHECK(creator.sampleLaengeMs() == 0.0);
        schieber("creatorFineTune")->setValue(-5);  // ein Zug an einem Regler fragt nicht wieder
        CHECK(verlangt == QStringList{QStringLiteral("user:gibt_es_nicht")});

        // --- die Wahl aus S2 a wird gemerkt und laesst sich zuruecknehmen
        CHECK(sampleKopieWahl() == SampleKopie::Fragen);
        setzeSampleKopieWahl(SampleKopie::Kopieren);
        CHECK(sampleKopieWahl() == SampleKopie::Kopieren);
        setzeSampleKopieWahl(SampleKopie::AmOrt);
        CHECK(sampleKopieWahl() == SampleKopie::AmOrt);
        setzeSampleKopieWahl(SampleKopie::Fragen);
        CHECK(sampleKopieWahl() == SampleKopie::Fragen);

        // --- zurueck zu einer gerechneten Schicht: die Zeilen gehen wieder
        const int alsSinus = welle->findData(static_cast<int>(Schwingung::Sinus));
        welle->setCurrentIndex(alsSinus);
        Q_EMIT welle->activated(alsSinus);
        CHECK(dateiZeile->isHidden());
        CHECK_FALSE(schieber("creatorVoices")->isHidden());
        CHECK(bildTitel->text() == QStringLiteral("One cycle"));
        CHECK(bild->griff(KurvenBild::GriffStart) < 0.0);
        CHECK(bild->werte().size() > 300);
        CHECK(recordKnopf->isHidden());
        CHECK_FALSE(creator.oeffneAufnahme(false));  // Record... gibt es nur bei einer Sample-Schicht

        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_SAMPLE_BILD"); !ziel.isEmpty())
        {
            // wie in der Skizze: eine Sample-Schicht mit Sustain loop, darunter ein Sinus
            vergissSampleDaten();  // fuer das Bild eine Aufnahme, die ausklingt
            InstrumentCreator gezeigt;
            QObject::connect(&gezeigt, &InstrumentCreator::sampleVerlangt, &gezeigt, [](const QString& nennung) {
                std::vector<float> daten(2 * 115200);  // 2,4 Sekunden, klingt aus
                for (std::size_t t = 0; t < 115200; ++t)
                    daten[2 * t] = daten[2 * t + 1] =
                        static_cast<float>(std::sin(2.0 * 3.14159265358979 * 220.0 * static_cast<double>(t) / 48000.0) *
                                           std::exp(-static_cast<double>(t) / 60000.0) * std::min(1.0, static_cast<double>(t) / 2400.0));
                merkeSampleDaten(nennung, 48000, std::move(daten));
            });
            Instrument cello;
            cello.titel = QStringLiteral("My Cello");
            cello.art = QStringLiteral("strings");
            cello.genres = {QStringLiteral("Ambient")};
            cello.beschreibung = QStringLiteral("One recorded cello note");
            Schicht& s = cello.schichten[0];
            s.name = QStringLiteral("Cello");
            s.schwingung = Schwingung::Sample;
            s.sample = QStringLiteral("user:cello_a3");
            s.grundton = 57;
            s.feinCent = 12.0;
            s.sampleStartMs = 20.0;
            s.schleife = true;
            s.schleifeVonMs = 1100.0;
            s.schleifeBisMs = 1850.0;
            s.halten = 1.0;
            s.ausklangMs = 300.0;
            Schicht koerper;
            koerper.name = QStringLiteral("Body");
            cello.schichten.push_back(koerper);
            gezeigt.oeffne(cello);
            (void)gezeigt.messe();  // damit das Ergebnis von Detect im Bild steht
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(ziel));
        }
    }

    TEST_CASE("Instrument-Creator: die Zeile Signal path — Kaesten waehlen, Add, Move, Remove und ziehen")
    {
        // Stufe 16c (S118); Skizze gebilligt, Entscheid Patrik S1 a. Ohne gezeigtes Fenster.
        // `LUMIVIZ_CREATOR_PATH_BILD=<datei.png>` legt zwei Bilder ab: die Schicht aus der Skizze mit
        // gewaehltem Filter, und (mit `_8` im Namen) dieselbe mit acht Bausteinen.
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
        SignalWeg* weg = creator.signalWeg();
        REQUIRE(weg != nullptr);
        weg->resize(1400, 40);  // breit genug fuer eine Zeile: die Kaesten stehen nebeneinander
        auto* titel = creator.findChild<QLabel*>(QStringLiteral("creatorPathTitle"));
        auto* kasten = creator.findChild<QLabel*>(QStringLiteral("creatorBlockTitle"));
        auto* welle = creator.findChild<QComboBox*>(QStringLiteral("creatorWaveform"));
        auto* art = creator.findChild<QComboBox*>(QStringLiteral("creatorFilterType"));
        REQUIRE(titel != nullptr);
        REQUIRE(kasten != nullptr);
        REQUIRE(welle != nullptr);
        REQUIRE(art != nullptr);
        CHECK(creator.findChild<QLabel*>(QStringLiteral("creatorPathHint")) != nullptr);  // sagt, dass man klicken und ziehen kann
        const auto namen = [&] {
            QStringList alle;
            for (const WegKasten& einer : weg->kaesten()) alle << einer.titel;
            return alle;
        };
        const auto reihe = [&]() -> const std::vector<SchichtBaustein>& { return creator.schicht().reihe; };

        // --- ein neues Instrument: Source, Envelope, Level — Source ist gewaehlt und zeigt den Oscillator
        Instrument frisch;
        frisch.titel = QStringLiteral("Fresh");
        creator.oeffne(frisch);
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Envelope"), QStringLiteral("Level")});
        CHECK(creator.stelle() == InstrumentCreator::kStelleSource);
        CHECK(weg->gewaehlt() == 0);
        CHECK(titel->text() == QString::fromUtf8("Signal path — Layer 1"));
        CHECK(kasten->text() == QStringLiteral("Source"));
        CHECK(weg->kaesten()[0].wert == QStringLiteral("Sine"));
        CHECK(weg->kaesten()[1].gestrichelt);  // die Envelope gibt es genau einmal
        CHECK(weg->kaesten()[1].wert == QStringLiteral("Decay 200 ms"));
        CHECK(weg->kaesten()[2].wert == QStringLiteral("100 %"));
        CHECK(weg->addMoeglich());
        CHECK(weg->addText() == QStringLiteral("+ Add"));
        CHECK_FALSE(welle->isHidden());
        CHECK_FALSE(schieber("creatorTranspose")->isHidden());
        CHECK(schieber("creatorAttack")->isHidden());
        CHECK(schieber("creatorLevel")->isHidden());
        CHECK(knopf("creatorBlockLeft")->isHidden());  // Source laesst sich weder schieben noch entfernen
        CHECK(knopf("creatorBlockRemove")->isHidden());
        CHECK_FALSE(weg->ziehbar(0));
        CHECK(weg->ziehbar(1));
        CHECK_FALSE(weg->ziehbar(2));

        // --- ein Klick auf Level: nur sein Regler
        klick(*weg, weg->kastenRect(2).center());
        CHECK(creator.stelle() == InstrumentCreator::kStelleLevel);
        CHECK(kasten->text() == QStringLiteral("Level"));
        CHECK_FALSE(schieber("creatorLevel")->isHidden());
        CHECK(welle->isHidden());
        CHECK(knopf("creatorBlockRemove")->isHidden());
        schieber("creatorLevel")->setValue(80);
        CHECK(weg->kaesten()[2].wert == QStringLiteral("80 %"));

        // --- ein Klick auf Envelope: Attack bis Release; sie laesst sich nicht entfernen
        klick(*weg, weg->kastenRect(1).center());
        CHECK(creator.stelle() == 0);
        CHECK(kasten->text() == QStringLiteral("Envelope"));
        CHECK_FALSE(schieber("creatorAttack")->isHidden());
        CHECK_FALSE(schieber("creatorRelease")->isHidden());
        CHECK(schieber("creatorLevel")->isHidden());
        CHECK_FALSE(knopf("creatorBlockRemove")->isHidden());
        CHECK_FALSE(knopf("creatorBlockRemove")->isEnabled());
        CHECK_FALSE(knopf("creatorBlockLeft")->isEnabled());  // der einzige Baustein
        CHECK_FALSE(knopf("creatorBlockRight")->isEnabled());
        CHECK_FALSE(creator.entferneBaustein());
        schieber("creatorDecay")->setValue(21);
        CHECK(weg->kaesten()[1].wert == QStringLiteral("Decay 210 ms"));

        // --- Add: ein Filter kommt hinter den gewaehlten und ist gewaehlt
        REQUIRE(creator.fuegeFilterEin());
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Envelope"), QStringLiteral("Filter"), QStringLiteral("Level")});
        CHECK(creator.stelle() == 1);
        CHECK(weg->gewaehlt() == 2);
        CHECK(kasten->text() == QStringLiteral("Filter"));
        CHECK(ist<SchichtHuelle>(reihe()[0]));
        CHECK(std::get<SchichtFilter>(reihe()[1]) == SchichtFilter{FilterArt::Tiefpass, 4.0, 0.0, 0.0});  // zu hoeren, nicht offen
        CHECK(weg->kaesten()[2].wert == QString::fromUtf8("Low-pass ×4.0"));
        CHECK_FALSE(schieber("creatorCutoff")->isHidden());
        CHECK(schieber("creatorCutoff")->value() == 40);
        CHECK(schieber("creatorAttack")->isHidden());
        schieber("creatorCutoff")->setValue(23);
        art->setCurrentIndex(art->findText(QStringLiteral("High-pass")));
        Q_EMIT art->activated(art->currentIndex());
        CHECK(std::get<SchichtFilter>(reihe()[1]) == SchichtFilter{FilterArt::Hochpass, 2.3, 0.0, 0.0});
        CHECK(weg->kaesten()[2].wert == QString::fromUtf8("High-pass ×2.3"));

        // --- ← Move: der Filter rueckt vor die Envelope
        CHECK(knopf("creatorBlockLeft")->isEnabled());
        CHECK_FALSE(knopf("creatorBlockRight")->isEnabled());
        knopf("creatorBlockLeft")->click();
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Filter"), QStringLiteral("Envelope"), QStringLiteral("Level")});
        CHECK(creator.stelle() == 0);  // er bleibt gewaehlt
        CHECK_FALSE(knopf("creatorBlockLeft")->isEnabled());
        CHECK(knopf("creatorBlockRight")->isEnabled());

        // --- Add bei gewaehltem Source: vorn
        klick(*weg, weg->kastenRect(0).center());
        REQUIRE(creator.fuegeDriveEin());
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Drive"), QStringLiteral("Filter"), QStringLiteral("Envelope"),
                                     QStringLiteral("Level")});
        CHECK(creator.stelle() == 0);
        CHECK(schieber("creatorDrive")->value() == 30);
        CHECK_FALSE(schieber("creatorDrive")->isHidden());
        schieber("creatorDrive")->setValue(40);
        CHECK(weg->kaesten()[1].wert == QStringLiteral("40 %"));
        CHECK(std::get<SchichtDrive>(reihe()[0]).wert == doctest::Approx(0.4));

        // --- ziehen: Drive hinter die Envelope (der Kick aus §4h) — er ist danach gewaehlt
        ziehe(*weg, weg->kastenRect(1).center(), weg->kastenRect(3).center());
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Filter"), QStringLiteral("Envelope"), QStringLiteral("Drive"),
                                     QStringLiteral("Level")});
        CHECK(creator.stelle() == 2);
        CHECK(ist<SchichtDrive>(reihe()[2]));
        // die Envelope laesst sich auch ziehen; ein fester Kasten nicht
        ziehe(*weg, weg->kastenRect(2).center(), weg->kastenRect(1).center());
        CHECK(ist<SchichtHuelle>(reihe()[0]));
        CHECK(creator.stelle() == 0);
        const std::vector<SchichtBaustein> vorher = reihe();
        ziehe(*weg, weg->kastenRect(0).center(), weg->kastenRect(2).center());
        CHECK(reihe() == vorher);
        CHECK(creator.stelle() == InstrumentCreator::kStelleSource);  // der Klick hat ihn nur gewaehlt
        // ein Klick ohne Zug verschiebt nichts
        klick(*weg, weg->kastenRect(3).center());
        CHECK(reihe() == vorher);
        CHECK(creator.stelle() == 2);

        // --- Remove: der gewaehlte Baustein geht, gewaehlt ist danach sein Nachbar
        CHECK(knopf("creatorBlockRemove")->isEnabled());
        knopf("creatorBlockRemove")->click();
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Envelope"), QStringLiteral("Filter"), QStringLiteral("Level")});
        CHECK(creator.stelle() == 1);

        // --- zwei Filter: jeder hat seine Werte, die Regler stellen den gewaehlten
        REQUIRE(creator.fuegeFilterEin());
        CHECK(creator.stelle() == 2);
        schieber("creatorCutoff")->setValue(80);
        CHECK(std::get<SchichtFilter>(reihe()[2]).teilton == doctest::Approx(8.0));
        CHECK(std::get<SchichtFilter>(reihe()[1]).teilton == doctest::Approx(2.3));
        CHECK(std::get<SchichtFilter>(reihe()[1]).art == FilterArt::Hochpass);
        klick(*weg, weg->kastenRect(2).center());  // der erste Filter
        CHECK(schieber("creatorCutoff")->value() == 23);
        CHECK(art->currentText() == QStringLiteral("High-pass"));
        klick(*weg, weg->kastenRect(3).center());  // der zweite
        CHECK(schieber("creatorCutoff")->value() == 80);
        CHECK(art->currentText() == QStringLiteral("Low-pass"));
        // „open": der Kasten steht matt da — der Filter bleibt in der Reihe
        schieber("creatorCutoff")->setValue(0);
        CHECK(weg->kaesten()[3].wert == QStringLiteral("open"));
        CHECK(weg->kaesten()[3].matt);
        CHECK(reihe().size() == 3);
        schieber("creatorCutoff")->setValue(80);

        // --- der Titel traegt den Namen der Schicht; Source nennt die Schwingung und den Weg der Tonhoehe
        creator.aendereSchicht([](Schicht& s) {
            s.name = QStringLiteral("Body");
            s.tonhoeheStartHt = 22.0;
        });
        CHECK(titel->text() == QString::fromUtf8("Signal path — Layer 1 \"Body\""));
        CHECK(weg->kaesten()[0].wert == QString::fromUtf8("Sine ↓ 22 st"));
        // die Saite hat keinen Filter: der Kasten sagt es
        creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Saite; });
        CHECK(weg->kaesten()[2].wert == QStringLiteral("skipped"));
        CHECK(weg->kaesten()[2].matt);
        CHECK(weg->kaesten()[0].wert == QString::fromUtf8("Plucked string · Classic"));  // nennt das Verfahren (Stufe 23e)
        creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sinus; });

        // --- hoechstens acht (K4 a): danach ist Add grau, und nichts kommt mehr dazu
        while (reihe().size() < static_cast<std::size_t>(kBausteineMax)) REQUIRE(creator.fuegeDriveEin());
        CHECK_FALSE(creator.fuegeDriveEin());
        CHECK_FALSE(creator.fuegeFilterEin());
        CHECK(reihe().size() == 8);
        CHECK(weg->kaesten().size() == 10);
        CHECK_FALSE(weg->addMoeglich());
        CHECK(weg->addText() == QStringLiteral("8 of 8"));
        CHECK(std::count_if(reihe().begin(), reihe().end(), ist<SchichtHuelle>) == 1);
        // in schmaler Breite bricht die Reihe um
        // ein nicht gezeigtes Widget erfaehrt von seiner Groesse erst beim Zeigen: hier von Hand
        const auto breite = [&](int b) {
            weg->resize(b, 40);
            QResizeEvent ereignis(weg->size(), QSize());
            QCoreApplication::sendEvent(weg, &ereignis);
        };
        CHECK(weg->zeilen() == 1);
        breite(420);
        CHECK(weg->zeilen() >= 2);
        CHECK(weg->kastenRect(9).top() > weg->kastenRect(0).top());
        breite(1400);
        // was die Zeile zeigt, ist was gespeichert wird
        Instrument gelesen;
        REQUIRE(ausJson(alsJson(creator.instrument()), gelesen));
        CHECK(gelesen.schichten[0].reihe == reihe());

        // --- eine andere Schicht beginnt bei Source
        klick(*weg, weg->kastenRect(4).center());
        CHECK(creator.stelle() == 3);
        REQUIRE(creator.neueSchicht());
        CHECK(creator.stelle() == InstrumentCreator::kStelleSource);
        CHECK(namen() == QStringList{QStringLiteral("Source"), QStringLiteral("Envelope"), QStringLiteral("Level")});
        CHECK(titel->text() == QString::fromUtf8("Signal path — Layer 2"));
        creator.waehle(0);
        CHECK(creator.stelle() == InstrumentCreator::kStelleSource);
        CHECK(weg->kaesten().size() == 10);

        // --- ein altes Instrument oeffnet als Filter, Drive, Envelope — nur was es hat
        const Instrument* bass = findeInstrument(QStringLiteral("acid_bass"));
        REQUIRE(bass != nullptr);
        creator.oeffne(*bass);
        CHECK(namen().first() == QStringLiteral("Source"));
        CHECK(namen().contains(QStringLiteral("Filter")));
        CHECK(namen().last() == QStringLiteral("Level"));
        CHECK_FALSE(creator.geaendert());  // ansehen und Kaesten waehlen aendert nichts
        klick(*weg, weg->kastenRect(1).center());
        CHECK_FALSE(creator.geaendert());

        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CREATOR_PATH_BILD"); !ziel.isEmpty())
        {
            // wie in der Skizze: Sine, die faellt, durch High-pass, Drive und Envelope
            Instrument skizze;
            skizze.titel = QStringLiteral("Sketch");
            Schicht& s = skizze.schichten[0];
            s.name = QStringLiteral("Body");
            s.tonhoeheStartHt = 22.0;
            s.abfallMs = 210.0;
            s.reihe = {SchichtFilter{FilterArt::Hochpass, 2.3, 0.0, 0.15}, SchichtDrive{0.4}, SchichtHuelle{}};
            InstrumentCreator gezeigt;
            gezeigt.oeffne(skizze);
            gezeigt.waehleStelle(0);
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(ziel));
            gezeigt.waehleStelle(InstrumentCreator::kStelleSource);
            while (gezeigt.fuegeDriveEin()) {}
            QCoreApplication::processEvents();
            QFileInfo info(ziel);
            CHECK(gezeigt.grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_8.") + info.suffix())));
        }
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
        // die Zeile nennt das Instrument als Text (seit 14c keine Dropdown-Liste mehr)
        auto* auswahl = editor.findChild<QLabel*>(QStringLiteral("instrumentSound"));
        REQUIRE(auswahl != nullptr);
        CHECK(auswahl->text() == QStringLiteral("My piano (my_piano.lvinst)"));
        CHECK(QDir::fromNativeSeparators(auswahl->toolTip()) == pfad);

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
            // ein zweites Bild (mit `_level` im Namen): der Kasten Level mit dem Regler Pan (Stufe 18)
            gezeigt.aendereSchicht([](Schicht& s) { s.pan = -0.2; });
            gezeigt.waehleStelle(InstrumentCreator::kStelleLevel);
            QCoreApplication::processEvents();
            const QFileInfo ort(bild);
            CHECK(gezeigt.grab().save(ort.dir().filePath(ort.completeBaseName() + QStringLiteral("_level.") + ort.suffix())));
        }
    }

    TEST_CASE("Instrument-Creator: eine Shape-Schicht — Form und Contour, die Regler, das Bild Shape as it plays")
    {
        // Stufe 19e (S121), Skizze Shape_Konzept §6. `LUMIVIZ_INSTRUMENT_SHAPE_BILD=<datei.png>` legt ein Bild ab.
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());

        InstrumentCreator creator;  // nicht gezeigt: kein Fenster
        Instrument neu;
        neu.titel = QStringLiteral("Shape test");
        creator.oeffne(neu);
        const auto teil = [&creator](const char* kennung) {
            auto* w = creator.findChild<QWidget*>(QLatin1String(kennung));
            REQUIRE(w != nullptr);
            return w;
        };
        const auto schieber = [&creator](const char* kennung) {
            auto* s = creator.findChild<QSlider*>(QLatin1String(kennung));
            REQUIRE(s != nullptr);
            return s;
        };
        auto* welle = creator.findChild<QComboBox*>(QStringLiteral("creatorWaveform"));
        REQUIRE(welle != nullptr);
        const int alsForm = welle->findData(static_cast<int>(Schwingung::Form));
        REQUIRE(alsForm >= 0);
        CHECK(welle->itemText(alsForm) == QStringLiteral("Shape"));

        // --- ein Sinus: von der Form ist nichts zu sehen
        for (const char* kennung : {"creatorShapeName", "creatorShapeBrowse", "creatorShapeContour", "creatorShapeHarmonics",
                                    "creatorShapeHarmonicsEnvelope", "creatorShapeAngle", "creatorShapeStart",
                                    "creatorShapeStereoXY", "creatorShapeReverse", "creatorShapePicture", "creatorShapeLegend"})
        {
            CAPTURE(kennung);
            CHECK(teil(kennung)->isHidden());
        }

        // --- Waveform Shape: zum ersten Mal eine Form — eine mitgelieferte, damit gleich etwas klingt
        welle->setCurrentIndex(alsForm);
        emit welle->activated(alsForm);
        CHECK(creator.schicht().schwingung == Schwingung::Form);
        CHECK(creator.schicht().form == QStringLiteral("heart"));
        for (const char* kennung : {"creatorShapeName", "creatorShapeBrowse", "creatorShapeContour", "creatorShapeHarmonics",
                                    "creatorShapeHarmonicsEnvelope", "creatorShapeAngle", "creatorShapeStart",
                                    "creatorShapeStereoXY", "creatorShapeReverse", "creatorShapePicture", "creatorShapeLegend"})
        {
            CAPTURE(kennung);
            CHECK_FALSE(teil(kennung)->isHidden());
        }
        // Voices, Detune, Pitch sweep und Vibrato gelten wie bei Sinus und Saege; die Obertoene und die Aufnahme nicht
        CHECK_FALSE(schieber("creatorVoices")->isHidden());
        CHECK_FALSE(schieber("creatorPitchSweep")->isHidden());
        CHECK_FALSE(schieber("creatorVibratoRate")->isHidden());
        CHECK(schieber("creatorHarmonic2")->isHidden());
        CHECK(teil("creatorSampleFile")->isHidden());
        auto* name = creator.findChild<QLabel*>(QStringLiteral("creatorShapeName"));
        CHECK(name->text() == QStringLiteral("Heart"));
        // der Kasten Source nennt die Form
        CHECK(creator.signalWeg()->kaesten()[0].wert == QString::fromUtf8("Shape · Heart"));
        auto* kontur = creator.findChild<QComboBox*>(QStringLiteral("creatorShapeContour"));
        REQUIRE(kontur->count() == 2);
        CHECK(kontur->itemText(0) == QStringLiteral("All, one path"));
        CHECK(kontur->itemText(1).startsWith(QStringLiteral("1 ")));
        CHECK(kontur->currentData().toInt() == 0);

        // --- die Regler schreiben in die Schicht
        schieber("creatorShapeHarmonics")->setValue(160);  // 40 Schritte je Verdopplung: 2 hoch 4
        CHECK(creator.schicht().harmonische == doctest::Approx(16.0));
        schieber("creatorShapeHarmonics")->setValue(0);
        CHECK(creator.schicht().harmonische == doctest::Approx(1.0));
        schieber("creatorShapeHarmonics")->setValue(360);
        CHECK(creator.schicht().harmonische == doctest::Approx(512.0));
        schieber("creatorShapeHarmonics")->setValue(184);
        CHECK(creator.schicht().harmonische == doctest::Approx(24.25).epsilon(0.01));
        schieber("creatorShapeHarmonicsEnvelope")->setValue(-20);
        CHECK(creator.schicht().harmonischeHuelle == doctest::Approx(-20.0));
        schieber("creatorShapeAngle")->setValue(90);
        CHECK(creator.schicht().winkelGrad == doctest::Approx(90.0));
        schieber("creatorShapeStart")->setValue(25);
        CHECK(creator.schicht().formStartProzent == doctest::Approx(25.0));
        auto* stereo = creator.findChild<QCheckBox*>(QStringLiteral("creatorShapeStereoXY"));
        auto* zurueck = creator.findChild<QCheckBox*>(QStringLiteral("creatorShapeReverse"));
        stereo->setChecked(true);
        zurueck->setChecked(true);
        CHECK(creator.schicht().stereoXY);
        CHECK(creator.schicht().rueckwaerts);

        // --- das Bild: die Zeichnung gestrichelt, die Form mit den eingestellten Harmonischen, der Pfeil
        FormBild* bild = creator.formBild();
        REQUIRE(bild != nullptr);
        REQUIRE(bild->zeichnung().size() > 100);
        REQUIRE(bild->umriss().size() == bild->zeichnung().size());
        CHECK(bild->winkelGrad() == doctest::Approx(90.0));
        const auto abweichung = [bild] {
            double groesste = 0.0;
            for (qsizetype n = 0; n < bild->umriss().size(); ++n)
                groesste = std::max(groesste, QLineF(bild->umriss()[n], bild->zeichnung()[n]).length());
            return groesste;
        };
        const double mit24 = abweichung();
        schieber("creatorShapeHarmonics")->setValue(0);
        const double mit1 = abweichung();
        schieber("creatorShapeHarmonics")->setValue(360);
        const double mit512 = abweichung();
        CHECK(mit1 > 0.2);     // eine Ellipse liegt weit neben dem Herz
        CHECK(mit24 < mit1);
        CHECK(mit512 < 1.0e-9);  // mit allen Teiltoenen ist es die Zeichnung
        auto* legende = creator.findChild<QLabel*>(QStringLiteral("creatorShapeLegend"));
        CHECK(legende->text().contains(QStringLiteral("512 harmonics")));
        schieber("creatorShapeHarmonics")->setValue(184);

        // --- eine andere Form: ihre Konturen stehen in der Zeile Contour, die Wahl faellt auf den ganzen Pfad
        creator.setzeForm(QStringLiteral("heart_and_star"));
        CHECK(creator.schicht().form == QStringLiteral("heart_and_star"));
        CHECK(name->text() == QStringLiteral("Heart and star"));
        REQUIRE(kontur->count() == 3);
        CHECK(kontur->itemText(1) == QStringLiteral("1 Heart"));
        CHECK(kontur->itemText(2) == QStringLiteral("2 Star"));
        kontur->setCurrentIndex(2);
        emit kontur->activated(2);
        CHECK(creator.schicht().formKontur == 2);
        creator.setzeForm(QStringLiteral("star"));
        CHECK(creator.schicht().formKontur == 0);
        // eine Form, die es nicht gibt: es steht da, das Bild ist leer, nichts stuerzt
        creator.setzeForm(QStringLiteral("gibt_es_nicht"));
        CHECK(name->text() == QStringLiteral("gibt_es_nicht (not found)"));
        CHECK(bild->umriss().isEmpty());
        creator.setzeForm(QStringLiteral("heart"));
        CHECK(bild->umriss().size() > 100);

        // --- anspielen, speichern, wieder oeffnen
        QStringList toene;
        QObject::connect(&creator, &InstrumentCreator::tonVerlangt, &creator,
                         [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(hoehe).arg(nennung); });
        REQUIRE(creator.spieleAn(60));
        CHECK(toene.size() == 1);
        Instrument vorhoer;
        REQUIRE(ladeInstrument(InstrumentCreator::vorhoerDatei(), vorhoer));
        CHECK(vorhoer.schichten[0].schwingung == Schwingung::Form);
        const Klang ton = rendereTon(vorhoer, 60, 200.0, 44100);
        CHECK(std::any_of(ton.daten.begin(), ton.daten.end(), [](float w) { return std::abs(w) > 0.05f; }));
        QFile::remove(InstrumentCreator::vorhoerDatei());
        const QString pfad = ordner.filePath(QStringLiteral("shape_test.lvinst"));
        REQUIRE(creator.speichereUnter(pfad));
        Instrument gelesen;
        REQUIRE(ladeInstrument(pfad, gelesen));
        CHECK(gelesen.schichten[0].form == QStringLiteral("heart"));
        CHECK(gelesen.schichten[0].winkelGrad == doctest::Approx(90.0));
        CHECK(gelesen.schichten[0].stereoXY);
        InstrumentCreator zweiter;
        zweiter.oeffne(gelesen);
        CHECK(zweiter.findChild<QComboBox*>(QStringLiteral("creatorWaveform"))->currentData().toInt() ==
              static_cast<int>(Schwingung::Form));
        CHECK(zweiter.findChild<QLabel*>(QStringLiteral("creatorShapeName"))->text() == QStringLiteral("Heart"));

        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_SHAPE_BILD"); !ziel.isEmpty())
        {
            InstrumentCreator gezeigt;
            Instrument zeig;
            zeig.titel = QStringLiteral("Heart lead");
            zeig.schichten[0].schwingung = Schwingung::Form;
            zeig.schichten[0].form = QStringLiteral("heart");
            zeig.schichten[0].harmonische = 12.0;
            zeig.schichten[0].harmonischeHuelle = 20.0;
            zeig.schichten[0].winkelGrad = 35.0;
            gezeigt.oeffne(zeig);
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(ziel));
        }
    }

    TEST_CASE("Instrument-Editor: mehrere Noten markieren — kopieren und einfuegen — ziehen und versetzen und drehen")
    {
        // Stufe 13c (S113), Skizze §4i; Entscheid V3 a und das Drehen im Kreis.
        // Die Noten: C4 ab 0 (2 lang), E4 ab 2 (1), G4 ab 4 (4), C5 ab 60 (4, bis ans Ende der 64 Schritte)
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        InstrumentEditor editor;  // nicht gezeigt: kein Fenster
        Loop loop = probe();
        loop.muster[0].noten = {{0, 2, 60, 1.0}, {2, 1, 64, 0.5}, {4, 4, 67, 0.8}, {60, 4, 72, 1.0}};
        loop.richte();
        QStringList gemeldet;
        QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            gemeldet << was;
            loop = neu;
            editor.setLoop(loop);  // wie das Panel: die Fassung gilt
        });
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        NotenRaster& raster = *editor.raster();
        raster.resize(kBreite, kHoehe);
        raster.zeigeHoehe(66);  // C4 bis C5 liegen im Ausschnitt
        auto* zeile = editor.findChild<QLabel*>(QStringLiteral("instrumentSelection"));
        REQUIRE(zeile != nullptr);
        // der Haken „Right-click deletes" steht im Editor; seine Stellung merkt die App —
        // hier gilt, was der Fall selbst setzt (der Haken selbst wird im Drum-Editor geprueft)
        CHECK(editor.findChild<QCheckBox*>(QStringLiteral("instrumentRightClickDeletes")) != nullptr);
        raster.setRechtsklickLoescht(false);

        const auto note = [&loop](int schritt, int hoehe) -> const Note* {
            const int platz = findeNote(loop.muster[0], {schritt, hoehe});
            return platz < 0 ? nullptr : &loop.muster[0].noten[static_cast<std::size_t>(platz)];
        };
        const auto auf = [&raster](int schritt, int hoehe) {
            return QPoint(raster.xVonSchritt(schritt) + 4, raster.yVonHoehe(hoehe) + 7);
        };
        const auto mausMit = [&raster](QEvent::Type art, QPoint wo, Qt::MouseButtons tasten, Qt::KeyboardModifiers dazu) {
            QMouseEvent ereignis(art, QPointF(wo), QPointF(raster.mapToGlobal(wo)),
                                 art == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton, tasten, dazu);
            QCoreApplication::sendEvent(&raster, &ereignis);
        };
        const auto taste = [&raster](int key, Qt::KeyboardModifiers dazu = Qt::NoModifier) {
            QKeyEvent ereignis(QEvent::KeyPress, key, dazu);
            QCoreApplication::sendEvent(&raster, &ereignis);
        };
        const auto ctrlKlick = [&](QPoint wo) {
            mausMit(QEvent::MouseButtonPress, wo, Qt::LeftButton, Qt::ControlModifier);
            mausMit(QEvent::MouseButtonRelease, wo, Qt::NoButton, Qt::ControlModifier);
        };

        CHECK(zeile->text().startsWith(QStringLiteral("Shift+drag selects notes")));
        CHECK(raster.einfuegemarke() == 0);

        SUBCASE("Shift+Ziehen zieht einen Rahmen, Ctrl+Klick nimmt eine Note dazu oder heraus")
        {
            // von links oben (in der Zeile von G4) bis rechts unten (C4, Schritt 7)
            mausMit(QEvent::MouseButtonPress, QPoint(raster.xVonSchritt(0) + 2, raster.yVonHoehe(67) + 2), Qt::LeftButton,
                    Qt::ShiftModifier);
            mausMit(QEvent::MouseMove, QPoint(raster.xVonSchritt(7) + 8, raster.yVonHoehe(60) + 10), Qt::LeftButton,
                    Qt::ShiftModifier);
            CHECK(raster.markierte().size() == 3u);  // im Zug schon zu sehen
            mausMit(QEvent::MouseButtonRelease, QPoint(raster.xVonSchritt(7) + 8, raster.yVonHoehe(60) + 10), Qt::NoButton,
                    Qt::ShiftModifier);
            CHECK(raster.markierte().size() == 3u);
            CHECK(gemeldet.isEmpty());  // markieren setzt keine Note und aendert nichts
            CHECK(loop.muster[0].noten.size() == 4u);
            CHECK(zeile->text().startsWith(QStringLiteral("3 notes selected")));

            ctrlKlick(auf(60, 72));
            CHECK(raster.markierte().size() == 4u);
            ctrlKlick(auf(2, 64));
            CHECK(raster.markierte().size() == 3u);
            CHECK_FALSE(raster.istMarkiert(*note(2, 64)));
            ctrlKlick(QPoint(raster.xVonSchritt(20) + 4, raster.yVonHoehe(62) + 7));  // ins Leere: nichts
            CHECK(loop.muster[0].noten.size() == 4u);

            // ein Klick auf eine von mehreren markierten: beim Druck bleiben alle, beim Loslassen nur sie
            maus(raster, QEvent::MouseButtonPress, auf(4, 67), Qt::LeftButton, Qt::LeftButton);
            CHECK(raster.markierte().size() == 3u);
            maus(raster, QEvent::MouseButtonRelease, auf(4, 67), Qt::LeftButton, Qt::NoButton);
            CHECK(raster.markierte() == std::vector<NotenOrt>{{4, 67}});
            CHECK(zeile->text().startsWith(QStringLiteral("1 note selected")));
            taste(Qt::Key_Escape);
            CHECK(raster.markierte().empty());
            taste(Qt::Key_A, Qt::ControlModifier);
            CHECK(raster.markierte().size() == 4u);
            CHECK(gemeldet.isEmpty());
        }
        SUBCASE("Ctrl+C und Ctrl+V: die Noten kommen an der Einfuegemarke dazu, die Marke rueckt weiter")
        {
            raster.markiere({{0, 60}, {2, 64}});
            taste(Qt::Key_C, Qt::ControlModifier);
            CHECK(raster.hatKopie());
            shiftKlick(raster, QPoint(raster.xVonSchritt(16) + 1, 5));  // Shift+Klick in die Zeile der Takte (B4 b)
            CHECK(raster.einfuegemarke() == 16);
            taste(Qt::Key_V, Qt::ControlModifier);
            CHECK(gemeldet.last() == QStringLiteral("Paste notes"));
            REQUIRE(note(16, 60) != nullptr);
            CHECK(note(16, 60)->laenge == 2);  // dieselbe Tonhoehe, dieselbe Laenge
            REQUIRE(note(18, 64) != nullptr);
            CHECK(note(18, 64)->laut == doctest::Approx(0.5));  // die Lautstaerke geht mit
            CHECK(loop.muster[0].noten.size() == 6u);
            CHECK(raster.einfuegemarke() == 19);  // drei Schritte lang war die Kopie
            CHECK(raster.markierte() == std::vector<NotenOrt>{{16, 60}, {18, 64}});

            // am Ende: was hinter dem Muster begaenne, bleibt weg — und der Editor sagt es
            raster.setzeEinfuegemarke(62);
            taste(Qt::Key_V, Qt::ControlModifier);
            CHECK(note(62, 60) != nullptr);
            CHECK(raster.meldung() == QStringLiteral("1 note(s) did not fit into the pattern"));
            CHECK(zeile->text().startsWith(QStringLiteral("1 note(s) did not fit into the pattern")));
            raster.setzeEinfuegemarke(63);
            taste(Qt::Key_V, Qt::ControlModifier);
            REQUIRE(note(63, 60) != nullptr);
            CHECK(note(63, 60)->laenge == 1);
            CHECK(raster.meldung() == QStringLiteral("1 note(s) did not fit into the pattern, 1 note(s) shortened at the "
                                                     "end of the pattern"));
        }
        SUBCASE("eine von mehreren markierten ziehen: alle gehen in Zeit und Tonhoehe mit")
        {
            raster.markiere({{0, 60}, {2, 64}});
            maus(raster, QEvent::MouseButtonPress, auf(0, 60), Qt::LeftButton, Qt::LeftButton);
            maus(raster, QEvent::MouseMove, auf(4, 62), Qt::NoButton, Qt::LeftButton);
            maus(raster, QEvent::MouseButtonRelease, auf(4, 62), Qt::LeftButton, Qt::NoButton);
            CHECK(gemeldet.last() == QStringLiteral("Move notes"));
            CHECK(note(4, 62) != nullptr);
            CHECK(note(6, 66) != nullptr);
            CHECK(note(0, 60) == nullptr);
            CHECK(note(4, 67) != nullptr);  // die nicht markierte G4 bleibt
            CHECK(raster.markierte() == std::vector<NotenOrt>{{4, 62}, {6, 66}});

            // zurueck und ueber den Anfang hinaus: die Gruppe bleibt, wo zuletzt Platz war
            maus(raster, QEvent::MouseButtonPress, auf(4, 62), Qt::LeftButton, Qt::LeftButton);
            maus(raster, QEvent::MouseMove, auf(0, 62), Qt::NoButton, Qt::LeftButton);
            maus(raster, QEvent::MouseMove, QPoint(NotenRaster::kTastenBreite - 40, raster.yVonHoehe(62) + 7), Qt::NoButton,
                 Qt::LeftButton);
            maus(raster, QEvent::MouseButtonRelease, QPoint(NotenRaster::kTastenBreite - 40, raster.yVonHoehe(62) + 7),
                 Qt::LeftButton, Qt::NoButton);
            CHECK(note(0, 62) != nullptr);
            CHECK(note(2, 66) != nullptr);
            CHECK(loop.muster[0].noten.size() == 4u);
        }
        SUBCASE("die Pfeiltasten versetzen — was unterwegs ueberdeckt wird, bleibt; am Rand ist Schluss; Alt dreht")
        {
            raster.markiere({{0, 60}});
            for (int i = 0; i < 4; ++i) taste(Qt::Key_Up);
            CHECK(gemeldet.last() == QStringLiteral("Move notes"));
            CHECK(note(0, 64) != nullptr);
            taste(Qt::Key_Right);
            taste(Qt::Key_Right);
            REQUIRE(note(2, 64) != nullptr);
            CHECK(note(2, 64)->laenge == 2);  // liegt jetzt auf der E4 und ersetzt sie
            taste(Qt::Key_Right);
            REQUIRE(note(2, 64) != nullptr);
            CHECK(note(2, 64)->laenge == 1);  // die ueberdeckte E4 ist wieder da
            CHECK(note(3, 64) != nullptr);
            taste(Qt::Key_Up, Qt::ShiftModifier);
            CHECK(note(3, 76) != nullptr);  // eine Oktave
            CHECK(loop.muster[0].noten.size() == 4u);

            // am Rand (V3 a): C5 reicht schon bis ans Ende
            raster.markiere({{60, 72}});
            const auto vorher = gemeldet.size();
            taste(Qt::Key_Right);
            CHECK(gemeldet.size() == vorher);
            CHECK(note(60, 72) != nullptr);
            // Alt+Pfeil dreht im Kreis: die Note wird am Ende gekuerzt — und bekommt zurueckgedreht ihre Laenge wieder
            taste(Qt::Key_Right, Qt::AltModifier);
            CHECK(gemeldet.last() == QStringLiteral("Rotate notes"));
            REQUIRE(note(61, 72) != nullptr);
            CHECK(note(61, 72)->laenge == 3);
            CHECK(raster.meldung() == QStringLiteral("1 note(s) shortened at the end of the pattern"));
            taste(Qt::Key_Left, Qt::AltModifier);
            REQUIRE(note(60, 72) != nullptr);
            CHECK(note(60, 72)->laenge == 4);
            CHECK(raster.meldung().isEmpty());
        }
        SUBCASE("Ctrl+D haengt die Auswahl an, Entf loescht alle markierten, der Rechtsklick loescht weiter eine Note")
        {
            raster.markiere({{0, 60}, {2, 64}});
            taste(Qt::Key_D, Qt::ControlModifier);
            CHECK(gemeldet.last() == QStringLiteral("Duplicate notes"));
            CHECK(note(3, 60) != nullptr);
            CHECK(note(5, 64) != nullptr);
            CHECK(loop.muster[0].noten.size() == 6u);
            CHECK(raster.markierte() == std::vector<NotenOrt>{{3, 60}, {5, 64}});
            taste(Qt::Key_Delete);
            CHECK(gemeldet.last() == QStringLiteral("Delete notes"));
            CHECK(loop.muster[0].noten.size() == 4u);
            CHECK(raster.markierte().empty());

            raster.markiere({{4, 67}});
            taste(Qt::Key_X, Qt::ControlModifier);
            CHECK(raster.hatKopie());
            CHECK(note(4, 67) == nullptr);
            // ohne den Haken „Right-click deletes" markiert der Rechtsklick die Note nur
            // (das Menue hat „Delete"); mit ihm loescht er sie sofort
            const auto vorRechtsklick = gemeldet.size();
            maus(raster, QEvent::MouseButtonPress, auf(0, 60), Qt::RightButton, Qt::RightButton);
            CHECK(gemeldet.size() == vorRechtsklick);
            CHECK(note(0, 60) != nullptr);
            CHECK(raster.markierte() == std::vector<NotenOrt>{{0, 60}});
            raster.setRechtsklickLoescht(true);
            maus(raster, QEvent::MouseButtonPress, auf(0, 60), Qt::RightButton, Qt::RightButton);
            CHECK(gemeldet.last() == QStringLiteral("Delete note"));
            CHECK(note(0, 60) == nullptr);
            CHECK(loop.muster[0].noten.size() == 2u);
            // auf eine von mehreren markierten loescht er nicht (dort oeffnet sich das Menue)
            raster.markiereAlle();
            const auto vorher = gemeldet.size();
            maus(raster, QEvent::MouseButtonPress, auf(2, 64), Qt::RightButton, Qt::RightButton);
            CHECK(gemeldet.size() == vorher);
            CHECK(loop.muster[0].noten.size() == 2u);
        }
        SUBCASE("das Bild: markierte Noten tragen einen Rand, die Einfuegemarke steht im Kopf")
        {
            raster.markiere({{0, 60}, {2, 64}, {4, 67}});
            raster.setzeEinfuegemarke(16);
            const QImage bild = raster.grab().toImage();
            CHECK_FALSE(bild.isNull());
            if (const QString datei = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_MARKEN_BILD"); !datei.isEmpty())
                CHECK(bild.save(datei));
        }
    }

    TEST_CASE("Instrument-Editor: Load instrument — die Datei kommt zu den eigenen oder bleibt am Ort")
    {
        // Entscheid E3 a (Patrik S117). Die Rueckfrage selbst (ein QMessageBox) wird nicht gezeigt:
        // geprueft ist, was nach ihrer Antwort geschieht. Der Benutzerordner ist ein Wegwerf-Ordner.
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
        struct Zurueck
        {
            ~Zurueck() { setzeEigenenInstrumentOrdnerFuerTests(QString()); }
        } zurueck;

        // zwei fremde Dateien mit demselben Namen an zwei Orten, mit verschiedenem Inhalt
        REQUIRE(QDir(tmp.path()).mkpath(QStringLiteral("a")));
        REQUIRE(QDir(tmp.path()).mkpath(QStringLiteral("b")));
        const QString fremd = QDir(tmp.path()).filePath(QStringLiteral("a/Fremd Klang.lvinst"));
        const QString anders = QDir(tmp.path()).filePath(QStringLiteral("b/Fremd Klang.lvinst"));
        Instrument eins = *findeInstrument(QStringLiteral("strings"));
        eins.titel = QStringLiteral("Fremder Klang");
        REQUIRE(speichereInstrument(eins, fremd));
        Instrument zwei = *findeInstrument(QStringLiteral("organ"));
        zwei.titel = QStringLiteral("Anderer Klang");
        REQUIRE(speichereInstrument(zwei, anders));

        InstrumentEditor editor;  // nicht gezeigt: kein Fenster
        Loop loop = probe();
        QStringList gemeldet;
        QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            gemeldet << was;
            loop = neu;
            editor.setLoop(loop);
        });
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        auto* auswahl = editor.findChild<QLabel*>(QStringLiteral("instrumentSound"));
        REQUIRE(auswahl != nullptr);

        // --- kopieren: der Ordner entsteht, die Datei heisst nach ihrer Datei, das Muster nennt user:<name>
        CHECK_FALSE(QFileInfo::exists(eigenerInstrumentOrdner()));
        REQUIRE(editor.nimmInstrumentDatei(fremd, true));
        CHECK(loop.muster[0].instrument == QStringLiteral("user:fremd_klang"));
        CHECK(gemeldet == QStringList{QStringLiteral("Change instrument")});
        CHECK(QFileInfo::exists(eigeneDatei(QStringLiteral("fremd_klang"))));
        CHECK(QFileInfo::exists(fremd));  // das Original bleibt
        CHECK(auswahl->text().startsWith(QStringLiteral("Fremder Klang")));
        Instrument geholt;
        REQUIRE(holeInstrument(QStringLiteral("user:fremd_klang"), geholt));
        CHECK(geholt.schichten == eins.schichten);

        // --- dieselbe Datei noch einmal: keine zweite Kopie, nichts aendert sich
        REQUIRE(editor.nimmInstrumentDatei(fremd, true));
        CHECK(gemeldet.size() == 1);
        CHECK_FALSE(QFileInfo::exists(eigeneDatei(QStringLiteral("fremd_klang_2"))));
        // ... auch aus dem Ordner der eigenen selbst gewaehlt
        CHECK(nimmZuEigenenInstrumenten(eigeneDatei(QStringLiteral("fremd_klang"))) == QStringLiteral("user:fremd_klang"));

        // --- eine andere Datei mit demselben Namen: sie bekommt eine Zahl, die erste bleibt
        REQUIRE(editor.nimmInstrumentDatei(anders, true));
        CHECK(loop.muster[0].instrument == QStringLiteral("user:fremd_klang_2"));
        REQUIRE(holeInstrument(QStringLiteral("user:fremd_klang"), geholt));
        CHECK(geholt.titel == QStringLiteral("Fremder Klang"));
        REQUIRE(holeInstrument(QStringLiteral("user:fremd_klang_2"), geholt));
        CHECK(geholt.titel == QStringLiteral("Anderer Klang"));
        CHECK(eigeneInstrumente().size() == 2);

        // --- am Ort lassen: im Muster steht der Pfad, kopiert wird nichts
        REQUIRE(editor.nimmInstrumentDatei(fremd, false));
        CHECK(loop.muster[0].instrument == fremd);
        CHECK(eigeneInstrumente().size() == 2);

        // --- was kein Instrument ist, wird nicht kopiert, und das Muster bleibt
        const QString unsinn = QDir(tmp.path()).filePath(QStringLiteral("unsinn.lvinst"));
        {
            QFile datei(unsinn);
            REQUIRE(datei.open(QIODevice::WriteOnly));
            datei.write("kein Instrument");
        }
        const int vorher = static_cast<int>(gemeldet.size());
        CHECK_FALSE(editor.nimmInstrumentDatei(unsinn, true));
        CHECK_FALSE(editor.nimmInstrumentDatei(QDir(tmp.path()).filePath(QStringLiteral("fehlt.lvinst")), true));
        CHECK(gemeldet.size() == vorher);
        CHECK(loop.muster[0].instrument == fremd);
        CHECK(eigeneInstrumente().size() == 2);
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
        // das Instrument steht als Text in der Zeile, daneben Browse... (seit 14c)
        auto* auswahl = editor.findChild<QLabel*>(QStringLiteral("instrumentSound"));
        REQUIRE(auswahl != nullptr);
        CHECK(auswahl->text() == QStringLiteral("Piano"));
        // davor das Datei-Icon der Instrumente (S120)
        auto* soundIcon = editor.findChild<QLabel*>(QStringLiteral("instrumentSoundIcon"));
        REQUIRE(soundIcon != nullptr);
        CHECK_FALSE(soundIcon->pixmap().isNull());
        CHECK(editor.findChild<QComboBox*>(QStringLiteral("instrumentSound")) == nullptr);
        REQUIRE(editor.findChild<QPushButton*>(QStringLiteral("instrumentBrowse")) != nullptr);

        // --- Instrument, Lautstaerke, Panorama, Stumm, Takte
        editor.setzeInstrument(QStringLiteral("strings"));
        CHECK(gemeldet.last() == QStringLiteral("Change instrument"));
        CHECK(loop.muster[0].instrument == QStringLiteral("strings"));
        CHECK(auswahl->text() == QStringLiteral("Strings"));
        // was sich nicht finden laesst, bleibt stehen und sagt es
        CHECK(InstrumentEditor::instrumentAnzeige(QStringLiteral("tuba")) == QStringLiteral("tuba (not found)"));
        CHECK(InstrumentEditor::instrumentAnzeige(QStringLiteral("user:gibt_es_nicht")) ==
              QStringLiteral("user:gibt_es_nicht (not found)"));
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

        // --- ein Instrument, das dieses Programm nicht kennt, bleibt in der Zeile stehen
        Loop fremd = loop;
        musterIn(fremd, QStringLiteral("Mel"))->instrument = QStringLiteral("tuba");
        editor.setLoop(fremd);
        CHECK(auswahl->text() == QStringLiteral("tuba (not found)"));
        editor.setLoop(loop);
        CHECK(auswahl->text() == InstrumentEditor::instrumentAnzeige(loop.muster[0].instrument));
        CHECK_FALSE(auswahl->text().contains(QStringLiteral("not found")));

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
