/**
 ****************************************************************************************
 * @file   test_LoopEditorTab.cpp
 * @brief  Tabs und Akkorde, Stufe 26c: die Tab-Linien im Instrument-Editor
 *         (docs/audio/Saiten_Konzept.md §1; Skizze gebilligt, Patrik S137, S-T5 a A, B, F, G)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; die Instrumente liegen als Dateien in einem
 * Wegwerf-Ordner. Bedient wird ueber den Editor (Merker S135): das Raster meldet, der
 * Editor baut die neue Fassung des Loops, sie kommt mit `setLoop` zurueck.
 * `LUMIVIZ_EDITOR_TAB_BILD=<datei.png>` legt ein Bild des Editors ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QTemporaryDir>

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

Instrument gitarre(bool bundlos = false)
{
    Instrument i;
    i.titel = QStringLiteral("Steel");
    i.saiten.leer = {40, 45, 50, 55, 59, 64};
    i.saiten.mensurMm = 645.0;
    i.saiten.bundlos = bundlos;
    i.schichten[0].schwingung = Schwingung::Saite;
    return i;
}

/// Ein Loop in 4/4 mit „Mel": ein Akkord E2 A2 D3 am Anfang, ein G3 auf Schritt 4 (leere G-Saite).
Loop probe(const QString& instrument)
{
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = instrument;
    mel.noten = {Note{0, 4, 40, 1.0, -1}, Note{0, 4, 45, 1.0, -1}, Note{0, 4, 50, 1.0, -1}, Note{4, 2, 55, 1.0, -1}};
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    return loop;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButtons tasten)
{
    QMouseEvent e(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), Qt::LeftButton, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &e);
}

void klick(QWidget& ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::NoButton);
}

void taste(QWidget& ziel, Qt::Key key, const QString& text = {})
{
    QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier, text);
    QCoreApplication::sendEvent(&ziel, &e);
}

/// Der Platz der Note an diesem Schritt mit dieser Tonhoehe; -1 = keine.
int platzVon(const Muster& m, int schritt, int hoehe)
{
    for (std::size_t i = 0; i < m.noten.size(); ++i)
        if (m.noten[i].schritt == schritt && m.noten[i].hoehe == hoehe) return static_cast<int>(i);
    return -1;
}

}  // namespace

TEST_CASE("Loop Editor Tab 26c: die Tab-Linien — Saiten, Bundzahlen, der Haken Tab, ohne Buende der Tonname")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(gitarre(), datei));

    InstrumentEditor editor;
    Loop loop = probe(datei);
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString&) {
        loop = neu;
        editor.setLoop(loop);
    });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    NotenRaster* raster = editor.raster();
    auto* haken = editor.findChild<QCheckBox*>(QStringLiteral("editorTab"));
    REQUIRE(haken != nullptr);
    CHECK_FALSE(haken->isHidden());
    CHECK(haken->isChecked());  // in der Test-Exe merkt der Editor nichts: von sich aus an

    // A: je Saite eine Linie, oben die hoechste; unter der Spur Strum, ueber den Noten
    CHECK(raster->tabAn());
    CHECK(raster->tabOben() == raster->strumOben() + raster->strumHoehe());
    CHECK(raster->tabHoehe() == 6 * NotenRaster::kTabZeile + 2 * NotenRaster::kTabRand);
    CHECK(raster->rasterOben() == raster->tabOben() + raster->tabHoehe());
    CHECK(raster->tabLinieY(5) < raster->tabLinieY(0));  // die hohe e-Saite oben
    for (int stelle = 0; stelle < 6; ++stelle) CHECK(raster->tabSaiteBei(raster->tabLinieY(stelle)) == stelle);
    CHECK(raster->tabSaiteBei(raster->tabOben() + 1) == -1);  // der Rand
    CHECK(raster->tabSaiteBei(raster->rasterOben() + 1) == -1);
    CHECK(raster->strumBei(QPoint(raster->xVonSchritt(0) + 4, raster->tabLinieY(5))) == -1);

    // B: an jeder Note der Bund — die leeren Saiten 0
    const Muster& mel = raster->muster();
    for (int i = 0; i < 4; ++i) CHECK(raster->tabText(i) == QStringLiteral("0"));
    const int g3 = platzVon(mel, 4, 55);
    REQUIRE(g3 >= 0);
    CHECK(raster->tabNoteBei(QPoint(raster->xVonSchritt(4) + 3, raster->tabLinieY(3))) == g3);
    CHECK(raster->tabNoteBei(QPoint(raster->xVonSchritt(4) + 3, raster->tabLinieY(2))) == -1);  // eine andere Saite
    CHECK(raster->tabNoteBei(QPoint(raster->xVonSchritt(6) + 3, raster->tabLinieY(3))) == -1);

    // F: der Haken nimmt die Linien weg, die Noten ruecken hinauf
    const int mitTab = raster->rasterOben();
    haken->setChecked(false);
    CHECK(raster->tabHoehe() == 0);
    CHECK(raster->rasterOben() == mitTab - 6 * NotenRaster::kTabZeile - 2 * NotenRaster::kTabRand);
    CHECK(raster->tabSaiteBei(raster->strumOben() + raster->strumHoehe() + 6) == -1);
    haken->setChecked(true);
    CHECK(raster->rasterOben() == mitTab);

    // G: die Hinweiszeile nennt die Bedienung der Linien
    CHECK(editor.findChild<QLabel*>(QStringLiteral("instrumentStringHint"))->text().contains(QStringLiteral("Tab lines")));

    // ohne Buende (S-L9 a): statt der Bundzahl der Tonname
    const QString bundlosDatei = QDir(ordner.path()).filePath(QStringLiteral("fretless.lvinst"));
    REQUIRE(speichereInstrument(gitarre(true), bundlosDatei));
    InstrumentEditor bundlos;
    REQUIRE(bundlos.zeige(probe(bundlosDatei), QStringLiteral("Mel")));
    CHECK(bundlos.raster()->tabText(platzVon(bundlos.raster()->muster(), 4, 55)) == tonName(55));

    // ohne Saiten: keine Linien und kein Haken
    InstrumentEditor ohne;
    REQUIRE(ohne.zeige(probe(QStringLiteral("piano")), QStringLiteral("Mel")));
    CHECK(ohne.raster()->tabHoehe() == 0);
    CHECK(ohne.findChild<QCheckBox*>(QStringLiteral("editorTab"))->isHidden());

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_TAB_BILD"); !ziel.isEmpty())
    {
        // fuers Bild: ein paar Bünde mehr, die Tab-Marke steht auf Schritt 8, D-Saite
        raster->setzeTabMarke(8, 2);
        taste(*raster, Qt::Key_5, QStringLiteral("5"));
        raster->setzeTabMarke(10, 3);
        taste(*raster, Qt::Key_7, QStringLiteral("7"));
        raster->setzeTabMarke(12, 1);
        editor.setAttribute(Qt::WA_DontShowOnScreen);
        editor.resize(1280, 760);
        editor.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        raster->zeigeHoehe(52);
        QCoreApplication::processEvents();
        CHECK(editor.grab().save(ziel));
    }
}

TEST_CASE("Loop Editor Tab 26c: ueber den Editor — Bund tippen (auch zweistellig), auf eine andere Saite ziehen")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(gitarre(), datei));

    InstrumentEditor editor;
    Loop loop = probe(datei);
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);
    });
    QList<int> angespielt;
    QObject::connect(&editor, &InstrumentEditor::tonVerlangt, &editor, [&](int hoehe) { angespielt << hoehe; });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    NotenRaster* raster = editor.raster();
    raster->resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 500);
    const auto mel = [&]() -> const Muster& { return *loop.findeMuster(QStringLiteral("Mel")); };

    // ein Klick auf die D-Saite an Schritt 8: die Tab-Marke, noch keine Note
    klick(*raster, QPoint(raster->xVonSchritt(8) + 4, raster->tabLinieY(2)));
    CHECK(raster->tabSchritt() == 8);
    CHECK(raster->tabSaite() == 2);
    CHECK(mel().noten.size() == 4);
    CHECK(gemeldet.isEmpty());

    // „5": eine Note auf der D-Saite, 5. Bund (G3), die Saite steht an der Note (S-M2 a) — im Loop
    taste(*raster, Qt::Key_5, QStringLiteral("5"));
    REQUIRE(gemeldet.size() == 1);
    CHECK(gemeldet.back() == QStringLiteral("Fret 5"));
    int neu = platzVon(mel(), 8, 55);
    REQUIRE(neu >= 0);
    CHECK(mel().noten[static_cast<std::size_t>(neu)].saite == 2);
    CHECK(mel().noten[static_cast<std::size_t>(neu)].laenge == 1);
    CHECK(raster->tabText(platzVon(raster->muster(), 8, 55)) == QStringLiteral("5"));

    // „1", „2" kurz hintereinander: erst Bund 1, dann 12 — dieselbe Note, kein zweiter Ton
    taste(*raster, Qt::Key_1, QStringLiteral("1"));
    taste(*raster, Qt::Key_2, QStringLiteral("2"));
    CHECK(gemeldet.mid(1) == QStringList{QStringLiteral("Fret 1"), QStringLiteral("Fret 12")});
    CHECK(mel().noten.size() == 5);
    neu = platzVon(mel(), 8, 62);
    REQUIRE(neu >= 0);
    CHECK(mel().noten[static_cast<std::size_t>(neu)].saite == 2);
    // ausserhalb der Buende: nichts
    raster->setzeTabMarke(8, 2);
    taste(*raster, Qt::Key_9, QStringLiteral("9"));
    taste(*raster, Qt::Key_9, QStringLiteral("9"));  // 99 gibt es nicht: die zweite 9 beginnt neu
    CHECK(platzVon(mel(), 8, 59) >= 0);

    // ziehen: die Zahl an Schritt 8 von der D- auf die G-Saite — derselbe Ton (B3), dort 4. Bund
    const QPoint zahl(raster->xVonSchritt(8) + 4, raster->tabLinieY(2));
    REQUIRE(raster->tabNoteBei(zahl) >= 0);
    gemeldet.clear();
    maus(*raster, QEvent::MouseButtonPress, zahl, Qt::LeftButton);
    maus(*raster, QEvent::MouseMove, QPoint(zahl.x(), raster->tabLinieY(3)), Qt::LeftButton);
    maus(*raster, QEvent::MouseButtonRelease, QPoint(zahl.x(), raster->tabLinieY(3)), Qt::NoButton);
    REQUIRE(gemeldet.size() == 1);
    CHECK(gemeldet.back() == QStringLiteral("String 3"));
    neu = platzVon(mel(), 8, 59);
    REQUIRE(neu >= 0);
    CHECK(mel().noten[static_cast<std::size_t>(neu)].saite == 3);
    CHECK(raster->tabText(platzVon(raster->muster(), 8, 59)) == QStringLiteral("4"));
    CHECK(raster->tabSaite() == 3);  // die Marke geht mit: eine Zahl aendert jetzt dort den Bund

    // ein Klick auf eine Zahl markiert ihre Note; eine Zahl aendert dann ihren Bund
    const int g3 = platzVon(raster->muster(), 4, 55);
    klick(*raster, QPoint(raster->xVonSchritt(4) + 4, raster->tabLinieY(3)));
    CHECK(raster->auswahl() == g3);
    taste(*raster, Qt::Key_2, QStringLiteral("2"));
    CHECK(platzVon(mel(), 4, 57) >= 0);  // G-Saite, 2. Bund: A3
    CHECK(platzVon(mel(), 4, 55) < 0);

    // links der Name der Saite spielt die leere an; Escape nimmt die Tab-Marke weg
    klick(*raster, QPoint(10, raster->tabLinieY(0)));
    CHECK(angespielt == QList<int>{40});
    taste(*raster, Qt::Key_Escape);
    CHECK(raster->tabSchritt() == -1);
    const std::size_t vorher = mel().noten.size();
    taste(*raster, Qt::Key_3, QStringLiteral("3"));  // ohne Marke: nichts
    CHECK(mel().noten.size() == vorher);
}

TEST_CASE("Loop Editor Tab Befund S137: die Hoehe eines Bends verdeckt keine Zahl")
{
    Anwendung anwendung;
    NotenRaster raster;
    raster.setSaiten(gitarre().saiten);
    // A3 auf der G-Saite (Bund 2), einen Schritt lang, mit Bend full — gleich danach H3 auf der H-Saite
    Note gebogen{0, 1, 57, 1.0, 3};
    gebogen.bend = bendVorlage(BendVorlage::Bend, 2.0);
    Muster m;
    m.name = QStringLiteral("Mel");
    m.art = MusterArt::Instrument;
    m.takte = 1;
    m.noten = {gebogen, Note{1, 1, 59, 1.0, 4}};
    raster.setMuster(m, 4, 4);
    raster.resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 500);
    REQUIRE(raster.tabText(0) == QStringLiteral("2"));
    REQUIRE(raster.tabText(1) == QStringLiteral("0"));

    // die Hoehe laege auf der Zahl der naechsten Note: sie steht nicht da (der Tooltip nennt sie)
    CHECK(raster.tabBendFeld(0).isNull());
    // dieselbe Note auf der Saite gleich danach: ebenso
    m.noten[1] = Note{1, 1, 59, 1.0, 3};
    raster.setMuster(m, 4, 4);
    CHECK(raster.tabBendFeld(0).isNull());
    // mit Platz dahinter steht sie rechts des Pfeils, im Band zur Linie darueber
    m.noten[1] = Note{8, 1, 59, 1.0, 4};
    raster.setMuster(m, 4, 4);
    const QRect feld = raster.tabBendFeld(0);
    REQUIRE_FALSE(feld.isNull());
    CHECK(feld.left() > raster.xVonSchritt(0) + 10);
    CHECK(feld.bottom() < raster.tabLinieY(3));
    CHECK(feld.top() >= raster.tabLinieY(4) - NotenRaster::kTabZeile / 2);
    // ohne Bend kein Feld
    CHECK(raster.tabBendFeld(1).isNull());
}
