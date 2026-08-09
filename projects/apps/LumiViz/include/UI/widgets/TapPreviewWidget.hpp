/**
 ****************************************************************************************
 * @file   TapPreviewWidget.hpp
 * @brief  Compact live preview of pipeline-stage data (Phase 4 Schritt 6)
 *
 * Draws tap-point data (bars/curve) or a gradient color strip inside a stage
 * group of the ConfigPanel. Pure QPainter — no OpenGL. Repaints only when the
 * data actually changed; costs nothing while hidden (the panel polls taps
 * only for visible previews).
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <QWidget>

#include <vector>

namespace lumi::modules {
class ColorGradientModule;
}

/**
 * @class TapPreviewWidget
 * @brief Mini visualization of one tap point or gradient handle
 */
class TapPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Mode
    {
        Bars,       ///< band amplitudes 0..1 (mini equalizer)
        Curve,      ///< sample data -1..1 (polyline)
        ColorStrip  ///< gradient handle rendered as horizontal strip
    };

    explicit TapPreviewWidget(Mode mode, QWidget* parent = nullptr);

    /// @brief New tap data (Bars/Curve); repaints only if it changed
    void setData(std::vector<float> data);

    /// @brief Gradient to render (ColorStrip; non-owning, visualizer-owned)
    void setGradient(const lumi::modules::ColorGradientModule* gradient);

    /// @brief Re-check the gradient (ColorStrip); repaints only if it changed
    void refreshGradient();

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paintBars(QPainter& painter);
    void paintCurve(QPainter& painter);
    void paintColorStrip(QPainter& painter);

    [[nodiscard]] std::vector<float> gradientSignature() const;

    Mode m_mode;
    std::vector<float> m_data;
    const lumi::modules::ColorGradientModule* m_gradient = nullptr;
    std::vector<float> m_gradientSignature;  ///< last painted gradient state
};
