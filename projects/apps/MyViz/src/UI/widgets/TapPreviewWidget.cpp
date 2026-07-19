/**
 ****************************************************************************************
 * @file   TapPreviewWidget.cpp
 * @brief  TapPreviewWidget implementation (Phase 4 Schritt 6)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/widgets/TapPreviewWidget.hpp"

#include "visualizers/modules/ColorGradientModule.hpp"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

namespace
{
constexpr int kPreviewHeight = 48;

QColor toQColor(const lumi::modules::Color4f& c)
{
    return QColor::fromRgbF(std::clamp(c[0], 0.0f, 1.0f), std::clamp(c[1], 0.0f, 1.0f),
                            std::clamp(c[2], 0.0f, 1.0f), std::clamp(c[3], 0.0f, 1.0f));
}

} // anonymous namespace

TapPreviewWidget::TapPreviewWidget(Mode mode, QWidget* parent)
    : QWidget(parent)
    , m_mode(mode)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void TapPreviewWidget::setData(std::vector<float> data)
{
    if (data == m_data)
    {
        return;  // unchanged buffer -> no repaint
    }
    m_data = std::move(data);
    update();
}

void TapPreviewWidget::setGradient(const lumi::modules::ColorGradientModule* gradient)
{
    m_gradient = gradient;
    m_gradientSignature = gradientSignature();
    update();
}

void TapPreviewWidget::refreshGradient()
{
    // Polled by the preview tick — repaint only on an actual gradient change
    auto signature = gradientSignature();
    if (signature == m_gradientSignature)
    {
        return;
    }
    m_gradientSignature = std::move(signature);
    update();
}

std::vector<float> TapPreviewWidget::gradientSignature() const
{
    std::vector<float> signature;
    if (m_gradient == nullptr)
    {
        return signature;
    }

    const auto& stops = m_gradient->stops();
    signature.reserve(2 + stops.size() * 5);
    signature.push_back(static_cast<float>(static_cast<int>(m_gradient->mode())));
    const auto solid = m_gradient->solidColor();
    signature.insert(signature.end(), {solid[0], solid[1], solid[2], solid[3]});
    for (const auto& stop : stops)
    {
        signature.push_back(stop.position);
        signature.insert(signature.end(),
                         {stop.color[0], stop.color[1], stop.color[2], stop.color[3]});
    }
    return signature;
}

QSize TapPreviewWidget::sizeHint() const
{
    return {200, kPreviewHeight};
}

void TapPreviewWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().base());
    painter.setPen(palette().color(QPalette::Mid));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    switch (m_mode)
    {
        case Mode::Bars:
            paintBars(painter);
            break;
        case Mode::Curve:
            paintCurve(painter);
            break;
        case Mode::ColorStrip:
            paintColorStrip(painter);
            break;
    }
}

void TapPreviewWidget::paintBars(QPainter& painter)
{
    if (m_data.empty())
    {
        return;
    }

    const int count = static_cast<int>(m_data.size());
    const QRectF area = rect().adjusted(2, 2, -2, -2);
    const qreal barWidth = area.width() / count;

    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().highlight());

    for (int i = 0; i < count; ++i)
    {
        const float value = std::clamp(m_data[i], 0.0f, 1.0f);
        const qreal barHeight = area.height() * value;
        painter.drawRect(QRectF(area.left() + i * barWidth,
                                area.bottom() - barHeight,
                                std::max(barWidth - 1.0, 1.0), barHeight));
    }
}

void TapPreviewWidget::paintCurve(QPainter& painter)
{
    if (m_data.size() < 2)
    {
        return;
    }

    const QRectF area = rect().adjusted(2, 2, -2, -2);
    const int count = static_cast<int>(m_data.size());

    // Zero line
    const qreal midY = area.center().y();
    painter.setPen(QPen(palette().color(QPalette::Mid), 1, Qt::DotLine));
    painter.drawLine(QPointF(area.left(), midY), QPointF(area.right(), midY));

    // Decimate to widget resolution — more segments than pixels are invisible
    const int step = std::max(1, count / std::max(1, static_cast<int>(area.width())));

    QPainterPath path;
    bool first = true;
    for (int i = 0; i < count; i += step)
    {
        const qreal x = area.left() + area.width() * i / (count - 1);
        const float value = std::clamp(m_data[i], -1.0f, 1.0f);
        const qreal y = midY - value * area.height() * 0.5;
        if (first)
        {
            path.moveTo(x, y);
            first = false;
        }
        else
        {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(palette().color(QPalette::Highlight), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

void TapPreviewWidget::paintColorStrip(QPainter& painter)
{
    if (m_gradient == nullptr)
    {
        return;
    }

    const auto& stops = m_gradient->stops();
    const QRect area = rect().adjusted(2, 2, -2, -2);

    // Solid mode keeps its color in solidColor(), not in the stop list
    if (m_gradient->mode() == lumi::modules::GradientMode::Solid)
    {
        painter.fillRect(area, toQColor(m_gradient->solidColor()));
        return;
    }

    // One fill with a QLinearGradient — Qt interpolates the stops
    QLinearGradient gradient(area.topLeft(), area.topRight());
    for (const auto& stop : stops)
    {
        gradient.setColorAt(std::clamp(stop.position, 0.0f, 1.0f), toQColor(stop.color));
    }
    painter.fillRect(area, gradient);
}
