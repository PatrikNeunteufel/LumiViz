/**
 ****************************************************************************************
 * @file   ConfigPanel.hpp
 * @brief  Configuration/Settings panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * ## ConfigPanel
 *
 * Provides application settings:
 * - Audio device selection and configuration
 * - Visualization settings (smoothing, colors)
 * - Performance settings (frame mode, VSync)
 *
 * ## Tabs
 *
 * 1. **Audio** - Device, buffer size, sample rate
 * 2. **Visuals** - Smoothing, peak hold, color scheme
 * 3. **Performance** - Frame mode, target FPS, VSync
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <vector>

class QTabWidget;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QSlider;

/**
 * @class ConfigPanel
 * @brief Panel for application settings
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
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onAudioDeviceChanged(int index);
    void onFrameModeChanged(int index);
    void onTargetFpsChanged(int value);
    void onVSyncChanged(bool checked);
    void onSmoothingChanged(int value);

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();
    void populateAudioDevices();
    void syncWithCurrentSettings();

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
    
    // State
    std::vector<int> m_subscriptionIds;
    bool m_isUpdating = false;  ///< Prevent feedback loops
};
