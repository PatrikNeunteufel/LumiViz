/**
 * @file test_ShapeFenster.cpp
 * @brief Klang aus einer Form, Stufe 20a: das Fenster „Edit shape" — zeichnen
 *        mit Pen, Move point, die Liste Contours, Preview und Play, Save und
 *        Save as..., und sein Weg aus dem Instrument-Creator (Edit..., New
 *        shape...) (docs/audio/Shape_Konzept.md §8a; Entscheide Z1 a und Z2 a,
 *        Patrik S122)
 *
 * Ohne Ton und ohne gezeigtes Fenster: Play spielt in eine Engine aus Pappe,
 * eigene Formen und Instrumente liegen in Wegwerf-Ordnern, die Dialoge selbst
 * werden nicht gezeigt. `LUMIVIZ_SHAPE_FENSTER_BILD=<datei.png>` legt ein Bild
 * des Fensters ab (zwei Konturen, Move point).
 */

#include <doctest.h>

#include "PappAufnahme.hpp"
#include "loop/FormFenster.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "shape/Kontur.hpp"
#include "shape/Zeichnen.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QTemporaryDir>
#include <QToolButton>
#include <QTreeWidget>

#include <cmath>
#include <vector>

using namespace lumi::loop;
namespace shape = lumi::shape;

namespace
{

constexpr double kPi = 3.14159265358979323846;

void maus(QWidget& ziel, QEvent::Type art, QPointF ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, ort, ziel.mapToGlobal(ort), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPointF ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::LeftButton, Qt::NoButton);
}

void taste(QWidget& ziel, int welche)
{
    QKeyEvent ereignis(QEvent::KeyPress, welche, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

/// Eigene Formen und Instrumente in Wegwerf-Ordnern, solange er lebt.
struct EigeneOrdner
{
    QTemporaryDir tmp;
    EigeneOrdner()
    {
        REQUIRE(tmp.isValid());
        shape::setzeEigenenKonturOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("contours")));
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
    }
    ~EigeneOrdner()
    {
        shape::setzeEigenenKonturOrdnerFuerTests(QString());
        setzeEigenenInstrumentOrdnerFuerTests(QString());
    }
};

/// Ein Dreieck in den Koordinaten der Zeichnung.
QPainterPath dreieck(double x = 0.0)
{
    QPainterPath pfad;
    pfad.moveTo(x + 50.0, 50.0);
    pfad.lineTo(x + 150.0, 50.0);
    pfad.lineTo(x + 100.0, 150.0);
    pfad.closeSubpath();
    return pfad;
}

}  // namespace

TEST_CASE("Shape Fenster: zeichnen mit Pen — Klicks, frei Hand, Doppelklick, Enter, Esc")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    EigeneOrdner ordner;

    FormFenster fenster;  // nicht gezeigt: kein Fenster
    FormFlaeche* flaeche = fenster.flaeche();
    flaeche->resize(400, 400);  // die leere Ansicht ist 200 × 200: ein Bildpunkt ist eine halbe Einheit
    REQUIRE(flaeche->imBild(QPointF(50.0, 50.0)) == QPointF(100.0, 100.0));

    // --- neu: leer, Pen gewaehlt, die Wege von aussen und Bridge noch grau, nichts zu speichern
    CHECK(fenster.herkunft() == FormFenster::Herkunft::Neu);
    CHECK(fenster.windowTitle() == QStringLiteral("Edit shape – New shape"));
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("shapeLocation"))->text() == QStringLiteral("Not saved yet"));
    CHECK(flaeche->werkzeug() == FormFlaeche::Werkzeug::Stift);
    CHECK(fenster.findChild<QToolButton*>(QStringLiteral("shapePen"))->isChecked());
    for (const char* grau : {"shapeOpenImage", "shapeBridge", "shapeShowImage", "shapeFromImage", "shapeUseAs"})  // Open SVG... seit 20b
    {
        INFO(grau);
        auto* teil = fenster.findChild<QWidget*>(QLatin1String(grau));
        REQUIRE(teil != nullptr);
        CHECK_FALSE(teil->isEnabled());
    }
    CHECK(fenster.findChild<QWidget*>(QStringLiteral("shapeDraw")) == nullptr);  // Z2 a: kein Draw
    auto* sichern = fenster.findChild<QPushButton*>(QStringLiteral("shapeSave"));
    auto* sichernAls = fenster.findChild<QPushButton*>(QStringLiteral("shapeSaveAs"));
    CHECK_FALSE(sichern->isEnabled());
    CHECK_FALSE(sichernAls->isEnabled());
    CHECK(fenster.form() == nullptr);
    CHECK(fenster.verlauf()->links().empty());

    // --- drei Klicks und ein Klick auf den ersten Punkt: ein Dreieck
    klick(*flaeche, QPointF(100.0, 100.0));
    klick(*flaeche, QPointF(300.0, 100.0));
    CHECK(shape::ankerpunkte(flaeche->angefangen()).size() == 2);
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("shapeHint"))->text().contains(QStringLiteral("Enter to close")));
    klick(*flaeche, QPointF(200.0, 300.0));
    klick(*flaeche, QPointF(103.0, 102.0));  // innerhalb des Fangkreises um den ersten Punkt
    CHECK(flaeche->angefangen().isEmpty());
    REQUIRE(fenster.zeichnung().konturen.size() == 1);
    const shape::Kontur erste = fenster.zeichnung().konturen[0];
    CHECK(erste.kennung == 1);
    CHECK(shape::pfadAlsText(erste.pfad) == QStringLiteral("M 50 50 L 150 50 L 100 150 L 50 50 Z"));
    CHECK(fenster.aktuelle() == 1);
    CHECK(fenster.form() != nullptr);
    CHECK(fenster.anteil(1) == doctest::Approx(1.0));
    CHECK(fenster.verlauf()->links().size() == 401);
    QTreeWidget* liste = fenster.liste();
    REQUIRE(liste->topLevelItemCount() == 1);
    CHECK(liste->topLevelItem(0)->text(0) == QStringLiteral("1 Contour"));
    CHECK(liste->topLevelItem(0)->text(1) == QStringLiteral("100 %"));
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("shapeContoursTitle"))->text() == QStringLiteral("Contours — 1 of 1 ticked"));
    CHECK(sichernAls->isEnabled());
    CHECK_FALSE(sichern->isEnabled());  // eine neue Form hat noch keinen Namen
    CHECK(fenster.geaendert());

    // --- frei Hand: ein Kreis, der am Anfang endet, schliesst von selbst
    std::vector<QPointF> kreis;
    for (int n = 0; n <= 120; ++n)
    {
        const double w = 2.0 * kPi * n / 120.0;
        kreis.emplace_back(200.0 + 120.0 * std::cos(w), 200.0 + 120.0 * std::sin(w));
    }
    maus(*flaeche, QEvent::MouseButtonPress, kreis.front(), Qt::LeftButton, Qt::LeftButton);
    for (std::size_t n = 1; n < kreis.size(); ++n) maus(*flaeche, QEvent::MouseMove, kreis[n], Qt::NoButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseButtonRelease, kreis.back(), Qt::LeftButton, Qt::NoButton);
    REQUIRE(fenster.zeichnung().konturen.size() == 2);
    const shape::Kontur& rund = fenster.zeichnung().konturen[1];
    CHECK(rund.kennung == 2);
    const std::vector<QPointF> rundAnker = shape::ankerpunkte(rund.pfad);
    INFO("Ankerpunkte im Kreis: " << rundAnker.size());
    CHECK(rundAnker.size() >= 6);
    CHECK(rundAnker.size() <= 40);
    for (const QPointF& a : rundAnker) CHECK(std::abs(QLineF(a, QPointF(100.0, 100.0)).length() - 60.0) < 1.5);
    CHECK(fenster.aktuelle() == 2);
    // beide spielen mit: zusammen der Umlauf ohne die zwei Spruenge, der Kreis ist laenger
    CHECK(fenster.anteil(1) + fenster.anteil(2) == doctest::Approx(1.0 - 2.0 * shape::kSprungAnteil));
    CHECK(fenster.anteil(2) > fenster.anteil(1));
    CHECK(fenster.verlauf()->grenzen().size() == 1);

    // --- Doppelklick schliesst; der erste Klick setzt seinen Punkt
    klick(*flaeche, QPointF(40.0, 40.0));
    klick(*flaeche, QPointF(80.0, 40.0));
    maus(*flaeche, QEvent::MouseButtonPress, QPointF(60.0, 80.0), Qt::LeftButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseButtonRelease, QPointF(60.0, 80.0), Qt::LeftButton, Qt::NoButton);
    maus(*flaeche, QEvent::MouseButtonDblClick, QPointF(60.0, 80.0), Qt::LeftButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseButtonRelease, QPointF(60.0, 80.0), Qt::LeftButton, Qt::NoButton);
    REQUIRE(fenster.zeichnung().konturen.size() == 3);
    CHECK(shape::ankerpunkte(fenster.zeichnung().konturen[2].pfad).size() == 3);

    // --- Enter schliesst, aber nicht aus zwei Punkten; Esc verwirft
    klick(*flaeche, QPointF(350.0, 350.0));
    klick(*flaeche, QPointF(380.0, 350.0));
    taste(*flaeche, Qt::Key_Return);
    CHECK(fenster.zeichnung().konturen.size() == 3);
    CHECK_FALSE(flaeche->angefangen().isEmpty());
    taste(*flaeche, Qt::Key_Escape);
    CHECK(flaeche->angefangen().isEmpty());
    klick(*flaeche, QPointF(350.0, 350.0));
    klick(*flaeche, QPointF(380.0, 350.0));
    klick(*flaeche, QPointF(365.0, 380.0));
    taste(*flaeche, Qt::Key_Enter);
    CHECK(fenster.zeichnung().konturen.size() == 4);
    // ein Eckpunkt und ein Zug frei Hand in einer Kontur
    klick(*flaeche, QPointF(20.0, 380.0));
    std::vector<QPointF> bogen;
    for (int n = 0; n <= 30; ++n) bogen.emplace_back(60.0 + 30.0 * std::cos(kPi * n / 30.0), 380.0 - 30.0 * std::sin(kPi * n / 30.0));
    maus(*flaeche, QEvent::MouseButtonPress, bogen.front(), Qt::LeftButton, Qt::LeftButton);
    for (std::size_t n = 1; n < bogen.size(); ++n) maus(*flaeche, QEvent::MouseMove, bogen[n], Qt::NoButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseButtonRelease, bogen.back(), Qt::LeftButton, Qt::NoButton);
    CHECK_FALSE(flaeche->angefangen().isEmpty());
    taste(*flaeche, Qt::Key_Return);
    REQUIRE(fenster.zeichnung().konturen.size() == 5);
    const QPainterPath& gemischt = fenster.zeichnung().konturen[4].pfad;
    CHECK(gemischt.elementAt(1).type == QPainterPath::LineToElement);
    CHECK(gemischt.elementAt(2).type == QPainterPath::CurveToElement);

    // --- Undo nimmt Schritt fuer Schritt zurueck, Redo bringt es wieder
    CHECK(fenster.rueckgaengig());
    CHECK(fenster.zeichnung().konturen.size() == 4);
    CHECK(fenster.wiederherstellen());
    CHECK(fenster.zeichnung().konturen.size() == 5);
    CHECK_FALSE(fenster.wiederherstellen());

    // wer Pen mit etwas Angefangenem verlaesst: was sich schliessen laesst, bleibt
    klick(*flaeche, QPointF(300.0, 20.0));
    klick(*flaeche, QPointF(330.0, 20.0));
    klick(*flaeche, QPointF(315.0, 50.0));
    fenster.setzeWerkzeug(FormFlaeche::Werkzeug::Schieben);
    CHECK(fenster.zeichnung().konturen.size() == 6);
    fenster.setzeWerkzeug(FormFlaeche::Werkzeug::Stift);
    klick(*flaeche, QPointF(300.0, 20.0));
    fenster.setzeWerkzeug(FormFlaeche::Werkzeug::Schieben);  // ein Punkt allein: verworfen
    CHECK(fenster.zeichnung().konturen.size() == 6);
    CHECK(flaeche->angefangen().isEmpty());
}

TEST_CASE("Shape Fenster: die Liste Contours, Move point, Save as... und Save, Used by")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    EigeneOrdner ordner;

    FormFenster fenster;
    fenster.flaeche()->resize(400, 400);
    const int a = fenster.fuegeKonturHinzu(dreieck());
    const int b = fenster.fuegeKonturHinzu(dreieck(60.0));
    REQUIRE(a == 1);
    REQUIRE(b == 2);
    QTreeWidget* liste = fenster.liste();

    // --- der Haken: eine Kontur ohne ihn spielt nicht mit
    liste->topLevelItem(0)->setCheckState(0, Qt::Unchecked);
    CHECK_FALSE(fenster.zeichnung().konturen[0].gewaehlt);
    CHECK(fenster.anteil(a) == doctest::Approx(-1.0));
    CHECK(fenster.anteil(b) == doctest::Approx(1.0));
    CHECK(liste->topLevelItem(0)->text(1) == QString::fromUtf8("–"));
    fenster.setzeGewaehlt(a, true);
    CHECK(liste->topLevelItem(0)->checkState(0) == Qt::Checked);

    // --- Rename, ↑ und ↓, Remove
    fenster.benenne(b, QStringLiteral("  Right  "));
    CHECK(fenster.zeichnung().konturen[1].name == QStringLiteral("Right"));
    CHECK(liste->topLevelItem(1)->text(0) == QStringLiteral("2 Right"));
    fenster.waehle(b);
    auto* hinauf = fenster.findChild<QPushButton*>(QStringLiteral("shapeUp"));
    auto* hinunter = fenster.findChild<QPushButton*>(QStringLiteral("shapeDown"));
    CHECK(hinauf->isEnabled());
    CHECK_FALSE(hinunter->isEnabled());
    hinauf->click();
    CHECK(fenster.zeichnung().konturen[0].kennung == b);
    CHECK(liste->topLevelItem(0)->text(0) == QStringLiteral("1 Right"));
    CHECK(fenster.aktuelle() == b);
    CHECK_FALSE(fenster.schiebe(b, -1));
    CHECK(fenster.schiebe(b, +1));
    CHECK(fenster.zeichnung().konturen[1].kennung == b);
    const int c = fenster.fuegeKonturHinzu(dreieck(-40.0));
    CHECK(c == 3);
    CHECK(fenster.entferne(c));
    CHECK_FALSE(fenster.entferne(c));
    CHECK(fenster.zeichnung().konturen.size() == 2);
    CHECK(fenster.zeichnung().naechsteKennung == 4);  // eine geloeschte Kennung kommt nicht wieder

    // --- Move point: mit der Maus ein Punkt — ein Schritt fuer Undo, gerechnet beim Loslassen
    fenster.setzeWerkzeug(FormFlaeche::Werkzeug::Schieben);
    FormFlaeche* flaeche = fenster.flaeche();
    const QPointF spitze = flaeche->imBild(QPointF(100.0, 150.0));  // die untere Ecke des ersten Dreiecks
    const auto vorher = fenster.verlauf()->links();
    maus(*flaeche, QEvent::MouseButtonPress, spitze, Qt::LeftButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseMove, spitze + QPointF(0.0, 20.0), Qt::NoButton, Qt::LeftButton);
    maus(*flaeche, QEvent::MouseMove, spitze + QPointF(0.0, 40.0), Qt::NoButton, Qt::LeftButton);
    CHECK(fenster.verlauf()->links() == vorher);  // waehrend des Ziehens wird nicht gerechnet
    maus(*flaeche, QEvent::MouseButtonRelease, spitze + QPointF(0.0, 40.0), Qt::LeftButton, Qt::NoButton);
    const shape::Kontur* links = fenster.zeichnung().finde(a);
    REQUIRE(links != nullptr);
    CHECK(shape::ankerpunkte(links->pfad)[2] == QPointF(100.0, 170.0));
    CHECK(fenster.verlauf()->links() != vorher);
    CHECK(fenster.rueckgaengig());
    CHECK(shape::ankerpunkte(fenster.zeichnung().finde(a)->pfad)[2] == QPointF(100.0, 150.0));
    CHECK(fenster.schiebeAnker(a, 0, QPointF(40.0, 40.0)));
    CHECK(shape::ankerpunkte(fenster.zeichnung().finde(a)->pfad)[0] == QPointF(40.0, 40.0));
    CHECK(shape::pfadAlsText(fenster.zeichnung().finde(a)->pfad).endsWith(QStringLiteral("L 40 40 Z")));
    CHECK_FALSE(fenster.schiebeAnker(a, 3, QPointF()));

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_FENSTER_BILD"); !ziel.isEmpty())
    {
        fenster.setAttribute(Qt::WA_DontShowOnScreen);
        fenster.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        CHECK(fenster.grab().save(ziel));
        fenster.hide();
    }

    // --- Save as...: eine neue eigene Form; ein vergebener Name geht nicht
    CHECK(fenster.vorschlagTitel() == QStringLiteral("My shape"));
    bool geht = false;
    CHECK(FormFenster::dateiZeile(QStringLiteral("Zwei Dreiecke"), &geht) == QStringLiteral("File: My files · contours · zwei_dreiecke.lvcontour"));
    CHECK(geht);
    CHECK(FormFenster::dateiZeile(QStringLiteral("!!"), &geht) == QStringLiteral("Type a name with letters or digits"));
    CHECK_FALSE(geht);
    const std::uint64_t formVorher = shape::zeichnungFassung();
    const std::uint64_t instrumentVorher = instrumentFassung();
    QStringList gemeldet;
    QObject::connect(&fenster, &FormFenster::gespeichert, [&gemeldet](const QString& n) { gemeldet << n; });
    const QString nennung = fenster.speichereAls(QStringLiteral("Zwei Dreiecke"));
    REQUIRE(nennung == QStringLiteral("user:zwei_dreiecke"));
    CHECK(gemeldet == QStringList{nennung});
    CHECK(shape::zeichnungFassung() > formVorher);  // der Knoten Shape und der Creator lesen neu
    CHECK(instrumentFassung() > instrumentVorher);  // gerechnete Toene entstehen neu
    CHECK(fenster.herkunft() == FormFenster::Herkunft::Eigene);
    CHECK_FALSE(fenster.geaendert());
    shape::Zeichnung gelesen;
    REQUIRE(shape::holeZeichnung(nennung, gelesen));
    CHECK(gelesen.titel == QStringLiteral("Zwei Dreiecke"));
    REQUIRE(gelesen.konturen.size() == 2);
    CHECK(gelesen.konturen[1].name == QStringLiteral("Right"));
    CHECK(gelesen.konturen == fenster.zeichnung().konturen);
    CHECK(FormFenster::dateiZeile(QStringLiteral("Zwei Dreiecke"), &geht).contains(QStringLiteral("already holds")));
    CHECK_FALSE(geht);
    CHECK(fenster.speichereAls(QStringLiteral("Zwei Dreiecke")).isEmpty());

    // --- Used by: ein eigenes Instrument mit einer Shape-Schicht, die die Form nennt
    Instrument lead;
    lead.titel = QStringLiteral("Triangle Lead");
    lead.schichten[0].schwingung = Schwingung::Form;
    lead.schichten[0].form = nennung;
    REQUIRE(speichereEigenes(lead, QStringLiteral("triangle_lead")));
    Instrument anderes;
    anderes.titel = QStringLiteral("Heart Pad");
    anderes.schichten[0].schwingung = Schwingung::Form;
    anderes.schichten[0].form = QStringLiteral("heart");
    REQUIRE(speichereEigenes(anderes, QStringLiteral("heart_pad")));
    CHECK(eigeneInstrumenteMitForm(nennung) == QStringList{QStringLiteral("Triangle Lead")});
    CHECK(eigeneInstrumenteMitForm(QStringLiteral("heart")) == QStringList{QStringLiteral("Heart Pad")});

    // --- wieder oeffnen und Save: ersetzt, nach einer Rueckfrage mit den Instrumenten
    FormFenster zweites;
    REQUIRE(zweites.bearbeite(nennung));
    CHECK(zweites.windowTitle() == QStringLiteral("Edit shape – Zwei Dreiecke"));
    CHECK(zweites.findChild<QLabel*>(QStringLiteral("shapeLocation"))->text() == QStringLiteral("My shapes"));
    CHECK(zweites.nutzerZeile() == QStringLiteral("Used by: Triangle Lead"));
    CHECK(zweites.ersetzenFrage().contains(QStringLiteral("Triangle Lead — they will play the new version")));
    CHECK(zweites.vorschlagTitel() == QStringLiteral("Zwei Dreiecke 2"));
    CHECK(zweites.findChild<QPushButton*>(QStringLiteral("shapeSave"))->isEnabled());
    zweites.benenne(b, QStringLiteral("Rechts"));
    CHECK(zweites.speichere() == nennung);
    REQUIRE(shape::holeZeichnung(nennung, gelesen));
    CHECK(gelesen.konturen[1].name == QStringLiteral("Rechts"));

    // --- eine mitgelieferte Form: lesen ja, Save nein
    FormFenster drittes;
    REQUIRE(drittes.bearbeite(QStringLiteral("heart")));
    CHECK(drittes.herkunft() == FormFenster::Herkunft::Mitgeliefert);
    CHECK(drittes.findChild<QLabel*>(QStringLiteral("shapeLocation"))->text() == QStringLiteral("Built-in, read-only"));
    CHECK_FALSE(drittes.findChild<QPushButton*>(QStringLiteral("shapeSave"))->isEnabled());
    CHECK(drittes.speichere().isEmpty());
    CHECK(drittes.nutzerZeile() == QStringLiteral("Used by: Heart Pad"));
    // Use as ist grau, zeigt aber, was in der Datei steht
    CHECK(drittes.findChild<QRadioButton*>(QStringLiteral("shapeUseAsLayers"))->isChecked());
    CHECK(drittes.findChild<QComboBox*>(QStringLiteral("shapeJumps"))->currentIndex() == 0);
    CHECK(drittes.vorschlagTitel() == QStringLiteral("Heart 2"));
    CHECK_FALSE(drittes.bearbeite(QStringLiteral("gibt_es_nicht")));
    CHECK(drittes.nennung() == QStringLiteral("heart"));  // bleibt, wie es war
}

TEST_CASE("Shape Fenster: Preview und Play — ein Umlauf links zur Seite, rechts nach oben, ohne Naht")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    FormFenster fenster;
    REQUIRE(fenster.bearbeite(QStringLiteral("circle")));
    auto* play = fenster.findChild<QPushButton*>(QStringLiteral("shapePlay"));
    CHECK_FALSE(play->isEnabled());  // ohne Engine
    CHECK_FALSE(fenster.spiele());

    // der Kreis: links ein Kosinus, rechts derselbe um ein Viertel versetzt
    const std::vector<float> klang = fenster.klang();
    REQUIRE(klang.size() == static_cast<std::size_t>(2 * FormFenster::kPlayRate));
    float spitze = 0.0f;
    for (const float wert : klang) spitze = std::max(spitze, std::abs(wert));
    CHECK(spitze == doctest::Approx(0.4).epsilon(0.02));
    // 11 Umlaeufe sind genau 2400 Abtastwerte: dort wiederholt es sich
    CHECK(klang[2 * 100] == doctest::Approx(klang[2 * 2500]).epsilon(1e-4));
    CHECK(klang[2 * 100 + 1] == doctest::Approx(klang[2 * 2500 + 1]).epsilon(1e-4));
    // die Sekunde wiederholt ohne Naht: der letzte Wert liegt beim ersten
    CHECK(std::abs(klang[2 * (FormFenster::kPlayRate - 1)] - klang[0]) < 0.02f);

    // Harmonics: 1 macht aus jeder Form eine Ellipse
    fenster.setzeHarmonische(0.3);
    CHECK(fenster.harmonische() == doctest::Approx(1.0));
    fenster.setzeHarmonische(5000.0);
    CHECK(fenster.harmonische() == doctest::Approx(shape::kHarmonischeMax));

    papp::AufnahmeEngine engine;
    fenster.setEngine(&engine);
    CHECK(play->isEnabled());
    play->click();
    REQUIRE(fenster.spielt());
    REQUIRE(engine.generator != nullptr);
    std::vector<float> aus(2 * 2400, 0.0f);
    engine.generator->render(aus.data(), 2400);
    float laut = 0.0f;
    for (const float wert : aus) laut = std::max(laut, std::abs(wert));
    CHECK(laut > 0.3f);
    CHECK(play->text() == QStringLiteral("■ Stop playing"));
    play->click();
    CHECK_FALSE(fenster.spielt());
    fenster.setEngine(nullptr);
}

TEST_CASE("Shape Fenster im Creator: Edit... und New shape... — die Schicht spielt, was gespeichert wird")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    EigeneOrdner ordner;

    InstrumentCreator creator;  // nicht gezeigt: kein Fenster
    Instrument neu;
    neu.titel = QStringLiteral("Shape test");
    neu.schichten[0].schwingung = Schwingung::Form;
    neu.schichten[0].form = QStringLiteral("heart");
    creator.oeffne(neu);
    auto* bearbeiten = creator.findChild<QPushButton*>(QStringLiteral("creatorShapeEdit"));
    REQUIRE(bearbeiten != nullptr);
    CHECK(bearbeiten->isEnabled());
    CHECK(bearbeiten->text() == QStringLiteral("Edit..."));
    auto* formName = creator.findChild<QLabel*>(QStringLiteral("creatorShapeName"));

    // --- Edit...: die mitgelieferte Form oeffnet sich; Save as... legt eine eigene an, die Schicht spielt sie
    REQUIRE(creator.oeffneFormBearbeiten(false));
    FormFenster* fenster = creator.formFenster();
    REQUIRE(fenster != nullptr);
    CHECK(fenster->nennung() == QStringLiteral("heart"));
    fenster->benenne(1, QStringLiteral("Mein Herz"));
    REQUIRE(fenster->speichereAls(QStringLiteral("Herz 2")) == QStringLiteral("user:herz_2"));
    CHECK(creator.schicht().form == QStringLiteral("user:herz_2"));
    CHECK(creator.schicht().formKontur == 0);
    CHECK(formName->text() == QStringLiteral("Herz 2"));

    // --- New shape...: leer; was gespeichert wird, spielt die Schicht
    REQUIRE(creator.oeffneNeueForm(false));
    CHECK(fenster->herkunft() == FormFenster::Herkunft::Neu);
    CHECK(fenster->zeichnung().konturen.empty());
    const int a = fenster->fuegeKonturHinzu(dreieck());
    REQUIRE(fenster->speichereAls(QStringLiteral("Dreieck")) == QStringLiteral("user:dreieck"));
    CHECK(creator.schicht().form == QStringLiteral("user:dreieck"));
    CHECK(formName->text() == QStringLiteral("Dreieck"));

    // --- dieselbe Form neu geschrieben: die gewaehlte Kontur bleibt — bis es sie nicht mehr gibt
    creator.aendereSchicht([a](Schicht& s) { s.formKontur = a; });
    REQUIRE(creator.oeffneFormBearbeiten(false));
    const int b = fenster->fuegeKonturHinzu(dreieck(60.0));
    REQUIRE(fenster->speichere() == QStringLiteral("user:dreieck"));
    CHECK(creator.schicht().formKontur == a);
    auto* kontur = creator.findChild<QComboBox*>(QStringLiteral("creatorShapeContour"));
    REQUIRE(kontur != nullptr);
    CHECK(kontur->count() == 3);  // All, one path und zwei Konturen: neu gelesen
    REQUIRE(creator.oeffneFormBearbeiten(false));
    REQUIRE(fenster->entferne(a));
    REQUIRE(fenster->speichere() == QStringLiteral("user:dreieck"));
    CHECK(creator.schicht().formKontur == 0);
    // das Bild „Shape, as it plays" zeigt die neue Form: das zweite Dreieck allein
    REQUIRE(creator.formBild() != nullptr);
    CHECK_FALSE(creator.formBild()->zeichnung().isEmpty());
    (void)b;

    // --- eine Form, die es nicht gibt: Edit... ist grau
    creator.setzeForm(QStringLiteral("gibt_es_nicht"));
    CHECK_FALSE(bearbeiten->isEnabled());
    CHECK_FALSE(creator.oeffneFormBearbeiten(false));
    // keine Shape-Schicht: weder Edit... noch New shape...
    creator.aendereSchicht([](Schicht& s) { s.schwingung = Schwingung::Sinus; });
    CHECK_FALSE(creator.oeffneFormBearbeiten(false));
    CHECK_FALSE(creator.oeffneNeueForm(false));
}
