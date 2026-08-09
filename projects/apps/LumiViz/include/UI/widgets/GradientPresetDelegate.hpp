/**
 ****************************************************************************************
 * @file   GradientPresetDelegate.hpp
 * @brief  Custom delegate for gradient preset preview in combo boxes
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/ColorGradientModule.hpp"

#include <QStyledItemDelegate>
#include <QComboBox>

namespace lumi::ui {

/**
 * @class GradientPresetDelegate
 * @brief Draws gradient preview next to preset names in combo boxes
 */
class GradientPresetDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GradientPresetDelegate(QObject* parent = nullptr);
    
    void setGradientModule(modules::ColorGradientModule* module);
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
               
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    void drawGradientPreview(QPainter* painter, const QRect& rect,
                             const std::string& presetName) const;
    
    modules::ColorGradientModule* m_gradientModule = nullptr;
    
    static constexpr int PREVIEW_WIDTH = 60;
    static constexpr int PREVIEW_HEIGHT = 16;
    static constexpr int MARGIN = 4;
};

} // namespace lumi::ui
