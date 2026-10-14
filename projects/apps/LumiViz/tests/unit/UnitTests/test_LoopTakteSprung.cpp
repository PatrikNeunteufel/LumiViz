/**
 * @file test_LoopTakteSprung.cpp
 * @brief Loop, S124: Muster ab 1 Takt (B1 a), Repeat when lengthening (B2 a),
 *        die Kopie traegt den Zeitraum des Rahmens (B3 a), ein Klick in den
 *        Zeitstrahl springt (B4 b) — Entscheide Patrik S124
 *
 * Ohne Ton und ohne gezeigtes Fenster; eigene Einstellungen (`LumiVizTest` /
 * `LoopTakteSprung`).
 */

#include <doctest.h>

#include "loop/Auswahl.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Spieler.hpp"
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QSettings>

#include <vector>

using namespace lumi::loop;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopTakteSprung"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButtons tasten, Qt::KeyboardModifiers dazu)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), art == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton,
                         tasten, dazu);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPoint ort, Qt::KeyboardModifiers dazu = Qt::NoModifier)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, dazu);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::NoButton, dazu);
}

/// Ein Drum-Muster „D" mit einer Spur und `takte` Takten (16 Schritte je Takt); Schritt `s` mit Wert `w`.
Loop drumLoop(int takte, std::vector<std::pair<int, double>> schritte)
{
    Loop loop;
    Muster d;
    d.name = QStringLiteral("D");
    d.takte = takte;
    d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    d.zeilen = {std::vector<double>(static_cast<std::size_t>(takte * 16), 0.0)};
    for (const auto& [s, w] : schritte) d.zeilen[0][static_cast<std::size_t>(s)] = w;
    loop.muster = {d};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("D"), 0, 1}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("LoopTakteSprung")
{
    TEST_CASE("Loop Takte: ein Muster ab 1 Takt, neue mit 4; Repeat when lengthening (B1 a, B2 a)")
    {
        CHECK(kTakteMin == 1);
        CHECK(kTakteNeu == 4);
        CHECK(Muster().takte == kTakteNeu);

        // --- Drum: 1 Takt mit zwei Schritten, auf 4 verlaengert — viermal derselbe Takt
        Loop loop = drumLoop(1, {{0, 1.0}, {6, 0.5}});
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("D"), 4, true));
        const Muster* d = loop.findeMuster(QStringLiteral("D"));
        REQUIRE(d != nullptr);
        CHECK(d->takte == 4);
        for (int takt = 0; takt < 4; ++takt)
        {
            CAPTURE(takt);
            CHECK(d->zeilen[0][static_cast<std::size_t>(takt * 16)] == 1.0);
            CHECK(d->zeilen[0][static_cast<std::size_t>(takt * 16 + 6)] == 0.5);
            CHECK(d->zeilen[0][static_cast<std::size_t>(takt * 16 + 1)] == 0.0);
        }
        // --- 3 Takte auf 4: die Takte 1, 2, 3, 1
        loop = drumLoop(3, {{0, 1.0}, {16, 0.7}, {32, 0.4}});
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("D"), 4, true));
        CHECK(loop.findeMuster(QStringLiteral("D"))->zeilen[0][48] == 1.0);
        // --- ohne Wiederholen: Stille wie bisher
        loop = drumLoop(1, {{0, 1.0}});
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("D"), 2, false));
        CHECK(loop.findeMuster(QStringLiteral("D"))->zeilen[0][16] == 0.0);
        // --- kuerzer: schneidet ab, das Wiederholen tut nichts
        loop = drumLoop(4, {{0, 1.0}, {20, 1.0}});
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("D"), 1, true));
        CHECK(loop.findeMuster(QStringLiteral("D"))->takte == 1);
        CHECK(loop.findeMuster(QStringLiteral("D"))->zeilen[0].size() == 16u);

        // --- Noten: 1 Takt auf 3, eine Note reicht ueber das Ende des Takts hinaus? Nein — sie wird
        // bei der letzten Wiederholung am Ende des Musters gekuerzt, wenn sie hinausreichte
        Loop noten;
        Muster mel;
        mel.name = QStringLiteral("M");
        mel.art = MusterArt::Instrument;
        mel.instrument = QStringLiteral("piano");
        mel.takte = 1;
        mel.noten = {{0, 4, 60, 1.0}, {12, 4, 64, 0.5}};
        noten.muster = {mel};
        noten.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("M"), 0, 1}}}};
        noten.richte();
        REQUIRE(setzeMusterTakte(noten, QStringLiteral("M"), 3, true));
        const Muster* m = noten.findeMuster(QStringLiteral("M"));
        REQUIRE(m != nullptr);
        CHECK(m->noten.size() == 6u);
        CHECK(findeNote(*m, {16, 60}) >= 0);
        CHECK(findeNote(*m, {28, 64}) >= 0);
        CHECK(findeNote(*m, {44, 64}) >= 0);
        CHECK(m->noten[static_cast<std::size_t>(findeNote(*m, {44, 64}))].laenge == 4);  // endet genau mit dem Muster

        // --- ein neuer Clip ist 4 Takte lang
        Loop leer;
        (void)kanalHinzu(leer);
        REQUIRE(neuerClip(leer, 0, 0, MusterArt::Drumkit) == 0);
        CHECK(leer.muster[0].takte == kTakteNeu);
    }

    TEST_CASE("Loop Takte: der Haken Repeat when lengthening in beiden Editoren (B2 a)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;  // leer: der Haken ist an (die Vorgabe)
        CHECK(wiederholenBeimVerlaengern());

        DrumEditor editor;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString&) { editor.setLoop(neu); });
        REQUIRE(editor.zeige(drumLoop(1, {{0, 1.0}, {8, 0.6}}), QStringLiteral("D")));
        auto* haken = editor.findChild<QCheckBox*>(QStringLiteral("drumRepeatBars"));
        REQUIRE(haken != nullptr);
        CHECK(haken->isChecked());
        editor.setzeTakte(2);
        CHECK(editor.loop().findeMuster(QStringLiteral("D"))->zeilen[0][24] == 0.6);
        // aus: die neuen Takte bleiben leer — und der Instrument-Editor sieht dieselbe Stellung
        haken->setChecked(false);
        CHECK_FALSE(wiederholenBeimVerlaengern());
        editor.setzeTakte(3);
        CHECK(editor.loop().findeMuster(QStringLiteral("D"))->zeilen[0][32] == 0.0);
        InstrumentEditor instrument;
        auto* andererHaken = instrument.findChild<QCheckBox*>(QStringLiteral("instrumentRepeatBars"));
        REQUIRE(andererHaken != nullptr);
        CHECK_FALSE(andererHaken->isChecked());
        // das Feld Bars geht bis 1 hinunter
        editor.setzeTakte(1);
        CHECK(editor.loop().findeMuster(QStringLiteral("D"))->takte == 1);
    }

    TEST_CASE("Loop Kopie: der Zeitraum des Rahmens geht mit — auch leere Raender (B3 a)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        // --- das Modell: mit Zeitraum beginnt die Kopie am Rahmen
        Muster mel;
        mel.art = MusterArt::Instrument;
        mel.takte = 4;
        mel.noten = {{4, 2, 60, 1.0}, {10, 1, 62, 1.0}};
        NotenKopie ohne = kopiereNoten(mel, {{4, 60}, {10, 62}});
        CHECK(ohne.schritte == 7);  // von der ersten Note bis zum Ende der letzten
        CHECK(ohne.noten.front().schritt == 0);
        NotenKopie mit = kopiereNoten(mel, {{4, 60}, {10, 62}}, std::pair<int, int>{0, 16});
        CHECK(mit.schritte == 16);
        CHECK(mit.noten.front().schritt == 4);

        // --- das Noten-Raster: Shift+Ziehen ueber Takt 1, darin eine Note bei Schritt 4
        NotenRaster raster;
        raster.resize(NotenRaster::kTastenBreite + 70 * NotenRaster::kSchrittBreite, 420);
        raster.setMuster(mel, 4, 4);
        raster.zeigeHoehe(61);
        Muster zuletzt = mel;
        QObject::connect(&raster, &NotenRaster::geaendert, &raster, [&](const Muster& neu, const QString&) {
            zuletzt = neu;
            raster.setMuster(neu, 4, 4);
        });
        const int y = raster.yVonHoehe(60) + 3;
        maus(raster, QEvent::MouseButtonPress, QPoint(raster.xVonSchritt(0) + 1, y), Qt::LeftButton, Qt::ShiftModifier);
        maus(raster, QEvent::MouseMove, QPoint(raster.xVonSchritt(16) - 1, y + 4), Qt::LeftButton, Qt::ShiftModifier);
        maus(raster, QEvent::MouseButtonRelease, QPoint(raster.xVonSchritt(16) - 1, y + 4), Qt::NoButton, Qt::ShiftModifier);
        REQUIRE(raster.markierte() == std::vector<NotenOrt>{{4, 60}});
        REQUIRE(raster.zeitraum().has_value());
        CHECK(*raster.zeitraum() == std::pair<int, int>{0, 16});
        REQUIRE(raster.kopiere());
        raster.setzeEinfuegemarke(16);
        REQUIRE(raster.fuegeEin());
        CHECK(findeNote(zuletzt, {20, 60}) >= 0);  // 4 hinter dem Anfang des Rahmens, nicht am Rahmen
        CHECK(raster.einfuegemarke() == 32);       // um die Breite des Rahmens weiter
        // Ctrl+A: das ganze Muster ist der Zeitraum
        raster.markiereAlle();
        CHECK(*raster.zeitraum() == std::pair<int, int>{0, 64});
        // eine Note einzeln markiert: kein Zeitraum mehr — die Kopie beginnt an der Note
        raster.markiere({{4, 60}});
        CHECK_FALSE(raster.zeitraum().has_value());

        // --- das Drum-Raster: ein Rahmen mit leeren Raendern (Befund Patrik S124) — der Versatz stimmt
        Loop loop = drumLoop(4, {{4, 1.0}, {9, 0.5}});
        SchrittRaster drum;
        drum.setMuster(*loop.findeMuster(QStringLiteral("D")), 4, 4);
        std::vector<SchrittOrt> rahmen;
        for (int s = 0; s < 16; ++s) rahmen.push_back({0, s});  // Takt 1, Schritt 0 und 15 leer
        drum.markiere(rahmen);
        REQUIRE(drum.kopiere());
        drum.setzeEinfuegemarke(16);
        REQUIRE(drum.fuegeEin());
        CHECK(drum.muster().zeilen[0][20] == 1.0);
        CHECK(drum.muster().zeilen[0][25] == 0.5);
        CHECK(drum.muster().zeilen[0][16] == 0.0);
        CHECK(drum.einfuegemarke() == 32);
    }

    TEST_CASE("Loop Sprung: Klick ins Lineal und in die Taktzeile; zwei Marken in den Editoren (B4 b)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        // --- Drum-Raster: Klick = Abspielstelle, Shift+Klick = Einfuegemarke
        Loop loop = drumLoop(4, {{0, 1.0}});
        SchrittRaster drum;
        drum.resize(drum.sizeHint());
        drum.setMuster(*loop.findeMuster(QStringLiteral("D")), 4, 4);
        std::vector<int> gewaehlt;
        QObject::connect(&drum, &SchrittRaster::abspielstelleGewaehlt, &drum, [&](int s) { gewaehlt.push_back(s); });
        CHECK(drum.abspielstelle() == 0);
        klick(drum, QPoint(drum.xVonSchritt(8) + 3, 5));
        CHECK(drum.abspielstelle() == 8);
        CHECK(drum.einfuegemarke() == 0);
        CHECK(gewaehlt == std::vector<int>{8});
        klick(drum, QPoint(drum.xVonSchritt(8) + 3, 5));  // derselbe Schritt noch einmal: wieder springen
        CHECK(gewaehlt == std::vector<int>{8, 8});
        klick(drum, QPoint(drum.xVonSchritt(20) + 3, 5), Qt::ShiftModifier);
        CHECK(drum.einfuegemarke() == 20);
        CHECK(drum.abspielstelle() == 8);
        // ein kuerzeres Muster: die Abspielstelle bleibt im Muster
        Muster kurz = *loop.findeMuster(QStringLiteral("D"));
        kurz.takte = 1;
        kurz.zeilen[0].resize(16);
        drum.setzeAbspielstelle(40);
        drum.setMuster(kurz, 4, 4);
        CHECK(drum.abspielstelle() == 15);

        // --- Noten-Raster: dasselbe
        Muster mel;
        mel.art = MusterArt::Instrument;
        mel.takte = 4;
        NotenRaster noten;
        noten.resize(NotenRaster::kTastenBreite + 70 * NotenRaster::kSchrittBreite, 420);
        noten.setMuster(mel, 4, 4);
        klick(noten, QPoint(noten.xVonSchritt(12) + 2, 5));
        CHECK(noten.abspielstelle() == 12);
        CHECK(noten.einfuegemarke() == 0);
        klick(noten, QPoint(noten.xVonSchritt(24) + 1, 5), Qt::ShiftModifier);
        CHECK(noten.einfuegemarke() == 24);
        CHECK(noten.abspielstelle() == 12);

        // --- die Taktachse: ein Klick ins Lineal waehlt eine Stelle in Schritten
        Taktachse achse;
        achse.resize(900, 260);
        Loop lang = drumLoop(4, {{0, 1.0}});
        lang.kanaele[0].clips = {{QStringLiteral("D"), 0, 2}};  // zwei Durchgaenge: 8 Takte
        lang.richte();
        achse.setLoop(lang, true);
        std::vector<int> stellen;
        QObject::connect(&achse, &Taktachse::stelleGewaehlt, &achse, [&](int s) { stellen.push_back(s); });
        klick(achse, QPoint(achse.xVonTakt(2) + 1, 5));
        REQUIRE(stellen.size() == 1u);
        CHECK(stellen[0] == 32);  // Takt 3 beginnt bei Schritt 32
        maus(achse, QEvent::MouseButtonPress, QPoint(achse.xVonTakt(1) + 1, 5), Qt::LeftButton, Qt::NoModifier);
        maus(achse, QEvent::MouseMove, QPoint(achse.xVonTakt(5) + 1, 5), Qt::LeftButton, Qt::NoModifier);
        maus(achse, QEvent::MouseButtonRelease, QPoint(achse.xVonTakt(5) + 1, 5), Qt::NoButton, Qt::NoModifier);
        CHECK(stellen.back() == 80);  // Ziehen: die Stelle folgt
        CHECK(achse.schrittBeiX(achse.xVonTakt(100)) == lang.durchlaufSchritte());  // nicht ueber das Ende

        // --- das Panel: die Startstelle steht als Marke im Lineal
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        LoopPanel panel(dienste);
        panel.neu();
        panel.setzeLoop(lang);
        CHECK(panel.startstelle() == 0);
        Q_EMIT panel.taktachse()->stelleGewaehlt(48);
        CHECK(panel.startstelle() == 48);
        CHECK(panel.taktachse()->startmarke() == 48.0);
        CHECK_FALSE(panel.spiele());  // ohne Audio-Engine spielt nichts — die Stelle bleibt
        CHECK(panel.startstelle() == 48);

        // --- der Spieler springt nur, wenn er spielt
        Spieler spieler;
        CHECK_FALSE(spieler.springe(8.0));
    }
}
