/**
 * @file SvgKonturen.cpp
 * @brief Klang aus einer Form: die Konturen eines SVG (Stufe 20b)
 */

#include "shape/SvgKonturen.hpp"

#include "shape/Kontur.hpp"  // kKonturenMax

#include <QBrush>
#include <QColor>
#include <QFile>
#include <QList>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QSvgRenderer>
#include <QTransform>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace lumi::shape
{

namespace
{

/// Kuerzer als das (in Einheiten der viewBox) ist ein Teilpfad keine Kontur.
constexpr double kMindestLaenge = 1e-6;

/// Ein Aufruf, wie er beim Zeichengeraet ankam — der Pfad schon an seiner Stelle.
struct Gezeichnet
{
    QPainterPath pfad;
    QColor farbe;  ///< die Fuellung, ohne sie der Strich; ungueltig = unsichtbar
};

/// Malt nichts: schreibt mit, was gezeichnet werden soll (der Beleg in test_ShapeBelege.cpp).
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

    void drawPath(const QPainterPath& pfad) override { merke(pfad); }
    void drawRects(const QRectF* rechtecke, int zahl) override
    {
        for (int i = 0; i < zahl; ++i)
        {
            QPainterPath pfad;
            pfad.addRect(rechtecke[i]);
            merke(pfad);
        }
    }
    void drawEllipse(const QRectF& rahmen) override
    {
        QPainterPath pfad;
        pfad.addEllipse(rahmen);
        merke(pfad);
    }
    void drawPolygon(const QPointF* punkte, int zahl, PolygonDrawMode modus) override
    {
        QPainterPath pfad;
        pfad.addPolygon(QPolygonF(QList<QPointF>(punkte, punkte + zahl)));
        if (modus != QPaintEngine::PolylineMode) pfad.closeSubpath();
        merke(pfad);
    }
    void drawPixmap(const QRectF&, const QPixmap&, const QRectF&) override {}

    std::vector<Gezeichnet> alles;

private:
    void merke(const QPainterPath& pfad)
    {
        Gezeichnet eintrag;
        eintrag.pfad = m_lage.map(pfad);
        // die Fuellung, wenn sie zu sehen ist — sonst der Strich (eine Linie ohne Fuellung)
        if (m_pinsel.style() != Qt::NoBrush && m_pinsel.color().alpha() > 0)
            eintrag.farbe = m_pinsel.color();
        else if (m_stift.style() != Qt::NoPen && m_stift.color().alpha() > 0)
            eintrag.farbe = m_stift.color();
        alles.push_back(std::move(eintrag));
    }

    QTransform m_lage;
    QBrush m_pinsel{Qt::NoBrush};
    QPen m_stift{Qt::NoPen};
};

class Geraet : public QPaintDevice
{
public:
    explicit Geraet(QSize groesse) : m_groesse(groesse) {}
    QPaintEngine* paintEngine() const override { return &schreiber; }
    mutable Schreiber schreiber;

protected:
    int metric(PaintDeviceMetric mass) const override
    {
        switch (mass)
        {
            case PdmWidth: return m_groesse.width();
            case PdmHeight: return m_groesse.height();
            case PdmWidthMM: return std::max(1, m_groesse.width() * 254 / 960);
            case PdmHeightMM: return std::max(1, m_groesse.height() * 254 / 960);
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

private:
    QSize m_groesse;
};

/// Einen Pfad in seine Teilpfade zerlegen; jeder geschlossen.
void zerlege(const QPainterPath& pfad, const QColor& farbe, std::vector<SvgTeil>& teile)
{
    QPainterPath teil;
    QPointF anfang;
    const auto fertig = [&] {
        if (teil.elementCount() < 2)
        {
            teil = QPainterPath();
            return;
        }
        SvgTeil neu;
        const QPointF ende = teil.currentPosition();
        neu.offen = std::abs(ende.x() - anfang.x()) > 1e-9 || std::abs(ende.y() - anfang.y()) > 1e-9;
        teil.closeSubpath();
        neu.laenge = teil.length();
        if (neu.laenge > kMindestLaenge)
        {
            neu.pfad = teil;
            neu.farbe = farbe.name(QColor::HexRgb);
            teile.push_back(std::move(neu));
        }
        teil = QPainterPath();
    };
    for (int i = 0; i < pfad.elementCount(); ++i)
    {
        const QPainterPath::Element e = pfad.elementAt(i);
        if (e.type == QPainterPath::MoveToElement)
        {
            fertig();
            anfang = QPointF(e.x, e.y);
            teil.moveTo(anfang);
        }
        else if (e.type == QPainterPath::LineToElement)
        {
            teil.lineTo(e.x, e.y);
        }
        else if (e.type == QPainterPath::CurveToElement && i + 2 < pfad.elementCount())
        {
            const QPainterPath::Element zwei = pfad.elementAt(i + 1);
            const QPainterPath::Element ende = pfad.elementAt(i + 2);
            teil.cubicTo(e.x, e.y, zwei.x, zwei.y, ende.x, ende.y);
            i += 2;
        }
    }
    fertig();
}

}  // namespace

SvgKonturen svgKonturen(const QByteArray& inhalt)
{
    SvgKonturen ergebnis;
    QSvgRenderer leser;
    if (inhalt.isEmpty() || !leser.load(inhalt) || !leser.isValid()) return ergebnis;
    ergebnis.gelesen = true;
    QRectF rahmen = leser.viewBoxF();
    if (rahmen.width() <= 0.0 || rahmen.height() <= 0.0) rahmen = QRectF(QPointF(0.0, 0.0), QSizeF(leser.defaultSize()));
    if (rahmen.width() <= 0.0 || rahmen.height() <= 0.0) rahmen = QRectF(0.0, 0.0, 100.0, 100.0);
    ergebnis.rahmen = rahmen;

    // in den Koordinaten der viewBox zeichnen: das Ziel ist die viewBox selbst
    Geraet geraet(QSize(std::max(1, static_cast<int>(std::ceil(rahmen.width()))), std::max(1, static_cast<int>(std::ceil(rahmen.height())))));
    {
        QPainter maler(&geraet);
        leser.render(&maler, rahmen);
    }

    std::vector<SvgTeil> alle;
    // was unsichtbar ist, faellt weg — QPainter reicht einen Pfad ohne Fuellung und Strich gar nicht erst weiter
    for (const Gezeichnet& g : geraet.schreiber.alles)
        if (g.farbe.isValid()) zerlege(g.pfad, g.farbe, alle);

    // ueber kKonturenMax hinaus: die laengsten bleiben, in der Reihenfolge der Datei (O2 a)
    if (alle.size() > static_cast<std::size_t>(kKonturenMax))
    {
        std::vector<std::size_t> platz(alle.size());
        std::iota(platz.begin(), platz.end(), std::size_t{0});
        std::stable_sort(platz.begin(), platz.end(), [&alle](std::size_t a, std::size_t b) { return alle[a].laenge > alle[b].laenge; });
        platz.resize(static_cast<std::size_t>(kKonturenMax));
        std::sort(platz.begin(), platz.end());
        std::vector<SvgTeil> behalten;
        behalten.reserve(platz.size());
        for (const std::size_t p : platz) behalten.push_back(std::move(alle[p]));
        ergebnis.weggefallen = static_cast<int>(alle.size()) - kKonturenMax;
        alle = std::move(behalten);
    }

    // eine Farbe, die alle teilen, ist die der Datei, nicht der Kontur: dann die Farben der Reihe (O3 a)
    if (!alle.empty() &&
        std::all_of(alle.begin(), alle.end(), [&alle](const SvgTeil& t) { return t.farbe == alle.front().farbe; }))
        for (SvgTeil& t : alle) t.farbe.clear();

    // der Haken: alle, solange es hoechstens kSvgAlleGewaehlt sind, sonst nur die laengste (O2 a)
    if (alle.size() > static_cast<std::size_t>(kSvgAlleGewaehlt))
    {
        const auto laengste = std::max_element(alle.begin(), alle.end(), [](const SvgTeil& a, const SvgTeil& b) { return a.laenge < b.laenge; });
        for (auto it = alle.begin(); it != alle.end(); ++it) it->gewaehlt = it == laengste;
    }
    ergebnis.teile = std::move(alle);
    return ergebnis;
}

SvgKonturen liesSvgKonturen(const QString& datei)
{
    QFile quelle(datei);
    if (!quelle.open(QIODevice::ReadOnly)) return {};
    return svgKonturen(quelle.readAll());
}

}  // namespace lumi::shape
