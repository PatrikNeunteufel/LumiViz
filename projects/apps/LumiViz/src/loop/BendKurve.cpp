/**
 * @file BendKurve.cpp
 * @brief Loop: die Kurve eines Bends zum Bearbeiten (Stufe 26d, S137)
 */

#include "loop/BendKurve.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

BendKurve::BendKurve(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("bendCurve"));
    setMouseTracking(true);
    setToolTip(tr("Drag a point · double-click an empty spot to add one, a point to remove it.\n"
                  "Left to right: the length of the note; up: the bend, in whole tones as in tabs"));
}

void BendKurve::setzePunkte(const std::vector<BendPunkt>& punkte)
{
    m_punkte = punkte;
    // ohne Bend: eine flache Linie zum Ziehen — zwei Punkte, ohne Biegung
    if (m_punkte.empty()) m_punkte = {{0.0, 0.0}, {1.0, 0.0}};
    update();
}

QRectF BendKurve::feld() const
{
    return QRectF(kRand + 10, kRand / 2.0, width() - kRand - 10 - kRand / 2.0, height() - kRand);
}

QPointF BendKurve::ortVon(const BendPunkt& punkt) const
{
    const QRectF f = feld();
    return {f.left() + punkt.zeit * f.width(), f.bottom() - punkt.halbtoene / kBendHalbtoeneMax * f.height()};
}

BendPunkt BendKurve::punktBei(QPointF ort) const
{
    const QRectF f = feld();
    const double zeit = std::clamp((ort.x() - f.left()) / std::max(1.0, f.width()), 0.0, 1.0);
    const double hoch = std::clamp((f.bottom() - ort.y()) / std::max(1.0, f.height()), 0.0, 1.0) * kBendHalbtoeneMax;
    return {zeit, std::round(hoch / kBendRaster) * kBendRaster};
}

int BendKurve::punktNahe(QPointF ort) const
{
    int bester = -1;
    double abstand = 7.0;
    for (std::size_t i = 0; i < m_punkte.size(); ++i)
    {
        const QPointF d = ortVon(m_punkte[i]) - ort;
        const double weit = std::hypot(d.x(), d.y());
        if (weit <= abstand)
        {
            abstand = weit;
            bester = static_cast<int>(i);
        }
    }
    return bester;
}

void BendKurve::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QColor text = pal.color(QPalette::Text);
    const QColor an = pal.color(QPalette::Highlight);
    p.fillRect(rect(), pal.color(QPalette::Base));
    const QRectF f = feld();
    // je Viertelton eine feine Linie, je Halbton eine kraeftigere mit der Hoehe in Ganztoenen
    for (int viertel = 0; viertel <= static_cast<int>(kBendHalbtoeneMax / kBendRaster); ++viertel)
    {
        const double h = viertel * kBendRaster;
        const double y = ortVon({0.0, h}).y();
        const bool halbton = viertel % 4 == 0;
        QColor linie = text;
        linie.setAlpha(halbton ? 90 : 30);
        p.setPen(QPen(linie, 1.0));
        p.drawLine(QPointF(f.left(), y), QPointF(f.right(), y));
        if (halbton)
        {
            p.setPen(text);
            p.drawText(QRectF(0, y - 9, kRand + 6, 18), Qt::AlignRight | Qt::AlignVCenter, h == 0.0 ? QStringLiteral("0") : bendText(h));
        }
    }
    for (int teil = 0; teil <= 4; ++teil)
    {
        QColor linie = text;
        linie.setAlpha(teil == 0 || teil == 4 ? 90 : 30);
        p.setPen(QPen(linie, 1.0));
        const double x = f.left() + f.width() * teil / 4.0;
        p.drawLine(QPointF(x, f.top()), QPointF(x, f.bottom()));
    }
    // die Kurve: vor dem ersten Punkt und nach dem letzten waagrecht weiter
    QPainterPath kurve;
    kurve.moveTo(ortVon({0.0, m_punkte.front().halbtoene}));
    for (const BendPunkt& punkt : m_punkte) kurve.lineTo(ortVon(punkt));
    kurve.lineTo(ortVon({1.0, m_punkte.back().halbtoene}));
    p.setPen(QPen(an, 2.0));
    p.drawPath(kurve);
    for (std::size_t i = 0; i < m_punkte.size(); ++i)
    {
        p.setPen(QPen(text, 1.2));
        p.setBrush(static_cast<int>(i) == m_gezogen ? an : pal.color(QPalette::Base));
        p.drawEllipse(ortVon(m_punkte[i]), 4.5, 4.5);
    }
}

void BendKurve::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    m_gezogen = punktNahe(event->position());
    update();
}

void BendKurve::mouseMoveEvent(QMouseEvent* event)
{
    if (m_gezogen < 0 || (event->buttons() & Qt::LeftButton) == 0)
    {
        setCursor(punktNahe(event->position()) >= 0 ? Qt::SizeAllCursor : Qt::CrossCursor);
        return;
    }
    BendPunkt neu = punktBei(event->position());
    // zwischen den Nachbarn: die Reihe der Punkte bleibt, wie sie ist
    const auto i = static_cast<std::size_t>(m_gezogen);
    if (i > 0) neu.zeit = std::max(neu.zeit, m_punkte[i - 1].zeit);
    if (i + 1 < m_punkte.size()) neu.zeit = std::min(neu.zeit, m_punkte[i + 1].zeit);
    if (neu == m_punkte[i]) return;
    m_punkte[i] = neu;
    update();
    Q_EMIT geaendert();
}

void BendKurve::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    m_gezogen = -1;
    update();
}

void BendKurve::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    if (const int nahe = punktNahe(event->position()); nahe >= 0)
    {
        // einen nehmen — einer bleibt immer
        if (m_punkte.size() > 1) m_punkte.erase(m_punkte.begin() + nahe);
    }
    else
    {
        if (m_punkte.size() >= static_cast<std::size_t>(kBendPunkteMax)) return;
        const BendPunkt neu = punktBei(event->position());
        const auto stelle = std::upper_bound(m_punkte.begin(), m_punkte.end(), neu,
                                             [](const BendPunkt& a, const BendPunkt& b) { return a.zeit < b.zeit; });
        m_punkte.insert(stelle, neu);
    }
    m_gezogen = -1;
    update();
    Q_EMIT geaendert();
}

}  // namespace lumi::loop
