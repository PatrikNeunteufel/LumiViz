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

#include "loop/Bearbeiten.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>

#include <memory>

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
}
