/**
 * @file test_ShapeBelege.cpp
 * @brief Klang aus einer Form: zwei Belege vor dem Bau (docs/audio/
 *        Shape_Konzept.md §7 und §8; Entscheide Patrik S120: D4 b, S1 a)
 *
 * 1. **SVG (S1 a):** Qt Svg gibt die Pfade eines SVG nicht heraus, zeichnet
 *    sie aber. Der Beleg laesst `QSvgRenderer` auf ein eigenes Zeichengeraet
 *    zeichnen, das nichts malt und jeden Aufruf mitschreibt — und prueft, dass
 *    Kurven als Kurven ankommen, an der richtigen Stelle, mit ihrer Farbe.
 *    Faellt er durch, gilt S1 b (die Datei selbst lesen).
 * 2. **Pfade ohne Anwendung (D4 b):** `QPainterPath` traegt Strecken und
 *    Kurven und rechnet sie zu Punkten um, ohne dass eine Qt-Anwendung laeuft
 *    — die Tests des Loop-Kerns laufen ohne.
 *
 * Kein Fenster, kein Ton. Der erste Fall braucht eine `QApplication`.
 */

#include <doctest.h>

#include <QApplication>
#include <QBrush>
#include <QByteArray>
#include <QColor>
#include <QList>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QSvgRenderer>
#include <QTransform>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace
{

/// Ein Aufruf, wie er beim Zeichengeraet ankam — der Pfad schon an seiner Stelle.
struct Gezeichnet
{
    QPainterPath pfad;
    QColor fuellung;  ///< ungueltig = nicht gefuellt
    QColor strich;    ///< ungueltig = ohne Strich
    std::string weg;  ///< ueber welchen Aufruf er kam
};

bool hatKurve(const QPainterPath& pfad)
{
    for (int i = 0; i < pfad.elementCount(); ++i)
        if (pfad.elementAt(i).type == QPainterPath::CurveToElement) return true;
    return false;
}

/// Malt nichts: schreibt mit, was gezeichnet werden soll.
class Schreiber : public QPaintEngine
{
public:
    Schreiber() : QPaintEngine(QPaintEngine::AllFeatures) {}

    using QPaintEngine::drawEllipse;
    using QPaintEngine::drawPolygon;
    using QPaintEngine::drawRects;

    bool begin(QPaintDevice*) override { return true; }
    bool end() override { return true; }
    Type type() const override { return QPaintEngine::User; }

    void updateState(const QPaintEngineState& stand) override
    {
        const QPaintEngine::DirtyFlags was = stand.state();
        if (was.testFlag(QPaintEngine::DirtyTransform)) m_lage = stand.transform();
        if (was.testFlag(QPaintEngine::DirtyBrush)) m_pinsel = stand.brush();
        if (was.testFlag(QPaintEngine::DirtyPen)) m_stift = stand.pen();
    }

    void drawPath(const QPainterPath& pfad) override { merke(pfad, "drawPath"); }
    void drawRects(const QRectF* rechtecke, int zahl) override
    {
        for (int i = 0; i < zahl; ++i)
        {
            QPainterPath pfad;
            pfad.addRect(rechtecke[i]);
            merke(pfad, "drawRects");
        }
    }
    void drawEllipse(const QRectF& rahmen) override
    {
        QPainterPath pfad;
        pfad.addEllipse(rahmen);
        merke(pfad, "drawEllipse");
    }
    void drawPolygon(const QPointF* punkte, int zahl, PolygonDrawMode modus) override
    {
        QPainterPath pfad;
        pfad.addPolygon(QPolygonF(QList<QPointF>(punkte, punkte + zahl)));
        if (modus != QPaintEngine::PolylineMode) pfad.closeSubpath();
        merke(pfad, "drawPolygon");
    }
    void drawPixmap(const QRectF&, const QPixmap&, const QRectF&) override {}

    std::vector<Gezeichnet> alles;

private:
    void merke(const QPainterPath& pfad, const char* weg)
    {
        Gezeichnet eintrag;
        eintrag.pfad = m_lage.map(pfad);
        if (m_pinsel.style() != Qt::NoBrush) eintrag.fuellung = m_pinsel.color();
        if (m_stift.style() != Qt::NoPen) eintrag.strich = m_stift.color();
        eintrag.weg = weg;
        alles.push_back(eintrag);
    }

    QTransform m_lage;
    QBrush m_pinsel{Qt::NoBrush};
    QPen m_stift{Qt::NoPen};
};

class Geraet : public QPaintDevice
{
public:
    QPaintEngine* paintEngine() const override { return &schreiber; }
    mutable Schreiber schreiber;

protected:
    int metric(PaintDeviceMetric mass) const override
    {
        switch (mass)
        {
            case PdmWidth: return 200;
            case PdmHeight: return 100;
            case PdmWidthMM: return 53;
            case PdmHeightMM: return 26;
            case PdmNumColors: return 16777216;
            case PdmDepth: return 32;
            case PdmDpiX:
            case PdmDpiY:
            case PdmPhysicalDpiX:
            case PdmPhysicalDpiY: return 96;
            case PdmDevicePixelRatio: return 1;
            case PdmDevicePixelRatioScaled: return static_cast<int>(QPaintDevice::devicePixelRatioFScale());
            default: return 0;
        }
    }
};

const Gezeichnet* mitFarbe(const std::vector<Gezeichnet>& alles, const QColor& farbe)
{
    for (const Gezeichnet& eintrag : alles)
        if (eintrag.fuellung == farbe || (!eintrag.fuellung.isValid() && eintrag.strich == farbe)) return &eintrag;
    return nullptr;
}

}  // namespace

TEST_CASE("Shape Beleg SVG: Qt Svg zeichnet auf ein mitschreibendes Geraet — Kurven, Lage und Farbe kommen an")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    // ein Pfad mit einer Kurve; ein verschobenes Rechteck; ein Kreis in einer
    // verschobenen und vergroesserten Gruppe; eine offene Linie nur mit Strich
    const QByteArray svg = R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 100">
  <path d="M 10 10 C 40 0, 60 40, 90 10 L 90 50 Z" fill="#ff0000"/>
  <rect x="0" y="0" width="20" height="10" fill="#00ff00" transform="translate(100,20)"/>
  <g transform="translate(150,50) scale(2)"><circle cx="0" cy="0" r="10" fill="#0000ff"/></g>
  <path d="M 10 80 L 60 80 L 60 95" stroke="#ffff00" fill="none"/>
</svg>)svg";

    QSvgRenderer leser(svg);
    REQUIRE(leser.isValid());
    Geraet geraet;
    {
        QPainter maler(&geraet);
        REQUIRE(maler.isActive());
        leser.render(&maler, QRectF(0, 0, 200, 100));
    }
    const std::vector<Gezeichnet>& alles = geraet.schreiber.alles;

    // was ankam, in Zahlen — der Beleg soll es nennen, nicht nur bestehen
    std::map<std::string, int> wege;
    for (const Gezeichnet& eintrag : alles) ++wege[eintrag.weg];
    std::string bericht = std::to_string(alles.size()) + " Aufrufe:";
    for (const auto& [weg, zahl] : wege) bericht += " " + weg + " x" + std::to_string(zahl);
    MESSAGE(bericht);
    REQUIRE(alles.size() >= 4);

    // der Pfad: seine Kurve ist eine Kurve geblieben, er liegt, wo er steht, und ist geschlossen
    const Gezeichnet* rot = mitFarbe(alles, QColor(0xff, 0x00, 0x00));
    REQUIRE(rot != nullptr);
    CHECK(hatKurve(rot->pfad));
    CHECK(rot->pfad.elementCount() <= 8);  // nicht in viele Strecken zerlegt
    CHECK(rot->pfad.elementAt(0).x == doctest::Approx(10.0));
    CHECK(rot->pfad.elementAt(0).y == doctest::Approx(10.0));
    CHECK(rot->pfad.boundingRect().right() == doctest::Approx(90.0).epsilon(0.01));
    CHECK(rot->pfad.boundingRect().bottom() == doctest::Approx(50.0).epsilon(0.01));
    {
        const QPainterPath::Element ende = rot->pfad.elementAt(rot->pfad.elementCount() - 1);
        CHECK(ende.x == doctest::Approx(10.0));
        CHECK(ende.y == doctest::Approx(10.0));
    }
    CHECK_FALSE(rot->strich.isValid());

    // das Rechteck: an seiner verschobenen Stelle
    const Gezeichnet* gruen = mitFarbe(alles, QColor(0x00, 0xff, 0x00));
    REQUIRE(gruen != nullptr);
    CHECK(gruen->pfad.boundingRect().left() == doctest::Approx(100.0));
    CHECK(gruen->pfad.boundingRect().top() == doctest::Approx(20.0));
    CHECK(gruen->pfad.boundingRect().width() == doctest::Approx(20.0));
    CHECK(gruen->pfad.boundingRect().height() == doctest::Approx(10.0));

    // der Kreis: Verschiebung und Vergroesserung der Gruppe sind angewandt, und er ist rund
    const Gezeichnet* blau = mitFarbe(alles, QColor(0x00, 0x00, 0xff));
    REQUIRE(blau != nullptr);
    CHECK(hatKurve(blau->pfad));
    CHECK(blau->pfad.boundingRect().left() == doctest::Approx(130.0).epsilon(0.01));
    CHECK(blau->pfad.boundingRect().top() == doctest::Approx(30.0).epsilon(0.01));
    CHECK(blau->pfad.boundingRect().width() == doctest::Approx(40.0).epsilon(0.01));
    CHECK(blau->pfad.boundingRect().height() == doctest::Approx(40.0).epsilon(0.01));

    // die offene Linie: ohne Fuellung, mit ihrem Strich, und nicht geschlossen
    const Gezeichnet* gelb = mitFarbe(alles, QColor(0xff, 0xff, 0x00));
    REQUIRE(gelb != nullptr);
    CHECK_FALSE(gelb->fuellung.isValid());
    CHECK(gelb->strich == QColor(0xff, 0xff, 0x00));
    {
        const QPainterPath::Element anfang = gelb->pfad.elementAt(0);
        const QPainterPath::Element ende = gelb->pfad.elementAt(gelb->pfad.elementCount() - 1);
        CHECK(anfang.x == doctest::Approx(10.0));
        CHECK(anfang.y == doctest::Approx(80.0));
        CHECK(ende.x == doctest::Approx(60.0));
        CHECK(ende.y == doctest::Approx(95.0));
    }
    for (const Gezeichnet& eintrag : alles)
        MESSAGE(eintrag.weg << ": " << eintrag.pfad.elementCount() << " Elemente, Kurve "
                            << std::string(hatKurve(eintrag.pfad) ? "ja" : "nein") << ", Fuellung "
                            << eintrag.fuellung.name().toStdString() << ", Strich "
                            << eintrag.strich.name().toStdString());
}

TEST_CASE("Shape Beleg Pfad: QPainterPath traegt Strecken und Kurven und wird zu Punkten, ohne Qt-Anwendung")
{
    REQUIRE(QCoreApplication::instance() == nullptr);  // hier laeuft keine

    QPainterPath pfad;
    pfad.moveTo(0.0, 0.0);
    pfad.lineTo(100.0, 0.0);
    pfad.cubicTo(150.0, 0.0, 150.0, 100.0, 100.0, 100.0);  // ein Bogen nach rechts
    pfad.lineTo(0.0, 100.0);
    pfad.closeSubpath();

    // die Stuecke stehen im Pfad, wie sie hineinkamen: so liest und schreibt sie die Datei
    int strecken = 0;
    int kurven = 0;
    for (int i = 0; i < pfad.elementCount(); ++i)
    {
        if (pfad.elementAt(i).type == QPainterPath::LineToElement) ++strecken;
        if (pfad.elementAt(i).type == QPainterPath::CurveToElement) ++kurven;
    }
    CHECK(strecken == 3);  // zwei eigene und die schliessende
    CHECK(kurven == 1);

    // zu Punkten: ein geschlossener Zug, der Bogen in mehrere Strecken zerlegt
    const QList<QPolygonF> zuege = pfad.toSubpathPolygons();
    REQUIRE(zuege.size() == 1);
    CHECK(zuege[0].size() > 6);
    CHECK(zuege[0].isClosed());
    double rechts = 0.0;
    for (const QPointF& punkt : zuege[0]) rechts = std::max(rechts, punkt.x());
    CHECK(rechts == doctest::Approx(137.5).epsilon(0.01));  // der Scheitel des Bogens

    // gleich weit entfernte Punkte entlang des Pfads — die Abtastung nach Bogenlaenge
    const double laenge = pfad.length();
    CHECK(laenge > 400.0);  // drei Strecken zu 100 und der Bogen
    CHECK(laenge < 470.0);
    const QPointF anfang = pfad.pointAtPercent(0.0);
    const QPointF ende = pfad.pointAtPercent(1.0);
    CHECK(anfang.x() == doctest::Approx(0.0));
    CHECK(anfang.y() == doctest::Approx(0.0));
    CHECK(ende.x() == doctest::Approx(0.0).epsilon(0.001));
    CHECK(ende.y() == doctest::Approx(0.0).epsilon(0.001));
    const QPointF nachStrecke = pfad.pointAtPercent(pfad.percentAtLength(50.0));
    CHECK(nachStrecke.x() == doctest::Approx(50.0).epsilon(0.01));
    CHECK(nachStrecke.y() == doctest::Approx(0.0).epsilon(0.01));
}
