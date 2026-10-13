/**
 * @file test_LoopDrumEditor.cpp
 * @brief Loop, Stufe 6b: der Drum-Editor — die Funktionen auf dem Muster
 *        (`loop/Bearbeiten`), das Schritt-Raster und das Fenster des Editors,
 *        dazu der Weg ueber das Panel mit Undo
 *        (docs/audio/Loop_Konzept.md §4d)
 *
 * Ohne Ton und ohne Fenster: Raster, Editor und Panel werden nicht gezeigt,
 * die Maus kommt als Ereignis. „Play pattern" und die Spitze der Summe stehen
 * in `test_LoopSpieler.cpp` (dort gibt es die Engine aus Pappe).
 */

#include <doctest.h>

#include "PappAufnahme.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Sample.hpp"
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QToolButton>

#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Eine Basis „Grund" (4 Takte, 16 Schritte je Takt) mit Kick und Snare, in
/// Drums zweimal hintereinander (0 bis 8) und einmal ab Takt 8; dahinter in
/// Drums eine zweite Basis „Fill" ab Takt 12.
Loop probe()
{
    Loop loop;
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 4;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                    {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.8, 0.0, false}};
    grund.zeilen = {std::vector<double>(64, 0.0), std::vector<double>(64, 0.0)};
    for (int takt = 0; takt < 4; ++takt)
    {
        grund.zeilen[0][static_cast<std::size_t>(takt * 16)] = 1.0;       // Schlag 1
        grund.zeilen[0][static_cast<std::size_t>(takt * 16 + 8)] = 0.5;   // Schlag 3
        grund.zeilen[1][static_cast<std::size_t>(takt * 16 + 4)] = 1.0;   // Schlag 2
        grund.zeilen[1][static_cast<std::size_t>(takt * 16 + 13)] = 0.7;  // im Schlag 4, zweites Sechzehntel
    }
    Muster fill;
    fill.name = QStringLiteral("Fill");
    fill.takte = 4;
    loop.muster = {grund, fill};
    loop.kanaele = {{QStringLiteral("Drums"),
                     true,
                     {{QStringLiteral("Grund"), 0, 1}, {QStringLiteral("Grund"), 4, 1}, {QStringLiteral("Fill"), 12, 1}}},
                    {QStringLiteral("Perc"), true, {{QStringLiteral("Grund"), 8, 1}}}};
    return loop;
}

bool inOrdnung(const Loop& loop)
{
    Loop gerichtet = loop;
    QStringList bericht;
    gerichtet.richte(&bericht);
    return bericht.isEmpty() && gerichtet == loop;
}

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

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopDrumEditor"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("LoopDrumEditor")
{
    TEST_CASE("Loop bearbeiten: einen Clip von seiner Basis loesen")
    {
        Loop loop = probe();
        REQUIRE(inOrdnung(loop));
        CHECK(basisNutzer(loop, QStringLiteral("Grund")) == 3);
        CHECK(neueBasisAus(loop, 0, 1) == QStringLiteral("Grund 2"));
        CHECK(loop.kanaele[0].clips[1].muster == QStringLiteral("Grund 2"));
        CHECK(basisNutzer(loop, QStringLiteral("Grund")) == 2);
        REQUIRE(loop.findeMuster(QStringLiteral("Grund 2")) != nullptr);
        // die Kopie traegt denselben Inhalt
        Muster kopie = *loop.findeMuster(QStringLiteral("Grund 2"));
        kopie.name = QStringLiteral("Grund");
        CHECK(kopie == *loop.findeMuster(QStringLiteral("Grund")));
        // der naechste freie Name
        CHECK(neueBasisAus(loop, 1, 0) == QStringLiteral("Grund 3"));
        // der letzte Clip der Basis: das Original geht mit
        CHECK(neueBasisAus(loop, 0, 0) == QStringLiteral("Grund 4"));
        CHECK(loop.findeMuster(QStringLiteral("Grund")) == nullptr);
        CHECK(neueBasisAus(loop, 0, 9).isEmpty());
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Loop bearbeiten: die Laenge des Musters — seine Clips wachsen mit, Clips dahinter ruecken")
    {
        Loop loop = probe();
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("Grund"), 6));
        const Muster* grund = loop.findeMuster(QStringLiteral("Grund"));
        REQUIRE(grund != nullptr);
        CHECK(grund->takte == 6);
        CHECK(grund->zeilen[0].size() == 96);
        CHECK(grund->zeilen[0][48] == 1.0);  // was da war, bleibt
        CHECK(grund->zeilen[0][64] == 0.0);  // angehaengt wird Stille
        // Drums: 0 bis 6, dann 6 bis 12 (rueckt von 4), Fill bleibt bei 12
        CHECK(loop.kanaele[0].clips[0].startTakt == 0);
        CHECK(loop.kanaele[0].clips[1].startTakt == 6);
        CHECK(loop.kanaele[0].clips[2].startTakt == 12);
        CHECK(loop.kanaele[1].clips[0].startTakt == 8);
        CHECK(inOrdnung(loop));

        // noch laenger: jetzt rueckt auch Fill
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("Grund"), 8));
        CHECK(loop.kanaele[0].clips[1].startTakt == 8);
        CHECK(loop.kanaele[0].clips[2].startTakt == 16);
        // kuerzer schneidet hinten ab; die Clips bleiben, wo sie sind
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("Grund"), 4));
        CHECK(loop.findeMuster(QStringLiteral("Grund"))->zeilen[0].size() == 64);
        CHECK(loop.kanaele[0].clips[1].startTakt == 8);
        // in ihren Grenzen
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("Grund"), 1));
        CHECK(loop.findeMuster(QStringLiteral("Grund"))->takte == kTakteMin);
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("Grund"), 999));
        CHECK(loop.findeMuster(QStringLiteral("Grund"))->takte == kTakteMax);
        CHECK_FALSE(setzeMusterTakte(loop, QStringLiteral("gibt es nicht"), 8));
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Loop bearbeiten: das Raster — jeder Schritt bleibt an seiner Stelle im Takt, wo es sie noch gibt")
    {
        Loop loop = probe();
        CHECK_FALSE(setzeRaster(loop, 4, 4));  // schon so
        // groeber: zwei Schritte je Schlag — die Achtel bleiben, die Sechzehntel dazwischen gehen
        REQUIRE(setzeRaster(loop, 4, 2));
        CHECK(loop.schritteJeTakt() == 8);
        const Muster* grund = loop.findeMuster(QStringLiteral("Grund"));
        REQUIRE(grund->zeilen[0].size() == 32);
        CHECK(grund->zeilen[0][0] == 1.0);   // Schlag 1
        CHECK(grund->zeilen[0][4] == 0.5);   // Schlag 3
        CHECK(grund->zeilen[1][2] == 1.0);   // Schlag 2
        CHECK(grund->zeilen[1][6] == 0.0);   // das zweite Sechzehntel im Schlag 4 gibt es nicht mehr
        CHECK(grund->zeilen[1][7] == 0.0);
        CHECK(inOrdnung(loop));

        // feiner: jeder Schritt steht, wo er stand
        Loop fein = probe();
        REQUIRE(setzeRaster(fein, 4, 8));
        const Muster* f = fein.findeMuster(QStringLiteral("Grund"));
        REQUIRE(f->zeilen[1].size() == 128);
        CHECK(f->zeilen[1][8] == 1.0);    // Schlag 2
        CHECK(f->zeilen[1][26] == 0.7);   // Schlag 4, zweites Sechzehntel = drittes Zweiunddreissigstel
        CHECK(f->zeilen[1][25] == 0.0);
        CHECK(inOrdnung(fein));

        // weniger Schlaege je Takt: der vierte Schlag faellt weg, die ersten drei bleiben
        Loop drei = probe();
        REQUIRE(setzeRaster(drei, 3, 4));
        const Muster* d = drei.findeMuster(QStringLiteral("Grund"));
        REQUIRE(d->zeilen[0].size() == 48);
        CHECK(d->zeilen[0][0] == 1.0);
        CHECK(d->zeilen[0][8] == 0.5);
        CHECK(d->zeilen[0][12] == 1.0);  // der naechste Takt beginnt jetzt hier
        CHECK(d->zeilen[1][4] == 1.0);
        CHECK(inOrdnung(drei));
        // in ihren Grenzen
        REQUIRE(setzeRaster(drei, 99, 0));
        CHECK(drei.schlaegeJeTakt == kRasterMax);
        CHECK(drei.schritteJeSchlag == 1);
    }

    TEST_CASE("Loop bearbeiten: Spuren anhaengen und entfernen")
    {
        Loop loop = probe();
        const QStringList kit = {QStringLiteral("kit:clap"), QStringLiteral("kit:kick"), QStringLiteral("kit:snare")};
        CHECK(spurHinzu(loop, QStringLiteral("Grund"), kit) == 2);
        const Muster* grund = loop.findeMuster(QStringLiteral("Grund"));
        CHECK(grund->spuren[2].name == QStringLiteral("Track 1"));
        CHECK(grund->spuren[2].klang == QStringLiteral("kit:clap"));  // der erste, den das Muster noch nicht spielt
        CHECK(grund->zeilen[2].size() == 64);
        CHECK(spurHinzu(loop, QStringLiteral("Grund"), kit) == 3);
        grund = loop.findeMuster(QStringLiteral("Grund"));
        CHECK(grund->spuren[3].name == QStringLiteral("Track 2"));
        CHECK(grund->spuren[3].klang.isEmpty());  // alle vergeben
        // ein leeres Muster bekommt seine erste Spur
        CHECK(spurHinzu(loop, QStringLiteral("Fill"), {}) == 0);
        CHECK(spurHinzu(loop, QStringLiteral("gibt es nicht"), kit) == -1);
        CHECK(inOrdnung(loop));

        CHECK(spurLoeschen(loop, QStringLiteral("Grund"), 0));
        grund = loop.findeMuster(QStringLiteral("Grund"));
        REQUIRE(grund->spuren.size() == 3);
        CHECK(grund->spuren[0].name == QStringLiteral("Snare"));
        CHECK(grund->zeilen[0][4] == 1.0);  // ihre Zeile ist mitgekommen
        CHECK_FALSE(spurLoeschen(loop, QStringLiteral("Grund"), 3));
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Schritt-Raster: ein Klick schaltet, Ziehen malt, unten zieht man die Lautstaerke")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        SchrittRaster raster;  // nicht gezeigt: kein Fenster
        const Loop loop = probe();
        raster.setMuster(loop.muster[0], 4, 4);
        CHECK_FALSE(raster.grab().isNull());

        int aenderungen = 0;
        QString zuletzt;
        QObject::connect(&raster, &SchrittRaster::geaendert, &raster, [&](const Muster&, const QString& was) {
            ++aenderungen;
            zuletzt = was;
        });
        int gewaehlt = -1;
        QObject::connect(&raster, &SchrittRaster::spurGewaehlt, &raster, [&](int spur) { gewaehlt = spur; });

        // Geometrie: 64 Schritte, zwischen den Schlaegen mehr Luft
        const int breit = SchrittRaster::kSchrittBreite + SchrittRaster::kSchrittLuft;
        CHECK(raster.xVonSchritt(1) - raster.xVonSchritt(0) == breit);
        CHECK(raster.xVonSchritt(4) - raster.xVonSchritt(3) == breit + SchrittRaster::kSchlagLuft);
        CHECK(raster.schrittBei(raster.xVonSchritt(13) + 3) == 13);
        CHECK(raster.schrittBei(raster.xVonSchritt(3) + SchrittRaster::kSchrittBreite + 1) == -1);  // die Luecke
        CHECK(raster.schrittBei(raster.xVonSchritt(63) + 3) == 63);
        CHECK(raster.schrittBei(raster.xVonSchritt(64) + 3) == -1);
        CHECK(raster.sizeHint().width() > 64 * breit);
        CHECK(raster.lautOben() == raster.yVonSpur(2) + SchrittRaster::kLuftHoehe);
        CHECK(raster.spur() == 0);  // die erste Spur ist angeklickt

        const auto zelle = [&](int spur, int schritt) {
            return QPoint(raster.xVonSchritt(schritt) + 4, raster.yVonSpur(spur) + SchrittRaster::kZeilenHoehe / 2);
        };

        // ein freier Schritt geht an, mit voller Lautstaerke — und die Spur ist gewaehlt
        klick(raster, zelle(1, 2));
        CHECK(aenderungen == 1);
        CHECK(zuletzt == QStringLiteral("Edit steps"));
        CHECK(raster.muster().zeilen[1][2] == 1.0);
        CHECK(gewaehlt == 1);
        CHECK(raster.spur() == 1);
        // ein gesetzter geht aus
        klick(raster, zelle(1, 4));
        CHECK(raster.muster().zeilen[1][4] == 0.0);
        CHECK(aenderungen == 2);

        // ziehen malt: von einem freien Schritt aus gehen alle ueberfahrenen an, einmal gemeldet
        maus(raster, QEvent::MouseButtonPress, zelle(0, 1), Qt::LeftButton, Qt::LeftButton);
        maus(raster, QEvent::MouseMove, zelle(0, 2), Qt::NoButton, Qt::LeftButton);
        maus(raster, QEvent::MouseMove, zelle(0, 3), Qt::NoButton, Qt::LeftButton);
        maus(raster, QEvent::MouseMove, zelle(1, 3), Qt::NoButton, Qt::LeftButton);  // in die Zeile daneben: es bleibt bei der Spur
        maus(raster, QEvent::MouseButtonRelease, zelle(0, 3), Qt::LeftButton, Qt::NoButton);
        CHECK(aenderungen == 3);
        CHECK(raster.muster().zeilen[0][1] == 1.0);
        CHECK(raster.muster().zeilen[0][2] == 1.0);
        CHECK(raster.muster().zeilen[0][3] == 1.0);
        CHECK(raster.muster().zeilen[1][3] == 0.0);
        // von einem gesetzten aus gehen sie aus; ein Schritt mit halber Lautstaerke auch
        maus(raster, QEvent::MouseButtonPress, zelle(0, 8), Qt::LeftButton, Qt::LeftButton);
        maus(raster, QEvent::MouseMove, zelle(0, 3), Qt::NoButton, Qt::LeftButton);
        maus(raster, QEvent::MouseButtonRelease, zelle(0, 3), Qt::LeftButton, Qt::NoButton);
        CHECK(raster.muster().zeilen[0][8] == 0.0);
        CHECK(raster.muster().zeilen[0][3] == 0.0);
        CHECK(raster.muster().zeilen[0][2] == 1.0);  // nicht ueberfahren (die Maus sprang)
        CHECK(aenderungen == 4);

        // die Lautstaerke je Schritt der angeklickten Spur (Kick): oben voll, in der Mitte halb, ganz unten aus
        REQUIRE(raster.spur() == 0);
        const int oben = raster.lautOben();
        const int hoch = SchrittRaster::kLautHoehe;
        klick(raster, QPoint(raster.xVonSchritt(0) + 4, oben + 3 + (hoch - 6) / 2));
        CHECK(zuletzt == QStringLiteral("Change step volume"));
        CHECK(raster.muster().zeilen[0][0] == doctest::Approx(0.5).epsilon(0.05));
        klick(raster, QPoint(raster.xVonSchritt(5) + 4, oben + 3));  // ein freier Schritt: er geht an
        CHECK(raster.muster().zeilen[0][5] == doctest::Approx(1.0));
        klick(raster, QPoint(raster.xVonSchritt(5) + 4, oben + hoch - 2));
        CHECK(raster.muster().zeilen[0][5] == 0.0);
        const int vorher = aenderungen;
        klick(raster, QPoint(raster.xVonSchritt(6) + 4, oben + hoch - 2));  // aus bleibt aus: nichts zu melden
        CHECK(aenderungen == vorher);

        raster.setMarke(17.5);
        CHECK_FALSE(raster.grab().isNull());
    }

    TEST_CASE("Drum-Editor: mehrere Schritte markieren, kopieren, einfuegen, versetzen und drehen")
    {
        // Stufe 13b (S113), Skizze §4i; Entscheide V1 a, V2 a, V3 a.
        // probe(): je Takt Kick auf Schritt 0 (1,0) und 8 (0,5), Snare auf 4 (1,0) und 13 (0,7)
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        DrumEditor editor;  // nicht gezeigt: kein Fenster
        int aenderungen = 0;
        QString zuletzt;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            ++aenderungen;
            zuletzt = was;
            editor.setLoop(neu);  // wie das Panel: die Fassung kommt zurueck
        });
        REQUIRE(editor.zeige(probe(), QStringLiteral("Grund")));
        SchrittRaster& raster = *editor.raster();
        auto* zeile = editor.findChild<QLabel*>(QStringLiteral("drumSelection"));
        REQUIRE(zeile != nullptr);
        const auto kick = [&editor](int schritt) {
            return editor.loop().findeMuster(QStringLiteral("Grund"))->zeilen[0][static_cast<std::size_t>(schritt)];
        };
        const auto snare = [&editor](int schritt) {
            return editor.loop().findeMuster(QStringLiteral("Grund"))->zeilen[1][static_cast<std::size_t>(schritt)];
        };
        const auto ort = [&raster](int spur, int schritt) {
            return QPoint(raster.xVonSchritt(schritt) + 3, raster.yVonSpur(spur) + 10);
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
        const auto rahmen = [&](int spurVon, int schrittVon, int spurBis, int schrittBis) {
            mausMit(QEvent::MouseButtonPress, ort(spurVon, schrittVon), Qt::LeftButton, Qt::ShiftModifier);
            mausMit(QEvent::MouseMove, ort(spurBis, schrittBis), Qt::LeftButton, Qt::ShiftModifier);
            mausMit(QEvent::MouseButtonRelease, ort(spurBis, schrittBis), Qt::NoButton, Qt::ShiftModifier);
        };

        // ohne Auswahl sagt die Zeile, wie man markiert
        CHECK(zeile->text().startsWith(QStringLiteral("Shift+drag selects steps")));
        CHECK(raster.einfuegemarke() == 0);

        SUBCASE("Shift+Ziehen markiert einen Bereich, Ctrl+Klick einen Schritt, nichts davon aendert das Muster")
        {
            rahmen(0, 0, 1, 15);
            CHECK(raster.markierte().size() == 32u);  // der ganze Bereich, auch die leeren Schritte
            CHECK(raster.istMarkiert(1, 7));
            CHECK(zeile->text().startsWith(QStringLiteral("32 steps in 2 tracks selected")));
            // von rechts unten nach links oben gezogen ist es derselbe Bereich
            rahmen(1, 15, 0, 0);
            CHECK(raster.markierte().size() == 32u);
            // Ctrl+Klick nimmt einen heraus und einen dazu
            mausMit(QEvent::MouseButtonPress, ort(0, 3), Qt::LeftButton, Qt::ControlModifier);
            mausMit(QEvent::MouseButtonRelease, ort(0, 3), Qt::NoButton, Qt::ControlModifier);
            CHECK(raster.markierte().size() == 31u);
            CHECK_FALSE(raster.istMarkiert(0, 3));
            mausMit(QEvent::MouseButtonPress, ort(1, 40), Qt::LeftButton, Qt::ControlModifier);
            mausMit(QEvent::MouseButtonRelease, ort(1, 40), Qt::NoButton, Qt::ControlModifier);
            CHECK(raster.istMarkiert(1, 40));
            CHECK(aenderungen == 0);
            CHECK(kick(3) == 0.0);  // der Ctrl+Klick hat den Schritt nicht gesetzt
            // ein Klick ohne Taste setzt weiter einen Schritt — die Marken bleiben
            klick(raster, ort(0, 3));
            CHECK(kick(3) == 1.0);
            CHECK(raster.markierte().size() == 32u);
            taste(Qt::Key_Escape);
            CHECK(raster.markierte().empty());
            taste(Qt::Key_A, Qt::ControlModifier);
            CHECK(raster.markierte().size() == 128u);
        }
        SUBCASE("Ctrl+C und Ctrl+V: die Kopie ersetzt den Bereich an der Einfuegemarke, die Marke rueckt weiter")
        {
            klick(raster, ort(0, 33));  // im dritten Takt ein Schritt, den der erste nicht hat
            REQUIRE(kick(33) == 1.0);
            rahmen(0, 0, 1, 15);
            taste(Qt::Key_C, Qt::ControlModifier);
            CHECK(raster.hatKopie());
            // ein Klick in die Zeile der Takte setzt die Marke
            klick(raster, QPoint(raster.xVonSchritt(32) + 3, 5));
            CHECK(raster.einfuegemarke() == 32);
            const int vorher = aenderungen;
            taste(Qt::Key_V, Qt::ControlModifier);
            CHECK(aenderungen == vorher + 1);
            CHECK(zuletzt == QStringLiteral("Paste steps"));
            CHECK(kick(33) == 0.0);  // V1 a: wo die Kopie leer ist, ist es danach leer
            CHECK(kick(32) == 1.0);
            CHECK(snare(45) == 0.7);
            CHECK(raster.einfuegemarke() == 48);
            CHECK(raster.markierte().size() == 32u);  // markiert ist, was eingefuegt wurde
            CHECK(raster.istMarkiert(0, 32));
            CHECK(inOrdnung(editor.loop()));
            // hinter dem Ende passt nichts mehr: der Editor sagt es
            klick(raster, QPoint(raster.xVonSchritt(63) + SchrittRaster::kSchrittBreite + 2, 5));
            CHECK(raster.einfuegemarke() == 64);
            taste(Qt::Key_V, Qt::ControlModifier);
            CHECK(aenderungen == vorher + 1);
            CHECK(raster.meldung() == QStringLiteral("4 step(s) did not fit into the pattern"));
            CHECK(zeile->text().startsWith(QStringLiteral("4 step(s) did not fit into the pattern")));
            // halb ueber das Ende: der vordere Teil kommt, der Rest wird gezaehlt
            raster.setzeEinfuegemarke(56);
            taste(Qt::Key_V, Qt::ControlModifier);
            CHECK(kick(56) == 1.0);
            CHECK(raster.meldung() == QStringLiteral("2 step(s) did not fit into the pattern"));
        }
        SUBCASE("dazulegen laesst stehen, was am Ziel gesetzt ist; Ctrl+X leert die Quelle")
        {
            klick(raster, ort(0, 33));
            rahmen(0, 0, 0, 15);  // nur die Kick im ersten Takt
            taste(Qt::Key_X, Qt::ControlModifier);
            CHECK(zuletzt == QStringLiteral("Clear steps"));
            CHECK(kick(0) == 0.0);
            CHECK(kick(8) == 0.0);
            // die Kopie (Kick auf 0 und 8) zwei Schritte hinter den Taktanfang: auf 34 und 42
            raster.setzeEinfuegemarke(34);
            taste(Qt::Key_V, Qt::ControlModifier | Qt::ShiftModifier);
            CHECK(zuletzt == QStringLiteral("Paste steps, keep existing"));
            CHECK(kick(34) == 1.0);
            CHECK(kick(42) == 0.5);
            CHECK(kick(33) == 1.0);  // lag im Bereich der Kopie auf einem leeren Schritt — und bleibt
            CHECK(kick(40) == 0.5);
            CHECK(snare(36) == 1.0); // die Snare war nicht in der Kopie
        }
        SUBCASE("verstreute Schritte: nur ihre Stellen werden geschrieben (V2 a)")
        {
            raster.markiere({{0, 0}, {0, 8}});
            REQUIRE(raster.kopiere());
            klick(raster, ort(0, 20));  // zwischen den Zielen ein Schritt
            REQUIRE(raster.fuegeEinBei({0, 17}, true));
            CHECK(kick(17) == 1.0);
            CHECK(kick(25) == 0.5);
            CHECK(kick(20) == 1.0);  // unberuehrt
            CHECK(raster.markierte() == std::vector<SchrittOrt>{{0, 17}, {0, 25}});
            // Paste here in eine andere Spur
            REQUIRE(raster.fuegeEinBei({1, 0}, true));
            CHECK(snare(0) == 1.0);
            CHECK(snare(8) == 0.5);
        }
        SUBCASE("die Pfeiltasten versetzen — was unterwegs ueberfahren wird, bleibt; am Rand ist Schluss")
        {
            raster.markiere({{0, 0}});
            for (int i = 0; i < 8; ++i) taste(Qt::Key_Right);
            CHECK(zuletzt == QStringLiteral("Move steps"));
            CHECK(aenderungen == 8);
            CHECK(kick(0) == 0.0);
            CHECK(kick(8) == 1.0);  // liegt jetzt auf der leisen Kick von Schritt 8
            taste(Qt::Key_Right);
            CHECK(kick(9) == 1.0);
            CHECK(kick(8) == 0.5);  // die ueberfahrene ist wieder da
            CHECK(raster.markierte() == std::vector<SchrittOrt>{{0, 9}});
            taste(Qt::Key_Down);
            CHECK(snare(9) == 1.0);
            CHECK(kick(9) == 0.0);
            // am Rand: unter der Snare ist keine Spur mehr (V3 a)
            const int vorher = aenderungen;
            taste(Qt::Key_Down);
            CHECK(aenderungen == vorher);
            CHECK(snare(9) == 1.0);
            // links am Anfang ebenso, fuer die ganze Auswahl
            raster.markiere({{0, 16}, {1, 20}, {1, 0}});
            taste(Qt::Key_Left);
            CHECK(aenderungen == vorher);
            CHECK(kick(16) == 1.0);
            CHECK(inOrdnung(editor.loop()));
        }
        SUBCASE("Alt+Pfeil dreht im Kreis, Entf leert, Ctrl+D haengt die Auswahl noch einmal an")
        {
            raster.markiere({{1, 61}});  // die leise Snare im letzten Takt
            for (int i = 0; i < 3; ++i) taste(Qt::Key_Right, Qt::AltModifier);
            CHECK(zuletzt == QStringLiteral("Rotate steps"));
            CHECK(snare(61) == 0.0);
            CHECK(snare(0) == 0.7);  // hinten hinaus, vorn herein
            taste(Qt::Key_Left, Qt::AltModifier);
            CHECK(snare(63) == 0.7);
            CHECK(snare(0) == 0.0);

            raster.markiere({{0, 0}, {0, 8}});
            taste(Qt::Key_Delete);
            CHECK(zuletzt == QStringLiteral("Clear steps"));
            CHECK(kick(0) == 0.0);
            CHECK(kick(8) == 0.0);

            rahmen(1, 0, 1, 7);  // die Snare, erste Takthaelfte (ein Schlag auf 4)
            klick(raster, ort(1, 9));  // dahinter ein Schritt, den die Kopie ersetzt
            taste(Qt::Key_D, Qt::ControlModifier);
            CHECK(zuletzt == QStringLiteral("Duplicate steps"));
            CHECK(snare(12) == 1.0);
            CHECK(snare(9) == 0.0);
            CHECK(snare(13) == 0.0);  // die leise Snare auf 13 lag im Ziel und ist ersetzt
            CHECK(raster.markierte().size() == 8u);
            CHECK(raster.istMarkiert(1, 8));
        }
        SUBCASE("der Haken Right-click deletes: aus oeffnet der Rechtsklick das Menue, an leert er den Schritt")
        {
            // Wunsch Patrik S113; die App merkt die Stellung
            auto* haken = editor.findChild<QCheckBox*>(QStringLiteral("drumRightClickDeletes"));
            REQUIRE(haken != nullptr);
            CHECK_FALSE(haken->isChecked());  // frische Einstellungen: aus
            CHECK_FALSE(raster.rechtsklickLoescht());
            const auto rechts = [&](QPoint wo) {
                QMouseEvent ereignis(QEvent::MouseButtonPress, QPointF(wo), QPointF(raster.mapToGlobal(wo)),
                                     Qt::RightButton, Qt::RightButton, Qt::NoModifier);
                QCoreApplication::sendEvent(&raster, &ereignis);
            };
            rechts(ort(0, 0));
            CHECK(aenderungen == 0);  // aus: der Rechtsklick leert nichts
            CHECK(kick(0) == 1.0);
            // was das Menue fuer den Schritt unter der Maus tut
            REQUIRE(raster.leereSchritt({0, 0}));
            CHECK(zuletzt == QStringLiteral("Clear step"));
            CHECK(kick(0) == 0.0);
            CHECK_FALSE(raster.leereSchritt({0, 0}));  // schon aus

            haken->setChecked(true);
            CHECK(raster.rechtsklickLoescht());
            CHECK(QSettings().value(QStringLiteral("loop/drumRightClickDeletes")).toBool());
            rechts(ort(0, 8));
            CHECK(kick(8) == 0.0);
            CHECK(zuletzt == QStringLiteral("Clear step"));
            const int vorher = aenderungen;
            rechts(ort(0, 9));  // ein leerer Schritt: nichts zu leeren
            CHECK(aenderungen == vorher);
            // auf einem von mehreren markierten leert er nicht (dort oeffnet sich das Menue)
            raster.markiere({{1, 4}, {1, 13}});
            rechts(ort(1, 4));
            CHECK(aenderungen == vorher);
            CHECK(snare(4) == 1.0);
            // ein neues Fenster kennt die Stellung
            DrumEditor zweiter;
            auto* zweiterHaken = zweiter.findChild<QCheckBox*>(QStringLiteral("drumRightClickDeletes"));
            REQUIRE(zweiterHaken != nullptr);
            CHECK(zweiterHaken->isChecked());
            CHECK(zweiter.raster()->rechtsklickLoescht());
        }
        SUBCASE("das Bild: markierte Schritte tragen einen Rand, die Einfuegemarke steht im Kopf")
        {
            rahmen(0, 0, 1, 15);
            raster.setzeEinfuegemarke(24);  // im sichtbaren Teil: Takt 2, dritter Schlag
            const QImage bild = editor.grab().toImage();
            CHECK_FALSE(bild.isNull());
            if (const QString datei = qEnvironmentVariable("LUMIVIZ_DRUM_MARKEN_BILD"); !datei.isEmpty())
                CHECK(bild.save(datei));
        }
    }

    TEST_CASE("Drum-Editor: zeigt eine Basis, jede Aenderung ist eine neue Fassung des Loops")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        DrumEditor editor;  // nicht gezeigt: kein Fenster
        editor.setKitKlaenge({QStringLiteral("kit:clap"), QStringLiteral("kit:kick"), QStringLiteral("kit:snare")});
        int aenderungen = 0;
        QString zuletzt;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            ++aenderungen;
            zuletzt = was;
            editor.setLoop(neu);  // wie das Panel: die Fassung kommt zurueck
        });

        CHECK_FALSE(editor.zeige(probe(), QStringLiteral("gibt es nicht")));
        REQUIRE(editor.zeige(probe(), QStringLiteral("Grund")));
        CHECK(editor.windowTitle() == QStringLiteral("Drum pattern: Grund"));
        auto* basis = editor.findChild<QLabel*>(QStringLiteral("drumBase"));
        auto* nutzer = editor.findChild<QLabel*>(QStringLiteral("drumUsers"));
        auto* takte = editor.findChild<QSpinBox*>(QStringLiteral("drumBars"));
        auto* schlaege = editor.findChild<QSpinBox*>(QStringLiteral("drumBeats"));
        auto* laut = editor.findChild<QLabel*>(QStringLiteral("drumStepVolume"));
        auto* spitze = editor.findChild<QLabel*>(QStringLiteral("drumPeak"));
        auto* play = editor.findChild<QPushButton*>(QStringLiteral("drumPlay"));
        REQUIRE(basis != nullptr);
        REQUIRE(nutzer != nullptr);
        REQUIRE(takte != nullptr);
        REQUIRE(schlaege != nullptr);
        REQUIRE(laut != nullptr);
        REQUIRE(spitze != nullptr);
        REQUIRE(play != nullptr);
        CHECK(basis->text() == QStringLiteral("Grund"));
        CHECK(nutzer->text().contains(QStringLiteral("used by 3 clips")));
        CHECK(takte->value() == 4);
        CHECK(schlaege->value() == 4);
        CHECK(laut->text() == QStringLiteral("Step volume: Kick"));
        CHECK(editor.raster()->muster() == probe().muster[0]);
        CHECK_FALSE(editor.grab().isNull());
        CHECK(aenderungen == 0);  // zeigen aendert nichts

        // die Takte: das Feld meldet, die Clips wachsen mit
        takte->setValue(6);
        CHECK(zuletzt == QStringLiteral("Change pattern length"));
        CHECK(editor.loop().findeMuster(QStringLiteral("Grund"))->takte == 6);
        CHECK(editor.loop().kanaele[0].clips[1].startTakt == 6);
        CHECK(editor.raster()->muster().zeilen[0].size() == 96);
        // das Raster des ganzen Loops
        schlaege->setValue(3);
        CHECK(zuletzt == QStringLiteral("Change grid"));
        CHECK(editor.loop().schlaegeJeTakt == 3);
        editor.setzeSwing(0.25);
        CHECK(editor.loop().swing == doctest::Approx(0.25));
        const int vorher = aenderungen;
        editor.setzeSwing(0.25);  // derselbe Wert: kein Schritt fuer Undo
        CHECK(aenderungen == vorher);

        // Spuren
        editor.spurHinzu();
        CHECK(zuletzt == QStringLiteral("Add track"));
        const Muster* grund = editor.loop().findeMuster(QStringLiteral("Grund"));
        REQUIRE(grund->spuren.size() == 3);
        CHECK(grund->spuren[2].klang == QStringLiteral("kit:clap"));
        CHECK(laut->text() == QStringLiteral("Step volume: Track 1"));  // die neue Spur ist angeklickt
        editor.spurUmbenennen(2, QStringLiteral(" Clap "));
        editor.setzeKlang(2, QStringLiteral("C:/samples/hand.wav"));
        editor.setzeLaut(2, 0.6);
        editor.setzePan(2, -0.3);
        editor.setzeStumm(2, true);
        grund = editor.loop().findeMuster(QStringLiteral("Grund"));
        CHECK(grund->spuren[2].name == QStringLiteral("Clap"));
        CHECK(grund->spuren[2].klang == QStringLiteral("C:/samples/hand.wav"));
        CHECK(grund->spuren[2].laut == doctest::Approx(0.6));
        CHECK(grund->spuren[2].pan == doctest::Approx(-0.3));
        CHECK(grund->spuren[2].stumm);
        CHECK(zuletzt == QStringLiteral("Mute track"));
        CHECK_FALSE(editor.grab().isNull());
        // `LUMIVIZ_DRUM_EDITOR_BILD=<datei.png>` legt ein Bild des Editors ab
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_DRUM_EDITOR_BILD"); !bild.isEmpty())
        {
            editor.raster()->setMarke(9.5);
            editor.setSpitze(0.84);
            CHECK(editor.grab().save(bild));
            editor.raster()->setMarke(-1.0);
            editor.setSpitze(-1.0);
        }

        // ein Schritt im Raster wird eine Fassung des Loops
        SchrittRaster* raster = editor.raster();
        klick(*raster, QPoint(raster->xVonSchritt(2) + 4, raster->yVonSpur(2) + SchrittRaster::kZeilenHoehe / 2));
        CHECK(zuletzt == QStringLiteral("Edit steps"));
        CHECK(editor.loop().findeMuster(QStringLiteral("Grund"))->zeilen[2][2] == 1.0);

        editor.spurLoeschen(2);
        CHECK(zuletzt == QStringLiteral("Remove track"));
        CHECK(editor.loop().findeMuster(QStringLiteral("Grund"))->spuren.size() == 2);

        // ein Muster ohne Spuren (Befund Patrik S111: der leere Editor sah zerrissen aus):
        // ein Hinweis steht da, und ein hohes Fenster laesst die Luft unten, nicht zwischen den Zeilen
        {
            auto* hinweis = editor.findChild<QLabel*>(QStringLiteral("drumEmptyHint"));
            REQUIRE(hinweis != nullptr);
            CHECK(hinweis->isHidden());
            Loop leerLoop;
            Muster leer;
            leer.name = QStringLiteral("Leer");
            leerLoop.muster = {leer};
            leerLoop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("Leer"), 0, 1}}}};
            leerLoop.richte();
            DrumEditor leerer;
            REQUIRE(leerer.zeige(leerLoop, QStringLiteral("Leer")));
            auto* leerHinweis = leerer.findChild<QLabel*>(QStringLiteral("drumEmptyHint"));
            REQUIRE(leerHinweis != nullptr);
            CHECK_FALSE(leerHinweis->isHidden());
            leerer.resize(1230, 900);
            leerer.setAttribute(Qt::WA_DontShowOnScreen);
            leerer.show();
            QCoreApplication::processEvents();
            auto* takteFeld = leerer.findChild<QSpinBox*>(QStringLiteral("drumBars"));
            auto* neu = leerer.findChild<QPushButton*>(QStringLiteral("drumAddTrack"));
            if (takteFeld != nullptr && neu != nullptr)
            {
                // Kopf, Raster-Zeile und Spuren stehen dicht untereinander, oben im Fenster
                const int kopfY = takteFeld->mapTo(&leerer, QPoint(0, 0)).y();
                const int neuY = neu->mapTo(&leerer, QPoint(0, 0)).y();
                CHECK(kopfY < 40);
                CHECK(neuY < 200);
                CHECK(leerHinweis->mapTo(&leerer, QPoint(0, 0)).y() < 320);
            }
            else
                MESSAGE("drumBars oder drumAddTrack nicht gefunden — Lage nicht geprueft");
            if (const QString bild = qEnvironmentVariable("LUMIVIZ_DRUM_EDITOR_BILD"); !bild.isEmpty())
                CHECK(leerer.grab().save(bild + QStringLiteral(".leer.png")));
            leerer.spurHinzu();
        }

        // Play pattern und die Spitze
        int verlangt = 0;
        bool spielen = false;
        QObject::connect(&editor, &DrumEditor::spielenVerlangt, &editor, [&](bool an) {
            ++verlangt;
            spielen = an;
        });
        play->click();
        CHECK(verlangt == 1);
        CHECK(spielen);
        editor.setSpielt(true, 5.5);
        CHECK(play->text() == QStringLiteral("Stop"));
        play->click();
        CHECK_FALSE(spielen);
        editor.setSpielt(false, 0.0);
        CHECK(play->text() == QStringLiteral("Play pattern"));
        editor.setSpitze(0.84);
        CHECK(spitze->text() == QStringLiteral("0.84"));
        CHECK(spitze->styleSheet().isEmpty());
        editor.setSpitze(1.21);
        CHECK(spitze->text() == QStringLiteral("1.21"));
        CHECK_FALSE(spitze->styleSheet().isEmpty());  // ueber 1: rot
        editor.setSpitze(-1.0);
        CHECK(spitze->text() == QStringLiteral("–"));

        // die Basis heisst jetzt anders: der Editor folgt ihr
        Loop umbenannt = editor.loop();
        REQUIRE(benenneBasisUm(umbenannt, QStringLiteral("Grund"), QStringLiteral("Beat")));
        editor.setLoop(umbenannt);
        CHECK(editor.muster() == QStringLiteral("Beat"));
        CHECK(basis->text() == QStringLiteral("Beat"));
        // die Basis gibt es nicht mehr: der Editor hat keine mehr
        Loop ohne = editor.loop();
        ohne.kanaele.clear();
        ohne.muster.clear();
        editor.setLoop(ohne);
        CHECK(editor.muster().isEmpty());
    }

    TEST_CASE("Loop Panel: Edit base oeffnet den Editor, seine Aenderungen laufen ueber Undo")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);

        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        panel.neu();
        panel.setzeLoop(probe());
        Taktachse* achse = panel.taktachse();
        CHECK(panel.editor() == nullptr);
        CHECK_FALSE(panel.oeffneEditor(QStringLiteral("gibt es nicht"), false));

        // die Achse verlangt den Editor der Basis des angeklickten Clips
        QString verlangt;
        QObject::connect(achse, &Taktachse::editorVerlangt, achse, [&](const QString& muster) { verlangt = muster; });
        achse->blockSignals(false);
        CHECK_FALSE(achse->bearbeiteBasis());  // nichts angeklickt

        REQUIRE(panel.oeffneEditor(QStringLiteral("Grund"), false));
        DrumEditor* editor = panel.editor();
        REQUIRE(editor != nullptr);
        CHECK(editor->muster() == QStringLiteral("Grund"));
        CHECK(editor->loop() == panel.loop());

        // eine Aenderung im Editor: das Panel fuehrt sie aus, die Achse zeigt sie, Undo nimmt sie zurueck
        const Loop vorher = panel.loop();
        editor->setzeTakte(6);
        CHECK(panel.loop().findeMuster(QStringLiteral("Grund"))->takte == 6);
        CHECK(achse->loop() == panel.loop());
        CHECK(editor->loop() == panel.loop());
        CHECK(bus->undoDescription() == "Change pattern length");
        REQUIRE(bus->undo());
        CHECK(panel.loop() == vorher);
        CHECK(editor->loop() == vorher);
        CHECK(editor->raster()->muster().zeilen[0].size() == 64);
        REQUIRE(bus->redo());
        CHECK(editor->raster()->muster().zeilen[0].size() == 96);
        REQUIRE(bus->undo());

        // umbenennen im Arrangement: der Editor folgt
        achse->waehle(0, 0);
        REQUIRE(achse->benenneBasisUm(QStringLiteral("Beat")));
        CHECK(editor->muster() == QStringLiteral("Beat"));

        // der Clip bekommt eine eigene Basis — die anderen zwei bleiben bei „Beat"
        achse->waehle(0, 1);
        const int basen = static_cast<int>(panel.loop().muster.size());
        REQUIRE(panel.taktachse()->auswahl() != nullptr);
        Loop geloest = panel.loop();
        const QString neu = neueBasisAus(geloest, 0, 1);
        CHECK(neu == QStringLiteral("Beat 2"));
        panel.bearbeite(geloest, QStringLiteral("Make new base"));
        CHECK(static_cast<int>(panel.loop().muster.size()) == basen + 1);
        CHECK(basisNutzer(panel.loop(), QStringLiteral("Beat")) == 2);
        REQUIRE(panel.oeffneEditor(neu, false));
        CHECK(editor->muster() == QStringLiteral("Beat 2"));  // dasselbe Fenster, die andere Basis
        editor->spurHinzu();
        CHECK(panel.loop().findeMuster(QStringLiteral("Beat 2"))->spuren.size() == 3);
        CHECK(panel.loop().findeMuster(QStringLiteral("Beat"))->spuren.size() == 2);  // das Original bleibt

        // ein anderer Loop: die Basis des Editors gibt es nicht mehr
        panel.neu();
        CHECK(editor->muster().isEmpty());
        (void)verlangt;
    }

    // Stufe 11c, Schritt 2 (Entscheid K1 a, Skizze gebilligt S115): die Auswahl „Sound" einer
    // Spur nennt Kit, My samples und darunter Instrument..., Sample file..., Record sample...;
    // an einer Instrument-Spur der Knopf „…" mit Note, Length, Play, Browse..., Edit instrument...
    TEST_CASE("Drum-Editor: der Klang einer Spur — Kit, eigene Samples, Instrument mit Note und Length, Datei, Aufnahme")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenSampleOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("samples")));
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
        vergissSampleDaten();
        struct Zurueck
        {
            ~Zurueck()
            {
                setzeEigenenSampleOrdnerFuerTests(QString());
                setzeEigenenInstrumentOrdnerFuerTests(QString());
                vergissSampleDaten();
            }
        } zurueck;
        const auto datei = [&tmp](const QString& rel, const QByteArray& inhalt) {
            const QString pfad = QDir(tmp.path()).filePath(rel);
            REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(inhalt);
            return pfad;
        };
        (void)datei(QStringLiteral("samples/knock.wav"), QByteArray("eins"));

        DrumEditor editor;  // nicht gezeigt: kein Fenster
        editor.setKitKlaenge({QStringLiteral("kit:clap"), QStringLiteral("kit:kick"), QStringLiteral("kit:snare")});
        int aenderungen = 0;
        QString zuletzt;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            ++aenderungen;
            zuletzt = was;
            editor.setLoop(neu);
        });
        int neuGeladen = 0;
        QObject::connect(&editor, &DrumEditor::klaengeGeaendert, &editor, [&] { ++neuGeladen; });
        REQUIRE(editor.zeige(probe(), QStringLiteral("Grund")));
        const auto spur = [&editor](int nr) -> const Spur& {
            return editor.loop().findeMuster(editor.muster())->spuren[static_cast<std::size_t>(nr)];
        };
        auto* klang0 = editor.findChild<QComboBox*>(QStringLiteral("drumTrackSound0"));
        auto* klang1 = editor.findChild<QComboBox*>(QStringLiteral("drumTrackSound1"));
        auto* mehr0 = editor.findChild<QToolButton*>(QStringLiteral("drumTrackMore0"));
        auto* mehr1 = editor.findChild<QToolButton*>(QStringLiteral("drumTrackMore1"));
        REQUIRE(klang0 != nullptr);
        REQUIRE(klang1 != nullptr);
        REQUIRE(mehr0 != nullptr);
        REQUIRE(mehr1 != nullptr);

        // --- die Auswahl: Kit, My samples, darunter die drei Wege
        const QStringList wege{QStringLiteral("Instrument..."), QStringLiteral("Sample file..."),
                               QStringLiteral("Record sample...")};
        CHECK(editor.klangZeilen(0) == QStringList{QStringLiteral("[Kit]"), QStringLiteral("kit:clap"), QStringLiteral("kit:kick"),
                                                    QStringLiteral("kit:snare"), QStringLiteral("[My samples]"),
                                                    QStringLiteral("user:knock")} + wege);
        CHECK(klang0->currentText() == QStringLiteral("kick"));
        CHECK(mehr0->isHidden());  // „…" gibt es nur an einer Instrument-Spur
        CHECK(mehr1->isHidden());
        CHECK(editor.klangZeilen(7).isEmpty());
        // eine Ueberschrift laesst sich nicht waehlen
        CHECK_FALSE(klang0->model()->index(0, 0).flags().testFlag(Qt::ItemIsEnabled));
        CHECK(klang0->model()->index(1, 0).flags().testFlag(Qt::ItemIsEnabled));

        // --- ein eigenes Sample als Klang
        editor.setzeKlang(0, QStringLiteral("user:knock"));
        CHECK(zuletzt == QStringLiteral("Change sound"));
        CHECK(spur(0).klang == QStringLiteral("user:knock"));
        CHECK(spur(0).spielt() == QStringLiteral("user:knock"));
        CHECK(klang0->currentText() == QStringLiteral("knock"));
        const int vorher = aenderungen;
        editor.setzeKlang(0, QStringLiteral("user:knock"));  // dasselbe noch einmal: kein Schritt fuer Undo
        CHECK(aenderungen == vorher);
        // ein eigenes, das es nicht mehr gibt, bleibt stehen und sagt es
        editor.setzeKlang(0, QStringLiteral("user:weg"));
        CHECK(klang0->currentText() == QStringLiteral("weg (not found)"));
        CHECK(editor.klangZeilen(0).contains(QStringLiteral("user:weg")));
        // eine Datei an freiem Ort steht unter „File"
        editor.setzeKlang(0, QStringLiteral("C:/samples/hand.wav"));
        CHECK(klang0->currentText() == QStringLiteral("hand.wav"));
        CHECK(editor.klangZeilen(0).contains(QStringLiteral("[File]")));
        CHECK_FALSE(editor.klangZeilen(1).contains(QStringLiteral("[File]")));

        // --- Sample file...: kopiert zu den eigenen (S2 a) oder am Ort gelassen
        const QString fremd = datei(QStringLiteral("fremd/Hand Clap.wav"), QByteArray("zwei"));
        REQUIRE(editor.nimmSampleDatei(0, fremd, true));
        CHECK(spur(0).klang == QStringLiteral("user:hand_clap"));
        CHECK(QFileInfo::exists(QDir(eigenerSampleOrdner()).filePath(QStringLiteral("hand_clap.wav"))));
        CHECK(editor.klangZeilen(0).contains(QStringLiteral("user:hand_clap")));  // steht gleich unter My samples
        REQUIRE(editor.nimmSampleDatei(0, fremd, false));
        CHECK(spur(0).klang == fremd);
        CHECK_FALSE(editor.nimmSampleDatei(0, QDir(tmp.path()).filePath(QStringLiteral("gibt/es/nicht.wav")), true));
        CHECK(spur(0).klang == fremd);

        // --- ein Instrument: die Note nach seiner Art, Length 250 ms; der Klang bleibt daneben stehen
        editor.setzeSpurInstrument(1, QStringLiteral("slap_bass"));
        CHECK(zuletzt == QStringLiteral("Change sound"));
        CHECK(spur(1).instrument == QStringLiteral("slap_bass"));
        CHECK(spur(1).klang == QStringLiteral("kit:snare"));
        CHECK(spur(1).hoehe == 36);  // ein Bass: C2
        CHECK(spur(1).dauerMs == kSpurDauerMs);
        CHECK(spur(1).spielt() == tonKlang(QStringLiteral("slap_bass"), 36, 250));
        CHECK(klang1->currentText() == QStringLiteral("Slap Bass"));
        CHECK(editor.klangZeilen(1).first() == QStringLiteral("instrument:slap_bass"));
        CHECK_FALSE(mehr1->isHidden());
        CHECK(mehr0->isHidden());
        CHECK(inOrdnung(editor.loop()));

        // --- „…": Note, Length, Play
        CHECK(editor.detail() == nullptr);
        CHECK_FALSE(editor.oeffneDetail(0, false));  // die Spur spielt kein Instrument
        REQUIRE(editor.oeffneDetail(1, false));
        REQUIRE(editor.detail() != nullptr);
        CHECK(editor.detailSpur() == 1);
        auto* note = editor.detail()->findChild<QComboBox*>(QStringLiteral("drumTrackNote"));
        auto* dauer = editor.detail()->findChild<QSpinBox*>(QStringLiteral("drumTrackLength"));
        auto* play = editor.detail()->findChild<QPushButton*>(QStringLiteral("drumTrackPlay"));
        auto* titel = editor.detail()->findChild<QLabel*>(QStringLiteral("drumTrackDetailTitle"));
        REQUIRE(note != nullptr);
        REQUIRE(dauer != nullptr);
        REQUIRE(play != nullptr);
        REQUIRE(titel != nullptr);
        CHECK(editor.detail()->findChild<QPushButton*>(QStringLiteral("drumTrackBrowse")) != nullptr);
        CHECK(editor.detail()->findChild<QPushButton*>(QStringLiteral("drumTrackEditInstrument")) != nullptr);
        CHECK(titel->text() == QStringLiteral("Snare plays Slap Bass"));
        CHECK(note->currentText() == QStringLiteral("C2"));
        CHECK(note->count() == kHoeheMax - kHoeheMin + 1);
        CHECK(dauer->value() == 250);
        dauer->setValue(120);
        CHECK(zuletzt == QStringLiteral("Change length"));
        CHECK(spur(1).dauerMs == 120);
        editor.setzeNote(1, 40);
        CHECK(zuletzt == QStringLiteral("Change note"));
        CHECK(spur(1).hoehe == 40);
        CHECK(note->currentData().toInt() == 40);  // das Feld folgt
        editor.setzeDauer(1, 0);
        CHECK(spur(1).dauerMs == kSpurDauerMinMs);
        editor.setzeDauer(1, 99999);
        CHECK(spur(1).dauerMs == kSpurDauerMaxMs);
        editor.setzeNote(1, 200);
        CHECK(spur(1).hoehe == kHoeheMax);
        editor.setzeNote(1, 40);
        editor.setzeDauer(1, 120);
        {
            const int davor = aenderungen;
            editor.setzeNote(0, 50);  // keine Instrument-Spur: nichts
            editor.setzeDauer(0, 50);
            CHECK(aenderungen == davor);
        }
        QStringList gespielt;
        QObject::connect(&editor, &DrumEditor::spurTonVerlangt, &editor,
                         [&](const QString& basis, int nr) { gespielt << QStringLiteral("%1 %2").arg(basis).arg(nr); });
        play->click();
        CHECK(gespielt == QStringList{QStringLiteral("Grund 1")});
        CHECK_FALSE(editor.spieleSpur(0));
        CHECK(gespielt.size() == 1);
        // `LUMIVIZ_DRUM_KLANG_BILD=<datei.png>` legt ein Bild des Editors und eines des Felds ab
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_DRUM_KLANG_BILD"); !bild.isEmpty())
        {
            CHECK(editor.grab().save(bild));
            editor.detail()->adjustSize();
            CHECK(editor.detail()->grab().save(bild + QStringLiteral(".feld.png")));
        }

        // --- Browse... / Instrument...: das Fenster Instruments waehlt fuer die Spur
        CHECK(editor.palette() == nullptr);
        CHECK_FALSE(editor.oeffnePalette(9, false));
        REQUIRE(editor.oeffnePalette(1, false));
        InstrumentPalette* palette = editor.palette();
        REQUIRE(palette != nullptr);
        auto* ziel = palette->findChild<QLabel*>(QStringLiteral("instrumentPaletteTarget"));
        REQUIRE(ziel != nullptr);
        CHECK(ziel->text() == QStringLiteral("For track: Snare (Grund)"));
        CHECK(palette->gewaehlt() == QStringLiteral("slap_bass"));  // das Instrument der Spur ist gewaehlt
        REQUIRE(palette->waehle(QStringLiteral("piano")));
        REQUIRE(palette->verwende());
        CHECK(spur(1).instrument == QStringLiteral("piano"));
        CHECK(spur(1).hoehe == 40);    // ein anderes Instrument: Note und Length bleiben
        CHECK(spur(1).dauerMs == 120);
        CHECK(titel->text() == QStringLiteral("Snare plays Piano"));
        // ein Klick im Fenster spielt das Instrument an — der Editor reicht es weiter
        QStringList toene;
        QObject::connect(&editor, &DrumEditor::instrumentTonVerlangt, &editor,
                         [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(nennung).arg(hoehe); });
        REQUIRE(palette->spieleAn());
        CHECK(toene == QStringList{QStringLiteral("piano 60")});
        // fuer eine Spur ohne Instrument: Use macht sie zur Instrument-Spur
        REQUIRE(editor.oeffnePalette(0, false));
        CHECK(ziel->text() == QStringLiteral("For track: Kick (Grund)"));
        REQUIRE(palette->waehle(QStringLiteral("strings")));
        REQUIRE(palette->verwende());
        CHECK(spur(0).instrument == QStringLiteral("strings"));
        CHECK(spur(0).hoehe == 60);
        CHECK(spur(0).klang == fremd);
        CHECK_FALSE(mehr0->isHidden());
        // die Spur heisst anders: die Zeile im Fenster folgt
        editor.spurUmbenennen(0, QStringLiteral("Pad"));
        CHECK(ziel->text() == QStringLiteral("For track: Pad (Grund)"));

        // --- Edit instrument...: der Creator; was er speichert, nennt die Spur danach
        CHECK(editor.creator() == nullptr);
        REQUIRE(editor.oeffneCreator(1, false));
        InstrumentCreator* creator = editor.creator();
        REQUIRE(creator != nullptr);
        CHECK(creator->instrument().titel == QStringLiteral("Piano"));
        REQUIRE(creator->speichereAlsEigenes(QStringLiteral("Mein Schlag")));
        CHECK(spur(1).instrument == QStringLiteral("user:mein_schlag"));
        CHECK(zuletzt == QStringLiteral("Change sound"));
        CHECK(neuGeladen == 1);
        CHECK(klang1->currentText() == QStringLiteral("Mein Schlag (My files)"));
        CHECK(spur(1).hoehe == 40);
        // eine Spur, die inzwischen wieder einen Klang spielt, bleibt dabei
        editor.setzeKlang(1, QStringLiteral("kit:snare"));
        REQUIRE(creator->speichere());
        CHECK(spur(1).instrument.isEmpty());
        CHECK(neuGeladen == 2);
        editor.setzeSpurInstrument(1, QStringLiteral("user:mein_schlag"));

        // --- ein Klang aus der Auswahl: die Spur spielt kein Instrument mehr, das Feld schliesst
        REQUIRE(editor.oeffneDetail(1, false));
        editor.setzeKlang(1, QStringLiteral("kit:kick"));
        CHECK(spur(1).instrument.isEmpty());
        CHECK(spur(1).klang == QStringLiteral("kit:kick"));
        CHECK(spur(1).hoehe == Spur().hoehe);
        CHECK(spur(1).dauerMs == kSpurDauerMs);
        CHECK(spur(1).spielt() == QStringLiteral("kit:kick"));
        CHECK(mehr1->isHidden());
        CHECK(editor.detailSpur() == -1);
        CHECK_FALSE(editor.oeffneCreator(1, false));  // kein Instrument: nichts zu bearbeiten

        // --- Record sample...: Save and use setzt die Aufnahme als Klang der Spur
        CHECK(editor.aufnahmeFenster() == nullptr);
        CHECK_FALSE(editor.oeffneAufnahme(9, false));
        papp::AufnahmeEngine aufnahmeEngine;
        aufnahmeEngine.eingaenge = {papp::eingang(0, QStringLiteral("Default"), true)};
        editor.setEngineGeber([&aufnahmeEngine]() -> IAudioEngine* { return &aufnahmeEngine; });
        REQUIRE(editor.oeffneAufnahme(0, false));
        AufnahmeFenster* aufnahme = editor.aufnahmeFenster();
        REQUIRE(aufnahme != nullptr);
        CHECK(aufnahme->findChild<QPushButton*>(QStringLiteral("recordStart"))->isEnabled());
        aufnahme->setzeAufnahme(std::vector<float>(2 * 24000, 0.25f), 48000);
        const int geladenVorher = neuGeladen;
        CHECK(aufnahme->speichereAls(QStringLiteral("Tisch")) == QStringLiteral("user:tisch"));
        CHECK(spur(0).klang == QStringLiteral("user:tisch"));
        CHECK(spur(0).instrument.isEmpty());  // die Spur spielte „strings": jetzt die Aufnahme
        CHECK(mehr0->isHidden());
        CHECK(klang0->currentText() == QStringLiteral("tisch"));
        CHECK(neuGeladen == geladenVorher + 1);
        // dasselbe Sample ersetzt (seit Stufe 17 nur ueber Save an einem geoeffneten):
        // am Loop aendert sich nichts, aber wer spielt, laedt neu
        REQUIRE(editor.oeffneAufnahme(0, false));
        aufnahme->setzeAufnahme(std::vector<float>(2 * 12000, 0.5f), 48000);
        CHECK(aufnahme->speichereAls(QStringLiteral("tisch")).isEmpty());  // vergeben: Save as... nimmt den Namen nicht
        aufnahme->bearbeite(QStringLiteral("user:tisch"), std::vector<float>(2 * 12000, 0.5f), 48000);
        const int schritte = aenderungen;
        CHECK(aufnahme->speichere() == QStringLiteral("user:tisch"));
        CHECK(aenderungen == schritte);
        CHECK(neuGeladen == geladenVorher + 2);
        editor.setEngineGeber({});
        aufnahme->setEngine(nullptr);  // die Engine aus Pappe geht vor dem Fenster

        // --- Edit sound... (Schritt 3, K1 a): aus dem Klang wird ein Instrument mit einer Sample-Schicht;
        //     am Loop aendert sich nichts, bis der Creator speichert
        {
            // wer die Audio-Engine hat, dekodiert — hier gibt der Test die Daten heraus: 100 ms
            QObject::connect(&editor, &DrumEditor::sampleVerlangt, &editor, [](const QString& nennung) {
                merkeSampleDaten(nennung, 48000, std::vector<float>(2 * 4800, 0.1f));
            });
            const Instrument aus = DrumEditor::instrumentAusKlang(QStringLiteral("user:knock"));
            CHECK(aus.name.isEmpty());  // noch keine Datei
            CHECK(aus.titel == QStringLiteral("Knock"));
            CHECK(aus.art == QStringLiteral("drums"));  // K3 a
            REQUIRE(aus.schichten.size() == 1);
            CHECK(aus.schichten[0].schwingung == Schwingung::Sample);
            CHECK(aus.schichten[0].sample == QStringLiteral("user:knock"));
            CHECK(aus.schichten[0].grundton == 60);
            CHECK(aus.schichten[0].anstiegMs == 0.0);
            CHECK(aus.schichten[0].halten == 1.0);
            CHECK(DrumEditor::instrumentAusKlang(QStringLiteral("kit:hat_closed")).titel == QStringLiteral("Hat closed"));
            CHECK(DrumEditor::instrumentAusKlang(QStringLiteral("C:/x/Big Tom.wav")).titel == QStringLiteral("Big Tom"));

            editor.setzeKlang(1, QStringLiteral("user:knock"));
            const int davor = aenderungen;
            REQUIRE(editor.bearbeiteKlang(1, false));
            CHECK(aenderungen == davor);  // nichts aendert sich vor Save
            CHECK(spur(1).instrument.isEmpty());
            CHECK(creator->instrument().titel == QStringLiteral("Knock"));
            CHECK(creator->datei().isEmpty());
            CHECK(creator->vorschlagTitel() == QStringLiteral("Knock"));  // ein neues: sein Titel, keine Zahl dahinter
            creator->aendereSchicht([](Schicht& s) { s.grundton = 48; });
            REQUIRE(creator->speichereAlsEigenes(QStringLiteral("Mein Knock")));
            CHECK(zuletzt == QStringLiteral("Edit sound"));
            CHECK(spur(1).instrument == QStringLiteral("user:mein_knock"));
            CHECK(spur(1).klang == QStringLiteral("user:knock"));  // bleibt daneben stehen
            CHECK(spur(1).hoehe == 48);     // K2 a: der Root note der Schicht
            CHECK(spur(1).dauerMs == 100);  // und die Laenge der Aufnahme
            CHECK(spur(1).spielt() == tonKlang(QStringLiteral("user:mein_knock"), 48, 100));
            CHECK(klang1->currentText() == QStringLiteral("Mein Knock (My files)"));
            CHECK_FALSE(mehr1->isHidden());
            // noch einmal gespeichert: Note und Length bleiben, wie sie jetzt stehen
            editor.setzeNote(1, 50);
            REQUIRE(creator->speichere());
            CHECK(spur(1).hoehe == 50);
            // an einer Instrument-Spur ist Edit sound... dasselbe wie Edit instrument...
            REQUIRE(editor.bearbeiteKlang(1, false));
            CHECK(creator->instrument().name == QStringLiteral("user:mein_knock"));
            // die Spur spielt beim Save etwas anderes als beim Edit sound...: sie bleibt dabei
            editor.setzeKlang(1, QStringLiteral("kit:kick"));
            REQUIRE(editor.bearbeiteKlang(1, false));
            CHECK(creator->instrument().titel == QStringLiteral("Kick"));
            editor.setzeKlang(1, QStringLiteral("kit:snare"));
            REQUIRE(creator->speichereAlsEigenes(QStringLiteral("Kick eigen")));
            CHECK(spur(1).instrument.isEmpty());
            CHECK(spur(1).klang == QStringLiteral("kit:snare"));
            // eine Spur ohne Klang: nichts zu bearbeiten
            editor.setzeKlang(1, QString());
            CHECK_FALSE(editor.bearbeiteKlang(1, false));
            CHECK_FALSE(editor.bearbeiteKlang(9, false));
            editor.setzeKlang(1, QStringLiteral("kit:kick"));
        }

        // --- eine Spur weniger: die Nummern verrutschen — das Fenster Instruments waehlt fuer keine mehr
        REQUIRE(editor.oeffnePalette(1, false));
        CHECK(ziel->text() == QStringLiteral("For track: Snare (Grund)"));
        editor.spurLoeschen(0);
        CHECK(palette->ziel().isEmpty());
        REQUIRE(palette->waehle(QStringLiteral("piano")));
        CHECK_FALSE(palette->verwende());
        CHECK(spur(0).instrument.isEmpty());
        CHECK(inOrdnung(editor.loop()));
    }

    TEST_CASE("Loop Panel: der Klang einer Drum-Spur laeuft ueber Undo")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);

        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        panel.neu();
        panel.setzeLoop(probe());
        REQUIRE(panel.oeffneEditor(QStringLiteral("Grund"), false));
        DrumEditor* editor = panel.editor();
        REQUIRE(editor != nullptr);
        const Loop vorher = panel.loop();

        editor->setzeSpurInstrument(1, QStringLiteral("slap_bass"));
        CHECK(panel.loop().findeMuster(QStringLiteral("Grund"))->spuren[1].instrument == QStringLiteral("slap_bass"));
        CHECK(bus->undoDescription() == "Change sound");
        editor->setzeNote(1, 43);
        CHECK(bus->undoDescription() == "Change note");
        REQUIRE(bus->undo());
        CHECK(panel.loop().findeMuster(QStringLiteral("Grund"))->spuren[1].hoehe == 36);
        REQUIRE(bus->undo());
        CHECK(panel.loop() == vorher);
        CHECK(editor->loop() == vorher);
        CHECK(editor->klangZeilen(1).first() == QStringLiteral("[Kit]"));  // die Auswahl zeigt wieder den Klang
        REQUIRE(bus->redo());
        CHECK(editor->klangZeilen(1).first() == QStringLiteral("instrument:slap_bass"));

        // Ctrl+Z im Fenster Instruments, geoeffnet aus dem Drum-Editor: derselbe Undo
        REQUIRE(editor->oeffnePalette(1, false));
        editor->palette()->nimmZurueck();
        CHECK(panel.loop() == vorher);
        editor->palette()->stelleWiederHer();
        CHECK(panel.loop().findeMuster(QStringLiteral("Grund"))->spuren[1].instrument == QStringLiteral("slap_bass"));

        // ohne Audio-Engine wird nichts angespielt
        CHECK_FALSE(panel.spieleSpur(QStringLiteral("Grund"), 1));
    }
}
