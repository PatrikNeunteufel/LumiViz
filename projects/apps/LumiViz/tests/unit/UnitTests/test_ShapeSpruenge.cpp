/**
 * @file test_ShapeSpruenge.cpp
 * @brief Klang aus einer Form, Stufe 20d: mehrere Konturen — die drei Arten
 *        der Spruenge im Ton, Bridge und Use as im Fenster Edit shape, Save
 *        mit Layers im Creator und die Farben der Schichten
 *        (docs/audio/Shape_Konzept.md §6, §8a, §9; Entscheide Patrik B1,
 *        B2 d, G1 a, G2 a, S120, und L1 a, F1 a, S122)
 *
 * Ohne Ton und ohne gezeigtes Fenster; eigene Formen und Instrumente in
 * Wegwerf-Ordnern. `LUMIVIZ_SHAPE_SPRUENGE_BILD=<datei.png>` legt ein Bild des
 * Fensters mit Bruecken und eines des Creators mit farbigen Schichten
 * (`_creator`) ab.
 */

#include <doctest.h>

#include "loop/FormFenster.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "shape/Kontur.hpp"
#include "shape/Tonform.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QLabel>
#include <QLineF>
#include <QListWidget>
#include <QRadioButton>
#include <QTemporaryDir>

#include <cmath>
#include <vector>

using namespace lumi::loop;
namespace shape = lumi::shape;

namespace
{

QPainterPath quadrat(double x, double y, double seite = 20.0)
{
    QPainterPath pfad;
    pfad.addRect(x, y, seite, seite);
    return pfad;
}

double gesamtLaenge(const std::vector<std::pair<QPointF, QPointF>>& linien)
{
    double summe = 0.0;
    for (const auto& [von, nach] : linien) summe += QLineF(von, nach).length();
    return summe;
}

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

}  // namespace

TEST_CASE("Shape Spruenge: in Reihenfolge der Liste, auf den kuerzesten Wegen, von Hand — und keine Ringe")
{
    shape::Zeichnung z;
    z.nutzung = shape::Nutzung::EinPfad;

    // --- zwei Quadrate in Reihenfolge der Liste: zwei Spruenge, je von Kante zu Kante (30)
    const int a = z.neueKontur(quadrat(0.0, 0.0));
    const int b = z.neueKontur(quadrat(50.0, 0.0));
    auto linien = shape::sprungLinien(z);
    REQUIRE(linien.size() == 2);
    CHECK(QLineF(linien[0].first, linien[0].second).length() == doctest::Approx(30.0).epsilon(0.02));
    CHECK(QLineF(linien[1].first, linien[1].second).length() == doctest::Approx(30.0).epsilon(0.02));
    CHECK(shape::tonform(z, 0).konturen.size() == 2);
    // eine Kontur allein: kein Sprung
    CHECK(shape::sprungLinien(shape::Zeichnung{}).empty());

    // --- Shortest: vier Quadrate an den Ecken, die Liste im Zickzack — die kuerzesten Wege sind kuerzer
    shape::Zeichnung ecken;
    ecken.nutzung = shape::Nutzung::EinPfad;
    (void)ecken.neueKontur(quadrat(0.0, 0.0));
    (void)ecken.neueKontur(quadrat(100.0, 100.0));
    (void)ecken.neueKontur(quadrat(100.0, 0.0));
    (void)ecken.neueKontur(quadrat(0.0, 100.0));
    const double liste = gesamtLaenge(shape::sprungLinien(ecken));
    ecken.spruenge = shape::Spruenge::Kuerzeste;
    const double kurz = gesamtLaenge(shape::sprungLinien(ecken));
    INFO("Liste " << liste << ", kuerzeste " << kurz);
    CHECK(kurz < liste - 50.0);  // die Liste waehlt schon naechste Punkte: 386 gegen 320
    CHECK(kurz == doctest::Approx(4.0 * 80.0).epsilon(0.03));  // einmal herum, je 80 von Kante zu Kante
    CHECK(shape::sprungLinien(ecken).size() == 4);

    // --- von Hand: A — B — C in einer Reihe, B hat zwei Bruecken
    shape::Zeichnung reihe;
    reihe.nutzung = shape::Nutzung::EinPfad;
    reihe.spruenge = shape::Spruenge::VonHand;
    const int ra = reihe.neueKontur(quadrat(0.0, 0.0));
    const int rb = reihe.neueKontur(quadrat(50.0, 0.0));
    const int rc = reihe.neueKontur(quadrat(100.0, 0.0));
    CHECK(shape::brueckeErlaubt(reihe, ra, rb));
    reihe.bruecken.push_back({ra, QPointF(20.0, 10.0), rb, QPointF(50.0, 10.0)});
    CHECK(shape::brueckeErlaubt(reihe, rb, rc));
    reihe.bruecken.push_back({rb, QPointF(70.0, 10.0), rc, QPointF(100.0, 10.0)});
    CHECK_FALSE(shape::brueckeErlaubt(reihe, ra, rc));  // schloesse einen Ring ueber B
    CHECK_FALSE(shape::brueckeErlaubt(reihe, ra, ra));
    CHECK_FALSE(shape::brueckeErlaubt(reihe, ra, 99));
    linien = shape::sprungLinien(reihe);
    // B wird an der Bruecke zu C verlassen und kommt dort zurueck: vier Spruenge, jeder eine Bruecke (30)
    REQUIRE(linien.size() == 4);
    for (const auto& [von, nach] : linien) CHECK(QLineF(von, nach).length() == doctest::Approx(30.0).epsilon(0.02));
    const shape::Tonform form = shape::tonform(reihe, 0);
    REQUIRE(form.konturen.size() == 4);
    double summe = 4.0 * form.sprung;
    double teilVonB = 0.0;
    for (const shape::TonKontur& k : form.konturen)
    {
        summe += k.anteil;
        if (k.umfang < 1.0) teilVonB += k.umfang;
    }
    CHECK(summe == doctest::Approx(1.0));
    CHECK(teilVonB == doctest::Approx(1.0).epsilon(0.01));  // die beiden Stuecke von B ergeben B einmal ganz
    // der Ton bleibt im Rahmen und springt nicht ins Leere
    for (int n = 0; n < 2000; ++n)
    {
        const shape::Punkt p = shape::tonwert(form, n / 2000.0, 48.0);
        CHECK(std::isfinite(p.real()));
        CHECK(std::abs(p) < 1.3);
    }

    // eine Kontur ohne Bruecke kommt in Reihenfolge der Liste dazu
    (void)reihe.neueKontur(quadrat(0.0, 60.0));
    CHECK(shape::sprungLinien(reihe).size() == 5);
    // eine Kontur ohne Haken spielt nicht mit, ihre Bruecken auch nicht
    reihe.konturen[0].gewaehlt = false;
    CHECK(shape::sprungLinien(reihe).size() == 3);  // B, C, D: die Bruecke B–C hin und zurueck, D angehaengt
    (void)a;
    (void)b;
}

TEST_CASE("Shape Spruenge im Fenster: Bridge mit der Maus, Ring abgewiesen, entfernen, Use as und Jumps, die Zahlen")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    FormFenster fenster;
    fenster.flaeche()->resize(400, 400);  // die leere Ansicht 0 … 200: ein Bildpunkt ist eine halbe Einheit
    const int a = fenster.fuegeKonturHinzu(quadrat(10.0, 10.0));
    const int b = fenster.fuegeKonturHinzu(quadrat(60.0, 10.0));
    const int c = fenster.fuegeKonturHinzu(quadrat(110.0, 10.0));
    const shape::Zeichnung& z = fenster.zeichnung();
    auto* layers = fenster.findChild<QRadioButton*>(QStringLiteral("shapeUseAsLayers"));
    auto* zahlen = fenster.findChild<QLabel*>(QStringLiteral("shapeUseAsCount"));
    REQUIRE(layers != nullptr);
    CHECK(layers->isChecked());
    CHECK(layers->isEnabled());
    CHECK(zahlen->text() == QStringLiteral("3 ticked contours · one layer each"));

    // --- One path, in Reihenfolge der Liste: drei Spruenge
    fenster.setzeNutzung(shape::Nutzung::EinPfad);
    CHECK(z.nutzung == shape::Nutzung::EinPfad);
    CHECK(fenster.zahlenZeile() == QStringLiteral("3 contours · 3 jumps"));
    fenster.setzeSpruenge(shape::Spruenge::VonHand);
    CHECK(fenster.zahlenZeile().contains(QStringLiteral("No bridges yet")));

    // --- Bridge: ein Punkt auf A, einer auf B
    fenster.setzeWerkzeug(FormFlaeche::Werkzeug::Bruecke);
    fenster.brueckeKlick(QPointF(30.0, 20.0));  // die rechte Kante von A
    CHECK(fenster.brueckenMeldung().contains(QStringLiteral("another contour")));
    fenster.brueckeKlick(QPointF(28.0, 20.0));  // noch einmal A: eine Bruecke verbindet zwei verschiedene
    CHECK(fenster.brueckenMeldung().contains(QStringLiteral("two different contours")));
    fenster.brueckeKlick(QPointF(60.0, 20.0));  // die linke Kante von B
    REQUIRE(z.bruecken.size() == 1);
    CHECK(z.bruecken[0].von == a);
    CHECK(z.bruecken[0].nach == b);
    CHECK(QLineF(z.bruecken[0].vonPunkt, QPointF(30.0, 20.0)).length() < 1.0);
    CHECK(fenster.zahlenZeile() == QStringLiteral("3 contours · 3 jumps\n1 not reached by a bridge — added in list order"));
    // daneben ins Leere
    fenster.brueckeKlick(QPointF(45.0, 150.0));
    CHECK(fenster.brueckenMeldung() == QStringLiteral("Click on a contour"));

    // --- B — C, dann A — C: ein Ring, nicht genommen
    fenster.brueckeKlick(QPointF(80.0, 20.0));
    fenster.brueckeKlick(QPointF(110.0, 20.0));
    REQUIRE(z.bruecken.size() == 2);
    CHECK(fenster.zahlenZeile() == QStringLiteral("3 contours · 4 jumps"));
    fenster.brueckeKlick(QPointF(20.0, 30.0));   // die untere Kante von A
    fenster.brueckeKlick(QPointF(120.0, 30.0));  // die untere Kante von C
    CHECK(z.bruecken.size() == 2);
    CHECK(fenster.brueckenMeldung().contains(QStringLiteral("close a ring")));

    // --- ein Klick auf die Mitte einer Bruecke entfernt sie; Undo bringt sie wieder
    fenster.brueckeKlick(QPointF(45.0, 20.0));
    CHECK(z.bruecken.size() == 1);
    CHECK(fenster.brueckenMeldung() == QStringLiteral("Bridge removed"));
    REQUIRE(fenster.rueckgaengig());
    CHECK(z.bruecken.size() == 2);

    // --- per Aufruf: setzt One path und Drawn by hand; Remove einer Kontur nimmt ihre Bruecken mit
    fenster.setzeNutzung(shape::Nutzung::Schichten);
    CHECK_FALSE(fenster.setzeBruecke(a, QPointF(), c, QPointF()));  // Ring
    REQUIRE(fenster.entferne(b));
    CHECK(z.bruecken.empty());
    REQUIRE(fenster.setzeBruecke(a, QPointF(30.0, 20.0), c, QPointF(110.0, 20.0)));
    CHECK(z.nutzung == shape::Nutzung::EinPfad);
    CHECK(z.spruenge == shape::Spruenge::VonHand);
    CHECK(fenster.findChild<QRadioButton*>(QStringLiteral("shapeUseAsOnePath"))->isChecked());
    CHECK_FALSE(layers->isChecked());
    CHECK_FALSE(fenster.entferneBruecke(5));

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_SPRUENGE_BILD"); !ziel.isEmpty())
    {
        FormFenster gezeigt;
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.show();
        QCoreApplication::processEvents();
        const int ga = gezeigt.fuegeKonturHinzu(quadrat(20.0, 40.0, 40.0));
        const int gb = gezeigt.fuegeKonturHinzu(quadrat(80.0, 40.0, 30.0));
        const int gc = gezeigt.fuegeKonturHinzu(quadrat(130.0, 40.0, 40.0));
        (void)gezeigt.fuegeKonturHinzu(quadrat(60.0, 120.0, 40.0));
        REQUIRE(gezeigt.setzeBruecke(ga, QPointF(60.0, 60.0), gb, QPointF(80.0, 55.0)));
        REQUIRE(gezeigt.setzeBruecke(gb, QPointF(110.0, 55.0), gc, QPointF(130.0, 60.0)));
        gezeigt.setzeWerkzeug(FormFlaeche::Werkzeug::Bruecke);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        gezeigt.hide();
    }
}

TEST_CASE("Shape Spruenge im Creator: Save mit Layers legt Kopien an, bis acht — und die Farben der Schichten")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    EigeneOrdner ordner;

    // --- die Farben (F1 a): Familien je Waveform, die n-te derselben Art die n-te Abstufung
    Instrument i;
    i.schichten.assign(5, Schicht{});
    i.schichten[1].schwingung = Schwingung::Saege;
    i.schichten[3].schwingung = Schwingung::Saege;
    const QColor sinus1 = schichtFarbe(i, 0);
    const QColor sinus2 = schichtFarbe(i, 2);
    const QColor saege1 = schichtFarbe(i, 1);
    CHECK(sinus1.isValid());
    CHECK(sinus1 != sinus2);
    CHECK(sinus1.blue() > sinus1.red());  // Sine: Blau
    CHECK(saege1.red() > saege1.blue());  // Saw: Rot
    CHECK(schichtFarbe(i, 3) != saege1);
    CHECK(schichtFarbe(i, 4) == QColor(QStringLiteral("#2556a8")));  // die dritte Sine-Schicht: die dritte Abstufung
    CHECK_FALSE(schichtFarbe(i, 9).isValid());
    // eine eigene Farbe gilt statt der abgeleiteten — und steht in der Datei
    i.schichten[0].farbe = QStringLiteral("#123456");
    CHECK(schichtFarbe(i, 0) == QColor(QStringLiteral("#123456")));
    CHECK(abgeleiteteSchichtFarbe(i, 0) == sinus1);
    Instrument gelesen;
    REQUIRE(ausJson(alsJson(i), gelesen));
    CHECK(gelesen.schichten[0].farbe == QStringLiteral("#123456"));
    CHECK(gelesen.schichten[1].farbe.isEmpty());
    QJsonObject kaputt = alsJson(i);
    QJsonArray schichten = kaputt.value(QStringLiteral("layers")).toArray();
    QJsonObject erste = schichten[0].toObject();
    erste[QStringLiteral("color")] = QStringLiteral("keine farbe");
    schichten[0] = erste;
    kaputt[QStringLiteral("layers")] = schichten;
    REQUIRE(ausJson(kaputt, gelesen));
    CHECK(gelesen.schichten[0].farbe.isEmpty());  // keine Farbe: die abgeleitete

    // --- eine Form mit drei Konturen in eigenen Farben; Shape-Schichten tragen die Farbe ihrer Kontur
    shape::Zeichnung drei;
    drei.titel = QStringLiteral("Drei");
    const int k1 = drei.neueKontur(quadrat(0.0, 0.0), QStringLiteral("Links"), QStringLiteral("#ff0000"));
    const int k2 = drei.neueKontur(quadrat(50.0, 0.0), QStringLiteral("Mitte"), QStringLiteral("#00ff00"));
    const int k3 = drei.neueKontur(quadrat(100.0, 0.0), QString(), QStringLiteral("#0000ff"));
    REQUIRE(shape::speichereEigeneZeichnung(drei, QStringLiteral("drei")));

    InstrumentCreator creator;  // nicht gezeigt
    Instrument neu;
    neu.titel = QStringLiteral("Drei Schichten");
    neu.schichten[0].schwingung = Schwingung::Form;
    neu.schichten[0].form = QStringLiteral("user:drei");
    neu.schichten[0].name = QStringLiteral("Pad");
    neu.schichten[0].laut = 0.5;
    neu.schichten[0].farbe = QStringLiteral("#abcdef");
    creator.oeffne(neu);

    // --- Save mit Layers: die offene Schicht die erste, je weitere eine Kopie dahinter
    REQUIRE(creator.oeffneFormBearbeiten(false));
    REQUIRE(creator.formFenster()->speichere() == QStringLiteral("user:drei"));
    const auto& s = creator.instrument().schichten;
    REQUIRE(s.size() == 3);
    CHECK(s[0].formKontur == k1);
    CHECK(s[1].formKontur == k2);
    CHECK(s[2].formKontur == k3);
    CHECK(s[1].name == QStringLiteral("Pad · Mitte"));
    CHECK(s[2].name == QStringLiteral("Pad · Contour"));
    CHECK(s[2].laut == doctest::Approx(0.5));  // eine Kopie der offenen Schicht
    CHECK(s[0].farbe == QStringLiteral("#abcdef"));  // die offene behaelt ihre eigene Farbe
    CHECK(s[1].farbe.isEmpty());                     // die Kopien tragen die ihrer Kontur
    CHECK(schichtFarbe(creator.instrument(), 1) == QColor(QStringLiteral("#00ff00")));
    CHECK(schichtFarbe(creator.instrument(), 2) == QColor(QStringLiteral("#0000ff")));
    CHECK(creator.formMeldung().isEmpty());
    // in der Liste der Schichten: der Farbpunkt
    auto* liste = creator.findChild<QListWidget*>(QStringLiteral("creatorLayers"));
    REQUIRE(liste != nullptr);
    CHECK(liste->item(1)->data(Qt::UserRole).toString() == QStringLiteral("#00ff00"));
    CHECK_FALSE(liste->item(1)->icon().isNull());
    // die eigene Farbe einer Schicht, und Automatic
    creator.setzeSchichtFarbe(2, QStringLiteral("#ffff00"));
    CHECK(liste->item(2)->data(Qt::UserRole).toString() == QStringLiteral("#ffff00"));
    creator.setzeSchichtFarbe(2, QString());
    CHECK(liste->item(2)->data(Qt::UserRole).toString() == QStringLiteral("#0000ff"));

    // --- One path: die offene Schicht spielt alle als einen Pfad
    REQUIRE(creator.oeffneFormBearbeiten(false));
    creator.formFenster()->setzeNutzung(shape::Nutzung::EinPfad);
    REQUIRE(creator.formFenster()->speichere() == QStringLiteral("user:drei"));
    CHECK(creator.schicht().formKontur == 0);
    CHECK(creator.instrument().schichten.size() == 3);

    // --- bis acht: mehr Konturen, als Platz ist — so viele, wie passen, und die Meldung
    shape::Zeichnung zehn;
    zehn.titel = QStringLiteral("Zehn");
    for (int n = 0; n < 10; ++n) (void)zehn.neueKontur(quadrat(n * 30.0, 0.0));
    REQUIRE(shape::speichereEigeneZeichnung(zehn, QStringLiteral("zehn")));
    creator.waehle(0);
    REQUIRE(creator.schicht().schwingung == Schwingung::Form);
    creator.setzeForm(QStringLiteral("user:zehn"));
    REQUIRE(creator.oeffneFormBearbeiten(false));
    REQUIRE(creator.formFenster()->speichere() == QStringLiteral("user:zehn"));
    CHECK(creator.instrument().schichten.size() == static_cast<std::size_t>(kSchichtenMax));
    // drei waren da (eine davon die offene, sie spielt die erste), fuenf kamen dazu, vier der neun weiteren passten nicht
    CHECK(creator.formMeldung() == QStringLiteral("4 ticked contours did not fit — an instrument holds 8 layers"));

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_SPRUENGE_BILD"); !ziel.isEmpty())
    {
        InstrumentCreator gezeigt;
        Instrument bunt;
        bunt.titel = QStringLiteral("Colours");
        bunt.schichten.assign(6, Schicht{});
        bunt.schichten[1].schwingung = Schwingung::Saege;
        bunt.schichten[2].schwingung = Schwingung::Saege;
        bunt.schichten[3].schwingung = Schwingung::Rechteck;
        bunt.schichten[4].schwingung = Schwingung::Form;
        bunt.schichten[4].form = QStringLiteral("user:drei");
        bunt.schichten[4].formKontur = k2;
        bunt.schichten[5].farbe = QStringLiteral("#ff00ff");
        gezeigt.oeffne(bunt);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.show();
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(QString(ziel).replace(QStringLiteral(".png"), QStringLiteral("_creator.png"))));
        gezeigt.hide();
    }
}

TEST_CASE("Shape Sichttest-Vorlagen: Shape.lvfx und XyScope.lvfx laden mit ihrem Knoten")
{
    // die Vorlagen der Liste Sichttest_Shape (S122) — vom Arbeitsordner aus (die Test-Exe aus dem Projekt-Root starten)
    namespace me = lumi::multieffect;
    me::ChainNode kette;
    QStringList bericht;
    REQUIRE(me::loadChainFromFile(QDir::current().filePath(QStringLiteral("asset/sichttest/presets/Shape.lvfx")), kette, &bericht));
    REQUIRE(kette.children.size() == 1);
    const auto* form = std::get_if<me::ShapeParams>(&kette.children[0].params);
    REQUIRE(form != nullptr);
    CHECK(form->shape == "heart");
    CHECK(form->showCircles);
    me::ChainNode scope;
    REQUIRE(me::loadChainFromFile(QDir::current().filePath(QStringLiteral("asset/sichttest/presets/XyScope.lvfx")), scope, &bericht));
    REQUIRE(scope.children.size() == 1);
    const auto* xy = std::get_if<me::XyScopeParams>(&scope.children[0].params);
    REQUIRE(xy != nullptr);
    CHECK_FALSE(xy->rotate45);  // eine Schicht mit Stereo XY zeichnet ihre Form aufrecht
}
