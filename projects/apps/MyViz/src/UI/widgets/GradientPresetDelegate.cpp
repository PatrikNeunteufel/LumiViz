/**
 ****************************************************************************************
 * @file   GradientPresetDelegate.cpp
 * @brief  Custom delegate for gradient preset preview in combo boxes
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/widgets/GradientPresetDelegate.hpp"

#include <QPainter>
#include <QApplication>

namespace lumi::ui {

GradientPresetDelegate::GradientPresetDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void GradientPresetDelegate::setGradientModule(modules::ColorGradientModule* module)
{
    m_gradientModule = module;
}

void GradientPresetDelegate::paint(QPainter* painter, 
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    painter->save();
    
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    bool isSelected = opt.state & QStyle::State_Selected;
    bool isHovered = opt.state & QStyle::State_MouseOver;
    
    // Draw background ourselves for consistent appearance
    if (isSelected)
    {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }
    else if (isHovered)
    {
        // Use a lighter highlight color for hover
        QColor hoverColor = opt.palette.highlight().color();
        hoverColor.setAlpha(128);
        painter->fillRect(opt.rect, hoverColor);
    }
    else
    {
        painter->fillRect(opt.rect, opt.palette.base());
    }
    
    // Get preset name
    QString text = index.data(Qt::DisplayRole).toString();
    std::string presetName = text.toStdString();
    
    // Calculate rects
    QRect previewRect(
        opt.rect.left() + MARGIN,
        opt.rect.top() + (opt.rect.height() - PREVIEW_HEIGHT) / 2,
        PREVIEW_WIDTH,
        PREVIEW_HEIGHT
    );
    
    QRect textRect(
        previewRect.right() + MARGIN * 2,
        opt.rect.top(),
        opt.rect.width() - PREVIEW_WIDTH - MARGIN * 3,
        opt.rect.height()
    );
    
    // Draw gradient preview
    drawGradientPreview(painter, previewRect, presetName);
    
    // Draw text with appropriate contrast
    if (isSelected)
    {
        painter->setPen(opt.palette.highlightedText().color());
    }
    else if (isHovered)
    {
        // Dark text on light hover background
        painter->setPen(opt.palette.text().color());
    }
    else
    {
        painter->setPen(opt.palette.text().color());
    }
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
    
    painter->restore();
}

QSize GradientPresetDelegate::sizeHint(const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    return QSize(baseSize.width() + PREVIEW_WIDTH + MARGIN * 3, 
                 qMax(baseSize.height(), PREVIEW_HEIGHT + MARGIN * 2));
}

void GradientPresetDelegate::drawGradientPreview(QPainter* painter, 
                                                  const QRect& rect,
                                                  const std::string& presetName) const
{
    if (!m_gradientModule)
    {
        painter->fillRect(rect, Qt::gray);
        return;
    }
    
    // Get preset data - temporarily load it to get colors
    // We'll use the builtin presets map or sample the gradient
    
    // Create a temporary copy to sample from
    modules::ColorGradientModule tempModule;
    tempModule.loadPreset(presetName);
    
    // Sample gradient pixel by pixel
    int width = rect.width();
    for (int x = 0; x < width; ++x)
    {
        float t = static_cast<float>(x) / static_cast<float>(width);
        auto c = tempModule.sample(t);
        
        QColor color = QColor::fromRgbF(
            qBound(0.0f, c[0], 1.0f),
            qBound(0.0f, c[1], 1.0f),
            qBound(0.0f, c[2], 1.0f),
            qBound(0.0f, c[3], 1.0f));
        
        painter->setPen(color);
        painter->drawLine(rect.x() + x, rect.y(), 
                          rect.x() + x, rect.y() + rect.height());
    }
    
    // Draw border
    painter->setPen(Qt::darkGray);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}

} // namespace lumi::ui
