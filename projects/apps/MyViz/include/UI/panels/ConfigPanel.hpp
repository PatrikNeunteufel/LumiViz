/**
 ****************************************************************************************
 * @file   ConfigPanel.hpp
 * @brief  Configuration/Settings panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

class QTabWidget;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QSlider;

/**
 * @class ConfigPanel
 * @brief Panel for application settings
 *
 * Provides:
 *   - Audio device selection
 *   - Visualization settings
 *   - Performance settings
 *   - Theme selection
 */
class ConfigPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ConfigPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ConfigPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onAudioDeviceChanged(int index);
    void onFrameModeChanged(int index);
    void onSmoothingChanged(int value);

private:
    void setupUI();
    void setupConnections();
    void populateAudioDevices();

    QWidget* createAudioTab();
    QWidget* createVisualsTab();
    QWidget* createPerformanceTab();

    // UI Elements
    QTabWidget* m_pTabWidget = nullptr;

    // Audio Tab
    QComboBox* m_pAudioDeviceCombo = nullptr;
    QSpinBox* m_pBufferSizeSpinBox = nullptr;
    QSpinBox* m_pSampleRateSpinBox = nullptr;

    // Visuals Tab
    QSlider* m_pSmoothingSlider = nullptr;
    QCheckBox* m_pPeakHoldCheckBox = nullptr;
    QComboBox* m_pColorSchemeCombo = nullptr;

    // Performance Tab
    QComboBox* m_pFrameModeCombo = nullptr;
    QSpinBox* m_pTargetFpsSpinBox = nullptr;
    QCheckBox* m_pVSyncCheckBox = nullptr;
};
