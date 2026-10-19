/**
 ****************************************************************************************
 * @file   test_LoopAkkordFenster.cpp
 * @brief  Tabs und Akkorde, Stufe 26e-2: das Fenster *Chords* — Baum, Galerie, Leiste,
 *         Anhoeren in vier Spielarten, Favoriten, *Recently used*, *unusual*, und der Weg
 *         ueber den Instrument-Editor: *Chords...*, **Use**, Ziehen ins Raster
 *         (docs/audio/Saiten_Konzept.md S-T8 a bis S-T12 b)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; eigene Einstellungen (`LumiVizTest` /
 * `LoopAkkordFenster`). Die Spielart merkt die Test-Exe nicht (Zeile unten). Die
 * mitgelieferten Akkorde vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 * `LUMIVIZ_CHORD_WINDOW_BILD=<datei.png>` legt ein Bild des Fensters ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/AkkordFenster.hpp"
#include "loop/GriffbildBild.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDropEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QToolButton>

#include <algorithm>
#include <utility>
#include <vector>

using namespace lumi::loop;

namespace
{

// die Test-Exe merkt die Spielart nicht (S-T10 a) — sonst hinge der Ausgang am Programm
const bool kSpielartNichtMerken = [] {
    AkkordFenster::setzeSpielartMerken(false);
    return true;
}();

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Anwendung()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopAkkordFenster"));
        QSettings().clear();
    }
    ~Anwendung() { QSettings().clear(); }
};

Saitensatz standard()
{
    Saitensatz s;
    s.leer = {40, 45, 50, 55, 59, 64};
    return s;
}

Akkord holeOderLeer(const QString& nennung)
{
    Akkord a;
    REQUIRE(holeAkkord(nennung, a));
    return a;
}

std::vector<int> hoehen(const Muster& m)
{
    std::vector<int> h;
    for (const Note& n : m.noten) h.push_back(n.hoehe);
    return h;
}

}  // namespace

TEST_CASE("Loop Akkord Fenster 26e-2: anhoeren in vier Spielarten, fremde Toene, Lage")
{
    Akkord c;
    c.titel = QStringLiteral("C");
    c.grundton = 0;
    c.art = QStringLiteral("major");
    c.leer = standard().leer;
    c.buende = {-1, 3, 2, 0, 1, 0};
    Strum strum;
    strum.spreadMs = 12.0;

    const Muster ab = akkordZumAnhoeren(c, standard(), strum, AkkordSpielart::Ab, 4);
    CHECK(hoehen(ab) == std::vector<int>{48, 52, 55, 60, 64});
    CHECK(ab.noten.front() == Note{0, 8, 48, 1.0, 1});  // auf ihrer Saite, zwei Schlaege lang
    CHECK(ab.anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Ab}});
    CHECK(ab.strum.spreadMs == 12.0);
    CHECK(akkordZumAnhoeren(c, standard(), strum, AkkordSpielart::Auf, 4).anschlaege ==
          std::vector<Anschlag>{Anschlag{0, Anschlagart::Auf}});
    CHECK(akkordZumAnhoeren(c, standard(), strum, AkkordSpielart::Zugleich, 4).anschlaege ==
          std::vector<Anschlag>{Anschlag{0, Anschlagart::Zugleich}});
    // Down-up: Down, eine halbe Zaehlzeit spaeter Up
    const Muster abAuf = akkordZumAnhoeren(c, standard(), strum, AkkordSpielart::AbAuf, 4);
    REQUIRE(abAuf.noten.size() == 10);
    CHECK(abAuf.noten[0].schritt == 0);
    CHECK(abAuf.noten[0].laenge == 2);
    CHECK(abAuf.noten[5].schritt == 2);
    CHECK(abAuf.anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Ab}, Anschlag{2, Anschlagart::Auf}});
    CHECK(akkordZumAnhoeren(c, standard(), strum, AkkordSpielart::AbAuf, 3).anschlaege.back().schritt == 1);
    // mit Capo zwei: alles zwei Halbtoene hoeher, dieselben Saiten
    Saitensatz capo = standard();
    capo.capo = 2;
    CHECK(hoehen(akkordZumAnhoeren(c, capo, strum, AkkordSpielart::Ab, 4)) == std::vector<int>{50, 54, 57, 62, 66});
    // eine andere Stimmung: nichts
    Saitensatz dropD = standard();
    dropD.leer[0] = 38;
    CHECK(akkordZumAnhoeren(c, dropD, strum, AkkordSpielart::Ab, 4).noten.empty());

    // die Lage, wie die Galerie sie nennt
    CHECK(lageText(c) == QStringLiteral("open"));
    Akkord barre = c;
    barre.buende = {-1, 3, 5, 5, 5, 3};
    CHECK(lageText(barre) == QStringLiteral("fr 3"));

    // fremde Toene (S-T12 b): chords-db bb_6_4 spielt C#, F#, A zu Bb6
    CHECK(fremdeTonklassen(c) == std::vector<int>{});
    CHECK(fremdeTonklassen(holeOderLeer(QStringLiteral("guitar/bb_6_4"))) == std::vector<int>{1, 6, 9});
    // ein Slash: der Bass gehoert dazu (A/G#)
    CHECK(fremdeTonklassen(holeOderLeer(QStringLiteral("guitar/a_over_ab_1"))) == std::vector<int>{});
    // alt: unbekannt, kein Urteil
    for (const Akkord& a : mitgelieferteAkkorde())
        if (a.art == QLatin1String("alt"))
        {
            CHECK_FALSE(fremdeTonklassen(a).has_value());
            break;
        }
}

TEST_CASE("Loop Akkord Fenster 26e-2: Baum, Galerie, Leiste, Spielart, Favoriten (LUMIVIZ_CHORD_WINDOW_BILD)")
{
    Anwendung anwendung;
    AkkordFenster fenster;
    Strum strum;
    fenster.setzeZiel(QStringLiteral("Mel"), QStringLiteral("Steel"), standard(), strum, 4);
    fenster.liesNeu();
    std::vector<Muster> gehoert;
    QObject::connect(&fenster, &AkkordFenster::vorhoerenVerlangt, &fenster, [&](const Muster& m) { gehoert.push_back(m); });
    std::vector<Akkord> verwendet;
    QObject::connect(&fenster, &AkkordFenster::verwenden, &fenster, [&](const Akkord& a) { verwendet.push_back(a); });

    // C: der Baum — oben Favoriten und Zuletzt, dann die Grundtoene der Gitarre (nur sie: die Stimmung)
    const QStringList baum = fenster.baumZeilen();
    REQUIRE(baum.size() > 14);
    CHECK(baum[0] == QString::fromUtf8("★ Favorites (0)"));
    CHECK(baum[1] == QStringLiteral("Recently used (0)"));
    CHECK(baum[2].startsWith(QStringLiteral("C (")));
    CHECK(baum[3] == QStringLiteral("  major (4)"));
    CHECK(baum.back() == QStringLiteral("Your chords (0)"));
    CHECK(fenster.ordner() == QStringLiteral("root:0"));  // zu Beginn C

    // D: die Galerie zu „C major" — die vier Lagen der Gitarre, keine von Drop D
    REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0:major")));
    CHECK(fenster.galerieZeilen() == QStringList{QStringLiteral("[C major · 4]"), QStringLiteral("guitar/c_major_1"),
                                                 QStringLiteral("guitar/c_major_2"), QStringLiteral("guitar/c_major_3"),
                                                 QStringLiteral("guitar/c_major_4")});
    // C allein: alle Arten unter Koepfen, Dur und Moll vorn
    REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0")));
    const QStringList alle = fenster.galerieZeilen();
    CHECK(alle.front() == QStringLiteral("[C major · 4]"));
    CHECK(alle.indexOf(QStringLiteral("[C minor · 4]")) == 5);
    CHECK(std::any_of(alle.begin(), alle.end(), [](const QString& z) { return z.startsWith(QStringLiteral("[C/E · ")); }));

    // E: waehlen zeigt Titel und Toene; F: verschieben
    REQUIRE(fenster.waehle(QStringLiteral("guitar/c_major_1")));
    CHECK(fenster.detailText() == QStringLiteral("Notes: C3 E3 G3 C4 E4"));
    CHECK(fenster.feld()->akkord().buende == std::vector<int>{-1, 3, 2, 0, 1, 0});
    REQUIRE(fenster.verschiebe(2));
    CHECK(fenster.detailText() == QStringLiteral("Notes: D3 F#3 A3 D4 F#4"));
    CHECK(fenster.feld()->akkord().buende == std::vector<int>{-1, 5, 4, 2, 3, 2});
    CHECK_FALSE(fenster.verschiebe(-3));  // unter den Sattel nicht
    CHECK(fenster.verschiebung() == 2);

    // G: anhoeren — zu Beginn Down; die Spielart bleibt fuer den naechsten Klick
    CHECK(fenster.spielart() == AkkordSpielart::Ab);
    REQUIRE(fenster.spieleAn());
    REQUIRE(gehoert.size() == 1);
    CHECK(gehoert.back().anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Ab}});
    CHECK(hoehen(gehoert.back()) == std::vector<int>{50, 54, 57, 62, 66});  // die verschobene Lage
    auto* abAuf = fenster.findChild<QToolButton*>(QStringLiteral("chordPlayDownUp"));
    REQUIRE(abAuf != nullptr);
    abAuf->click();
    CHECK(fenster.spielart() == AkkordSpielart::AbAuf);
    REQUIRE(gehoert.size() == 2);
    CHECK(gehoert.back().anschlaege.size() == 2);
    // ein Klick auf ein anderes Bild der Galerie: waehlt (Verschiebung 0) und spielt Down-up
    fenster.resize(680, 620);
    fenster.galerie()->resize(470, fenster.galerie()->minimumHeight());
    const QRect zelle = fenster.galerie()->zelle(QStringLiteral("guitar/c_major_2"));
    REQUIRE(!zelle.isEmpty());
    QMouseEvent druck(QEvent::MouseButtonPress, zelle.center(), fenster.galerie()->mapToGlobal(zelle.center()), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(fenster.galerie(), &druck);
    CHECK(fenster.gewaehlt() == QStringLiteral("guitar/c_major_2"));
    CHECK(fenster.verschiebung() == 0);
    REQUIRE(gehoert.size() == 3);
    CHECK(gehoert.back().anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Ab}, Anschlag{2, Anschlagart::Auf}});
    // nicht gemerkt in der Test-Exe
    CHECK_FALSE(QSettings().contains(QStringLiteral("chordWindow/playAs")));

    // ★ und Recently used
    fenster.setzeFavorit(true);
    CHECK(fenster.favoriten() == QStringList{QStringLiteral("guitar/c_major_2")});
    CHECK(fenster.baumZeilen().front() == QString::fromUtf8("★ Favorites (1)"));
    REQUIRE(fenster.verwende());
    REQUIRE(verwendet.size() == 1);
    CHECK(verwendet.back().buende == holeOderLeer(QStringLiteral("guitar/c_major_2")).buende);
    CHECK(fenster.zuletzt() == QStringList{QStringLiteral("guitar/c_major_2")});
    REQUIRE(fenster.waehleOrdner(QStringLiteral("recent")));
    CHECK(fenster.galerieZeilen() == QStringList{QStringLiteral("[Recently used]"), QStringLiteral("guitar/c_major_2")});
    // gemerkt in den Einstellungen; ein neues Fenster liest sie
    AkkordFenster zweites;
    zweites.setzeZiel(QStringLiteral("Mel"), QStringLiteral("Steel"), standard(), strum, 4);
    CHECK(zweites.favoriten() == QStringList{QStringLiteral("guitar/c_major_2")});
    CHECK(zweites.zuletzt() == QStringList{QStringLiteral("guitar/c_major_2")});

    // B: suchen ueber alle Grundtoene, die Art filtert
    fenster.setzeSuche(QStringLiteral("Cm7"));
    CHECK(fenster.galerieZeilen().contains(QStringLiteral("guitar/c_m7_1")));
    CHECK_FALSE(fenster.galerieZeilen().contains(QStringLiteral("guitar/c_major_1")));
    fenster.setzeSuche(QString());
    REQUIRE(fenster.waehleOrdner(QStringLiteral("root:7")));
    fenster.setzeArt(QStringLiteral("sus4"));
    CHECK(fenster.galerieZeilen().front().startsWith(QStringLiteral("[Gsus4 · ")));
    CHECK(fenster.galerieZeilen().size() > 1);
    fenster.setzeArt(QString());

    // S-T12 b: unusual
    REQUIRE(fenster.waehle(QStringLiteral("guitar/bb_6_4")));
    CHECK(fenster.ungewoehnlichText() == QStringLiteral("unusual: C#, F#, A not in Bb6"));
    REQUIRE(fenster.waehle(QStringLiteral("guitar/c_major_1")));
    CHECK(fenster.ungewoehnlichText().isEmpty());

    // S-T4 a: computed bei einer anderen Stimmung — Drop D
    Saitensatz dropD = standard();
    dropD.leer[0] = 38;
    fenster.setzeZiel(QStringLiteral("Mel"), QStringLiteral("Drop D"), dropD, strum, 4);
    CHECK(fenster.gewaehlt().isEmpty());  // c_major_1 der Gitarre gibt es hier nicht
    REQUIRE(fenster.waehle(QStringLiteral("drop_d/d_major_1")));
    auto* gerechnet = fenster.findChild<QLabel*>(QStringLiteral("chordComputed"));
    REQUIRE(gerechnet != nullptr);
    CHECK(gerechnet->isVisibleTo(&fenster));

    // ohne Muster: Use ist aus
    fenster.setzeZiel(QString(), QString(), dropD, strum, 4);
    CHECK_FALSE(fenster.verwende());

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CHORD_WINDOW_BILD"); !ziel.isEmpty())
    {
        fenster.setzeZiel(QStringLiteral("Mel"), QStringLiteral("Steel"), standard(), strum, 4);
        REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0")));
        REQUIRE(fenster.waehle(QStringLiteral("guitar/c_major_1")));
        fenster.setAttribute(Qt::WA_DontShowOnScreen);
        fenster.show();
        QCoreApplication::processEvents();
        CHECK(fenster.grab().save(ziel));
        MESSAGE("Fenster Chords: " << ziel.toStdString());
        fenster.hide();
    }
}

TEST_CASE("Loop Akkord Fenster 26e-2: ueber den Editor — Chords..., Use an die Einfuegemarke, Ziehen ins Raster")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument steel;
    steel.titel = QStringLiteral("Steel");
    steel.saiten.leer = standard().leer;
    steel.schichten[0].schwingung = Schwingung::Saite;
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(steel, datei));

    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = datei;
    mel.noten = {Note{4, 2, 40, 1.0, 0}, Note{4, 2, 55, 1.0, 3}};  // E2 auf der tiefen E-Saite und G3 auf der G-Saite an Schritt 4
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();

    InstrumentEditor editor;
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);
    });
    QStringList angespielt;
    QObject::connect(&editor, &InstrumentEditor::akkordVorhoerenVerlangt, &editor,
                     [&](const QString& nennung, const Muster&) { angespielt << nennung; });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    const auto* knopf = editor.findChild<QPushButton*>(QStringLiteral("instrumentChords"));
    REQUIRE(knopf != nullptr);
    CHECK_FALSE(knopf->isHidden());  // ein Saiteninstrument

    editor.oeffneAkkorde(false);
    AkkordFenster* fenster = editor.akkordFenster();
    REQUIRE(fenster != nullptr);
    CHECK(fenster->findChild<QLabel*>(QStringLiteral("chordTarget"))->text() ==
          QStringLiteral("For: Steel · Mel — E2 A2 D3 G3 B3 E4, capo 0"));
    REQUIRE(fenster->waehle(QStringLiteral("guitar/c_major_1")));
    REQUIRE(fenster->spieleAn());
    CHECK(angespielt == QStringList{datei});  // mit dem Instrument des Musters

    // Use an die Einfuegemarke (Schritt 4): was dort auf seinen Saiten liegt, weicht — E2 auf der freien Saite bleibt
    NotenRaster* raster = editor.raster();
    raster->setzeEinfuegemarke(4);
    REQUIRE(fenster->verwende());
    REQUIRE(!gemeldet.isEmpty());
    CHECK(gemeldet.back() == QStringLiteral("Chord C"));
    const Muster* jetzt = loop.findeMuster(QStringLiteral("Mel"));
    REQUIRE(jetzt != nullptr);
    std::vector<std::pair<int, int>> anVier;  // (Hoehe, Saite)
    for (const Note& n : jetzt->noten)
        if (n.schritt == 4) anVier.emplace_back(n.hoehe, n.saite);
    std::sort(anVier.begin(), anVier.end());
    CHECK(anVier == std::vector<std::pair<int, int>>{{40, 0}, {48, 1}, {52, 2}, {55, 3}, {60, 4}, {64, 5}});
    CHECK(raster->markierte().size() == 5);

    // ziehen: ein Griffbild landet am Schritt unter der Maus, mit seiner Verschiebung
    raster->resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 600);
    QMimeData daten;
    daten.setData(QLatin1String(AkkordFenster::kMimeTyp), QByteArrayLiteral("guitar/c_major_1\n2"));
    const QPointF ort(raster->xVonSchritt(10) + 4, raster->tabLinieY(2));
    QDragEnterEvent hinein(ort.toPoint(), Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(raster, &hinein);
    CHECK(hinein.isAccepted());
    QDropEvent fallen(ort, Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(raster, &fallen);
    CHECK(fallen.isAccepted());
    jetzt = loop.findeMuster(QStringLiteral("Mel"));
    std::vector<int> anZehn;
    for (const Note& n : jetzt->noten)
        if (n.schritt == 10) anZehn.push_back(n.hoehe);
    std::sort(anZehn.begin(), anZehn.end());
    CHECK(anZehn == std::vector<int>{50, 54, 57, 62, 66});  // D-Dur: zwei Buende hoeher
    // etwas anderes laesst das Raster fallen
    QMimeData fremd;
    fremd.setText(QStringLiteral("C"));
    QDragEnterEvent anders(ort.toPoint(), Qt::CopyAction, &fremd, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(raster, &anders);
    CHECK_FALSE(anders.isAccepted());

    // ohne Saiten: kein Knopf
    Instrument flach;
    flach.titel = QStringLiteral("Pad");
    const QString flachDatei = QDir(ordner.path()).filePath(QStringLiteral("pad.lvinst"));
    REQUIRE(speichereInstrument(flach, flachDatei));
    editor.setzeInstrument(flachDatei);
    CHECK(knopf->isHidden());
}
