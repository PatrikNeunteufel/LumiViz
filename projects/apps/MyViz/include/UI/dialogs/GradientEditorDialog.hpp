/**
 ****************************************************************************************
 * @file   GradientEditorDialog.hpp
 * @brief  Dialog for editing multi-stop color gradients with midpoints
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/ColorGradientModule.hpp"

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <vector>
#include <functional>

namespace lumi::ui {

// =============================================================================
// GradientBarWidget - Visual gradient editor with draggable stops
// =============================================================================

/**
 * @class GradientBarWidget
 * @brief Custom widget displaying gradient bar with interactive color stops
 */
class GradientBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GradientBarWidget(QWidget* parent = nullptr);
    ~GradientBarWidget() override = default;

    // Set the gradient module to edit
    void setGradient(modules::ColorGradientModule* gradient);
    
    // Refresh the display
    void updateDisplay();

signals:
    void stopSelected(int index);
    void stopMoved(int index, float position);
    void stopColorChanged(int index);
    void midpointMoved(int index, float position);
    void gradientChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    QSize sizeHint() const override { return QSize(400, 60); }
    QSize minimumSizeHint() const override { return QSize(200, 40); }

private:
    float xToPosition(int x) const;
    int positionToX(float pos) const;
    int hitTestStop(const QPoint& pos) const;
    int hitTestMidpoint(const QPoint& pos) const;

    modules::ColorGradientModule* m_gradient = nullptr;
    int m_selectedStop = -1;
    int m_selectedMidpoint = -1;
    bool m_dragging = false;
    
    static constexpr int STOP_SIZE = 12;
    static constexpr int MIDPOINT_SIZE = 8;
    static constexpr int BAR_HEIGHT = 30;
    static constexpr int MARGIN = 10;
};

// =============================================================================
// GradientEditorDialog
// =============================================================================

/**
 * @class GradientEditorDialog
 * @brief Dialog for editing color gradients
 * 
 * Features:
 *   - Visual gradient bar with draggable stops
 *   - Color picker for each stop
 *   - Adjustable midpoints between stops
 *   - Preset save/load/reset
 */
class GradientEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GradientEditorDialog(modules::ColorGradientModule* gradient, 
                                   QWidget* parent = nullptr);
    ~GradientEditorDialog() override = default;

    // Callback when gradient is modified
    using ChangeCallback = std::function<void()>;
    void setChangeCallback(ChangeCallback callback) { m_changeCallback = callback; }

signals:
    void gradientChanged();

private slots:
    void onStopSelected(int index);
    void onColorButtonClicked();
    void onAddStopClicked();
    void onRemoveStopClicked();
    void onPresetChanged(int index);
    void onSavePresetClicked();
    void onResetClicked();
    void onMidpointSliderChanged(int value);

private:
    void setupUi();
    void updateStopControls();
    void updatePresetCombo();
    void notifyChange();

    modules::ColorGradientModule* m_gradient;
    ChangeCallback m_changeCallback;

    // UI Elements
    GradientBarWidget* m_gradientBar = nullptr;
    QPushButton* m_colorButton = nullptr;
    QSlider* m_midpointSlider = nullptr;
    QLabel* m_midpointLabel = nullptr;
    QLabel* m_positionLabel = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_removeButton = nullptr;
    QComboBox* m_presetCombo = nullptr;
    QPushButton* m_savePresetButton = nullptr;
    QPushButton* m_resetButton = nullptr;

    int m_currentStop = -1;
    int m_currentMidpoint = -1;
};

} // namespace lumi::ui
