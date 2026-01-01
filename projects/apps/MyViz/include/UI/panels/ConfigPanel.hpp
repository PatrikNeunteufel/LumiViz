/**
 ****************************************************************************************
 * @file   ConfigPanel.hpp
 * @brief  Visualizer configuration panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 *
 * @details
 * ## ConfigPanel
 *
 * Provides configuration for the currently active visualizer:
 * - Smoothing settings
 * - Peak hold toggle
 * - Color scheme selection
 * - Visualizer-specific parameters
 *
 * When multiple VisualizerWidgets exist, this panel controls the active one.
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <vector>

class QComboBox;
class QCheckBox;
class QSlider;
class QLabel;

/**
 * @class ConfigPanel
 * @brief Panel for visualizer configuration
 */
class ConfigPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ConfigPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ConfigPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;

private Q_SLOTS:
    void onSmoothingChanged(int value);
    void onPeakHoldChanged(bool checked);
    void onColorSchemeChanged(int index);

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();

    // UI Elements
    QSlider* m_pSmoothingSlider = nullptr;
    QLabel* m_pSmoothingLabel = nullptr;
    QCheckBox* m_pPeakHoldCheckBox = nullptr;
    QComboBox* m_pColorSchemeCombo = nullptr;
    
    // State
    std::vector<int> m_subscriptionIds;
    bool m_isUpdating = false;
};
